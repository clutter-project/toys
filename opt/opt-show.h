#ifndef _HAVE_OPT_SHOW_H
#define _HAVE_OPT_SHOW_H

#include <glib-object.h>

#include "opt.h"

G_BEGIN_DECLS

#define OPT_TYPE_SHOW opt_show_get_type()

#define OPT_SHOW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  OPT_TYPE_SHOW, OptShow))

#define OPT_SHOW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  OPT_TYPE_SHOW, OptShowClass))

#define OPT_IS_SHOW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  OPT_TYPE_SHOW))

#define OPT_IS_SHOW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  OPT_TYPE_SHOW))

#define OPT_SHOW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  OPT_TYPE_SHOW, OptShowClass))

typedef struct OptShowPrivate OptShowPrivate;
typedef struct _OptShowClass  OptShowClass;
 
struct _OptShow
{
  GObject              parent;
  OptShowPrivate      *priv;
};

struct _OptShowClass
{
  GObjectClass         parent_class;
};

GType opt_show_get_type (void);

OptShow* 
opt_show_new (void);

void
opt_show_add_slide (OptShow *self, OptSlide *slide);

void
opt_show_run (OptShow *self);

void
opt_show_advance (OptShow *self);

void
opt_show_retreat (OptShow *self);

gboolean
opt_show_export (OptShow *self, const char *path, GError **error);

G_END_DECLS

#endif
