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

struct                          _NFlickInfoWorkerPrivate
{
        gchar *PhotoId;
        gchar *Token;
        gint32 Width;
        gint32 Height;
        GdkPixbuf *Pixbuf;

        char *rotation;
        char *realname;
        char *desc;
};

enum 
{
        ARG_0,
        ARG_PIXBUF,
        ARG_ROTATION,
        ARG_REALNAME,
        ARG_DESC
};

static void                     nflick_info_worker_class_init (NFlickInfoWorkerClass *klass);

static void                     nflick_info_worker_init (NFlickInfoWorker *self);

static gboolean                 private_init (NFlickInfoWorker *self, NFlickInfoWorkerPrivate *private);

static void                     private_dispose (NFlickInfoWorkerPrivate *private);

static void                     nflick_info_worker_dispose (NFlickInfoWorker *self);

static void                     nflick_info_worker_finalize (NFlickInfoWorker *self);

static NFlickWorkerStatus       thread_func (NFlickInfoWorker *self);

static void                     nflick_info_worker_get_property (NFlickInfoWorker *self, guint propid, 
                                                                GValue *value, GParamSpec *pspec);

