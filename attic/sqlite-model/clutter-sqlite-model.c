/*
 * ClutterSqliteModel
 *
 * An sqlite3-backed ClutterModel implementation.
 *
 * Authored By Chris Lord  <chris@openedhand.com>
 *
 * Copyright (C) 2008 OpenedHand
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * NB: Inspiration taken from the 'woohaa' toy by Matthew Allum and 
 *     GValue conversion code copied from ClutterListModel, by
 *     Matthew Allum, Neil Jagdish Patel and Emmanuele Bassi.
 */

#include <string.h>
#include <glib.h>

#include "clutter-sqlite-model.h"

#define CLUTTER_SQLITE_TYPE_MODEL_ITER            (clutter_sqlite_model_iter_get_type())
#define CLUTTER_SQLITE_MODEL_ITER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), \
                                                   CLUTTER_SQLITE_TYPE_MODEL_ITER,   \
                                                   ClutterSqliteModelIter))
#define CLUTTER_SQLITE_IS_MODEL_ITER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), \
                                                   CLUTTER_SQLITE_TYPE_MODEL_ITER))
#define CLUTTER_SQLITE_MODEL_ITER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                                   CLUTTER_SQLITE_TYPE_MODEL_ITER,   \
                                                   ClutterSqliteModelIterClass))
#define CLUTTER_SQLITE_IS_MODEL_ITER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                                                   CLUTTER_SQLITE_TYPE_MODEL_ITER))
#define CLUTTER_SQLITE_MODEL_ITER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                                   CLUTTER_SQLITE_TYPE_MODEL_ITER,   \
                                                   ClutterSqliteModelIterClass))

typedef struct _ClutterSqliteModelIter      ClutterSqliteModelIter;
typedef struct _ClutterSqliteModelIterClass ClutterSqliteModelIterClass;

struct _ClutterSqliteModelIter
{
  ClutterModelIter  parent_instance;
  
  guint             is_parent;
  gboolean          is_last;
  gint              row;
  gint              rowid;
};

struct _ClutterSqliteModelIterClass
{
  ClutterModelIterClass parent_class;
};

G_DEFINE_TYPE (ClutterSqliteModel, clutter_sqlite_model, CLUTTER_TYPE_MODEL)
G_DEFINE_TYPE (ClutterSqliteModelIter, clutter_sqlite_model_iter, \
               CLUTTER_TYPE_MODEL_ITER)

#define MODEL_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CLUTTER_SQLITE_TYPE_MODEL, \
   ClutterSqliteModelPrivate))

enum
{
  PROP_0,
  
  PROP_DB,
  PROP_TABLE,
  PROP_COL_NAMES,
  PROP_COL_TYPES,
  PROP_STATEMENT,
};

enum 
{
  SQL_ADD_ROW = 0,
  SQL_GET_ROW,
  SQL_DELETE_ROW,
  N_SQL_STATEMENTS
};

/* TODO: Optimisation: Do select statements like the update statements and have
 *       a separate query per column.
 */
static const gchar *sql_statements[] =
  {
    "insert into %s(\"%s\") values(NULL);",
    "select *,rowid from %s where rowid=:rowid;",
    "delete from %s where rowid=:rowid;",
  };

static const gchar *sql_update_statement =
  "update %s set %s=:value where rowid=:rowid;";

struct _ClutterSqliteModelPrivate
{
  sqlite3            *db;
  gchar             **col_names;
  ClutterSqliteIntV  *col_types;
  gchar              *table;
  gint                n_columns;

  sqlite3_stmt       *statements[N_SQL_STATEMENTS];
  sqlite3_stmt      **update_statements;

  gboolean            complete;
  sqlite3_stmt       *statement;
  guint               version;
  GPtrArray          *rowids;
  GHashTable         *rowid_to_row;
  
  gboolean            skip_add;
  gboolean            skip_change;
  gboolean            skip_remove;
};

/* Retries are every half a second */
#define META_MAX_TRIES 30

/* In case another process/thread is using this db, deal with locking */
#define DB_RETRY_TIME 2000
#define DB_RETRY_WAIT 0

static ClutterModelIter *
clutter_sqlite_model_iter_new (ClutterSqliteModel *db,
                               gint                row);

static ClutterModelIter *
clutter_sqlite_model_iter_new_from_rowid (ClutterSqliteModel *db,
                                          gint                rowid);


