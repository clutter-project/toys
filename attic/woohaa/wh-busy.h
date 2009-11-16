#ifndef _HAVE_WOOHAA_BUSY_H
#define _HAVE_WOOHAA_BUSY_H

#include <clutter/clutter.h>
#include <glib-object.h>

G_BEGIN_DECLS
#define WOOHAA_TYPE_BUSY woohaa_busy_get_type()

#define WOOHAA_BUSY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  WOOHAA_TYPE_BUSY, WoohaaBusy))

#define WOOHAA_BUSY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  WOOHAA_TYPE_BUSY, WoohaaBusyClass))

#define WOOHAA_IS_BUSY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  WOOHAA_TYPE_BUSY))

#define WOOHAA_IS_BUSY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  WOOHAA_TYPE_BUSY))

#define WOOHAA_BUSY_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  WOOHAA_TYPE_BUSY, WoohaaBusyClass))

typedef struct _WoohaaBusy        WoohaaBusy;
typedef struct _WoohaaBusyClass   WoohaaBusyClass;
typedef struct _WoohaaBusyPrivate WoohaaBusyPrivate;

struct _WoohaaBusy
{
  /*< private >*/
  ClutterActor       parent;
  WoohaaBusyPrivate *priv;
}; 

struct _WoohaaBusyClass 
{
  /*< private >*/
  ClutterGroupClass parent_class;

  /* Future padding */
  void (* __reserved1) (void);
  void (* __reserved2) (void);
  void (* __reserved3) (void);
  void (* __reserved4) (void);
  void (* __reserved5) (void);
  void (* __reserved6) (void);
}; 

GType         woohaa_busy_get_type  (void) G_GNUC_CONST;
ClutterActor *woohaa_busy_new       (void);
void          woohaa_busy_fade_out (WoohaaBusy *busy, gint timeout);
void          woohaa_busy_fade_in  (WoohaaBusy *busy, gint timeout);
void          woohaa_busy_bounce   (WoohaaBusy *busy);
G_END_DECLS

#endif
