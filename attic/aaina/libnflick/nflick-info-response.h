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

#ifndef __NFLICKINFORESPONSE_H__
#define __NFLICKINFORESPONSE_H__
 
#include <gtk/gtk.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libintl.h>
#include <string.h>
#include "nflick-api-response.h"
#include "nflick-flickr.h"
#include "nflick-types.h"

struct                          _NFlickInfoResponse
{
        NFlickApiResponse Parent;
        NFlickInfoResponsePrivate *Private;
};

struct                          _NFlickInfoResponseClass 
{
        NFlickApiResponseClass ParentClass;
};

GType                           nflick_info_response_get_type (void);
  
void
nflick_info_response_get (NFlickInfoResponse *self,
                          gchar **rotation,
                          gchar **realname,
                          gchar **desc);

#endif
