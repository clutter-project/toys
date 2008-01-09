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


#include "astro-panel.h"

#include <libastro-desktop/astro-defines.h>

#include "astro-systray.h"

G_DEFINE_TYPE (AstroPanel, astro_panel, CLUTTER_TYPE_GROUP);

#define ASTRO_PANEL_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
        ASTRO_TYPE_PANEL, AstroPanelPrivate))

#define PADDING 4
	
struct _AstroPanelPrivate
{
  ClutterActor *panel_bg;
  ClutterActor *home;
  ClutterActor *title;
  ClutterActor *systray;
  ClutterActor *close;
};

enum
{
  SHOW_HOME,
  CLOSE_WINDOW,

  LAST_SIGNAL
};
static guint _panel_signals[LAST_SIGNAL] = { 0 };


/* Public Functions */

/* Callbacks */
static gboolean
on_home_clicked (ClutterActor *home, ClutterEvent *event, AstroPanel *panel)
{
  g_debug ("home button clicked");

  g_signal_emit (panel, _panel_signals[SHOW_HOME], 0);
  return FALSE;
}

static gboolean
on_close_clicked (ClutterActor *home, ClutterEvent *event, AstroPanel *panel)
{
  g_debug ("close button clicked");

  g_signal_emit (panel, _panel_signals[CLOSE_WINDOW], 0);
  return FALSE;
}


/* GObject stuff */
static void
astro_panel_class_init (AstroPanelClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);

  _panel_signals[SHOW_HOME] = 
    g_signal_new ("show-home",
                  G_OBJECT_CLASS_TYPE (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (AstroPanelClass, show_home),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  _panel_signals[CLOSE_WINDOW] = 
    g_signal_new ("close-window",
                  G_OBJECT_CLASS_TYPE (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (AstroPanelClass, close_window),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  g_type_class_add_private (gobject_class, sizeof (AstroPanelPrivate));
}

static void
astro_panel_init (AstroPanel *panel)
{
  AstroPanelPrivate *priv;
  ClutterColor white = { 0xff, 0xff, 0xff, 0xff };
  GdkPixbuf *pixbuf;
  gchar *font;

  priv = panel->priv = ASTRO_PANEL_GET_PRIVATE (panel);

  clutter_actor_set_size (CLUTTER_ACTOR (panel),
                          CSW (), ASTRO_PANEL_HEIGHT ());

  /* Background rect */
  priv->panel_bg = clutter_rectangle_new_with_color (&white);
  clutter_container_add_actor (CLUTTER_CONTAINER (panel), priv->panel_bg);
  clutter_actor_set_size (priv->panel_bg, CSW(), ASTRO_PANEL_HEIGHT ());
  clutter_actor_set_position (priv->panel_bg, 0, 0);
  clutter_actor_set_opacity (priv->panel_bg, 0);

  /* Home button */
  pixbuf = gdk_pixbuf_new_from_file_at_size (PKGDATADIR "/icons/home.png",
                                             ASTRO_PANEL_HEIGHT () - PADDING,
                                             ASTRO_PANEL_HEIGHT () - PADDING,
                                             NULL);
  if (pixbuf)
    {
      priv->home = clutter_texture_new_from_pixbuf (pixbuf);
      clutter_container_add_actor (CLUTTER_CONTAINER (panel), priv->home);
      clutter_actor_set_position (priv->home, PADDING/2, PADDING/2);
      clutter_actor_set_reactive (priv->home, TRUE);

      g_signal_connect (priv->home, "button-release-event",
                        G_CALLBACK (on_home_clicked), panel);
    }

  /* Title label */
  font = g_strdup_printf ("Sans %d", (int)(ASTRO_PANEL_HEIGHT () * 0.4));
  priv->title = clutter_label_new_full (font, "Home", &white);
  clutter_container_add_actor (CLUTTER_CONTAINER (panel), priv->title);
  clutter_actor_set_position (priv->title, 
                              clutter_actor_get_width (priv->home)+(PADDING*3),
          (ASTRO_PANEL_HEIGHT ()/2)-(clutter_actor_get_height (priv->title)/3));
  g_free (font);

  /* Close button */
  pixbuf = gdk_pixbuf_new_from_file_at_size (PKGDATADIR "/icons/close.png",
                                           ASTRO_PANEL_HEIGHT () - PADDING,
                                           ASTRO_PANEL_HEIGHT () - PADDING,
                                           NULL);
  if (pixbuf)
    {
      priv->close = clutter_texture_new_from_pixbuf (pixbuf);
      clutter_container_add_actor (CLUTTER_CONTAINER (panel), priv->close);
      clutter_actor_set_anchor_point_from_gravity (priv->close,
                                                   CLUTTER_GRAVITY_WEST);
      clutter_actor_set_position (priv->close, 
                   CSW () - clutter_actor_get_width (priv->close) - (PADDING/2),
                                  ASTRO_PANEL_HEIGHT () /2);
      clutter_actor_set_reactive (priv->close, TRUE);

      g_signal_connect (priv->close, "button-release-event",
                        G_CALLBACK (on_close_clicked), panel);
    }

  /* Systray */
  priv->systray = astro_systray_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (panel), priv->systray);
  clutter_actor_set_position (priv->systray,
                              CSW () 
                                - clutter_actor_get_width (priv->close)
                                - clutter_actor_get_width (priv->systray)
                                - PADDING*2,
                              PADDING/2);

  clutter_actor_show_all (CLUTTER_ACTOR (panel));
}

ClutterActor * 
astro_panel_new (void)
{
  AstroPanel *panel =  g_object_new (ASTRO_TYPE_PANEL,
									                       NULL);

  return CLUTTER_ACTOR (panel);
}

