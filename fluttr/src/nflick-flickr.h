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

#ifndef __NFLICKFLICKR_H__
#define __NFLICKFLICKR_H__

/* Some stock stuff obtained from flickr. That's public, really */

#define                         NFLICK_FLICKR_API_KEY "9dab4c07b676141e059c092e10693e05"

#define                         NFLICK_FLICKR_SHARED_SECRET "8e608a77989c0397"

#define                         NFLICK_FLICKR_HOST "www.flickr.com"

#define                         NFLICK_FLICKR_REST_END_POINT "/services/rest/"

/* Request parameters */

#define                         NFLICK_FLICKR_API_PARAM_KEY "api_key"

#define                         NFLICK_FLICKR_API_PARAM_METHOD "method"

#define                         NFLICK_FLICKR_API_PARAM_MINI_TOKEN "mini_token"

#define                         NFLICK_FLICKR_API_PARAM_TOKEN "auth_token"

#define                         NFLICK_FLICKR_API_PARAM_SIGNATURE "api_sig"

#define                         NFLICK_FLICKR_API_PARAM_USER_ID "user_id"

#define                         NFLICK_FLICKR_API_PARAM_PHOTOSET_ID "photoset_id"

#define                         NFLICK_FLICKR_API_PARAM_PHOTO_ID "photo_id"

#define                         NFLICK_FLICKR_API_PARAM_PER_PAGE "per_page"

/* Possible methods */

#define                         NFLICK_FLICKR_API_METHOD_GET_FULL_TOKEN "flickr.auth.getFullToken"

#define                         NFLICK_FLICKR_API_METHOD_PHOTOSETS_GET_LIST "flickr.photosets.getList"

#define                         NFLICK_FLICKR_API_METHOD_PHOTOSETS_GET_PHOTOS "flickr.photosets.getPhotos"

#define                         NFLICK_FLICKR_API_METHOD_PHOTOS_GET_SIZES "flickr.photos.getSizes"

#define                         NFLICK_FLICKR_API_METHOD_PHOTOS_NOT_IN_SET "flickr.photos.getNotInSet"

#endif
