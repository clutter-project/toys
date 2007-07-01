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

static GObjectClass*            ParentClass = NULL;

struct                          _NFlickPhotoSetPrivate
{
        gchar *Name;
        gint32 Count;
        gchar *Id;
        gboolean Fetched;
        GList *PhotoDataList;
};

enum
{
        ARG_0,
        ARG_COMBO_TEXT,
        ARG_COUNT,
        ARG_ID,
        ARG_FETCHED,
        ARG_LIST
};

static void                     nflick_photo_set_class_init (NFlickPhotoSetClass *klass);

static void                     nflick_photo_set_init (NFlickPhotoSet *self);

static gboolean                 private_init (NFlickPhotoSet *self, NFlickPhotoSetPrivate *private);

static void                     private_dispose (NFlickPhotoSetPrivate *private);

static void                     nflick_photo_set_dispose (NFlickPhotoSet *self);

static void                     nflick_photo_set_finalize (NFlickPhotoSet *self);

static void                     nflick_photo_set_get_property (NFlickPhotoSet *self, guint propid, 
                                                               GValue *value, GParamSpec *pspec);

