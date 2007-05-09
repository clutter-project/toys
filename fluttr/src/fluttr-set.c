/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-hand.com>
 */

#include "fluttr-set.h"

#include "fluttr-behave.h"
#include "fluttr-settings.h"
#include "fluttr-photo.h"


G_DEFINE_TYPE (FluttrSet, fluttr_set, CLUTTER_TYPE_GROUP);

#define FLUTTR_SET_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
	FLUTTR_TYPE_SET, \
	FluttrSetPrivate))
	
#define FONT "DejaVu Sans Book"
#define FRAME 2
#define X_ANGLE 90

#define ACT_SCALE 0.3

struct _FluttrSetPrivate
{
	gchar			*setid;
	gchar			*name;
	NFlickPhotoSet		*set;
	GList			*photos;
	
	/* The all-important pixbuf fetching variables */
	NFlickWorker		*worker;

	
	/* The actual actors */
	ClutterActor		*text;
	ClutterActor		*photo1;
	ClutterActor		*photo2;
	ClutterActor		*photo3;
	
	/* Transformation code */
	gint 			 new_x;
	gint			 new_y;
	gfloat			 new_scale;
	ClutterTimeline		*trans_time;
	ClutterAlpha		*trans_alpha;
	ClutterBehaviour	*trans_behave;
		
	/* Activate animation */
	gboolean		 active;
	gfloat			 scale;
	ClutterTimeline		*act_time;
	ClutterAlpha		*act_alpha;
	ClutterBehaviour	*act_behave;	
};

enum
{
	PROP_0,
	PROP_ID,
	PROP_NAME,
	PROP_SET
};

void
_fluttr_set_fetch_pixbuf (FluttrSet *set, guint width, guint height);

/* Will return the default size of the FluttrSet square for the current stage */
guint
fluttr_set_get_default_size (void)
{
	guint width = CLUTTER_STAGE_WIDTH ();
	guint height = CLUTTER_STAGE_HEIGHT ();
	
	if (width > height)
		return height/3;
	else
		return width /3;
}

guint
fluttr_set_get_default_width (void)
{
	return fluttr_set_get_default_size ();
}

guint
fluttr_set_get_default_height (void)
{
	return fluttr_set_get_default_width () * 1.4;
}

/* If active, scale the set, if not, scale it down */
void
fluttr_set_set_active (FluttrSet *set, gboolean active)
{
	FluttrSetPrivate *priv;

	g_return_if_fail (FLUTTR_IS_SET (set));
	priv = FLUTTR_SET_GET_PRIVATE(set);
	
	if (priv->active == active)
		return;
	
	priv->active = active;
	
	if (!clutter_timeline_is_playing (priv->act_time))
		clutter_timeline_start (priv->act_time);
	else
		clutter_timeline_rewind (priv->act_time);
}


/* Set the new x and y position of the actor, and start (or rewind) the main
   timeline */
void
fluttr_set_update_position (FluttrSet *set, gint x, gint y)
{
        FluttrSetPrivate *priv;
        
        g_return_if_fail (FLUTTR_IS_SET (set));
        priv = FLUTTR_SET_GET_PRIVATE(set);
        
        if ((priv->new_x == x) && (priv->new_y == y)) {
        	return;
        }
        priv->new_x = x;
        priv->new_y = y;
        /*clutter_actor_set_position (set, x, y);
        
        */
        if (clutter_timeline_is_playing (priv->trans_time))
        	clutter_timeline_rewind (priv->trans_time);
        else	
        	clutter_timeline_start (priv->trans_time);	
        
}

