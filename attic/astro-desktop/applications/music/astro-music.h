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


#include <glib.h>
#include <clutter/clutter.h>
#include <libastro-desktop/astro-application.h>

#ifndef _HAVE_ASTRO_MUSIC_H
#define _HAVE_ASTRO_MUSIC_H

G_BEGIN_DECLS

#define ASTRO_TYPE_MUSIC astro_music_get_type()

#define ASTRO_MUSIC(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  ASTRO_TYPE_MUSIC, \
  AstroMusic))

#define ASTRO_MUSIC_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
  ASTRO_TYPE_MUSIC, \
  AstroMusicClass))

#define ASTRO_IS_MUSIC(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  ASTRO_TYPE_MUSIC))

#define ASTRO_IS_MUSIC_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  ASTRO_TYPE_MUSIC))

#define ASTRO_MUSIC_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
  ASTRO_TYPE_MUSIC, \
  AstroMusicClass))

typedef struct _AstroMusic AstroMusic;
typedef struct _AstroMusicClass AstroMusicClass;
typedef struct _AstroMusicPrivate AstroMusicPrivate;

struct _AstroMusic
{
  AstroApplication       parent;
	
  /*< private >*/
  AstroMusicPrivate   *priv;
};

struct _AstroMusicClass 
{
  /*< private >*/
  AstroApplicationClass parent_class;
}; 

GType astro_music_get_type (void) G_GNUC_CONST;

AstroApplication *  astro_music_new       (const gchar  *title,
                                             GdkPixbuf    *icon);
  
G_END_DECLS

#endif
