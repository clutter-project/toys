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
#include "aaina-source-flickr.h"

G_DEFINE_TYPE (AainaSourceFlickr, aaina_source_flickr, AAINA_TYPE_SOURCE);
	
/* GObject stuff */
static void
aaina_source_flickr_class_init (AainaSourceFlickrClass *klass)
{
  ;
}


static void
aaina_source_flickr_init (AainaSourceFlickr *source_flickr)
{
  ;
}

static void
on_thread_abort (void *null)
{
  g_print ("abort\n");
}

static void
on_thread_error (void *null)
{
  g_print ("error\n");
}

static void
on_thread_ok (void *null)
{
  g_print ("ok\n");
}

static void
get_photos (void)
{
  NFlickWorker *worker;
  NFlickWorkerStatus *status;

  worker = (NFlickWorker*)nflick_photo_search_worker_new (" ", " ");

  nflick_worker_set_aborted_idle (worker, 
                                  (NFlickWorkerIdleFunc)on_thread_abort);
  nflick_worker_set_error_idle (worker, 
                                (NFlickWorkerIdleFunc)on_thread_error);
  nflick_worker_set_ok_idle (worker,
                             (NFlickWorkerIdleFunc)on_thread_ok);

  nflick_worker_start (worker);
}

AainaSource*
aaina_source_flickr_new (AainaLibrary *library, const gchar *dir)
{
  AainaSourceFlickr         *source_flickr;

  source_flickr = g_object_new (AAINA_TYPE_SOURCE_FLICKR, 
                                   NULL);


  get_photos ();

  return AAINA_SOURCE (source_flickr);
}

