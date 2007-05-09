/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-`hand.com>
 */

#include <config.h>
#include <glib.h>
#include <clutter/clutter.h>

#include "fluttr-library.h"
#include "fluttr-photo.h"
#include "fluttr-set.h"

#include <libnflick/nflick.h>

#ifndef _HAVE_FLUTTR_LIST_VIEW_H
#define _HAVE_FLUTTR_LIST_VIEW_H


G_BEGIN_DECLS

#define FLUTTR_TYPE_LIST_VIEW fluttr_list_view_get_type()

#define FLUTTR_LIST_VIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	FLUTTR_TYPE_LIST_VIEW, \
	FluttrListView))

#define FLUTTR_LIST_VIEWCLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	FLUTTR_TYPE_LIST_VIEW, \
	FluttrListViewClass))

#define FLUTTR_IS_LIST_VIEW(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	FLUTTR_TYPE_LIST_VIEW))

#define FLUTTR_IS_LIST_VIEW_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
	FLUTTR_TYPE_LIST_VIEW))

#define FLUTTR_LIST_VIEW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	FLUTTR_TYPE_LIST_VIEW, \
	FluttrListViewClass))

typedef struct _FluttrListView FluttrListView;
typedef struct _FluttrListViewClass FluttrListViewClass;
typedef struct _FluttrListViewPrivate FluttrListViewPrivate;

struct _FluttrListView
{
	ClutterGroup         parent;
	
	/* private */
	FluttrListViewPrivate   *priv;
};

struct _FluttrListViewClass 
{
	/*< private >*/
	ClutterGroupClass parent_class;

	void (*_fluttr_list_view_1) (void);
	void (*_fluttr_list_view_2) (void);
	void (*_fluttr_list_view_3) (void);
	void (*_fluttr_list_view_4) (void);
}; 

GType fluttr_list_view_get_type (void) G_GNUC_CONST;

ClutterActor* 
fluttr_list_view_new ();

FluttrPhoto*
fluttr_list_view_get_active (FluttrListView *list_view);

void
fluttr_list_view_activate (FluttrListView *list_view);

void 
fluttr_list_view_advance (FluttrListView *list_view, gint n);

void
fluttr_list_view_advance_row (FluttrListView *list_view, gint n);

void
fluttr_list_view_advance_col (FluttrListView *list_view, gint n);


G_END_DECLS

#endif
