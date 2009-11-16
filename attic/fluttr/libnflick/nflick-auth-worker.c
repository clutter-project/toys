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

#include "nflick-auth-worker.h"
#include "nflick-auth-worker-private.h"

GType                           nflick_auth_worker_get_type (void)
{
        static GType objecttype = 0;

        if (!objecttype) {

                static const GTypeInfo objectinfo = {
                        sizeof (NFlickAuthWorkerClass), 
                        NULL, 
                        NULL,
                        (GClassInitFunc) nflick_auth_worker_class_init,
                        NULL,
                        NULL, 
                        sizeof (NFlickAuthWorker), 
                        4, 
                        (GInstanceInitFunc) nflick_auth_worker_init,
                };
                objecttype = g_type_register_static (NFLICK_TYPE_WORKER, "NFlickAuthWorker",
                                                     &objectinfo, 0);
        }
        return objecttype;
}

static void                     nflick_auth_worker_class_init (NFlickAuthWorkerClass *klass)
{
        GObjectClass *gobjectclass = (GObjectClass *) klass;
        NFlickWorkerClass *workerclass = (NFlickWorkerClass *) klass;

        gobjectclass->dispose = (gpointer) nflick_auth_worker_dispose;
        gobjectclass->finalize = (gpointer) nflick_auth_worker_finalize;
        gobjectclass->get_property = (gpointer) nflick_auth_worker_get_property;
        
        g_object_class_install_property (gobjectclass, ARG_TOKEN,
                                         g_param_spec_string
                                         ("token", "Token", "Unique flick full token",
                                         NULL, G_PARAM_READABLE));

        g_object_class_install_property (gobjectclass, ARG_USER_NAME,
                                         g_param_spec_string
                                         ("username", "UserName", "Flickr user name",
                                         NULL, G_PARAM_READABLE));

        g_object_class_install_property (gobjectclass, ARG_FULL_NAME,
                                         g_param_spec_string
                                         ("fullname", "FullName", "Flickr full user name",
                                         NULL, G_PARAM_READABLE));

        g_object_class_install_property (gobjectclass, ARG_USER_NSID,
                                         g_param_spec_string
                                         ("usernsid", "UserNsid", "Unique nsid identyfying user in flickr",
                                         NULL, G_PARAM_READABLE));

        workerclass->ThreadFunc = (NFlickWorkerThreadFunc) thread_func;

        ParentClass = g_type_class_ref (NFLICK_TYPE_WORKER);
}

static void                     nflick_auth_worker_init (NFlickAuthWorker *self)
{
        g_return_if_fail (NFLICK_IS_AUTH_WORKER (self));

        self->Private = NULL;

        NFlickAuthWorkerPrivate *priv = g_new0 (NFlickAuthWorkerPrivate, 1);
        g_return_if_fail (priv != NULL);
        
        if (private_init (self, priv) == TRUE) {
                self->Private = priv;
                nflick_worker_set_message ((NFlickWorker *) self, gettext ("Authorizing token..."));
        } else {
                private_dispose (priv);
                g_free (priv);
                self->Private = NULL;
        }
}

static gboolean                 private_init (NFlickAuthWorker *self, NFlickAuthWorkerPrivate *private)
{
        g_return_val_if_fail (NFLICK_IS_AUTH_WORKER (self), FALSE);
        g_return_val_if_fail (private != NULL, FALSE);

        private->MiniToken = NULL;
        private->UserName = NULL;
        private->FullName = NULL;
        private->UserNsid = NULL;
        private->Token = NULL;

        return TRUE;
}

static void                     private_dispose (NFlickAuthWorkerPrivate *private)
{
        g_return_if_fail (private != NULL);

        if (private->MiniToken != NULL) {
                g_free (private->MiniToken);
                private->MiniToken = NULL;
        }

        if (private->UserName != NULL) {
                g_free (private->UserName);
                private->UserName = NULL;
        }

        if (private->FullName != NULL) {
                g_free (private->FullName);
                private->FullName = NULL;
        }

        if (private->Token != NULL) {
                g_free (private->Token);
                private->Token = NULL;
        }

        if (private->UserNsid != NULL) {
                g_free (private->UserNsid);
                private->UserNsid = NULL;
        }
}

