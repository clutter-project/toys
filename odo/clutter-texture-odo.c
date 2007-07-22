/*
 * Clutter.
 *
 * An OpenGL based 'interactive canvas' library.
 *
 * Authored By Tomas Frydrych  <tf@openedhand.com>
 *
 * Copyright (C) 2007 OpenedHand
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

/**
 * SECTION:clutter-clone-texture
 * @short_description: Actor for cloning existing textures in an 
 * efficient way.
 *
 * #ClutterTextureOdo allows the cloning of existing #ClutterTexture based
 * actors whilst saving underlying graphics resources.
 */

#include <clutter/clutter.h>
#include <clutter/cogl.h>

#include "clutter-texture-odo.h"

enum
{
  PROP_0,
  PROP_PARENT_TEXTURE,
  PROP_DISTORT_FUNC,
  PROP_DISTORT_FUNC_DATA,
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
};

static void
texture_odo_render_to_gl_quad (ClutterTextureOdo *otex, 
			       int             x1, 
			       int             y1, 
			       int             x2, 
			       int             y2)
{
  gint   qx1 = 0, qx2 = 0, qy1 = 0, qy2 = 0;
  gint   qwidth = 0, qheight = 0;
  gint   x, y, i =0, lastx = 0, lasty = 0;
  gint   n_x_tiles, n_y_tiles; 
  gint   pwidth, pheight, pwidth2, pheight2;
  float tx, ty;

  ClutterTextureOdoPrivate *priv = otex->priv;
  ClutterActor *parent_actor = CLUTTER_ACTOR (priv->parent_texture);

  priv = otex->priv;

  qwidth  = x2 - x1;
  qheight = y2 - y1;

  if (!CLUTTER_ACTOR_IS_REALIZED (parent_actor))
      clutter_actor_realize (parent_actor);

  /* Only paint if parent is in a state to do so */
  if (!clutter_texture_has_generated_tiles (priv->parent_texture))
    return;

  clutter_texture_get_base_size (priv->parent_texture, &pwidth, &pheight); 

  if (!clutter_feature_available (CLUTTER_FEATURE_TEXTURE_RECTANGLE))
    {
      pwidth2 = clutter_util_next_p2 (pwidth);
      pheight2 = clutter_util_next_p2 (pheight);
    }
  
  if (!clutter_texture_is_tiled (priv->parent_texture))
    {
      clutter_texture_bind_tile (priv->parent_texture, 0);

      /* NPOTS textures *always* used if extension available
       */
      if (clutter_feature_available (CLUTTER_FEATURE_TEXTURE_RECTANGLE))
	{
	  tx = (float) pwidth;
	  ty = (float) pheight;
	}
      else
	{
	  tx = (float) pwidth / (float) pwidth2;  
	  ty = (float) pheight / (float) pheight2;
	}

      if (!priv->distort_func)
	{
	  cogl_texture_quad (x1, x2, y1, y2, 
			     0,
			     0,
			     CLUTTER_FLOAT_TO_FIXED (tx),
			     CLUTTER_FLOAT_TO_FIXED (ty));
	}
      else
	{
	  guint i, j;
	  gfloat txf, tyf1, tyf2;
	  
	  glShadeModel(GL_SMOOTH);

	  for (j = 0; j < pheight - 1; j++)
	    {
	      glBegin(GL_QUAD_STRIP);
	      
	      if (clutter_feature_available(CLUTTER_FEATURE_TEXTURE_RECTANGLE))
		{
		  tyf1 = (float) j;
		  tyf2 = (float) (j+1);
		}
	      else
		{
		  tyf1 = (float)j / (float) pheight2;
		  tyf2 = (float)(j+1) / (float) pheight2;
		}
	      
	      for (i = 0; i < pwidth; i++)
		{
		  gint x2, y2, z2;

		    if (clutter_feature_available(CLUTTER_FEATURE_TEXTURE_RECTANGLE))
		    {
		      txf  = (float) i;
		    }
		  else
		    {
		      txf = (float)i / (float) pwidth2;  
		    }
		  
		  priv->distort_func (priv->parent_texture,
				      i, j, 0, &x2, &y2, &z2,
				      priv->distort_func_data);
		  
		  glTexCoord2f (txf, tyf1);
		  glVertex3i(x2, y2, z2);
		  
		  priv->distort_func (priv->parent_texture,
				      i, j + 1, 0, &x2, &y2, &z2,
				      priv->distort_func_data);
		  
		  glTexCoord2f (txf, tyf2);
		  glVertex3i(x2, y2, z2);
		}
	      glEnd();
	  }
	}

      return;
    }

  /* FIXME */
  clutter_texture_get_n_tiles (priv->parent_texture, &n_x_tiles, &n_y_tiles); 

  for (x = 0; x < n_x_tiles; x++)
    {
      lasty = 0;

      for (y = 0; y < n_y_tiles; y++)
	{
	  gint actual_w, actual_h;
	  gint xpos, ypos, xsize, ysize, ywaste, xwaste;
	  
	  clutter_texture_bind_tile (priv->parent_texture, i);
	 
	  clutter_texture_get_x_tile_detail (priv->parent_texture, 
					     x, &xpos, &xsize, &xwaste);

	  clutter_texture_get_y_tile_detail (priv->parent_texture, 
					     y, &ypos, &ysize, &ywaste);

	  actual_w = xsize - xwaste;
	  actual_h = ysize - ywaste;

	  tx = (float) actual_w / xsize;
	  ty = (float) actual_h / ysize;

	  qx1 = x1 + lastx;
	  qx2 = qx1 + ((qwidth * actual_w ) / pwidth );
	  
	  qy1 = y1 + lasty;
	  qy2 = qy1 + ((qheight * actual_h) / pheight );

	  cogl_texture_quad (qx1, qx2, qy1, qy2, 
			     0,
			     0,
			     CLUTTER_FLOAT_TO_FIXED (tx),
			     CLUTTER_FLOAT_TO_FIXED (ty));

	  lasty += qy2 - qy1;	  

	  i++;
	}
      lastx += qx2 - qx1;
    }
}

