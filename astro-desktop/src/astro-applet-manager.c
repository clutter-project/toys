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


#include "astro-applet-manager.h"

#include <libastro-desktop/astro-defines.h>

#include "astro-applet.h"

G_DEFINE_TYPE (AstroAppletManager, astro_applet_manager, CLUTTER_TYPE_GROUP);

#define ASTRO_APPLET_MANAGER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), ASTRO_TYPE_APPLET_MANAGER, AstroAppletManagerPrivate))

struct _AstroAppletManagerPrivate
{
  GList *applets;

  ClutterEffectTemplate *show_temp;
  ClutterTimeline       *show_time;

  ClutterEffectTemplate *hide_temp;
  ClutterTimeline       *hide_time;
};


/* GObject stuff */
static void
astro_applet_manager_show (ClutterActor *appman)
{
  AstroAppletManagerPrivate *priv;
  static ClutterTimeline *show_time = NULL;
  
  g_return_if_fail (ASTRO_IS_APPLET_MANAGER (appman));
  priv = ASTRO_APPLET_MANAGER (appman)->priv;

  if (CLUTTER_IS_TIMELINE (show_time) &&clutter_timeline_is_playing (show_time))
    {
      clutter_timeline_stop (show_time);
      g_object_unref (show_time);
    }

  clutter_actor_set_x (appman, clutter_actor_get_width (appman) * -1);
  CLUTTER_ACTOR_CLASS (astro_applet_manager_parent_class)->show (appman);

  show_time = clutter_effect_move (priv->show_temp,
                                   CLUTTER_ACTOR (appman),
                                   ASTRO_APPLET_PADDING,
                                   clutter_actor_get_y (CLUTTER_ACTOR (appman)),
                                   NULL, NULL);
}

static void
on_hide_timeline_completed (ClutterTimeline *timeline, ClutterActor *appman)
{
  CLUTTER_ACTOR_CLASS (astro_applet_manager_parent_class)->hide (appman);
}

static void
astro_applet_manager_hide (ClutterActor *appman)
{
  AstroAppletManagerPrivate *priv;
  static ClutterTimeline *hide_time = NULL;
  
  g_return_if_fail (ASTRO_IS_APPLET_MANAGER (appman));
  priv = ASTRO_APPLET_MANAGER (appman)->priv;

  if (CLUTTER_IS_TIMELINE (hide_time) &&clutter_timeline_is_playing (hide_time))
    {
      clutter_timeline_stop (hide_time);
      g_object_unref (hide_time);
    }
  
  hide_time = clutter_effect_move (priv->hide_temp,
                                   CLUTTER_ACTOR (appman),
                                   -1*clutter_actor_get_width (appman),
                                   clutter_actor_get_y (CLUTTER_ACTOR (appman)),
                                   NULL, NULL);

  g_signal_connect (hide_time, "completed",
                    G_CALLBACK (on_hide_timeline_completed), appman);
}

static void
astro_applet_manager_class_init (AstroAppletManagerClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  actor_class->show = astro_applet_manager_show;
  actor_class->hide = astro_applet_manager_hide;

  g_type_class_add_private (gobject_class, sizeof (AstroAppletManagerPrivate));
}

static ClutterActor *
_load_script (const gchar *name)
{
  ClutterScript *script;
  ClutterActor *applet;
  ClutterActor *child = NULL;
  GError *error = NULL;
  gint res;
  
  script = clutter_script_new ();

  clutter_script_load_from_file (script, name, &error);
  if (error)
    {
      g_warning ("Unable to load applet: %s", error->message);
      g_error_free (error);
      return NULL;
    }
  
  res = clutter_script_get_objects (script, "applet-child", &child, NULL);
   if (res == 3)
    {
      g_warning ("Unable to load script: %s", name);
      return NULL;  
    }

   if (!CLUTTER_IS_ACTOR (child))
    {
      g_warning ("Did not get child\n");
      return NULL;
    }

  applet = astro_applet_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (applet), child);
  clutter_actor_set_position (child, 
                              ASTRO_APPLET_PADDING, ASTRO_APPLET_PADDING);

  return applet;
}

static void
astro_applet_manager_init (AstroAppletManager *applet_manager)
{
  AstroAppletManagerPrivate *priv;
  GDir *dir;
  GError *error = NULL;
  const gchar *name;
  gint offset = 0;

  applet_manager->priv = ASTRO_APPLET_MANAGER_GET_PRIVATE (applet_manager);
  priv = applet_manager->priv;

  /* Load applets */
  dir = g_dir_open (PKGDATADIR "/applets", 0, &error);
  if (error)
    {
      g_warning ("Can't open applet directory: %s", error->message);
      g_error_free (error);
      return;
    }
  
  while ((name = g_dir_read_name (dir)))
    {
      if (g_str_has_suffix (name, ".json"))
        {
          ClutterActor *applet = NULL;
          gchar *filename = g_strdup_printf ("%s%s",
                                             PKGDATADIR"/applets/", 
                                             name);

          applet = _load_script (filename);

          if (!CLUTTER_IS_ACTOR (applet))
            { 
              g_free (filename);
              continue;
            }
          clutter_container_add_actor (CLUTTER_CONTAINER (applet_manager),
                                       applet);
          clutter_actor_set_position (applet, offset, 0);
          
          offset+= clutter_actor_get_width (applet) + ASTRO_APPLET_PADDING;
          g_free (filename);
        }
    }
  g_dir_close (dir);

  priv->show_time = clutter_timeline_new_for_duration (600);
  priv->show_temp = clutter_effect_template_new (priv->show_time, 
                                                 clutter_sine_inc_func);
  priv->hide_time = clutter_timeline_new_for_duration (300);
  priv->hide_temp = clutter_effect_template_new (priv->hide_time, 
                                                 clutter_sine_inc_func);
}

ClutterActor * 
astro_applet_manager_new (void)
{
  AstroAppletManager *applet_manager =  g_object_new (ASTRO_TYPE_APPLET_MANAGER,
									                       NULL);
  return CLUTTER_ACTOR (applet_manager);
}

