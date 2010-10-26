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
 * Based on clutter-list-model.c
 *    Copyright (C) 2006 OpenedHand, by
 *    Matthew Allum  <mallum@openedhand.com>
 *    Neil Jagdish Patel <njp@o-hand.com>
 *    Emmanuele Bassi <ebassi@openedhand.com>
 */

/*
  README

  This is a ClutterModel subclass to proxy GObjects, instead of holding the
  data itself, like ClutterListModel. The model has to be constructed so that
  the object is held by column #0. All other colums can be mapped to the
  objects' properties in any desired order.
*/

#include "foo-object-store.h"

typedef struct FooObjectStoreIter_ FooObjectStoreIter;

static void
foo_object_store_object_property_notify (GObject         *object,
                                         GParamSpec      *pspec,
                                         FooObjectStore  *self);

static void
foo_object_store_detach_object          (FooObjectStore  *self,
                                         GObject         *object);

static void
foo_object_store_attach_object          (FooObjectStore  *self,
                                         GObject         *object);

/*
 * FooObjectStore declaration.
 */

G_DEFINE_TYPE (FooObjectStore, foo_object_store, CLUTTER_TYPE_MODEL)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), FOO_TYPE_OBJECT_STORE, FooObjectStorePrivate))

typedef struct
{
  GSequence           *sequence;
  FooObjectStoreIter  *cached_iter;
} FooObjectStorePrivate;

/*
 * FooObjectStoreIter.
 */

#define FOO_TYPE_OBJECT_STORE_ITER foo_object_store_iter_get_type()

#define FOO_OBJECT_STORE_ITER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  FOO_TYPE_OBJECT_STORE_ITER, FooObjectStoreIter))

#define FOO_OBJECT_STORE_ITER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  FOO_TYPE_OBJECT_STORE_ITER, FooObjectStoreIterClass))

#define FOO_IS_OBJECT_STORE_ITER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  FOO_TYPE_OBJECT_STORE_ITER))

#define FOO_IS_OBJECT_STORE_ITER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  FOO_TYPE_OBJECT_STORE_ITER))

#define FOO_OBJECT_STORE_ITER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  FOO_TYPE_OBJECT_STORE_ITER, FooObjectStoreIterClass))

struct FooObjectStoreIter_ {
  ClutterModelIter   parent;
  GSequenceIter     *seq_iter;  /* NULL means it's the end iter. */
};

typedef struct {
  ClutterModelIterClass parent;
} FooObjectStoreIterClass;

GType foo_object_store_iter_get_type (void) G_GNUC_CONST;

G_DEFINE_TYPE (FooObjectStoreIter, foo_object_store_iter, CLUTTER_TYPE_MODEL_ITER)

static void
foo_object_store_iter_get_value (ClutterModelIter *iter,
                                 guint             column,
                                 GValue           *value)
{
  FooObjectStoreIter  *self;
  GObject             *object;
  const gchar         *name;

  g_return_if_fail (FOO_IS_OBJECT_STORE_ITER (iter));
  g_return_if_fail (value);

  self = FOO_OBJECT_STORE_ITER (iter);
  if (!self->seq_iter)
    return;
  object = g_sequence_get (self->seq_iter);

  if (column == 0)
    {
      g_value_set_object (value, object);
    }
  else
    {
      name = clutter_model_get_column_name (clutter_model_iter_get_model (iter),
                                            column);
      g_object_get_property (object, name, value);
    }
}

static void
foo_object_store_iter_set_value (ClutterModelIter *iter,
                                 guint             column,
                                 const GValue     *value)
{
  FooObjectStoreIter  *self;
  ClutterModel        *model;
  GObject             *object;

  g_return_if_fail (FOO_IS_OBJECT_STORE_ITER (iter));
  g_return_if_fail (value);

  self = FOO_OBJECT_STORE_ITER (iter);
  model = clutter_model_iter_get_model (iter);
  object = g_sequence_get (self->seq_iter);

  if (column == 0)
    {
      /* Set "master" column. NULL is legal since append() does an empty row. */
      if (object)
        {
          foo_object_store_detach_object (FOO_OBJECT_STORE (model), object);
          g_object_unref (object);
        }

      object = g_value_get_object (value);
      g_sequence_set (self->seq_iter, g_object_ref (object));

      /* Hook up "changed" notifications for the object's properties. */
      foo_object_store_attach_object (FOO_OBJECT_STORE (model), object);
    }
  else if (object)
    {
      const gchar *name = clutter_model_get_column_name (model, column);
      g_object_set_property (object, name, value);
    }
  else
    {
      g_warning ("%s Cannot set column %d on NULL object",
                 G_STRLOC,
                 column);
    }
}

