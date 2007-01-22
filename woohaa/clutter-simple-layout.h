/* clutter-simple-layout.h */

#ifndef _CLUTTER_SIMPLE_LAYOUT
#define _CLUTTER_SIMPLE_LAYOUT

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define CLUTTER_TYPE_SIMPLE_LAYOUT clutter_simple_layout_get_type()

#define CLUTTER_SIMPLE_LAYOUT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  CLUTTER_TYPE_SIMPLE_LAYOUT, ClutterSimpleLayout))

#define CLUTTER_SIMPLE_LAYOUT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  CLUTTER_TYPE_SIMPLE_LAYOUT, ClutterSimpleLayoutClass))

#define CLUTTER_IS_SIMPLE_LAYOUT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  CLUTTER_TYPE_SIMPLE_LAYOUT))

#define CLUTTER_IS_SIMPLE_LAYOUT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  CLUTTER_TYPE_SIMPLE_LAYOUT))

#define CLUTTER_SIMPLE_LAYOUT_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  CLUTTER_TYPE_SIMPLE_LAYOUT, ClutterSimpleLayoutClass))

typedef struct {
  ClutterGroup parent;
} ClutterSimpleLayout;

typedef struct {
  ClutterGroupClass parent_class;
} ClutterSimpleLayoutClass;

GType clutter_simple_layout_get_type (void);

ClutterActor*
clutter_simple_layout_new (void);

void
clutter_simple_layout_fade_out (ClutterSimpleLayout *self, 
				ClutterAlpha        *alpha);

void
clutter_simple_layout_fade_in (ClutterSimpleLayout *self, 
			       ClutterAlpha        *alpha);

G_END_DECLS

#endif /* _CLUTTER_SIMPLE_LAYOUT */