ClutterSqliteIntV *
clutter_sqlite_intv_copy (const ClutterSqliteIntV *intv)
{
  ClutterSqliteIntV *copy;
  
  copy = g_memdup (intv, sizeof (ClutterSqliteIntV));
  copy->data = g_memdup (intv->data, sizeof (gint) * copy->length);
  
  return copy;
}

void
clutter_sqlite_intv_free (ClutterSqliteIntV *intv)
{
  g_free (intv->data);
  g_free (intv);
}

GType
clutter_sqlite_intv_get_type (void)
{
  static GType our_type = 0;
  
  if (!our_type)
    our_type = g_boxed_type_register_static ("ClutterSqliteIntV",
                                             (GBoxedCopyFunc) clutter_sqlite_intv_copy,
                                             (GBoxedFreeFunc) clutter_sqlite_intv_free);
  
  return our_type;
}

static void
reset_statement (ClutterSqliteModel *model)
{
  ClutterSqliteModelPrivate *priv  = model->priv;

  priv->version ++;
  priv->complete = FALSE;
  if (priv->rowids->len > 0)
    g_ptr_array_remove_range (priv->rowids, 0, priv->rowids->len);
  g_hash_table_remove_all (priv->rowid_to_row);
}

static void
clutter_sqlite_model_get_property (GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  ClutterSqliteModelPrivate *priv = CLUTTER_SQLITE_MODEL (object)->priv;
  
  switch (property_id)
    {
    case PROP_DB:
      g_value_set_pointer (value, priv->db);
      break;
    case PROP_TABLE:
      g_value_set_string (value, priv->table);
      break;
    case PROP_COL_NAMES:
      g_value_set_boxed (value, priv->col_names);
      break;
    case PROP_COL_TYPES:
      g_value_set_boxed (value, priv->col_types);
      break;
    case PROP_STATEMENT:
      g_value_set_pointer (value, priv->statement);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
clutter_sqlite_model_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  ClutterSqliteModelPrivate *priv = CLUTTER_SQLITE_MODEL (object)->priv;

  switch (property_id)
    {
    case PROP_DB:
      priv->db = g_value_get_pointer (value);
      break;
    case PROP_TABLE:
      if (priv->table)
        g_free (priv->table);
      priv->table = g_value_dup_string (value);
      break;
    case PROP_COL_NAMES:
      if (priv->col_names)
        g_strfreev (priv->col_names);
      priv->col_names = g_value_dup_boxed (value);
      break;
    case PROP_COL_TYPES:
      if (priv->col_types)
        clutter_sqlite_intv_free (priv->col_types);
      priv->col_types = g_value_dup_boxed (value);
      break;
    case PROP_STATEMENT:
      reset_statement (CLUTTER_SQLITE_MODEL (object));
      if (priv->statement)
        sqlite3_reset (priv->statement);
      priv->statement = g_value_get_pointer (value);
      g_signal_emit_by_name (object, "sort-changed");
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
clutter_sqlite_model_dispose (GObject *object)
{
  G_OBJECT_CLASS (clutter_sqlite_model_parent_class)->dispose (object);
}

static void
clutter_sqlite_model_finalize (GObject *object)
{
  gint i;
  
  ClutterSqliteModelPrivate *priv = CLUTTER_SQLITE_MODEL (object)->priv;

  /* Remove db change notification */
  sqlite3_update_hook (priv->db, NULL, NULL);

  g_strfreev (priv->col_names);
  clutter_sqlite_intv_free (priv->col_types);
  g_ptr_array_free (priv->rowids, TRUE);
  g_hash_table_destroy (priv->rowid_to_row);

  /* Finalize statements */
  for (i = 0; i < N_SQL_STATEMENTS; i++)
    if (priv->statements[i])
      sqlite3_finalize (priv->statements[i]);
  
  for (i = 0; priv->update_statements[i]; i++)
    sqlite3_finalize (priv->update_statements[i]);
  g_free (priv->update_statements);
  
  G_OBJECT_CLASS (clutter_sqlite_model_parent_class)->finalize (object);
}

static glong
time_val_diff (GTimeVal *val1, GTimeVal *val2)
{
  glong diff;
  
  diff = (val2->tv_sec - val1->tv_sec) * 1000;
  diff += (val2->tv_usec - val1->tv_usec) / 1000;
  
  return diff;
}

static int
sqlite3_step_retry (sqlite3_stmt *stmt)
{
  GTimeVal val1, val2;
  int result = SQLITE_BUSY;
  
  g_assert (stmt);
  
  g_get_current_time (&val1);
  
  while (result == SQLITE_BUSY) {
    if ((result = sqlite3_step (stmt)) != SQLITE_BUSY)
      break;
    
    g_get_current_time (&val2);
    
    if (time_val_diff (&val1, &val2) >= DB_RETRY_TIME)
      break;
  }
  
  if (result == SQLITE_BUSY)
    g_warning ("Database busy, could not execute query");
  
  return result;
}

static gboolean
statement_next (ClutterSqliteModel *model,
                gboolean            complete,
                gint                find_rowid,
                gint                stop_on_row)
{
  ClutterSqliteModelPrivate *priv = model->priv;
  gboolean                   last = FALSE;
  
  if (!priv->statement)
    return TRUE;
  
  priv->version ++;
  
  if (priv->rowids->len == 0)
    sqlite3_reset (priv->statement);
  
  do
    {
      int result = sqlite3_step_retry (priv->statement);
      gint rowid = sqlite3_column_int (priv->statement, priv->n_columns);
      
      if (result == SQLITE_ROW)
        {
          g_hash_table_insert (priv->rowid_to_row,
                               GINT_TO_POINTER (rowid),
                               GINT_TO_POINTER (priv->rowids->len) + 1);
          g_ptr_array_add (priv->rowids, GINT_TO_POINTER (rowid));
          
          if (rowid == find_rowid)
            break;
        }
      else if ((result == SQLITE_DONE) || (result == SQLITE_OK))
        {
          sqlite3_reset (priv->statement);
          priv->complete = TRUE;
          last = TRUE;
          break;
        }
      else
        {
          g_warning ("Error while stepping main statement: %s",
                     sqlite3_errmsg (priv->db));
          break;
        }
      
      if ((priv->rowids->len - 1) == stop_on_row)
        break;
    } while (complete);
  
  return last;
}

static void
clutter_sqlite_update_hook (void          *user_data,
                            int            type,
                            const char    *db_name,
                            const char    *table,
                            sqlite_int64   rowid)
{
  ClutterModelIter          *iter;
  ClutterSqliteModel        *model = user_data;
  ClutterSqliteModelPrivate *priv  = model->priv;
  gboolean                   skip = FALSE;
  
  if (strcmp (priv->table, table) != 0)
    return;
  
  /* We need to be able to skip row additions/changes as ClutterModel emits 
   * these signals itself, where as we want to emit them for all additions/
   * changes using sqlite hooks.
   */
  if ((type == SQLITE_INSERT) && (priv->skip_add))
    {
      skip = TRUE;
      priv->skip_add = FALSE;
    }
  else if ((type == SQLITE_UPDATE) && (priv->skip_change))
    {
      skip = TRUE;
      priv->skip_change = FALSE;
    }
  else if ((type == SQLITE_DELETE) && (priv->skip_remove))
    {
      skip = TRUE;
      priv->skip_remove = FALSE;
    }
  
  if (!skip)
    {
      switch (type)
        {
        case SQLITE_INSERT:
          iter = clutter_sqlite_model_iter_new_from_rowid (model, rowid);
          g_signal_emit_by_name (model, "row-added", iter);
          if (iter)
            g_object_unref (iter);
          break;
        case SQLITE_DELETE:
          g_signal_emit_by_name (model, "row-removed", NULL);
          break;
        case SQLITE_UPDATE:
          iter = clutter_sqlite_model_iter_new_from_rowid (model, rowid);
          g_signal_emit_by_name (model, "row-changed", iter);
          if (iter)
            g_object_unref (iter);
          break;
        }
    }
  
  /* Reset our index, it may not be valid anymore */
  reset_statement (model);
}

static guint
clutter_sqlite_model_get_n_rows (ClutterModel *model)
{
  ClutterSqliteModel        *sqlite_model = CLUTTER_SQLITE_MODEL (model);
  ClutterSqliteModelPrivate *priv         = sqlite_model->priv;
  
  if (!priv->complete)
    statement_next (sqlite_model, TRUE, -1, -1);
  
  return priv->rowids->len;
}

static guint
clutter_sqlite_model_get_n_columns (ClutterModel *model)
{
  ClutterSqliteModelPrivate *priv = CLUTTER_SQLITE_MODEL (model)->priv;
  return priv->n_columns;
}

static const gchar *
clutter_sqlite_model_get_column_name (ClutterModel *model, guint column)
{
  ClutterSqliteModelPrivate *priv = CLUTTER_SQLITE_MODEL (model)->priv;
  return priv->col_names[column];
}

static GType
clutter_sqlite_model_get_column_type (ClutterModel *model, guint column)
{
  ClutterSqliteModelPrivate *priv = CLUTTER_SQLITE_MODEL (model)->priv;

  switch (priv->col_types->data[column])
    {
      case SQLITE_INTEGER :
        return G_TYPE_INT;
      case SQLITE_FLOAT :
        return G_TYPE_DOUBLE;
      case SQLITE_BLOB :
      case SQLITE_TEXT :
        return G_TYPE_STRING;
      case SQLITE_NULL :
      default :
        return G_TYPE_INVALID;
    }
}

static ClutterModelIter *
clutter_sqlite_model_insert_row (ClutterModel *model, gint index)
{
  /* Note: This ignores index and just 'appends' to the table */
  gint                       result;
  ClutterSqliteModel        *sqlite_model = CLUTTER_SQLITE_MODEL (model);
  ClutterSqliteModelPrivate *priv         = sqlite_model->priv;

  /* Cancel the current iteration through the db,
   * we'll be reset on add anyway */
  if (!priv->complete && priv->rowids->len)
    sqlite3_reset (priv->statement);

  /* Skip the add hook, ClutterModel generates the row-added signal */
  priv->skip_add = TRUE;
  result = sqlite3_step_retry (priv->statements[SQL_ADD_ROW]);
  sqlite3_reset (priv->statements[SQL_ADD_ROW]);

  if (result == SQLITE_DONE)
    {
      gint              rowid = sqlite3_last_insert_rowid (priv->db);
      ClutterModelIter *iter  =
        clutter_sqlite_model_iter_new_from_rowid (sqlite_model, rowid);
      if (iter)
        return iter;
      else
        g_warning ("Failed to get iter for newly created row, "
                   "probably about to assert.");
    }
  else
    g_warning ("Failed to create row, probably about to assert: %s",
               sqlite3_errmsg (priv->db));
  
  return NULL;
}

static void
clutter_sqlite_model_remove_row (ClutterModel *model, guint row)
{
  ClutterSqliteModel        *sqlite_model = CLUTTER_SQLITE_MODEL (model);
  ClutterSqliteModelPrivate *priv         = sqlite_model->priv;
  ClutterModelIter          *iter;

  /* Fire off 'removed' signal. We do this here, so at least for rows 
   * removed through ClutterModel, we can pass a valid iter.
   */
  iter = clutter_sqlite_model_iter_new (sqlite_model, row);
  if (iter)
    {
      priv->skip_remove = TRUE;
      g_signal_emit_by_name (model, "row-removed", iter);
      g_object_unref (iter);
    }

  if (!priv->complete && priv->rowids->len)
    sqlite3_reset (priv->statement);

  sqlite3_bind_int (priv->statements[SQL_DELETE_ROW],
                    1,
                    GPOINTER_TO_INT (priv->rowids->pdata[row]));
  sqlite3_step_retry (priv->statements[SQL_DELETE_ROW]);
  sqlite3_reset (priv->statements[SQL_DELETE_ROW]);
}

static ClutterModelIter *
clutter_sqlite_model_get_iter_at_row (ClutterModel *model, guint row)
{
  return clutter_sqlite_model_iter_new (CLUTTER_SQLITE_MODEL (model), row);
}

static void
clutter_sqlite_model_resort (ClutterModel         *model,
                             ClutterModelSortFunc  func,
                             gpointer              data)
{
}

static GObject *
clutter_sqlite_model_constructor (GType                  type,
                                  guint                  n_properties,
                                  GObjectConstructParam *properties)
{
  GObjectClass              *gobject_class;
  GObject                   *obj;
  ClutterSqliteModelPrivate *priv;
  gint                       i;
  
  gobject_class = G_OBJECT_CLASS (clutter_sqlite_model_parent_class);
  obj = gobject_class->constructor (type, n_properties, properties);
  priv = CLUTTER_SQLITE_MODEL (obj)->priv;
  
  /* Set the busy-timeout */
  sqlite3_busy_timeout (priv->db, DB_RETRY_WAIT);
  
  /* Generate and precompile statements */
  for (i = 0; i < N_SQL_STATEMENTS; i++)
    {
      gchar *text =
        g_strdup_printf (sql_statements[i], priv->table, priv->col_names[0]);
      
      if (sqlite3_prepare (priv->db,
                           text,
                           -1, 
                           &priv->statements[i],
                           NULL) != SQLITE_OK)
        g_warning ("Failed to prepare '%s': %s", 
                   text,
                   sqlite3_errmsg (priv->db));
      
      g_free (text);
    }
  
  priv->n_columns = priv->col_types->length;
  
  priv->update_statements =
    g_malloc0 (sizeof (sqlite3_stmt *) * priv->n_columns);
  for (i = 0; i < priv->n_columns; i++)
    {
      gchar *text = g_strdup_printf (sql_update_statement,
                                     priv->table,
                                     priv->col_names[i]);
      if (sqlite3_prepare (priv->db,
                           text,
                           -1,
                           &priv->update_statements[i],
                           NULL) != SQLITE_OK)
        g_warning ("Failed to prepare '%s' : %s", 
                   text,
                   sqlite3_errmsg (priv->db));
      
      g_free (text);
    }
  
  /* Hook onto data change notification */
  sqlite3_update_hook (priv->db, clutter_sqlite_update_hook, obj);
  
  return obj;
}

static void
clutter_sqlite_model_class_init (ClutterSqliteModelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterModelClass *model_class = CLUTTER_MODEL_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ClutterSqliteModelPrivate));

  object_class->constructor  = clutter_sqlite_model_constructor;
  object_class->get_property = clutter_sqlite_model_get_property;
  object_class->set_property = clutter_sqlite_model_set_property;
  object_class->dispose      = clutter_sqlite_model_dispose;
  object_class->finalize     = clutter_sqlite_model_finalize;
  
  model_class->get_n_rows      = clutter_sqlite_model_get_n_rows;
  model_class->get_n_columns   = clutter_sqlite_model_get_n_columns;
  model_class->get_column_name = clutter_sqlite_model_get_column_name;
  model_class->get_column_type = clutter_sqlite_model_get_column_type;
  model_class->insert_row      = clutter_sqlite_model_insert_row;
  model_class->remove_row      = clutter_sqlite_model_remove_row;
  model_class->get_iter_at_row = clutter_sqlite_model_get_iter_at_row;
  model_class->resort          = clutter_sqlite_model_resort;

  g_object_class_install_property (object_class,
                                   PROP_DB,
                                   g_param_spec_pointer ("db",
                                                         "Database",
                                                         "Sqlite3 database "
                                                         "pointer",
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class,
                                   PROP_TABLE,
                                   g_param_spec_string ("table",
                                                        "Table name",
                                                        "Sqlite3 table name",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class,
                                   PROP_COL_NAMES,
                                   g_param_spec_boxed ("col-names",
                                                       "Column names",
                                                       "Sqlite3 column names",
                                                       G_TYPE_STRV,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class,
                                   PROP_COL_TYPES,
                                   g_param_spec_boxed ("col-types",
                                                       "Column types",
                                                       "Sqlite3 column types",
                                                       CLUTTER_SQLITE_TYPE_INTV,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class,
                                   PROP_STATEMENT,
                                   g_param_spec_pointer ("statement",
                                                         "Statement",
                                                         "Sqlite3 statement "
                                                         "pointer",
                                                         G_PARAM_READWRITE));
}

static void
clutter_sqlite_model_init (ClutterSqliteModel *self)
{
  ClutterSqliteModelPrivate *priv = self->priv = MODEL_PRIVATE (self);
  
  priv->rowids = g_ptr_array_new ();
  priv->rowid_to_row = g_hash_table_new (NULL, NULL);
}

ClutterModel *
clutter_sqlite_model_new (sqlite3 *db, const gchar *table, ...)
{
  ClutterModel      *model;
  gint               n_columns;
  const gchar       *name;
  GStrv              name_array;
  ClutterSqliteIntV *type_array;
  GList             *names = NULL;
  GList             *types = NULL;
  
  va_list args;
  
  va_start (args, table);
  
  n_columns = 0;
  name = va_arg (args, const gchar *);
  for (; name; name = va_arg (args, const gchar *))
    {
      names = g_list_prepend (names, g_strdup (name));
      types = g_list_prepend (types, GINT_TO_POINTER (va_arg (args, gint)));
      
      n_columns ++;
    }
  
  va_end (args);
  
  name_array = g_malloc0 (sizeof (gchar *) * (n_columns + 1));

  type_array = g_new (ClutterSqliteIntV, 1);
  type_array->length = n_columns;
  type_array->data = g_malloc (sizeof (gint) * n_columns);
  
  while (names)
    {
      n_columns--;
      name_array[n_columns] = names->data;
      type_array->data[n_columns] = GPOINTER_TO_INT (types->data);
      
      names = g_list_delete_link (names, names);
      types = g_list_delete_link (types, types);
    }
  
  model = CLUTTER_MODEL (g_object_new (CLUTTER_SQLITE_TYPE_MODEL,
                                       "db", db,
                                       "table", table,
                                       "col-names", name_array,
                                       "col-types", type_array,
                                       NULL));
  
  g_strfreev (name_array);
  clutter_sqlite_intv_free (type_array);
  
  return model;
}

static void
clutter_sqlite_model_iter_get_value (ClutterModelIter *iter,
                                     guint             column,
                                     GValue           *value)
{
  sqlite3_stmt *statement;
  GType         column_type;
  gboolean      converted    = FALSE;
  GValue        real_value   = { 0, };
  GValue        column_value = { 0, };

  ClutterModel              *model   = clutter_model_iter_get_model (iter);
  ClutterSqliteModelPrivate *priv    = CLUTTER_SQLITE_MODEL (model)->priv;
  ClutterSqliteModelIter    *sqliter = CLUTTER_SQLITE_MODEL_ITER (iter);
  
  if (!priv->statement)
    return;

  column_type = clutter_sqlite_model_get_column_type (model, column);
  if (column_type == G_TYPE_INVALID)
    return;
  
  if (sqliter->is_parent == priv->version)
    statement = priv->statement;
  else
    {
      gint rowid = (sqliter->row == -1) ?
        sqliter->rowid : GPOINTER_TO_INT (priv->rowids->pdata[sqliter->row]);
      sqlite3_bind_int (priv->statements[SQL_GET_ROW], 1, rowid);
      if (sqlite3_step_retry (priv->statements[SQL_GET_ROW]) != SQLITE_ROW)
        {
          g_warning ("Error getting row: %s", sqlite3_errmsg (priv->db));
          sqlite3_reset (priv->statements[SQL_GET_ROW]);
          return;
        }
      
      statement = priv->statements[SQL_GET_ROW];
    }
  
  g_value_init (&column_value, column_type);
  switch (column_type)
    {
      case G_TYPE_STRING :
        if (priv->col_types->data[column] == SQLITE_TEXT)
          g_value_set_string (&column_value, (const gchar *)
                              sqlite3_column_text (statement, column));
        break;
      case G_TYPE_INT :
        g_value_set_int (&column_value,
                         sqlite3_column_int (statement, column));
        break;
      case G_TYPE_BOOLEAN :
        g_value_set_boolean (&column_value,
                             sqlite3_column_int (statement, column));
        break;
      default :
        g_value_unset (&column_value);
        if (sqliter->is_parent != priv->version)
          sqlite3_reset (priv->statements[SQL_GET_ROW]);
        return;
    }
  
  if (sqliter->is_parent != priv->version)
    sqlite3_reset (priv->statements[SQL_GET_ROW]);

  if (!g_type_is_a (G_VALUE_TYPE (value), column_type))
    {
      if (!g_value_type_compatible (G_VALUE_TYPE (value), column_type) &&
          !g_value_type_compatible (column_type, G_VALUE_TYPE (value)))
        {
          g_warning ("%s: Unable to convert from %s to %s",
                     G_STRLOC,
                     g_type_name (G_VALUE_TYPE (value)),
                     g_type_name (column_type));
          return;
        }
      
      if (!g_value_transform (&column_value, &real_value))
        {
          g_warning ("%s: Unable to make conversion from %s to %s",
                     G_STRLOC, 
                     g_type_name (column_type),
                     g_type_name (G_VALUE_TYPE (value)));
          g_value_unset (&real_value);
        }
      
      converted = TRUE;
    }
    
  if (converted)
    {
      g_value_copy (&real_value, value);
      g_value_unset (&real_value);
    }
  else
    g_value_copy (&column_value, value);
  
  g_value_unset (&column_value);
}

static void
clutter_sqlite_model_iter_set_value (ClutterModelIter *iter,
                                     guint             column,
                                     const GValue     *value)
{
  gint         res, rowid;
  GType        column_type;
  gboolean     converted  = FALSE;
  GValue       real_value = { 0, };
    
  ClutterModel              *model        = clutter_model_iter_get_model (iter);
  ClutterSqliteModel        *sqlite_model = CLUTTER_SQLITE_MODEL (model);
  ClutterSqliteModelPrivate *priv         = sqlite_model->priv;
  ClutterSqliteModelIter    *sqliter      = CLUTTER_SQLITE_MODEL_ITER (iter);

  if (!priv->statement)
    return;
  
  column_type = clutter_sqlite_model_get_column_type (model, column);
  if (column_type == G_TYPE_INVALID)
    return;

  g_value_init (&real_value, column_type);
  
  if (!g_type_is_a (G_VALUE_TYPE (value), column_type))
    {
      if (!g_value_type_compatible (G_VALUE_TYPE (value), column_type) &&
          !g_value_type_compatible (column_type, G_VALUE_TYPE (value)))
        {
          g_warning ("%s: Unable to convert from %s to %s",
                     G_STRLOC,
                     g_type_name (G_VALUE_TYPE (value)),
                     g_type_name (column_type));
          return;
        }
      
      if (!g_value_transform (value, &real_value))
        {
          g_warning ("%s: Unable to make conversion from %s to %s",
                     G_STRLOC, 
                     g_type_name (G_VALUE_TYPE (value)),
                     g_type_name (column_type));
          g_value_unset (&real_value);
        }
      
      converted = TRUE;
    }
  
  if (!converted)
    g_value_copy (value, &real_value);
  
  if (!priv->complete && priv->rowids->len)
    statement_next (sqlite_model, TRUE, -1, -1);
  
  switch (column_type)
    {
      case G_TYPE_STRING :
        sqlite3_bind_text (priv->update_statements[column],
                           1, g_value_get_string (&real_value), -1,
                           SQLITE_TRANSIENT);
        break;
      case G_TYPE_INT :
        sqlite3_bind_int (priv->update_statements[column],
                          1, g_value_get_int (&real_value));
        break;
      case G_TYPE_BOOLEAN :
        sqlite3_bind_int (priv->update_statements[column],
                          1, g_value_get_boolean (&real_value));
        break;
      case G_TYPE_ENUM :
        sqlite3_bind_int (priv->update_statements[column],
                          1, g_value_get_enum (&real_value));
        break;
      case G_TYPE_OBJECT :
        /* TODO: Let's think about this later */
      default :
        goto _iter_set_value_skip_write;
    }
  
  rowid = (sqliter->row == -1) ?
    sqliter->rowid : GPOINTER_TO_INT (priv->rowids->pdata[sqliter->row]);
  sqlite3_bind_int (priv->update_statements[column], 2, rowid);
  priv->skip_change = TRUE;
  sqlite3_step_retry (priv->update_statements[column]);
  res = sqlite3_reset (priv->update_statements[column]);

  if (res != SQLITE_OK)
    g_warning ("Unable to write to db (%d): %s",
               res,
               sqlite3_errmsg (priv->db));

_iter_set_value_skip_write:
  
  g_value_unset (&real_value);
}

static gboolean
clutter_sqlite_model_iter_is_first (ClutterModelIter *iter)
{
  ClutterSqliteModelIter *sqliter = CLUTTER_SQLITE_MODEL_ITER (iter);
  return (sqliter->row == 0) ? TRUE : FALSE;
}

static gboolean
clutter_sqlite_model_iter_is_last (ClutterModelIter *iter)
{
  ClutterSqliteModelIter *sqliter = CLUTTER_SQLITE_MODEL_ITER (iter);
  return sqliter->is_last;
}

static ClutterModelIter *
clutter_sqlite_model_iter_new (ClutterSqliteModel *model,
                               gint                row)
{
  ClutterSqliteModelIter    *iter;
  ClutterSqliteModelPrivate *priv = model->priv;
  
  if (!priv->statement)
    return NULL;
  
  if (row && (row > (gint)priv->rowids->len))
    {
      if (!priv->complete)
        statement_next (model, TRUE, -1, row);
      
      if (row > priv->rowids->len)
        return NULL;
    }
  
  iter = g_object_new (CLUTTER_SQLITE_TYPE_MODEL_ITER,
                       "model", model,
                       "row", row,
                       NULL);
  iter->row = row;
  iter->is_last = (row >= priv->rowids->len) ? TRUE : FALSE;
  
  if ((row == priv->rowids->len) && (!priv->complete))
    {
      iter->is_last = statement_next (model, FALSE, -1, -1);
      iter->is_parent = priv->version;
    }
  
  return CLUTTER_MODEL_ITER (iter);
}

static ClutterModelIter *
clutter_sqlite_model_iter_new_from_rowid (ClutterSqliteModel *model,
                                          gint                rowid)
{
  ClutterSqliteModelPrivate *priv = model->priv;
  ClutterModelIter          *iter;
  ClutterSqliteModelIter    *sqlite_iter;
  
  if (!priv->statement)
    return NULL;
  
  iter = clutter_sqlite_model_iter_new (model, -1);
  sqlite_iter = CLUTTER_SQLITE_MODEL_ITER (iter);
  sqlite_iter->rowid = rowid;
  
  return iter;
}

static ClutterModelIter *
clutter_sqlite_model_iter_next (ClutterModelIter *iter)
{
  ClutterModelIter          *new_iter;
  ClutterSqliteModelIter    *sqliter = CLUTTER_SQLITE_MODEL_ITER (iter);
  ClutterSqliteModel        *model;
  ClutterSqliteModelPrivate *priv;
  
  if (sqliter->is_last)
    return NULL;
  
  model = CLUTTER_SQLITE_MODEL (clutter_model_iter_get_model (iter));
  priv = model->priv;

  /* If we don't yet have a row set, try to get one */
  if (sqliter->row < 0)
    {
      sqliter->row = GPOINTER_TO_INT (
        g_hash_table_lookup (priv->rowid_to_row,
                             GINT_TO_POINTER (sqliter->rowid)));

      if (!priv->complete && !sqliter->row)
        {
          statement_next (model, TRUE, sqliter->rowid, -1);
          sqliter->row = GPOINTER_TO_INT (
            g_hash_table_lookup (priv->rowid_to_row,
                                 GINT_TO_POINTER (sqliter->rowid)));
        }
      
      if (!sqliter->row)
        {
          sqliter->row = -1;
          return NULL;
        }
    }
  
  new_iter = clutter_sqlite_model_iter_new (model, sqliter->row + 1);
  
  if (sqliter->is_parent == priv->version)
    {
      ClutterSqliteModelIter *sqlite_iter =
        CLUTTER_SQLITE_MODEL_ITER (new_iter);
      statement_next (model, FALSE, -1, -1);
      sqlite_iter->is_parent = priv->version;
    }

  g_object_unref (sqliter);

  return new_iter;
}

static ClutterModelIter *
clutter_sqlite_model_iter_prev (ClutterModelIter *iter)
{
  ClutterModelIter          *new_iter;
  ClutterSqliteModelIter    *sqliter = CLUTTER_SQLITE_MODEL_ITER (iter);
  ClutterSqliteModel        *model;
  
  if (sqliter->row == 0)
    return NULL;
  
  model = CLUTTER_SQLITE_MODEL (clutter_model_iter_get_model (iter));
  new_iter = clutter_sqlite_model_iter_new (model, sqliter->row - 1);

  g_object_unref (sqliter);
  
  return new_iter;
}

static void
clutter_sqlite_model_iter_class_init (ClutterSqliteModelIterClass *klass)
{
  ClutterModelIterClass *iter_class = CLUTTER_MODEL_ITER_CLASS (klass);
  
  iter_class->get_value = clutter_sqlite_model_iter_get_value;
  iter_class->set_value = clutter_sqlite_model_iter_set_value;
  iter_class->is_first  = clutter_sqlite_model_iter_is_first;
  iter_class->is_last   = clutter_sqlite_model_iter_is_last;
  iter_class->next      = clutter_sqlite_model_iter_next;
  iter_class->prev      = clutter_sqlite_model_iter_prev;
}

static void
clutter_sqlite_model_iter_init (ClutterSqliteModelIter *iter)
{
}

