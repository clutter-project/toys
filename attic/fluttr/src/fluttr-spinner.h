/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-`hand.com>
 */

#include <config.h>
#include <glib.h>
#include <clutter/clutter.h>

#ifndef _HAVE_FLUTTR_SPINNER_H
#define _HAVE_FLUTTR_SPINNER_H


G_BEGIN_DECLS

#define FLUTTR_TYPE_SPINNER fluttr_spinner_get_type()

#define FLUTTR_SPINNER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	FLUTTR_TYPE_SPINNER, \
	FluttrSpinner))

#define FLUTTR_SPINNER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	FLUTTR_TYPE_SPINNER, \
	FluttrSpinnerClass))

#define FLUTTR_IS_SPINNER(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	FLUTTR_TYPE_SPINNER))

#define FLUTTR_IS_SPINNER_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
	FLUTTR_TYPE_SPINNER))

#define FLUTTR_SPINNER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	FLUTTR_TYPE_SPINNER, \
	FluttrSpinnerClass))

typedef struct _FluttrSpinner FluttrSpinner;
typedef struct _FluttrSpinnerClass FluttrSpinnerClass;
typedef struct _FluttrSpinnerPrivate FluttrSpinnerPrivate;

struct _FluttrSpinner
{
	ClutterTexture         parent;
	
	/* private */
	FluttrSpinnerPrivate   *priv;
};

struct _FluttrSpinnerClass 
{
	/*< private >*/
	ClutterTextureClass parent_class;
}; 

GType fluttr_spinner_get_type (void) G_GNUC_CONST;

ClutterActor* 
fluttr_spinner_new (void);

void
fluttr_spinner_spin (FluttrSpinner *spinner, gboolean spin);

G_END_DECLS

#endif
