#ifndef _HAVE_OPT_H
#define _HAVE_OPT_H

#include <glib.h>
#include <clutter/clutter.h>

typedef struct _OptSlide OptSlide;
typedef struct _OptShow OptShow;
typedef struct _OptTransition OptTransition;
typedef struct _OptMenu OptMenu;

#include "opt-show.h"
#include "opt-slide.h"
#include "opt-transition.h"
#include "opt-menu.h"

gboolean
opt_config_load (OptShow     *show, 
		 const gchar *filename,
		 GError     **error);

#endif
