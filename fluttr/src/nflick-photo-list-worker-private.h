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

struct                          _NFlickPhotoListWorkerPrivate
{
        gchar *Id;
        gchar *Token;
        GList *PhotoDataList;
};

enum 
{
        ARG_0,
};

static void                     nflick_photo_list_worker_class_init (NFlickPhotoListWorkerClass *klass);

static void                     nflick_photo_list_worker_init (NFlickPhotoListWorker *self);

static gboolean                 private_init (NFlickPhotoListWorker *self, NFlickPhotoListWorkerPrivate *private);

static void                     private_dispose (NFlickPhotoListWorkerPrivate *private);

static void                     nflick_photo_list_worker_dispose (NFlickPhotoListWorker *self);

static void                     nflick_photo_list_worker_finalize (NFlickPhotoListWorker *self);

static NFlickWorkerStatus       thread_func (NFlickPhotoListWorker *self);

static void                     nflick_photo_list_worker_get_property (NFlickPhotoListWorker *self, guint propid, 
                                                                       GValue *value, GParamSpec *pspec);