/* Allows smooth transforms (position & size) on th widget...looks goooood*/
static void
fluttr_set_trans_alpha_func (ClutterBehaviour *behave,
			       guint		 alpha_value,
			       gpointer		 data)
{
        FluttrSetPrivate *priv;
        gfloat factor;
        gint old_x, old_y;
        gint x, y;
        
        
        g_return_if_fail (FLUTTR_IS_SET (data));
        priv = FLUTTR_SET_GET_PRIVATE(data);
        
        /* Calculate the factor */
        factor = (gfloat)alpha_value / CLUTTER_ALPHA_MAX_ALPHA;
        
        /* Load up the orignal values */
        old_x = clutter_actor_get_x (CLUTTER_ACTOR (data));
        old_y = clutter_actor_get_y (CLUTTER_ACTOR (data));
        
        /* We first calculate the new x pos */
        if (old_x == priv->new_x) {
        	x = 0;
        	//g_print ("Same x %d\n", x);
        } else if (old_x < priv->new_x) {
        	/* We're moving to the positive */
        	if (old_x < 0)
        		x = ((-1*old_x)+priv->new_x) * factor;
        	else
        		x = (priv->new_x - old_x) * factor;
        } else {
        	/* We're moving to the left */
        	if (priv->new_x < 0) 
        		x = ((-1*priv->new_x)+old_x) * -1 * factor;
        	else
        		x = (old_x - priv->new_x) * -1 * factor;
        }
        
        /* Then the new y pos */
        if (old_y == priv->new_y) {
        	y = 0;
        	//g_print ("Same y %d %d\n", y, priv->new_y);
        
        } else if (old_y < priv->new_y) {
        	/* We're moving to the bottom */
        	if (old_y < 0)
        		y = ((-1*old_y)+priv->new_y) * factor;
        	else
        		y = (priv->new_y - old_y) * factor;
        } else {
        	/* We're moving to the top */
        	if (priv->new_y < 0) 
        		y = ((-1*priv->new_y)+old_y) * -1 * factor;
        	else
        		y = (old_y - priv->new_y) * -1 * factor;
        }
       	
        x += old_x;
        y += old_y;	
       	
        clutter_actor_set_position (CLUTTER_ACTOR (data), x, y);
        /*g_print ("%s %d %d\n", priv->setid, x, y);*/
        
	if (CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR(data)))
		clutter_actor_queue_redraw (CLUTTER_ACTOR(data));        
}


/* Moves the pixbuf texture on the y axis when it is active*/
static void
fluttr_set_act_alpha_func (ClutterBehaviour *behave,
		       	      guint		alpha_value,
		              gpointer		data)
{
        FluttrSetPrivate *priv;
	gfloat factor;
	guint size = fluttr_set_get_default_size ();
	
	g_return_if_fail (FLUTTR_IS_SET (data));
        priv = FLUTTR_SET_GET_PRIVATE(data);
	
	factor = (gfloat) alpha_value / CLUTTER_ALPHA_MAX_ALPHA;
	
	if (priv->active)
		priv->scale = 1 + (ACT_SCALE * factor);
	else
		priv->scale = (1 +ACT_SCALE)- (ACT_SCALE *factor);
	
	
	size = size * priv->scale;
	
	
	//clutter_actor_set_scale (CLUTTER_ACTOR (data), y
	//clutter_actor_set_position (CLUTTER_ACTOR (data), x, y);
	
	if (CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR(data)))
		clutter_actor_queue_redraw (CLUTTER_ACTOR(data));
}

static void
_refresh_thumbs (FluttrSet *set, const gchar *id, const gchar *name)
{
	FluttrSetPrivate *priv;
	gint i = 0;
	ClutterActor *photo = NULL;
	
	g_return_if_fail (FLUTTR_IS_SET (set));
	priv = FLUTTR_SET_GET_PRIVATE(set);
	
	i = g_list_length (priv->photos);
	
	if (i > 3)
		return;
	else if (i == 1)
		photo = priv->photo1;
	else if (i == 2)
		photo = priv->photo2;
	else
		photo = priv->photo3;

	if (photo) {
		g_object_set (G_OBJECT (photo),
			      "photoid", id,
			      "name", name,
			      NULL);
		fluttr_photo_fetch_pixbuf (FLUTTR_PHOTO (photo));
	}
	
	
}

void
fluttr_set_append_photo (FluttrSet *set, const gchar *id, const gchar *name)
{
	FluttrPhotoData *data;
	FluttrSetPrivate *priv;

	g_return_if_fail (FLUTTR_IS_SET (set));
	priv = FLUTTR_SET_GET_PRIVATE(set);
	
	data = g_new0 (FluttrPhotoData, 1);	
	
	data->id = g_strdup (id);
	data->name = g_strdup (id);
	data->pixbuf = NULL;
	
	priv->photos = g_list_append (priv->photos, (gpointer)data);
	
	_refresh_thumbs (set, id, name);
}

