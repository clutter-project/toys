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

#include "nflick-api-request.h"
#include "nflick-api-request-private.h"

GType                           nflick_api_request_get_type (void)
{
        static GType objecttype = 0;

        if (!objecttype) {

                static const GTypeInfo objectinfo = {
                        sizeof (NFlickApiRequestClass), 
                        NULL, 
                        NULL,
                        (GClassInitFunc) nflick_api_request_class_init,
                        NULL,
                        NULL, 
                        sizeof (NFlickApiRequest), 
                        4, 
                        (GInstanceInitFunc) nflick_api_request_init,
                };
                objecttype = g_type_register_static (G_TYPE_OBJECT, "NFlickApiRequest",
                                                     &objectinfo, 0);
        }
        return objecttype;
}

static void                     nflick_api_request_class_init (NFlickApiRequestClass *klass)
{
        GObjectClass *gobjectclass = (GObjectClass *) klass;

        gobjectclass->dispose = (gpointer) nflick_api_request_dispose;
        gobjectclass->finalize = (gpointer) nflick_api_request_finalize;

        ParentClass = g_type_class_ref (G_TYPE_OBJECT);
}

static void                     nflick_api_request_init (NFlickApiRequest *self)
{
        g_return_if_fail (NFLICK_IS_API_REQUEST (self));

        self->Private = NULL;

        NFlickApiRequestPrivate *priv = g_new0 (NFlickApiRequestPrivate, 1);
        g_return_if_fail (priv != NULL);
        
        if (private_init (self, priv) == TRUE) 
                self->Private = priv;
        else {
                private_dispose (priv);
                g_free (priv);
                self->Private = NULL;
        }
}

static gboolean                 private_init (NFlickApiRequest *self, NFlickApiRequestPrivate *private)
{
        g_return_val_if_fail (NFLICK_IS_API_REQUEST (self), FALSE);
        g_return_val_if_fail (private != NULL, FALSE);

        private->Hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
        g_return_val_if_fail (private->Hash != NULL, FALSE);

        private->Buffer = NULL;
        private->BytesRead = 0;

        return TRUE;
}

static void                     private_dispose (NFlickApiRequestPrivate *private)
{
        g_return_if_fail (private != NULL);

        if (private->Hash != NULL) {
                g_hash_table_destroy (private->Hash);
                private->Hash = NULL;
        }

        if (private->Buffer != NULL) {
                g_free (private->Buffer);
                private->Buffer = NULL;
        }
}

static void                     nflick_api_request_dispose (NFlickApiRequest *self)
{
        g_return_if_fail (NFLICK_IS_API_REQUEST (self));

        if (self->Private != NULL)
                private_dispose (self->Private);

        G_OBJECT_CLASS (ParentClass)->dispose (G_OBJECT (self));
}

static void                     nflick_api_request_finalize (NFlickApiRequest *self)
{
        g_return_if_fail (NFLICK_IS_API_REQUEST (self));
        
        if (self->Private != NULL) {
                g_free (self->Private);
                self->Private = NULL;
        }

        G_OBJECT_CLASS (ParentClass)->finalize (G_OBJECT (self));
}

NFlickApiRequest*               nflick_api_request_new (const gchar *method)
{
        g_return_val_if_fail (method != NULL, NULL);

        NFlickApiRequest *self = g_object_new (NFLICK_TYPE_API_REQUEST, NULL);
        g_return_val_if_fail (self != NULL, NULL);

        if (self->Private == NULL) {
                g_object_unref (self);
                return NULL;
        }

        nflick_api_request_add_parameter (self, NFLICK_FLICKR_API_PARAM_METHOD, method);
        nflick_api_request_add_parameter (self, NFLICK_FLICKR_API_PARAM_KEY, NFLICK_FLICKR_API_KEY);

        return self;
}

void                            nflick_api_request_add_parameter (NFlickApiRequest *self, 
                                                                  const gchar *param, const gchar *val)
{
        g_return_if_fail (NFLICK_IS_API_REQUEST (self));
        g_return_if_fail (param != NULL);

        g_hash_table_insert (self->Private->Hash, g_strdup (param), g_strdup (val));
}

static gchar*                   get_path (NFlickApiRequest *self)
{
        g_return_val_if_fail (NFLICK_IS_API_REQUEST (self), NULL);

        GList *list = NULL;
        gchar *str = NULL;
        g_hash_table_foreach (self->Private->Hash, (GHFunc) foreach_composer_list, &list);
        g_list_foreach (list, (GFunc) foreach_composer_str, &str);
        g_list_foreach (list, (GFunc) g_free, NULL);

        return str;
}

static gchar*                   get_path_sig (NFlickApiRequest *self)
{
        g_return_val_if_fail (NFLICK_IS_API_REQUEST (self), NULL);

        GList *list = NULL;
        gchar *str = g_strdup_printf ("%s", NFLICK_FLICKR_SHARED_SECRET);
        g_hash_table_foreach (self->Private->Hash, (GHFunc) foreach_composer_list_sig, &list);
        g_list_foreach (list, (GFunc) foreach_composer_str_sig, &str);
        g_list_foreach (list, (GFunc) g_free, NULL);

        return str;
}

static void                     foreach_composer_list (gchar *param, gchar *val, GList **list)
{
         /* Silently ignore empty vals */
        if (param == NULL || list == NULL)
                return;

        gchar *str = g_strdup_printf ("%s=%s", param, val);
        g_return_if_fail (str != NULL);

        *list = g_list_insert_sorted (*list, str, (GCompareFunc) strcmp);
}

