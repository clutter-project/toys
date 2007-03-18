#ifndef _WH_VIDEO_MODEL
#define _WH_VIDEO_MODEL

#include <clutter/clutter.h>
#include <libgnomevfs/gnome-vfs.h>
#include <glib-object.h>
#include "wh-video-model-row.h"
#include "eggsequence.h"

G_BEGIN_DECLS

#define WH_TYPE_VIDEO_MODEL wh_video_model_get_type()

#define WH_VIDEO_MODEL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  WH_TYPE_VIDEO_MODEL, WHVideoModel))

#define WH_VIDEO_MODEL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  WH_TYPE_VIDEO_MODEL, WHVideoModelClass))

#define CLUTTER_IS_VIDEO_MODEL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  WH_TYPE_VIDEO_MODEL))

#define CLUTTER_IS_VIDEO_MODEL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  WH_TYPE_VIDEO_MODEL))

#define WH_VIDEO_MODEL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  WH_TYPE_VIDEO_MODEL, WHVideoModelClass))

typedef struct {
  GObject                   parent;
} WHVideoModel;

typedef struct {
  GObjectClass parent_class;

  void (*reordered) (WHVideoModel *model);
  void (*filter_change) (WHVideoModel *model);
  void (*row_change) (WHVideoModel *model, WHVideoModelRow *row);
  void (*row_added) (WHVideoModel *model, WHVideoModelRow *row);

} WHVideoModelClass;

typedef gint (*WHCompareRowFunc) (WHVideoModelRow *a,
				  WHVideoModelRow *b,
				  gpointer        data);

typedef gboolean  (*WHFilterRowFunc) (WHVideoModel    *model,
				      WHVideoModelRow *row,
				      gpointer         data);

typedef void (*WHForeachRowFunc) (WHVideoModel    *model,
				  WHVideoModelRow *row,
				  gpointer         data);

GType wh_video_model_get_type (void);

WHVideoModel*
wh_video_model_new ();

guint
wh_video_model_row_count (WHVideoModel *model);

WHVideoModelRow*
wh_video_model_get_row (WHVideoModel *model, gint index);

void
wh_video_model_append_row (WHVideoModel *model, WHVideoModelRow *row);

void
wh_video_model_set_filter (WHVideoModel    *model,
			   WHFilterRowFunc  filter, 
			   gpointer         data);

void
wh_video_model_set_sort_func (WHVideoModel     *model, 
			      WHCompareRowFunc  func, 
			      gpointer          userdata);

void
wh_video_model_foreach (WHVideoModel      *model, 
		        WHForeachRowFunc   func,
			gpointer           data);

G_END_DECLS

#endif