static gboolean
foo_object_store_iter_is_first (ClutterModelIter *iter)
{
  FooObjectStoreIter    *self;
  ClutterModel          *store;
  FooObjectStorePrivate *priv;
  FooObjectStoreIter    *cached_iter;
  gboolean               is_seq_first;
  gboolean               is_first = TRUE;

  g_return_val_if_fail (FOO_IS_OBJECT_STORE_ITER (iter), FALSE);

  self = FOO_OBJECT_STORE_ITER (iter);
  store = clutter_model_iter_get_model (iter);
  priv = GET_PRIVATE (store);
  cached_iter = priv->cached_iter;

  /* Go backwards from iter looking for non-filtered rows. */
  for (is_seq_first = g_sequence_iter_is_begin (self->seq_iter),
       cached_iter->seq_iter = g_sequence_iter_prev (self->seq_iter);
       !is_seq_first;
       is_seq_first = g_sequence_iter_is_begin (cached_iter->seq_iter),
       cached_iter->seq_iter = g_sequence_iter_prev (cached_iter->seq_iter))
    {
      if (clutter_model_filter_iter (store, CLUTTER_MODEL_ITER (cached_iter)))
        {
          is_first = FALSE;
          break;
        }
    }

  return is_first;
}

static gboolean
foo_object_store_iter_is_last (ClutterModelIter *iter)
{
  g_return_val_if_fail (FOO_IS_OBJECT_STORE_ITER (iter), TRUE);
  g_return_val_if_fail (FOO_OBJECT_STORE_ITER (iter)->seq_iter, TRUE);

  return g_sequence_iter_is_end (FOO_OBJECT_STORE_ITER (iter)->seq_iter);
}

static ClutterModelIter *
foo_object_store_iter_next (ClutterModelIter *iter)
{
  FooObjectStoreIter  *self;
  ClutterModel        *store;
  guint                row;

  g_return_val_if_fail (FOO_IS_OBJECT_STORE_ITER (iter), NULL);

  self = FOO_OBJECT_STORE_ITER (iter);
  store = clutter_model_iter_get_model (iter);
  row = clutter_model_iter_get_row (iter);

  if (clutter_model_get_filter_set (store))
    {
      /* Look for next non-filtered row. */
      for (self->seq_iter = g_sequence_iter_next (self->seq_iter);
           !g_sequence_iter_is_end (self->seq_iter);
           self->seq_iter = g_sequence_iter_next (self->seq_iter))
        {
          if (clutter_model_filter_iter (store, iter))
            {
              g_object_set (iter, "row", row + 1, NULL);
              /* self->seq_iter already points to the correct row. */
              break;
            }
        }
    }
  else if (!g_sequence_iter_is_end (self->seq_iter))
    {
      g_object_set (iter, "row", row + 1, NULL);
      self->seq_iter = g_sequence_iter_next (self->seq_iter);
    }

  return iter;
}

static ClutterModelIter *
foo_object_store_iter_prev (ClutterModelIter *iter)
{
  FooObjectStoreIter  *self;
  ClutterModel        *store;
  guint                row;

  g_return_val_if_fail (FOO_IS_OBJECT_STORE_ITER (iter), NULL);

  self = FOO_OBJECT_STORE_ITER (iter);
  store = clutter_model_iter_get_model (iter);
  row = clutter_model_iter_get_row (iter);

  if (clutter_model_get_filter_set (store))
    {
      /* Look for prev non-filtered row. */
      for (self->seq_iter = g_sequence_iter_prev (self->seq_iter);
           !g_sequence_iter_is_begin (self->seq_iter);
           self->seq_iter = g_sequence_iter_next (self->seq_iter))
        {
          if (clutter_model_filter_iter (store, iter))
            {
              g_object_set (iter, "row", row - 1, NULL);
              /* self->seq_iter already points to the correct row. */
              break;
            }
        }
    }
  else if (!g_sequence_iter_is_begin (self->seq_iter))
    {
      g_object_set (iter, "row", row - 1, NULL);
      self->seq_iter = g_sequence_iter_prev (self->seq_iter);
    }

  return iter;
}

