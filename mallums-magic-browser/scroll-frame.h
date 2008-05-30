#ifndef _SCROLL_FRAME
#define _SCROLL_FRAME

#include <glib-object.h>
#include <tidy/tidy.h>

#include <webkit/webkit.h>

G_BEGIN_DECLS

#define SCROLL_TYPE_FRAME scroll_frame_get_type ()

typedef struct _ScrollFramePrivate ScrollFramePrivate;

#define SCROLL_FRAME(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SCROLL_TYPE_FRAME, ScrollFrame))

typedef struct {
  TidyFrame parent;
  ScrollFramePrivate *priv;
} ScrollFrame;

typedef struct {
  TidyFrameClass parent;
} ScrollFrameClass;

GType scroll_frame_get_type (void);

void scroll_frame_add_webkit (ScrollFrame   *frame,
			      WebKitWebView *web_view);

G_END_DECLS

#endif
