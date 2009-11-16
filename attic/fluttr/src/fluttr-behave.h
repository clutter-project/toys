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

#ifndef _FLUTTR_BEHAVE_H_
#define _FLUTTR_BEHAVE_H_

#include <glib-object.h>
#include <clutter/clutter.h>

#define FLUTTR_TYPE_BEHAVE (fluttr_behave_get_type ())

#define FLUTTR_BEHAVE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj),\
	FLUTTR_TYPE_BEHAVE, \
	FluttrBehave))

#define FLUTTR_BEHAVE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	FLUTTR_TYPE_BEHAVE, \
	FluttrBehaveClass))

#define CLUTTER_IS_BEHAVE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj),\
	FLUTTR_TYPE_BEHAVE))

#define CLUTTER_IS_BEHAVE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),\
	FLUTTR_TYPE_BEHAVE))

#define FLUTTR_BEHAVE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	FLUTTR_TYPE_BEHAVE, \
	FluttrBehaveClass))

typedef struct _FluttrBehave        FluttrBehave;
typedef struct _FluttrBehaveClass   FluttrBehaveClass;
typedef struct _FluttrBehavePrivate FluttrBehavePrivate;
 
struct _FluttrBehave
{
	ClutterBehaviour        parent;	
};

struct _FluttrBehaveClass
{
	ClutterBehaviourClass   parent_class;
};

typedef void (*FluttrBehaveAlphaFunc) (ClutterBehaviour  *behave, 
				     guint32 		alpha_value,
				     gpointer		data);

GType fluttr_behave_get_type (void) G_GNUC_CONST;

ClutterBehaviour* 
fluttr_behave_new (ClutterAlpha 		*alpha,
		 FluttrBehaveAlphaFunc 	 func,
		 gpointer		 data);

guint32 
alpha_sine_inc_func (ClutterAlpha *alpha,
		     gpointer      dummy);	
		     
guint32 
alpha_linear_inc_func (ClutterAlpha *alpha,
		       gpointer      dummy);		     	 

#endif /* _FLUTTR_BEHAVE_H_ */