static ClutterModelIter *
foo_object_store_iter_copy (ClutterModelIter *iter)
{
  FooObjectStoreIter  *self;
  ClutterModelIter    *copy;

  g_return_val_if_fail (FOO_IS_OBJECT_STORE_ITER (iter), NULL);

  self = FOO_OBJECT_STORE_ITER (iter);
  copy = g_object_new (FOO_TYPE_OBJECT_STORE_ITER,
                       "model", clutter_model_iter_get_model (iter),
                       "row", clutter_model_iter_get_row (iter),
                       NULL);

  /* this is safe, because the seq_iter pointer on the passed
   * iterator will be always be overwritten in ::next or ::prev */
  FOO_OBJECT_STORE_ITER (copy)->seq_iter = self->seq_iter;

  return copy;
}

static void
foo_object_store_iter_class_init (FooObjectStoreIterClass *klass)
{
  ClutterModelIterClass *iter_class = CLUTTER_MODEL_ITER_CLASS (klass);

  iter_class->get_value = foo_object_store_iter_get_value;
  iter_class->set_value = foo_object_store_iter_set_value;
  iter_class->is_first  = foo_object_store_iter_is_first;
  iter_class->is_last   = foo_object_store_iter_is_last;
  iter_class->next      = foo_object_store_iter_next;
  iter_class->prev      = foo_object_store_iter_prev;
  iter_class->copy      = foo_object_store_iter_copy;
}

static void
foo_object_store_iter_init (FooObjectStoreIter *iter)
{
  iter->seq_iter = NULL;
}

/*
 * FooObjectStore.
 */

static void
foo_object_store_finalize (GObject *gobject)
{
  FooObjectStorePrivate *priv = GET_PRIVATE (gobject);

  g_sequence_free (priv->sequence);
  priv->sequence = NULL;

  G_OBJECT_CLASS (foo_object_store_parent_class)->finalize (gobject);
}

static void
_detach_if_non_null (GObject        *object,
                     FooObjectStore *self)
{
  if (G_IS_OBJECT (object))
    foo_object_store_detach_object (self, object);
}

static void
foo_object_store_dispose (GObject *gobject)
{
  FooObjectStorePrivate *priv = GET_PRIVATE (gobject);

  g_sequence_foreach_range (g_sequence_get_begin_iter (priv->sequence),
                            g_sequence_get_end_iter (priv->sequence),
                            (GFunc) _detach_if_non_null,
                            gobject);

  g_sequence_remove_range (g_sequence_get_begin_iter (priv->sequence),
                           g_sequence_get_end_iter (priv->sequence));

  if (priv->cached_iter)
    {
      g_object_unref (priv->cached_iter);
      priv->cached_iter = NULL;
    }

  G_OBJECT_CLASS (foo_object_store_parent_class)->dispose (gobject);
}

static ClutterModelIter *
foo_object_store_get_iter_at_row (ClutterModel *self,
                                  guint         row)
{
  FooObjectStorePrivate *priv;
  FooObjectStoreIter    *cached_iter;
  FooObjectStoreIter    *iter = NULL;
  gint                   r = -1;

  g_return_val_if_fail (FOO_IS_OBJECT_STORE (self), NULL);

  priv = GET_PRIVATE (self);
  cached_iter = priv->cached_iter;

  /* Work the cached iter to defer object instantiation until success. */
  if (clutter_model_get_filter_set (self))
    {
      /* Count matching rows. */
      for (cached_iter->seq_iter = g_sequence_get_begin_iter (priv->sequence);
           !g_sequence_iter_is_end (cached_iter->seq_iter);
           cached_iter->seq_iter = g_sequence_iter_next (cached_iter->seq_iter))
        {
          if (clutter_model_filter_iter (self, CLUTTER_MODEL_ITER (cached_iter)))
            {
              r++;
              if ((unsigned) r == row)
                break;
            }
        }
    }
  else
    {
      r = row;
      cached_iter->seq_iter = g_sequence_get_iter_at_pos (priv->sequence, row);
    }

  if (r > -1)
    {
      iter = g_object_new (FOO_TYPE_OBJECT_STORE_ITER,
                            "model", self,
                            "row", r,
                            NULL);
      iter->seq_iter = cached_iter->seq_iter;
    }

  return (ClutterModelIter *) iter;
}

