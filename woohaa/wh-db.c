#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <libgnomevfs/gnome-vfs.h>
#include <glib.h>
#include <sys/types.h>
#include <regex.h>
#include <gdk-pixbuf/gdk-pixdata.h>

#include "wh-db.h"
#include "wh-video-model.h"
#include "wh-video-model-row.h"

 #include "wh-video-model.h"
#include <string.h>

G_DEFINE_TYPE (WHDB, wh_db, G_TYPE_OBJECT);

#define DB_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), WH_TYPE_DB, WHDBPrivate))

typedef struct _WHDBPrivate WHDBPrivate;

struct _WHDBPrivate
{
  sqlite3 *db;
};

enum
{
  ROW_CREATED,
  ROW_DELETED,
  LAST_SIGNAL
};

static guint _db_signals[LAST_SIGNAL] = { 0 };

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

static gchar *SQLStatementText[] = 
  {
    "select n_views, vtime, mtime, thumbnail from meta where path=:path;",
    "update meta set active=1, mtime=:mtime where path=:path;",
    "insert into meta(path, n_views, active, vtime, mtime, thumbnail)"
    "           values(:path, 0, 1, 0, :mtime, 0);",
    "select path, n_views, vtime, mtime, thumbnail from meta where active=1;",
    "update meta set thumbnail=:thumbnail, n_views=:n_views, vtime=:vtime "
    "            where path=:path;"
  };

static sqlite3_stmt *SQLStatements[N_SQL_STATEMENTS];

static gboolean
wh_db_walk_directory (WHDB *db, const gchar *uri);

static void
wh_db_media_file_found (WHDB                    *db, 
			const char              *uri,
			GnomeVFSFileInfo        *vfs_info);

static void 
on_vfs_monitor_event (GnomeVFSMonitorHandle   *handle,
		      const gchar             *monitor_uri,
		      const gchar             *info_uri,
		      GnomeVFSMonitorEventType event_type,
		      gpointer                 user_data);

static void
wh_db_get_property (GObject *object, guint property_id,
			     GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
wh_db_set_property (GObject *object, guint property_id,
				  const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
wh_db_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (wh_db_parent_class)->dispose)
    G_OBJECT_CLASS (wh_db_parent_class)->dispose (object);
}

static void
wh_db_finalize (GObject *object)
{
  G_OBJECT_CLASS (wh_db_parent_class)->finalize (object);
}

static void
wh_db_class_init (WHDBClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (WHDBPrivate));

  object_class->get_property = wh_db_get_property;
  object_class->set_property = wh_db_set_property;
  object_class->dispose = wh_db_dispose;
  object_class->finalize = wh_db_finalize;

  _db_signals[ROW_CREATED] =
    g_signal_new ("row-created",
		  G_OBJECT_CLASS_TYPE (object_class),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (WHDBClass, row_created),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__OBJECT,
		  G_TYPE_NONE, 1, WH_TYPE_VIDEO_MODEL_ROW);
}

static void
wh_db_init (WHDB *self)
{
  int           res, i;
  const gchar  *data_dir;
  gchar        *db_filename, *path;
  WHDBPrivate  *priv = DB_PRIVATE(self);

  gnome_vfs_init ();

  data_dir = g_get_user_data_dir ();

  db_filename = g_build_filename (data_dir, "woohaa", "db", NULL);
  path = g_path_get_dirname (db_filename);
  g_mkdir_with_parents (path, 0755);

  res = sqlite3_open(db_filename, &priv->db);

  g_free(path); 
  g_free(db_filename);

  if (res)
    {
      g_error("Can't open database: %s\n", sqlite3_errmsg(priv->db));
      sqlite3_close(priv->db);
      return;
    }

  /* Create DB if not already existing - preexisting will silently fail */
  if (sqlite3_exec(priv->db, SQL_CREATE_TABLES, NULL, NULL, NULL))
    g_warning("Can't create table: %s\n", sqlite3_errmsg(priv->db));
  
  /* Next mark fields inactive */
  if (sqlite3_exec(priv->db, "update meta set active=0;", NULL, NULL, NULL))
    g_warning("Can't mark table inactive: %s\n", sqlite3_errmsg(priv->db));
  
  /* precompile statements */
  for (i=0; i<N_SQL_STATEMENTS; i++)
    if (sqlite3_prepare(priv->db, SQLStatementText[i], -1, 
			&SQLStatements[i], NULL) != SQLITE_OK)
      g_warning("Failed to prepare '%s' : %s", 
		SQLStatementText[i], sqlite3_errmsg(priv->db));
}

