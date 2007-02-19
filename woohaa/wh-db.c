#include <stdio.h>
#include <sqlite3.h>
#include <libgnomevfs/gnome-vfs.h>
#include <glib.h>

#include "wh-db.h"
#include "wh-video-model.h"
#include "wh-video-model-row.h"

#define SQL_CREATE_TABLES \
 "CREATE TABLE IF NOT EXISTS meta(path text, n_views int, active int, " \
 "                  vtime integer, mtime integer, thumbnail blob, "      \
 "                  primary key (path), unique(path));" 

enum 
  {
    SQL_GET_ROW_VIA_PATH = 0,
    SQL_SET_ACTIVE_VIA_PATH,
    SQL_ADD_NEW_ROW,
    SQL_GET_ACTIVE_ROWS,
    SQL_UPDATE_ROW,
    N_SQL_STATEMENTS
  };

gchar *SQLStatementText[] = 
  {
    "select * from meta where path=:path;",
    "update meta set active=1, mtime=:mtime where path=:path;",
    "insert into meta(path, n_views, active, vtime, mtime, thumbnail)"
    "           values(:path, 0, 1, 0, :mtime, 0);",
    "select path, n_views, vtime, mtime, thumbnail from meta where active=1;",
    "update meta set n_views=:n_views, vtime=:vtime  where path=:path;"
  };

static sqlite3_stmt *SQLStatements[N_SQL_STATEMENTS];

static gboolean
wh_db_add_path (sqlite3 *db, const gchar *uri);

static void
wh_db_create_sql_statements (sqlite3 *db)
{
  gint i;

  for (i=0;i<N_SQL_STATEMENTS;i++)
  if (sqlite3_prepare(db, SQLStatementText[i], -1, 
		      &SQLStatements[i], NULL) != SQLITE_OK)
    g_warning("Failed to prepare '%s' : %s", 
	      SQLStatementText[i], sqlite3_errmsg(db));

  /* NOTE: call sqlite3_finalize(sqlite3_stmt *pStmt); to free */
}

static gboolean 
wh_db_path_exists (sqlite3 *db, const gchar *uri)
{
  gboolean      res = FALSE;
  sqlite3_stmt *stmt = SQLStatements[SQL_GET_ROW_VIA_PATH];
  
  sqlite3_bind_text (stmt, 1, uri, -1, SQLITE_STATIC);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    res = TRUE;

  sqlite3_reset(stmt);

  return res;
}

void
wh_db_populate_model (sqlite3 *db, WHVideoModel *model)
{
  sqlite3_stmt *stmt = SQLStatements[SQL_GET_ACTIVE_ROWS];

  while (sqlite3_step(stmt) == SQLITE_ROW)
    {
      WHVideoModelRow *row;
      gchar           *title;

      row = wh_video_model_row_new ();

      /* path, n_views, vtime, mtime, thumbnail */

      wh_video_model_row_set_path 
	(row, (const gchar*)sqlite3_column_text(stmt, 0));

      title = g_path_get_basename ((const gchar*)sqlite3_column_text(stmt, 0));
      wh_video_model_row_set_title (row, title);
      g_free(title);

      wh_video_model_row_set_n_views (row, sqlite3_column_int(stmt, 1));
      wh_video_model_row_set_age (row, sqlite3_column_int(stmt, 3));

      wh_video_model_row_set_renderer (row, wh_video_row_renderer_new (row)); 

      wh_video_model_append_row (model, row);
    }

  sqlite3_reset(stmt);
}

void
wh_db_sync_row (sqlite3 *db, WHVideoModelRow *row)
{
  sqlite3_stmt *stmt = SQLStatements[SQL_UPDATE_ROW];

  sqlite3_bind_int (stmt, 1, wh_video_model_row_get_n_views (row));
  sqlite3_bind_int (stmt, 2, wh_video_model_row_get_vtime (row));
  sqlite3_bind_text (stmt, 3, wh_video_model_row_get_path (row), 
		     -1, SQLITE_STATIC);
  sqlite3_step(stmt);
  sqlite3_reset(stmt);
}

static gboolean
wh_db_add_file (sqlite3 *db, const gchar *uri, time_t mtime)
{
  /* See if we already have file in db.
   *  YES - mark active.
   *  NO  - add it set vtime, n_views to 0 etc
  */

  if (wh_db_path_exists (db, uri))
    {
      sqlite3_stmt *stmt = SQLStatements[SQL_SET_ACTIVE_VIA_PATH];

      sqlite3_bind_int (stmt, 1, mtime);
      sqlite3_bind_text (stmt, 2, uri, -1, SQLITE_STATIC);

      sqlite3_step(stmt);
      sqlite3_reset(stmt);
    }
  else
    {
      sqlite3_stmt *stmt = SQLStatements[SQL_ADD_NEW_ROW];
  
      sqlite3_bind_text (stmt, 1, uri, -1, SQLITE_STATIC);
      sqlite3_bind_int (stmt, 2, mtime); /* mtime */

      sqlite3_step(stmt);
      sqlite3_reset(stmt);
    }

  return TRUE;
}

