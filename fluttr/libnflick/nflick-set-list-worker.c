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

#include "nflick-set-list-worker.h"
#include "nflick-set-list-worker-private.h"

GType                           nflick_set_list_worker_get_type (void)
{
        static GType objecttype = 0;

        if (!objecttype) {

                static const GTypeInfo objectinfo = {
                        sizeof (NFlickSetListWorkerClass), 
                        NULL, 
                        NULL,
                        (GClassInitFunc) nflick_set_list_worker_class_init,
                        NULL,
                        NULL, 
                        sizeof (NFlickSetListWorker), 
                        4, 
                        (GInstanceInitFunc) nflick_set_list_worker_init,
                };
                objecttype = g_type_register_static (NFLICK_TYPE_WORKER, "NFlickSetListWorker",
                                                     &objectinfo, 0);
        }
        return objecttype;
}

static void                     nflick_set_list_worker_class_init (NFlickSetListWorkerClass *klass)
{
        GObjectClass *gobjectclass = (GObjectClass *) klass;
        NFlickWorkerClass *workerclass = (NFlickWorkerClass *) klass;

        gobjectclass->dispose = (gpointer) nflick_set_list_worker_dispose;
        gobjectclass->finalize = (gpointer) nflick_set_list_worker_finalize;
        gobjectclass->get_property = (gpointer) nflick_set_list_worker_get_property;
        
        workerclass->ThreadFunc = (NFlickWorkerThreadFunc) thread_func;

        ParentClass = g_type_class_ref (NFLICK_TYPE_WORKER);
}

