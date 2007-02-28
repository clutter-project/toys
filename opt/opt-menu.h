#ifndef _HAVE_OPT_MENU_H
#define _HAVE_OPT_MENU_H

#include <glib-object.h>

#include "opt.h"

G_BEGIN_DECLS

#define OPT_TYPE_MENU opt_menu_get_type()

#define OPT_MENU(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  OPT_TYPE_MENU, OptMenu))

#define OPT_MENU_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  OPT_TYPE_MENU, OptMenuClass))

#define OPT_IS_MENU(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  OPT_TYPE_MENU))

#define OPT_IS_MENU_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  OPT_TYPE_MENU))

#define OPT_MENU_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  OPT_TYPE_MENU, OptMenuClass))

typedef struct OptMenuPrivate OptMenuPrivate;
typedef struct _OptMenuClass  OptMenuClass;
 
struct _OptMenu
{
  ClutterGroup    parent;
  OptMenuPrivate *priv;
};

struct _OptMenuClass
{
  /*< private >*/
  ClutterGroupClass parent_class;
};

OptMenu *opt_menu_new (OptShow * show);
void opt_menu_add_slide (OptMenu * menu, OptSlide * slide);
void opt_menu_set_current_slide (OptMenu * menu, gint slide_no);
void opt_menu_pop (OptMenu * menu);
void opt_menu_popdown (OptMenu * menu);

G_END_DECLS

#endif
