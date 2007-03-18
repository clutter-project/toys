#ifndef _WH_BUSY
#define _WH_BUSY

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define WH_TYPE_BUSY wh_busy_get_type()

#define WH_BUSY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  WH_TYPE_BUSY, WHBusy))

#define WH_BUSY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  WH_TYPE_BUSY, WHBusyClass))

#define WH_IS_BUSY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  WH_TYPE_BUSY))

#define WH_IS_BUSY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  WH_TYPE_BUSY))

#define WH_BUSY_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  WH_TYPE_BUSY, WHBusyClass))

typedef struct {
  ClutterActor parent;
} WHBusy;

typedef struct {
  ClutterActorClass parent_class;
} WHBusyClass;

GType wh_busy_get_type (void);

ClutterActor* wh_busy_new (void);

G_END_DECLS

#endif /* _WH_BUSY */
