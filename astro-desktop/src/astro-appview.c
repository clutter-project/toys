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


#include "astro-appview.h"

#include <math.h>
#include <libastro-desktop/astro-defines.h>
#include <libastro-desktop/astro-application.h>

#include "astro-panel.h"
#include "astro-example.h"
#include "astro-appicon.h"

G_DEFINE_TYPE (AstroAppview, astro_appview, CLUTTER_TYPE_GROUP);

#define ASTRO_APPVIEW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
        ASTRO_TYPE_APPVIEW, AstroAppviewPrivate))
#define VARIANCE 100
#define MAX_BLUR 10.0

struct _AstroAppviewPrivate
{
  GList                 *apps;
  gint                   active;

  /* Timeline stuff */
  ClutterEffectTemplate *move_temp;
  ClutterTimeline       *move_time;

  ClutterEffectTemplate *show_temp;
  ClutterTimeline       *show_time;

  ClutterEffectTemplate *hide_temp;
  ClutterTimeline       *hide_time;
};

enum
{
  LAUNCH_APP,

  LAST_SIGNAL
};
static guint _appview_signals[LAST_SIGNAL] = { 0 };

/* Private functions */
static void
ensure_layout (AstroAppview *view)
{
  AstroAppviewPrivate *priv;
  GList *l;
  gint groupx = 0;
  gint center = 0;
  gint i = 0;
  
  priv = view->priv;

  groupx = clutter_actor_get_x (CLUTTER_ACTOR (view));
  center = CSW()/2;

  l = clutter_container_get_children (CLUTTER_CONTAINER (view));
  for (l; l; l = l->next)
    {
      ClutterActor *icon = l->data;
      gint realx, diff, y_diff;;
      gfloat scale;

      realx = clutter_actor_get_x (icon) + groupx;
      
      if (realx > center && realx < CSW ())
        {
          diff = center - (realx - center);
        }
      else if (realx > 0 && realx <= center)
        {
          diff = realx;
        }
      else
        {
          diff = 0;
        }
  
      scale = (gfloat)diff/center;
      scale = 0.2 + (0.8 * scale);
      clutter_actor_set_scale (icon, scale, scale);

      if (realx < center)
        {
          gfloat angle, sine;

          angle = scale * (3.14*2);
          sine = sin (0.5 *angle);
          
          y_diff = (CSH()/2) + (VARIANCE * sine);
        }
      else
        {
          gfloat angle, sine;

          angle = scale * (3.14*2);
          sine = sin (0.5*angle);
          
          y_diff = (CSH()/2) - (VARIANCE * sine);

        }
      clutter_actor_set_y (icon, y_diff);
      
      astro_appicon_set_blur (ASTRO_APPICON (icon), (1.0 - scale) * MAX_BLUR);

      i++;
    }
}

static void
on_move_timeline_new_frame (ClutterTimeline *timeline, 
                            gint             frame,
                            AstroAppview    *view)
{
  g_return_if_fail (ASTRO_IS_APPVIEW (view));
  
  ensure_layout (view);
}

static void
on_appicon_clicked (AstroAppicon     *icon, 
                    AstroApplication *app, 
                    AstroAppview     *view)
{
  AstroAppviewPrivate *priv;
  AstroApplication *active_app;

  g_return_if_fail (ASTRO_IS_APPVIEW (view));
  priv = view->priv;

  active_app = g_list_nth_data (priv->apps, priv->active);

  if (active_app == app)
    g_print ("Active\n");
  else
    {
      gint new_active = g_list_index (priv->apps, app);
      astro_appview_advance (view, new_active - priv->active);
    }
}

static void
astro_appview_show (ClutterActor *view)
{
  AstroAppviewPrivate *priv;
  static ClutterTimeline *show_time = NULL;
  
  g_return_if_fail (ASTRO_IS_APPVIEW (view));
  priv = ASTRO_APPVIEW (view)->priv;

  if (CLUTTER_IS_TIMELINE (show_time) &&clutter_timeline_is_playing (show_time))
    {
      clutter_timeline_stop (show_time);
      g_object_unref (show_time);
    }

  clutter_actor_set_x (view, -1* clutter_actor_get_width (view));
  CLUTTER_ACTOR_CLASS (astro_appview_parent_class)->show (view);

  show_time = clutter_effect_move (priv->show_temp,
                                   CLUTTER_ACTOR (view),
                             (CSW()/2)- (priv->active * ASTRO_APPICON_SIZE ()),
                             clutter_actor_get_y (CLUTTER_ACTOR (view)),
                                   NULL, NULL);

  g_signal_connect (show_time, "new-frame",
                    G_CALLBACK (on_move_timeline_new_frame), view); 
}

static void
on_hide_timeline_completed (ClutterTimeline *timeline, ClutterActor *view)
{
  CLUTTER_ACTOR_CLASS (astro_appview_parent_class)->hide (view);
}