GList*
fluttr_set_get_photos (FluttrSet *set)
{
	FluttrSetPrivate *priv;

	g_return_val_if_fail (FLUTTR_IS_SET (set), NULL);
	priv = FLUTTR_SET_GET_PRIVATE(set);
	
	return priv->photos;
}

static void
_update_text (FluttrSet *set)
{
	FluttrSetPrivate *priv;

	g_return_if_fail (FLUTTR_IS_SET (set));
	priv = FLUTTR_SET_GET_PRIVATE(set);
	
	clutter_label_set_text (CLUTTER_LABEL (priv->text),
						priv->name);
	
	g_object_set (G_OBJECT (priv->text),
		      "x", (fluttr_set_get_default_width ()/2)
		     	   - (clutter_actor_get_width (priv->text)/2),
		      NULL);
}

/* GObject Stuff */

static void
fluttr_set_set_property (GObject      *object, 
			  guint         prop_id,
			  const GValue *value, 
			  GParamSpec   *pspec)
{
	FluttrSetPrivate *priv;

	g_return_if_fail (FLUTTR_IS_SET (object));
	priv = FLUTTR_SET_GET_PRIVATE(object);

	switch (prop_id) {
		case PROP_ID:
			if (priv->setid != NULL)
				g_free (priv->setid);
			priv->setid = g_strdup (g_value_get_string (value));
			break;
		case PROP_NAME:
			if (priv->name != NULL)
				g_free (priv->name);
			priv->name =g_strdup (g_value_get_string (value));
			_update_text (FLUTTR_SET (object));
			break;	
		
		case PROP_SET:
			priv->set = g_value_get_object (value);
			break;		
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, 
							   pspec);
			break;
	}
}

static void
fluttr_set_get_property (GObject    *object, 
			  guint       prop_id,
			  GValue     *value, 
			  GParamSpec *pspec)
{
	FluttrSetPrivate *priv;
	
	g_return_if_fail (FLUTTR_IS_SET (object));
	priv = FLUTTR_SET_GET_PRIVATE(object);

	switch (prop_id) {
		case PROP_ID:
			g_value_set_string (value, priv->setid);
			break;
		
		case PROP_NAME:
			g_value_set_string (value, priv->name);
			
		case PROP_SET:
			g_value_set_object (value, G_OBJECT (priv->set));
			break;
			
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id,
							   pspec);
		break;
	} 
}

static void 
fluttr_set_dispose (GObject *object)
{
	FluttrSet         *self = FLUTTR_SET(object);
	FluttrSetPrivate  *priv;  

	priv = self->priv;
  
	G_OBJECT_CLASS (fluttr_set_parent_class)->dispose (object);
}

static void 
fluttr_set_finalize (GObject *object)
{
	G_OBJECT_CLASS (fluttr_set_parent_class)->finalize (object);
}

static void
fluttr_set_class_init (FluttrSetClass *klass)
{
	GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
	ClutterActorClass   *parent_class; 

	parent_class = CLUTTER_ACTOR_CLASS (fluttr_set_parent_class);

	gobject_class->finalize     = fluttr_set_finalize;
	gobject_class->dispose      = fluttr_set_dispose;
	gobject_class->get_property = fluttr_set_get_property;
	gobject_class->set_property = fluttr_set_set_property;	

	g_type_class_add_private (gobject_class, sizeof (FluttrSetPrivate));
	
	/* Class properties */
	g_object_class_install_property 
		(gobject_class,
		 PROP_ID,
		 g_param_spec_string ("setid",
		 "SetID",
		 "The Flickr set id",
		 NULL,
		 G_PARAM_CONSTRUCT|G_PARAM_READWRITE));	
		 
	g_object_class_install_property 
		(gobject_class,
		 PROP_NAME,
		 g_param_spec_string ("name",
		 "Name",
		 "The Flickr set name",
		 NULL,
		 G_PARAM_CONSTRUCT|G_PARAM_READWRITE));		

	g_object_class_install_property 
		(gobject_class,
		 PROP_SET,
		 g_param_spec_object ("set",
		 "Set",
		 "The Flickr set",
		 NFLICK_TYPE_PHOTO_SET,
		 G_PARAM_CONSTRUCT|G_PARAM_READWRITE));		 		 		 
  

}

