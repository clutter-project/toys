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

#ifndef __NFLICKAPIRESPONSE_H__
#define __NFLICKAPIRESPONSE_H__
 
#include <gtk/gtk.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libintl.h>
#include <string.h>
#include "nflick-flickr.h"
#include "nflick-types.h"
#include "nflick-api-request.h"

struct                          _NFlickApiResponse
{
        GObject Parent;
        NFlickApiResponsePrivate *Private;
};

struct                          _NFlickApiResponseClass 
{
        GObjectClass ParentClass;
        NFlickApiRequestParseFunc ParseFunc;
};

GType                           nflick_api_response_get_type (void);

void                            nflick_api_response_set_error (NFlickApiResponse *self, const gchar *error);

void                            nflick_api_response_add_error (NFlickApiResponse *self, const gchar *error);

gboolean                        nflick_api_response_parse (NFlickApiResponse *self, const gchar *xml);

NFlickApiResponse*              nflick_api_response_new_from_request (GType type, NFlickApiRequest *request);

#endif
