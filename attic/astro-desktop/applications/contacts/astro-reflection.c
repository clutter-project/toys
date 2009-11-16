/*
 * Copyright (C) 2007 OpenedHand Limited
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Neil Jagdish Patel <njp@o-hand.com>
 */


#include "astro-reflection.h"

#include <libastro-desktop/astro-defines.h>
#include <libastro-desktop/astro-utils.h>

#include "clutter-reflect-texture.h"

G_DEFINE_TYPE (AstroReflection, astro_reflection, CLUTTER_TYPE_GROUP);

#define ASTRO_REFLECTION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
        ASTRO_TYPE_REFLECTION, AstroReflectionPrivate))

static GdkPixbuf *disc_bg = NULL;

struct _AstroReflectionPrivate
{
  ClutterActor *songs;
  ClutterActor *songs_reflect;
  ClutterActor *texture;
  ClutterActor *reflect;
  GdkPixbuf    *pixbuf;

  ClutterEffectTemplate *songs_temp;
  ClutterTimeline       *songs_time;
};

enum
{
  PROP_0,

  PROP_PIXBUF
};

static void
fix_clip (ClutterTimeline *timeline,
          gint             frame_num,
          AstroReflection *reflection)
{
  AstroReflectionPrivate *priv;
  gint size;
   
  g_return_if_fail (ASTRO_IS_REFLECTION (reflection));
  priv = reflection->priv;

  size = clutter_actor_get_width (priv->songs);

  astro_utils_set_clip (priv->songs_reflect,
                        size - clutter_actor_get_x (priv->songs_reflect),
                        0, size, size);
}

void
astro_reflection_set_active (AstroReflection *reflection,
                             gboolean         active)
{
  AstroReflectionPrivate *priv;
  static ClutterTimeline *fade_time = NULL;
  gint x = 0;
  gint fade = 0;
   
  g_return_if_fail (ASTRO_IS_REFLECTION (reflection));
  priv = reflection->priv;

  if (active)
  {
    x = clutter_actor_get_width (priv->texture); 
    fade = 100;
  }
  
  clutter_effect_move (priv->songs_temp,
                       priv->songs,
                       x, clutter_actor_get_y (priv->songs),
                       NULL, NULL);
  clutter_effect_move (priv->songs_temp,
                       priv->songs_reflect,
                       x, clutter_actor_get_y (priv->songs_reflect),
                       NULL, NULL);

  fade_time = clutter_effect_fade (priv->songs_temp,
                                   priv->songs_reflect,
                                   fade,
                                   NULL, NULL);
  g_signal_connect (fade_time, "new-frame",
                    G_CALLBACK (fix_clip), reflection);

}

void
astro_reflection_set_pixbuf (AstroReflection *reflection,
                             GdkPixbuf       *pixbuf)
{
  AstroReflectionPrivate *priv;
  gint height;
  
  g_return_if_fail (ASTRO_IS_REFLECTION (reflection));
  priv = reflection->priv;

  if (CLUTTER_IS_ACTOR (priv->texture))
    clutter_actor_destroy (priv->texture);
  
  if (CLUTTER_IS_ACTOR (priv->reflect))
    clutter_actor_destroy (priv->reflect);

  height = gdk_pixbuf_get_height (pixbuf);

  /* Songs widget */
  if (!disc_bg)
    {
      disc_bg = gdk_pixbuf_new_from_file_at_size (PKGDATADIR"/disc_bg.svg",
                                                  height, height, NULL);
    }
  priv->songs = clutter_texture_new_from_pixbuf (disc_bg);
  clutter_container_add_actor (CLUTTER_CONTAINER (reflection), priv->songs);
  clutter_actor_set_size (priv->songs, height, height);
  clutter_actor_set_position (priv->songs, 0, 0);

  priv->songs_reflect = clutter_reflect_texture_new (CLUTTER_TEXTURE (priv->songs),
                                               height * 0.7);
  clutter_actor_set_opacity (priv->songs_reflect, 0);
  clutter_container_add_actor (CLUTTER_CONTAINER (reflection), 
                               priv->songs_reflect);
  clutter_actor_set_position (priv->songs_reflect, 0, height+1);
     
  /* Album cover */
  priv->texture = g_object_new (CLUTTER_TYPE_TEXTURE,
                                "pixbuf", pixbuf,
                                "tiled", FALSE,
                                NULL);

  clutter_container_add_actor (CLUTTER_CONTAINER (reflection),
                               priv->texture);
  clutter_actor_set_position (priv->texture, 0, 0);
  
  priv->reflect = clutter_reflect_texture_new (CLUTTER_TEXTURE (priv->texture),
                                               height * 0.7);
  clutter_actor_set_opacity (priv->reflect, 100);
  clutter_container_add_actor (CLUTTER_CONTAINER (reflection), 
                               priv->reflect);
  clutter_actor_set_position (priv->reflect, 0, height+1);
  
  clutter_actor_set_anchor_point (CLUTTER_ACTOR (reflection),
                                  clutter_actor_get_width (priv->texture)/2,
                                  height/2);

  clutter_actor_show_all (CLUTTER_ACTOR (reflection));
}

/* GObject stuff */
static void
astro_reflection_set_property (GObject      *object, 
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  AstroReflectionPrivate *priv;
  
  g_return_if_fail (ASTRO_IS_REFLECTION (object));
  priv = ASTRO_REFLECTION (object)->priv;

  switch (prop_id)
  {
    case PROP_PIXBUF:
      astro_reflection_set_pixbuf (ASTRO_REFLECTION (object),
                                   GDK_PIXBUF (g_value_get_object (value)));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
astro_reflection_get_property (GObject    *object, 
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AstroReflectionPrivate *priv;
  
  g_return_if_fail (ASTRO_IS_REFLECTION (object));
  priv = ASTRO_REFLECTION (object)->priv;

  switch (prop_id)
  {
    case PROP_PIXBUF:
      g_value_set_object (value, G_OBJECT (priv->pixbuf));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
astro_reflection_class_init (AstroReflectionClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = astro_reflection_set_property;
  gobject_class->get_property = astro_reflection_get_property;

  g_object_class_install_property (
    gobject_class,
    PROP_PIXBUF,
    g_param_spec_object ("pixbuf",
                         "Pixbuf",
                         "A pixbuf",
                         GDK_TYPE_PIXBUF,
                         G_PARAM_READWRITE));

  g_type_class_add_private (gobject_class, sizeof (AstroReflectionPrivate));
}

static void
astro_reflection_init (AstroReflection *reflection)
{
  AstroReflectionPrivate *priv;
  priv = reflection->priv = ASTRO_REFLECTION_GET_PRIVATE (reflection);

  priv->texture = NULL;
  priv->reflect = NULL;

  priv->songs_time = clutter_timeline_new_for_duration (600);
  priv->songs_temp = clutter_effect_template_new (priv->songs_time, 
                                                  clutter_sine_inc_func);
}

ClutterActor *
astro_reflection_new (GdkPixbuf *pixbuf)
{
  ClutterActor *reflection =  g_object_new (ASTRO_TYPE_REFLECTION,
                                            "pixbuf", pixbuf,
                                            NULL);
  return CLUTTER_ACTOR (reflection);
}

