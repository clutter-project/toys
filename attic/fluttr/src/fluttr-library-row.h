/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-hand.com>
 */


#include <config.h>
#include <glib.h>
#include <clutter/clutter.h>

#include <libnflick/nflick.h>

#ifndef _HAVE_FLUTTR_LIBRARY_ROW_H
#define _HAVE_FLUTTR_LIBRARY_ROW_H

G_BEGIN_DECLS

#define FLUTTR_TYPE_LIBRARY_ROW fluttr_library_row_get_type()

#define FLUTTR_LIBRARY_ROW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	FLUTTR_TYPE_LIBRARY_ROW, \
	FluttrLibraryRow))

#define FLUTTR_LIBRARY_ROW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	FLUTTR_TYPE_LIBRARY_ROW, \
	FluttrLibraryRowClass))

#define FLUTTR_IS_LIBRARY_ROW(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	FLUTTR_TYPE_LIBRARY_ROW))

#define FLUTTR_IS_LIBRARY_ROW_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
	FLUTTR_TYPE_LIBRARY_ROW))

#define FLUTTR_LIBRARY_ROW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	FLUTTR_TYPE_LIBRARY_ROW, \
	FluttrLibraryRowClass))

typedef struct _FluttrLibraryRow FluttrLibraryRow;
typedef struct _FluttrLibraryRowClass FluttrLibraryRowClass;
typedef struct _FluttrLibraryRowPrivate FluttrLibraryRowPrivate;

struct _FluttrLibraryRow
{
	GObject         parent;
	
	/* private */
	FluttrLibraryRowPrivate   *priv;
};

struct _FluttrLibraryRowClass 
{
	/*< private >*/
	GObjectClass parent_class;
}; 

GType                           
fluttr_library_row_get_type (void);

FluttrLibraryRow*
fluttr_library_row_new (gchar *id, gchar *name, NFlickPhotoSet *set);

G_END_DECLS

#endif