static ClutterModelIter *
foo_object_store_insert_row (ClutterModel *self,
                             gint          index_)
{
  FooObjectStorePrivate *priv;
  FooObjectStoreIter    *iter;

  g_return_val_if_fail (FOO_IS_OBJECT_STORE (self), NULL);

  priv = GET_PRIVATE (self);
  iter = g_object_new (FOO_TYPE_OBJECT_STORE_ITER,
                       "model", self,
                       NULL);

  if (index_ < 0)
    {
      iter->seq_iter = g_sequence_append (priv->sequence, NULL);
      g_object_set (iter,
                    "row", g_sequence_get_length (priv->sequence) - 1,
                    NULL);
    }
  else
    {
      GSequenceIter *seq_iter;
      seq_iter = g_sequence_get_iter_at_pos (priv->sequence, index_);
      iter->seq_iter = g_sequence_insert_before (seq_iter, NULL);
      g_object_set (iter,
                    "row", index_,
                    NULL);
    }

  return CLUTTER_MODEL_ITER (iter);
}

static void
foo_object_store_remove_row (ClutterModel *self,
                             guint         row)
{
  FooObjectStorePrivate *priv;
  FooObjectStoreIter    *iter;

  g_return_if_fail (FOO_IS_OBJECT_STORE (self));

  priv = GET_PRIVATE (self);
  iter = g_object_new (FOO_TYPE_OBJECT_STORE_ITER,
                       "model", self,
                       "row", row,
                       NULL);
  iter->seq_iter = g_sequence_get_iter_at_pos (priv->sequence, row);

  /* the actual row is removed from the sequence inside
   * the ::row-removed signal class handler, so that every
   * handler connected to ::row-removed will still get
   * a valid iterator, and every signal connected to
   * ::row-removed with the AFTER flag will get an updated
   * store */
  g_signal_emit_by_name (self, "row-removed", iter);
  g_object_unref (iter);
}

typedef struct
{
  ClutterModel          *store;
  const gchar           *property_name;
  ClutterModelSortFunc   func;
  gpointer               data;
} SortClosure;

static gint
sort_store_default (gconstpointer a,
                    gconstpointer b,
                    gpointer      data)
{
  SortClosure *closure = data;
  GValue       p1 = { 0, };
  GValue       p2 = { 0, };

  if (a == NULL && b == NULL)
    return 0;
  else if (a == NULL)
    return -1;
  else if (b == NULL)
    return 1;

  g_object_get_property (G_OBJECT (a), closure->property_name, &p1);
  g_object_get_property (G_OBJECT (b), closure->property_name, &p2);

  return closure->func (closure->store, &p1, &p2, closure->data);
}

static void
foo_object_store_resort (ClutterModel         *self,
                         ClutterModelSortFunc  func,
                         gpointer              data)
{
  FooObjectStorePrivate *priv;
  SortClosure            closure;
  gint                   column;

  g_return_if_fail (FOO_IS_OBJECT_STORE (self));

  priv = GET_PRIVATE (self);
  column = clutter_model_get_sorting_column (self);

  closure.store = self;
  closure.property_name = clutter_model_get_column_name (self, column);
  closure.func = func;
  closure.data = data;

  g_sequence_sort (priv->sequence, sort_store_default, &closure);
}

static void
foo_object_store_row_removed (ClutterModel     *self,
                              ClutterModelIter *iter_)
{
  FooObjectStoreIter  *iter;
  GObject             *object;

  iter = FOO_OBJECT_STORE_ITER (iter_);
  object = g_sequence_get (iter->seq_iter);

  if (G_IS_OBJECT (object))
    {
      foo_object_store_detach_object (FOO_OBJECT_STORE (self), object);
      g_object_unref (object);
    }

  g_sequence_remove (iter->seq_iter);
}

static void
foo_object_store_class_init (FooObjectStoreClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterModelClass *store_class = CLUTTER_MODEL_CLASS (klass);

  g_type_class_add_private (klass, sizeof (FooObjectStorePrivate));

  gobject_class->dispose = foo_object_store_dispose;
  gobject_class->finalize = foo_object_store_finalize;

  store_class->get_iter_at_row = foo_object_store_get_iter_at_row;
  store_class->insert_row      = foo_object_store_insert_row;
  store_class->remove_row      = foo_object_store_remove_row;
  store_class->resort          = foo_object_store_resort;

  store_class->row_removed     = foo_object_store_row_removed;
}

/*
 * The destroy function is called regardless of NULL data.
 */
static void
_unref_if_non_null (GObject *object)
{
  if (G_IS_OBJECT (object))
    g_object_unref (object);
}

static void
foo_object_store_init (FooObjectStore *self)
{
  FooObjectStorePrivate *priv = GET_PRIVATE (self);

  priv->sequence = g_sequence_new ((GDestroyNotify) _unref_if_non_null);
  priv->cached_iter = g_object_new (FOO_TYPE_OBJECT_STORE_ITER,
                                    "model", self,
                                    NULL);
}

