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

#ifndef __NFLICKTYPES_H__
#define __NFLICKTYPES_H__

#include <libxml/parser.h>
#include <libxml/tree.h>

/* Window */

typedef struct                  _NFlickWindowClass NFlickWindowClass;

typedef struct                  _NFlickWindow NFlickWindow;

typedef struct                  _NFlickWindowPrivate NFlickWindowPrivate;

#define                         NFLICK_TYPE_WINDOW (nflick_window_get_type ())

#define                         NFLICK_IS_WINDOW(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_WINDOW))

#define                         NFLICK_WINDOW(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_WINDOW, NFlickWindow))

#define                         NFLICK_WINDOW_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_WINDOW, NFlickWindowClass))

/* Wait dialog */

#define                         NFLICK_WAIT_DIALOG_RESPONSE_ABORTED 1000

#define                         NFLICK_WAIT_DIALOG_RESPONSE_ERROR 1001

#define                         NFLICK_WAIT_DIALOG_RESPONSE_OK 1002

typedef struct                  _NFlickWaitDialogClass NFlickWaitDialogClass;

typedef struct                  _NFlickWaitDialog NFlickWaitDialog;

typedef struct                  _NFlickWaitDialogPrivate NFlickWaitDialogPrivate;

#define                         NFLICK_TYPE_WAIT_DIALOG (nflick_wait_dialog_get_type ())

#define                         NFLICK_IS_WAIT_DIALOG(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_WAIT_DIALOG))

#define                         NFLICK_WAIT_DIALOG(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_WAIT_DIALOG, NFlickWaitDialog))

#define                         NFLICK_WAIT_DIALOG_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_WAIT_DIALOG, NFlickWaitDialogClass))

/* Token dialog */

typedef struct                  _NFlickTokenDialogClass NFlickTokenDialogClass;

typedef struct                  _NFlickTokenDialog NFlickTokenDialog;

typedef struct                  _NFlickTokenDialogPrivate NFlickTokenDialogPrivate;

#define                         NFLICK_TYPE_TOKEN_DIALOG (nflick_token_dialog_get_type ())

#define                         NFLICK_IS_TOKEN_DIALOG(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_TOKEN_DIALOG))

#define                         NFLICK_TOKEN_DIALOG(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_TOKEN_DIALOG, NFlickTokenDialog))

#define                         NFLICK_TOKEN_DIALOG_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_TOKEN_DIALOG, NFlickTokenDialogClass))

/* Cache dialog */

typedef struct                  _NFlickCacheDialogClass NFlickCacheDialogClass;

typedef struct                  _NFlickCacheDialog NFlickCacheDialog;

typedef struct                  _NFlickCacheDialogPrivate NFlickCacheDialogPrivate;

#define                         NFLICK_TYPE_CACHE_DIALOG (nflick_cache_dialog_get_type ())

#define                         NFLICK_IS_CACHE_DIALOG(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_CACHE_DIALOG))

#define                         NFLICK_CACHE_DIALOG(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_CACHE_DIALOG, NFlickCacheDialog))

#define                         NFLICK_CACHE_DIALOG_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_CACHE_DIALOG, NFlickCacheDialogClass))

/* Welcome VBox */

typedef struct                  _NFlickWelcomeVBoxClass NFlickWelcomeVBoxClass;

typedef struct                  _NFlickWelcomeVBox NFlickWelcomeVBox;

typedef struct                  _NFlickWelcomeVBoxPrivate NFlickWelcomeVBoxPrivate;

#define                         NFLICK_TYPE_WELCOME_VBOX (nflick_welcome_vbox_get_type ())

#define                         NFLICK_IS_WELCOME_VBOX(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_WELCOME_VBOX))

#define                         NFLICK_WELCOME_VBOX(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_WELCOME_VBOX, NFlickWelcomeVBox)

#define                         NFLICK_WELCOME_VBOX_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_WELCOME_VBOX, NFlickWelcomeVBoxClass))

/* Show VBox */

typedef struct                  _NFlickShowVBoxClass NFlickShowVBoxClass;

typedef struct                  _NFlickShowVBox NFlickShowVBox;

typedef struct                  _NFlickShowVBoxPrivate NFlickShowVBoxPrivate;

#define                         NFLICK_TYPE_SHOW_VBOX (nflick_show_vbox_get_type ())

#define                         NFLICK_IS_SHOW_VBOX(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_SHOW_VBOX))

#define                         NFLICK_SHOW_VBOX(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_SHOW_VBOX, NFlickShowVBox)

#define                         NFLICK_SHOW_VBOX_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_SHOW_VBOX, NFlickShowVBoxClass))

