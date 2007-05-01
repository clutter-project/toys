/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-hand.com>
 */

#include "fluttr-spinner.h"

#include "fluttr-behave.h"	

G_DEFINE_TYPE (FluttrSpinner, fluttr_spinner, CLUTTER_TYPE_TEXTURE);

#define FLUTTR_SPINNER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
	FLUTTR_TYPE_SPINNER, \
	FluttrSpinnerPrivate))
	
#define FONT "DejaVu Sans Book"

static GdkPixbuf		*spinner_pixbuf = NULL;


struct _FluttrSpinnerPrivate
{
	ClutterTimeline		*timeline;
	ClutterAlpha		*alpha;
	ClutterBehaviour 	*behave;
};


/* Starts the timeline */
void
fluttr_spinner_spin (FluttrSpinner *spinner, gboolean spin)
{
	FluttrSpinnerPrivate *priv;
	
	g_return_if_fail (FLUTTR_IS_SPINNER (spinner));
	priv = FLUTTR_SPINNER_GET_PRIVATE (spinner);
	
	if (spin)
		clutter_timeline_start (priv->timeline);
	else
		clutter_timeline_stop (priv->timeline);
}


/* Spins the spinner texture on its y-axis */
static void
fluttr_spinner_alpha_func (ClutterBehaviour *behave,
		       	   guint	     alpha_value,
		           gpointer	     data)
{
	FluttrSpinnerPrivate *priv;
	gfloat factor;
	gfloat angle;
	
	g_return_if_fail (FLUTTR_IS_SPINNER (data));
	priv = FLUTTR_SPINNER_GET_PRIVATE (data);
	
	/* First we calculate the factor (how far we are along the timeline
	   between 0-1
	*/
	factor = (gfloat)alpha_value / CLUTTER_ALPHA_MAX_ALPHA;
	
	/* Calculate the angle */
	angle = factor * 360.0;
	
	/* Set the new angle */
	clutter_actor_rotate_z (CLUTTER_ACTOR (data), angle, 
			clutter_actor_get_width (CLUTTER_ACTOR (data)) /2,
			clutter_actor_get_height (CLUTTER_ACTOR (data)) /2);
	
	if (CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR(data)))
		clutter_actor_queue_redraw (CLUTTER_ACTOR(data));	
} 		
		
/* GObject Stuff */

static void 
fluttr_spinner_dispose (GObject *object)
{
	FluttrSpinner         *self = FLUTTR_SPINNER(object);
	FluttrSpinnerPrivate  *priv;  

	priv = self->priv;
  
	G_OBJECT_CLASS (fluttr_spinner_parent_class)->dispose (object);
}

static void 
fluttr_spinner_finalize (GObject *object)
{
	G_OBJECT_CLASS (fluttr_spinner_parent_class)->finalize (object);
}

static void
fluttr_spinner_class_init (FluttrSpinnerClass *klass)
{
	GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
	ClutterActorClass   *parent_class; 

	parent_class = CLUTTER_ACTOR_CLASS (fluttr_spinner_parent_class);

	gobject_class->finalize     = fluttr_spinner_finalize;
	gobject_class->dispose      = fluttr_spinner_dispose;
	
	g_type_class_add_private (gobject_class, sizeof (FluttrSpinnerPrivate));
}

static void
fluttr_spinner_init (FluttrSpinner *self)
{
	FluttrSpinnerPrivate *priv;
	priv = FLUTTR_SPINNER_GET_PRIVATE (self);
	
	priv->timeline = clutter_timeline_new (40, 50);
	clutter_timeline_set_loop (priv->timeline, TRUE);
	priv->alpha = clutter_alpha_new_full (priv->timeline,
					      alpha_linear_inc_func,
					      NULL, NULL);
	priv->behave = fluttr_behave_new (priv->alpha,
					  fluttr_spinner_alpha_func,
					  (gpointer)self);
}

ClutterActor*
fluttr_spinner_new (void)
{
	ClutterGroup         *spinner;

	spinner = g_object_new (FLUTTR_TYPE_SPINNER, 
				NULL);

	if (spinner_pixbuf == NULL) {
  		spinner_pixbuf = gdk_pixbuf_new_from_file_at_scale (PKGDATADIR \
  						    	"/spinner.svg",
  						    CLUTTER_STAGE_HEIGHT ()/9,
  						    CLUTTER_STAGE_HEIGHT ()/9,
  						    FALSE,
  						    NULL);
	}
	if (spinner_pixbuf)
		clutter_texture_set_pixbuf (CLUTTER_TEXTURE (spinner),
					    spinner_pixbuf);
	else
		g_print ("Could not load spinner\n");	
 	return CLUTTER_ACTOR (spinner);
}