WHDB*
wh_db_new ()
{
  return g_object_new (WH_TYPE_DB, NULL);
}

gboolean
uri_is_media (const gchar *uri)
{
  /* Suck */
  /* FIXME: use gstreamer tag foo |gvfs mime type to identify */
  return (g_str_has_suffix(uri, ".avi")
	  || g_str_has_suffix(uri, ".mpeg")
	  || g_str_has_suffix(uri, ".wmv")
	  || g_str_has_suffix(uri, ".mov")
	  || g_str_has_suffix(uri, ".mpg"));
}

gboolean
wh_db_import_uri (WHDB *db, const gchar *uri)
{
  GnomeVFSResult          vfs_result;
  GnomeVFSFileInfo       *vfs_info = NULL;
  GnomeVFSFileInfoOptions vfs_options;
  gboolean                ret = FALSE;

  vfs_options = 
    GNOME_VFS_FILE_INFO_DEFAULT
    |GNOME_VFS_FILE_INFO_FOLLOW_LINKS
    |GNOME_VFS_FILE_INFO_GET_ACCESS_RIGHTS;

  /* FIXME: hack - really I think importing needs to be in a thread */
  while (g_main_context_pending (NULL))
    g_main_context_iteration (NULL, FALSE);

  vfs_info   = gnome_vfs_file_info_new ();
  vfs_result = gnome_vfs_get_file_info (uri, vfs_info, vfs_options);

  if (vfs_result != GNOME_VFS_OK)
    goto cleanup;

  if (! (vfs_info->valid_fields & (GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS
				   |GNOME_VFS_FILE_INFO_FIELDS_TYPE)))
    goto cleanup;

  /* GNOME_VFS_PERM_ACCESS_READABLE would be better, but only the
   * file method implements it */
  if (! (vfs_info->permissions & GNOME_VFS_PERM_USER_READ))
    goto cleanup;

  if (vfs_info->type == GNOME_VFS_FILE_TYPE_DIRECTORY)
    {
      GnomeVFSMonitorHandle   *monitor_handle;

      ret = wh_db_walk_directory (db, uri);

      gnome_vfs_monitor_add (&monitor_handle,
			     uri,
			     GNOME_VFS_MONITOR_DIRECTORY,
			     on_vfs_monitor_event,
			     db);
    }
  else if (vfs_info->type == GNOME_VFS_FILE_TYPE_REGULAR)
    {
      if (uri_is_media(uri))
	  wh_db_media_file_found (db, uri, vfs_info); 

      ret = TRUE;
    }

 cleanup:

 if (vfs_info)
    gnome_vfs_file_info_unref (vfs_info);
  
  return ret;
}

static gboolean
wh_db_walk_directory (WHDB *db, const gchar *uri)
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

  /* FIXME: hack - really I think importing needs to be in a thread */
  while (g_main_context_pending (NULL))
    g_main_context_iteration (NULL, FALSE);

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
	      ret |= wh_db_import_uri (db, entry_uri); 
	      g_free(entry_uri);
	    }
	}

      /* FIXME: hack - really I think importing needs to be in a thread */
      while (g_main_context_pending (NULL))
	g_main_context_iteration (NULL, FALSE);
    }

 cleanup:
  if (vfs_info)
    gnome_vfs_file_info_unref (vfs_info);
  
  if (vfs_handle)
    gnome_vfs_directory_close (vfs_handle);

 return ret;
}