/* Worker */

typedef struct                  _NFlickWorkerClass NFlickWorkerClass;

typedef struct                  _NFlickWorker NFlickWorker;

typedef struct                  _NFlickWorkerPrivate NFlickWorkerPrivate;

#define                         NFLICK_TYPE_WORKER (nflick_worker_get_type ())

#define                         NFLICK_IS_WORKER(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_WORKER))

#define                         NFLICK_WORKER(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_WORKER, NFlickWorker)

#define                         NFLICK_WORKER_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_WORKER, NFlickWorkerClass))

enum                            
{
        NFLICK_WORKER_STATUS_IDLE,
        NFLICK_WORKER_STATUS_OK,
        NFLICK_WORKER_STATUS_ABORTED,
        NFLICK_WORKER_STATUS_RUNNING,
        NFLICK_WORKER_STATUS_ERROR

}                               typedef NFlickWorkerStatus;

typedef                         NFlickWorkerStatus (*NFlickWorkerThreadFunc) (NFlickWorker *self);

typedef                         gboolean (*NFlickWorkerIdleFunc) (NFlickWorker *self);

/* Api request */

typedef struct                  _NFlickApiRequestClass NFlickApiRequestClass;

typedef struct                  _NFlickApiRequest NFlickApiRequest;

typedef struct                  _NFlickApiRequestPrivate NFlickApiRequestPrivate;

#define                         NFLICK_TYPE_API_REQUEST (nflick_api_request_get_type ())

#define                         NFLICK_IS_API_REQUEST(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_API_REQUEST))

#define                         NFLICK_API_REQUEST(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_API_REQUEST, NFlickApiRequest)

#define                         NFLICK_API_REQUEST_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_API_REQUEST, NFlickApiRequestClass))

/* Api response */

typedef struct                  _NFlickApiResponseClass NFlickApiResponseClass;

typedef struct                  _NFlickApiResponse NFlickApiResponse;

typedef struct                  _NFlickApiResponsePrivate NFlickApiResponsePrivate;

#define                         NFLICK_TYPE_API_RESPONSE (nflick_api_response_get_type ())

#define                         NFLICK_IS_API_RESPONSE(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_API_RESPONSE))

#define                         NFLICK_API_RESPONSE(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_API_RESPONSE, NFlickApiResponse)

typedef                         void (*NFlickApiRequestParseFunc) \
                                (NFlickApiResponse *self, xmlDoc *doc, xmlNode *children, gboolean *result, gboolean *parse_error);

#define                         NFLICK_API_RESPONSE_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_API_RESPONSE, NFlickApiResponseClass))

/* Gft response */

typedef struct                  _NFlickGftResponseClass NFlickGftResponseClass;

typedef struct                  _NFlickGftResponse NFlickGftResponse;

typedef struct                  _NFlickGftResponsePrivate NFlickGftResponsePrivate;

#define                         NFLICK_TYPE_GFT_RESPONSE (nflick_gft_response_get_type ())

#define                         NFLICK_IS_GFT_RESPONSE(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_GFT_RESPONSE))

#define                         NFLICK_GFT_RESPONSE(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_GFT_RESPONSE, NFlickGftResponse)

#define                         NFLICK_GFT_RESPONSE_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_GFT_RESPONSE, NFlickGftResponseClass))

/* Photo set */

typedef struct                  _NFlickPhotoSetClass NFlickPhotoSetClass;

typedef struct                  _NFlickPhotoSet NFlickPhotoSet;

typedef struct                  _NFlickPhotoSetPrivate NFlickPhotoSetPrivate;

#define                         NFLICK_TYPE_PHOTO_SET (nflick_photo_set_get_type ())

#define                         NFLICK_IS_PHOTO_SET(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_PHOTO_SET))

#define                         NFLICK_PHOTO_SET(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_PHOTO_SET, NFlickPhotoSet)

#define                         NFLICK_PHOTO_SET_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_PHOTO_SET, NFlickPhotoSetClass))

/* Thmb table */

typedef struct                  _NFlickThmbTableClass NFlickThmbTableClass;

typedef struct                  _NFlickThmbTable NFlickThmbTable;

typedef struct                  _NFlickThmbTablePrivate NFlickThmbTablePrivate;

#define                         NFLICK_TYPE_THMB_TABLE (nflick_thmb_table_get_type ())

#define                         NFLICK_IS_THMB_TABLE(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_THMB_TABLE))

#define                         NFLICK_THMB_TABLE(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_THMB_TABLE, NFlickThmbTable)

#define                         NFLICK_THMB_TABLE_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_THMB_TABLE, NFlickThmbTableClass))