ClutterModel *
foo_object_store_new (guint n_columns,
                      ...)
{
  ClutterModel  *self;
  GType          types[n_columns];
  const gchar   *names[n_columns];
  guint          i;
  va_list        args;

  g_return_val_if_fail (n_columns > 0, NULL);

  self = g_object_new (FOO_TYPE_OBJECT_STORE, NULL);

  va_start (args, n_columns);

  for (i = 0; i < n_columns; i++)
    {
      types[i] = va_arg (args, GType);
      names[i] = va_arg (args, gchar *);
    }

  va_end (args);

  clutter_model_set_types (self, n_columns, types);
  clutter_model_set_names (self, n_columns, names);

  return self;
}

static void
foo_object_store_object_property_notify (GObject         *object,
                                         GParamSpec      *pspec,
                                         FooObjectStore  *self)
{
  FooObjectStorePrivate *priv = GET_PRIVATE (self);
  GSequenceIter         *seq_iter;
  FooObjectStoreIter    *iter;
  GSequenceIter         *notify_seq_iter = NULL;

  /* Find corresponding row of changed object. */
  for (seq_iter = g_sequence_get_begin_iter (priv->sequence);
       !g_sequence_iter_is_end (seq_iter);
       seq_iter = g_sequence_iter_next (seq_iter))
    {
      if (object == g_sequence_get (seq_iter))
        {
          notify_seq_iter = seq_iter;
          break;
        }
    }

  g_return_if_fail (notify_seq_iter);

  iter = g_object_new (FOO_TYPE_OBJECT_STORE_ITER,
                       "model", self,
                       NULL);
  iter->seq_iter = notify_seq_iter;
  g_signal_emit_by_name (self, "row-changed", iter);
  g_object_unref (iter);
}

static void
foo_object_store_attach_object (FooObjectStore  *self,
                                GObject         *object)
{
  guint        n_columns;
  const gchar *column_name;
  gchar       *signal_name;
  guint        i;

  /* Start at column 1 because 0 hold the actual object. */
  n_columns = clutter_model_get_n_columns (CLUTTER_MODEL (self));
  for (i = 1; i < n_columns; i++)
    {
      column_name = clutter_model_get_column_name (CLUTTER_MODEL (self), i);
      signal_name = g_strdup_printf ("notify::%s", column_name);
      g_signal_connect (object,
                        signal_name,
                        G_CALLBACK (foo_object_store_object_property_notify),
                        self);
      g_free (signal_name);
    }
}

static void
foo_object_store_detach_object (FooObjectStore  *self,
                                GObject         *object)
{
  g_signal_handlers_disconnect_by_func (object,
                                        foo_object_store_object_property_notify,
                                        self);
}

void
foo_object_store_foreach_unfiltered (FooObjectStore          *self,
                                     ClutterModelForeachFunc  func,
                                     gpointer                 user_data)
{
  FooObjectStorePrivate *priv;
  GSequenceIter *seq_iter;

  g_return_if_fail (FOO_IS_OBJECT_STORE (self));
  g_return_if_fail (func);

  priv = GET_PRIVATE (self);

  for (seq_iter = g_sequence_get_begin_iter (priv->sequence);
       !g_sequence_iter_is_end (seq_iter);
       seq_iter = g_sequence_iter_next (seq_iter))
    {
      priv->cached_iter->seq_iter = seq_iter;
      if (!func ((ClutterModel *) self,
                 (ClutterModelIter *) priv->cached_iter,
                 user_data))
        {
          break;
        }
    }
}

gint
foo_object_store_remove (FooObjectStore  *self,
                         GObject         *object)
{
  FooObjectStorePrivate *priv;
  GSequenceIter *seq_iter;
  gint           row = -1;

  g_return_val_if_fail (FOO_IS_OBJECT_STORE (self), FALSE);
  g_return_val_if_fail (G_IS_OBJECT (object), FALSE);

  priv = GET_PRIVATE (self);

  for (seq_iter = g_sequence_get_begin_iter (priv->sequence);
       !g_sequence_iter_is_end (seq_iter);
       seq_iter = g_sequence_iter_next (seq_iter))
    {
      GObject *o = g_sequence_get (seq_iter);
      row++;
      if (o == object)
        {
          foo_object_store_remove_row (CLUTTER_MODEL (self), row);
          break;
        }
    }

  return row;
}