static void                     nflick_auth_worker_dispose (NFlickAuthWorker *self)
{
        g_return_if_fail (NFLICK_IS_AUTH_WORKER (self));

        if (self->Private != NULL)
                private_dispose (self->Private);

        G_OBJECT_CLASS (ParentClass)->dispose (G_OBJECT (self));
}

static void                     nflick_auth_worker_finalize (NFlickAuthWorker *self)
{
        g_return_if_fail (NFLICK_IS_AUTH_WORKER (self));
        
        if (self->Private != NULL) {
                g_free (self->Private);
                self->Private = NULL;
        }

        G_OBJECT_CLASS (ParentClass)->finalize (G_OBJECT (self));
}

static NFlickWorkerStatus       thread_func (NFlickAuthWorker *self)
{
        NFlickApiRequest *full_token_request = NULL; 
        NFlickApiResponse *full_token_response = NULL;
        NFlickWorkerStatus status = NFLICK_WORKER_STATUS_OK;

        full_token_request = nflick_api_request_new (NFLICK_FLICKR_API_METHOD_GET_FULL_TOKEN);
        if (full_token_request == NULL)
                goto Error;
        
        nflick_api_request_add_parameter (full_token_request, 
                                          NFLICK_FLICKR_API_PARAM_MINI_TOKEN, 
                                          self->Private->MiniToken);

        nflick_api_request_sign (full_token_request);
        
        if (nflick_api_request_exec (full_token_request) != TRUE) {
                nflick_worker_set_network_error ((NFlickWorker *) self);
                goto Error;
        }

        if (nflick_worker_is_aborted ((NFlickWorker *) self) == TRUE)
                goto Abort;

        full_token_response = nflick_api_response_new_from_request (NFLICK_TYPE_GFT_RESPONSE, full_token_request);
        if (full_token_response == NULL)
                goto Error;

        if (nflick_worker_parse_api_response ((NFlickWorker*) self, full_token_response) == FALSE)
                goto Error;

        /* Get out variables */
        g_object_get (G_OBJECT (full_token_response), 
                      "username", &self->Private->UserName,
                      "fullname", &self->Private->FullName,
                      "usernsid", &self->Private->UserNsid,
                      "token", &self->Private->Token, NULL);

        if (self->Private->UserName == NULL ||
            self->Private->FullName == NULL ||
            self->Private->Token == NULL ||
            self->Private->UserNsid == NULL)
                goto Error;

        /* All ok */
        goto Done;

Abort:
        status = NFLICK_WORKER_STATUS_ABORTED;
        goto Done;

Error:
        status = NFLICK_WORKER_STATUS_ERROR;

Done:
        if (full_token_request != NULL) 
                g_object_unref (full_token_request);

        if (full_token_response != NULL) 
                g_object_unref (full_token_response);

        return status;
}

NFlickAuthWorker*               nflick_auth_worker_new (const gchar *minitoken)
{
        g_return_val_if_fail (minitoken != NULL, NULL);

        NFlickAuthWorker *self = g_object_new (NFLICK_TYPE_AUTH_WORKER, NULL);
        g_return_val_if_fail (self != NULL, NULL);

        if (self->Private == NULL) {
                g_object_unref (self);
                return NULL;
        }

        self->Private->MiniToken = g_strdup (minitoken);
        
        return self;
}

static void                     nflick_auth_worker_get_property (NFlickAuthWorker *self, guint propid, 
                                                                 GValue *value, GParamSpec *pspec)
{
        g_return_if_fail (NFLICK_IS_AUTH_WORKER (self));
        g_assert (self->Private != NULL);
                
        switch (propid) {
                
                case ARG_USER_NAME:
                        g_value_set_string (value, self->Private->UserName);
                break;
        
                case ARG_FULL_NAME:
                        g_value_set_string (value, self->Private->FullName);
                break;
 
                case ARG_TOKEN:
                        g_value_set_string (value, self->Private->Token);
                break;
       
                case ARG_USER_NSID:
                        g_value_set_string (value, self->Private->UserNsid);
                break;

                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (self), propid, pspec);
                break;
        }
}
