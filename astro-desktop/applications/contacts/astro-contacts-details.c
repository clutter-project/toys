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

static GdkPixbuf *contact_bar_pixbuf = NULL;
static GdkPixbuf *photo_bg_pixbuf    = NULL;

struct _AstroContactDetailsPrivate
{
  ClutterActor *photo_bg;
  ClutterActor *photo;
  GdkPixbuf    *photo_pixbuf;

  ClutterActor *contact_bar;

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
    
  g_return_if_fail (ASTRO_IS_CONTACT_DETAILS (details));
  priv = details->priv;


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
      g_value_set_object (value, G_OBJECT (priv->photo_pixbuf));
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

  if (!contact_bar_pixbuf)
    contact_bar_pixbuf = gdk_pixbuf_new_from_file (PKGDATADIR"/contact-bar.svg",
                                                   NULL);
  if (!photo_bg_pixbuf)
    photo_bg_pixbuf = gdk_pixbuf_new_from_file (PKGDATADIR"/applet_bg.png",
                                                NULL);

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

