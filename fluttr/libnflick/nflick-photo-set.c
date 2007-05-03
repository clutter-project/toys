/******************************************************************************/
/*                                                                            */
/* GPL license, Copyright (c) 2005-2006 by:                                   */
/*                                                                            */
/* Authors:                                                                   */
/*      Michael Dominic K. <michaldominik@gmail.com>                          */
/*                                                                            */
/* This program is free software; you can redistribute it and/or modify it    */
/* under the terms of the GNU General Public License as published by the      */
/* Free Software Foundation; either version 2, or (at your option) any later  */
/* version.                                                                   */
/*                                                                            */
/* This program is distributed in the hope that it will be useful, but        */
/* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY */
/* or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   */
/* for more details.                                                          */
/*                                                                            */
/* You should have received a copy of the GNU General Public License along    */
/* with this program; if not, write to the Free Software Foundation, Inc., 59 */
/* Temple Place - Suite 330, Boston, MA 02111-1307, USA.                      */
/*                                                                            */
/******************************************************************************/

#include "nflick-photo-set.h"
#include "nflick-photo-set-private.h"

GType                           nflick_photo_set_get_type (void)
{
        static GType objecttype = 0;

        if (!objecttype) {

                static const GTypeInfo objectinfo = {
                        sizeof (NFlickPhotoSetClass), 
                        NULL, 
                        NULL,
                        (GClassInitFunc) nflick_photo_set_class_init,
                        NULL,
                        NULL, 
                        sizeof (NFlickPhotoSet), 
                        4, 
                        (GInstanceInitFunc) nflick_photo_set_init,
                };
                objecttype = g_type_register_static (G_TYPE_OBJECT, "NFlickPhotoSet",
                                                     &objectinfo, 0);
        }
        return objecttype;
}

static void                     nflick_photo_set_class_init (NFlickPhotoSetClass *klass)
{
        GObjectClass *gobjectclass = (GObjectClass *) klass;

        gobjectclass->dispose = (gpointer) nflick_photo_set_dispose;
        gobjectclass->finalize = (gpointer) nflick_photo_set_finalize;
        gobjectclass->get_property = (gpointer) nflick_photo_set_get_property;
        
        g_object_class_install_property (gobjectclass, ARG_COMBO_TEXT,
                                         g_param_spec_string
                                         ("combotext", "ComboText", "A text to put in combobox",
                                         NULL, G_PARAM_READABLE));

        g_object_class_install_property (gobjectclass, ARG_ID,
                                         g_param_spec_string
                                         ("id", "Id", "Photoset id",
                                         NULL, G_PARAM_READABLE));

        g_object_class_install_property (gobjectclass, ARG_COUNT,
                                         g_param_spec_int 
                                         ("count", "Count", "Number of items",
                                         -5000, 5000, 0, G_PARAM_READABLE));
        /* FIXME Use actual max/min vals for int */

        g_object_class_install_property (gobjectclass, ARG_FETCHED,
                                         g_param_spec_boolean 
                                         ("fetched", "Fetched", "If the photoset information was fetched",
                                         FALSE, G_PARAM_READABLE));

        g_object_class_install_property (gobjectclass, ARG_LIST,
                                         g_param_spec_pointer 
                                         ("list", "List", "A list of all the pointers",
                                         G_PARAM_READABLE));

        ParentClass = g_type_class_ref (G_TYPE_OBJECT);
}

static void                     nflick_photo_set_init (NFlickPhotoSet *self)
{
        g_return_if_fail (NFLICK_IS_PHOTO_SET (self));

        self->Private = NULL;

        NFlickPhotoSetPrivate *priv = g_new0 (NFlickPhotoSetPrivate, 1);
        g_return_if_fail (priv != NULL);
        
        if (private_init (self, priv) == TRUE) 
                self->Private = priv;
        else {
                private_dispose (priv);
                g_free (priv);
                self->Private = NULL;
        }
}

