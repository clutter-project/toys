/*
 * ClutterTextureOdo.
 *
 * A deformable texture actor.
 *
 * Authored By Tomas Frydrych  <tf@openedhand.com>
 *
 * Copyright (C) 2007,2008 OpenedHand
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
 */

#include <clutter/clutter.h>

#include "clutter-texture-odo.h"

enum
{
  PROP_0,
  PROP_PARENT_TEXTURE,
  PROP_DISTORT_FUNC,
  PROP_DISTORT_FUNC_DATA,
  PROP_TILE_WIDTH,
  PROP_TILE_HEIGHT,
  PROP_MESH,
};

G_DEFINE_TYPE (ClutterTextureOdo,
               clutter_texture_odo,
               CLUTTER_TYPE_ACTOR);

#define CLUTTER_TEXTURE_ODO_GET_PRIVATE(obj) \
(G_TYPE_INSTANCE_GET_PRIVATE ((obj), CLUTTER_TYPE_TEXTURE_ODO, ClutterTextureOdoPrivate))

#ifndef CLUTTER_PARAM_READWRITE
#define CLUTTER_PARAM_READWRITE \
        G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |G_PARAM_STATIC_BLURB
#endif

struct _ClutterTextureOdoPrivate
{
  ClutterTexture      *parent_texture;

  ClutterTextureDistortFunc    distort_func;
  gpointer                     distort_func_data;

  guint tile_width;
  guint tile_height;

  ClutterMesh   mesh;
  
  ClutterTextureOdoCullMode    cull_mode;
};

