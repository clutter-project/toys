/*
* Authored By Neil Jagdish Patel <njp@o-hand.com>
 *
 * Copyright (C) 2007 OpenedHand
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <libaaina/aaina-photo.h>
#include <libnflick/nflick-photo-search-worker.h>
#include <libnflick/nflick.h>
#include "aaina-source-flickr.h"

G_DEFINE_TYPE (AainaSourceFlickr, aaina_source_flickr, AAINA_TYPE_SOURCE);

#define AAINA_SOURCE_FLICKR_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
	AAINA_TYPE_SOURCE_FLICKR, \
	AainaSourceFlickrPrivate))

#define CHECK_TIMEOUT 20000

struct _AainaSourceFlickrPrivate
{
  AainaLibrary *library;
  gchar        *tags;

  /* table of already downloaded photos */
  GHashTable   *table;

  /* Queue of photos to download */
  GQueue       *queue;
  gboolean      running;
  NFlickWorker *pix_worker;
  AainaPhoto   *current;

  NFlickWorker *worker;

};

static gboolean get_photos (AainaSourceFlickr *source);
static gboolean get_pixbuf (AainaSourceFlickr *source);


static gboolean
on_pixbuf_thread_abort (AainaSourceFlickr *source)
{
  g_print ("abort\n");
  return FALSE;
}

static gboolean
on_pixbuf_thread_error (AainaSourceFlickr *source)
{
  AainaSourceFlickrPrivate *priv;
  gchar *error = NULL;

  g_return_val_if_fail (AAINA_IS_SOURCE_FLICKR (source), FALSE);
  priv = source->priv;

  g_object_get (G_OBJECT (priv->pix_worker), "error", &error, NULL);
  if (error)
  {
    g_warning ("%s\n", error);
  }
  else
    g_print ("error\n");
  return FALSE;
}

static gboolean
on_pixbuf_thread_ok (AainaSourceFlickr *source)
{
  AainaSourceFlickrPrivate *priv;
  GdkPixbuf *pixbuf;
  
  g_return_val_if_fail (AAINA_IS_SOURCE_FLICKR (source), FALSE);
  priv = source->priv;

  g_object_get (G_OBJECT (priv->pix_worker), "pixbuf", &pixbuf, NULL);

  /* Set the current photo's pixbuf and add it to the library */
  if (pixbuf)
  {
    aaina_photo_set_pixbuf (priv->current, pixbuf);
    aaina_library_append_photo (priv->library, priv->current);
    priv->current = NULL;

    g_print ("Got pixbuf\n");
  }
  else
  {
    g_print ("No pixbuf\n");
  }
  
  /* Now we do the work for the next one */
  if (g_queue_get_length (priv->queue))
  {
    priv->current = AAINA_PHOTO (g_queue_pop_head (priv->queue));
    g_timeout_add (100, (GSourceFunc)get_pixbuf, (gpointer)source);

    priv->running = TRUE;
  }
  else
    priv->running = FALSE;

  return FALSE;

}

static gboolean
get_pixbuf (AainaSourceFlickr *source)
{
  AainaSourceFlickrPrivate *priv;
  NFlickWorker *worker;
  AainaPhoto *photo;
  gchar *id;

  g_return_val_if_fail (AAINA_IS_SOURCE_FLICKR (source), FALSE);
  priv = source->priv;

  if (priv->current == NULL)
    return FALSE;

  photo = priv->current;
  g_object_get (G_OBJECT (photo), "id", &id, NULL);

  g_print ("Getting %s\n", id);

  worker = (NFlickWorker*)nflick_show_worker_new (id,
                                                  CLUTTER_STAGE_WIDTH ()/2,
                                                  CLUTTER_STAGE_HEIGHT ()/2,
                                                  " ");
  priv->pix_worker = worker;

  nflick_worker_set_custom_data (worker, source);
  nflick_worker_set_aborted_idle (worker, 
                                  (NFlickWorkerIdleFunc)on_pixbuf_thread_abort);
  nflick_worker_set_error_idle (worker, 
                                (NFlickWorkerIdleFunc)on_pixbuf_thread_error);
  nflick_worker_set_ok_idle (worker,
                             (NFlickWorkerIdleFunc)on_pixbuf_thread_ok);

  nflick_worker_start (worker);  

  priv->running = TRUE;
  return FALSE;
}


