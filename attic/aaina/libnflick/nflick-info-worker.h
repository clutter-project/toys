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

#ifndef __NFLICKINFOWORKER_H__
#define __NFLICKINFOWORKER_H__
 
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

struct                          _NFlickInfoWorker
{
        NFlickWorker Parent;
        NFlickInfoWorkerPrivate *Private;
};

struct                          _NFlickInfoWorkerClass 
{
        NFlickWorkerClass ParentClass;
};

GType                           nflick_info_worker_get_type (void);

NFlickInfoWorker*               nflick_info_worker_new (const gchar *photoid, gint32 width, gint32 height, const gchar *token);

void
nflick_info_worker_get (NFlickInfoWorker    *self,
                        gchar              **rotation,
                        gchar              **realname,
                        gchar              **desc);
#endif