static void
astro_appview_hide (ClutterActor *view)
{
  AstroAppviewPrivate *priv;
  static ClutterTimeline *hide_time = NULL;
  
  g_return_if_fail (ASTRO_IS_APPVIEW (view));
  priv = ASTRO_APPVIEW (view)->priv;

  if (CLUTTER_IS_TIMELINE (hide_time) &&clutter_timeline_is_playing (hide_time))
    {
      clutter_timeline_stop (hide_time);
      g_object_unref (hide_time);
    }
  
  hide_time = clutter_effect_move (priv->hide_temp,
                                   CLUTTER_ACTOR (view),
                                   -1 * clutter_actor_get_width (view),
                                   clutter_actor_get_y (CLUTTER_ACTOR (view)),
                                   NULL, NULL);

  g_signal_connect (hide_time, "new-frame",
                    G_CALLBACK (on_move_timeline_new_frame), view); 
  g_signal_connect (hide_time, "completed",
                    G_CALLBACK (on_hide_timeline_completed), view);
  //priv->active = 0;
}
/* Public Functions */
void
astro_appview_set_app_list (AstroAppview *view, 
                            GList        *apps)
{
  AstroAppviewPrivate *priv;
  GList *l;
  gint offset = 0;

  g_return_if_fail (ASTRO_IS_APPVIEW (view));
  priv = view->priv;

  priv->apps = apps;
  priv->active = 0;

  /* Add all the icons */
  for (l = apps; l; l = l->next)
    {
      AstroApplication *app = l->data;
      ClutterActor *icon = astro_appicon_new (app);

      clutter_container_add_actor (CLUTTER_CONTAINER (view), icon);
      clutter_actor_set_size (icon, ASTRO_APPICON_SIZE (),ASTRO_APPICON_SIZE());
      clutter_actor_set_anchor_point_from_gravity (icon,CLUTTER_GRAVITY_CENTER);

      clutter_actor_set_position (icon, offset, CSH ()/2);
      clutter_actor_show (icon);
      g_signal_connect (icon, "clicked",
                        G_CALLBACK (on_appicon_clicked), view);

      offset += ASTRO_APPICON_SIZE ();
    }
  astro_appview_advance (view, 0);
}

void
astro_appview_advance (AstroAppview *view,
                       gint          n)
{
  AstroAppviewPrivate *priv;
  static ClutterTimeline *move_time = NULL;
  gint new_active;

  g_return_if_fail (ASTRO_IS_APPVIEW (view));
  priv = view->priv;

  new_active = priv->active + n;
  if (new_active < 0 || new_active >= g_list_length (priv->apps))
    return;
  priv->active = new_active;

  if (CLUTTER_IS_TIMELINE (move_time) &&clutter_timeline_is_playing (move_time))
    {
      clutter_timeline_stop (move_time);
      g_object_unref (move_time);
    }
 
  move_time = clutter_effect_move (priv->move_temp,
                                   CLUTTER_ACTOR (view),
                              (CSW()/2)- (priv->active * ASTRO_APPICON_SIZE ()),
                                    clutter_actor_get_y (CLUTTER_ACTOR (view)),
                                    NULL, NULL);

  g_signal_connect (move_time, "new-frame",
                    G_CALLBACK (on_move_timeline_new_frame), view);
}

/* GObject stuff */
static void
astro_appview_class_init (AstroAppviewClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  actor_class->show = astro_appview_show;
  actor_class->hide = astro_appview_hide;

  _appview_signals[LAUNCH_APP] = 
    g_signal_new ("launch-app",
                  G_OBJECT_CLASS_TYPE (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (AstroAppviewClass, launch_app),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 0,
                  ASTRO_TYPE_APPLICATION);


  g_type_class_add_private (gobject_class, sizeof (AstroAppviewPrivate));
}

static void
astro_appview_init (AstroAppview *appview)
{
  AstroAppviewPrivate *priv;
  priv = appview->priv = ASTRO_APPVIEW_GET_PRIVATE (appview);

  priv->active = 0;
  priv->apps = NULL;

  priv->move_time = clutter_timeline_new_for_duration (300);
  priv->move_temp = clutter_effect_template_new (priv->move_time, 
                                                 clutter_sine_inc_func);

  priv->show_time = clutter_timeline_new_for_duration (600);
  priv->show_temp = clutter_effect_template_new (priv->show_time, 
                                                 clutter_sine_inc_func);
  priv->hide_time = clutter_timeline_new_for_duration (300);
  priv->hide_temp = clutter_effect_template_new (priv->hide_time, 
                                                 clutter_sine_inc_func);
 }

ClutterActor * 
astro_appview_new (void)
{
  AstroAppview *appview =  g_object_new (ASTRO_TYPE_APPVIEW,
									                       NULL);

  return CLUTTER_ACTOR (appview);
}

