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
  gint hello;
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

/* Public Functions */

/* Private functions */


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
  AstroSongsPrivate *priv;
  ClutterColor white = { 0xff, 0xff, 0xff, 0xff };
  gchar *font = NULL;
  gint i, offset = ROW_SPACING/2;

  priv = songs->priv = ASTRO_SONGS_GET_PRIVATE (songs);

  font = g_strdup_printf ("Sans %d", FONT_SIZE);

  for (i = 0; i < 10; i++)
    {
      ClutterActor *row;
           
      row = clutter_label_new_full (font, song_names[i], &white);
      
      clutter_container_add_actor (CLUTTER_CONTAINER (songs), row);
      clutter_actor_set_anchor_point_from_gravity (row, 
                                                   CLUTTER_GRAVITY_WEST);
      clutter_actor_set_position (row, 10, offset);
      clutter_actor_set_scale (row, 1/ALBUM_SCALE, 1/ALBUM_SCALE);
      clutter_actor_set_opacity (row, i%2 ? 200: 255);

      offset += ROW_SPACING;
    }
  
  clutter_actor_set_clip (CLUTTER_ACTOR (songs), 
                          0, 0, ALBUM_SIZE-2, ALBUM_SIZE-2);
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

