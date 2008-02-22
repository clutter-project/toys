
#ifndef _YH_YOUTUBE_H
#define _YH_YOUTUBE_H

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define YH_TYPE_YOUTUBE yh_youtube_get_type()

#define YH_YOUTUBE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  YH_TYPE_YOUTUBE, YHYoutube))

#define YH_YOUTUBE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  YH_TYPE_YOUTUBE, YHYoutubeClass))

#define YH_IS_YOUTUBE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  YH_TYPE_YOUTUBE))

#define YH_IS_YOUTUBE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  YH_TYPE_YOUTUBE))

#define YH_YOUTUBE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  YH_TYPE_YOUTUBE, YHYoutubeClass))

enum {
  YH_YOUTUBE_COL_TITLE,
  YH_YOUTUBE_COL_AUTHOR,
  YH_YOUTUBE_COL_DESCRIPTION,
  YH_YOUTUBE_COL_RATING,
  YH_YOUTUBE_COL_THUMBS,
  YH_YOUTUBE_COL_MIMES,
  YH_YOUTUBE_COL_URIS,
  
  YH_YOUTUBE_COL_LAST
};

typedef struct _YHYoutube         YHYoutube;
typedef struct _YHYoutubeClass    YHYoutubeClass;
typedef struct _YHYoutubePrivate  YHYoutubePrivate;

struct _YHYoutube {
  GObject           parent;
  
  YHYoutubePrivate *priv;
};

struct _YHYoutubeClass {
  GObjectClass parent_class;
  
  /* Signals */
  void (* complete)  (YHYoutube *youtube, void *handle);
  void (* model)     (YHYoutube *youtube, ClutterModel *model);
  void (* thumbnail) (YHYoutube *youtube, GdkPixbuf *pixbuf);
  void (* link)      (YHYoutube *youtube, const gchar *url);
};

GType yh_youtube_get_type (void);

YHYoutube *
yh_youtube_get_default ();

void *yh_youtube_query         (YHYoutube *youtube, const gchar *search_string);
void *yh_youtube_get_thumb     (YHYoutube *youtube, const gchar *url);
void  yh_youtube_cancel        (YHYoutube *youtube, void        *handle);
void *yh_youtube_get_http_link (YHYoutube *youtube, const gchar *url);

G_END_DECLS

#endif /* _YH_YOUTUBE_H */

