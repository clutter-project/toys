#ifndef _HAVE_OPT_SLIDE_H
#define _HAVE_OPT_SLIDE_H

#include <glib-object.h>

#include "opt.h"

G_BEGIN_DECLS

#define OPT_TYPE_SLIDE opt_slide_get_type()

#define OPT_SLIDE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  OPT_TYPE_SLIDE, OptSlide))

#define OPT_SLIDE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  OPT_TYPE_SLIDE, OptSlideClass))

#define OPT_IS_SLIDE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  OPT_TYPE_SLIDE))

#define OPT_IS_SLIDE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
   OPT_TYPE_SLIDE))

#define OPT_SLIDE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  OPT_TYPE_SLIDE, OptSlideClass))

typedef struct OptSlidePrivate OptSlidePrivate;
typedef struct _OptSlideClass  OptSlideClass;
 
struct _OptSlide
{
  ClutterGroup          parent;
  OptSlidePrivate      *priv;
};

struct _OptSlideClass
{
  ClutterGroupClass     parent_class;
};

GType opt_slide_get_type (void);

OptSlide* 
opt_slide_new (OptShow *show);

void
opt_slide_set_title (OptSlide    *slide, 
		     const gchar *title,
		     const gchar *font,
		     ClutterColor col);

void
opt_slide_add_bullet_text_item (OptSlide    *slide, 
				const gchar *title,
				const gchar *font,
				ClutterColor col);

void
opt_slide_add_bullet (OptSlide *slide, ClutterElement *element);

const ClutterElement*
opt_slide_get_title (OptSlide *slide);

GList*
opt_slide_get_bullets (OptSlide *slide);

void
opt_slide_set_transition (OptSlide *slide, OptTransition *trans);

OptTransition*
opt_slide_get_transition (OptSlide *slide);

G_END_DECLS

#endif
