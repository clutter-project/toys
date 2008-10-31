#ifndef _HAVE_WH_SCREEN_VIDEO_H
#define _HAVE_WH_SCREEN_VIDEO_H

#include <clutter/clutter.h>
#include "wh-video-view.h"

G_BEGIN_DECLS

#define WH_TYPE_SCREEN_VIDEO wh_screen_video_get_type()

#define WH_SCREEN_VIDEO(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  WH_TYPE_SCREEN_VIDEO, WHScreenVideo))

#define WH_SCREEN_VIDEO_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  WH_TYPE_SCREEN_VIDEO, WHScreenVideoClass))

#define WH_IS_SCREEN_VIDEO(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  WH_TYPE_SCREEN_VIDEO))

#define WH_IS_SCREEN_VIDEO_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  WH_TYPE_SCREEN_VIDEO))

#define WH_SCREEN_VIDEO_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  WH_TYPE_SCREEN_VIDEO, WHScreenVideoClass))

typedef struct {
  ClutterActor parent;
} WHScreenVideo;

typedef struct {
  ClutterActorClass parent_class;

  void (*started) (WHScreenVideo *screen);
  void (*finished) (WHScreenVideo *screen);

} WHScreenVideoClass;

GType wh_screen_video_get_type (void);

ClutterActor* wh_screen_video_new (void);

gboolean
wh_screen_video_activate (WHScreenVideo *screen, WHVideoView *view);

void
wh_screen_video_deactivate (WHScreenVideo *screen);

gboolean
wh_screen_video_get_playing (WHScreenVideo *screen);

G_END_DECLS

#endif
