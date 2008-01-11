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


#include "astro-music-window.h"

#include <libastro-desktop/astro-defines.h>
#include <libastro-desktop/astro-application.h>
#include <libastro-desktop/astro-window.h>

G_DEFINE_TYPE (AstroMusicWindow, astro_music_window, ASTRO_TYPE_WINDOW);

#define ASTRO_MUSIC_WINDOW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
        ASTRO_TYPE_MUSIC_WINDOW, AstroMusicWindowPrivate))

#define ALBUM_DIR PKGDATADIR"/albums"
#define ALBUM_SIZE (CSW()/3)

struct _AstroMusicWindowPrivate
{
  ClutterActor *albums;
  ClutterActor *label;

  ClutterActor *player;
};

/* Public Functions */

/* Private functions */
static ClutterActor *
make_cover (const gchar *filename)
{
  GdkPixbuf *pixbuf;
  ClutterActor *texture;

  pixbuf = gdk_pixbuf_new_from_file_at_size (filename,
                                             ALBUM_SIZE, ALBUM_SIZE,
                                             NULL);
  if (!pixbuf)
    return NULL;

  texture = clutter_texture_new_from_pixbuf (pixbuf);

  clutter_actor_set_anchor_point_from_gravity (texture, CLUTTER_GRAVITY_CENTER);

  return texture;
}

static void
load_albums (AstroMusicWindow *window)
{
  AstroMusicWindowPrivate *priv;
  GDir *dir;
  const gchar *leaf;
  GError *error = NULL;
  gint offset = 0;

  priv = window->priv;

  dir = g_dir_open (ALBUM_DIR, 0, &error);
  if (error)
    {
      g_warning ("Cannot load albums: %s", error->message);
      g_error_free (error);
      return;
    }
  
  while ((leaf = g_dir_read_name (dir)))
    {
      ClutterActor *cover;
      gchar *filename;

      if (!g_str_has_suffix (leaf, ".jpg"))
        continue;

      filename = g_build_filename (ALBUM_DIR, leaf, NULL);
      cover = make_cover (filename);

      if (!CLUTTER_IS_ACTOR (cover))
        {
          g_free (filename);
          continue;
        }

      clutter_container_add_actor (CLUTTER_CONTAINER (priv->albums), cover);
      clutter_actor_set_position (cover, offset, 0);
      clutter_actor_show_all (cover);

      g_free (filename);

      offset += ALBUM_SIZE;
    }
}

/* GObject stuff */
static void
astro_music_window_class_init (AstroMusicWindowClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (gobject_class, sizeof (AstroMusicWindowPrivate));
}

static void
astro_music_window_init (AstroMusicWindow *window)
{
  AstroMusicWindowPrivate *priv;
  ClutterColor white = { 0xff, 0xff, 0xff, 0xff };

  priv = window->priv = ASTRO_MUSIC_WINDOW_GET_PRIVATE (window);

  priv->albums = clutter_group_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (window), priv->albums);
  clutter_actor_set_anchor_point_from_gravity (priv->albums, 
                                               CLUTTER_GRAVITY_WEST);
  clutter_actor_set_position (priv->albums, 0, CSH()/2);
  load_albums (window);

  priv->label = clutter_label_new_full ("Sans 18", 
                                        "Jay Z - American Gangster",
                                        &white);
  clutter_label_set_line_wrap (CLUTTER_LABEL (priv->label), FALSE);
  clutter_container_add_actor (CLUTTER_CONTAINER (window), priv->label);
  clutter_actor_set_anchor_point_from_gravity (priv->label, 
                                               CLUTTER_GRAVITY_CENTER);
  clutter_actor_set_position (priv->label, CSW()/2, CSH()*0.8);

  clutter_actor_show_all (CLUTTER_ACTOR (window));
}

AstroWindow * 
astro_music_window_new (void)
{
  AstroWindow *music_window =  g_object_new (ASTRO_TYPE_MUSIC_WINDOW,
									                       NULL);

  return music_window;
}

