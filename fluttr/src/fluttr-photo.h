/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-hand.com>
 */


#include <config.h>
#include <glib.h>
#include <clutter/clutter.h>

#include <libnflick/nflick.h>

#ifndef _HAVE_FLUTTR_PHOTO_H
#define _HAVE_FLUTTR_PHOTO_H

G_BEGIN_DECLS

#define FLUTTR_TYPE_PHOTO fluttr_photo_get_type()

#define FLUTTR_PHOTO(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	FLUTTR_TYPE_PHOTO, \
	FluttrPhoto))

#define FLUTTR_PHOTO_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	FLUTTR_TYPE_PHOTO, \
	FluttrPhotoClass))

#define FLUTTR_IS_PHOTO(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	FLUTTR_TYPE_PHOTO))

#define FLUTTR_IS_PHOTO_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
	FLUTTR_TYPE_PHOTO))

#define FLUTTR_PHOTO_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	FLUTTR_TYPE_PHOTO, \
	FluttrPhotoClass))

typedef struct _FluttrPhoto FluttrPhoto;
typedef struct _FluttrPhotoClass FluttrPhotoClass;
typedef struct _FluttrPhotoPrivate FluttrPhotoPrivate;

struct _FluttrPhoto
{
	ClutterGroup         parent;
	
	/* private */
	FluttrPhotoPrivate   *priv;
};

struct _FluttrPhotoClass 
{
	/*< private >*/
	ClutterGroupClass parent_class;

	void (*pixbuf_loaded) (FluttrPhoto *photo, gchar *null);
	void (*fetch_error) (FluttrPhoto *photo, gchar *msg);
	void (*_fluttr_photo_3) (void);
	void (*_fluttr_photo_4) (void);
}; 

GType fluttr_photo_get_type (void) G_GNUC_CONST;

ClutterActor* 
fluttr_photo_new (void);

void
fluttr_photo_fetch_pixbuf (FluttrPhoto *photo);

void
fluttr_photo_update_position (FluttrPhoto *photo, gint x, gint y);

guint
fluttr_photo_get_default_size (void);

guint
fluttr_photo_get_default_width (void);

guint
fluttr_photo_get_default_height (void);

void
fluttr_photo_show_options (FluttrPhoto *photo, gboolean show);

void
fluttr_photo_set_active (FluttrPhoto *photo, gboolean active);

G_END_DECLS

#endif
