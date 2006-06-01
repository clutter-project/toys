#ifndef _HAVE_OPT_H
#define _HAVE_OPT_H

#include <glib.h>
#include <clutter/clutter.h>

typedef enum OptTransitionType 
{
  OPT_TRANS_CUBE
}
OptTransitionType;

typedef struct _OptSlide OptSlide;
typedef struct _OptShow OptShow;
typedef struct _OptTransition OptTransition;

#include "opt-show.h"
#include "opt-slide.h"
#include "opt-transition.h"

gboolean
opt_config_load (OptShow     *show, 
		 const gchar *filename,
		 GError     **error);

#endif