static gboolean 
wh_db_get_uri (const gchar *uri, 
	       gint        *n_views, 
	       gint        *vtime, 
	       gint        *mtime,
	       GdkPixbuf  **thumb)
{
  gboolean      res = FALSE;
  sqlite3_stmt *stmt = SQLStatements[SQL_GET_ROW_VIA_PATH];
  
  sqlite3_bind_text (stmt, 1, uri, -1, SQLITE_STATIC);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    {
      if (n_views)
	*n_views = sqlite3_column_int(stmt, 0);
      if (vtime)
	*vtime = sqlite3_column_int(stmt, 1);
      if (mtime)
	*mtime = sqlite3_column_int(stmt, 2);

      if (thumb)
	{
	  int         len;
	  GdkPixdata *pixdata;
	  guint8 *blob = NULL;

	  blob = (guint8 *)sqlite3_column_blob (stmt, 3);
	  len  = sqlite3_column_bytes (stmt, 3);

	  if (sqlite3_column_type (stmt,3) == SQLITE_BLOB)
	    {
	      pixdata = g_new0 (GdkPixdata, 1);

	      if (gdk_pixdata_deserialize (pixdata, len, (const guint8*)blob, 
					   NULL))
		*thumb = gdk_pixbuf_from_pixdata (pixdata, TRUE, NULL);

	      g_free (pixdata);
	    }
	}
      res = TRUE;
    }

  sqlite3_reset(stmt);

  return res;
}

static gchar*
wh_db_parse_video_uri_info (const char *uri,
			    gchar     **series,
			    gchar     **episode)
{
  gchar     *base, *res;
  regex_t   *regex;
  size_t     nmatch = 4;
  regmatch_t pmatch[4];

  /* HAXOR Regexp to extract 'meta data' from common TV show naming  */
#define TV_REGEXP "(.*)\\.?[Ss]+([0-9]+)[._ ]*[Ee]+[Pp]*([0-9]+)"

  base = g_path_get_basename (uri);

  regex = g_malloc0(sizeof(regex_t));

  if (regcomp(regex, TV_REGEXP, REG_EXTENDED) != 0)
    {
      printf("regexp creation failed\n");
    }

  if (regexec(regex, base, nmatch, pmatch, 0) == 0)
    {
      char *name;

      name = g_strndup (base + pmatch[1].rm_so, 
			pmatch[1].rm_eo - pmatch[1].rm_so);

      name = g_strdelimit (name, "._", ' ');

      *series =  g_strndup (base + pmatch[2].rm_so, 
			    pmatch[2].rm_eo - pmatch[2].rm_so);

      *episode = g_strndup (base + pmatch[3].rm_so, 
		            pmatch[3].rm_eo - pmatch[3].rm_so);

      res = name;

      if (res == NULL || *res == 0)
	{
	  char *dirname;

	  /* Assume we have series & episode but no name so grab
	   * name from parent direcory - handles show-name/s01e01.avi
           * style naiming.
           */  
	  dirname = g_path_get_dirname (uri);
	  name = g_path_get_basename (dirname);
	  g_free (dirname);
	  
	  name = g_strdelimit (name, "._", ' ');
      
	  res = name;
	}

      g_free (base);
    }
  else
    {
      gchar *p;

      p = g_strrstr (base, "."); *p = '\0';
      base = g_strdelimit (base, "._", ' ');

      res = base;
    }

  g_free (regex);

  return res;
}

