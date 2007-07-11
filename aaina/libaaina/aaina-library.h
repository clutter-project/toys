/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Matthew Allum  <mallum@openedhand.com>
 */

#ifndef _AAINA_LIBRARY
#define _AAINA_LIBRARY

#include <glib-object.h>
#include "aaina-photo.h"
#include "eggsequence.h"

G_BEGIN_DECLS

#define AAINA_TYPE_LIBRARY aaina_library_get_type()

#define AAINA_LIBRARY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	AAINA_TYPE_LIBRARY, \
	AainaLibrary))

#define AAINA_LIBRARY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	AAINA_TYPE_LIBRARY, \
	AainaLibraryClass))

#define AAINA_IS_LIBRARY(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	AAINA_TYPE_LIBRARY))

#define AAINA_IS_LIBRARY_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
	AAINA_TYPE_LIBRARY))

#define AAINA_LIBRARY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	AAINA_TYPE_LIBRARY, \
	AainaLibraryClass))

typedef struct {
	GObject		parent;

} AainaLibrary;

typedef struct {
	GObjectClass parent_class;

	void (*reordered) (AainaLibrary *library);
	void (*filter_change) (AainaLibrary *library);
	void (*photo_change) (AainaLibrary *library, AainaPhoto *photo);
	void (*photo_added) (AainaLibrary *library, AainaPhoto *photo);

} AainaLibraryClass;

typedef gint (*AainaCompareRowFunc) (AainaPhoto *a,
				     AainaPhoto *b,
				     gpointer        data);

typedef gboolean  (*AainaFilterRowFunc) (AainaLibrary    *library,
				         AainaPhoto *photo,
				         gpointer         data);

typedef gboolean (*AainaForeachRowFunc) (AainaLibrary    *library,
				         AainaPhoto *photo,
				         gpointer         data);

GType aaina_library_get_type (void);

AainaLibrary*
aaina_library_new ();

guint
aaina_library_photo_count (AainaLibrary *library);

AainaPhoto*
aaina_library_get_photo (AainaLibrary *library, gint index);

void
aaina_library_append_photo (AainaLibrary *library, AainaPhoto *photo);

void
aaina_library_remove_photo (AainaLibrary *library, const AainaPhoto *photo);

void
aaina_library_set_filter (AainaLibrary    *library,
			  AainaFilterRowFunc  filter, 
			  gpointer         data);

void
aaina_library_set_sort_func (AainaLibrary     *library, 
			     AainaCompareRowFunc  func, 
			     gpointer          userdata);

void
aaina_library_foreach (AainaLibrary      *library, 
		      AainaForeachRowFunc   func,
		      gpointer           data);

gboolean
aaina_library_get_pending (AainaLibrary *library);
void
aaina_library_set_pending (AainaLibrary *library, gboolean pending);

gboolean
aaina_library_is_full (AainaLibrary *library);
void
aaina_library_set_max (AainaLibrary *library, gint max_photos);

G_END_DECLS

#endif
