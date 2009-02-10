#ifndef _MM_BROWSER
#define _MM_BROWSER

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define MM_TYPE_BROWSER mm_browser_get_type ()

typedef struct _MmBrowserPrivate MmBrowserPrivate;

#define MM_BROWSER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), MM_TYPE_BROWSER, MmBrowser))

typedef struct {
  ClutterGroup parent;
  MmBrowserPrivate *priv;
} MmBrowser;

typedef struct {
  ClutterGroupClass parent_class;
} MmBrowserClass;

GType mm_browser_get_type (void);

MmBrowser *mm_browser_new (void);
void       mm_browser_open (MmBrowser  *browser,
                            const char *address);

G_END_DECLS

#endif
