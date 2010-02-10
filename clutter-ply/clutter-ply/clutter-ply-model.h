/*
 * Clutter PLY - A library for displaying PLY models in a Clutter scene
 * Copyright (C) 2010  Intel Corporation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#if !defined(__CLUTTER_PLY_H_INSIDE__) && !defined(CLUTTER_PLY_COMPILATION)
#error "Only <clutter-ply/clutter-ply.h> can be included directly."
#endif

#ifndef __CLUTTER_PLY_MODEL_H__
#define __CLUTTER_PLY_MODEL_H__

#include <glib-object.h>
#include <clutter/clutter.h>
#include <clutter-ply/clutter-ply-data.h>

G_BEGIN_DECLS

#define CLUTTER_PLY_TYPE_MODEL                  \
  (clutter_ply_model_get_type())
#define CLUTTER_PLY_MODEL(obj)                          \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               CLUTTER_PLY_TYPE_MODEL,  \
                               ClutterPlyModel))
#define CLUTTER_PLY_MODEL_CLASS(klass)                  \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                    \
                            CLUTTER_PLY_TYPE_MODEL,     \
                            ClutterPlyModelClass))
#define CLUTTER_PLY_IS_MODEL(obj)                       \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               CLUTTER_PLY_TYPE_MODEL))
#define CLUTTER_PLY_IS_MODEL_CLASS(klass)               \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                    \
                            CLUTTER_PLY_TYPE_MODEL))
#define CLUTTER_PLY_MODEL_GET_CLASS(obj)                \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              CLUTTER_PLY_TYPE_MODEL,   \
                              ClutterPlyModelClass))

typedef struct _ClutterPlyModel        ClutterPlyModel;
typedef struct _ClutterPlyModelClass   ClutterPlyModelClass;
typedef struct _ClutterPlyModelPrivate ClutterPlyModelPrivate;

struct _ClutterPlyModelClass
{
  ClutterActorClass parent_class;
};

struct _ClutterPlyModel
{
  ClutterActor parent;

  ClutterPlyModelPrivate *priv;
};

GType clutter_ply_model_get_type (void) G_GNUC_CONST;

ClutterActor *clutter_ply_model_new (void);

ClutterActor *clutter_ply_model_new_from_file (const gchar *filename,
                                               GError **error);

CoglHandle clutter_ply_model_get_material (ClutterPlyModel *self);
void clutter_ply_model_set_material (ClutterPlyModel *self,
                                     CoglHandle material);

ClutterPlyData *clutter_ply_model_get_data (ClutterPlyModel *self);
void clutter_ply_model_set_data (ClutterPlyModel *self,
                                 ClutterPlyData *data);

G_END_DECLS

#endif /* __CLUTTER_PLY_MODEL_H__ */
