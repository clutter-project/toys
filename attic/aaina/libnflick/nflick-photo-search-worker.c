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

#include "nflick-photo-search-worker.h"
#include "nflick-photo-search-response.h"

static NFlickWorkerClass*       ParentClass = NULL;

struct                          _NFlickPhotoSearchWorkerPrivate
{
        gchar *UserNsid;
        gchar *Token;
        GList *PhotoSets;
};

enum 
{
        ARG_0,
};

static void                     
nflick_photo_search_worker_class_init (NFlickPhotoSearchWorkerClass *klass);

static void                     
nflick_photo_search_worker_init (NFlickPhotoSearchWorker *self);

static gboolean                 
private_init (NFlickPhotoSearchWorker *self, 
              NFlickPhotoSearchWorkerPrivate *priv);

static void                     
private_dispose (NFlickPhotoSearchWorkerPrivate *priv);

static void                     
nflick_photo_search_worker_dispose (NFlickPhotoSearchWorker *self);

static void                     
nflick_photo_search_worker_finalize (NFlickPhotoSearchWorker *self);

static NFlickWorkerStatus       
thread_func (NFlickPhotoSearchWorker *self);

static void                     
nflick_photo_search_worker_get_property (NFlickPhotoSearchWorker *self, 
                                         guint propid, 
                                         GValue *value, GParamSpec *pspec);
GType                           
nflick_photo_search_worker_get_type (void)
{
        static GType objecttype = 0;

        if (!objecttype) {

                static const GTypeInfo objectinfo = {
                        sizeof (NFlickPhotoSearchWorkerClass), 
                        NULL, 
                        NULL,
                        (GClassInitFunc) nflick_photo_search_worker_class_init,
                        NULL,
                        NULL, 
                        sizeof (NFlickPhotoSearchWorker), 
                        4, 
                        (GInstanceInitFunc) nflick_photo_search_worker_init,
                };
                objecttype = g_type_register_static (NFLICK_TYPE_WORKER, "NFlickPhotoSearchWorker",
                                                     &objectinfo, 0);
        }
        return objecttype;
}

static void                     
nflick_photo_search_worker_class_init (NFlickPhotoSearchWorkerClass *klass)
{
        GObjectClass *gobjectclass = (GObjectClass *) klass;
        NFlickWorkerClass *workerclass = (NFlickWorkerClass *) klass;

        gobjectclass->dispose = (gpointer) nflick_photo_search_worker_dispose;
        gobjectclass->finalize = (gpointer) nflick_photo_search_worker_finalize;
        gobjectclass->get_property = (gpointer) nflick_photo_search_worker_get_property;
        
        workerclass->ThreadFunc = (NFlickWorkerThreadFunc) thread_func;

        ParentClass = g_type_class_ref (NFLICK_TYPE_WORKER);
}

static void                     
nflick_photo_search_worker_init (NFlickPhotoSearchWorker *self)
{
        g_return_if_fail (NFLICK_IS_PHOTO_SEARCH_WORKER (self));

        self->Private = NULL;

        NFlickPhotoSearchWorkerPrivate *priv = g_new0 (NFlickPhotoSearchWorkerPrivate, 1);
        g_return_if_fail (priv != NULL);
        
        if (private_init (self, priv) == TRUE) {
                self->Private = priv;
                nflick_worker_set_message ((NFlickWorker *) self, gettext ("Parsing photosets..."));
        } else {
                private_dispose (priv);
                g_free (priv);
                self->Private = NULL;
        }
}

static gboolean                 
private_init (NFlickPhotoSearchWorker *self, NFlickPhotoSearchWorkerPrivate *priv)
{
        g_return_val_if_fail (NFLICK_IS_PHOTO_SEARCH_WORKER (self), FALSE);
        g_return_val_if_fail (priv != NULL, FALSE);

        priv->UserNsid = NULL;
        priv->Token = NULL;

        return TRUE;
}

static void                     
private_dispose (NFlickPhotoSearchWorkerPrivate *priv)
{
        g_return_if_fail (priv != NULL);

        if (priv->Token != NULL) {
                g_free (priv->Token);
                priv->Token = NULL;
        }

        if (priv->UserNsid != NULL) {
                g_free (priv->UserNsid);
                priv->UserNsid = NULL;
        }

        if (priv->PhotoSets != NULL) {

                GList *iterator;
        
                for (iterator = priv->PhotoSets; iterator; iterator = g_list_next (iterator))
                        if (iterator->data != NULL)
                                g_object_unref (iterator->data);
                
                g_list_free (priv->PhotoSets);
                priv->PhotoSets = NULL;
        }
}

static void                     
nflick_photo_search_worker_dispose (NFlickPhotoSearchWorker *self)
{
        g_return_if_fail (NFLICK_IS_PHOTO_SEARCH_WORKER (self));

        if (self->Private != NULL)
                private_dispose (self->Private);

        G_OBJECT_CLASS (ParentClass)->dispose (G_OBJECT (self));
}

