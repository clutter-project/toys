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

struct                          _NFlickPhotoListResponsePrivate
{
        GList *PhotoDataList;
};

enum 
{
        ARG_0,
};

static void                     nflick_photo_list_response_class_init (NFlickPhotoListResponseClass *klass);

static void                     nflick_photo_list_response_init (NFlickPhotoListResponse *self);

static gboolean                 private_init (NFlickPhotoListResponse *self, NFlickPhotoListResponsePrivate *private);

static void                     private_dispose (NFlickPhotoListResponsePrivate *private);

static void                     nflick_photo_list_response_dispose (NFlickPhotoListResponse *self);

static void                     nflick_photo_list_response_finalize (NFlickPhotoListResponse *self);

static void                     parse_func (NFlickPhotoListResponse *self, xmlDoc *doc, xmlNode *children, gboolean *result, gboolean *parse_error);

static void                     nflick_photo_list_response_get_property (NFlickPhotoListResponse *self, guint propid, 
                                                                         GValue *value, GParamSpec *pspec);