static gboolean                 private_init (NFlickPhotoSet *self, NFlickPhotoSetPrivate *private)
{
        g_return_val_if_fail (NFLICK_IS_PHOTO_SET (self), FALSE);
        g_return_val_if_fail (private != NULL, FALSE);

        private->Name = NULL;
        private->Count = 0;
        private->Id = NULL;
        private->Fetched = FALSE;
        private->PhotoDataList = NULL;

        return TRUE;
}

static void                     private_dispose (NFlickPhotoSetPrivate *private)
{
        g_return_if_fail (private != NULL);

        if (private->Name != NULL) {
                g_free (private->Name);
                private->Name = NULL;
        }
        
        if (private->Id != NULL) {
                g_free (private->Id);
                private->Id = NULL;
        }

        if (private->PhotoDataList != NULL) {

                GList *iterator;
        
                for (iterator = private->PhotoDataList; iterator; iterator = g_list_next (iterator))
                        if (iterator->data != NULL)
                                nflick_photo_data_free ((NFlickPhotoData *) iterator->data);
                
                g_list_free (private->PhotoDataList);
                private->PhotoDataList = NULL;
        }
}

static void                     nflick_photo_set_dispose (NFlickPhotoSet *self)
{
        g_return_if_fail (NFLICK_IS_PHOTO_SET (self));

        if (self->Private != NULL)
                private_dispose (self->Private);

        G_OBJECT_CLASS (ParentClass)->dispose (G_OBJECT (self));
}

static void                     nflick_photo_set_finalize (NFlickPhotoSet *self)
{
        g_return_if_fail (NFLICK_IS_PHOTO_SET (self));
        
        if (self->Private != NULL) {
                g_free (self->Private);
                self->Private = NULL;
        }

        G_OBJECT_CLASS (ParentClass)->finalize (G_OBJECT (self));
}

NFlickPhotoSet*                 nflick_photo_set_new_no_set (gint32 count)
{
        g_return_val_if_fail (count >= 0, NULL);

        return nflick_photo_set_new (gettext ("Photos without a set"), NULL, count);
}

NFlickPhotoSet*                 nflick_photo_set_new (const gchar *name, const gchar *id, gint32 count)
{
        g_return_val_if_fail (name != NULL, NULL);
        g_return_val_if_fail (count >= 0, NULL);

        NFlickPhotoSet *self = g_object_new (NFLICK_TYPE_PHOTO_SET, NULL);
        g_return_val_if_fail (self != NULL, NULL);

        if (self->Private == NULL) {
                g_object_unref (self);
                return NULL;
        }

        self->Private->Name = g_strdup (name);

        if (id != NULL)
                self->Private->Id = g_strdup (id);

        self->Private->Count = count;
        
        return self;
}

void                            nflick_photo_set_give_list (NFlickPhotoSet *self, GList *list)
{
        g_return_if_fail (NFLICK_IS_PHOTO_SET (self));
        g_return_if_fail (self->Private->Fetched == FALSE);

        self->Private->PhotoDataList = list;
        self->Private->Fetched = TRUE;
        self->Private->Count = g_list_length (list);
}

static void                     nflick_photo_set_get_property (NFlickPhotoSet *self, guint propid, 
                                                               GValue *value, GParamSpec *pspec)
{
        g_return_if_fail (NFLICK_IS_PHOTO_SET (self));
        g_assert (self->Private != NULL);
                
        switch (propid) {
                
                case ARG_COMBO_TEXT: {
                        gchar *str = g_strdup_printf ("%s (%d)", self->Private->Name, self->Private->Count);
                        g_value_take_string (value, str);
                } break;
   
                case ARG_COUNT: {
                        g_value_set_int (value, self->Private->Count);
                } break;
    
                case ARG_ID: {
                        g_value_set_string (value, self->Private->Id);
                } break;

                case ARG_FETCHED: {
                        g_value_set_boolean (value, self->Private->Fetched);
                } break;

                case ARG_LIST: {
                        g_value_set_pointer (value, self->Private->PhotoDataList);
                } break;
 
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (self), propid, pspec);
                break;
        }
}
