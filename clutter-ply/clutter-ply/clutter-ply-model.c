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

/**
 * SECTION:clutter-ply-model
 * @short_description: An actor that can be used to render a PLY model.
 *
 * #ClutterPlyModel is an actor subclass that can be used to render a
 * 3D model. The model is a normal #ClutterActor that can be animated
 * and positioned with the methods of #ClutterActor. The model is
 * rendered at the origin of the actor so that a vertex at 0,0,0 will
 * be at the top left of the actor's allocation.
 *
 * The actual data for the model is stored in a separate object called
 * #ClutterPlyData. This can be used to share the data for a model
 * between multiple actors without having to duplicate resources of
 * the data. Alternatively clutter_ply_model_new_from_file() can be
 * used as a convenience wrapper to easily make an actor out of a PLY
 * without having to worry about #ClutterPlyData. To share the data
 * with another actor, call clutter_ply_model_get_data() on an
 * existing actor then call clutter_ply_model_set_data() with the
 * return value on a new actor.
 *
 * The model can be rendered with any Cogl material. By default the
 * model will use a solid white material. The material color is
 * blended with the model's vertex colors so the white material will
 * cause the vertex colors to be used directly. #ClutterPlyData is
 * able to load texture coordinates from the PLY file so it is
 * possible to render a textured model by setting a texture layer on
 * the material, like so:
 *
 * |[
 *   /&ast; Create an actor out of a PLY model file &ast;/
 *   ClutterActor *model
 *     = clutter_ply_model_new_from_file ("some-model.ply", NULL);
 *   /&ast; Get a handle to the default material for the actor &ast;/
 *   CoglHandle material
 *     = clutter_ply_model_get_material (CLUTTER_PLY_MODEL (model));
 *   /&ast; Load a texture image from a file &ast;/
 *   CoglHandle texture
 *     = cogl_texture_new_from_file ("some-image.png", COGL_TEXTURE_NONE,
 *                                   COGL_PIXEL_FORMAT_ANY, NULL);
 *   /&ast; Set a texture layer on the material &ast;/
 *   cogl_material_set_layer (material, 0, texture);
 *   /&ast; The texture is now referenced by the material so we can
 *     drop the reference we have &ast;/
 *   cogl_handle_unref (texture);
 * ]|
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib-object.h>
#include <string.h>
#include <cogl/cogl.h>
#include <clutter/clutter.h>

#include "clutter-ply-model.h"
#include "clutter-ply-data.h"

static void clutter_ply_model_dispose (GObject *object);

static void clutter_ply_model_get_property (GObject *object,
                                            guint prop_id,
                                            GValue *value,
                                            GParamSpec *pspec);
static void clutter_ply_model_set_property (GObject *object,
                                            guint prop_id,
                                            const GValue *value,
                                            GParamSpec *pspec);

static void clutter_ply_model_paint (ClutterActor *actor);

static void clutter_ply_model_pick (ClutterActor *actor,
                                    const ClutterColor *pick_color);

G_DEFINE_TYPE (ClutterPlyModel, clutter_ply_model, CLUTTER_TYPE_ACTOR);

#define CLUTTER_PLY_MODEL_GET_PRIVATE(obj)                      \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CLUTTER_PLY_TYPE_MODEL,  \
                                ClutterPlyModelPrivate))

struct _ClutterPlyModelPrivate
{
  ClutterPlyData *data;
  CoglHandle material, pick_material;
};

enum
  {
    PROP_0,

    PROP_MATERIAL,
    PROP_DATA
  };

static void
clutter_ply_model_class_init (ClutterPlyModelClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  ClutterActorClass *actor_class = (ClutterActorClass *) klass;
  GParamSpec *pspec;

  gobject_class->dispose = clutter_ply_model_dispose;
  gobject_class->get_property = clutter_ply_model_get_property;
  gobject_class->set_property = clutter_ply_model_set_property;

  actor_class->paint = clutter_ply_model_paint;
  actor_class->pick = clutter_ply_model_pick;

  pspec = g_param_spec_boxed ("material",
                              "Material",
                              "The Cogl material to render with",
                              COGL_TYPE_HANDLE,
                              G_PARAM_READABLE | G_PARAM_WRITABLE
                              | G_PARAM_STATIC_NAME
                              | G_PARAM_STATIC_NICK
                              | G_PARAM_STATIC_BLURB);
  g_object_class_install_property (gobject_class, PROP_MATERIAL, pspec);

  pspec = g_param_spec_object ("data",
                               "Data",
                               "The ClutterPlyData to render",
                               CLUTTER_PLY_TYPE_DATA,
                               G_PARAM_READABLE | G_PARAM_WRITABLE
                               | G_PARAM_STATIC_NAME
                               | G_PARAM_STATIC_NICK
                               | G_PARAM_STATIC_BLURB);
  g_object_class_install_property (gobject_class, PROP_DATA, pspec);

  g_type_class_add_private (klass, sizeof (ClutterPlyModelPrivate));
}

static void
clutter_ply_model_init (ClutterPlyModel *self)
{
  ClutterPlyModelPrivate *priv;

  priv = self->priv = CLUTTER_PLY_MODEL_GET_PRIVATE (self);

  /* Default to a plain white material */
  priv->material = cogl_material_new ();
}