static void
clutter_texture_odo_paint (ClutterActor *self)
{
  ClutterTextureOdoPrivate  *priv;
  ClutterActor                *parent_texture;
  gint                         x1, y1, x2, y2;
  GLenum                       target_type;
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

  /* FIXME: figure out nicer way of getting at this info...  
   */  
  if (clutter_feature_available (CLUTTER_FEATURE_TEXTURE_RECTANGLE) &&
      clutter_texture_is_tiled (CLUTTER_TEXTURE (parent_texture)) == FALSE)
    {
      target_type = CGL_TEXTURE_RECTANGLE_ARB;
      cogl_enable (CGL_ENABLE_TEXTURE_RECT|CGL_ENABLE_BLEND);
    }
  else
    {
      target_type = CGL_TEXTURE_2D;
      cogl_enable (CGL_ENABLE_TEXTURE_2D|CGL_ENABLE_BLEND);
    }

  col.alpha = clutter_actor_get_opacity (self);
  cogl_color (&col);

  clutter_actor_get_coords (self, &x1, &y1, &x2, &y2);

  /* Parent paint translated us into position */
  texture_odo_render_to_gl_quad (CLUTTER_TEXTURE_ODO (self), 
				   0, 0, x2 - x1, y2 - y1);

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
  
  
  g_type_class_add_private (gobject_class, sizeof (ClutterTextureOdoPrivate));
}

static void
clutter_texture_odo_init (ClutterTextureOdo *self)
{
  ClutterTextureOdoPrivate *priv;

  self->priv = priv = CLUTTER_TEXTURE_ODO_GET_PRIVATE (self);
  priv->parent_texture = NULL;
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
