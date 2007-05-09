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

#ifndef _HAVE_FLUTTR_SET_H
#define _HAVE_FLUTTR_SET_H

G_BEGIN_DECLS

#define FLUTTR_TYPE_SET fluttr_set_get_type()

#define FLUTTR_SET(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	FLUTTR_TYPE_SET, \
	FluttrSet))

#define FLUTTR_SET_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	FLUTTR_TYPE_SET, \
	FluttrSetClass))

#define FLUTTR_IS_SET(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	FLUTTR_TYPE_SET))

#define FLUTTR_IS_SET_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
	FLUTTR_TYPE_SET))

#define FLUTTR_SET_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	FLUTTR_TYPE_SET, \
	FluttrSetClass))

typedef struct _FluttrSet FluttrSet;
typedef struct _FluttrSetClass FluttrSetClass;
typedef struct _FluttrSetPrivate FluttrSetPrivate;

struct _FluttrSet
{
	ClutterGroup         parent;
	
	/* private */
	FluttrSetPrivate   *priv;
};

struct _FluttrSetClass 
{
	/*< private >*/
	ClutterGroupClass parent_class;

	void (*pixbuf_loaded) (FluttrSet *set, gchar *null);
	void (*fetch_error) (FluttrSet *set, gchar *msg);
	void (*_fluttr_set_3) (void);
	void (*_fluttr_set_4) (void);
}; 

typedef struct {
	gchar		*id;
	gchar		*name;
	GdkPixbuf	*pixbuf;

} FluttrPhotoData;

GType fluttr_set_get_type (void) G_GNUC_CONST;

ClutterActor* 
fluttr_set_new (NFlickPhotoSet *photo_set);

void
fluttr_set_append_photo (FluttrSet *set, const gchar *id, const gchar *name);

GList*
fluttr_set_get_photos (FluttrSet *set);

void
fluttr_set_fetch_pixbuf (FluttrSet *set);

void
fluttr_set_update_position (FluttrSet *set, gint x, gint y);

guint
fluttr_set_get_default_size (void);

guint
fluttr_set_get_default_width (void);

guint
fluttr_set_get_default_height (void);

void
fluttr_set_set_options (FluttrSet *set, ClutterActor *options);

void
fluttr_set_set_active (FluttrSet *set, gboolean active);

G_END_DECLS

#endif
