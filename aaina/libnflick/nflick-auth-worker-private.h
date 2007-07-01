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

struct                          _NFlickAuthWorkerPrivate
{
        gchar *MiniToken;
        gchar *UserName;
        gchar *FullName;
        gchar *Token;
        gchar *UserNsid;
};

enum 
{
        ARG_0,
        ARG_USER_NAME,
        ARG_FULL_NAME,
        ARG_TOKEN,
        ARG_USER_NSID
};

static void                     nflick_auth_worker_class_init (NFlickAuthWorkerClass *klass);

static void                     nflick_auth_worker_init (NFlickAuthWorker *self);

static gboolean                 private_init (NFlickAuthWorker *self, NFlickAuthWorkerPrivate *private);

static void                     private_dispose (NFlickAuthWorkerPrivate *private);

static void                     nflick_auth_worker_dispose (NFlickAuthWorker *self);

static void                     nflick_auth_worker_finalize (NFlickAuthWorker *self);

static NFlickWorkerStatus       thread_func (NFlickAuthWorker *self);

static void                     nflick_auth_worker_get_property (NFlickAuthWorker *self, guint propid, 
                                                                 GValue *value, GParamSpec *pspec);

