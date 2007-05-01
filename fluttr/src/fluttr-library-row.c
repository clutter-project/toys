/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-`hand.com>
 */


#include "fluttr-library-row.h"

static FluttrLibraryRow*  fluttr_library_row_copy (const FluttrLibraryRow *self);

static void  		  fluttr_library_row_free (FluttrLibraryRow *self);

GType                           
fluttr_library_row_get_type (void)
{
        static GType objecttype = 0;

        if (!objecttype) {
                objecttype = g_boxed_type_register_static 
                        ("FluttrLibraryRow", 
                         (GBoxedCopyFunc) fluttr_library_row_copy,
                         (GBoxedFreeFunc) fluttr_library_row_free);
        }
        
        return objecttype;
}

static FluttrLibraryRow*                
fluttr_library_row_copy (const FluttrLibraryRow *self)
{
        g_return_val_if_fail (self != NULL, NULL);

        FluttrLibraryRow *new = g_new (FluttrLibraryRow, 1);
        
        new->id = (self->id != NULL) ? g_strdup (self->id) : NULL;
        new->name = (self->name != NULL) ? g_strdup (self->name) : NULL;
 
        return new;
}

static void                            
fluttr_library_row_free (FluttrLibraryRow *self)
{
       ;
}

FluttrLibraryRow*
fluttr_library_row_new (gchar *id, gchar *name, NFlickPhotoSet *set)
{
	FluttrLibraryRow *row = g_new0 (FluttrLibraryRow, 1);
	
	row->id = id;
	row->name = name;
	row->set = set;
	
	return row;
}