/**
 * clutter_ply_model_new:
 *
 * Constructs a new #ClutterPlyModel. Nothing will be rendered by the
 * model until a #ClutterPlyData is attached using
 * clutter_ply_model_set_data().
 *
 * Return value: a new #ClutterPlyModel.
 */

ClutterActor *
clutter_ply_model_new (void)
{
  ClutterActor *self = g_object_new (CLUTTER_PLY_TYPE_MODEL, NULL);

  return self;
}

/**
 * clutter_ply_model_new_from_file:
 * @filename: The name of a PLY file to load.
 * @error: Return location for a #GError or %NULL.
 *
 * This is a convenience function that creates a new #ClutterPlyData
 * and immediately loads the data in @filename. If the load succeeds a
 * new #ClutterPlyModel will be created for the data. The model has a
 * default white material so that if vertices of the model have any
 * color attributes they will be used directly. The material does not
 * have textures by default so if you want the model to be textured
 * you will need to modify the material.
 *
 * Return value: a new #ClutterPlyModel or %NULL if the load failed.
 */
ClutterActor *
clutter_ply_model_new_from_file (const gchar *filename,
                                 GError **error)
{
  ClutterPlyData *data = clutter_ply_data_new ();
  ClutterActor *model = NULL;

  if (clutter_ply_data_load (data, filename, error))
    {
      model = clutter_ply_model_new ();
      clutter_ply_model_set_data (CLUTTER_PLY_MODEL (model), data);
    }

  g_object_unref (data);

  return model;
}

static void
clutter_ply_model_dispose (GObject *object)
{
  ClutterPlyModel *self = (ClutterPlyModel *) object;
  ClutterPlyModelPrivate *priv = self->priv;

  clutter_ply_model_set_data (self, NULL);
  clutter_ply_model_set_material (self, COGL_INVALID_HANDLE);

  if (priv->pick_material)
    {
      cogl_handle_unref (priv->pick_material);
      priv->pick_material = COGL_INVALID_HANDLE;
    }

  G_OBJECT_CLASS (clutter_ply_model_parent_class)->dispose (object);
}

/**
 * clutter_ply_model_set_material:
 * @self: A #ClutterPlyModel instance
 * @material: A handle to a Cogl material
 *
 * Replaces the material that will be used to render the model with
 * the given one. By default a #ClutterPlyModel will use a solid white
 * material. However the color of the material is still blended with
 * the vertex colors so the white material will cause the vertex
 * colors to be used directly. If you want the model to be textured
 * you will need to create a material that has a texture layer and set
 * it with this function.
 */
void
clutter_ply_model_set_material (ClutterPlyModel *self,
                                CoglHandle material)
{
  ClutterPlyModelPrivate *priv;

  g_return_if_fail (CLUTTER_PLY_IS_MODEL (self));
  g_return_if_fail (material == COGL_INVALID_HANDLE
                    || cogl_is_material (material));

  priv = self->priv;

  if (material)
    cogl_handle_ref (material);

  if (priv->material)
    cogl_handle_unref (priv->material);

  priv->material = material;

  clutter_actor_queue_redraw (CLUTTER_ACTOR (self));

  g_object_notify (G_OBJECT (self), "material");
}

/**
 * clutter_ply_model_get_material:
 * @self: A #ClutterPlyModel instance
 *
 * Gets the material that will be used to render the model. The
 * material can be modified to affect the appearence of the model. By
 * default the material will be solid white.
 *
 * Return value: a handle to the Cogl material used by the model.
 */
