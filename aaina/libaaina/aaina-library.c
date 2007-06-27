/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Matthew Allum  <mallum@openedhand.com>
 */

#include "aaina-library.h"
#include <string.h>

G_DEFINE_TYPE (AainaLibrary, aaina_library, G_TYPE_OBJECT);

#define LIBRARY_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), AAINA_TYPE_LIBRARY, AainaLibraryPrivate))

typedef struct _AainaLibraryPrivate AainaLibraryPrivate;

enum
{
	REORDERED,
	PHOTO_CHANGED,
	PHOTO_ADDED,
	FILTER,
	LAST_SIGNAL
};

static guint _library_signals[LAST_SIGNAL] = { 0 };

struct _AainaLibraryPrivate
{
	AainaFilterRowFunc	filter;
	gpointer		filter_data;
	AainaCompareRowFunc 	sort;
	gpointer         	sort_data;
	EggSequence     	*photos;
};

static void
aaina_library_get_property (GObject *object, guint property_id,
			     GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, 
							   pspec);
	}
}

static void
aaina_library_set_property (GObject *object, guint property_id,
				  const GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, 
							   pspec);
	}
}

static void
aaina_library_dispose (GObject *object)
{
	if (G_OBJECT_CLASS (aaina_library_parent_class)->dispose)
		G_OBJECT_CLASS (aaina_library_parent_class)->dispose (object);
}

static void
aaina_library_finalize (GObject *object)
{
	G_OBJECT_CLASS (aaina_library_parent_class)->finalize (object);
}

static void
aaina_library_class_init (AainaLibraryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (AainaLibraryPrivate));

	object_class->get_property = aaina_library_get_property;
	object_class->set_property = aaina_library_set_property;
	object_class->dispose = aaina_library_dispose;
	object_class->finalize = aaina_library_finalize;

	_library_signals[REORDERED] =
		g_signal_new ("photos-reordered",
		  	      G_OBJECT_CLASS_TYPE (object_class),
		              G_SIGNAL_RUN_FIRST,
		  	      G_STRUCT_OFFSET (AainaLibraryClass, reordered),
		  	      NULL, NULL,
		  	      g_cclosure_marshal_VOID__VOID,
		  	      G_TYPE_NONE, 0);

	_library_signals[FILTER] =
		g_signal_new ("filter-changed",
			     G_OBJECT_CLASS_TYPE (object_class),
			     G_SIGNAL_RUN_FIRST,
			     G_STRUCT_OFFSET (AainaLibraryClass, filter_change),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__VOID,
			     G_TYPE_NONE, 0);

	_library_signals[PHOTO_CHANGED] =
		g_signal_new ("photo-changed",
			     G_OBJECT_CLASS_TYPE (object_class),
			     G_SIGNAL_RUN_FIRST,
			     G_STRUCT_OFFSET (AainaLibraryClass, photo_change),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__OBJECT,
			     G_TYPE_NONE, 1, AAINA_TYPE_PHOTO);

	_library_signals[PHOTO_ADDED] =
		g_signal_new ("photo-added",
			     G_OBJECT_CLASS_TYPE (object_class),
			     G_SIGNAL_RUN_FIRST,
			     G_STRUCT_OFFSET (AainaLibraryClass, photo_added),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__OBJECT,
			     G_TYPE_NONE, 1, AAINA_TYPE_PHOTO);

}

static void
aaina_library_init (AainaLibrary *self)
{
	AainaLibraryPrivate *priv = LIBRARY_PRIVATE(self);

	priv->photos = egg_sequence_new (NULL);
}

static gboolean 
check_filter (AainaLibrary *library, EggSequenceIter *iter)
{
	AainaLibraryPrivate *priv = LIBRARY_PRIVATE(library);  
	gboolean res;

	if (priv->filter == NULL)
		return TRUE;

	res = priv->filter(library, (AainaPhoto*)egg_sequence_get (iter), 
		     	   priv->filter_data); 
	return res;
}

guint
aaina_library_photo_count (AainaLibrary *library)
{
	AainaLibraryPrivate *priv = LIBRARY_PRIVATE(library);  
	EggSequenceIter     *iter;
	gint                 n = 0;

	if (priv->filter == NULL)
		return egg_sequence_get_length (priv->photos);    

	iter = egg_sequence_get_begin_iter (priv->photos);

	while (!egg_sequence_iter_is_end (iter)) {
		if (check_filter (library, iter))
			n++;
		iter = egg_sequence_iter_next (iter);
	}

	return n;
}

