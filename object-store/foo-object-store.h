/*
 * Copyright (c) 2010, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Author: Rob Staudinger <robsta@linux.intel.com>
 */

#ifndef FOO_OBJECT_STORE_H
#define FOO_OBJECT_STORE_H

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define FOO_TYPE_OBJECT_STORE foo_object_store_get_type()

#define FOO_OBJECT_STORE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  FOO_TYPE_OBJECT_STORE, FooObjectStore))

#define FOO_OBJECT_STORE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  FOO_TYPE_OBJECT_STORE, FooObjectStoreClass))

#define FOO_IS_OBJECT_STORE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  FOO_TYPE_OBJECT_STORE))

#define FOO_IS_OBJECT_STORE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  FOO_TYPE_OBJECT_STORE))

#define FOO_OBJECT_STORE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  FOO_TYPE_OBJECT_STORE, FooObjectStoreClass))

typedef struct FooObjectStore_        FooObjectStore;
typedef struct FooObjectStoreClass_   FooObjectStoreClass;

struct FooObjectStore_ {
  ClutterModel parent;
};

struct FooObjectStoreClass_ {
  ClutterModelClass parent;
};

GType foo_object_store_get_type (void) G_GNUC_CONST;

ClutterModel * foo_object_store_new (guint n_columns,
                                     ...);

void foo_object_store_foreach_unfiltered (FooObjectStore          *self,
                                          ClutterModelForeachFunc  func,
                                          gpointer                 user_data);

gint foo_object_store_remove (FooObjectStore  *self,
                              GObject         *object);

G_END_DECLS

#endif /* FOO_OBJECT_STORE_H */
