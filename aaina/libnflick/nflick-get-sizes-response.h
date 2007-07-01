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

#ifndef __NFLICKGETSIZESRESPONSE_H__
#define __NFLICKGETSIZESRESPONSE_H__
 
#include <gtk/gtk.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libintl.h>
#include <string.h>
#include "nflick-api-response.h"
#include "nflick-flickr.h"
#include "nflick-types.h"

struct                          _NFlickGetSizesResponse
{
        NFlickApiResponse Parent;
        NFlickGetSizesResponsePrivate *Private;
};

struct                          _NFlickGetSizesResponseClass 
{
        NFlickApiResponseClass ParentClass;
};

GType                           nflick_get_sizes_response_get_type (void);

gchar*                          nflick_get_sizes_response_find_match (NFlickGetSizesResponse *self, gint32 *width, gint32 *height, gboolean *rotated);

gint32                          nflick_get_sizes_response_height_for (gint32 width, gint32 height, gint32 fit_width);

gint32                          nflick_get_sizes_response_width_for (gint32 width, gint32 height, gint32 fit_height);

#endif
