/*
 * An example Pong game in Clutter.
 *
 * Authored By: Rob Bradford <rob@o-hand.com>
 *
 * Copyright (C) 2007 Robert Bradford
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Artwork released under the same license.
 */

#include <clutter/clutter.h>
#ifdef CLUTTER_FLAVOUR_GLX
#include <clutter/clutter-glx.h>
#endif
#include <math.h>
#include <errno.h>
#include <stdlib.h>

#include <X11/keysym.h>

typedef enum {
  BAT_DIRECTION_UP,
  BAT_DIRECTION_DOWN,
  BAT_DIRECTION_NONE
} BatDirection;

typedef enum {
  BAT_RIGHT,
  BAT_LEFT
} Bat;

typedef struct _Ball
{
  ClutterActor *actor;
  double velocity;
  double angle;
} Ball;

typedef struct _PongApp
{
  ClutterActor *bat[2];
  ClutterActor *stage;
  BatDirection direction[2];
  guint timer;
  Ball *ball;
} PongApp;

#define BORDER 32

static void
move_bat (ClutterActor *actor, BatDirection direction)
{
  int x, y, dy;

  y = clutter_actor_get_y (actor);
  x = clutter_actor_get_x (actor);

  switch (direction)
  {
    case BAT_DIRECTION_UP:
      dy = -8;
      break;
    case BAT_DIRECTION_DOWN:
      dy = 8;
      break;
    default:
        break;
  }

  if ((y + dy) > BORDER && 
      (y + dy) < (CLUTTER_STAGE_HEIGHT () - clutter_actor_get_height (actor) - BORDER))
    clutter_actor_set_position (actor, x, y + dy);
}

static void
key_press_event_cb (ClutterStage *stage, ClutterEvent *event, gpointer user_data)
{
  ClutterKeyEvent *keyevent = (ClutterKeyEvent *)event;
  PongApp *app = (PongApp *)user_data;

  switch (keyevent->keyval)
  {
    case XK_a:
      app->direction[BAT_LEFT] = BAT_DIRECTION_UP;
      break;
    case XK_z:
      app->direction[BAT_LEFT] = BAT_DIRECTION_DOWN;
      break;
    case XK_k:
      app->direction[BAT_RIGHT] = BAT_DIRECTION_UP;
      break;
    case XK_m:
      app->direction[BAT_RIGHT] = BAT_DIRECTION_DOWN;
      break;
    default:
      break;
  }
}

static void
key_release_event_cb (ClutterStage *stage, ClutterEvent *event, gpointer user_data)
{
  ClutterKeyEvent *keyevent = (ClutterKeyEvent *)event;
  PongApp *app = (PongApp *)user_data;

  switch (keyevent->keyval)
  {
    case XK_a:
    case XK_z:
      app->direction[BAT_LEFT] = BAT_DIRECTION_NONE;
      break;
    case XK_k:
    case XK_m:
      app->direction[BAT_RIGHT] = BAT_DIRECTION_NONE;
    default:
      break;
  }
}

static void
position_ball (PongApp *app)
{
  Ball *ball = app->ball;
  int dx, dy;

  int ball_x, ball_y;
  int ball_w, ball_h;

  int bat_x, bat_y;
  int bat_w, bat_h;


  dy = round (ball->velocity * sin (ball->angle));
  dx = round (ball->velocity * cos (ball->angle));

  ball_x = clutter_actor_get_x (ball->actor);
  ball_y = clutter_actor_get_y (ball->actor);

  ball_w = clutter_actor_get_width (ball->actor);
  ball_h = clutter_actor_get_height (ball->actor);

  if (ball_x > CLUTTER_STAGE_WIDTH () / 2)
  {
    bat_x = clutter_actor_get_x (app->bat[BAT_RIGHT]);
    bat_y = clutter_actor_get_y (app->bat[BAT_RIGHT]);
  } else {
    bat_x = clutter_actor_get_x (app->bat[BAT_LEFT]);
    bat_y = clutter_actor_get_y (app->bat[BAT_LEFT]);
  }

  bat_w = clutter_actor_get_width (app->bat[BAT_LEFT]);
  bat_h = clutter_actor_get_height (app->bat[BAT_LEFT]);

  if (ball_y + dy + ball_h > (CLUTTER_STAGE_HEIGHT () - BORDER))
  {
    ball->angle = -ball->angle;
    position_ball (app);
    return;
  }

  if (ball_y + dy < BORDER)
  {
    ball->angle = -ball->angle;
    position_ball (app);
    return;
  }

  if (ball_x + dx + ball_w > (CLUTTER_STAGE_WIDTH () - BORDER))
  {
    ball->angle = -ball->angle;
    ball->velocity = -ball->velocity;
    position_ball (app);
    return;
  }

  if (ball_x + dx < BORDER)
  {
    ball->angle = -ball->angle;
    ball->velocity = -ball->velocity;
    position_ball (app);
    return;
  }

  clutter_actor_move_by (ball->actor, dx, dy);
}

