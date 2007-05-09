/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-`hand.com>
 */

#include <config.h>
#include <glib.h>
#include <clutter/clutter.h>

#include <libnflick/nflick.h>

#include "fluttr-photo.h"

#ifndef _HAVE_FLUTTR_VIEWER_H
#define _HAVE_FLUTTR_VIEWER_H


G_BEGIN_DECLS

#define FLUTTR_TYPE_VIEWER fluttr_viewer_get_type()

#define FLUTTR_VIEWER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	FLUTTR_TYPE_VIEWER, \
	FluttrViewer))

#define FLUTTR_VIEWER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	FLUTTR_TYPE_VIEWER, \
	FluttrViewerClass))

#define FLUTTR_IS_VIEWER(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	FLUTTR_TYPE_VIEWER))

#define FLUTTR_IS_VIEWER_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
	FLUTTR_TYPE_VIEWER))

#define FLUTTR_VIEWER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	FLUTTR_TYPE_VIEWER, \
	FluttrViewerClass))

typedef struct _FluttrViewer FluttrViewer;
typedef struct _FluttrViewerClass FluttrViewerClass;
typedef struct _FluttrViewerPrivate FluttrViewerPrivate;

struct _FluttrViewer
{
	ClutterGroup         parent;
	
	/* private */
	FluttrViewerPrivate   *priv;
};

struct _FluttrViewerClass 
{
	/*< private >*/
	ClutterGroupClass parent_class;

	void (*successful) (FluttrViewer *viewer, NFlickWorker *worker);
	void (*error) (FluttrViewer *viewer, gchar *msg);
	void (*_fluttr_viewer_3) (void);
	void (*_fluttr_viewer_4) (void);
}; 

GType fluttr_viewer_get_type (void) G_GNUC_CONST;

ClutterActor* 
fluttr_viewer_new (void);

void
fluttr_viewer_go (FluttrViewer *viewer, FluttrPhoto *photo);

void
fluttr_viewer_show (FluttrViewer *viewer, gboolean show);

G_END_DECLS

#endif
