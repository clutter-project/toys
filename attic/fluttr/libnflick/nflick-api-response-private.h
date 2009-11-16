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

struct                          _NFlickApiResponsePrivate
{
        gchar *Xml;
        gchar *Error;
        gboolean Success;
        gboolean ParseError;
};

enum
{
        ARG_0,
        ARG_ERROR,
        ARG_PARSE_ERROR, 
        ARG_XML, 
        ARG_SUCCESS
};

static void                     nflick_api_response_class_init (NFlickApiResponseClass *klass);

static void                     nflick_api_response_init (NFlickApiResponse *self);

static gboolean                 private_init (NFlickApiResponse *self, NFlickApiResponsePrivate *private);

static void                     private_dispose (NFlickApiResponsePrivate *private);

static void                     nflick_api_response_dispose (NFlickApiResponse *self);

static void                     nflick_api_response_finalize (NFlickApiResponse *self);

static void                     nflick_api_response_get_property (NFlickApiResponse *self, guint propid, 
                                                                  GValue *value, GParamSpec *pspec);


