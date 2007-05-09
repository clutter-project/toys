/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-hand.com>
 */

#include "fluttr-set-view.h"

G_DEFINE_TYPE (FluttrSetView, fluttr_set_view, CLUTTER_TYPE_GROUP);

#define FLUTTR_SET_VIEW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
	FLUTTR_TYPE_SET_VIEW, \
	FluttrSetViewPrivate))
	
struct _FluttrSetViewPrivate
{
	GList			*sets;
	gint 			 active_set;
	ClutterActor		*active_actor;
	gint			 active_col;
};

enum
{
	PROP_0,
	PROP_LIBRARY
};

#define N_COLS 3

static ClutterGroupClass	*parent_class = NULL;

FluttrSet*
fluttr_set_view_get_active (FluttrSetView *set_view)
{
	FluttrSetViewPrivate *priv;
		
	g_return_val_if_fail (FLUTTR_IS_SET_VIEW (set_view), NULL);
	priv = FLUTTR_SET_VIEW_GET_PRIVATE(set_view);
	
	return FLUTTR_SET (priv->active_actor);
}

void 
fluttr_set_view_advance (FluttrSetView *set_view, gint n)
{
	FluttrSetViewPrivate *priv;
	gint len;
	gint i = 0;
	ClutterActor *set = NULL;
	guint width = fluttr_set_get_default_width ();
	guint height = fluttr_set_get_default_height ();
	gint x1;
	gint active_row = 0;
	gint offset = height/2;
	gint padding = width/2;
		
	g_return_if_fail (FLUTTR_IS_SET_VIEW (set_view));
	priv = FLUTTR_SET_VIEW_GET_PRIVATE(set_view);

	len = clutter_group_get_n_children (CLUTTER_GROUP (set_view));
	
	/* Make sure we are within the bounds of the number of albums */
	priv->active_set+= n;
	if (priv->active_set < 0) {
		priv->active_set  = 0;
	} else if (priv->active_set > len-1) {
		priv->active_set = len -1;
	} else
		;
	/* Find the magic row */	
	active_row = 0;
	gint row = 0;
	gint col = 0;
	
	for (i = 0; i < len; i++) {
		if (i == priv->active_set) {
			active_row = row;
			break;
		}
		col++;
		if (col > (N_COLS-1)) {
			col = 0;
			row++;
		}
	}
	
	/* Figure out the base x value */
	x1 = ((width) * N_COLS ) + (padding*(N_COLS-1));
	x1 = (CLUTTER_STAGE_WIDTH ()/2)-(x1/2);
	
	/* Iterate through actors, calculating their new x positions, and make
	   sure they are on the right place (left, right or center) */
	col = 0;
	row = 0;

	offset = -1 * ((height) + padding) * active_row;
	offset += (CLUTTER_STAGE_HEIGHT () /2) - (height/2);
	
	for (i = 0; i < len; i++) {
		set = clutter_group_get_nth_child (CLUTTER_GROUP (set_view), i);
		 
		gint x = x1 + (col * (width + padding));
		gint y = offset;
		fluttr_set_update_position (FLUTTR_SET (set), x, y);
		
		col++;
		if (col > (N_COLS-1)) {
			col = 0;
			row++;
			offset += height + padding;
		}
		if (i == priv->active_set) {
			priv->active_actor = set;	
			fluttr_set_set_active (FLUTTR_SET (set), TRUE);
		} else
			fluttr_set_set_active (FLUTTR_SET (set), FALSE);	
			
		/* Update the position of the ring */
	}
}

/* We make all the 'viewable' sets fall down, leaving just the main one */
void
fluttr_set_view_activate (FluttrSetView *set_view)
{
	;
}

void
fluttr_set_view_advance_row (FluttrSetView *set_view, gint n)
{
	fluttr_set_view_advance (set_view, (N_COLS * n));
}

void
fluttr_set_view_advance_col (FluttrSetView *set_view, gint n)
{
	fluttr_set_view_advance (set_view, n);
}

void
fluttr_set_view_add_set (FluttrSetView *set_view, FluttrSet *set)
{
	gint x = CLUTTER_STAGE_WIDTH () /2;
	gint y = CLUTTER_STAGE_HEIGHT ()/2;
	g_return_if_fail (FLUTTR_IS_SET_VIEW (set_view));
	
	
	clutter_group_add (CLUTTER_GROUP (set_view), CLUTTER_ACTOR (set));
	clutter_actor_set_position (CLUTTER_ACTOR (set), x, y);
	clutter_actor_show_all (CLUTTER_ACTOR (set));
}

/* GObject Stuff */

static void
fluttr_set_view_set_property (GObject      *object, 
			  guint         prop_id,
			  const GValue *value, 
			  GParamSpec   *pspec)
{
	FluttrSetViewPrivate *priv;

	g_return_if_fail (FLUTTR_IS_SET_VIEW (object));
	priv = FLUTTR_SET_VIEW_GET_PRIVATE(object);

	switch (prop_id) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, 
							   pspec);
			break;
	}
}

static void
fluttr_set_view_get_property (GObject    *object, 
			  guint       prop_id,
			  GValue     *value, 
			  GParamSpec *pspec)
{
	FluttrSetViewPrivate *priv;
	
	g_return_if_fail (FLUTTR_IS_SET_VIEW (object));
	priv = FLUTTR_SET_VIEW_GET_PRIVATE(object);

	switch (prop_id) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id,
							   pspec);
		break;
	} 
}

static void
fluttr_set_view_paint (ClutterActor *actor)
{
	FluttrSetView        *set;
	FluttrSetViewPrivate *priv;

	set = FLUTTR_SET_VIEW(actor);

	priv = FLUTTR_SET_VIEW_GET_PRIVATE(set);

	glPushMatrix();
	
	gint i;
	gint len = clutter_group_get_n_children (CLUTTER_GROUP (actor)); 
	for (i = 0; i < len; i++) {
		ClutterActor* child;

		child = clutter_group_get_nth_child (CLUTTER_GROUP(actor), i);
		if (child) {
			clutter_actor_paint (child);
		}
	}

	glPopMatrix();
}

static void 
fluttr_set_view_dispose (GObject *object)
{
	FluttrSetView         *self = FLUTTR_SET_VIEW(object);
	FluttrSetViewPrivate  *priv;  

	priv = self->priv;
  
	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void 
fluttr_set_view_finalize (GObject *object)
{
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
fluttr_set_view_class_init (FluttrSetViewClass *klass)
{
	GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
	ClutterActorClass   *actor_class = CLUTTER_ACTOR_CLASS (klass);
	
	parent_class = CLUTTER_GROUP_CLASS (klass);

	actor_class->paint           = fluttr_set_view_paint;
	
	gobject_class->finalize     = fluttr_set_view_finalize;
	gobject_class->dispose      = fluttr_set_view_dispose;
	gobject_class->get_property = fluttr_set_view_get_property;
	gobject_class->set_property = fluttr_set_view_set_property;	

	g_type_class_add_private (gobject_class, sizeof (FluttrSetViewPrivate));
		
}

static void
fluttr_set_view_init (FluttrSetView *self)
{
	FluttrSetViewPrivate *priv;
	priv = FLUTTR_SET_VIEW_GET_PRIVATE (self);
	
	priv->active_set = 0;
	priv->active_col = 0;
	
}

ClutterActor*
fluttr_set_view_new (void)
{
	ClutterGroup         *set_view;

	set_view = g_object_new (FLUTTR_TYPE_SET_VIEW, 
			          NULL);

	return CLUTTER_ACTOR (set_view);
}

