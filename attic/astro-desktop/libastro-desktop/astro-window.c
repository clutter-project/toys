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


#include "astro-window.h"

#include "astro-defines.h"

G_DEFINE_TYPE (AstroWindow, astro_window, CLUTTER_TYPE_GROUP);

#define ASTRO_WINDOW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
        ASTRO_TYPE_WINDOW, AstroWindowPrivate))

struct _AstroWindowPrivate
{
  ClutterEffectTemplate *show_temp;
  ClutterTimeline       *show_time;

  ClutterEffectTemplate *hide_temp;
  ClutterTimeline       *hide_time;
};


static void
astro_window_show (ClutterActor *view)
{
  AstroWindowPrivate *priv;
  static ClutterTimeline *show_time = NULL;
  
  g_return_if_fail (ASTRO_IS_WINDOW (view));
  priv = ASTRO_WINDOW (view)->priv;

  if (CLUTTER_IS_TIMELINE (show_time) &&clutter_timeline_is_playing (show_time))
    {
      clutter_timeline_stop (show_time);
      g_object_unref (show_time);
    }

  CLUTTER_ACTOR_CLASS (astro_window_parent_class)->show (view);

  show_time = clutter_effect_fade (priv->show_temp,
                                   CLUTTER_ACTOR (view),
                                   255,
                                   NULL, NULL);
}

static void
astro_window_hide (ClutterActor *view)
{
  AstroWindowPrivate *priv;
  static ClutterTimeline *hide_time = NULL;
  
  g_return_if_fail (ASTRO_IS_WINDOW (view));
  priv = ASTRO_WINDOW (view)->priv;

  if (CLUTTER_IS_TIMELINE (hide_time) &&clutter_timeline_is_playing (hide_time))
    {
      clutter_timeline_stop (hide_time);
      g_object_unref (hide_time);
    }
  
  hide_time = clutter_effect_fade (priv->hide_temp,
                                   CLUTTER_ACTOR (view),
                                   0,
     (ClutterEffectCompleteFunc)
        CLUTTER_ACTOR_CLASS (astro_window_parent_class)->hide,
                                   NULL);
}



void               
astro_window_close      (AstroWindow *window)
{
  AstroWindowPrivate *priv;
  static ClutterTimeline *hide_time = NULL;
  
  g_return_if_fail (ASTRO_IS_WINDOW (window));
  priv = ASTRO_WINDOW (window)->priv;

  if (CLUTTER_IS_TIMELINE (hide_time) &&clutter_timeline_is_playing (hide_time))
    {
      clutter_timeline_stop (hide_time);
      g_object_unref (hide_time);
    }

  hide_time = clutter_effect_move (priv->hide_temp,
                                   CLUTTER_ACTOR (window),
                                   CSW(),
                                   clutter_actor_get_y (CLUTTER_ACTOR (window)),
                                   NULL, NULL);
  
  hide_time = clutter_effect_fade (priv->hide_temp,
                                   CLUTTER_ACTOR (window),
                                   0,
                            (ClutterEffectCompleteFunc)clutter_actor_destroy,
                            NULL);
}

/* GObject stuff */
static void
astro_window_class_init (AstroWindowClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  actor_class->show = astro_window_show;
  actor_class->hide = astro_window_hide;

  g_type_class_add_private (gobject_class, sizeof (AstroWindowPrivate));
}


static void
astro_window_init (AstroWindow *window)
{
  AstroWindowPrivate *priv;

  priv = window->priv = ASTRO_WINDOW_GET_PRIVATE (window);

  clutter_actor_set_opacity (CLUTTER_ACTOR (window), 0);

  priv->show_time = clutter_timeline_new_for_duration (300);
  priv->show_temp = clutter_effect_template_new (priv->show_time, 
                                                 clutter_sine_inc_func);
  priv->hide_time = clutter_timeline_new_for_duration (300);
  priv->hide_temp = clutter_effect_template_new (priv->hide_time, 
                                                 clutter_sine_inc_func);;
}

ClutterActor *
astro_window_new ()
{
  return g_object_new (ASTRO_TYPE_WINDOW, NULL);
}

