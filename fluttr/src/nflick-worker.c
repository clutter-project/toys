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

#include "nflick-worker.h"
#include "nflick-worker-private.h"

GType                           nflick_worker_get_type (void)
{
        static GType objecttype = 0;

        if (!objecttype) {

                static const GTypeInfo objectinfo = {
                        sizeof (NFlickWorkerClass), 
                        NULL, 
                        NULL,
                        (GClassInitFunc) nflick_worker_class_init,
                        NULL,
                        NULL, 
                        sizeof (NFlickWorker), 
                        4, 
                        (GInstanceInitFunc) nflick_worker_init,
                };
                objecttype = g_type_register_static (G_TYPE_OBJECT, "NFlickWorker",
                                                     &objectinfo, 0);
        }
        return objecttype;
}

static void                     nflick_worker_class_init (NFlickWorkerClass *klass)
{
        GObjectClass *gobjectclass = (GObjectClass *) klass;

        gobjectclass->dispose = (gpointer) nflick_worker_dispose;
        gobjectclass->finalize = (gpointer) nflick_worker_finalize;
        gobjectclass->get_property = (gpointer) nflick_worker_get_property;
        
        g_object_class_install_property (gobjectclass, ARG_ERROR,
                                         g_param_spec_string
                                         ("error", "Error", "Message describing the error",
                                         NULL, G_PARAM_READABLE));

        g_object_class_install_property (gobjectclass, ARG_STATUS,
                                         g_param_spec_int 
                                         ("status", "Status", "Current worker status",
                                         -5000, 5000, NFLICK_WORKER_STATUS_IDLE, G_PARAM_READABLE));
        /* FIXME Use actual max/min vals for int */
                                         
        g_object_class_install_property (gobjectclass, ARG_MESSAGE,
                                         g_param_spec_string
                                         ("message", "Message", "Message describing the thread status",
                                         NULL, G_PARAM_READABLE));

        ParentClass = g_type_class_ref (G_TYPE_OBJECT);

        klass->ThreadFunc = NULL;
}

static void                     nflick_worker_init (NFlickWorker *self)
{
        g_return_if_fail (NFLICK_IS_WORKER (self));

        self->Private = NULL;

        NFlickWorkerPrivate *priv = g_new0 (NFlickWorkerPrivate, 1);
        g_return_if_fail (priv != NULL);
        
        if (private_init (self, priv) == TRUE) 
                self->Private = priv;
        else {
                private_dispose (priv);
                g_free (priv);
                self->Private = NULL;
        }
}

static gboolean                 private_init (NFlickWorker *self, NFlickWorkerPrivate *private)
{
        g_return_val_if_fail (NFLICK_IS_WORKER (self), FALSE);
        g_return_val_if_fail (private != NULL, FALSE);

        private->Thread = NULL;

        private->Mutex = g_mutex_new ();
        g_return_val_if_fail (private->Mutex != NULL, FALSE);

        private->Started = FALSE;
        private->Status = NFLICK_WORKER_STATUS_IDLE;
        private->Error = NULL;
        private->AbortRequested = FALSE;
        
        /* Null the idle functions */
        private->OkIdle = NULL;
        private->AbortedIdle = NULL;
        private->MsgChangeIdle = NULL;
        private->ErrorIdle = NULL;
        private->CustomData = NULL;
        
        /* Initialize the message to a stubby one */
        private->Message = g_strdup (gettext ("Working..."));
        
        return TRUE;
}

static void                     private_dispose (NFlickWorkerPrivate *private)
{
        g_return_if_fail (private != NULL);

        if (private->Thread != NULL) {
                g_thread_join (private->Thread);
                private->Thread = NULL;
        }

        if (private->Mutex != NULL) {
                g_mutex_free (private->Mutex);
                private->Mutex = NULL;
        }

        if (private->Error != NULL) {
                g_free (private->Error);
                private->Error = NULL;
        }

        if (private->Message != NULL) {
                g_free (private->Message);
                private->Message = NULL;
        }
}

void                            nflick_worker_start (NFlickWorker *self)
{
        g_return_if_fail (NFLICK_IS_WORKER (self));

        WORKER_LOCK (self);
        if (self->Private->Started == TRUE) {
                g_warning ("Worker was already started");
        } else {
                self->Private->Thread = g_thread_create ((GThreadFunc) thread_start, self, TRUE, NULL);
                /* FIXME Check for NULL */
        }

        WORKER_UNLOCK (self);
}

