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

#ifndef _HAVE_WH_VIDEO_VIEW_H
#define _HAVE_WH_VIDEO_VIEW_H

#include <glib-object.h>
#include <clutter/clutter.h>
#include "wh-video-model.h"
#include "wh-video-model-row.h"

G_BEGIN_DECLS

#define WH_TYPE_VIDEO_VIEW wh_video_view_get_type()

#define WH_VIDEO_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  WH_TYPE_VIDEO_VIEW, WHVideoView))

#define WH_VIDEO_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  WH_TYPE_VIDEO_VIEW, WHVideoViewClass))

#define WH_IS_VIDEO_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  WH_TYPE_VIDEO_VIEW))

#define WH_IS_VIDEO_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  WH_TYPE_VIDEO_VIEW))

#define WH_VIDEO_VIEW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  WH_TYPE_VIDEO_VIEW, WHVideoViewClass))

typedef struct _WHVideoView WHVideoView;
typedef struct _WHVideoViewClass WHVideoViewClass;
typedef struct _WHVideoViewPrivate WHVideoViewPrivate;

struct _WHVideoView
{
  ClutterActor         parent;
  /*< private >*/
  WHVideoViewPrivate   *priv;
};

struct _WHVideoViewClass 
{
  /*< private >*/
  ClutterActorClass parent_class;

  void (*_wh_video_view_1) (void);
  void (*_wh_video_view_2) (void);
  void (*_wh_video_view_3) (void);
  void (*_wh_video_view_4) (void);
}; 

GType wh_video_view_get_type (void) G_GNUC_CONST;

ClutterActor*
wh_video_view_new (WHVideoModel      *model, 
		   gint               n_items_visible);
void
wh_video_view_advance (WHVideoView *view, gint n);

void
wh_video_view_enable_animation (WHVideoView *view, gboolean active);

WHVideoModelRow*
wh_video_view_get_selected (WHVideoView *view);

G_END_DECLS

#endif
