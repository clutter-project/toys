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

#include "fluttr-set.h"


#ifndef _HAVE_FLUTTR_SET_VIEW_H
#define _HAVE_FLUTTR_SET_VIEW_H


G_BEGIN_DECLS

#define FLUTTR_TYPE_SET_VIEW fluttr_set_view_get_type()

#define FLUTTR_SET_VIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	FLUTTR_TYPE_SET_VIEW, \
	FluttrSetView))

#define FLUTTR_SET_VIEWCLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	FLUTTR_TYPE_SET_VIEW, \
	FluttrSetViewClass))

#define FLUTTR_IS_SET_VIEW(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	FLUTTR_TYPE_SET_VIEW))

#define FLUTTR_IS_SET_VIEW_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
	FLUTTR_TYPE_SET_VIEW))

#define FLUTTR_SET_VIEW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	FLUTTR_TYPE_SET_VIEW, \
	FluttrSetViewClass))

typedef struct _FluttrSetView FluttrSetView;
typedef struct _FluttrSetViewClass FluttrSetViewClass;
typedef struct _FluttrSetViewPrivate FluttrSetViewPrivate;

struct _FluttrSetView
{
	ClutterGroup         parent;
	
	/* private */
	FluttrSetViewPrivate   *priv;
};

struct _FluttrSetViewClass 
{
	/*< private >*/
	ClutterGroupClass parent_class;

	void (*_fluttr_set_view_1) (void);
	void (*_fluttr_set_view_2) (void);
	void (*_fluttr_set_view_3) (void);
	void (*_fluttr_set_view_4) (void);
}; 

GType fluttr_set_view_get_type (void) G_GNUC_CONST;

ClutterActor* 
fluttr_set_view_new (void);

void
fluttr_set_view_add_set (FluttrSetView *set_view, FluttrSet *set);

FluttrSet*
fluttr_set_view_get_active (FluttrSetView *set_view);

void
fluttr_set_view_activate (FluttrSetView *set_view);

void 
fluttr_set_view_advance (FluttrSetView *set_view, gint n);

void
fluttr_set_view_advance_row (FluttrSetView *set_view, gint n);

void
fluttr_set_view_advance_col (FluttrSetView *set_view, gint n);


G_END_DECLS

#endif
