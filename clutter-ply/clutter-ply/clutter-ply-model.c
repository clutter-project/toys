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
 * and positioned with the methods of #ClutterActor.
 *
 * By default the model will be scaled to best fit within the size of
 * the actor. Therefore it is possible to take a small model that may
 * have positions ranging between -1 and 1 and draw it at a larger
 * size just by setting the size on the actor. This behaviour can be
 * disabled with clutter_ply_model_set_fit_to_allocation().
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

static void clutter_ply_model_get_preferred_width (ClutterActor *actor,
                                                   gfloat for_height,
                                                   gfloat *minimum_width,
                                                   gfloat *natural_width);
static void clutter_ply_model_get_preferred_height (ClutterActor *actor,
                                                    gfloat for_width,
                                                    gfloat *minimum_height,
                                                    gfloat *natural_height);

static void clutter_ply_model_allocate (ClutterActor *actor,
                                        const ClutterActorBox *box,
                                        ClutterAllocationFlags flags);

G_DEFINE_TYPE (ClutterPlyModel, clutter_ply_model, CLUTTER_TYPE_ACTOR);

#define CLUTTER_PLY_MODEL_GET_PRIVATE(obj)                      \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CLUTTER_PLY_TYPE_MODEL,  \
                                ClutterPlyModelPrivate))

struct _ClutterPlyModelPrivate
{
  ClutterPlyData *data;
  CoglHandle material, pick_material;
  /* Whether the model should be transformed to fill the allocation */
  gboolean fit_to_allocation;
  /* The amount to scale (on all axes) when fit_to_allocation is
     TRUE. This is calculated in the allocate method */
  gfloat scale;
  /* Translation used when fit_to_allocation is TRUE. This is
     calculated in the allocate method */
  gfloat translate_x, translate_y, translate_z;
};

enum
  {
    PROP_0,

    PROP_MATERIAL,
    PROP_DATA,
    PROP_FIT_TO_ALLOCATION
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
  actor_class->get_preferred_width = clutter_ply_model_get_preferred_width;
  actor_class->get_preferred_height = clutter_ply_model_get_preferred_height;
  actor_class->allocate = clutter_ply_model_allocate;

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

  pspec = g_param_spec_boolean ("fit-to-allocation",
                                "Fit to allocation",
                                "Whether to transform the model so that "
                                "it fills the actor's allocation while "
                                "preserving the aspect ratio",
                                TRUE,
                                G_PARAM_READABLE | G_PARAM_WRITABLE
                                | G_PARAM_STATIC_NAME
                                | G_PARAM_STATIC_NICK
                                | G_PARAM_STATIC_BLURB);
  g_object_class_install_property (gobject_class,
                                   PROP_FIT_TO_ALLOCATION, pspec);

  g_type_class_add_private (klass, sizeof (ClutterPlyModelPrivate));
}

