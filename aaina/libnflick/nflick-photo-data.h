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

#ifndef __NFLICKPHOTODATA_H__
#define __NFLICKPHOTODATA_H__

#include <gtk/gtk.h>
#include "nflick-types.h"

struct _NFlickPhotoData
{
        gchar *Id;
        gchar *Name;
};

GType                           nflick_photo_data_get_type (void);

NFlickPhotoData*                nflick_photo_data_copy (const NFlickPhotoData *self);

void                            nflick_photo_data_free (NFlickPhotoData *self);

NFlickPhotoData*                nflick_photo_data_new (const gchar *id, const gchar *name);

#endif
