/*
 * Copyright (C) 2007 Matthew Allum
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Matthew Allum  <mallum@openedhand.com>
 */

#include "fluttr-library.h"
#include <string.h>

G_DEFINE_TYPE (FluttrLibrary, fluttr_library, G_TYPE_OBJECT);

#define LIBRARY_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), FLUTTR_TYPE_LIBRARY, FluttrLibraryPrivate))

typedef struct _FluttrLibraryPrivate FluttrLibraryPrivate;

enum
{
	REORDERED,
	LIBRARY_ROW_CHANGED,
	LIBRARY_ROW_ADDED,
	FILTER,
	LAST_SIGNAL
};

static guint _library_signals[LAST_SIGNAL] = { 0 };

struct _FluttrLibraryPrivate
{
	FluttrFilterRowFunc	filter;
	gpointer		filter_data;
	FluttrCompareRowFunc 	sort;
	gpointer         	sort_data;
	EggSequence     	*library_rows;
};

static void
fluttr_library_get_property (GObject *object, guint property_id,
			     GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, 
							   pspec);
	}
}

static void
fluttr_library_set_property (GObject *object, guint property_id,
				  const GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, 
							   pspec);
	}
}

static void
fluttr_library_dispose (GObject *object)
{
	if (G_OBJECT_CLASS (fluttr_library_parent_class)->dispose)
		G_OBJECT_CLASS (fluttr_library_parent_class)->dispose (object);
}

static void
fluttr_library_finalize (GObject *object)
{
	G_OBJECT_CLASS (fluttr_library_parent_class)->finalize (object);
}

static void
fluttr_library_class_init (FluttrLibraryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (FluttrLibraryPrivate));

	object_class->get_property = fluttr_library_get_property;
	object_class->set_property = fluttr_library_set_property;
	object_class->dispose = fluttr_library_dispose;
	object_class->finalize = fluttr_library_finalize;

	_library_signals[REORDERED] =
		g_signal_new ("library_rows-reordered",
		  	      G_OBJECT_CLASS_TYPE (object_class),
		              G_SIGNAL_RUN_FIRST,
		  	      G_STRUCT_OFFSET (FluttrLibraryClass, reordered),
		  	      NULL, NULL,
		  	      g_cclosure_marshal_VOID__VOID,
		  	      G_TYPE_NONE, 0);

	_library_signals[FILTER] =
		g_signal_new ("filter-changed",
			     G_OBJECT_CLASS_TYPE (object_class),
			     G_SIGNAL_RUN_FIRST,
			     G_STRUCT_OFFSET (FluttrLibraryClass, filter_change),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__VOID,
			     G_TYPE_NONE, 0);

	_library_signals[LIBRARY_ROW_CHANGED] =
		g_signal_new ("library_row-changed",
			     G_OBJECT_CLASS_TYPE (object_class),
			     G_SIGNAL_RUN_FIRST,
			     G_STRUCT_OFFSET (FluttrLibraryClass, library_row_change),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__OBJECT,
			     G_TYPE_NONE, 1, FLUTTR_TYPE_LIBRARY_ROW);

	_library_signals[LIBRARY_ROW_ADDED] =
		g_signal_new ("library_row-added",
			     G_OBJECT_CLASS_TYPE (object_class),
			     G_SIGNAL_RUN_FIRST,
			     G_STRUCT_OFFSET (FluttrLibraryClass, library_row_added),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__OBJECT,
			     G_TYPE_NONE, 1, FLUTTR_TYPE_LIBRARY_ROW);

}

static void
fluttr_library_init (FluttrLibrary *self)
{
	FluttrLibraryPrivate *priv = LIBRARY_PRIVATE(self);

	priv->library_rows = egg_sequence_new (NULL);
}

static gboolean 
check_filter (FluttrLibrary *library, EggSequenceIter *iter)
{
	FluttrLibraryPrivate *priv = LIBRARY_PRIVATE(library);  
	gboolean res;

	if (priv->filter == NULL)
		return TRUE;

	res = priv->filter(library, (FluttrLibraryRow*)egg_sequence_get (iter), 
		     	   priv->filter_data); 
	return res;
}

