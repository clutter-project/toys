#ifndef _CLUTTER_VIDEO_MODEL
#define _CLUTTER_VIDEO_MODEL

#include <clutter/clutter.h>
#include <libgnomevfs/gnome-vfs.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define CLUTTER_TYPE_VIDEO_MODEL clutter_video_model_get_type()

#define CLUTTER_VIDEO_MODEL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  CLUTTER_TYPE_VIDEO_MODEL, ClutterVideoModel))

#define CLUTTER_VIDEO_MODEL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  CLUTTER_TYPE_VIDEO_MODEL, ClutterVideoModelClass))

#define CLUTTER_IS_VIDEO_MODEL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  CLUTTER_TYPE_VIDEO_MODEL))

#define CLUTTER_IS_VIDEO_MODEL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  CLUTTER_TYPE_VIDEO_MODEL))

#define CLUTTER_VIDEO_MODEL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  CLUTTER_TYPE_VIDEO_MODEL, ClutterVideoModelClass))

typedef struct {
  GObject                   parent;
} ClutterVideoModel;

typedef struct {
  GObjectClass parent_class;
} ClutterVideoModelClass;

typedef struct ClutterVideoModelItem
{
  gchar        *uri;
  time_t        mtime;
  ClutterActor *group, *title, *image;
}
ClutterVideoModelItem;

typedef void (*ClutterVideoModelFunc) (ClutterVideoModel     *model,
				       ClutterVideoModelItem *item,
				       gpointer               user_data);

GType clutter_video_model_get_type (void);

ClutterVideoModel*
clutter_video_model_new (void);

void
clutter_video_model_view_foreach (ClutterVideoModel     *model,
				  ClutterVideoModelFunc  func,
				  gpointer               userdata);

guint
clutter_video_model_item_count (ClutterVideoModel *model);

ClutterVideoModelItem*
clutter_video_model_get_item (ClutterVideoModel *model, gint index);

void
clutter_video_model_set_cell_size (ClutterVideoModel *model,
				   gint               width,
				   gint               height);

G_END_DECLS

#endif