static void
wh_db_media_file_found (WHDB                    *db, 
			const char              *uri,
			GnomeVFSFileInfo        *vfs_info)
{
  WHVideoModelRow *row;
  gchar           *title, *episode = NULL, *series = NULL;
  gint             n_views = 0, mtime = 0, vtime = 0;  
  GdkPixbuf       *thumb = NULL;

  /* See if we already have file in db.
   *  YES - mark active.
   *  NO  - add it set vtime, n_views to 0 etc
  */
  if (wh_db_get_uri (uri, &n_views, &vtime, &mtime, &thumb))
    {
      /* Update  */
      if (vfs_info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_MTIME)
	mtime = vfs_info->mtime;

      sqlite3_stmt *stmt = SQLStatements[SQL_SET_ACTIVE_VIA_PATH];

      sqlite3_bind_int (stmt, 1, mtime);
      sqlite3_bind_text (stmt, 2, uri, -1, SQLITE_STATIC);

      sqlite3_step(stmt);
      sqlite3_reset(stmt);
    }
  else
    {
      /* New - create row entry*/
      if (vfs_info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_MTIME)
	mtime = vfs_info->mtime;

      sqlite3_stmt *stmt = SQLStatements[SQL_ADD_NEW_ROW];
  
      sqlite3_bind_text (stmt, 1, uri, -1, SQLITE_STATIC);
      sqlite3_bind_int (stmt, 2, mtime); /* mtime */

      sqlite3_step(stmt);
      sqlite3_reset(stmt);
    }

  row = wh_video_model_row_new ();
  wh_video_model_row_set_path (row, uri);

  title = wh_db_parse_video_uri_info ((const char *)uri,
				      &series,
				      &episode);

  wh_video_model_row_set_title (row, title);
  wh_video_model_row_set_extended_info (row, series, episode);

  g_free(title);

  if (thumb)
    {
      wh_video_model_row_set_thumbnail (row, thumb);
      g_object_unref (thumb);
    }

  wh_video_model_row_set_n_views (row, n_views);
  wh_video_model_row_set_age (row, mtime);
  wh_video_model_row_set_vtime (row, vtime);

  g_signal_emit (db, _db_signals[ROW_CREATED], 0, row);

  g_object_unref (row);
}

void
wh_db_sync_row (WHVideoModelRow *row)
{
  GdkPixdata   *pixdata = NULL;
  GdkPixbuf    *pixbuf = NULL;
  guint8       *data = NULL;
  sqlite3_stmt *stmt = SQLStatements[SQL_UPDATE_ROW];

  sqlite3_bind_int (stmt, 2, wh_video_model_row_get_n_views (row));
  sqlite3_bind_int (stmt, 3, wh_video_model_row_get_vtime (row));

  pixbuf = wh_video_model_row_get_thumbnail (row);

  if (pixbuf)
    {
      guint       len = 0;

      pixdata = g_new0 (GdkPixdata, 1);
      gdk_pixdata_from_pixbuf (pixdata, pixbuf, FALSE);

      data = gdk_pixdata_serialize (pixdata, &len);

      sqlite3_bind_blob(stmt, 1, (void*)data, len, SQLITE_STATIC);
    }
  else
    {
      sqlite3_bind_null(stmt, 1);
    }

  sqlite3_bind_text (stmt, 4, wh_video_model_row_get_path (row), 
		     -1, SQLITE_STATIC);

  sqlite3_step(stmt);
  sqlite3_reset(stmt);

  g_free (pixdata);
  g_free (data);
}

static void 
on_vfs_monitor_event (GnomeVFSMonitorHandle   *handle,
		      const gchar             *monitor_uri,
		      const gchar             *info_uri,
		      GnomeVFSMonitorEventType event_type,
		      gpointer                 user_data)
{
  WHDB *db = (WHDB*)user_data;

  if (event_type == GNOME_VFS_MONITOR_EVENT_CREATED)
    {
      wh_db_import_uri (db, info_uri);
      return;
    }

  if (event_type == GNOME_VFS_MONITOR_EVENT_DELETED)
    printf("file '%s' deleted\n", info_uri);

  if (event_type == GNOME_VFS_MONITOR_EVENT_CHANGED)
    printf("file '%s' changed\n", info_uri);
}
