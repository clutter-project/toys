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


#include "astro-applet.h"

#include <libastro-desktop/tidy-texture-frame.h>

#include <libastro-desktop/astro-defines.h>

G_DEFINE_TYPE (AstroApplet, astro_applet, CLUTTER_TYPE_GROUP);

#define ASTRO_APPLET_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
        ASTRO_TYPE_APPLET, AstroAppletPrivate))

static GdkPixbuf    *applet_bg = NULL;
static ClutterActor *texture = NULL;
	
struct _AstroAppletPrivate
{
  ClutterActor *texture;
};

/* GObject stuff */
static void
astro_applet_paint (ClutterActor *applet)
{
  AstroAppletPrivate *priv;
  GList *c;

  g_return_if_fail (ASTRO_IS_APPLET (applet));
  priv = ASTRO_APPLET (applet)->priv;

  clutter_actor_set_size (priv->texture,
                          clutter_actor_get_width (applet),
                          clutter_actor_get_height (applet));

  c = clutter_container_get_children (CLUTTER_CONTAINER (applet));
  for (c = c; c; c = c->next)
    clutter_actor_paint (c->data);
    
}

static void
astro_applet_class_init (AstroAppletClass *klass)
{ 
  GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  actor_class->paint = astro_applet_paint;
  
  g_type_class_add_private (gobject_class, sizeof (AstroAppletPrivate));
}

static void
astro_applet_init (AstroApplet *applet)
{
  AstroAppletPrivate *priv;
  
  priv = applet->priv = ASTRO_APPLET_GET_PRIVATE (applet);

  if (!CLUTTER_IS_TEXTURE (texture))
    {
      applet_bg = gdk_pixbuf_new_from_file (PKGDATADIR "/applet_bg.png", NULL);
      texture = g_object_new (CLUTTER_TYPE_TEXTURE,
                              "pixbuf", applet_bg,
                              "tiled", FALSE,
                              NULL);

    }
  
  priv->texture = tidy_texture_frame_new (CLUTTER_TEXTURE (texture), 
                                          15, 15, 15, 15);
  clutter_container_add_actor (CLUTTER_CONTAINER (applet), priv->texture);
  
  clutter_actor_show_all (CLUTTER_ACTOR (applet));
}

ClutterActor * 
astro_applet_new (void)
{
  AstroApplet *applet =  g_object_new (ASTRO_TYPE_APPLET,
									                     NULL);

  return CLUTTER_ACTOR (applet);
}

