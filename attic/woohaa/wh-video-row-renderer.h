#ifndef _WH_VIDEO_ROW_RENDERER
#define _WH_VIDEO_ROW_RENDERER

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define WH_TYPE_VIDEO_ROW_RENDERER wh_video_row_renderer_get_type()

#define WH_VIDEO_ROW_RENDERER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  WH_TYPE_VIDEO_ROW_RENDERER, WHVideoRowRenderer))

#define WH_VIDEO_ROW_RENDERER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  WH_TYPE_VIDEO_ROW_RENDERER, WHVideoRowRendererClass))

#define WH_IS_VIDEO_ROW_RENDERER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  WH_TYPE_VIDEO_ROW_RENDERER))

#define WH_IS_VIDEO_ROW_RENDERER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  WH_TYPE_VIDEO_ROW_RENDERER))

#define WH_VIDEO_ROW_RENDERER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  WH_TYPE_VIDEO_ROW_RENDERER, WHVideoRowRendererClass))

typedef struct {
  ClutterActor         parent;
} WHVideoRowRenderer;

typedef struct {
  ClutterActorClass parent_class;
} WHVideoRowRendererClass;

#include "wh-video-model-row.h"

GType wh_video_row_renderer_get_type (void);

WHVideoRowRenderer*
wh_video_row_renderer_new (WHVideoModelRow *row);

void
wh_video_row_renderer_set_active (WHVideoRowRenderer *renderer, 
				  gboolean            setting);

G_END_DECLS

#endif /* _WH_VIDEO_ROW_RENDERER */
