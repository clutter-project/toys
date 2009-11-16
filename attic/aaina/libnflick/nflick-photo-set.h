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

#ifndef __NFLICKPHOTOSET_H__
#define __NFLICKPHOTOSET_H__
 
#include <gtk/gtk.h>
#include <libintl.h>
#include <string.h>
#include "nflick-flickr.h"
#include "nflick-types.h"
#include "nflick-photo-data.h"

struct                          _NFlickPhotoSet
{
        GObject Parent;
        NFlickPhotoSetPrivate *Private;
};

struct                          _NFlickPhotoSetClass 
{
        GObjectClass ParentClass;
};

GType                           nflick_photo_set_get_type (void);

NFlickPhotoSet*                 nflick_photo_set_new (const gchar *name, const gchar *id, gint32 count);

void                            nflick_photo_set_give_list (NFlickPhotoSet *self, GList *list);

NFlickPhotoSet*                 nflick_photo_set_new_no_set (gint32 count);

#endif
