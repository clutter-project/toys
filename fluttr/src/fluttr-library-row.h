/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-`hand.com>
 */


#include <config.h>
#include <glib.h>
#include <clutter/clutter.h>

#include "nflick-photo-set.h"

#ifndef _HAVE_FLUTTR_LIBRARY_ROW_H
#define _HAVE_FLUTTR_LIBRARY_ROW_H

typedef struct _FluttrLibraryRow FluttrLibraryRow;
		
#define        FLUTTR_TYPE_LIBRARY_ROW (fluttr_library_row_get_type ())
#define        FLUTTR_LIBRARY_ROW  (FluttrLibraryRow*)

struct _FluttrLibraryRow {
	gchar 			*id;
	gchar 			*name;
	ClutterActor		*photo;
	
	NFlickPhotoSet		*set;
	
} ;

GType                           
fluttr_library_row_get_type (void);

FluttrLibraryRow*
fluttr_library_row_new (gchar *id, gchar *name, NFlickPhotoSet *set);



#endif
