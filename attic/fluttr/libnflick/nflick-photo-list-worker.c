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

#include "nflick-photo-list-worker.h"
#include "nflick-photo-list-worker-private.h"

GType                           nflick_photo_list_worker_get_type (void)
{
        static GType objecttype = 0;

        if (!objecttype) {

                static const GTypeInfo objectinfo = {
                        sizeof (NFlickPhotoListWorkerClass), 
                        NULL, 
                        NULL,
                        (GClassInitFunc) nflick_photo_list_worker_class_init,
                        NULL,
                        NULL, 
                        sizeof (NFlickPhotoListWorker), 
                        4, 
                        (GInstanceInitFunc) nflick_photo_list_worker_init,
                };
                objecttype = g_type_register_static (NFLICK_TYPE_WORKER, "NFlickPhotoListWorker",
                                                     &objectinfo, 0);
        }
        return objecttype;
}

static void                     nflick_photo_list_worker_class_init (NFlickPhotoListWorkerClass *klass)
{
        GObjectClass *gobjectclass = (GObjectClass *) klass;
        NFlickWorkerClass *workerclass = (NFlickWorkerClass *) klass;

        gobjectclass->dispose = (gpointer) nflick_photo_list_worker_dispose;
        gobjectclass->finalize = (gpointer) nflick_photo_list_worker_finalize;
        gobjectclass->get_property = (gpointer) nflick_photo_list_worker_get_property;
        
        workerclass->ThreadFunc = (NFlickWorkerThreadFunc) thread_func;

        ParentClass = g_type_class_ref (NFLICK_TYPE_WORKER);
}

static void                     nflick_photo_list_worker_init (NFlickPhotoListWorker *self)
{
        g_return_if_fail (NFLICK_IS_PHOTO_LIST_WORKER (self));

        self->Private = NULL;

        NFlickPhotoListWorkerPrivate *priv = g_new0 (NFlickPhotoListWorkerPrivate, 1);
        g_return_if_fail (priv != NULL);
        
        if (private_init (self, priv) == TRUE) {
                self->Private = priv;
                nflick_worker_set_message ((NFlickWorker *) self, gettext ("Loading photoset data..."));
        } else {
                private_dispose (priv);
                g_free (priv);
                self->Private = NULL;
        }
}

static gboolean                 private_init (NFlickPhotoListWorker *self, NFlickPhotoListWorkerPrivate *private)
{
        g_return_val_if_fail (NFLICK_IS_PHOTO_LIST_WORKER (self), FALSE);
        g_return_val_if_fail (private != NULL, FALSE);

        private->Id = NULL;
        private->Token = NULL;
        private->PhotoDataList = NULL;

        return TRUE;
}

static void                     private_dispose (NFlickPhotoListWorkerPrivate *private)
{
        g_return_if_fail (private != NULL);

        if (private->Id != NULL) {
                g_free (private->Id);
                private->Id = NULL;
        }

        if (private->Token != NULL) {
                g_free (private->Token);
                private->Token = NULL;
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

static void                     nflick_photo_list_worker_dispose (NFlickPhotoListWorker *self)
{
        g_return_if_fail (NFLICK_IS_PHOTO_LIST_WORKER (self));

        if (self->Private != NULL)
                private_dispose (self->Private);

        G_OBJECT_CLASS (ParentClass)->dispose (G_OBJECT (self));
}

static void                     nflick_photo_list_worker_finalize (NFlickPhotoListWorker *self)
{
        g_return_if_fail (NFLICK_IS_PHOTO_LIST_WORKER (self));
        
        if (self->Private != NULL) {
                g_free (self->Private);
                self->Private = NULL;
        }

        G_OBJECT_CLASS (ParentClass)->finalize (G_OBJECT (self));
}

static NFlickWorkerStatus       thread_func (NFlickPhotoListWorker *self)
{
        NFlickApiRequest *get_photolist_request = NULL; 
        NFlickWorkerStatus status = NFLICK_WORKER_STATUS_OK;
        NFlickApiResponse *photo_list_response = NULL;

        get_photolist_request = nflick_api_request_new (NFLICK_FLICKR_API_METHOD_PHOTOSETS_GET_PHOTOS);
        if (get_photolist_request == NULL)
                goto Error;
        
        nflick_api_request_add_parameter (get_photolist_request, 
                                          NFLICK_FLICKR_API_PARAM_TOKEN, 
                                          self->Private->Token);

        nflick_api_request_add_parameter (get_photolist_request, 
                                          NFLICK_FLICKR_API_PARAM_PHOTOSET_ID, 
                                          self->Private->Id);

        nflick_api_request_sign (get_photolist_request);
        if (nflick_api_request_exec (get_photolist_request) != TRUE) {
                nflick_worker_set_network_error ((NFlickWorker *) self);
                goto Error;
        }

        if (nflick_worker_is_aborted ((NFlickWorker *) self) == TRUE)
                goto Abort;

        photo_list_response = nflick_api_response_new_from_request (NFLICK_TYPE_PHOTO_LIST_RESPONSE, get_photolist_request);
        if (photo_list_response == NULL)
                goto Error;

        if (nflick_worker_parse_api_response ((NFlickWorker*) self, photo_list_response) == FALSE)
                goto Error;

        self->Private->PhotoDataList = nflick_photo_list_response_take_list ((NFlickPhotoListResponse *) photo_list_response);

        /* All ok */
        goto Done;

Abort:
        status = NFLICK_WORKER_STATUS_ABORTED;
        goto Done;

Error:
        status = NFLICK_WORKER_STATUS_ERROR;

Done:
        if (get_photolist_request != NULL) 
                g_object_unref (get_photolist_request);

        if (photo_list_response != NULL) 
                g_object_unref (photo_list_response);

        return status;
}

NFlickPhotoListWorker*         nflick_photo_list_worker_new (const gchar *id, const gchar *token)
{
        g_return_val_if_fail (id != NULL, NULL);

        NFlickPhotoListWorker *self = g_object_new (NFLICK_TYPE_PHOTO_LIST_WORKER, NULL);
        g_return_val_if_fail (self != NULL, NULL);

        if (self->Private == NULL) {
                g_object_unref (self);
                return NULL;
        }

        self->Private->Id = g_strdup (id);
        self->Private->Token = g_strdup (token);
        
        return self;
}

static void                     nflick_photo_list_worker_get_property (NFlickPhotoListWorker *self, guint propid, 
                                                                       GValue *value, GParamSpec *pspec)
{
        g_return_if_fail (NFLICK_IS_PHOTO_LIST_WORKER (self));
        g_assert (self->Private != NULL);
                
        switch (propid) {
                
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (self), propid, pspec);
                break;
        }
}

GList*                          nflick_photo_list_worker_take_list (NFlickPhotoListWorker *self)
{
        g_return_val_if_fail (NFLICK_IS_PHOTO_LIST_WORKER (self), NULL);

        GList *lst = self->Private->PhotoDataList;
        self->Private->PhotoDataList = NULL;

        return lst;
}
