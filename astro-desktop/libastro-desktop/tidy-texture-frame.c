/* tidy-texture-frame.h: Expandible texture actor
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
 * SECTION:tidy-texture-frame
 * @short_description: Actor for cloning existing textures in an 
 * efficient way.
 *
 * #TidyTextureFrame
 *
 */

#include <clutter/cogl.h>

#include "tidy-texture-frame.h"
#include "tidy-private.h"

enum
{
  PROP_0,
  PROP_LEFT,
  PROP_TOP,
  PROP_RIGHT,
  PROP_BOTTOM
};

G_DEFINE_TYPE (TidyTextureFrame,
	       tidy_texture_frame,
	       CLUTTER_TYPE_CLONE_TEXTURE);

#define TIDY_TEXTURE_FRAME_GET_PRIVATE(obj) \
(G_TYPE_INSTANCE_GET_PRIVATE ((obj), TIDY_TYPE_TEXTURE_FRAME, TidyTextureFramePrivate))

struct _TidyTextureFramePrivate
{
  gint left, top, right, bottom;
};

static void
tidy_texture_frame_paint (ClutterActor *self)
{
  TidyTextureFramePrivate    *priv;
  ClutterActor                *parent_texture;
  guint                        width, height;
  gint                         pwidth, pheight, ex, ey;
  ClutterFixed                 tx1, ty1, tx2, ty2, tw, th;
  GLenum                       target_type;
  ClutterColor                 col = { 0xff, 0xff, 0xff, 0xff };

  priv = TIDY_TEXTURE_FRAME (self)->priv;

  /* no need to paint stuff if we don't have a texture to reflect */
  if (!clutter_clone_texture_get_parent_texture (CLUTTER_CLONE_TEXTURE(self)))
    return;

  /* parent texture may have been hidden, there for need to make sure its 
   * realised with resources available.  
  */
  parent_texture 
    = CLUTTER_ACTOR 
       (clutter_clone_texture_get_parent_texture(CLUTTER_CLONE_TEXTURE(self)));
  if (!CLUTTER_ACTOR_IS_REALIZED (parent_texture))
    clutter_actor_realize (parent_texture);

  if (clutter_texture_is_tiled (CLUTTER_TEXTURE (parent_texture)))
    {
      g_warning("tiled textures not yet supported...");
      return;
    }

  cogl_push_matrix ();

#define FX(x) CLUTTER_INT_TO_FIXED(x)

  clutter_texture_get_base_size (CLUTTER_TEXTURE(parent_texture), 
				 &pwidth, &pheight); 
  clutter_actor_get_size (self, &width, &height); 

  tx1 = FX (priv->left);
  tx2 = FX (pwidth - priv->right);
  ty1 = FX (priv->top);
  ty2 = FX (pheight - priv->bottom);
  tw = FX (pwidth);
  th = FX (pheight);

  if (clutter_feature_available (CLUTTER_FEATURE_TEXTURE_RECTANGLE))
    {
      target_type = CGL_TEXTURE_RECTANGLE_ARB;
      cogl_enable (CGL_ENABLE_TEXTURE_RECT|CGL_ENABLE_BLEND);
    }
  else
    {
      target_type = CGL_TEXTURE_2D;
      cogl_enable (CGL_ENABLE_TEXTURE_2D|CGL_ENABLE_BLEND);

      tw = clutter_util_next_p2 (pwidth);
      th = clutter_util_next_p2 (pheight);

      tx1 = tx1/tw;
      tx2 = tx2/tw;
      ty1 = ty1/th;
      ty2 = ty2/th;
      tw = FX(pwidth)/tw;
      th = FX(pheight)/th;
    }

  col.alpha = clutter_actor_get_opacity (self);
  cogl_color (&col);

  clutter_texture_bind_tile (CLUTTER_TEXTURE(parent_texture), 0);

  ex = width - priv->right;
  if (ex < 0) 
    ex = priv->right; 		/* FIXME */

  ey = height - priv->bottom;
  if (ey < 0) 
    ey = priv->bottom; 		/* FIXME */

  /* top left corner */
  cogl_texture_quad (0, 
		     priv->left, /* FIXME: clip if smaller */
		     0,
		     priv->top,
		     0,
		     0,
		     tx1,
		     ty1);

  /* top middle */
  cogl_texture_quad (priv->left,
		     ex,
		     0,
		     priv->top,
		     tx1,
		     0,
		     tx2,
		     ty1);

  /* top right */
  cogl_texture_quad (ex,
		     width,
		     0,
		     priv->top,
		     tx2,
		     0,
		     tw,
		     ty1);

  /* mid left */
  cogl_texture_quad (0, 
		     priv->left,
		     priv->top,
		     ey,
		     0,
		     ty1,
		     tx1,
		     ty2);

  /* center */
  cogl_texture_quad (priv->left,
		     ex,
		     priv->top,
		     ey,
		     tx1,
		     ty1,
		     tx2,
		     ty2);

  /* mid right */
  cogl_texture_quad (ex,
		     width,
		     priv->top,
		     ey,
		     tx2,
		     ty1,
		     tw,
		     ty2);

  /* bottom left */
  cogl_texture_quad (0, 
		     priv->left,
		     ey,
		     height,
		     0,
		     ty2,
		     tx1,
		     th);

  /* bottom center */
  cogl_texture_quad (priv->left,
		     ex,
		     ey,
		     height,
		     tx1,
		     ty2,
		     tx2,
		     th);

  /* bottom right */
  cogl_texture_quad (ex,
		     width,
		     ey,
		     height,
		     tx2,
		     ty2,
		     tw,
		     th);

  cogl_pop_matrix ();

#undef FX
}


