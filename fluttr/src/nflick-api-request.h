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

#ifndef __NFLICKAPIREQUEST_H__
#define __NFLICKAPIREQUEST_H__
 
#include <gtk/gtk.h>
#include <libintl.h>
#include <ne_uri.h>
#include <ne_session.h>
#include <ne_basic.h>
#include <ne_utils.h>
#include <ne_md5.h>
#include <string.h>
#include "nflick-flickr.h"
#include "nflick-types.h"

struct                          _NFlickApiRequest
{
        GObject Parent;
        NFlickApiRequestPrivate *Private;
};

struct                          _NFlickApiRequestClass 
{
        GObjectClass ParentClass;
};

GType                           nflick_api_request_get_type (void);

NFlickApiRequest*               nflick_api_request_new (const gchar *method);

void                            nflick_api_request_add_parameter (NFlickApiRequest *self, 
                                                                  const gchar *param, const gchar *val);

gboolean                        nflick_api_request_exec (NFlickApiRequest *self);

gboolean                        nflick_api_request_sign (NFlickApiRequest *self);

gchar*                          nflick_api_request_take_buffer (NFlickApiRequest *self);

#endif
