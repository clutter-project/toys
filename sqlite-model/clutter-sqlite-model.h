
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

#ifndef _CLUTTER_SQLITE_MODEL
#define _CLUTTER_SQLITE_MODEL

#include <glib-object.h>
#include <sqlite3.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define CLUTTER_SQLITE_TYPE_INTV  (clutter_sqlite_intv_get_type())

typedef struct _ClutterSqliteIntV ClutterSqliteIntV;

struct _ClutterSqliteIntV
{
  guint  length;
  gint  *data;
};

ClutterSqliteIntV *clutter_sqlite_intv_copy (const ClutterSqliteIntV *intv);
void               clutter_sqlite_intv_free (ClutterSqliteIntV       *intv);


#define CLUTTER_SQLITE_TYPE_MODEL (clutter_sqlite_model_get_type())

#define CLUTTER_SQLITE_MODEL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  CLUTTER_SQLITE_TYPE_MODEL, ClutterSqliteModel))

#define CLUTTER_SQLITE_MODEL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  CLUTTER_SQLITE_TYPE_MODEL, ClutterSqliteModelClass))

#define CLUTTER_SQLITE_IS_MODEL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  CLUTTER_SQLITE_TYPE_MODEL))

#define CLUTTER_SQLITE_IS_MODEL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  CLUTTER_SQLITE_TYPE_MODEL))

#define CLUTTER_SQLITE_MODEL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  CLUTTER_SQLITE_TYPE_MODEL, ClutterSqliteModelClass))

typedef struct _ClutterSqliteModel        ClutterSqliteModel;
typedef struct _ClutterSqliteModelPrivate ClutterSqliteModelPrivate;
typedef struct _ClutterSqliteModelClass   ClutterSqliteModelClass;

struct _ClutterSqliteModel {
  ClutterModel               parent;
  
  ClutterSqliteModelPrivate *priv;
};

struct _ClutterSqliteModelClass {
  ClutterModelClass parent_class;
};

GType clutter_sqlite_model_get_type (void);

ClutterModel *clutter_sqlite_model_new (sqlite3 *db, const gchar *table, ...);

G_END_DECLS

#endif

