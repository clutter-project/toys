/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-hand.com>
 */


/* This is a utility ClutterBehaviour-derived class, in which you can set the
   alphanotify function. It is useful for situations where you do not need the
   full capabilities of the ClutterBehvaiour class, you just want a function to
   be called for each iteration along the timeline
*/

#include "fluttr-behave.h"

#include "math.h"

G_DEFINE_TYPE (FluttrBehave, fluttr_behave, CLUTTER_TYPE_BEHAVIOUR);

#define FLUTTR_BEHAVE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
	FLUTTR_TYPE_BEHAVE, \
	FluttrBehavePrivate))

struct _FluttrBehavePrivate
{
	FluttrBehaveAlphaFunc     func;
	gpointer		data;
};

guint32 
alpha_sine_inc_func (ClutterAlpha *alpha,
		     gpointer      dummy)
{
	ClutterTimeline *timeline;
	gint current_frame_num, n_frames;
	gdouble x, sine;
  
	timeline = clutter_alpha_get_timeline (alpha);

	current_frame_num = clutter_timeline_get_current_frame (timeline);
	n_frames = clutter_timeline_get_n_frames (timeline);

	x = (gdouble) (current_frame_num * 0.5f * M_PI) / n_frames ;
	/* sine = (sin (x - (M_PI / 0.5f)) + 1.0f) * 0.5f; */
  
	sine = (sin (x - (M_PI / 0.5f))) ;
  
	return (guint32)(sine * (gdouble) CLUTTER_ALPHA_MAX_ALPHA);
}

guint32 
alpha_linear_inc_func (ClutterAlpha *alpha,
		       gpointer      dummy)
{
	ClutterTimeline *timeline;
	gint current_frame_num, n_frames;
	gdouble x;
  
	timeline = clutter_alpha_get_timeline (alpha);

	current_frame_num = clutter_timeline_get_current_frame (timeline);
	n_frames = clutter_timeline_get_n_frames (timeline);

	x = (gdouble) (current_frame_num) / n_frames ;
	/* sine = (sin (x - (M_PI / 0.5f)) + 1.0f) * 0.5f; */
  
	return (guint32)(x * (gdouble) CLUTTER_ALPHA_MAX_ALPHA);      
}

static void
fluttr_behave_alpha_notify (ClutterBehaviour *behave, guint32 alpha_value)
{
	FluttrBehave *fluttr_behave = FLUTTR_BEHAVE(behave);
	FluttrBehavePrivate *priv;
	
	priv = FLUTTR_BEHAVE_GET_PRIVATE (fluttr_behave);
	
	if (priv->func != NULL) {
		priv->func (behave, alpha_value, priv->data);
	}
}

static void
fluttr_behave_class_init (FluttrBehaveClass *klass)
{
	GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
	ClutterBehaviourClass *behave_class = CLUTTER_BEHAVIOUR_CLASS (klass);

	behave_class->alpha_notify = fluttr_behave_alpha_notify;
	
	g_type_class_add_private (gobject_class, sizeof (FluttrBehavePrivate));
}

static void
fluttr_behave_init (FluttrBehave *self)
{
	FluttrBehavePrivate *priv;
	
	priv = FLUTTR_BEHAVE_GET_PRIVATE (self);
	
	priv->func = NULL;
	priv->data = NULL;
}

ClutterBehaviour*
fluttr_behave_new (ClutterAlpha 		*alpha,
		 FluttrBehaveAlphaFunc 	 func,
		 gpointer		 data)
{
	FluttrBehave *behave;
	FluttrBehavePrivate *priv;
	
	behave = g_object_new (FLUTTR_TYPE_BEHAVE, 
			       "alpha", alpha,
			       NULL);

	priv = FLUTTR_BEHAVE_GET_PRIVATE (behave);  
	
	priv->func = func;
	priv->data = data;
		
	return CLUTTER_BEHAVIOUR(behave);
}
