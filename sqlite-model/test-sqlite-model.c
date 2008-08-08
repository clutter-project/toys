#include <clutter/clutter.h>
#include <sqlite3.h>
#include "clutter-sqlite-model.h"
#include <string.h>
#include <glib/gstdio.h>

/* Test taken from Clutter and modified to use ClutterSqliteModel */

/* gcc -o test-sqlite-model *.c `pkg-config --cflags --libs clutter-0.8 sqlite3` -Wall -g */

#define N_ROWS 1000

enum
{
  COLUMN_FOO,
  COLUMN_BAR,

  N_COLUMNS
};

static sqlite3_stmt *statement = NULL;

static void
set_query (ClutterModel *model, const gchar *query)
{
  sqlite3_stmt *old_stmt;
  sqlite3 *db;
  
  old_stmt = statement;

  g_object_get (G_OBJECT (model), "db", &db, NULL);
  if (sqlite3_prepare (db, query, -1, &statement, NULL) != SQLITE_OK)
    g_error ("Error preparing query: %s", sqlite3_errmsg (db));
  g_object_set (G_OBJECT (model), "statement", statement, NULL);

  if (old_stmt)
    sqlite3_finalize (old_stmt);
}

static void
print_iter (ClutterModelIter *iter,
            const gchar      *text)
{
  ClutterModel *model;
  gint i;
  gchar *string;

  model = clutter_model_iter_get_model (iter);

  clutter_model_iter_get (iter, COLUMN_FOO, &i, COLUMN_BAR, &string, -1);

  g_print ("[row:%02d]: %s: (%s: %d), (%s: %s)\n",
           clutter_model_iter_get_row (iter),
           text,
           clutter_model_get_column_name (model, COLUMN_FOO), i,
           clutter_model_get_column_name (model, COLUMN_BAR), string);

  g_free (string);
}

static gboolean
foreach_func (ClutterModel     *model,
              ClutterModelIter *iter,
              gpointer          dummy)
{
  gint i;
  gchar *string;

  clutter_model_iter_get (iter, COLUMN_FOO, &i, COLUMN_BAR, &string, -1);

  g_print ("[row:%02d]: Foreach: %d, %s\n",
           clutter_model_iter_get_row (iter),
           i, string);
  
  g_free (string);

  return TRUE;
}

static void
on_row_changed (ClutterModel     *model,
                ClutterModelIter *iter)
{
  print_iter (iter, "Changed");
}

static void
filter_model (ClutterModel *model)
{
  ClutterModelIter *iter;

  g_print ("\n* Changing Query: reverse alpha\n");
  set_query (model, "select *,rowid from mytable order by bar desc;");

  g_signal_connect (model, "row-changed", G_CALLBACK (on_row_changed), NULL);
  
  iter = clutter_model_get_iter_at_row (model, 0);
  clutter_model_iter_set (iter, COLUMN_BAR, "Changed string of 0th row, "
                                            "automatically gets sorted",
                                -1);
  g_object_unref (iter);

  clutter_model_foreach (model, foreach_func, NULL);

  g_print ("\n* Unset filter\n");
  clutter_model_set_filter (model, NULL, NULL, NULL);

  while (clutter_model_get_n_rows (model))
    clutter_model_remove (model, 0);
  
  clutter_main_quit ();
}

static void
iterate (ClutterModel *model)
{
  ClutterModelIter *iter;
  
  iter = clutter_model_get_first_iter (model);

  while (!clutter_model_iter_is_last (iter))
    {
      print_iter (iter, "Forward Iteration");
      iter = clutter_model_iter_next (iter);
    }
  g_object_unref (iter);

  iter = clutter_model_get_last_iter (model);  
  do
    {
      print_iter (iter, "Reverse Iteration");
      iter = clutter_model_iter_prev (iter);
    }
  while (!clutter_model_iter_is_first (iter));
  
  print_iter (iter, "Reverse Iteration");
  g_object_unref (iter);

  filter_model (model);
}


static gboolean
populate_model (ClutterModel *model)
{
  gint i;

  for (i = 0; i < N_ROWS; i++)
    {
      gchar *string = g_strdup_printf ("String %d", i);

      clutter_model_append (model,
                            COLUMN_FOO, i,
                            COLUMN_BAR, string,
                            -1);
      g_free (string);
    }

  clutter_model_foreach (model, foreach_func, NULL);
  iterate (model);

  return FALSE;
}

static void
on_row_added (ClutterModel     *model,
              ClutterModelIter *iter,
              gpointer          dummy)
{
  gint i;
  gchar *string;

  clutter_model_iter_get (iter, COLUMN_FOO, &i, COLUMN_BAR, &string, -1);

  g_print ("[row:%02d]: Added: %d, %s\n",
           clutter_model_iter_get_row (iter),
           i, string);

  g_free (string);
}

static void
on_row_removed (ClutterModel     *model,
                ClutterModelIter *iter,
                gpointer          dummy)
{
  print_iter (iter, "Removed");
}

static void
on_sort_changed (ClutterModel *model)
{
  g_print ("*** Sort Changed   ***\n\n");
  clutter_model_foreach (model, foreach_func, NULL);
}

static void
on_filter_changed (ClutterModel *model)
{
  g_print ("*** Filter Changed ***\n\n");
}
 
int
main (int argc, char *argv[])
{
  sqlite3         *db;
  ClutterModel    *model;
  const gchar     *file = "test-sqlite-db.db";

  clutter_init (&argc, &argv);

  if (sqlite3_open (file, &db))
    g_error ("Error creating database: %s", sqlite3_errmsg (db));
  
  if (sqlite3_exec (db,
                    "CREATE TABLE IF NOT EXISTS mytable(foo int, bar text);",
                    NULL,
                    NULL,
                    NULL))
    g_error ("Can't create table: %s", sqlite3_errmsg (db));
  
  model = clutter_sqlite_model_new (db, "mytable",
                                    "foo", SQLITE_INTEGER,
                                    "bar", SQLITE_TEXT,
                                    NULL);

  set_query (model, "select *,rowid from mytable order by bar;");

  g_timeout_add (1000, (GSourceFunc) populate_model, model);

  g_signal_connect (model, "row-added",
                    G_CALLBACK (on_row_added), NULL);
  g_signal_connect (model, "row-removed",
                    G_CALLBACK (on_row_removed), NULL);
  g_signal_connect (model, "sort-changed",
                    G_CALLBACK (on_sort_changed), NULL);
  g_signal_connect (model, "filter-changed",
                    G_CALLBACK (on_filter_changed), NULL);

  clutter_main();
  
  g_object_unref (model);
  
  g_remove (file);
  
  return 0;
}

