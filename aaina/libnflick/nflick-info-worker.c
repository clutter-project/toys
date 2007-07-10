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

#include "nflick-info-worker.h"
#include "nflick-info-worker-private.h"

GType                           
nflick_info_worker_get_type (void)
{
        static GType objecttype = 0;

        if (!objecttype) {

                static const GTypeInfo objectinfo = {
                        sizeof (NFlickInfoWorkerClass), 
                        NULL, 
                        NULL,
                        (GClassInitFunc) nflick_info_worker_class_init,
                        NULL,
                        NULL, 
                        sizeof (NFlickInfoWorker), 
                        4, 
                        (GInstanceInitFunc) nflick_info_worker_init,
                };
                objecttype = g_type_register_static (NFLICK_TYPE_WORKER, "NFlickInfoWorker",
                                                     &objectinfo, 0);
        }
        return objecttype;
}

static void                     
nflick_info_worker_class_init (NFlickInfoWorkerClass *klass)
{
        GObjectClass *gobjectclass = (GObjectClass *) klass;
        NFlickWorkerClass *workerclass = (NFlickWorkerClass *) klass;

        gobjectclass->dispose = (gpointer) nflick_info_worker_dispose;
        gobjectclass->finalize = (gpointer) nflick_info_worker_finalize;
        gobjectclass->get_property = (gpointer) nflick_info_worker_get_property;

        g_object_class_install_property (gobjectclass, ARG_PIXBUF,
                                         g_param_spec_object 
                                         ("pixbuf", "Pixbuf", "Pixbuf",
                                         GDK_TYPE_PIXBUF, G_PARAM_READABLE));
        
        workerclass->ThreadFunc = (NFlickWorkerThreadFunc) thread_func;

        ParentClass = g_type_class_ref (NFLICK_TYPE_WORKER);
}

static void                     
nflick_info_worker_init (NFlickInfoWorker *self)
{
        g_return_if_fail (NFLICK_IS_INFO_WORKER (self));

        self->Private = NULL;

        NFlickInfoWorkerPrivate *priv = g_new0 (NFlickInfoWorkerPrivate, 1);
        g_return_if_fail (priv != NULL);
        
        if (private_init (self, priv) == TRUE) {
                self->Private = priv;
                nflick_worker_set_message ((NFlickWorker *) self, 
                			    gettext ("Loading photo..."));
        } else {
                private_dispose (priv);
                g_free (priv);
                self->Private = NULL;
        }
}

static gboolean                 
private_init (NFlickInfoWorker *self, NFlickInfoWorkerPrivate *private)
{
        g_return_val_if_fail (NFLICK_IS_INFO_WORKER (self), FALSE);
        g_return_val_if_fail (private != NULL, FALSE);

        private->PhotoId = NULL;
        private->Token = NULL;

        return TRUE;
}

static void                     
private_dispose (NFlickInfoWorkerPrivate *private)
{
        g_return_if_fail (private != NULL);

        if (private->Token != NULL) {
                g_free (private->Token);
                private->Token = NULL;
        }

        if (private->PhotoId != NULL) {
                g_free (private->PhotoId);
                private->PhotoId = NULL;
        }

        if (private->Pixbuf != NULL) {
                g_object_unref (private->Pixbuf);
                private->Pixbuf = NULL;
        }
}

static void                     
nflick_info_worker_dispose (NFlickInfoWorker *self)
{
        g_return_if_fail (NFLICK_IS_INFO_WORKER (self));

        if (self->Private != NULL)
                private_dispose (self->Private);

        G_OBJECT_CLASS (ParentClass)->dispose (G_OBJECT (self));
}

static void                    
nflick_info_worker_finalize (NFlickInfoWorker *self)
{
        g_return_if_fail (NFLICK_IS_INFO_WORKER (self));
        
        if (self->Private != NULL) {
                g_free (self->Private);
                self->Private = NULL;
        }

        G_OBJECT_CLASS (ParentClass)->finalize (G_OBJECT (self));
}