static void
texture_odo_render_to_gl_quad (ClutterTextureOdo *otex)
{
  const ClutterColor white = { 0xff, 0xff, 0xff, 0xff };
  gint   pwidth, pheight;
  gboolean first;
  CoglHandle handle;
  CoglTextureVertex vertices[4];

  ClutterTextureOdoPrivate *priv = otex->priv;
  ClutterActor *parent_actor = CLUTTER_ACTOR (priv->parent_texture);

  priv = otex->priv;

  if (!CLUTTER_ACTOR_IS_REALIZED (parent_actor))
      clutter_actor_realize (parent_actor);

  clutter_texture_get_base_size (priv->parent_texture, &pwidth, &pheight); 
  handle = clutter_texture_get_cogl_texture (priv->parent_texture);
  
  vertices[0].color = white;
  vertices[1].color = white;
  vertices[2].color = white;
  vertices[3].color = white;

  if (priv->mesh.points)
    {
      guint i, j, im, jm;
      gfloat txf, tyf1, tyf2;
      guint tile_width, tile_height;

      tile_width  = pwidth  / (priv->mesh.dimension_x - 1);
      tile_height = pheight / (priv->mesh.dimension_y - 1);

      for (j = 0, jm = 0; j < pheight - 1; ++jm)
        {
          tyf1 = (float) j;
          tyf2 = (float) (j + tile_height);
          
          first = TRUE;
          
          for (i = 0, im = 0; i < pwidth; ++im)
            {
              txf  = (float) i;

#define P(a,b) priv->mesh.points[a*priv->mesh.dimension_x + b]
              vertices[0] = vertices[3];
              vertices[1] = vertices[2];
              vertices[2].x = P (im, jm).x;
              vertices[2].y = P (im, jm).y;
              vertices[2].z = P (im, jm).z;
              vertices[2].tx = CLUTTER_FLOAT_TO_FIXED (txf/(float)pwidth);
              vertices[2].ty = CLUTTER_FLOAT_TO_FIXED (tyf1/(float)pheight);
              vertices[3].x = P (im, jm+1).x;
              vertices[3].y = P (im, jm+1).y;
              vertices[3].z = P (im, jm+1).z;
              vertices[3].tx = CLUTTER_FLOAT_TO_FIXED (txf/(float)pwidth);
              vertices[3].ty = CLUTTER_FLOAT_TO_FIXED (tyf2/(float)pheight);
              
              if (!first)
                cogl_texture_polygon (handle, 4, vertices, FALSE);
              else
                first = FALSE;
#undef P
              /* Handle any remnant not divisible by tile_width */
              i += tile_width;
              
              if (i >= pwidth && i < pwidth + tile_width - 1)
                i = pwidth - 1;
            }

          /* Handle any remnant not divisible by tile_height */
          j += tile_height;

          if (j > pheight && j < pheight + tile_height - 1)
            j = pheight - 1;
      }
    }
  else if (priv->distort_func)
    {
      guint i, j;
      gfloat txf, tyf1, tyf2;
      gboolean skip1, skip2, skip3, skip4;
      
      skip1 = skip2 = skip3 = skip4 = FALSE;
      
      for (j = 0; j < pheight - 1; )
        {
          tyf1 = (float) j;
          tyf2 = (float) (j + priv->tile_height);
          
          first = TRUE;
          
          for (i = 0; i < pwidth; )
            {
              ClutterFixed x2, y2, z2;

              txf  = (float) i;
              
              skip1 = skip4;
              skip2 = skip3;
              
              vertices[0] = vertices[3];
              vertices[1] = vertices[2];
              
              skip3 = !priv->distort_func (priv->parent_texture,
                                  CLUTTER_INT_TO_FIXED (i),
                                  CLUTTER_INT_TO_FIXED (j), 0, &x2, &y2, &z2,
                                  &vertices[2].color,
                                  priv->distort_func_data);
              
              vertices[2].x = x2;
              vertices[2].y = y2;
              vertices[2].z = z2;
              vertices[2].tx = CLUTTER_FLOAT_TO_FIXED (txf/(float)pwidth);
              vertices[2].ty = CLUTTER_FLOAT_TO_FIXED (tyf1/(float)pheight);
              
              skip4 = !priv->distort_func (priv->parent_texture,
                                  CLUTTER_INT_TO_FIXED (i),
                                  CLUTTER_INT_TO_FIXED (j) +
                                    CLUTTER_INT_TO_FIXED (priv->tile_height),
                                  0, &x2, &y2, &z2,
                                  &vertices[3].color,
                                  priv->distort_func_data);
              
              vertices[3].x = x2;
              vertices[3].y = y2;
              vertices[3].z = z2;
              vertices[3].tx = CLUTTER_FLOAT_TO_FIXED (txf/(float)pwidth);
              vertices[3].ty = CLUTTER_FLOAT_TO_FIXED (tyf2/(float)pheight);

              if (!first)
                {
                  if (!skip1 || !skip2 || !skip3 || !skip4)
                    {
                      if (priv->cull_mode == ODO_CULL_NONE)
                        cogl_texture_polygon (handle, 4, vertices, FALSE);
                      else
                        {
                          /* Do back/front-face culling */
                          ClutterFixed dot;
                          
                          /* Calculate the dot product of the normal and the
                           * z unit vector. */
                          dot = clutter_qmulx (vertices[1].x - vertices[0].x,
                                               vertices[2].y - vertices[0].y) -
                              clutter_qmulx (vertices[1].y - vertices[0].y,
                                             vertices[2].x - vertices[0].x);
                          
                          if (((dot < 0) &&
                               (priv->cull_mode == ODO_CULL_FRONT)) ||
                              ((dot >= 0) &&
                               (priv->cull_mode == ODO_CULL_BACK)))
                            cogl_texture_polygon (handle, 4, vertices, TRUE);
	                      }
                    }
                }
              else
                first = FALSE;

              /* Handle any remnant not divisible by tile_width */
              i += priv->tile_width;

              if (i >= pwidth && i < pwidth + priv->tile_width - 1)
                i = pwidth - 1;
            }

          /* Handle any remnant not divisible by tile_height */
          j += priv->tile_height;

          if (j > pheight && j < pheight + priv->tile_height - 1)
            j = pheight - 1;
        }
    }
  else
    {
      cogl_texture_rectangle (handle,
                              0,
                              0,
                              CLUTTER_INT_TO_FIXED (pwidth),
                              CLUTTER_INT_TO_FIXED (pheight),
                              0,
                              0,
                              CFX_ONE,
                              CFX_ONE);
    }
}

static void
clutter_texture_odo_paint (ClutterActor *self)
{
  ClutterTextureOdoPrivate  *priv;
  ClutterActor                *parent_texture;
  ClutterColor                 col = { 0xff, 0xff, 0xff, 0xff };

  priv = CLUTTER_TEXTURE_ODO (self)->priv;

  /* no need to paint stuff if we don't have a texture to clone */
  if (!priv->parent_texture)
    return;

  /* parent texture may have been hidden, there for need to make sure its 
   * realised with resources available.  
  */
  parent_texture = CLUTTER_ACTOR (priv->parent_texture);
  if (!CLUTTER_ACTOR_IS_REALIZED (parent_texture))
    clutter_actor_realize (parent_texture);

  cogl_push_matrix ();

  col.alpha = clutter_actor_get_opacity (self);
  cogl_color (&col);

  /* Parent paint translated us into position */
  texture_odo_render_to_gl_quad (CLUTTER_TEXTURE_ODO (self));

  cogl_pop_matrix ();
}

