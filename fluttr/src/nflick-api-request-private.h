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

struct                          _NFlickApiRequestPrivate
{
        GHashTable *Hash;
        gchar *Buffer;
        gint32 BytesRead;
};

static void                     nflick_api_request_class_init (NFlickApiRequestClass *klass);

static void                     nflick_api_request_init (NFlickApiRequest *self);

static gboolean                 private_init (NFlickApiRequest *self, NFlickApiRequestPrivate *private);

static void                     private_dispose (NFlickApiRequestPrivate *private);

static void                     nflick_api_request_dispose (NFlickApiRequest *self);

static void                     nflick_api_request_finalize (NFlickApiRequest *self);

static gchar*                   get_path (NFlickApiRequest *self);

static void                     foreach_composer_list (gchar *param, gchar *val, GList **list);

static void                     foreach_composer_str (gchar *val, gchar **str);

static gchar*                   get_path_sig (NFlickApiRequest *self);

static void                     foreach_composer_list_sig (gchar *param, gchar *val, GList **list);

static void                     foreach_composer_str_sig (gchar *val, gchar **str);

static int                      block_reader (NFlickApiRequest *self, gchar *buffer, int len);