static gboolean
on_thread_abort (AainaSourceFlickr *source)
{
  g_print ("abort\n");
  return FALSE;
}

static gboolean
on_thread_error (AainaSourceFlickr *source)
{
  g_print ("error\n");
  return FALSE;
}

static gboolean
on_thread_ok (AainaSourceFlickr *source)
{
  AainaSourceFlickrPrivate *priv;
  GList *list = NULL, *l;

  g_return_val_if_fail (AAINA_IS_SOURCE_FLICKR (source), FALSE);
  priv = source->priv;

  /* Grab the list of photos from the worker */
  list = nflick_photo_search_worker_take_list (NFLICK_PHOTO_SEARCH_WORKER 
                                                                (priv->worker));
  g_print ("\n\n");
  for (l = list; l != NULL; l = l->next)
  {
    FlickrPhoto *photo = (FlickrPhoto*)l->data;
    ClutterActor *aphoto = NULL;
    gpointer res;

    if (!photo)
      continue;

    /* If the photo is already present, return */
    if (g_hash_table_lookup (priv->table, photo->id) != NULL)
      continue;
    
    /* Insert into the hash table */
    g_hash_table_insert (priv->table, 
                         g_strdup (photo->id), 
                         GINT_TO_POINTER (1));

    /* Create a aaina photo objec, but don't add it to the library yet */
    aphoto = aaina_photo_new ();
    g_object_set (G_OBJECT (aphoto),
                  "id", photo->id,
                  "title", photo->title, 
                  "author", photo->user,
                  NULL);
    
    /* Add the photo to the download queue */
    g_queue_push_tail (priv->queue, (gpointer)aphoto);
    
    g_print ("%s : %s\n", photo->id, photo->title);

    /* Free the the photo struct and its data */
    g_free (photo->id);
    g_free (photo->title);
    g_free (photo->user);
    g_free (photo);
    l->data = NULL;
  }
  g_print ("Table size = %d\n", g_hash_table_size (priv->table));

  g_list_free (list);

  g_timeout_add (CHECK_TIMEOUT, (GSourceFunc)get_photos, (gpointer)source);
  if (!priv->running)
  {
    priv->current = AAINA_PHOTO (g_queue_pop_head (priv->queue));
    g_timeout_add (200, (GSourceFunc)get_pixbuf, (gpointer)source);
  }
  return FALSE;
}

static gboolean
get_photos (AainaSourceFlickr *source)
{
  NFlickWorker *worker;
  NFlickWorkerStatus *status;

  worker = (NFlickWorker*)nflick_photo_search_worker_new (source->priv->tags,
                                                          " ");
  if (G_IS_OBJECT (source->priv->worker))
    g_object_unref (G_OBJECT (source->priv->worker));

  source->priv->worker = worker;

  nflick_worker_set_custom_data (worker, source);
  nflick_worker_set_aborted_idle (worker, 
                                  (NFlickWorkerIdleFunc)on_thread_abort);
  nflick_worker_set_error_idle (worker, 
                                (NFlickWorkerIdleFunc)on_thread_error);
  nflick_worker_set_ok_idle (worker,
                             (NFlickWorkerIdleFunc)on_thread_ok);

  nflick_worker_start (worker);

  return FALSE;
}

/* GObject stuff */
static void
aaina_source_flickr_class_init (AainaSourceFlickrClass *klass)
{
  GObjectClass    *gobject_class = G_OBJECT_CLASS (klass);
  
  g_type_class_add_private (gobject_class, sizeof (AainaSourceFlickrPrivate));

}


static void
aaina_source_flickr_init (AainaSourceFlickr *source_flickr)
{
  AainaSourceFlickrPrivate *priv;

  priv = source_flickr->priv = AAINA_SOURCE_FLICKR_GET_PRIVATE (source_flickr);

  priv->table = g_hash_table_new ((GHashFunc)g_str_hash, 
                                  (GEqualFunc)g_str_equal);

  priv->queue = g_queue_new ();
  priv->running = FALSE;
}

AainaSource*
aaina_source_flickr_new (AainaLibrary *library, const gchar *tags)
{
  AainaSourceFlickr         *source_flickr;

  source_flickr = g_object_new (AAINA_TYPE_SOURCE_FLICKR, 
                                   NULL);

  source_flickr->priv->tags = g_strdup (tags);
  source_flickr->priv->library = library;
  get_photos (source_flickr);

  return AAINA_SOURCE (source_flickr);
}

