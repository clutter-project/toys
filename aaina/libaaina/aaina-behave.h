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

#ifndef _AAINA_BEHAVE_H_
#define _AAINA_BEHAVE_H_

#include <glib-object.h>
#include <clutter/clutter.h>

#define AAINA_TYPE_BEHAVE (aaina_behave_get_type ())

#define AAINA_BEHAVE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj),\
	AAINA_TYPE_BEHAVE, \
	AainaBehave))

#define AAINA_BEHAVE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	AAINA_TYPE_BEHAVE, \
	AainaBehaveClass))

#define CLUTTER_IS_BEHAVE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj),\
	AAINA_TYPE_BEHAVE))

#define CLUTTER_IS_BEHAVE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),\
	AAINA_TYPE_BEHAVE))

#define AAINA_BEHAVE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	AAINA_TYPE_BEHAVE, \
	AainaBehaveClass))

typedef struct _AainaBehave        AainaBehave;
typedef struct _AainaBehaveClass   AainaBehaveClass;
typedef struct _AainaBehavePrivate AainaBehavePrivate;
 
struct _AainaBehave
{
	ClutterBehaviour        parent;	
};

struct _AainaBehaveClass
{
	ClutterBehaviourClass   parent_class;
};

typedef void (*AainaBehaveAlphaFunc) (ClutterBehaviour  *behave, 
				     guint32 		alpha_value,
				     gpointer		data);

GType aaina_behave_get_type (void) G_GNUC_CONST;

ClutterBehaviour* 
aaina_behave_new (ClutterAlpha 		*alpha,
		 AainaBehaveAlphaFunc 	 func,
		 gpointer		 data);

guint32 
alpha_sine_inc_func (ClutterAlpha *alpha,
		     gpointer      dummy);	
		     
guint32 
alpha_linear_inc_func (ClutterAlpha *alpha,
		       gpointer      dummy);		     	 

#endif /* _AAINA_BEHAVE_H_ */