static void
clutter_ply_model_init (ClutterPlyModel *self)
{
  ClutterPlyModelPrivate *priv;

  priv = self->priv = CLUTTER_PLY_MODEL_GET_PRIVATE (self);

  /* Default to a plain white material */
  priv->material = cogl_material_new ();

  priv->fit_to_allocation = TRUE;
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
clutter_ply_model_render_data (ClutterPlyModel *self)
{
  ClutterPlyModelPrivate *priv = self->priv;

  if (priv->fit_to_allocation)
    {
      cogl_push_matrix ();

      cogl_translate (priv->translate_x,
                      priv->translate_y,
                      priv->translate_z);
      cogl_scale (priv->scale, priv->scale, priv->scale);
    }

  clutter_ply_data_render (priv->data);

  if (priv->fit_to_allocation)
    cogl_pop_matrix ();
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

  clutter_ply_model_render_data (self);
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

  clutter_ply_model_render_data (self);
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

  clutter_actor_queue_relayout (CLUTTER_ACTOR (self));

  g_object_notify (G_OBJECT (self), "data");
}

/**
 * clutter_ply_model_get_fit_to_allocation:
 * @self: A #ClutterPlyModel instance
 *
 * Return value: whether the actor will try to scale the model to fit
 * within the allocation.
 */
gboolean
clutter_ply_model_get_fit_to_allocation (ClutterPlyModel *self)
{
  g_return_val_if_fail (CLUTTER_PLY_IS_MODEL (self), FALSE);

  return self->priv->fit_to_allocation;
}

/**
 * clutter_ply_model_set_fit_to_allocation:
 * @self: A #ClutterPlyModel instance
 * @fit_to_allocation: New value
 *
 * This sets whether the actor should scale the model to fit the
 * actor's allocation. If it's %TRUE then all of the axes of the model
 * will be scaled by the same amount to fill the allocation as much as
 * possible without distorting the aspect ratio. The model is also
 * translated so that it is at the center of the allocation and
 * centered at 0 along the z axis. The size along the z axis is not
 * considered when calculating a scale so if the model is largest
 * along that axis then the actor may appear too large. The
 * transformations are applied in addition to the actor's
 * transformations so it is still possible scale the actor further
 * using the scale-x and scale-y properties. The preferred size of the
 * actor will be the width and height of the model. If
 * width-for-height or height-for-width allocation is being used then
 * #ClutterPlyModel will return whatever width or height will exactly
 * preserve the aspect ratio.
 *
 * If the value is %FALSE then the actor is not transformed so the
 * origin of the model will be the top left corner of the actor. The
 * preferred size of the actor will be maximum extents of the model
 * although the allocation is not considered during paint so if the
 * model extends past the allocated size then it will draw outside the
 * allocation.
 *
 * The default value is %TRUE.
 */
void
clutter_ply_model_set_fit_to_allocation (ClutterPlyModel *self,
                                         gboolean fit_to_allocation)
{
  ClutterPlyModelPrivate *priv;

  g_return_if_fail (CLUTTER_PLY_IS_MODEL (self));

  priv = self->priv;

  if (priv->fit_to_allocation != fit_to_allocation)
    {
      priv->fit_to_allocation = fit_to_allocation;
      clutter_actor_queue_relayout (CLUTTER_ACTOR (self));
      g_object_notify (G_OBJECT (self), "fit-to-allocation");
    }
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

    case PROP_FIT_TO_ALLOCATION:
      g_value_set_boolean (value,
                           clutter_ply_model_get_fit_to_allocation (model));
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

    case PROP_FIT_TO_ALLOCATION:
      clutter_ply_model_set_fit_to_allocation (model,
                                               g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
clutter_ply_model_get_preferred_width (ClutterActor *actor,
                                       gfloat for_height,
                                       gfloat *minimum_width_p,
                                       gfloat *natural_width_p)
{
  ClutterPlyModel *model = CLUTTER_PLY_MODEL (actor);
  ClutterPlyModelPrivate *priv = model->priv;
  ClutterVertex min_vertex, max_vertex;
  gfloat minimum_width, natural_width;

  if (priv->data)
    {
      clutter_ply_data_get_extents (priv->data, &min_vertex, &max_vertex);

      if (priv->fit_to_allocation)
        {
          gfloat model_width = max_vertex.x - min_vertex.x;
          gfloat model_height = max_vertex.y - min_vertex.y;

          if (for_height < 0.0f || model_height == 0.0f)
            natural_width = model_width;
          else
            /* Pick a width that would preserve the aspect ratio */
            natural_width = for_height * model_width / model_height;

          minimum_width = 0.0f;
        }
      else
        /* We can't report if the actor draws to the left of the origin so
           the best we can do is to report the extent to the right of the
           origin. If the data also contains vertices to the left of the
           origin then this won't be the actual width */
        minimum_width = natural_width = max_vertex.x;
    }
  else
    minimum_width = natural_width = 0.0f;

  if (minimum_width_p)
    *minimum_width_p = minimum_width;
  if (natural_width_p)
    *natural_width_p = natural_width;
}

static void
clutter_ply_model_get_preferred_height (ClutterActor *actor,
                                        gfloat for_width,
                                        gfloat *minimum_height_p,
                                        gfloat *natural_height_p)
{
  ClutterPlyModel *model = CLUTTER_PLY_MODEL (actor);
  ClutterPlyModelPrivate *priv = model->priv;
  ClutterVertex min_vertex, max_vertex;
  gfloat minimum_height, natural_height;

  if (priv->data)
    {
      clutter_ply_data_get_extents (priv->data, &min_vertex, &max_vertex);

      if (priv->fit_to_allocation)
        {
          gfloat model_width = max_vertex.x - min_vertex.x;
          gfloat model_height = max_vertex.y - min_vertex.y;

          if (for_width < 0.0f || model_width == 0.0f)
            natural_height = model_height;
          else
            /* Pick a height that would preserve the aspect ratio */
            natural_height = for_width * model_height / model_width;

          minimum_height = 0.0f;
        }
      else
        /* We can't report if the actor draws above the origin so the
           best we can do is to report the extent below the origin. If
           the data also contains vertices above the origin then this
           won't be the actual height */
        minimum_height = natural_height = max_vertex.y;
    }
  else
    minimum_height = natural_height = 0.0f;

  if (minimum_height_p)
    *minimum_height_p = minimum_height;
  if (natural_height_p)
    *natural_height_p = natural_height;
}

static gfloat
clutter_ply_model_calculate_scale (gfloat target_extents,
                                   gfloat min,
                                   gfloat max)
{
  if (min == max)
    return G_MAXFLOAT;
  else
    return target_extents / (max - min);
}

static void
clutter_ply_model_allocate (ClutterActor *actor,
                            const ClutterActorBox *box,
                            ClutterAllocationFlags flags)
{
  ClutterPlyModel *self = CLUTTER_PLY_MODEL (actor);
  ClutterPlyModelPrivate *priv = self->priv;

  CLUTTER_ACTOR_CLASS (clutter_ply_model_parent_class)
    ->allocate (actor, box, flags);

  if (priv->fit_to_allocation && priv->data)
    {
      ClutterVertex min_vertex, max_vertex;
      gfloat scale, min_scale;

      /* Try to scale the model uniformly so that it will fill the
         maximum amount of space without breaking the aspect
         ratio. The model is then centered in the allocation */
      clutter_ply_data_get_extents (priv->data, &min_vertex, &max_vertex);

      min_scale = clutter_ply_model_calculate_scale (box->x2 - box->x1,
                                                     min_vertex.x,
                                                     max_vertex.x);
      scale = clutter_ply_model_calculate_scale (box->y2 - box->y1,
                                                 min_vertex.y,
                                                 max_vertex.y);
      if (min_scale > scale)
        min_scale = scale;

      if (min_scale >= G_MAXFLOAT)
        min_scale = 0.0f;

      priv->scale = min_scale;

      priv->translate_x = ((box->x2 - box->x1) / 2.0f
                           - (min_vertex.x + max_vertex.x) / 2.0f * min_scale);
      priv->translate_y = ((box->y2 - box->y1) / 2.0f
                           - (min_vertex.y + max_vertex.y) / 2.0f * min_scale);
      priv->translate_z = -(min_vertex.z + max_vertex.z) / 2.0f * min_scale;
    }
}