static NFlickWorkerStatus       
thread_func (NFlickInfoWorker *self)
{
        NFlickApiRequest *get_sizes_request = NULL; 
        NFlickApiResponse *get_sizes_response = NULL;
        gchar *uri = NULL;
        NFlickWorkerStatus status = NFLICK_WORKER_STATUS_OK;
        gdouble vbox_aspect = (gdouble) self->Private->Width / (gdouble) self->Private->Height;
        gdouble pixbuf_aspect = -1;
        gint32 final_width = -1;
        gint32 final_height = -1;
        gboolean rotated = FALSE;

        get_sizes_request = nflick_api_request_new ("flickr.photos.getinfo");
        if (get_sizes_request == NULL)
                goto Error;

        nflick_api_request_add_parameter (get_sizes_request, 
                                          NFLICK_FLICKR_API_PARAM_PHOTO_ID, 
                                          self->Private->PhotoId);

        nflick_api_request_sign (get_sizes_request);
        if (nflick_api_request_exec (get_sizes_request) != TRUE) 
        {
                nflick_worker_set_network_error ((NFlickWorker *) self);
                goto Error;
        }

        if (nflick_worker_is_aborted ((NFlickWorker *) self) == TRUE)
                goto Abort;

        get_sizes_response = 
              nflick_api_response_new_from_request (NFLICK_TYPE_INFO_RESPONSE, 
                                                    get_sizes_request);

        if (get_sizes_response == NULL)
                goto Error;

        if (nflick_worker_parse_api_response ((NFlickWorker*) self, 
                                              get_sizes_response) == FALSE)
                goto Error;
        
        nflick_info_response_get ((NFlickInfoResponse*)get_sizes_response,
                                  &self->Private->rotation,
                                  &self->Private->realname,
                                  &self->Private->desc);

        /* All ok */
        goto Done;

Abort:
        status = NFLICK_WORKER_STATUS_ABORTED;
        goto Done;

Error:
        status = NFLICK_WORKER_STATUS_ERROR;

Done:
        if (get_sizes_request != NULL) 
                g_object_unref (get_sizes_request);

        if (get_sizes_response != NULL) 
                g_object_unref (get_sizes_response);

        if (uri != NULL)
                g_free (uri);

        return status;
}

void
nflick_info_worker_get (NFlickInfoWorker    *self,
                        gchar              **rotation,
                        gchar              **realname,
                        gchar              **desc)
{
  g_return_if_fail (NFLICK_IS_INFO_WORKER (self));

  *rotation = self->Private->rotation;
  *realname = self->Private->realname;
  *desc = self->Private->desc;
}

NFlickInfoWorker*               
nflick_info_worker_new (const gchar *photoid, gint32 width, gint32 height, const gchar *token)
{
        g_return_val_if_fail (token != NULL, NULL);
        g_return_val_if_fail (photoid != NULL, NULL);

        NFlickInfoWorker *self = g_object_new (NFLICK_TYPE_INFO_WORKER, NULL);
        g_return_val_if_fail (self != NULL, NULL);

        if (self->Private == NULL) {
                g_object_unref (self);
                return NULL;
        }

        self->Private->Token = g_strdup (token);
        self->Private->PhotoId= g_strdup (photoid);
        self->Private->Width = width;
        self->Private->Height = height;

        return self;
}

static void                     
nflick_info_worker_get_property (NFlickInfoWorker *self, guint propid, 
                                                                 GValue *value, GParamSpec *pspec)
{
        g_return_if_fail (NFLICK_IS_INFO_WORKER (self));
        g_assert (self->Private != NULL);
                
        switch (propid) {
               
                case ARG_PIXBUF:
                        g_value_set_object (value, self->Private->Pixbuf);
                break;

                case ARG_ROTATION:

 
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (self), propid, pspec);
                break;

        }
}