static void
set_parent_texture (ClutterTextureOdo *otex,
                    ClutterTexture      *texture)
{
  ClutterTextureOdoPrivate *priv = otex->priv;
  ClutterActor *actor = CLUTTER_ACTOR (otex);

  if (priv->parent_texture)
    {
      g_object_unref (priv->parent_texture);
      priv->parent_texture = NULL;
    }

  clutter_actor_hide (actor);

  if (texture) 
    {
      gint width, height;

      priv->parent_texture = g_object_ref (texture);

      /* Sync up the size to parent texture base pixbuf size. */
      clutter_texture_get_base_size (texture, &width, &height);
      clutter_actor_set_size (actor, width, height);

      /* queue a redraw if the cloned texture is already visible */
      if (CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR (priv->parent_texture)) &&
          CLUTTER_ACTOR_IS_VISIBLE (actor))
        clutter_actor_queue_redraw (actor);
    }
      
}

static void 
clutter_texture_odo_dispose (GObject *object)
{
  ClutterTextureOdo         *self = CLUTTER_TEXTURE_ODO(object);
  ClutterTextureOdoPrivate  *priv = self->priv;  

  if (priv->parent_texture)
    g_object_unref (priv->parent_texture);

  priv->parent_texture = NULL;

  G_OBJECT_CLASS (clutter_texture_odo_parent_class)->dispose (object);
}

static void 
clutter_texture_odo_finalize (GObject *object)
{
  G_OBJECT_CLASS (clutter_texture_odo_parent_class)->finalize (object);
}