static void                     
nflick_photo_search_worker_finalize (NFlickPhotoSearchWorker *self)
{
        g_return_if_fail (NFLICK_IS_PHOTO_SEARCH_WORKER (self));
        
        if (self->Private != NULL) {
                g_free (self->Private);
                self->Private = NULL;
        }

        G_OBJECT_CLASS (ParentClass)->finalize (G_OBJECT (self));
}

static NFlickWorkerStatus       
thread_func (NFlickPhotoSearchWorker *self)
{
        NFlickApiRequest *get_photosets_request = NULL; 
        NFlickWorkerStatus status = NFLICK_WORKER_STATUS_OK;
        NFlickApiResponse *photo_search_response = NULL;
        gchar *first_id = NULL;
        NFlickPhotoSet *first_set = NULL;
        NFlickApiRequest *first_photolist_request = NULL; 
        NFlickApiResponse *first_photo_list_response = NULL;
        GList *first_list = NULL;
        NFlickApiRequest *unsetted_request = NULL; 
        NFlickApiResponse *unsetted_response = NULL;
        GList *unsetted_list = NULL;
        NFlickPhotoSet *unsetted_set = NULL;

        get_photosets_request = nflick_api_request_new ("flickr.photos.search");
        if (get_photosets_request == NULL)
                g_error ("request did not equal NULL, run for the hills\n");
        
        nflick_api_request_add_parameter (get_photosets_request, 
                                          "tags", self->Private->UserNsid);
        nflick_api_request_add_parameter (get_photosets_request,
                                          "sort", "date-posted-desc");

        nflick_api_request_sign (get_photosets_request);
        if (nflick_api_request_exec (get_photosets_request) != TRUE) {
                nflick_worker_set_network_error ((NFlickWorker *) self);
                goto Error;
        }

        if (nflick_worker_is_aborted ((NFlickWorker *) self) == TRUE)
                goto Abort;
        //gchar *buffer = nflick_api_request_take_buffer (get_photosets_request);
        //g_print ("%s\n", buffer);
        
        photo_search_response = nflick_api_response_new_from_request (
                     NFLICK_TYPE_PHOTO_SEARCH_RESPONSE, get_photosets_request);
        if (photo_search_response == NULL)
                goto Error;
        
        if (nflick_worker_parse_api_response ((NFlickWorker*) self, photo_search_response) == FALSE)
                goto Error;
        
        self->Private->PhotoSets = nflick_photo_search_response_take_list ((NFlickPhotoSearchResponse *) photo_search_response);
        /*
        GList *l;
        for (l = self->Private->PhotoSets; l != NULL; l = l->next)
        {
          FlickrPhoto *photo = (FlickrPhoto*)l->data;
          g_print ("%s %s %s\n", photo->id, photo->title, photo->user);
        }
        */
        goto Done;

Abort:
        status = NFLICK_WORKER_STATUS_ABORTED;
        g_print ("Abort\n");
        goto Done;

Error:
        status = NFLICK_WORKER_STATUS_ERROR;
        g_print ("Error\n");
Done:
        if (get_photosets_request != NULL) 
                g_object_unref (get_photosets_request);

        if (photo_search_response != NULL) 
                g_object_unref (photo_search_response);

        if (first_photolist_request != NULL) 
                g_object_unref (first_photolist_request);

        if (unsetted_response != NULL) 
                g_object_unref (unsetted_response);

        if (unsetted_request != NULL) 
                g_object_unref (unsetted_request);

        if (first_photo_list_response != NULL) 
                g_object_unref (first_photo_list_response);

        if (first_id != NULL)
                g_free (first_id);

        return status;
}

NFlickPhotoSearchWorker*            
nflick_photo_search_worker_new (const gchar *usernsid, const gchar *token)
{
        g_return_val_if_fail (token != NULL, NULL);
        g_return_val_if_fail (usernsid != NULL, NULL);

        NFlickPhotoSearchWorker *self = g_object_new (NFLICK_TYPE_PHOTO_SEARCH_WORKER, NULL);
        g_return_val_if_fail (self != NULL, NULL);

        if (self->Private == NULL) {
                g_object_unref (self);
                return NULL;
        }

        self->Private->Token = g_strdup (token);
        self->Private->UserNsid = g_strdup (usernsid);
        self->Private->PhotoSets = NULL;

        return self;
}

GList*                          
nflick_photo_search_worker_take_list (NFlickPhotoSearchWorker *self)
{
        g_return_val_if_fail (NFLICK_IS_PHOTO_SEARCH_WORKER (self), NULL);

        GList *lst = self->Private->PhotoSets;
        self->Private->PhotoSets = NULL;

        return lst;
}

static void                     
nflick_photo_search_worker_get_property (NFlickPhotoSearchWorker *self, 
                                         guint propid, 
                                         GValue *value, GParamSpec *pspec)
{
        g_return_if_fail (NFLICK_IS_PHOTO_SEARCH_WORKER (self));
        g_assert (self->Private != NULL);
                
        switch (propid) {
                
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (self), propid, pspec);
                break;
        }
}
