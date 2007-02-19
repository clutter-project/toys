#ifndef _WH_DB
#define _WH_DB

#include <glib.h>
#include <sqlite3.h>
#include "wh-video-model.h"
#include "wh-video-model-row.h"

G_BEGIN_DECLS

void
wh_db_populate_model (sqlite3 *db, WHVideoModel *model);

void
wh_db_sync_row (sqlite3 *db, WHVideoModelRow *row);

sqlite3*
wh_db_init (const char *paths);

G_END_DECLS

#endif
