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


#include "astro-songs.h"

#include <libastro-desktop/astro-defines.h>
#include <libastro-desktop/astro-application.h>
#include <libastro-desktop/astro-window.h>

#include "astro-music-window.h"

G_DEFINE_TYPE (AstroSongs, astro_songs, CLUTTER_TYPE_GROUP);

#define ASTRO_SONGS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
        ASTRO_TYPE_SONGS, AstroSongsPrivate))
	
struct _AstroSongsPrivate
{
  ClutterActor *group;
  ClutterActor *events;

  gboolean mousedown;
  gint     lasty;
  gint     starty;
  guint32  start_time;
  gint     endy;
  
  ClutterEffectTemplate *temp;
  ClutterTimeline       *timeline;
};

static gchar *song_names[] = {
  "Oh Timbaland",
  "Give It To Me",
  "Release",
  "The Way I Are",
  "Bounce",
  "Come And Get Me",
  "Kill Yourself",
  "Boardmeeting",
  "Fantasy",
  "Screem",
  "Miscommunication",
  "Bombay",
  "Throw It On Me",
  "Time",
  "One And Only",
  "Apologize",
  "2 Man Show",
  "Hello"
};

static gboolean on_event (AstroSongs *songs, ClutterEvent *event);

/* Public Functions */
void 
astro_songs_set_active (AstroSongs *songs, gboolean active)
{
  AstroSongsPrivate *priv;

  g_return_if_fail (ASTRO_IS_SONGS (songs));
  priv = songs->priv;

  if (active)
    {
      g_signal_connect (songs, "event", G_CALLBACK (on_event), NULL);
    }
  else
    {
      g_signal_handlers_disconnect_by_func (songs, on_event, NULL);
      priv->mousedown = FALSE;
    }
}


/* Private functions */
static void
bounds_check (ClutterActor *group, AstroSongs *songs)
{
  AstroSongsPrivate *priv;
  gint y, height;

  g_return_if_fail (ASTRO_IS_SONGS (songs));
  priv = songs->priv;

  height = clutter_actor_get_height (group);
  y = clutter_actor_get_y (group);
   
  if (y < 0 && y < (-1*height+(ALBUM_SIZE/2)))
  {
    y = (-1*height) + ALBUM_SIZE/2;
  }
  
  if ( y > 0 && y > (ALBUM_SIZE/2))
    {
      y = ALBUM_SIZE/2;
    }
  
  g_print ("%d %d %d\n\n", y, height, ALBUM_SIZE/2);
      
  priv->timeline = clutter_effect_move (priv->temp, priv->group,
                                        clutter_actor_get_x (priv->group),
                                        y, 
                                        NULL, NULL);
}

static gboolean 
on_event (AstroSongs *songs, ClutterEvent *event)
{
#define TIMEOUT 400
#define SPEED_FACTOR 1.5
  AstroSongsPrivate *priv = songs->priv;
    
  if (event->type == CLUTTER_BUTTON_PRESS)
    {
      priv->starty = priv->lasty = event->button.y;
      
      priv->start_time = event->button.time;
    
      if (clutter_timeline_is_playing (priv->timeline))
        clutter_timeline_stop (priv->timeline);
    
    }
  else if (event->type == CLUTTER_BUTTON_RELEASE)
    {
      gint endy = clutter_actor_get_y (priv->group);
      guint32 time = event->button.time - priv->start_time;
      gfloat factor;
      
      factor = 2.0 - (time/event->button.time);
    
      if (time > TIMEOUT)
        {
          priv->endy = endy;
        }
      else if (event->button.y > priv->starty)
        {
          /* 
          * The mouse from left to right, so we have to *add* pixels to the
          * current group position to make it move to the right
          */
          endy += (event->motion.y - priv->starty) * SPEED_FACTOR * factor;
          priv->endy = endy;
        }
      else if (event->button.y < priv->starty)
        {
          /* 
          * The mouse from right to left, so we have to *minus* p.yels to the
          * current group position to make it move to the left
          */
          endy -= (priv->starty - event->button.y) * SPEED_FACTOR * factor;
          priv->endy = endy;
        }
      else
        {
          /* If the click was fast, treat it as a standard 'clicked' event */
          if (time < TIMEOUT)
            g_debug ("Song clicked\n");
          priv->starty = priv->lasty = 0;
          return FALSE;
        }
  
     priv->timeline = clutter_effect_move (priv->temp, priv->group,
                                          clutter_actor_get_x (priv->group),
                                          priv->endy, 
                                      (ClutterEffectCompleteFunc)bounds_check, 
                                          songs);

      priv->starty =  priv->lasty = 0;
    }
  else if (event->type == CLUTTER_MOTION)
    {
      gint offset;

      if (!priv->starty)
        return FALSE;
      if (event->motion.y > priv->lasty)
        {
          offset = event->motion.y - priv->lasty;
        }
      else
        {
          offset = priv->lasty - event->motion.y;
          offset *= -1;
        }
      priv->lasty = event->motion.y;
      clutter_actor_set_position (priv->group,
                                  clutter_actor_get_x (priv->group),
                                  clutter_actor_get_y (priv->group)+ offset);
    }  
  
  return FALSE;
}

/* GObject stuff */
static void
astro_songs_class_init (AstroSongsClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
  
  g_type_class_add_private (gobject_class, sizeof (AstroSongsPrivate));
}

static void
astro_songs_init (AstroSongs *songs)
{
#define FONT_SIZE (ALBUM_SIZE/8)
#define ROW_SPACING (FONT_SIZE*1.5)
#define PAD 2
  AstroSongsPrivate *priv;
  ClutterColor white = { 0xff, 0xff, 0xff, 0xff };
  gchar *font = NULL;
  gint i, offset = ROW_SPACING/2;

  priv = songs->priv = ASTRO_SONGS_GET_PRIVATE (songs);

  priv->mousedown = FALSE;

  priv->group = clutter_group_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (songs), priv->group);
  clutter_actor_set_position (priv->group, 0, 0);

  font = g_strdup_printf ("Sans %d", FONT_SIZE);

  for (i = 0; i < 10; i++)
    {
      ClutterActor *row;
           
      row = clutter_label_new_full (font, song_names[i], &white);
      
      clutter_container_add_actor (CLUTTER_CONTAINER (priv->group), row);
      clutter_actor_set_anchor_point_from_gravity (row, 
                                                   CLUTTER_GRAVITY_WEST);
      clutter_actor_set_position (row, 10, offset);
      clutter_actor_set_scale (row, 1/ALBUM_SCALE, 1/ALBUM_SCALE);
      clutter_actor_set_opacity (row, i%2 ? 200: 255);

      offset += ROW_SPACING;
    }

  priv->timeline = clutter_timeline_new_for_duration (1000);
  priv->temp = clutter_effect_template_new (priv->timeline,
                                            clutter_sine_inc_func);

  clutter_actor_set_reactive (CLUTTER_ACTOR (songs), TRUE);
  
  clutter_actor_set_clip (CLUTTER_ACTOR (songs), 
                          PAD, PAD, ALBUM_SIZE-(2*PAD), ALBUM_SIZE-(2*PAD));
 
  clutter_actor_show_all (CLUTTER_ACTOR (priv->group));
  clutter_actor_show_all (CLUTTER_ACTOR (songs));
  g_free (font);
}

ClutterActor * 
astro_songs_new (void)
{
  ClutterActor *songs =  g_object_new (ASTRO_TYPE_SONGS,
	                                     NULL);

  return songs;
}

