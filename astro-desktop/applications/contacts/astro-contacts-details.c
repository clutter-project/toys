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


#include "astro-contacts-details.h"

#include <libastro-desktop/astro-defines.h>
#include <libastro-desktop/astro-utils.h>

#include "clutter-reflect-texture.h"

G_DEFINE_TYPE (AstroContactDetails, astro_contact_details, CLUTTER_TYPE_GROUP);

#define ASTRO_CONTACT_DETAILS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
        ASTRO_TYPE_CONTACT_DETAILS, AstroContactDetailsPrivate))

/*static GdkPixbuf *photo_bg = NULL;*/

struct _AstroContactDetailsPrivate
{
  ClutterActor *details;
  ClutterActor *details_reflect;
  ClutterActor *texture;
  ClutterActor *reflect;
  GdkPixbuf    *pixbuf;

  ClutterEffectTemplate *details_temp;
  ClutterTimeline       *details_time;
};

enum
{
  PROP_0,

  PROP_PIXBUF
};

void
astro_contact_details_set_pixbuf (AstroContactDetails *details,
                                  GdkPixbuf           *pixbuf)
{
  AstroContactDetailsPrivate *priv;
  gint height;
  
  g_return_if_fail (ASTRO_IS_CONTACT_DETAILS (details));
  priv = details->priv;

  if (CLUTTER_IS_ACTOR (priv->texture))
    clutter_actor_destroy (priv->texture);
  
  if (CLUTTER_IS_ACTOR (priv->reflect))
    clutter_actor_destroy (priv->reflect);

  height = gdk_pixbuf_get_height (pixbuf);
    
  /* Album cover */
  priv->texture = g_object_new (CLUTTER_TYPE_TEXTURE,
                                "pixbuf", pixbuf,
                                "tiled", FALSE,
                                NULL);

  clutter_container_add_actor (CLUTTER_CONTAINER (details),
                               priv->texture);
  clutter_actor_set_position (priv->texture, 0, 0);
  
  priv->reflect = clutter_reflect_texture_new (CLUTTER_TEXTURE (priv->texture),
                                               height * 0.7);
  clutter_actor_set_opacity (priv->reflect, 100);
  clutter_container_add_actor (CLUTTER_CONTAINER (details), 
                               priv->reflect);
  clutter_actor_set_position (priv->reflect, 0, height+1);
  
  clutter_actor_set_anchor_point (CLUTTER_ACTOR (details),
                                  clutter_actor_get_width (priv->texture)/2,
                                  height/2);

  clutter_actor_show_all (CLUTTER_ACTOR (details));
}

/* GObject stuff */
static void
astro_contact_details_set_property (GObject      *object, 
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  AstroContactDetailsPrivate *priv;
  
  g_return_if_fail (ASTRO_IS_CONTACT_DETAILS (object));
  priv = ASTRO_CONTACT_DETAILS (object)->priv;

  switch (prop_id)
  {
    case PROP_PIXBUF:
      astro_contact_details_set_pixbuf (ASTRO_CONTACT_DETAILS (object),
                                   GDK_PIXBUF (g_value_get_object (value)));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
astro_contact_details_get_property (GObject    *object, 
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AstroContactDetailsPrivate *priv;
  
  g_return_if_fail (ASTRO_IS_CONTACT_DETAILS (object));
  priv = ASTRO_CONTACT_DETAILS (object)->priv;

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
astro_contact_details_class_init (AstroContactDetailsClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = astro_contact_details_set_property;
  gobject_class->get_property = astro_contact_details_get_property;

  g_object_class_install_property (
    gobject_class,
    PROP_PIXBUF,
    g_param_spec_object ("pixbuf",
                         "Pixbuf",
                         "A pixbuf",
                         GDK_TYPE_PIXBUF,
                         G_PARAM_READWRITE));

  g_type_class_add_private (gobject_class, sizeof (AstroContactDetailsPrivate));
}

static void
astro_contact_details_init (AstroContactDetails *details)
{
  AstroContactDetailsPrivate *priv;
  priv = details->priv = ASTRO_CONTACT_DETAILS_GET_PRIVATE (details);

  priv->texture = NULL;
  priv->reflect = NULL;
  
  priv->details_time = clutter_timeline_new_for_duration (600);
  priv->details_temp = clutter_effect_template_new (priv->details_time, 
                                                    clutter_sine_inc_func);
}

ClutterActor *
astro_contact_details_new (void)
{
  ClutterActor *details =  g_object_new (ASTRO_TYPE_CONTACT_DETAILS,
                                                 NULL);
  return CLUTTER_ACTOR (details);
}

