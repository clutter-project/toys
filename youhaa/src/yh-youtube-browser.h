
#ifndef _YH_YOUTUBE_BROWSER_H
#define _YH_YOUTUBE_BROWSER_H

#include <glib-object.h>
#include <clutter/clutter.h>
#include "yh-youtube.h"

G_BEGIN_DECLS

#define YH_TYPE_YOUTUBE_BROWSER yh_youtube_browser_get_type()

#define YH_YOUTUBE_BROWSER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  YH_TYPE_YOUTUBE_BROWSER, YHYoutubeBrowser))

#define YH_YOUTUBE_BROWSER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  YH_TYPE_YOUTUBE_BROWSER, YHYoutubeBrowserClass))

#define YH_IS_YOUTUBE_BROWSER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  YH_TYPE_YOUTUBE_BROWSER))

#define YH_IS_YOUTUBE_BROWSER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  YH_TYPE_YOUTUBE_BROWSER))

#define YH_YOUTUBE_BROWSER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  YH_TYPE_YOUTUBE_BROWSER, YHYoutubeBrowserClass))

typedef struct _YHYoutubeBrowser         YHYoutubeBrowser;
typedef struct _YHYoutubeBrowserClass    YHYoutubeBrowserClass;
typedef struct _YHYoutubeBrowserPrivate  YHYoutubeBrowserPrivate;

struct _YHYoutubeBrowser {
  ClutterActor             parent;
  
  YHYoutubeBrowserPrivate *priv;
};

struct _YHYoutubeBrowserClass {
  ClutterActorClass parent_class;

  /* Signals */
  void (* related)  (YHYoutubeBrowser *browser, ClutterModelIter *iter);
};

GType yh_youtube_browser_get_type (void);

ClutterActor *
yh_youtube_browser_new (ClutterModel *model, YHYoutube *youtube);

G_END_DECLS

#endif /* _YH_YOUTUBE_BROWSER_H */

