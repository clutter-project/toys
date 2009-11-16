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

static NFlickApiResponse*       ParentClass = NULL;

struct                          _NFlickGftResponsePrivate
{
        gchar *UserName;
        gchar *UserNsid;
        gchar *FullName;
        gchar *Token;
};

enum 
{
        ARG_0,
        ARG_USER_NAME,
        ARG_FULL_NAME,
        ARG_TOKEN,
        ARG_USER_NSID
};

static void                     nflick_gft_response_class_init (NFlickGftResponseClass *klass);

static void                     nflick_gft_response_init (NFlickGftResponse *self);

static gboolean                 private_init (NFlickGftResponse *self, NFlickGftResponsePrivate *private);

static void                     private_dispose (NFlickGftResponsePrivate *private);

static void                     nflick_gft_response_dispose (NFlickGftResponse *self);

static void                     nflick_gft_response_finalize (NFlickGftResponse *self);

static void                     parse_func (NFlickGftResponse *self, xmlDoc *doc, xmlNode *children, gboolean *result, gboolean *parse_error);

static gboolean                 all_fields_valid (NFlickGftResponse *self);

static void                     fill_blanks (NFlickGftResponse *self);

static void                     nflick_gft_response_get_property (NFlickGftResponse *self, guint propid, 
                                                                  GValue *value, GParamSpec *pspec);
