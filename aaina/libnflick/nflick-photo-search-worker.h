/******************************************************************************/
/*                                                                            */
/* GPL license, Copyright (c) 2005-2006 by:                                   */
/*                                                                            */
/* Authors:                                                                   */
/*      Michael Dominic K. <michaldominik@gmail.com>                          */
/*      Neil J. Patel      <njp@o-hand.com>                                   */
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

#ifndef __NFLICKSETLISTWORKER_H__
#define __NFLICKSETLISTWORKER_H__
 
#include <gtk/gtk.h>
#include <libintl.h>
#include "nflick-worker.h"
#include "nflick-api-request.h"
#include "nflick-api-response.h"
#include "nflick-set-list-response.h"
#include "nflick-photo-list-response.h"
#include "nflick-photo-set.h"
#include "nflick-types.h"
#include "nflick-no-set-response.h"

typedef struct _NFlickPhotoSearchWorker NFlickPhotoSearchWorker;
typedef struct _NFlickPhotoSearchWorkerClass NFlickPhotoSearchWorkerClass;
typedef struct _NFlickPhotoSearchWorkerPrivate NFlickPhotoSearchWorkerPrivate;

struct _NFlickPhotoSearchWorker
{
        NFlickWorker Parent;
        NFlickPhotoSearchWorkerPrivate *Private;
};

struct _NFlickPhotoSearchWorkerClass 
{
        NFlickWorkerClass ParentClass;
};

GType nflick_photo_search_worker_get_type (void);

NFlickPhotoSearchWorker*            
nflick_photo_search_worker_new (const gchar *usernsid, const gchar *token);

GList*
nflick_photo_search_worker_take_list (NFlickPhotoSearchWorker *self);

#endif