static void                     nflick_set_list_worker_init (NFlickSetListWorker *self)
{
        g_return_if_fail (NFLICK_IS_SET_LIST_WORKER (self));

        self->Private = NULL;

        NFlickSetListWorkerPrivate *priv = g_new0 (NFlickSetListWorkerPrivate, 1);
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

static gboolean                 private_init (NFlickSetListWorker *self, NFlickSetListWorkerPrivate *priv)
{
        g_return_val_if_fail (NFLICK_IS_SET_LIST_WORKER (self), FALSE);
        g_return_val_if_fail (priv != NULL, FALSE);

        priv->UserNsid = NULL;
        priv->Token = NULL;

        return TRUE;
}

static void                     private_dispose (NFlickSetListWorkerPrivate *priv)
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

static void                     nflick_set_list_worker_dispose (NFlickSetListWorker *self)
{
        g_return_if_fail (NFLICK_IS_SET_LIST_WORKER (self));

        if (self->Private != NULL)
                private_dispose (self->Private);

        G_OBJECT_CLASS (ParentClass)->dispose (G_OBJECT (self));
}

static void                     nflick_set_list_worker_finalize (NFlickSetListWorker *self)
{
        g_return_if_fail (NFLICK_IS_SET_LIST_WORKER (self));
        
        if (self->Private != NULL) {
                g_free (self->Private);
                self->Private = NULL;
        }

        G_OBJECT_CLASS (ParentClass)->finalize (G_OBJECT (self));
}

static NFlickWorkerStatus       thread_func (NFlickSetListWorker *self)
{
        NFlickApiRequest *get_photosets_request = NULL; 
        NFlickWorkerStatus status = NFLICK_WORKER_STATUS_OK;
        NFlickApiResponse *set_list_response = NULL;
        gchar *first_id = NULL;
        NFlickPhotoSet *first_set = NULL; /* Do not dispose, it's not reffed */
        NFlickApiRequest *first_photolist_request = NULL; 
        NFlickApiResponse *first_photo_list_response = NULL;
        GList *first_list = NULL;
        NFlickApiRequest *unsetted_request = NULL; 
        NFlickApiResponse *unsetted_response = NULL;
        GList *unsetted_list = NULL;
        NFlickPhotoSet *unsetted_set = NULL; /* Do not dispose, it's not reffed */

        get_photosets_request = nflick_api_request_new (NFLICK_FLICKR_API_METHOD_PHOTOSETS_GET_LIST);
        if (get_photosets_request == NULL)
                goto Error;
        
        nflick_api_request_add_parameter (get_photosets_request, 
                                          NFLICK_FLICKR_API_PARAM_TOKEN, 
                                          self->Private->Token);

        nflick_api_request_add_parameter (get_photosets_request, 
                                          NFLICK_FLICKR_API_PARAM_USER_ID, 
                                          self->Private->UserNsid);

        nflick_api_request_sign (get_photosets_request);
        if (nflick_api_request_exec (get_photosets_request) != TRUE) {
                nflick_worker_set_network_error ((NFlickWorker *) self);
                goto Error;
        }

        if (nflick_worker_is_aborted ((NFlickWorker *) self) == TRUE)
                goto Abort;

        set_list_response = nflick_api_response_new_from_request (NFLICK_TYPE_SET_LIST_RESPONSE, get_photosets_request);
        if (set_list_response == NULL)
                goto Error;

        if (nflick_worker_parse_api_response ((NFlickWorker*) self, set_list_response) == FALSE)
                goto Error;

        self->Private->PhotoSets = nflick_set_list_response_take_list ((NFlickSetListResponse *) set_list_response);

        /* Let's fetch information about the unsetted photos */
        nflick_worker_set_message ((NFlickWorker *) self, gettext ("Parsing photos without set..."));

        unsetted_request = nflick_api_request_new (NFLICK_FLICKR_API_METHOD_PHOTOS_NOT_IN_SET);
        if (unsetted_request == NULL)
                goto Error;
        
        nflick_api_request_add_parameter (unsetted_request, 
                                          NFLICK_FLICKR_API_PARAM_TOKEN, 
                                          self->Private->Token);

        /* We try to get 500 photos per page. 500 is a maximum value. 
         * FIXME: We should check if 500 is enough. Someone might have more than 
         * 500 photos */

        nflick_api_request_add_parameter (unsetted_request, 
                                          NFLICK_FLICKR_API_PARAM_PER_PAGE,
                                          "500");

        nflick_api_request_sign (unsetted_request);
        if (nflick_api_request_exec (unsetted_request) != TRUE) {
                nflick_worker_set_network_error ((NFlickWorker *) self);
                goto Error;
        }

        if (nflick_worker_is_aborted ((NFlickWorker *) self) == TRUE)
                goto Abort;

        unsetted_response = nflick_api_response_new_from_request (NFLICK_TYPE_NO_SET_RESPONSE, unsetted_request);
        if (unsetted_response == NULL)
                goto Error;

        if (nflick_worker_parse_api_response ((NFlickWorker*) self, unsetted_response) == FALSE)
                goto Error;

        unsetted_list = nflick_no_set_response_take_list ((NFlickNoSetResponse *) unsetted_response);
        /* FIXME: Here we could expose the "count" property on the PhotoSetResponse and NoSetResponse */
        unsetted_set = nflick_photo_set_new_no_set (g_list_length (unsetted_list)); 
        nflick_photo_set_give_list (unsetted_set, unsetted_list);

        /* Append the set to our set list... */
        self->Private->PhotoSets = g_list_append (self->Private->PhotoSets, 
                                                   unsetted_set);

        /* If the user has not sets, finish now */
        if (self->Private->PhotoSets->data == (gpointer) unsetted_set) {
                goto Done;
        }
        /* Now let's try fetching the photos for first photo set */
        nflick_worker_set_message ((NFlickWorker *) self, gettext ("Loading photoset data..."));
        
        GList *sets = self->Private->PhotoSets;
        GList *set;
        gint i = g_list_length (sets);
        
        for (set = sets; set != NULL; set = set->next) {
        	first_set = (NFlickPhotoSet*)set->data;
        
        	g_object_get (G_OBJECT (first_set), "id", &first_id, NULL);

        	first_photolist_request = nflick_api_request_new
        			 (NFLICK_FLICKR_API_METHOD_PHOTOSETS_GET_PHOTOS);
        	if (first_photolist_request == NULL)
                	goto Error;
        
        	nflick_api_request_add_parameter (first_photolist_request, 
                                          NFLICK_FLICKR_API_PARAM_TOKEN, 
                                          self->Private->Token);

        	nflick_api_request_add_parameter (first_photolist_request, 
                                          NFLICK_FLICKR_API_PARAM_PHOTOSET_ID, 
                                          first_id);

        	nflick_api_request_sign (first_photolist_request);
        	if (nflick_api_request_exec (first_photolist_request) != TRUE) {
                	nflick_worker_set_network_error ((NFlickWorker *) self);
                	g_warning ("Error : %s", first_id);
        	}

        	if (nflick_worker_is_aborted ((NFlickWorker *) self) == TRUE)
                	g_warning ("Abort : %s", first_id);

        	first_photo_list_response = nflick_api_response_new_from_request 
        	     (NFLICK_TYPE_PHOTO_LIST_RESPONSE, first_photolist_request);
        	if (first_photo_list_response == NULL)
                	g_warning ("No photos : %s", first_id);

        	if (nflick_worker_parse_api_response ((NFlickWorker*) self, 
        				first_photo_list_response) == FALSE)
                	g_warning ("Abort : No Photos %s", first_id);

        	first_list = nflick_photo_list_response_take_list
        	        ((NFlickPhotoListResponse *) first_photo_list_response);
        	nflick_photo_set_give_list (first_set, first_list);
        }

        /* All ok */
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

        if (set_list_response != NULL) 
                g_object_unref (set_list_response);

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

NFlickSetListWorker*            nflick_set_list_worker_new (const gchar *usernsid, const gchar *token)
{
        g_return_val_if_fail (token != NULL, NULL);
        g_return_val_if_fail (usernsid != NULL, NULL);

        NFlickSetListWorker *self = g_object_new (NFLICK_TYPE_SET_LIST_WORKER, NULL);
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

GList*                          nflick_set_list_worker_take_list (NFlickSetListWorker *self)
{
        g_return_val_if_fail (NFLICK_IS_SET_LIST_WORKER (self), NULL);

        GList *lst = self->Private->PhotoSets;
        self->Private->PhotoSets = NULL;

        return lst;
}

static void                     nflick_set_list_worker_get_property (NFlickSetListWorker *self, guint propid, 
                                                                 GValue *value, GParamSpec *pspec)
{
        g_return_if_fail (NFLICK_IS_SET_LIST_WORKER (self));
        g_assert (self->Private != NULL);
                
        switch (propid) {
                
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (self), propid, pspec);
                break;
        }
}
