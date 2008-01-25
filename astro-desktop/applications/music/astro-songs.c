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

  gboolean mousedown;
  gint     last_motion;
  gint     start_y;
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
static gboolean
on_event (AstroSongs *songs, ClutterEvent *event)
{
  AstroSongsPrivate *priv;

  g_return_val_if_fail (ASTRO_IS_SONGS (songs), FALSE);
  priv = songs->priv;
 
  if (event->type == CLUTTER_BUTTON_PRESS)
    {
      g_debug ("button press\n");

      priv->mousedown = TRUE;
      priv->last_motion = priv->start_y = event->button.y;

      return TRUE;
    }
  else if (event->type == CLUTTER_BUTTON_RELEASE)
    {
      g_debug ("button release\n");

      priv->mousedown = FALSE;

      return TRUE;
    }
  else if (event->type == CLUTTER_MOTION)
    {
      if (!priv->mousedown)
        return FALSE;
      g_debug ("motion");

      clutter_actor_set_y (priv->group,
                           clutter_actor_get_y (CLUTTER_ACTOR (priv->group)) +
                             event->motion.y - priv->last_motion);

      priv->last_motion = event->motion.y;
    }
  else
    {

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

