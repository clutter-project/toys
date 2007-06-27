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

#include "aaina-behave.h"

#include "math.h"

G_DEFINE_TYPE (AainaBehave, aaina_behave, CLUTTER_TYPE_BEHAVIOUR);

#define AAINA_BEHAVE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
	AAINA_TYPE_BEHAVE, \
	AainaBehavePrivate))

struct _AainaBehavePrivate
{
	AainaBehaveAlphaFunc     func;
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
aaina_behave_alpha_notify (ClutterBehaviour *behave, guint32 alpha_value)
{
	AainaBehave *aaina_behave = AAINA_BEHAVE(behave);
	AainaBehavePrivate *priv;
	
	priv = AAINA_BEHAVE_GET_PRIVATE (aaina_behave);
	
	if (priv->func != NULL) {
		priv->func (behave, alpha_value, priv->data);
	}
}

static void
aaina_behave_class_init (AainaBehaveClass *klass)
{
	GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
	ClutterBehaviourClass *behave_class = CLUTTER_BEHAVIOUR_CLASS (klass);

	behave_class->alpha_notify = aaina_behave_alpha_notify;
	
	g_type_class_add_private (gobject_class, sizeof (AainaBehavePrivate));
}

static void
aaina_behave_init (AainaBehave *self)
{
	AainaBehavePrivate *priv;
	
	priv = AAINA_BEHAVE_GET_PRIVATE (self);
	
	priv->func = NULL;
	priv->data = NULL;
}

ClutterBehaviour*
aaina_behave_new (ClutterAlpha 		*alpha,
		 AainaBehaveAlphaFunc 	 func,
		 gpointer		 data)
{
	AainaBehave *behave;
	AainaBehavePrivate *priv;
	
	behave = g_object_new (AAINA_TYPE_BEHAVE, 
			       "alpha", alpha,
			       NULL);

	priv = AAINA_BEHAVE_GET_PRIVATE (behave);  
	
	priv->func = func;
	priv->data = data;
		
	return CLUTTER_BEHAVIOUR(behave);
}