CoglHandle
clutter_ply_model_get_material (ClutterPlyModel *self)
{
  g_return_val_if_fail (CLUTTER_PLY_IS_MODEL (self), COGL_INVALID_HANDLE);

  return self->priv->material;
}

static void
clutter_ply_model_paint (ClutterActor *actor)
{
  ClutterPlyModel *self = CLUTTER_PLY_MODEL (actor);
  ClutterPlyModelPrivate *priv;

  g_return_if_fail (CLUTTER_PLY_IS_MODEL (self));

  priv = self->priv;

  /* Silently fail if we haven't got any data or a material */
  if (priv->data == NULL || priv->material == COGL_INVALID_HANDLE)
    return;

  cogl_set_source (priv->material);

  clutter_ply_data_render (priv->data);
}

static void
clutter_ply_model_pick (ClutterActor *actor,
                        const ClutterColor *pick_color)
{
  ClutterPlyModel *self = CLUTTER_PLY_MODEL (actor);
  ClutterPlyModelPrivate *priv;
  CoglColor color;

  g_return_if_fail (CLUTTER_PLY_IS_MODEL (self));

  priv = self->priv;

  /* Silently fail if we haven't got any data */
  if (priv->data == NULL)
    return;

  if (priv->pick_material == COGL_INVALID_HANDLE)
    {
      GError *error = NULL;
      priv->pick_material = cogl_material_new ();
      if (!cogl_material_set_layer_combine (priv->pick_material, 0,
                                            "RGBA=REPLACE(CONSTANT)",
                                            &error))
        {
          g_warning ("Error setting pick combine: %s", error->message);
          g_clear_error (&error);
        }
    }

  cogl_color_set_from_4ub (&color,
                           pick_color->red,
                           pick_color->green,
                           pick_color->blue,
                           255);
  cogl_material_set_layer_combine_constant (priv->pick_material, 0, &color);

  cogl_set_source (priv->pick_material);

  clutter_ply_data_render (priv->data);
}

/**
 * clutter_ply_model_get_data:
 * @self: A #ClutterPlyModel instance
 *
 * Gets the model data that will be used to render the actor.
 *
 * Return value: A pointer to a #ClutterPlyData instance or %NULL if
 * no data has been set yet.
 */
ClutterPlyData *
clutter_ply_model_get_data (ClutterPlyModel *self)
{
  g_return_val_if_fail (CLUTTER_PLY_IS_MODEL (self), NULL);

  return self->priv->data;
}

/**
 * clutter_ply_model_set_data:
 * @self: A #ClutterPlyModel instance
 * @data: The new #ClutterPlyData
 *
 * Replaces the data used by the actor with @data. A reference is
 * taken on @data so if you no longer need it you should unref it with
 * g_object_unref().
 */
void
clutter_ply_model_set_data (ClutterPlyModel *self,
                            ClutterPlyData *data)
{
  ClutterPlyModelPrivate *priv;

  g_return_if_fail (CLUTTER_PLY_IS_MODEL (self));
  g_return_if_fail (data == NULL || CLUTTER_PLY_IS_DATA (data));

  priv = self->priv;

  if (data)
    g_object_ref (data);

  if (priv->data)
    g_object_unref (priv->data);

  priv->data = data;

  clutter_actor_queue_redraw (CLUTTER_ACTOR (self));

  g_object_notify (G_OBJECT (self), "data");
}

static void
clutter_ply_model_get_property (GObject *object,
                                guint prop_id,
                                GValue *value,
                                GParamSpec *pspec)
{
  ClutterPlyModel *model = CLUTTER_PLY_MODEL (object);

  switch (prop_id)
    {
    case PROP_MATERIAL:
      g_value_set_boxed (value, clutter_ply_model_get_material (model));
      break;

    case PROP_DATA:
      g_value_set_object (value, clutter_ply_model_get_data (model));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
clutter_ply_model_set_property (GObject *object,
                                guint prop_id,
                                const GValue *value,
                                GParamSpec *pspec)
{
  ClutterPlyModel *model = CLUTTER_PLY_MODEL (object);

  switch (prop_id)
    {
    case PROP_MATERIAL:
      clutter_ply_model_set_material (model, g_value_get_boxed (value));
      break;

    case PROP_DATA:
      clutter_ply_model_set_data (model, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}
