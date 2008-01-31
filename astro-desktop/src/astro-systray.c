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


#include "astro-systray.h"

#include <time.h>
#include <libastro-desktop/astro-defines.h>

G_DEFINE_TYPE (AstroSystray, astro_systray, CLUTTER_TYPE_GROUP);

#define ASTRO_SYSTRAY_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
        ASTRO_TYPE_SYSTRAY, AstroSystrayPrivate))

#define PADDING 10

struct _AstroSystrayPrivate
{
  ClutterActor *bt;
  ClutterActor *nm;
  ClutterActor *time;
};

static gboolean 
set_time (AstroSystray *systray)
{
  AstroSystrayPrivate *priv;
  time_t rawtime;
  struct tm *timeinfo;
  char buffer [100];

  g_return_val_if_fail (ASTRO_IS_SYSTRAY (systray), FALSE);
  priv = systray->priv;

  time (&rawtime);
  timeinfo = localtime (&rawtime);

  strftime (buffer, 100, "%a %d %b,%H:%M   ", timeinfo);
  
  clutter_label_set_text (CLUTTER_LABEL (priv->time), buffer);
  
  return TRUE;
}

/* GObject stuff */
static void
astro_systray_class_init (AstroSystrayClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (gobject_class, sizeof (AstroSystrayPrivate));
}

static void
astro_systray_init (AstroSystray *systray)
{
  AstroSystrayPrivate *priv;
  GdkPixbuf *pixbuf;
  ClutterColor white = { 0xff, 0xff, 0xff, 0xff };
  gint width;
  gchar *font;
  
  priv = systray->priv = ASTRO_SYSTRAY_GET_PRIVATE (systray);

  pixbuf = gdk_pixbuf_new_from_file (PKGDATADIR"/icons/bt.png", NULL);
  if (pixbuf)
    {
      priv->bt = clutter_texture_new_from_pixbuf (pixbuf);
      clutter_container_add_actor (CLUTTER_CONTAINER (systray), priv->bt);
      clutter_actor_set_anchor_point_from_gravity (priv->bt,
                                                   CLUTTER_GRAVITY_CENTER);
      clutter_actor_set_position (priv->bt, 0, ASTRO_PANEL_HEIGHT ()/2);
    }
  
  pixbuf = gdk_pixbuf_new_from_file (PKGDATADIR"/icons/nm.png", NULL);
  if (pixbuf)
    {
      priv->nm = clutter_texture_new_from_pixbuf (pixbuf);
      clutter_container_add_actor (CLUTTER_CONTAINER (systray), priv->nm);
      clutter_actor_set_anchor_point_from_gravity (priv->nm,
                                                   CLUTTER_GRAVITY_WEST);
      clutter_actor_set_position (priv->nm, 
                                  clutter_actor_get_width (priv->bt) + PADDING,
                                  ASTRO_PANEL_HEIGHT () /2);
    }

  width = clutter_actor_get_width (CLUTTER_ACTOR (systray));

  /* Time date */
  font = g_strdup_printf ("Sans %d", (int)(ASTRO_PANEL_HEIGHT () * 0.4));
  priv->time = clutter_label_new_full (font, "   ", &white);
  clutter_label_set_line_wrap (CLUTTER_LABEL (priv->time), FALSE);
  clutter_container_add_actor (CLUTTER_CONTAINER (systray), priv->time);
  clutter_actor_set_anchor_point_from_gravity (priv->time,CLUTTER_GRAVITY_WEST);
  set_time (systray);
  clutter_actor_set_position (priv->time, width + PADDING,
                              ASTRO_PANEL_HEIGHT ()/2);

  g_timeout_add (1000, (GSourceFunc)set_time, systray);
  g_free (font);

  clutter_actor_show_all (CLUTTER_ACTOR (systray));
}

ClutterActor * 
astro_systray_new (void)
{
  AstroSystray *systray =  g_object_new (ASTRO_TYPE_SYSTRAY,
									                       NULL);

  return CLUTTER_ACTOR (systray);
}