gboolean
timeout_cb (gpointer user_data)
{
  PongApp *app = (PongApp *)user_data;

  if (app->direction[BAT_LEFT] != BAT_DIRECTION_NONE)
    move_bat (app->bat[BAT_LEFT], app->direction[BAT_LEFT]);

  if (app->direction[BAT_RIGHT] != BAT_DIRECTION_NONE)
    move_bat (app->bat[BAT_RIGHT], app->direction[BAT_RIGHT]);

  position_ball (app);

  return TRUE;
}

static void
create_actors (PongApp *app)
{
  GError *error = NULL;
  GdkPixbuf *pixbuf;

  pixbuf = gdk_pixbuf_new_from_file ("pong-bat.png", &error);

  if (error)
  {
    g_warning ("Problem creating pixbuf: %s", error->message);
    g_clear_error (&error);
    exit (1);
  }

  app->bat[BAT_RIGHT] = clutter_texture_new_from_pixbuf (pixbuf);
  app->bat[BAT_LEFT] = clutter_texture_new_from_pixbuf (pixbuf);
  g_object_unref (pixbuf);

  pixbuf = gdk_pixbuf_new_from_file ("pong-ball.png", &error);

  if (error)
  {
    g_warning ("Problem creating pixbuf: %s", error->message);
    g_clear_error (&error);
    exit (1);
  }

  app->ball->actor = clutter_texture_new_from_pixbuf (pixbuf);

  g_object_unref (pixbuf);

  clutter_group_add (CLUTTER_GROUP (app->stage), app->bat[BAT_RIGHT]);
  clutter_group_add (CLUTTER_GROUP (app->stage), app->bat[BAT_LEFT]);
  clutter_group_add (CLUTTER_GROUP (app->stage), app->ball->actor);
}

static void
position_actors (PongApp *app)
{
  int bat_w, bat_h;

  bat_w = clutter_actor_get_width (app->bat[BAT_RIGHT]);
  bat_h = clutter_actor_get_height (app->bat[BAT_RIGHT]);

  clutter_actor_set_position (app->bat[BAT_RIGHT], 
      CLUTTER_STAGE_WIDTH () - BORDER - bat_w,
      (CLUTTER_STAGE_HEIGHT () - bat_h) / 2);

  clutter_actor_set_position (app->bat[BAT_LEFT], 
      BORDER, 
      (CLUTTER_STAGE_HEIGHT () - bat_h) / 2);

  clutter_actor_set_position (app->ball->actor, 
      (CLUTTER_STAGE_WIDTH () - clutter_actor_get_width (app->ball->actor)) / 2,
      (CLUTTER_STAGE_HEIGHT () - clutter_actor_get_height (app->ball->actor)) / 2);
}

int
main (int argc, char **argv)
{
  ClutterColor bgcolour;
  GError *error = NULL;

  PongApp *app = NULL;

  clutter_init_with_args (&argc, &argv,
                          NULL,
                          NULL,
                          NULL,
                          &error);
  if (error)
  {
    g_warning ("Unable to initialise Clutter:\n%s",
        error->message);
    g_error_free (error);
    exit (1);
  }

  app = g_new0 (PongApp, 1);
  app->ball = g_new0 (Ball, 1);

  app->stage = clutter_stage_get_default ();

  create_actors (app);

  g_object_set (app->stage, "fullscreen", TRUE, NULL);

  clutter_actor_show_all (app->stage);

  position_actors (app);

  app->direction[BAT_RIGHT] = BAT_DIRECTION_NONE;
  app->direction[BAT_LEFT] = BAT_DIRECTION_NONE;


  app->ball->velocity = 10.0;
  app->ball->angle = 0.4;

  g_signal_connect (app->stage, "key-press-event", 
      (GCallback)key_press_event_cb, app);

  g_signal_connect (app->stage, "key-release-event", 
      (GCallback)key_release_event_cb, app);

  app->timer = g_timeout_add (20, (GSourceFunc)timeout_cb, app);

  clutter_main ();

  g_object_unref (app->stage);
  g_object_unref (app->bat[BAT_RIGHT]);
  g_object_unref (app->bat[BAT_LEFT]);
  g_object_unref (app->ball->actor);
  g_free (app->ball);
  g_free (app);

  return 0;
}
