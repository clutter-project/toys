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

#ifndef __NFLICKSHOWWORKER_H__
#define __NFLICKSHOWWORKER_H__
 
#include <gtk/gtk.h>
#include <libintl.h>
#include "nflick-worker.h"
#include "nflick-api-request.h"
#include "nflick-api-response.h"
#include "nflick-get-sizes-response.h"
#include "nflick-set-list-response.h"
#include "nflick-photo-list-response.h"
#include "nflick-photo-set.h"
#include "nflick-types.h"
#include "nflick-pixbuf-fetch.h"

struct                          _NFlickShowWorker
{
        NFlickWorker Parent;
        NFlickShowWorkerPrivate *Private;
};

struct                          _NFlickShowWorkerClass 
{
        NFlickWorkerClass ParentClass;
};

GType                           nflick_show_worker_get_type (void);

NFlickShowWorker*               nflick_show_worker_new (const gchar *photoid, gint32 width, gint32 height, const gchar *token);

#endif
