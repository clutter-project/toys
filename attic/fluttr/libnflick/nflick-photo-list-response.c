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

#include "nflick-photo-list-response.h"
#include "nflick-photo-list-response-private.h"

GType                           nflick_photo_list_response_get_type (void)
{
        static GType objecttype = 0;

        if (!objecttype) {

                static const GTypeInfo objectinfo = {
                        sizeof (NFlickPhotoListResponseClass), 
                        NULL, 
                        NULL,
                        (GClassInitFunc) nflick_photo_list_response_class_init,
                        NULL,
                        NULL, 
                        sizeof (NFlickPhotoListResponse), 
                        4, 
                        (GInstanceInitFunc) nflick_photo_list_response_init,
                };
                objecttype = g_type_register_static (NFLICK_TYPE_API_RESPONSE, "NFlickPhotoListResponse",
                                                     &objectinfo, 0);
        }
        return objecttype;
}

static void                     nflick_photo_list_response_class_init (NFlickPhotoListResponseClass *klass)
{
        GObjectClass *gobjectclass = (GObjectClass *) klass;
        NFlickApiResponseClass *apiresponseclass = (NFlickApiResponseClass *) klass;

        gobjectclass->dispose = (gpointer) nflick_photo_list_response_dispose;
        gobjectclass->finalize = (gpointer) nflick_photo_list_response_finalize;
        gobjectclass->get_property = (gpointer) nflick_photo_list_response_get_property;
        
        apiresponseclass->ParseFunc = (gpointer) parse_func;

        ParentClass = g_type_class_ref (NFLICK_TYPE_API_RESPONSE);
}

static void                     nflick_photo_list_response_init (NFlickPhotoListResponse *self)
{
        g_return_if_fail (NFLICK_IS_PHOTO_LIST_RESPONSE (self));
        self->Private = NULL;

        NFlickPhotoListResponsePrivate *priv = g_new0 (NFlickPhotoListResponsePrivate, 1);
        g_return_if_fail (priv != NULL);
        
        if (private_init (self, priv) == TRUE) 
                self->Private = priv;
        else {
                private_dispose (priv);
                g_free (priv);
                self->Private = NULL;
        }
}

static gboolean                 private_init (NFlickPhotoListResponse *self, NFlickPhotoListResponsePrivate *private)
{
        g_return_val_if_fail (NFLICK_IS_PHOTO_LIST_RESPONSE (self), FALSE);
        g_return_val_if_fail (private != NULL, FALSE);

        private->PhotoDataList = NULL;

        return TRUE;
}

static void                     private_dispose (NFlickPhotoListResponsePrivate *private)
{
        g_return_if_fail (private != NULL);

        if (private->PhotoDataList != NULL) {

                GList *iterator;
        
                for (iterator = private->PhotoDataList; iterator; iterator = g_list_next (iterator))
                        if (iterator->data != NULL)
                                nflick_photo_data_free ((NFlickPhotoData *) iterator->data);
                
                g_list_free (private->PhotoDataList);
                private->PhotoDataList = NULL;
        }
}

static void                     nflick_photo_list_response_dispose (NFlickPhotoListResponse *self)
{
        g_return_if_fail (NFLICK_IS_PHOTO_LIST_RESPONSE (self));

        if (self->Private != NULL)
                private_dispose (self->Private);

        G_OBJECT_CLASS (ParentClass)->dispose (G_OBJECT (self));
}

static void                     nflick_photo_list_response_finalize (NFlickPhotoListResponse *self)
{
        g_return_if_fail (NFLICK_IS_PHOTO_LIST_RESPONSE (self));
        
        if (self->Private != NULL) {
                g_free (self->Private);
                self->Private = NULL;
        }

        G_OBJECT_CLASS (ParentClass)->finalize (G_OBJECT (self));
}

static void                     parse_func (NFlickPhotoListResponse *self, xmlDoc *doc, xmlNode *children, gboolean *result, gboolean *parse_error)
{
        g_return_if_fail (NFLICK_IS_PHOTO_LIST_RESPONSE (self));
        g_return_if_fail (children != NULL);
        g_return_if_fail (doc != NULL);
        g_return_if_fail (result != NULL && parse_error != NULL);

        xmlNode *cur_node = NULL;

        for (cur_node = children; cur_node; cur_node = cur_node->next) {
      
                if (cur_node->type == XML_ELEMENT_NODE && strcmp (cur_node->name, "photoset") == 0) {

                        xmlNode *set_node = NULL;
                        for (set_node = cur_node->children; set_node; set_node = set_node->next) {
                                
                                if (set_node->type == XML_ELEMENT_NODE && strcmp (set_node->name, "photo") == 0) {

                                        gchar *id = xmlGetProp (set_node, "id");
                                        gchar *name = xmlGetProp (set_node, "title");

                                        if (id != NULL && name != NULL) {
                                                NFlickPhotoData *photo_data = nflick_photo_data_new (id, name);
                                                if (photo_data != NULL) 
                                                        self->Private->PhotoDataList = g_list_append (self->Private->PhotoDataList, photo_data);
                                        }

                                        if (id != NULL)
                                                g_free (id);
                                        if (name != NULL)
                                                g_free (name);
                                }
                        }
                }
        }

        /* Finished */
        *result = TRUE;
        *parse_error = FALSE;
}

static void                     nflick_photo_list_response_get_property (NFlickPhotoListResponse *self, guint propid, 
                                                                        GValue *value, GParamSpec *pspec)
{
        g_return_if_fail (NFLICK_IS_PHOTO_LIST_RESPONSE (self));
        g_assert (self->Private != NULL);
                
        switch (propid) {
                
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (self), propid, pspec);
                break;
        }
}

GList*                          nflick_photo_list_response_take_list (NFlickPhotoListResponse *self)
{
        g_return_val_if_fail (NFLICK_IS_PHOTO_LIST_RESPONSE (self), NULL);

        GList *lst = self->Private->PhotoDataList;
        self->Private->PhotoDataList = NULL;

        return lst;
}


