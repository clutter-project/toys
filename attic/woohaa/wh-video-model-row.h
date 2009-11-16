/* wh-video-model-row.h */
#ifndef _WH_VIDEO_MODEL_ROW
#define _WH_VIDEO_MODEL_ROW

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

#define WH_TYPE_VIDEO_MODEL_ROW wh_video_model_row_get_type()

#define WH_VIDEO_MODEL_ROW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  WH_TYPE_VIDEO_MODEL_ROW, WHVideoModelRow))

#define WH_VIDEO_MODEL_ROW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  WH_TYPE_VIDEO_MODEL_ROW, WHVideoModelRowClass))

#define WH_IS_VIDEO_MODEL_ROW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  WH_TYPE_VIDEO_MODEL_ROW))

#define WH_IS_VIDEO_MODEL_ROW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  WH_TYPE_VIDEO_MODEL_ROW))

#define WH_VIDEO_MODEL_ROW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  WH_TYPE_VIDEO_MODEL_ROW, WHVideoModelRowClass))

typedef struct {
  GObject parent;
} WHVideoModelRow;

typedef struct {
  GObjectClass parent_class;
} WHVideoModelRowClass;

#include "wh-video-row-renderer.h"

GType wh_video_model_row_get_type (void);

WHVideoModelRow* wh_video_model_row_new (void);

G_CONST_RETURN gchar*
wh_video_model_row_get_path (WHVideoModelRow *row);

void
wh_video_model_row_set_path (WHVideoModelRow *row, const gchar *path);

G_CONST_RETURN gchar*
wh_video_model_row_get_title (WHVideoModelRow *row);

void
wh_video_model_row_set_title (WHVideoModelRow *row, const gchar *title);

gint
wh_video_model_row_get_age (WHVideoModelRow *row);

void
wh_video_model_row_set_age (WHVideoModelRow *row, gint age);

gint
wh_video_model_row_get_n_views (WHVideoModelRow *row);

void
wh_video_model_row_set_n_views (WHVideoModelRow *row, gint n_views);

gint
wh_video_model_row_get_vtime (WHVideoModelRow *row);

void
wh_video_model_row_set_vtime (WHVideoModelRow *row, gint vtime);

void
wh_video_model_row_set_renderer (WHVideoModelRow    *row, 
				 WHVideoRowRenderer *renderer);

WHVideoRowRenderer*
wh_video_model_row_get_renderer (WHVideoModelRow    *row);

void
wh_video_model_row_set_extended_info (WHVideoModelRow *row,
				      const gchar     *series,
				      const gchar     *episode);

void
wh_video_model_row_get_extended_info (WHVideoModelRow *row,
				      gchar          **series,
				      gchar          **episode);

GdkPixbuf*
wh_video_model_row_get_thumbnail (WHVideoModelRow *row);

void
wh_video_model_row_set_thumbnail (WHVideoModelRow *row,
				  GdkPixbuf       *pixbuf);

G_END_DECLS

#endif /* _WH_VIDEO_MODEL_ROW */

