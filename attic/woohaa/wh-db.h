#ifndef _WH_DB
#define _WH_DB

#include <glib-object.h>
#include "wh-video-model-row.h"

G_BEGIN_DECLS

#define WH_TYPE_DB wh_db_get_type()

#define WH_DB(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  WH_TYPE_VIDEO_MODEL, WHVideoModel))

#define WH_DB_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  WH_TYPE_DB, WHDBClass))

#define WH_IS_DB(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  WH_TYPE_DB))

#define WH_IS_DB_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  WH_TYPE_DB))

#define WH_DB_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  WH_TYPE_DB, WHDBClass))

typedef struct {
  GObject                   parent;
} WHDB;

typedef struct {
  GObjectClass parent_class;

  void (*row_created) (WHDB *db, WHVideoModelRow *row);
} WHDBClass;

GType wh_db_get_type (void);

WHDB*
wh_db_new ();

gboolean
wh_db_import_uri (WHDB *db, const gchar *path);

void
wh_db_sync_row (WHVideoModelRow *row);

G_END_DECLS

#endif
