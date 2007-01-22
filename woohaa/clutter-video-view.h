/*
 *
 * Authored By XXXXX
 *
 * Copyright (C) 2006 XXXXXX
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

#ifndef _HAVE_CLUTTER_VIDEO_VIEW_H
#define _HAVE_CLUTTER_VIDEO_VIEW_H

#include <glib-object.h>
#include <clutter/clutter.h>
#include "clutter-video-model.h"

G_BEGIN_DECLS

#define CLUTTER_TYPE_VIDEO_VIEW clutter_video_view_get_type()

#define CLUTTER_VIDEO_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  CLUTTER_TYPE_VIDEO_VIEW, ClutterVideoView))

#define CLUTTER_VIDEO_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  CLUTTER_TYPE_VIDEO_VIEW, ClutterVideoViewClass))

#define CLUTTER_IS_VIDEO_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  CLUTTER_TYPE_VIDEO_VIEW))

#define CLUTTER_IS_VIDEO_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  CLUTTER_TYPE_VIDEO_VIEW))

#define CLUTTER_VIDEO_VIEW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  CLUTTER_TYPE_VIDEO_VIEW, ClutterVideoViewClass))

typedef struct _ClutterVideoView ClutterVideoView;
typedef struct _ClutterVideoViewClass ClutterVideoViewClass;
typedef struct _ClutterVideoViewPrivate ClutterVideoViewPrivate;

struct _ClutterVideoView
{
  ClutterActor         parent;
  /*< private >*/
  ClutterVideoViewPrivate   *priv;
};

struct _ClutterVideoViewClass 
{
  /*< private >*/
  ClutterActorClass parent_class;

  void (*_clutter_video_view_1) (void);
  void (*_clutter_video_view_2) (void);
  void (*_clutter_video_view_3) (void);
  void (*_clutter_video_view_4) (void);
}; 

GType clutter_video_view_get_type (void) G_GNUC_CONST;

ClutterActor*
clutter_video_view_new (ClutterVideoModel *model, 
			gint               width, 
			gint               height,
			gint               n_items_visible);
void
clutter_video_view_advance (ClutterVideoView *view, gint n);

void
clutter_video_view_switch (ClutterVideoView *view);

ClutterVideoModelItem*
clutter_video_view_get_selected (ClutterVideoView *view);

G_END_DECLS

#endif
