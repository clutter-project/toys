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

static NFlickWorkerClass*       ParentClass = NULL;

struct                          _NFlickSetListWorkerPrivate
{
        gchar *UserNsid;
        gchar *Token;
        GList *PhotoSets;
};

enum 
{
        ARG_0,
};

static void                     nflick_set_list_worker_class_init (NFlickSetListWorkerClass *klass);

static void                     nflick_set_list_worker_init (NFlickSetListWorker *self);

static gboolean                 private_init (NFlickSetListWorker *self, NFlickSetListWorkerPrivate *priv);

static void                     private_dispose (NFlickSetListWorkerPrivate *priv);

static void                     nflick_set_list_worker_dispose (NFlickSetListWorker *self);

static void                     nflick_set_list_worker_finalize (NFlickSetListWorker *self);

static NFlickWorkerStatus       thread_func (NFlickSetListWorker *self);

static void                     nflick_set_list_worker_get_property (NFlickSetListWorker *self, guint propid, 
                                                                     GValue *value, GParamSpec *pspec);

