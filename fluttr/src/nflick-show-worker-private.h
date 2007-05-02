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

static NFlickWorker*            ParentClass = NULL;

struct                          _NFlickShowWorkerPrivate
{
        gchar *PhotoId;
        gchar *Token;
        gint32 Width;
        gint32 Height;
        GdkPixbuf *Pixbuf;
};

enum 
{
        ARG_0,
        ARG_PIXBUF,
};

static void                     nflick_show_worker_class_init (NFlickShowWorkerClass *klass);

static void                     nflick_show_worker_init (NFlickShowWorker *self);

static gboolean                 private_init (NFlickShowWorker *self, NFlickShowWorkerPrivate *private);

static void                     private_dispose (NFlickShowWorkerPrivate *private);

static void                     nflick_show_worker_dispose (NFlickShowWorker *self);

static void                     nflick_show_worker_finalize (NFlickShowWorker *self);

static NFlickWorkerStatus       thread_func (NFlickShowWorker *self);

static void                     nflick_show_worker_get_property (NFlickShowWorker *self, guint propid, 
                                                                GValue *value, GParamSpec *pspec);

