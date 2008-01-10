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


#include "astro-desktop.h"

#include <libastro-desktop/astro-defines.h>
#include <libastro-desktop/astro-application.h>

#include "astro-applet-manager.h"
#include "astro-appview.h"
#include "astro-example.h"
#include "astro-panel.h"

G_DEFINE_TYPE (AstroDesktop, astro_desktop, CLUTTER_TYPE_GROUP);

#define ASTRO_DESKTOP_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
        ASTRO_TYPE_DESKTOP, AstroDesktopPrivate))
	
struct _AstroDesktopPrivate
{
  ClutterActor 		 *panel;
  ClutterActor     *appview;
  ClutterActor     *applets;

  GList            *apps;
  GList            *apps_modules;
	AstroApplication *active_app;
};

/* Public Functions */

/* Private functions */
static void
on_appview_activated (AstroAppview     *appview, 
                      AstroApplication *application,
                      AstroDesktop     *desktop)
{
  AstroDesktopPrivate *priv;

  g_return_if_fail (ASTRO_IS_DESKTOP (desktop));
  priv = desktop->priv;

  g_debug ("Application activated\n");
}

static gboolean
on_key_release_event (ClutterActor *actor, 
                      ClutterEvent *event,
                      AstroDesktop *desktop)
{
  AstroDesktopPrivate *priv;
  static gboolean showed = TRUE;

  g_return_val_if_fail (ASTRO_IS_DESKTOP (desktop), FALSE);
  priv = desktop->priv;

  switch (event->key.keyval)
    {
      case CLUTTER_Return:
      case CLUTTER_KP_Enter:
      case CLUTTER_ISO_Enter:
        g_print ("Activate\n");
        if (showed)
          {
            clutter_actor_hide (priv->appview);
            clutter_actor_hide (priv->applets);
          }
        else
          {
            clutter_actor_show (priv->appview);
            clutter_actor_show (priv->applets);
          }
        showed = !showed;
        break;
      case CLUTTER_Left:
      case CLUTTER_KP_Left:
        astro_appview_advance (ASTRO_APPVIEW (priv->appview), -1);
        break;
      case CLUTTER_Right:
      case CLUTTER_KP_Right:
        astro_appview_advance (ASTRO_APPVIEW (priv->appview), 1);
        break;
      default:
        ;
    }

  return FALSE;
}

static void
load_applications (AstroDesktop *desktop)
{
  AstroDesktopPrivate *priv;
  GdkPixbuf *pixbuf;
  gint i;

  g_return_if_fail (ASTRO_IS_DESKTOP (desktop));
  priv = desktop->priv;

  /* FIXME: We should scan a directory for .so files */
  pixbuf = gdk_pixbuf_new_from_file (PKGDATADIR "/icons/exec.png", NULL);
  for (i = 0; i < 10; i++)
    {
      AstroApplication *app;
      gchar *title;
      
      title = g_strdup_printf ("Example %d", i+1);
      app = astro_example_new (title,
                               pixbuf);
      g_free (title);

      priv->apps = g_list_append (priv->apps, app);
    }
}

/* GObject stuff */
static void
astro_desktop_class_init (AstroDesktopClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (gobject_class, sizeof (AstroDesktopPrivate));
}

static void
astro_desktop_init (AstroDesktop *desktop)
{
  AstroDesktopPrivate *priv;
  priv = desktop->priv = ASTRO_DESKTOP_GET_PRIVATE (desktop);

  /* Load the panel */
  priv->panel = astro_panel_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (desktop), priv->panel);
  clutter_actor_set_position (priv->panel, 0, 0);

  /* Load the applications */
  load_applications (desktop);
  priv->appview = astro_appview_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (desktop), priv->appview);
  clutter_actor_set_size (priv->appview, 
                          ASTRO_WINDOW_WIDTH (), 
                          ASTRO_WINDOW_HEIGHT ());
  clutter_actor_set_position (priv->appview, CSW(), 0);
  astro_appview_set_app_list (ASTRO_APPVIEW (priv->appview), priv->apps);

  g_signal_connect (priv->appview, "launch-app",
                    G_CALLBACK (on_appview_activated), desktop);

  /* Load the applets */
  priv->applets = astro_applet_manager_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (desktop), priv->applets);
  clutter_actor_set_position (priv->applets, 
                              CSW(), 
                       CSH() - ASTRO_APPLET_HEIGHT() -(ASTRO_APPLET_PADDING*3));

  g_signal_connect (desktop, "key-release-event",
                    G_CALLBACK (on_key_release_event), desktop);

  clutter_grab_keyboard (CLUTTER_ACTOR (desktop));
  clutter_actor_show_all (CLUTTER_ACTOR (desktop));
}

ClutterActor * 
astro_desktop_new (void)
{
  AstroDesktop *desktop =  g_object_new (ASTRO_TYPE_DESKTOP,
									                       NULL);

  return CLUTTER_ACTOR (desktop);
}