static gboolean
wh_db_add_dir (sqlite3 *db, const gchar *uri)
{
  GnomeVFSResult           vfs_result;
  GnomeVFSDirectoryHandle *vfs_handle = NULL;
  GnomeVFSFileInfoOptions  vfs_options;
  GnomeVFSFileInfo        *vfs_info = NULL;
  gboolean                 ret = TRUE;

  vfs_options = 
    GNOME_VFS_FILE_INFO_DEFAULT
    |GNOME_VFS_FILE_INFO_FOLLOW_LINKS
    |GNOME_VFS_FILE_INFO_GET_ACCESS_RIGHTS;

  vfs_result = gnome_vfs_directory_open (&vfs_handle, uri, vfs_options);

  if (vfs_result != GNOME_VFS_OK)	
    goto cleanup;

  vfs_info = gnome_vfs_file_info_new ();

  while (gnome_vfs_directory_read_next(vfs_handle, vfs_info) == GNOME_VFS_OK)
    {
      if (vfs_info->name
	  && strcmp(vfs_info->name, ".")
	  && strcmp(vfs_info->name, ".."))
	{
	  gchar *entry_uri = NULL;

	  entry_uri = g_strconcat(uri, "/", vfs_info->name, NULL);

	  if (entry_uri)
	    {
	      ret |= wh_db_add_path (db, entry_uri); 
	      g_free(entry_uri);
	    }
	}
    }

 cleanup:
  if (vfs_info)
    gnome_vfs_file_info_unref (vfs_info);
  
  if (vfs_handle)
    gnome_vfs_directory_close (vfs_handle);

 return ret;
}


static gboolean
wh_db_add_path (sqlite3 *db, const gchar *uri)
{
  GnomeVFSResult          vfs_result;
  GnomeVFSFileInfo       *vfs_info = NULL;
  GnomeVFSFileInfoOptions vfs_options;
  gboolean                ret = FALSE;

  vfs_options = 
    GNOME_VFS_FILE_INFO_DEFAULT
    |GNOME_VFS_FILE_INFO_FOLLOW_LINKS
    |GNOME_VFS_FILE_INFO_GET_ACCESS_RIGHTS;

  vfs_info = gnome_vfs_file_info_new ();
    
  vfs_result = gnome_vfs_get_file_info (uri, vfs_info, vfs_options);

  if (vfs_result != GNOME_VFS_OK)	
    goto cleanup;

  if (! (vfs_info->valid_fields & (GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS
				   |GNOME_VFS_FILE_INFO_FIELDS_TYPE)))
    goto cleanup;

  if (! (vfs_info->permissions & GNOME_VFS_PERM_ACCESS_READABLE))
    goto cleanup;


  if (vfs_info->type == GNOME_VFS_FILE_TYPE_DIRECTORY)
    {
      ret = wh_db_add_dir (db,uri);
    }
  else if (vfs_info->type == GNOME_VFS_FILE_TYPE_REGULAR)
    {
      time_t mtime = 0;

      if (vfs_info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_MTIME)
	mtime = vfs_info->mtime;

      /* FIXME: use gstreamer tag foo |gvfs mime type to identify */
      if (g_str_has_suffix(uri, ".avi")
	  || g_str_has_suffix(uri, ".mpeg")
	  || g_str_has_suffix(uri, ".wmv")
	  || g_str_has_suffix(uri, ".mov")
	  || g_str_has_suffix(uri, ".mpg"))
	{
	  wh_db_add_file (db, uri, mtime);
	}
      
      ret = TRUE;
    }

 cleanup:
 
 if (vfs_info)
    gnome_vfs_file_info_unref (vfs_info);
  
  return ret;
}

static void
wh_db_create_tables (sqlite3 *db)
{ 
  /* FIXME: create our tableif not already exisiting */
  if (sqlite3_exec(db, SQL_CREATE_TABLES, NULL, NULL, NULL))
    g_warning("Can't create table: %s\n", sqlite3_errmsg(db));

  /* Next mark fields inactive */
  if (sqlite3_exec(db, "update meta set active=0;", NULL, NULL, NULL))
    g_warning("Can't mark table inactive: %s\n", sqlite3_errmsg(db));
}

sqlite3*
wh_db_init (const char *paths)
{
  sqlite3 *db;
  int      rc, i = 0;
  gchar   *p, **pathv;
  
  gnome_vfs_init ();

  rc = sqlite3_open("/tmp/foodb", &db);

  if (rc)
    {
      g_error("Can't open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return NULL;
    }

  /* FIXME: error checks */
  wh_db_create_tables (db);

  wh_db_create_sql_statements (db);

  pathv = g_strsplit (paths, ":", 0);

  while (pathv[i] != NULL)
    wh_db_add_path (db, pathv[i++]);

  g_strfreev(pathv) ;

  return db;
}

#if 0
int 
main (int argc, char **argv)
{
  sqlite3 *db;

  db = wh_db_init (argv[1]);

}
#endif
