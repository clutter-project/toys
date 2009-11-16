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

static GObject*                 ParentClass = NULL;

struct                          _NFlickWorkerPrivate
{
        GThread *Thread;
        gboolean Started;
        GMutex *Mutex;
        NFlickWorkerStatus Status;
        gchar *Error;
        gchar *Message;

        NFlickWorkerIdleFunc AbortedIdle;
        NFlickWorkerIdleFunc OkIdle;
        NFlickWorkerIdleFunc ErrorIdle;
        NFlickWorkerIdleFunc MsgChangeIdle;
        gpointer CustomData;
        
        gboolean AbortRequested;
};

enum 
{
        ARG_0,
        ARG_ERROR,
        ARG_MESSAGE,
        ARG_STATUS
};

#define                         WORKER_LOCK(obj) (g_mutex_lock (obj->Private->Mutex))

#define                         WORKER_UNLOCK(obj) (g_mutex_unlock (obj->Private->Mutex))

static void                     nflick_worker_class_init (NFlickWorkerClass *klass);

static void                     nflick_worker_init (NFlickWorker *self);

static gboolean                 private_init (NFlickWorker *self, NFlickWorkerPrivate *private);

static void                     private_dispose (NFlickWorkerPrivate *private);

static void                     nflick_worker_dispose (NFlickWorker *self);

static void                     nflick_worker_finalize (NFlickWorker *self);

static void                     thread_start (NFlickWorker *self);

static void                     set_error_no_lock (NFlickWorker *self, const gchar *error);

static void                     nflick_worker_get_property (NFlickWorker *self, guint propid, 
                                                            GValue *value, GParamSpec *pspec);

