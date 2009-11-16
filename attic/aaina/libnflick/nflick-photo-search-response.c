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

#include "nflick-photo-search-response.h"
#include "nflick-photo-search-response-private.h"


GType                           nflick_photo_search_response_get_type (void)
{
        static GType objecttype = 0;

        if (!objecttype) {

                static const GTypeInfo objectinfo = {
                        sizeof (NFlickPhotoSearchResponseClass), 
                        NULL, 
                        NULL,
                        (GClassInitFunc) nflick_photo_search_response_class_init,
                        NULL,
                        NULL, 
                        sizeof (NFlickPhotoSearchResponse), 
                        4, 
                        (GInstanceInitFunc) nflick_photo_search_response_init,
                };
                objecttype = g_type_register_static (NFLICK_TYPE_API_RESPONSE, "NFlickPhotoSearchResponse",
                                                     &objectinfo, 0);
        }
        return objecttype;
}


static void                     nflick_photo_search_response_class_init (NFlickPhotoSearchResponseClass *klass)
{
        GObjectClass *gobjectclass = (GObjectClass *) klass;
        NFlickApiResponseClass *apiresponseclass = (NFlickApiResponseClass *) klass;

        gobjectclass->dispose = (gpointer) nflick_photo_search_response_dispose;
        gobjectclass->finalize = (gpointer) nflick_photo_search_response_finalize;
        gobjectclass->get_property = (gpointer) nflick_photo_search_response_get_property;
        
        apiresponseclass->ParseFunc = (gpointer) parse_func;

        ParentClass = g_type_class_ref (NFLICK_TYPE_API_RESPONSE);
}

static void                     nflick_photo_search_response_init (NFlickPhotoSearchResponse *self)
{
        g_return_if_fail (NFLICK_IS_PHOTO_SEARCH_RESPONSE (self));
        self->Private = NULL;

        NFlickPhotoSearchResponsePrivate *priv = g_new0 (NFlickPhotoSearchResponsePrivate, 1);
        g_return_if_fail (priv != NULL);
        
        if (private_init (self, priv) == TRUE) 
                self->Private = priv;
        else {
                private_dispose (priv);
                g_free (priv);
                self->Private = NULL;
        }
}

static gboolean                 private_init (NFlickPhotoSearchResponse *self, NFlickPhotoSearchResponsePrivate *private)
{
        g_return_val_if_fail (NFLICK_IS_PHOTO_SEARCH_RESPONSE (self), FALSE);
        g_return_val_if_fail (private != NULL, FALSE);

        private->PhotoSets = NULL;

        return TRUE;
}

static void                     
private_dispose (NFlickPhotoSearchResponsePrivate *private)
{
        g_return_if_fail (private != NULL);
        return;

        if (private->PhotoSets != NULL) {

                GList *iterator;
        
                for (iterator = private->PhotoSets; iterator; iterator = g_list_next (iterator))
                        if (iterator->data != NULL)
                                g_object_unref (iterator->data);
                
                g_list_free (private->PhotoSets);
                private->PhotoSets = NULL;
        }
}

GList*                          
nflick_photo_search_response_take_list (NFlickPhotoSearchResponse *self)
{
        g_return_val_if_fail (NFLICK_IS_PHOTO_SEARCH_RESPONSE (self), NULL);

        GList *lst = self->Private->PhotoSets;
        self->Private->PhotoSets = NULL;

        return lst;
}

static void                     nflick_photo_search_response_dispose (NFlickPhotoSearchResponse *self)
{
        g_return_if_fail (NFLICK_IS_PHOTO_SEARCH_RESPONSE (self));

        if (self->Private != NULL)
                private_dispose (self->Private);

        G_OBJECT_CLASS (ParentClass)->dispose (G_OBJECT (self));
}

static void                     nflick_photo_search_response_finalize (NFlickPhotoSearchResponse *self)
{
        g_return_if_fail (NFLICK_IS_PHOTO_SEARCH_RESPONSE (self));
        
        if (self->Private != NULL) {
                g_free (self->Private);
                self->Private = NULL;
        }

        G_OBJECT_CLASS (ParentClass)->finalize (G_OBJECT (self));
}

static void                     
parse_func (NFlickPhotoSearchResponse *self, 
            xmlDoc *doc, 
            xmlNode *children, 
            gboolean *result, 
            gboolean *parse_error)
{
  g_return_if_fail (NFLICK_IS_PHOTO_SEARCH_RESPONSE (self));
  g_return_if_fail (children != NULL);
  g_return_if_fail (doc != NULL);
  g_return_if_fail (result != NULL && parse_error != NULL);

  xmlNode *cur_node = NULL;

  for (cur_node = children; cur_node; cur_node = cur_node->next) 
  {
      
    if (cur_node->type == XML_ELEMENT_NODE 
          && strcmp (cur_node->name, "photos") == 0) 
    {
                        
      xmlNode *sets_node = NULL;
      for (sets_node = cur_node->children; sets_node;
                                                sets_node = sets_node->next) 
      {
                                
        if (sets_node->type == XML_ELEMENT_NODE 
                                  && strcmp (sets_node->name, "photo") == 0) 
        {

          gchar *id = xmlGetProp (sets_node, "id");
          gchar *title = xmlGetProp (sets_node, "title");
          gchar *user = xmlGetProp (sets_node, "owner");

          FlickrPhoto *photo = g_new0 (FlickrPhoto, 1);
          photo->id = id;
          photo->title = title;
          photo->user = user;

          self->Private->PhotoSets = g_list_append (self->Private->PhotoSets,
                                                      photo);
        }
      }
    }
  }
  /* Finished */
  *result = TRUE;
  *parse_error = FALSE;
}

static void                     nflick_photo_search_response_get_property (NFlickPhotoSearchResponse *self, guint propid, 
                                                                        GValue *value, GParamSpec *pspec)
{
        g_return_if_fail (NFLICK_IS_PHOTO_SEARCH_RESPONSE (self));
        g_assert (self->Private != NULL);
                
        switch (propid) {
                
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (self), propid, pspec);
                break;
        }
}