static void                     thread_start (NFlickWorker *self)
{
        g_return_if_fail (NFLICK_IS_WORKER (self));

        WORKER_LOCK (self);

        /* Get the class and call the proper function */
        NFlickWorkerClass *klass = (NFlickWorkerClass *) G_OBJECT_GET_CLASS (self);
        g_assert (klass != NULL);

        if (klass->ThreadFunc == NULL) {
                g_warning ("No thread func");
                set_error_no_lock (self, gettext ("Internal threading error, no thread function. "
                                                  "Please file a bug report."));
                self->Private->Status = NFLICK_WORKER_STATUS_ERROR;
                
                if (self->Private->ErrorIdle != NULL) 
                        g_idle_add ((GSourceFunc) self->Private->ErrorIdle, 
                                    (self->Private->CustomData != NULL) ? self->Private->CustomData : self);
                                    

                WORKER_UNLOCK (self);
                goto Done;
        }

        self->Private->Status = NFLICK_WORKER_STATUS_RUNNING;
        WORKER_UNLOCK (self);

        /* Here we're waiting, waiting, waiting... */
        NFlickWorkerStatus status = klass->ThreadFunc (self);
        
        WORKER_LOCK (self);
        
        /* Our last chance for an abort */
        if (self->Private->AbortRequested == TRUE)
                status = NFLICK_WORKER_STATUS_ABORTED;
        
        self->Private->Status = status;
        
        switch (status) {
                
                case NFLICK_WORKER_STATUS_RUNNING:
                case NFLICK_WORKER_STATUS_IDLE:
                self->Private->Status = NFLICK_WORKER_STATUS_ERROR;
                set_error_no_lock (self, gettext ("Internal threading error, thread in running after function done. "
                                                  "Please file a bug report."));
                /* Fire error func */
                if (self->Private->ErrorIdle != NULL) 
                        g_idle_add ((GSourceFunc) self->Private->ErrorIdle, 
                                    (self->Private->CustomData != NULL) ? self->Private->CustomData : self);
                break;
                
                case NFLICK_WORKER_STATUS_ERROR:
                if (self->Private->Error == NULL)
                        set_error_no_lock (self, gettext ("Error in thread, but no error was set. "
                                                          "Please file a bug report."));
                /* Fire error func */
                if (self->Private->ErrorIdle != NULL) 
                        g_idle_add ((GSourceFunc) self->Private->ErrorIdle, 
                                    (self->Private->CustomData != NULL) ? self->Private->CustomData : self);
                break;
                
                case NFLICK_WORKER_STATUS_OK:
                /* Fire ok func */
                if (self->Private->OkIdle != NULL) 
                        g_idle_add ((GSourceFunc) self->Private->OkIdle, 
                                    (self->Private->CustomData != NULL) ? self->Private->CustomData : self);

                break;
                
                case NFLICK_WORKER_STATUS_ABORTED:
                /* Fire aborted func */
                if (self->Private->AbortedIdle != NULL) 
                        g_idle_add ((GSourceFunc) self->Private->AbortedIdle, 
                                    (self->Private->CustomData != NULL) ? self->Private->CustomData : self);

                break;
        }
        
        WORKER_UNLOCK (self);

        Done:
        return;
}

static void                     set_error_no_lock (NFlickWorker *self, const gchar *error)
{
        g_return_if_fail (NFLICK_IS_WORKER (self));
        
        if (self->Private->Error != NULL)
                g_free (self->Private->Error);

        self->Private->Error = g_strdup (error);
}

void                            nflick_worker_set_message (NFlickWorker *self, const gchar *msg)
{
        g_return_if_fail (NFLICK_IS_WORKER (self));

        WORKER_LOCK (self);
        if (self->Private->Message != NULL)
                g_free (self->Private->Message);
        
        self->Private->Message = g_strdup (msg);
        
        /* Notify */
        if (self->Private->MsgChangeIdle != NULL) 
                g_idle_add ((GSourceFunc) self->Private->MsgChangeIdle, 
                            (self->Private->CustomData != NULL) ? self->Private->CustomData : self);
        
        WORKER_UNLOCK (self);
}

void                            nflick_worker_set_network_error (NFlickWorker *self)
{
        g_return_if_fail (NFLICK_IS_WORKER (self));

        nflick_worker_set_error (self, gettext ("A network error occured while trying to connect to flickr. "
                                                "Please check your connection settings."));
}

