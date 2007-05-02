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

struct                          _PixbufFetchHelper 
{
        gint32 Width;
        gint32 Height;
        GdkPixbufLoader *Loader;
        FILE *CacheFile;
} typedef PixbufFetchHelper;

static int                      block_reader (PixbufFetchHelper *helper, gchar *buffer, int len);

static void                     on_size_prepared (GdkPixbufLoader *loader, gint width, gint height, PixbufFetchHelper *helper);

static gchar*                   get_cache_file (const gchar *token);

