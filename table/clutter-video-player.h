/*
 * Clutter.
 *
 * An OpenGL based 'interactive canvas' library.
 *
 * Authored By Matthew Allum  <mallum@openedhand.com>
 *
 * Copyright (C) 2006 OpenedHand
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
 */

#ifndef __CLUTTER_VIDEO_PLAYER_H__
#define __CLUTTER_VIDEO_PLAYER_H__

#include <clutter/clutter-group.h>
#include <clutter/clutter-color.h>
#include <clutter/clutter-event.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

#define CLUTTER_TYPE_VIDEO_PLAYER (clutter_video_player_get_type())

#define CLUTTER_VIDEO_PLAYER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  CLUTTER_TYPE_VIDEO_PLAYER, ClutterVideoPlayer))

#define CLUTTER_VIDEO_PLAYER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  CLUTTER_TYPE_VIDEO_PLAYER, ClutterVideoPlayerClass))

#define CLUTTER_IS_VIDEO_PLAYER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  CLUTTER_TYPE_VIDEO_PLAYER))

#define CLUTTER_IS_VIDEO_PLAYER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  CLUTTER_TYPE_VIDEO_PLAYER))

#define CLUTTER_VIDEO_PLAYER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  CLUTTER_TYPE_VIDEO_PLAYER, ClutterVideoPlayerClass))

typedef struct _ClutterVideoPlayer        ClutterVideoPlayer;
typedef struct _ClutterVideoPlayerClass   ClutterVideoPlayerClass;
typedef struct _ClutterVideoPlayerPrivate ClutterVideoPlayerPrivate;

struct _ClutterVideoPlayer
{
  ClutterGroup parent_instance;
  
  /*< private >*/
  ClutterVideoPlayerPrivate *priv;
}; 

struct _ClutterVideoPlayerClass 
{
  ClutterGroupClass parent_class;
};

GType         clutter_video_player_get_type         (void) G_GNUC_CONST;

ClutterActor *clutter_video_player_new              (const gchar * uri);

gint          clutter_video_player_get_width        (ClutterVideoPlayer * player);
gint          clutter_video_player_get_height       (ClutterVideoPlayer * player);

G_END_DECLS

#endif /* __CLUTTER_VIDEO_PLAYER_H__ */
