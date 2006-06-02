#ifndef _HAVE_OPT_TRANSITION_H
#define _HAVE_OPT_TRANSITION_H

#include <glib-object.h>

#include "opt.h"

G_BEGIN_DECLS

#define OPT_TYPE_TRANSITION opt_transition_get_type()

#define OPT_TRANSITION(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  OPT_TYPE_TRANSITION, OptTransition))

#define OPT_TRANSITION_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  OPT_TYPE_TRANSITION, OptTransitionClass))

#define OPT_IS_TRANSITION(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  OPT_TYPE_TRANSITION))

#define OPT_IS_TRANSITION_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  OPT_TYPE_TRANSITION))

#define OPT_TRANSITION_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  OPT_TYPE_TRANSITION, OptTransitionClass))

typedef enum OptTransitionStyle
{
  OPT_TRANSITION_ANY,
  OPT_TRANSITION_CUBE,
  OPT_TRANSITION_FLIP,
  OPT_TRANSITION_YZ_FLIP,
  OPT_TRANSITION_FADE
}
OptTransitionStyle;

typedef struct OptTransitionPrivate OptTransitionPrivate;
typedef struct _OptTransitionClass  OptTransitionClass;
 
struct _OptTransition
{
  ClutterTimeline       parent;
  OptTransitionPrivate *priv;
};

struct _OptTransitionClass
{
  ClutterTimelineClass         parent_class;
};

GType opt_transition_get_type (void);

OptTransition*
opt_transition_new (OptTransitionStyle style);

OptTransitionStyle
opt_transition_get_style (OptTransition     *trans);

void
opt_transition_set_style (OptTransition     *trans,
			  OptTransitionStyle style);
void
opt_transition_set_from (OptTransition *trans, OptSlide *slide);

void
opt_transition_set_to (OptTransition *trans, OptSlide *slide);

OptSlide*
opt_transition_get_from (OptTransition *trans);

OptSlide*
opt_transition_get_to (OptTransition *trans);

G_END_DECLS

#endif