AainaPhoto*
aaina_library_get_photo (AainaLibrary *library, gint index)
{
	AainaLibraryPrivate *priv = LIBRARY_PRIVATE(library);
	EggSequenceIter     *iter;
	gint                 n = 0;

	if (priv->filter == NULL)
		return (AainaPhoto*)egg_sequence_get 
			     (egg_sequence_get_iter_at_pos (priv->photos, index));

	iter = egg_sequence_get_begin_iter (priv->photos);

	while (!egg_sequence_iter_is_end (iter)) {
		if (check_filter (library, iter)) {
			if (n == index)
				return (AainaPhoto*)egg_sequence_get (iter);
			n++;
		}
		iter = egg_sequence_iter_next (iter);
	}

	return NULL;
}

static void
on_photo_changed (GObject *obj, GParamSpec   *arg1,
				   gpointer      data)
{
	return;
	AainaLibrary        *library = AAINA_LIBRARY(data);
	AainaLibraryPrivate *priv;

	priv = LIBRARY_PRIVATE(library);

	/* thumbnail changing does not effect ordering */
	if (!strcmp(g_param_spec_get_name(arg1), "thumbnail"))
		return;

	if (priv->sort) {
		egg_sequence_sort (priv->photos, 
			  	(GCompareDataFunc)priv->sort, priv->sort_data);
		g_signal_emit (library, _library_signals[REORDERED], 0);
	}

	g_signal_emit (library, _library_signals[PHOTO_CHANGED], 0, 
		 					AAINA_PHOTO(obj));
} 

void
aaina_library_append_photo (AainaLibrary *library, AainaPhoto *photo)
{
	AainaLibraryPrivate *priv = LIBRARY_PRIVATE(library);
	EggSequenceIter     *iter;

	g_signal_connect (photo, "notify", G_CALLBACK (on_photo_changed), library);

	g_object_ref (photo);

	if (priv->sort)
		iter = egg_sequence_insert_sorted (priv->photos, (gpointer)photo,
				     	   	  (GCompareDataFunc)priv->sort,
				       	   	   priv->sort_data);
	else
		iter = egg_sequence_append (priv->photos, (gpointer)photo);

	if (check_filter (library, iter))
		g_signal_emit (library, _library_signals[PHOTO_ADDED], 0, photo);
}


void
aaina_library_foreach (AainaLibrary      *library, 
		        AainaForeachRowFunc   func,
			gpointer           data)
{
	AainaLibraryPrivate *priv = LIBRARY_PRIVATE(library);
	EggSequenceIter     *iter;

	iter = egg_sequence_get_begin_iter (priv->photos);

	while (!egg_sequence_iter_is_end (iter)) {
		if (check_filter (library, iter))
			if (func (library, 
			    	  (AainaPhoto*)egg_sequence_get (iter),
		                  data) == FALSE)
	  			return;
	
		iter = egg_sequence_iter_next (iter);
	}
}

void
aaina_library_set_sort_func (AainaLibrary     *library, 
			      AainaCompareRowFunc  func, 
			      gpointer          userdata)
{
	AainaLibraryPrivate *priv = LIBRARY_PRIVATE(library);

	priv->sort      = func;
	priv->sort_data = userdata;

	if (func) {
		egg_sequence_sort (priv->photos, (GCompareDataFunc)func, userdata);
		g_signal_emit (library, _library_signals[REORDERED], 0);
	}
}

void
aaina_library_set_filter (AainaLibrary    *library,
			   AainaFilterRowFunc  filter, 
			   gpointer         data)
{
	AainaLibraryPrivate *priv = LIBRARY_PRIVATE(library);
	AainaFilterRowFunc      prev_filter;

	prev_filter = priv->filter;

	priv->filter      = filter;
	priv->filter_data = data;

	if (prev_filter != priv->filter)
		g_signal_emit (library, _library_signals[FILTER], 0);
}

AainaLibrary*
aaina_library_new ()
{
	return g_object_new (AAINA_TYPE_LIBRARY, NULL);
}

