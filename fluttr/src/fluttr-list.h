/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Listor: Neil J. Patel  <njp@o-`hand.com>
 */

#include <config.h>
#include <glib.h>
#include <clutter/clutter.h>

#include "nflick-worker.h"

#ifndef _HAVE_FLUTTR_LIST_H
#define _HAVE_FLUTTR_LIST_H


G_BEGIN_DECLS

#define FLUTTR_TYPE_LIST fluttr_list_get_type()

#define FLUTTR_LIST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	FLUTTR_TYPE_LIST, \
	FluttrList))

#define FLUTTR_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	FLUTTR_TYPE_LIST, \
	FluttrListClass))

#define FLUTTR_IS_LIST(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	FLUTTR_TYPE_LIST))

#define FLUTTR_IS_LIST_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
	FLUTTR_TYPE_LIST))

#define FLUTTR_LIST_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	FLUTTR_TYPE_LIST, \
	FluttrListClass))

typedef struct _FluttrList FluttrList;
typedef struct _FluttrListClass FluttrListClass;
typedef struct _FluttrListPrivate FluttrListPrivate;

struct _FluttrList
{
	ClutterGroup         parent;
	
	/* private */
	FluttrListPrivate   *priv;
};

struct _FluttrListClass 
{
	/*< private >*/
	ClutterGroupClass parent_class;

	void (*successful) (FluttrList *list, NFlickWorker *worker);
	void (*error) (FluttrList *list, gchar *msg);
	void (*_fluttr_list_3) (void);
	void (*_fluttr_list_4) (void);
}; 

GType fluttr_list_get_type (void) G_GNUC_CONST;

ClutterActor* 
fluttr_list_new (void);

void
fluttr_list_go (FluttrList *list);

G_END_DECLS

#endif