guint
fluttr_library_row_count (FluttrLibrary *library)
{
	FluttrLibraryPrivate *priv = LIBRARY_PRIVATE(library);  
	EggSequenceIter     *iter;
	gint                 n = 0;

	if (priv->filter == NULL)
		return egg_sequence_get_length (priv->library_rows);    

	iter = egg_sequence_get_begin_iter (priv->library_rows);

	while (!egg_sequence_iter_is_end (iter)) {
		if (check_filter (library, iter))
			n++;
		iter = egg_sequence_iter_next (iter);
	}

	return n;
}

FluttrLibraryRow*
fluttr_library_get_library_row (FluttrLibrary *library, gint index)
{
	FluttrLibraryPrivate *priv = LIBRARY_PRIVATE(library);
	EggSequenceIter     *iter;
	gint                 n = 0;

	if (priv->filter == NULL)
		return (FluttrLibraryRow*)egg_sequence_get 
			     (egg_sequence_get_iter_at_pos (priv->library_rows, index));

	iter = egg_sequence_get_begin_iter (priv->library_rows);

	while (!egg_sequence_iter_is_end (iter)) {
		if (check_filter (library, iter)) {
			if (n == index)
				return (FluttrLibraryRow*)egg_sequence_get (iter);
			n++;
		}
		iter = egg_sequence_iter_next (iter);
	}

	return NULL;
}

static void
on_library_row_changed (GObject *obj, GParamSpec   *arg1,
				   gpointer      data)
{
	return;
	FluttrLibrary        *library = FLUTTR_LIBRARY(data);
	FluttrLibraryPrivate *priv;

	priv = LIBRARY_PRIVATE(library);

	/* thumbnail changing does not effect ordering */
	if (!strcmp(g_param_spec_get_name(arg1), "thumbnail"))
		return;

	if (priv->sort) {
		egg_sequence_sort (priv->library_rows, 
			  	(GCompareDataFunc)priv->sort, priv->sort_data);
		g_signal_emit (library, _library_signals[REORDERED], 0);
	}

	g_signal_emit (library, _library_signals[LIBRARY_ROW_CHANGED], 0, 
		 					FLUTTR_LIBRARY_ROW(obj));
} 

void
fluttr_library_append_library_row (FluttrLibrary *library, FluttrLibraryRow *library_row)
{
	FluttrLibraryPrivate *priv = LIBRARY_PRIVATE(library);
	EggSequenceIter     *iter;

	g_object_ref (library_row);

	if (priv->sort)
		iter = egg_sequence_insert_sorted (priv->library_rows, (gpointer)library_row,
				     	   	  (GCompareDataFunc)priv->sort,
				       	   	   priv->sort_data);
	else
		iter = egg_sequence_append (priv->library_rows, (gpointer)library_row);

	if (check_filter (library, iter))
		g_signal_emit (library, _library_signals[LIBRARY_ROW_ADDED], 0, library_row);
}


void
fluttr_library_foreach (FluttrLibrary      *library, 
		        FluttrForeachRowFunc   func,
			gpointer           data)
{
	FluttrLibraryPrivate *priv = LIBRARY_PRIVATE(library);
	EggSequenceIter     *iter;

	iter = egg_sequence_get_begin_iter (priv->library_rows);

	while (!egg_sequence_iter_is_end (iter)) {
		if (check_filter (library, iter))
			if (func (library, 
			    	  (FluttrLibraryRow*)egg_sequence_get (iter),
		                  data) == FALSE)
	  			return;
	
		iter = egg_sequence_iter_next (iter);
	}
}

void
fluttr_library_set_sort_func (FluttrLibrary     *library, 
			      FluttrCompareRowFunc  func, 
			      gpointer          userdata)
{
	FluttrLibraryPrivate *priv = LIBRARY_PRIVATE(library);

	priv->sort      = func;
	priv->sort_data = userdata;

	if (func) {
		egg_sequence_sort (priv->library_rows, (GCompareDataFunc)func, userdata);
		g_signal_emit (library, _library_signals[REORDERED], 0);
	}
}

void
fluttr_library_set_filter (FluttrLibrary    *library,
			   FluttrFilterRowFunc  filter, 
			   gpointer         data)
{
	FluttrLibraryPrivate *priv = LIBRARY_PRIVATE(library);
	FluttrFilterRowFunc      prev_filter;

	prev_filter = priv->filter;

	priv->filter      = filter;
	priv->filter_data = data;

	if (prev_filter != priv->filter)
		g_signal_emit (library, _library_signals[FILTER], 0);
}

FluttrLibrary*
fluttr_library_new ()
{
	return g_object_new (FLUTTR_TYPE_LIBRARY, NULL);
}

