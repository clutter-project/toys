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

#include "nflick-pixbuf-fetch.h"
#include "nflick-pixbuf-fetch-private.h"

GdkPixbuf*                      nflick_pixbuf_fetch_try_cache (const gchar *token)
{
        return NULL;
}

GdkPixbuf*                      nflick_pixbuf_fetch (const gchar *url, gint32 width, gint32 height, const gchar *cache_token)
{
        g_return_val_if_fail (url != NULL, NULL);

        ne_uri *uri = NULL;         /* Neon uri */
        ne_request *request = NULL; /* Http request */
        ne_session *session = NULL; /* Neon session */
        gboolean result = TRUE;     
        GdkPixbuf *pixbuf = NULL;

        /* Allocate new neon uri */
        uri = g_new0 (ne_uri, 1);
        if (uri == NULL) {
                result = FALSE;
                goto Done;
        }

        /* Parse the incoming url into valid neon uri */
        if (ne_uri_parse (url, uri) || uri->host == NULL || uri->path == NULL) {
	        result = FALSE;
                goto Done;
        }

        /* Set defaults. */
        if (uri->scheme == NULL)
                uri->scheme = g_strdup ("http");
        if (uri->port == 0)
                uri->port = ne_uri_defaultport (uri->scheme);

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

        /* Allocate our struct */
        PixbufFetchHelper *helper = g_new0 (PixbufFetchHelper, 1);
        if (helper == NULL) {
                result = FALSE;
                goto Done;
        }
        
        helper->Loader = gdk_pixbuf_loader_new ();
        if (helper->Loader == NULL) {
                result = FALSE;
                goto Done;
        }

        // Open the cache file if applies...
        // FIXME: Move this shit as func param
        
        if (cache_token != NULL && 1) {
                gchar *file_name = NULL;
                file_name = get_cache_file (cache_token);
                if (file_name != NULL) {
                        helper->CacheFile = fopen (file_name, "wb");
                        g_free (file_name);
                }
        }
	
        g_signal_connect (G_OBJECT (helper->Loader), "size-prepared", (gpointer) on_size_prepared, helper);

        helper->Width = width;
        helper->Height = height;

        ne_add_response_body_reader (request, ne_accept_always, (gpointer) block_reader, helper);

        result = (ne_request_dispatch (request) == NE_OK) ? TRUE : FALSE;

        if (helper->CacheFile != NULL)
                fclose (helper->CacheFile);
        gdk_pixbuf_loader_close (helper->Loader, NULL); 
        
        if (result == TRUE) {
                pixbuf = gdk_pixbuf_loader_get_pixbuf (helper->Loader);
                if (pixbuf)
                        g_object_ref (pixbuf);
        } else {
                // FIXME: Remove the cached file
        }

Done:
        if (uri != NULL) {
                ne_uri_free (uri);
                g_free (uri);
        }

        if (session != NULL)
                ne_session_destroy (session);

        if (request != NULL)
                ne_request_destroy (request);

        if (helper != NULL) {
                if (helper->Loader != NULL)
                        g_object_unref (helper->Loader);
                g_free (helper);
        }

        return pixbuf;
}

static gchar*                   get_cache_file (const gchar *token)
{
        g_return_val_if_fail (token != NULL, NULL);

        return g_build_filename ("cache", token, NULL);
}

static int                      block_reader (PixbufFetchHelper *helper, gchar *buffer, int len)
{
        g_return_val_if_fail (helper != NULL, -1);
        g_return_val_if_fail (helper->Loader != NULL, -1);

        if (helper->CacheFile != NULL)
                fwrite (buffer, 1, len, helper->CacheFile);

        gdk_pixbuf_loader_write (helper->Loader, buffer, len, NULL);
        
        return 0; 
}

static void                     on_size_prepared (GdkPixbufLoader *loader, gint width, gint height, PixbufFetchHelper *helper)
{
        g_return_if_fail (helper != NULL);

        if (helper->Width == 0 && helper->Height == 0)
                return;

        if (width != helper->Width && height != helper->Height)
                gdk_pixbuf_loader_set_size (loader, helper->Width, helper->Height);
}

