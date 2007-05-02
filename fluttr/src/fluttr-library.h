/*
 * Copyright (C) 2007 Matthew Allum
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Matthew Allum  <mallum@openedhand.com>
 */
#ifndef _FLUTTR_LIBRARY
#define _FLUTTR_LIBRARY

#include <clutter/clutter.h>
//#include <libgnomevfs/gnome-vfs.h>
#include <glib-object.h>
#include "fluttr-library-row.h"
#include "eggsequence.h"

G_BEGIN_DECLS

#define FLUTTR_TYPE_LIBRARY fluttr_library_get_type()

#define FLUTTR_LIBRARY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	FLUTTR_TYPE_LIBRARY, \
	FluttrLibrary))

#define FLUTTR_LIBRARY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	FLUTTR_TYPE_LIBRARY, \
	FluttrLibraryClass))

#define FLUTTR_IS_LIBRARY(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	FLUTTR_TYPE_LIBRARY))

#define FLUTTR_IS_LIBRARY_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
	FLUTTR_TYPE_LIBRARY))

#define FLUTTR_LIBRARY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	FLUTTR_TYPE_LIBRARY, \
	FluttrLibraryClass))

typedef struct {
	GObject		parent;

} FluttrLibrary;

typedef struct {
	GObjectClass parent_class;

	void (*reordered) (FluttrLibrary *library);
	void (*filter_change) (FluttrLibrary *library);
	void (*library_row_change) (FluttrLibrary *library, FluttrLibraryRow *library_row);
	void (*library_row_added) (FluttrLibrary *library, FluttrLibraryRow *library_row);

} FluttrLibraryClass;

typedef gint (*FluttrCompareRowFunc) (FluttrLibraryRow *a,
				     FluttrLibraryRow *b,
				     gpointer        data);

typedef gboolean  (*FluttrFilterRowFunc) (FluttrLibrary    *library,
				         FluttrLibraryRow *library_row,
				         gpointer         data);

typedef gboolean (*FluttrForeachRowFunc) (FluttrLibrary    *library,
				         FluttrLibraryRow *library_row,
				         gpointer         data);

GType fluttr_library_get_type (void);

FluttrLibrary*
fluttr_library_new ();

guint
fluttr_library_row_count (FluttrLibrary *library);

FluttrLibraryRow*
fluttr_library_get_library_row (FluttrLibrary *library, gint index);

void
fluttr_library_append_library_row (FluttrLibrary *library, FluttrLibraryRow *library_row);

void
fluttr_library_set_filter (FluttrLibrary    *library,
			  FluttrFilterRowFunc  filter, 
			  gpointer         data);

void
fluttr_library_set_sort_func (FluttrLibrary     *library, 
			     FluttrCompareRowFunc  func, 
			     gpointer          userdata);

void
fluttr_library_foreach (FluttrLibrary      *library, 
		      FluttrForeachRowFunc   func,
		      gpointer           data);

G_END_DECLS

#endif