/* Thmb image */

typedef struct                  _NFlickThmbImageClass NFlickThmbImageClass;

typedef struct                  _NFlickThmbImage NFlickThmbImage;

typedef struct                  _NFlickThmbImagePrivate NFlickThmbImagePrivate;

#define                         NFLICK_TYPE_THMB_IMAGE (nflick_thmb_image_get_type ())

#define                         NFLICK_IS_THMB_IMAGE(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_THMB_IMAGE))

#define                         NFLICK_THMB_IMAGE(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_THMB_IMAGE, NFlickThmbImage)

#define                         NFLICK_THMB_IMAGE_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_THMB_IMAGE, NFlickThmbImageClass))

/* Set list response */

typedef struct                  _NFlickSetListResponseClass NFlickSetListResponseClass;

typedef struct                  _NFlickSetListResponse NFlickSetListResponse;

typedef struct                  _NFlickSetListResponsePrivate NFlickSetListResponsePrivate;

#define                         NFLICK_TYPE_SET_LIST_RESPONSE (nflick_set_list_response_get_type ())

#define                         NFLICK_IS_SET_LIST_RESPONSE(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_SET_LIST_RESPONSE))

#define                         NFLICK_SET_LIST_RESPONSE(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_SET_LIST_RESPONSE, NFlickSetListResponse)

#define                         NFLICK_SET_LIST_RESPONSE_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_SET_LIST_RESPONSE, NFlickSetListResponseClass))

/* Photo list response */

typedef struct                  _NFlickPhotoListResponseClass NFlickPhotoListResponseClass;

typedef struct                  _NFlickPhotoListResponse NFlickPhotoListResponse;

typedef struct                  _NFlickPhotoListResponsePrivate NFlickPhotoListResponsePrivate;

#define                         NFLICK_TYPE_PHOTO_LIST_RESPONSE (nflick_photo_list_response_get_type ())

#define                         NFLICK_IS_PHOTO_LIST_RESPONSE(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_PHOTO_LIST_RESPONSE))

#define                         NFLICK_PHOTO_LIST_RESPONSE(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_PHOTO_LIST_RESPONSE, NFlickPhotoListResponse)

#define                         NFLICK_PHOTO_LIST_RESPONSE_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_PHOTO_LIST_RESPONSE, NFlickPhotoListResponseClass))

/* No set response */

typedef struct                  _NFlickNoSetResponseClass NFlickNoSetResponseClass;

typedef struct                  _NFlickNoSetResponse NFlickNoSetResponse;

typedef struct                  _NFlickNoSetResponsePrivate NFlickNoSetResponsePrivate;

#define                         NFLICK_TYPE_NO_SET_RESPONSE (nflick_no_set_response_get_type ())

#define                         NFLICK_IS_NO_SET_RESPONSE(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_NO_SET_RESPONSE))

#define                         NFLICK_NO_SET_RESPONSE(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_NO_SET_RESPONSE, NFlickNoSetResponse)

#define                         NFLICK_NO_SET_RESPONSE_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_NO_SET_RESPONSE, NFlickNoSetResponseClass))

/* GetSizes response */

typedef struct                  _NFlickGetSizesResponseClass NFlickGetSizesResponseClass;

typedef struct                  _NFlickGetSizesResponse NFlickGetSizesResponse;

typedef struct                  _NFlickGetSizesResponsePrivate NFlickGetSizesResponsePrivate;

#define                         NFLICK_TYPE_GET_SIZES_RESPONSE (nflick_get_sizes_response_get_type ())

#define                         NFLICK_IS_GET_SIZES_RESPONSE(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_GET_SIZES_RESPONSE))

#define                         NFLICK_GET_SIZES_RESPONSE(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_GET_SIZES_RESPONSE, NFlickGetSizesResponse)

#define                         NFLICK_GET_SIZES_RESPONSE_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_GET_SIZES_RESPONSE, NFlickGetSizesResponseClass))

/* Auth worker */

typedef struct                  _NFlickAuthWorkerClass NFlickAuthWorkerClass;

typedef struct                  _NFlickAuthWorker NFlickAuthWorker;

typedef struct                  _NFlickAuthWorkerPrivate NFlickAuthWorkerPrivate;

#define                         NFLICK_TYPE_AUTH_WORKER (nflick_auth_worker_get_type ())

#define                         NFLICK_IS_AUTH_WORKER(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_AUTH_WORKER))

#define                         NFLICK_AUTH_WORKER(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_AUTH_WORKER, NFlickAuthWorker)

#define                         NFLICK_AUTH_WORKER_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_AUTH_WORKER, NFlickAuthWorkerClass))