static void                     foreach_composer_str (gchar *val, gchar **str)
{
        /* Silently ignore empty vals */
        if (val == NULL)
                return;

        gchar *old = *str;

        if (*str != NULL) { 
                *str = g_strdup_printf ("%s&%s", *str, val);
                g_free (old);
        } else
                *str = g_strdup_printf ("%s", val);
}

static void                     foreach_composer_list_sig (gchar *param, gchar *val, GList **list)
{
         /* Silently ignore empty vals */
        if (param == NULL || list == NULL)
                return;

        gchar *str = g_strdup_printf ("%s%s", param, val);
        g_return_if_fail (str != NULL);

        *list = g_list_insert_sorted (*list, str, (GCompareFunc) strcmp);
}

static void                     foreach_composer_str_sig (gchar *val, gchar **str)
{
        /* Silently ignore empty vals */
        if (val == NULL)
                return;

        gchar *old = *str;

        if (*str != NULL) { 
                *str = g_strdup_printf ("%s%s", *str, val);
                g_free (old);
        } else
                *str = g_strdup_printf ("%s", val);
}

gboolean                        nflick_api_request_sign (NFlickApiRequest *self)
{
        g_return_val_if_fail (NFLICK_IS_API_REQUEST (self), FALSE);

        gchar *path_sig = NULL;
        gpointer ctx = NULL;
        gpointer ctx_output = NULL;
        gchar *ascii = NULL;
        gboolean res = TRUE;

        path_sig = get_path_sig (self); 
        if (path_sig == NULL)
                goto Failure;

        ctx = ne_md5_create_ctx ();
        if (ctx == NULL)
                goto Failure;

        ne_md5_process_bytes (path_sig, strlen (path_sig), ctx);
        ctx_output = g_malloc (16);
        if (ctx_output == NULL)
                goto Failure;

        ne_md5_finish_ctx (ctx, ctx_output);
        ascii = g_malloc (33);
        if (ascii == NULL)
                goto Failure;

        ne_md5_to_ascii (ctx_output, ascii);
        if (ascii [32] != 0)
                goto Failure;

        /* Now it's time to sign it... */
        nflick_api_request_add_parameter (self, NFLICK_FLICKR_API_PARAM_SIGNATURE, ascii);

        goto Finish;

Failure:
        res = FALSE;
        g_warning ("Failure during md5 computation/signing");

Finish:
        if (path_sig != NULL)
                g_free (path_sig);
        if (ctx != NULL)
                g_free (ctx);
        if (ctx_output != NULL)
                g_free (ctx_output);
        if (ascii != NULL)
                g_free (ascii);

        return res;
}

static int                      block_reader (NFlickApiRequest *self, gchar *buffer, int len)
{
        g_return_val_if_fail (NFLICK_IS_API_REQUEST (self), -1);

        if (self->Private->Buffer == NULL) {
                self->Private->Buffer = g_malloc (len + 1);
                memcpy (self->Private->Buffer, buffer, len);
                self->Private->Buffer [len] = 0;
                self->Private->BytesRead = 0;
        } else {
                gchar *old_ptr = self->Private->Buffer;
                self->Private->Buffer = g_malloc (self->Private->BytesRead + len + 1);
                memcpy (self->Private->Buffer, old_ptr, self->Private->BytesRead);
                memcpy (self->Private->Buffer + self->Private->BytesRead, buffer, len);
                self->Private->Buffer [len + self->Private->BytesRead] = 0;
                
                g_free (old_ptr);
        }

        self->Private->BytesRead += len;
        return 0; 
}

gboolean                        nflick_api_request_exec (NFlickApiRequest *self)
{
        g_return_val_if_fail (NFLICK_IS_API_REQUEST (self), FALSE);

        gchar *path_str = NULL;     /* The full path */
        gchar *uri_str = NULL;      /* The actual uri to use */
        ne_uri *uri = NULL;         /* Neon uri */
        ne_request *request = NULL; /* Http request */
        ne_session *session = NULL; /* Neon session */
        gboolean result = TRUE;     /* result */

        path_str = get_path (self);
        if (path_str == NULL) {
                result = FALSE;
                goto Done;
        }
        
        uri_str = g_strdup_printf ("%s?%s", NFLICK_FLICKR_REST_END_POINT, path_str);
        if (uri_str == NULL) {
                result = FALSE;
                goto Done;
        }

        uri = g_new0 (ne_uri, 1);
        if (uri == NULL) {
                result = FALSE;
                goto Done;
        }

        /* Fill-out the params */
        uri->scheme = "http";
        uri->port = ne_uri_defaultport (uri->scheme);  
        uri->host = NFLICK_FLICKR_HOST;
        uri->path = uri_str;

        /* Create the session */
        session = ne_session_create (uri->scheme, uri->host, uri->port);
        if (session == NULL) {
                result = FALSE;
                goto Done;
        }

        /* Create the request */
        request = ne_request_create (session, "GET", uri->path);
        if (request == NULL) {
                result = FALSE;
                goto Done;
        }

        ne_add_response_body_reader (request, ne_accept_always, (gpointer) block_reader, self);

        result == (ne_request_dispatch (request) == NE_OK) ? TRUE : FALSE;
        if (self->Private->Buffer == NULL)
                result = FALSE;

Done:
        if (path_str != NULL)
                g_free (path_str);

        if (uri_str != NULL)
                g_free (uri_str);

        if (uri != NULL)
                g_free (uri);

        if (session != NULL)
                ne_session_destroy (session);

        if (request != NULL)
                ne_request_destroy (request);

        return result;
}

gchar*                          nflick_api_request_take_buffer (NFlickApiRequest *self)

{
        g_return_val_if_fail (NFLICK_IS_API_REQUEST (self), NULL);

        gchar *buf = self->Private->Buffer;
        self->Private->Buffer = NULL;

        return buf;
}
