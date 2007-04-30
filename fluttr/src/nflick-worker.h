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

#ifndef __NFLICKWORKER_H__
#define __NFLICKWORKER_H__
 
#include <gtk/gtk.h>
#include <libintl.h>
#include "nflick-api-response.h"
#include "nflick-types.h"

struct                          _NFlickWorker
{
        GObject Parent;
        NFlickWorkerPrivate *Private;
};

struct                          _NFlickWorkerClass 
{
        GObjectClass ParentClass;
        NFlickWorkerThreadFunc ThreadFunc;
};

GType                           nflick_worker_get_type (void);

void                            nflick_worker_start (NFlickWorker *self);

void                            nflick_worker_set_error (NFlickWorker *self, const gchar *error);

void                            nflick_worker_set_custom_data (NFlickWorker *self, gpointer data);

void                            nflick_worker_set_aborted_idle (NFlickWorker *self, NFlickWorkerIdleFunc func);

void                            nflick_worker_set_ok_idle (NFlickWorker *self, NFlickWorkerIdleFunc func);

void                            nflick_worker_set_error_idle (NFlickWorker *self, NFlickWorkerIdleFunc func);

void                            nflick_worker_set_msg_change_idle (NFlickWorker *self, NFlickWorkerIdleFunc func);

void                            nflick_worker_set_message (NFlickWorker *self, const gchar *msg);

void                            nflick_worker_request_abort (NFlickWorker *self);

gboolean                        nflick_worker_is_aborted (NFlickWorker *self);

void                            nflick_worker_set_network_error (NFlickWorker *self);

gboolean                        nflick_worker_parse_api_response (NFlickWorker *self, NFlickApiResponse *response);

#endif
