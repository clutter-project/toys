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

#include <libastro-desktop/astro-application.h>
#include <libastro-desktop/astro-defines.h>
#include <libastro-desktop/astro-window.h>

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
  ClutterActor     *active_window;
};

/* Public Functions */

/* Private functions */
static void
astro_desktop_show_application (AstroDesktop     *desktop,
                                AstroApplication *application)
{
  AstroDesktopPrivate *priv;

  g_return_if_fail (ASTRO_IS_DESKTOP (desktop));
  priv = desktop->priv;
 
  if (ASTRO_IS_WINDOW (priv->active_window))
    {
      astro_window_close (ASTRO_WINDOW (priv->active_window));
    }

  clutter_ungrab_keyboard ();  
  clutter_actor_hide (priv->appview);
  clutter_actor_hide (priv->applets);

  priv->active_window = (ClutterActor*)astro_application_get_window 
                                                                 (application);
  clutter_container_add_actor (CLUTTER_CONTAINER (desktop), 
                               priv->active_window);
  clutter_actor_set_position (priv->active_window, 
                              0, 
                              ASTRO_PANEL_HEIGHT ());
  clutter_actor_show (priv->active_window);
}

static void
astro_desktop_hide_application (AstroDesktop     *desktop)
{
  AstroDesktopPrivate *priv;

  g_return_if_fail (ASTRO_IS_DESKTOP (desktop));
  priv = desktop->priv;
 
  if (!ASTRO_IS_WINDOW (priv->active_window))
    return; 
 
  astro_window_close (ASTRO_WINDOW (priv->active_window));

  clutter_actor_show (priv->applets);
  clutter_actor_show (priv->appview);

  clutter_grab_keyboard (CLUTTER_ACTOR (desktop));
}


static void
on_appview_activated (AstroAppview     *appview, 
                      AstroApplication *application,
                      AstroDesktop     *desktop)
{
  AstroDesktopPrivate *priv;

  g_return_if_fail (ASTRO_IS_DESKTOP (desktop));
  priv = desktop->priv;

  astro_desktop_show_application (desktop, application);
}

static gboolean
on_key_release_event (ClutterActor *actor, 
                      ClutterEvent *event,
                      AstroDesktop *desktop)
{
  AstroDesktopPrivate *priv;
  AstroApplication *application;

  g_return_val_if_fail (ASTRO_IS_DESKTOP (desktop), FALSE);
  priv = desktop->priv;

  switch (event->key.keyval)
    {
      case CLUTTER_Return:
      case CLUTTER_KP_Enter:
      case CLUTTER_ISO_Enter:
        application = astro_appview_get_active_app 
                                               (ASTRO_APPVIEW (priv->appview));
        astro_desktop_show_application (desktop, application);
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
on_panel_home_clicked (AstroPanel *panel, AstroDesktop *desktop)
{
  g_return_if_fail (ASTRO_IS_DESKTOP (desktop));

  astro_desktop_hide_application (desktop);
}

static AstroApplication *
_load_app_module (const gchar *filename)
{
  GModule *module;
  AstroApplication *app;
  AstroApplicationInitFunc init_func;

  module = g_module_open (filename, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);
  if (module == NULL)
    {
      g_warning ("Unable to load module %s : %s\n",filename, g_module_error ());
      return NULL;
    }

  /* Try and load the init symbol */
  if (g_module_symbol (module, "astro_application_factory_init",
                       (void*)&init_func)) 
    {
      app = (AstroApplication*)init_func ();
      if (ASTRO_IS_APPLICATION (app))
        {
          g_object_set_data (G_OBJECT (app), "module", module);
          return app;
        }
    } 
  
  g_warning ("Cannot init module %s: %s", filename, g_module_error ());

  g_module_close (module);

  return NULL;
}

static void
load_applications (AstroDesktop *desktop)
{
  AstroDesktopPrivate *priv;
  GdkPixbuf *pixbuf;
  GDir *dir;
  const gchar *leaf;
  gint i;

  g_return_if_fail (ASTRO_IS_DESKTOP (desktop));
  priv = desktop->priv;

  /* Load .so applications */
  dir = g_dir_open (PKGLIBDIR"/apps", 0, NULL);
  if (!dir)
    { 
      g_warning ("%s doesn't exist", PKGLIBDIR"/apps");
      return;
    }
  while ((leaf = g_dir_read_name (dir)))
    {
      AstroApplication *app;
      gchar *filename;

      if (!g_str_has_suffix (leaf, ".so"))
        continue;

      filename = g_build_filename (PKGLIBDIR"/apps", leaf, NULL);
      app = _load_app_module (filename);

      if (ASTRO_IS_APPLICATION (app))
        priv->apps = g_list_append (priv->apps, app);
      else
        g_debug ("load failed\n");

      g_free (filename);
    }
  g_dir_close (dir);
  
  pixbuf = gdk_pixbuf_new_from_file (PKGDATADIR "/icons/exec.png", NULL);
  for (i = 0; i < 5; i++)
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
  g_signal_connect (priv->panel, "show-home",
                    G_CALLBACK (on_panel_home_clicked), desktop);
  g_signal_connect (priv->panel, "close-window",
                    G_CALLBACK (on_panel_home_clicked), desktop);

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