static void
clutter_texture_odo_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  ClutterTextureOdo *otex = CLUTTER_TEXTURE_ODO (object);
  ClutterTextureOdoPrivate * priv = otex->priv;
  
  switch (prop_id)
    {
    case PROP_PARENT_TEXTURE:
      set_parent_texture (otex, g_value_get_object (value));
      break;
    case PROP_DISTORT_FUNC:
      priv->distort_func = g_value_get_pointer(value);
      break;
    case PROP_DISTORT_FUNC_DATA:
      priv->distort_func_data = g_value_get_pointer(value);
      break;
    case PROP_TILE_WIDTH:
      priv->tile_width = g_value_get_int (value);
      break;
    case PROP_TILE_HEIGHT:
      priv->tile_height = g_value_get_int (value);
      break;
    case PROP_MESH:
      {
        ClutterMesh * mesh = g_value_get_boxed (value);

        if (mesh)
          priv->mesh = *mesh;
        else
          priv->mesh.points = NULL;
      }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
clutter_texture_odo_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  ClutterTextureOdo *otex = CLUTTER_TEXTURE_ODO (object);
  ClutterTextureOdoPrivate * priv = otex->priv;

  switch (prop_id)
    {
    case PROP_PARENT_TEXTURE:
      g_value_set_object (value, otex->priv->parent_texture);
      break;
    case PROP_DISTORT_FUNC:
      g_value_set_pointer (value, priv->distort_func);
      break;
    case PROP_DISTORT_FUNC_DATA:
      g_value_set_pointer (value, priv->distort_func_data);
      break;
    case PROP_TILE_WIDTH:
      g_value_set_int (value, priv->tile_width);
      break;
    case PROP_TILE_HEIGHT:
      g_value_set_int (value, priv->tile_height);
      break;
    case PROP_MESH:
      g_value_set_boxed (value, &priv->mesh);

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
clutter_texture_odo_class_init (ClutterTextureOdoClass *klass)
{
  GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  actor_class->paint = clutter_texture_odo_paint;

  gobject_class->finalize     = clutter_texture_odo_finalize;
  gobject_class->dispose      = clutter_texture_odo_dispose;
  gobject_class->set_property = clutter_texture_odo_set_property;
  gobject_class->get_property = clutter_texture_odo_get_property;

  g_object_class_install_property (gobject_class,
                                     PROP_PARENT_TEXTURE,
                                   g_param_spec_object ("parent-texture",
                                                           "Parent Texture",
                                                        "The parent texture to clone",
                                                        CLUTTER_TYPE_TEXTURE,
                                                        (G_PARAM_CONSTRUCT | CLUTTER_PARAM_READWRITE)));

  g_object_class_install_property (gobject_class, PROP_DISTORT_FUNC,
                                   g_param_spec_pointer ("distort-func",
                                                         "distortion function",
                                                         "distortion function",
                                                         G_PARAM_CONSTRUCT | CLUTTER_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_DISTORT_FUNC_DATA,
                                   g_param_spec_pointer ("distort-func-data",
                                                         "Data for distortion function",
                                                         "Data for distortion function",
                                                         G_PARAM_CONSTRUCT | CLUTTER_PARAM_READWRITE));
  

  g_object_class_install_property (gobject_class,
                                   PROP_TILE_WIDTH,
                                   g_param_spec_int ("tile-width",
                                                "width of the tile to use",
                                                "width of the tile to use",
                                                0, G_MAXINT,
                                                0,
                                                G_PARAM_CONSTRUCT | CLUTTER_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_TILE_HEIGHT,
                                   g_param_spec_int ("tile-height",
                                                "height of the tile to use",
                                                "height of the tile to use",
                                                0, G_MAXINT,
                                                0,
                                                G_PARAM_CONSTRUCT | CLUTTER_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_MESH,
                                   g_param_spec_boxed ("mesh",
                                                       "Mesh",
                                                       "Mesh to lay texture over",
                                                       CLUTTER_TYPE_MESH,
                                                       G_PARAM_CONSTRUCT | CLUTTER_PARAM_READWRITE));
  
  g_type_class_add_private (gobject_class, sizeof (ClutterTextureOdoPrivate));
}

static void
clutter_texture_odo_init (ClutterTextureOdo *self)
{
  ClutterTextureOdoPrivate *priv;

  self->priv = priv = CLUTTER_TEXTURE_ODO_GET_PRIVATE (self);

  priv->tile_width = priv->tile_height = 8;
}

/**
 * clutter_texture_odo_new:
 * @texture: a #ClutterTexture or %NULL
 *
 * Creates an efficient 'clone' of a pre-existing texture if which it 
 * shares the underlying pixbuf data.
 *
 * You can use clutter_texture_odo_set_parent_texture() to change the
 * parent texture to be cloned.
 *
 * Return value: the newly created #ClutterTextureOdo
 */
ClutterActor *
clutter_texture_odo_new (ClutterTexture *texture)
{
  g_return_val_if_fail (texture == NULL || CLUTTER_IS_TEXTURE (texture), NULL);

  return g_object_new (CLUTTER_TYPE_TEXTURE_ODO,
                        "parent-texture", texture,
                       NULL);
}

/**
 * clutter_texture_odo_get_parent_texture:
 * @clone: a #ClutterTextureOdo
 * 
 * Retrieves the parent #ClutterTexture used by @clone.
 *
 * Return value: a #ClutterTexture actor, or %NULL
 *
 * Since: 0.2
 */
ClutterTexture *
clutter_texture_odo_get_parent_texture (ClutterTextureOdo *clone)
{
  g_return_val_if_fail (CLUTTER_IS_TEXTURE_ODO (clone), NULL);

  return clone->priv->parent_texture;
}

/**
 * clutter_texture_odo_set_parent_texture:
 * @clone: a #ClutterTextureOdo
 * @texture: a #ClutterTexture or %NULL
 *
 * Sets the parent texture cloned by the #ClutterTextureOdo.
 *
 * Since: 0.2
 */
void
clutter_texture_odo_set_parent_texture (ClutterTextureOdo *clone,
                                        ClutterTexture      *texture)
{
  g_return_if_fail (CLUTTER_IS_TEXTURE_ODO (clone));
  g_return_if_fail (texture == NULL || CLUTTER_IS_TEXTURE (texture));

  g_object_ref (clone);

  set_parent_texture (clone, texture);

  g_object_notify (G_OBJECT (clone), "parent-texture");
  g_object_unref (clone);
}

static ClutterMesh*
clutter_mesh_copy (const ClutterMesh *mesh)
{
  ClutterMesh *result = g_new (ClutterMesh, 1);

  *result = *mesh;

  return result;
}

GType
clutter_mesh_get_type (void)
{
  static GType our_type = 0;

  if (our_type == 0)
    our_type = g_boxed_type_register_static (g_intern_static_string ("ClutterMesh"),
                                             (GBoxedCopyFunc) clutter_mesh_copy,
                                             (GBoxedFreeFunc) g_free);

  return our_type;
}

static ClutterMeshPoint*
clutter_mesh_point_copy (const ClutterMeshPoint *point)
{
  ClutterMeshPoint *result = g_new (ClutterMeshPoint, 1);

  *result = *point;

  return result;
}

GType
clutter_mesh_point_get_type (void)
{
  static GType our_type = 0;

  if (our_type == 0)
    our_type = g_boxed_type_register_static (g_intern_static_string ("ClutterMeshPoint"),
                                             (GBoxedCopyFunc) clutter_mesh_point_copy,
                                             (GBoxedFreeFunc) g_free);

  return our_type;
}

void
clutter_texture_odo_set_cull_mode (ClutterTextureOdo   *otex,
                                   ClutterTextureOdoCullMode mode)
{
  g_return_if_fail (CLUTTER_IS_TEXTURE_ODO (otex));
  otex->priv->cull_mode = mode;
  clutter_actor_queue_redraw (CLUTTER_ACTOR (otex));
}

ClutterTextureOdoCullMode
clutter_texture_odo_get_cull_mode (ClutterTextureOdo   *otex)
{
  g_return_val_if_fail (CLUTTER_IS_TEXTURE_ODO (otex), ODO_CULL_NONE);
  return otex->priv->cull_mode;
}