static void
fluttr_set_init (FluttrSet *self)
{
	FluttrSetPrivate *priv;
	ClutterColor rect_col   = { 0xff, 0xff, 0xff, 0xff };
	ClutterActor *label, *photo;
	gint width = fluttr_set_get_default_width ();
	gint height = fluttr_set_get_default_height ();
	gchar *font;
		
	priv = FLUTTR_SET_GET_PRIVATE (self);
	
	/* Create the text label */
	font = g_strdup_printf ("%s %d", FONT, height/12);
	label = clutter_label_new_full (font, "Set name", &rect_col);
	priv->text = label;
	clutter_label_set_line_wrap (CLUTTER_LABEL (label), FALSE);
	clutter_actor_set_size (label, width, height/12);
	clutter_actor_set_position (label, 0, height-(height/12));
	clutter_group_add (CLUTTER_GROUP (self), label);
	
	/* Set up the photos */
	photo = fluttr_photo_new ();
	priv->photo1 = photo;
	clutter_actor_set_size (photo, fluttr_photo_get_default_width ()/2,
				       fluttr_photo_get_default_height ()/2);
	clutter_group_add (CLUTTER_GROUP (self), photo);
	clutter_actor_set_position (photo, 
			        (width/2)-(clutter_actor_get_width(photo)/2),
			        (height/2)-(clutter_actor_get_height(photo)/2));
	clutter_actor_rotate_z (photo, 30, 
				clutter_actor_get_width (photo) /2,
				clutter_actor_get_height (photo)/2);
	
	
	photo = fluttr_photo_new ();
	priv->photo2 = photo;
	clutter_actor_set_size (photo, fluttr_photo_get_default_width ()/2,
				       fluttr_photo_get_default_height ()/2);
	clutter_group_add (CLUTTER_GROUP (self), photo);
	clutter_actor_set_position (photo, 
			        (width/2)-(clutter_actor_get_width(photo)/2),
			        (height/2)-(clutter_actor_get_height(photo)/2));
	clutter_actor_rotate_z (photo, -20, 
				clutter_actor_get_width (photo) /2,
				clutter_actor_get_height (photo)/2);
	
	
	photo = fluttr_photo_new ();
	priv->photo3 = photo;
	clutter_actor_set_size (photo, fluttr_photo_get_default_width ()/2,
				       fluttr_photo_get_default_height ()/2);
	clutter_group_add (CLUTTER_GROUP (self), photo);	
	clutter_actor_set_position (photo, 
			        (width/2)-(clutter_actor_get_width(photo)/2),
			        (height/2)-(clutter_actor_get_height(photo)/2));
	clutter_actor_rotate_z (photo, 0, 
				clutter_actor_get_width (photo) /2,
				clutter_actor_get_height (photo)/2);
	
	
	/* Setup the transformation */
	priv->new_x = priv->new_y = priv->new_scale = 0;
	priv->trans_time = clutter_timeline_new (40, 40);
	priv->trans_alpha = clutter_alpha_new_full (priv->trans_time,
					      alpha_linear_inc_func,
					      NULL, NULL);
	priv->trans_behave = fluttr_behave_new (priv->trans_alpha,
					  fluttr_set_trans_alpha_func,
					  (gpointer)self);		
					  
	/* Setup the activating line */
	priv->act_time = clutter_timeline_new (60, 240);
	priv->act_alpha = clutter_alpha_new_full (priv->act_time,
					      	   alpha_linear_inc_func,
					      	   NULL, NULL);
	priv->act_behave = fluttr_behave_new (priv->act_alpha,
					       fluttr_set_act_alpha_func,
					       (gpointer)self);

}

ClutterActor*
fluttr_set_new (NFlickPhotoSet *photo_set)
{
	ClutterGroup         *set;
	gchar		     *setid;
	gchar		     *name;
	
	g_object_get (G_OBJECT (photo_set), 
		      "id", &setid,
		      "combotext", &name,
		      NULL);

	set = g_object_new (FLUTTR_TYPE_SET, 
			    "setid", setid,
			    "name", name,
			    "set", photo_set,
			     NULL);
	return CLUTTER_ACTOR (set);
}

