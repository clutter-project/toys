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

#include "nflick-set-list-response.h"
#include "nflick-set-list-response-private.h"

GType                           nflick_set_list_response_get_type (void)
{
        static GType objecttype = 0;

        if (!objecttype) {

                static const GTypeInfo objectinfo = {
                        sizeof (NFlickSetListResponseClass), 
                        NULL, 
                        NULL,
                        (GClassInitFunc) nflick_set_list_response_class_init,
                        NULL,
                        NULL, 
                        sizeof (NFlickSetListResponse), 
                        4, 
                        (GInstanceInitFunc) nflick_set_list_response_init,
                };
                objecttype = g_type_register_static (NFLICK_TYPE_API_RESPONSE, "NFlickSetListResponse",
                                                     &objectinfo, 0);
        }
        return objecttype;
}

static void                     nflick_set_list_response_class_init (NFlickSetListResponseClass *klass)
{
        GObjectClass *gobjectclass = (GObjectClass *) klass;
        NFlickApiResponseClass *apiresponseclass = (NFlickApiResponseClass *) klass;

        gobjectclass->dispose = (gpointer) nflick_set_list_response_dispose;
        gobjectclass->finalize = (gpointer) nflick_set_list_response_finalize;
        gobjectclass->get_property = (gpointer) nflick_set_list_response_get_property;
        
        apiresponseclass->ParseFunc = (gpointer) parse_func;

        ParentClass = g_type_class_ref (NFLICK_TYPE_API_RESPONSE);
}

static void                     nflick_set_list_response_init (NFlickSetListResponse *self)
{
        g_return_if_fail (NFLICK_IS_SET_LIST_RESPONSE (self));
        self->Private = NULL;

        NFlickSetListResponsePrivate *priv = g_new0 (NFlickSetListResponsePrivate, 1);
        g_return_if_fail (priv != NULL);
        
        if (private_init (self, priv) == TRUE) 
                self->Private = priv;
        else {
                private_dispose (priv);
                g_free (priv);
                self->Private = NULL;
        }
}

static gboolean                 private_init (NFlickSetListResponse *self, NFlickSetListResponsePrivate *private)
{
        g_return_val_if_fail (NFLICK_IS_SET_LIST_RESPONSE (self), FALSE);
        g_return_val_if_fail (private != NULL, FALSE);

        private->PhotoSets = NULL;

        return TRUE;
}

static void                     private_dispose (NFlickSetListResponsePrivate *private)
{
        g_return_if_fail (private != NULL);

        if (private->PhotoSets != NULL) {

                GList *iterator;
        
                for (iterator = private->PhotoSets; iterator; iterator = g_list_next (iterator))
                        if (iterator->data != NULL)
                                g_object_unref (iterator->data);
                
                g_list_free (private->PhotoSets);
                private->PhotoSets = NULL;
        }
}

GList*                          nflick_set_list_response_take_list (NFlickSetListResponse *self)
{
        g_return_val_if_fail (NFLICK_IS_SET_LIST_RESPONSE (self), NULL);

        GList *lst = self->Private->PhotoSets;
        self->Private->PhotoSets = NULL;

        return lst;
}

static void                     nflick_set_list_response_dispose (NFlickSetListResponse *self)
{
        g_return_if_fail (NFLICK_IS_SET_LIST_RESPONSE (self));

        if (self->Private != NULL)
                private_dispose (self->Private);

        G_OBJECT_CLASS (ParentClass)->dispose (G_OBJECT (self));
}

static void                     nflick_set_list_response_finalize (NFlickSetListResponse *self)
{
        g_return_if_fail (NFLICK_IS_SET_LIST_RESPONSE (self));
        
        if (self->Private != NULL) {
                g_free (self->Private);
                self->Private = NULL;
        }

        G_OBJECT_CLASS (ParentClass)->finalize (G_OBJECT (self));
}

static void                     parse_func (NFlickSetListResponse *self, xmlDoc *doc, xmlNode *children, gboolean *result, gboolean *parse_error)
{
        g_return_if_fail (NFLICK_IS_SET_LIST_RESPONSE (self));
        g_return_if_fail (children != NULL);
        g_return_if_fail (doc != NULL);
        g_return_if_fail (result != NULL && parse_error != NULL);

        xmlNode *cur_node = NULL;

        for (cur_node = children; cur_node; cur_node = cur_node->next) {
      
                if (cur_node->type == XML_ELEMENT_NODE && strcmp (cur_node->name, "photosets") == 0) {
                        
                        xmlNode *sets_node = NULL;
                        for (sets_node = cur_node->children; sets_node; sets_node = sets_node->next) {
                                
                                if (sets_node->type == XML_ELEMENT_NODE && strcmp (sets_node->name, "photoset") == 0) {

                                        gchar *id = xmlGetProp (sets_node, "id");
                                        gchar *count = xmlGetProp (sets_node, "photos");
                                        gchar *title = NULL;
                                        gint32 count_val = 0;
                                        NFlickPhotoSet *photo_set = NULL;

                                        xmlNode *this_node = NULL;
                                        for (this_node = sets_node->children; this_node; this_node = this_node->next) {
                                                if (this_node->type == XML_ELEMENT_NODE && strcmp (this_node->name, "title") == 0) {
                                                        if (title != NULL)
                                                                g_free (title);
                                                        title = xmlNodeListGetString (doc, this_node->xmlChildrenNode, 1);
                                                }
                                        }

                                        count_val = atoi (count);

                                        if (count_val != 0 &&
                                            id != NULL &&
                                            title != NULL)
                                                photo_set = nflick_photo_set_new (title, id, count_val);

                                        if (photo_set != NULL)
                                                self->Private->PhotoSets = g_list_append (self->Private->PhotoSets, photo_set);

                                        /* Free */
                                        if (id != NULL)
                                                g_free (id);
                                        if (count != NULL)
                                                g_free (count);
                                        if (title != NULL)
                                                g_free (title);
                                }
                        }
                }
        }

        /* Finished */
        *result = TRUE;
        *parse_error = FALSE;
}

static void                     nflick_set_list_response_get_property (NFlickSetListResponse *self, guint propid, 
                                                                        GValue *value, GParamSpec *pspec)
{
        g_return_if_fail (NFLICK_IS_SET_LIST_RESPONSE (self));
        g_assert (self->Private != NULL);
                
        switch (propid) {
                
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (self), propid, pspec);
                break;
        }
}