static void
tidy_texture_frame_set_property (GObject      *object,
				    guint         prop_id,
				    const GValue *value,
				    GParamSpec   *pspec)
{
  TidyTextureFrame         *ctexture = TIDY_TEXTURE_FRAME (object);
  TidyTextureFramePrivate  *priv = ctexture->priv;  

  switch (prop_id)
    {
    case PROP_LEFT:
      priv->left = g_value_get_int (value);
      break;
    case PROP_TOP:
      priv->top = g_value_get_int (value);
      break;
    case PROP_RIGHT:
      priv->right = g_value_get_int (value);
      break;
    case PROP_BOTTOM:
      priv->bottom = g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
tidy_texture_frame_get_property (GObject    *object,
				    guint       prop_id,
				    GValue     *value,
				    GParamSpec *pspec)
{
  TidyTextureFrame *ctexture = TIDY_TEXTURE_FRAME (object);
  TidyTextureFramePrivate  *priv = ctexture->priv;  

  switch (prop_id)
    {
    case PROP_LEFT:
      g_value_set_int (value, priv->left);
      break;
    case PROP_TOP:
      g_value_set_int (value, priv->top);
      break;
    case PROP_RIGHT:
      g_value_set_int (value, priv->right);
      break;
    case PROP_BOTTOM:
      g_value_set_int (value, priv->bottom);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
tidy_texture_frame_class_init (TidyTextureFrameClass *klass)
{
  GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  actor_class->paint = tidy_texture_frame_paint;

  gobject_class->set_property = tidy_texture_frame_set_property;
  gobject_class->get_property = tidy_texture_frame_get_property;

  g_object_class_install_property 
            (gobject_class,
	     PROP_LEFT,
	     g_param_spec_int ("left",
			       "left",
			       "",
			       0, G_MAXINT,
			       0,
			       TIDY_PARAM_READWRITE));

  g_object_class_install_property 
            (gobject_class,
	     PROP_TOP,
	     g_param_spec_int ("top",
			       "top",
			       "",
			       0, G_MAXINT,
			       0,
			       TIDY_PARAM_READWRITE));

  g_object_class_install_property 
            (gobject_class,
	     PROP_BOTTOM,
	     g_param_spec_int ("bottom",
			       "bottom",
			       "",
			       0, G_MAXINT,
			       0,
			       TIDY_PARAM_READWRITE));
  
  g_object_class_install_property 
            (gobject_class,
	     PROP_RIGHT,
	     g_param_spec_int ("right",
			       "right",
			       "",
			       0, G_MAXINT,
			       0,
			       TIDY_PARAM_READWRITE));
  
  g_type_class_add_private (gobject_class, sizeof (TidyTextureFramePrivate));
}

static void
tidy_texture_frame_init (TidyTextureFrame *self)
{
  TidyTextureFramePrivate *priv;

  self->priv = priv = TIDY_TEXTURE_FRAME_GET_PRIVATE (self);
}

/**
 * tidy_texture_frame_new:
 * @texture: a #ClutterTexture or %NULL
 *
 * FIXME
 *
 * Return value: the newly created #TidyTextureFrame
 */
ClutterActor*
tidy_texture_frame_new (ClutterTexture *texture, 
			gint            left,
			gint            top,
			gint            right,
			gint            bottom)
{
  g_return_val_if_fail (texture == NULL || CLUTTER_IS_TEXTURE (texture), NULL);

  return g_object_new (TIDY_TYPE_TEXTURE_FRAME,
 		       "parent-texture", texture,
		       "left", left,
		       "top", top,
		       "right", right,
		       "bottom", bottom,
		       NULL);
}

