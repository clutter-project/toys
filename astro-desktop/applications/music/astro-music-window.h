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

#ifndef _HAVE_ASTRO_MUSIC_WINDOW_H
#define _HAVE_ASTRO_MUSIC_WINDOW_H

G_BEGIN_DECLS

#define ASTRO_TYPE_MUSIC_WINDOW astro_music_window_get_type()

#define ASTRO_MUSIC_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  ASTRO_TYPE_MUSIC_WINDOW, \
  AstroMusicWindow))

#define ASTRO_MUSIC_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
  ASTRO_TYPE_MUSIC_WINDOW, \
  AstroMusicWindowClass))

#define ASTRO_IS_MUSIC_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  ASTRO_TYPE_MUSIC_WINDOW))

#define ASTRO_IS_MUSIC_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  ASTRO_TYPE_MUSIC_WINDOW))

#define ASTRO_MUSIC_WINDOW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
  ASTRO_TYPE_MUSIC_WINDOW, \
  AstroMusicWindowClass))

typedef struct _AstroMusicWindow AstroMusicWindow;
typedef struct _AstroMusicWindowClass AstroMusicWindowClass;
typedef struct _AstroMusicWindowPrivate AstroMusicWindowPrivate;

struct _AstroMusicWindow
{
  AstroWindow      parent;
	
  /*< private >*/
  AstroMusicWindowPrivate   *priv;
};

struct _AstroMusicWindowClass 
{
  /*< private >*/
  AstroWindowClass parent_class;
}; 

GType astro_music_window_get_type (void) G_GNUC_CONST;

AstroWindow *  astro_music_window_new       (void);
  
G_END_DECLS

#endif