/* Show worker */

typedef struct                  _NFlickShowWorkerClass NFlickShowWorkerClass;

typedef struct                  _NFlickShowWorker NFlickShowWorker;

typedef struct                  _NFlickShowWorkerPrivate NFlickShowWorkerPrivate;

#define                         NFLICK_TYPE_SHOW_WORKER (nflick_show_worker_get_type ())

#define                         NFLICK_IS_SHOW_WORKER(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_SHOW_WORKER))

#define                         NFLICK_SHOW_WORKER(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_SHOW_WORKER, NFlickShowWorker)

#define                         NFLICK_SHOW_WORKER_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_SHOW_WORKER, NFlickShowWorkerClass))

/* Set worker */

typedef struct                  _NFlickSetListWorkerClass NFlickSetListWorkerClass;

typedef struct                  _NFlickSetListWorker NFlickSetListWorker;

typedef struct                  _NFlickSetListWorkerPrivate NFlickSetListWorkerPrivate;

#define                         NFLICK_TYPE_SET_LIST_WORKER (nflick_set_list_worker_get_type ())

#define                         NFLICK_IS_SET_LIST_WORKER(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_SET_LIST_WORKER))

#define                         NFLICK_SET_LIST_WORKER(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_SET_LIST_WORKER, NFlickSetListWorker)

#define                         NFLICK_SET_LIST_WORKER_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_SET_LIST_WORKER, NFlickSetListWorkerClass))

/* Photo list worker */

typedef struct                  _NFlickPhotoListWorkerClass NFlickPhotoListWorkerClass;

typedef struct                  _NFlickPhotoListWorker NFlickPhotoListWorker;

typedef struct                  _NFlickPhotoListWorkerPrivate NFlickPhotoListWorkerPrivate;

#define                         NFLICK_TYPE_PHOTO_LIST_WORKER (nflick_photo_list_worker_get_type ())

#define                         NFLICK_IS_PHOTO_LIST_WORKER(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_PHOTO_LIST_WORKER))

#define                         NFLICK_PHOTO_LIST_WORKER(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_PHOTO_LIST_WORKER, NFlickPhotoListWorker)

#define                         NFLICK_PHOTO_LIST_WORKER_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_PHOTO_LIST_WORKER, NFlickPhotoListWorkerClass))

/* Photos vbox */

typedef struct                  _NFlickPhotosVBoxClass NFlickPhotosVBoxClass;

typedef struct                  _NFlickPhotosVBox NFlickPhotosVBox;

typedef struct                  _NFlickPhotosVBoxPrivate NFlickPhotosVBoxPrivate;

#define                         NFLICK_TYPE_PHOTOS_VBOX (nflick_photos_vbox_get_type ())

#define                         NFLICK_IS_PHOTOS_VBOX(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_PHOTOS_VBOX))

#define                         NFLICK_PHOTOS_VBOX(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_PHOTOS_VBOX, NFlickPhotosVBox)

#define                         NFLICK_PHOTOS_VBOX_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_PHOTOS_VBOX, NFlickPhotosVBoxClass))

/* Set Combo */

typedef struct                  _NFlickSetComboClass NFlickSetComboClass;

typedef struct                  _NFlickSetCombo NFlickSetCombo;

typedef struct                  _NFlickSetComboPrivate NFlickSetComboPrivate;

#define                         NFLICK_TYPE_SET_COMBO (nflick_set_combo_get_type ())

#define                         NFLICK_IS_SET_COMBO(obj) \
                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NFLICK_TYPE_SET_COMBO))

#define                         NFLICK_SET_COMBO(obj) \
                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NFLICK_TYPE_SET_COMBO, NFlickSetCombo)

#define                         NFLICK_SET_COMBO_GET_CLASS(obj) \
                                (G_TYPE_INSTANCE_GET_CLASS ((obj), NFLICK_TYPE_SET_COMBO, NFlickSetComboClass))

/* Processor */

typedef                         void (*NFlickProcessorFreeFunc) (gpointer data);

typedef                         gboolean (*NFlickProcessorJobFunc) (gpointer data, gchar **error);

typedef                         gboolean (*NFlickProcessorErrorFunc) (gchar *msg);

typedef                         gboolean (*NFlickProcessorDoneFunc) (gpointer data);

typedef struct                  _NFlickProcessorResult NFlickProcessorResult;

/* Model */

typedef struct                  _NFlickModel NFlickModel;

/* Photo data */

typedef struct                  _NFlickPhotoData NFlickPhotoData;

#define                         NFLICK_TYPE_PHOTO_DATA (nflick_photo_data_get_type ())

/* End */

#endif