gboolean                        nflick_worker_parse_api_response (NFlickWorker *self, NFlickApiResponse *response)
{
        g_return_val_if_fail (NFLICK_IS_WORKER (self), FALSE);
        g_return_val_if_fail (NFLICK_IS_API_RESPONSE (response), FALSE);

        gboolean success = FALSE;

        g_object_get (G_OBJECT (response), "success", &success, NULL);

        if (success == TRUE)
                return TRUE;
        else {
                gboolean parse_error = FALSE;
                gchar *error = NULL;

                g_object_get (G_OBJECT (response), "error", &error, "parseerror", &parse_error, NULL);
                
                if (parse_error == TRUE) {
                        gchar *e = g_strdup_printf ("%s\n\n%s", 
                                                    gettext ("An error occurred while parsing the flickr api response. "
                                                             "Please file a bug report. Error details: "), error);
                        nflick_worker_set_error (self, e);
                        if (e != NULL)
                                g_free (e);
                } else 
                        nflick_worker_set_error (self, error);
                
                if (error != NULL)
                        g_free (error);

                return FALSE;
        }
}

void                            nflick_worker_set_error (NFlickWorker *self, const gchar *error)
{
        g_return_if_fail (NFLICK_IS_WORKER (self));
        g_return_if_fail (error != NULL);

        WORKER_LOCK (self);
        set_error_no_lock (self, error);
        WORKER_UNLOCK (self);
}

void                            nflick_worker_set_custom_data (NFlickWorker *self, gpointer data)
{
        g_return_if_fail (NFLICK_IS_WORKER (self));
        
        WORKER_LOCK (self);
        self->Private->CustomData = data;
        WORKER_UNLOCK (self);
}

void                            nflick_worker_set_aborted_idle (NFlickWorker *self, NFlickWorkerIdleFunc func)
{
        g_return_if_fail (NFLICK_IS_WORKER (self));
        
        WORKER_LOCK (self);
        self->Private->AbortedIdle = func;
        WORKER_UNLOCK (self);
}

void                            nflick_worker_set_ok_idle (NFlickWorker *self, NFlickWorkerIdleFunc func)
{
        g_return_if_fail (NFLICK_IS_WORKER (self));
        
        WORKER_LOCK (self);
        self->Private->OkIdle = func;
        WORKER_UNLOCK (self);
}

void                            nflick_worker_set_error_idle (NFlickWorker *self, NFlickWorkerIdleFunc func)
{
        g_return_if_fail (NFLICK_IS_WORKER (self));
        
        WORKER_LOCK (self);
        self->Private->ErrorIdle = func;
        WORKER_UNLOCK (self);
}

void                            nflick_worker_request_abort (NFlickWorker *self)
{
        g_return_if_fail (NFLICK_IS_WORKER (self));
        
        WORKER_LOCK (self);
        self->Private->AbortRequested = TRUE;
        WORKER_UNLOCK (self);
}

gboolean                        nflick_worker_is_aborted (NFlickWorker *self)
{
        g_return_val_if_fail (NFLICK_IS_WORKER (self), FALSE);
        
        WORKER_LOCK (self);
        gboolean ret = self->Private->AbortRequested;
        WORKER_UNLOCK (self);
        
        return ret;
}

void                            nflick_worker_set_msg_change_idle (NFlickWorker *self, NFlickWorkerIdleFunc func)
{
        g_return_if_fail (NFLICK_IS_WORKER (self));
        
        WORKER_LOCK (self);
        self->Private->MsgChangeIdle = func;
        WORKER_UNLOCK (self);
}

static void                     nflick_worker_dispose (NFlickWorker *self)
{
        g_return_if_fail (NFLICK_IS_WORKER (self));

        if (self->Private != NULL)
                private_dispose (self->Private);

        G_OBJECT_CLASS (ParentClass)->dispose (G_OBJECT (self));
}

static void                     nflick_worker_finalize (NFlickWorker *self)
{
        g_return_if_fail (NFLICK_IS_WORKER (self));
        
        if (self->Private != NULL) {
                g_free (self->Private);
                self->Private = NULL;
        }

        G_OBJECT_CLASS (ParentClass)->finalize (G_OBJECT (self));
}

static void                     nflick_worker_get_property (NFlickWorker *self, guint propid, 
                                                            GValue *value, GParamSpec *pspec)
{
        g_return_if_fail (NFLICK_IS_WORKER (self));
        g_assert (self->Private != NULL);
                
        switch (propid) {
                
                case ARG_ERROR:
                        WORKER_LOCK (self);
                        g_value_set_string (value, self->Private->Error);
                        WORKER_UNLOCK (self);
                break;
        
                case ARG_STATUS:
                        WORKER_LOCK (self);
                        g_value_set_int (value, self->Private->Status);
                        WORKER_UNLOCK (self);
                break;
        
                case ARG_MESSAGE:
                        WORKER_LOCK (self);
                        g_value_set_string (value, self->Private->Message);
                        WORKER_UNLOCK (self);
                        break;

                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (self), propid, pspec);
                break;
        }
}
