/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-hand.com>
 */

#include "fluttr-photo.h"

#include "fluttr-behave.h"
#include "fluttr-settings.h"

#include "nflick-worker.h"
#include "nflick-show-worker.h"

G_DEFINE_TYPE (FluttrPhoto, fluttr_photo, CLUTTER_TYPE_GROUP);

#define FLUTTR_PHOTO_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
	FLUTTR_TYPE_PHOTO, \
	FluttrPhotoPrivate))
	
#define FONT "DejaVu Sans Book"
#define FRAME 2

static GdkPixbuf	*default_pic = NULL;

struct _FluttrPhotoPrivate
{
	gchar			*photoid;
	gchar			*name;
	NFlickPhotoSet		*set;
	
	/* The all-important pixbuf fetching variables */
	NFlickWorker		*worker;
	GdkPixbuf		*pixbuf;
	
	/* Transformation code */
	gint 			 new_x;
	gint			 new_y;
	gfloat			 new_scale;
	ClutterTimeline		*trans_time;
	ClutterAlpha		*trans_alpha;
	ClutterBehaviour	*trans_behave;

	
	/* The actual actors */
	ClutterActor		*frame;
	ClutterActor		*clip;
	ClutterActor		*texture;
	ClutterActor		*options; /* The 'flip' side */
	
	/* Swap pixbuf code */
	ClutterTimeline		*swap_time;
	ClutterAlpha		*swap_alpha;
	ClutterBehaviour	*swap_behave;	
};

enum
{
	PROP_0,
	PROP_ID,
	PROP_NAME,
	PROP_PIXBUF,
	PROP_SET
};

enum
{
	LOADED,
	ERROR,
	LAST_SIGNAL
};

static guint _photo_signals[LAST_SIGNAL] = { 0 };

/* Set the new x and y position of the actor, and start (or rewind) the main
   timeline */
void
fluttr_photo_update_position (FluttrPhoto *photo, gint x, gint y)
{
        FluttrPhotoPrivate *priv;
        
        g_return_if_fail (FLUTTR_IS_PHOTO (photo));
        priv = FLUTTR_PHOTO_GET_PRIVATE(photo);
        
        priv->new_x = x;
        priv->new_y = y;
        
        if (clutter_timeline_is_playing (priv->trans_time))
        	clutter_timeline_rewind (priv->trans_time);
        else	
        	clutter_timeline_start (priv->trans_time);	
}

/* Allows smooth transforms (position & size) on th widget...looks goooood*/
static void
fluttr_photo_trans_alpha_func (ClutterBehaviour *behave,
			       guint		 alpha_value,
			       gpointer		 data)
{
        FluttrPhotoPrivate *priv;
        gfloat factor;
        gint old_x, old_y;
        gint x, y;
        
        
        g_return_if_fail (FLUTTR_IS_PHOTO (data));
        priv = FLUTTR_PHOTO_GET_PRIVATE(data);
        
        /* Calculate the factor */
        factor = (gfloat)alpha_value / CLUTTER_ALPHA_MAX_ALPHA;
        
        /* Load up the orignal values */
        old_x = clutter_actor_get_x (CLUTTER_ACTOR (data));
        old_y = clutter_actor_get_y (CLUTTER_ACTOR (data));
        
        
        /* We first calculate the new x pos */
        if (old_x == priv->new_x)
        	x = old_x;
        else if (old_x < priv->new_x) {
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
        if (old_y == priv->new_y)
        	y = old_y;
        else if (old_y < priv->new_y) {
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
        
        clutter_actor_set_position (CLUTTER_ACTOR (data), x, y);
        
	if (CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR(data)))
		clutter_actor_queue_redraw (CLUTTER_ACTOR(data));        
}

/* Fade out text, change text, then fade in, all within one play of the timeline
   just to keep things interesting :) */
static void
fluttr_photo_swap_alpha_func (ClutterBehaviour *behave,
		       	      guint		alpha_value,
		              gpointer		data)
{
        FluttrPhotoPrivate *priv;
	gfloat factor;
	guint size = fluttr_photo_get_default_size ();
	gint w, h;
	
        g_return_if_fail (FLUTTR_IS_PHOTO (data));
        priv = FLUTTR_PHOTO_GET_PRIVATE(data);
	
	factor = (gfloat) alpha_value / CLUTTER_ALPHA_MAX_ALPHA;
	
	if (priv->pixbuf != NULL && factor > 0.5) {
		w = gdk_pixbuf_get_width (priv->pixbuf);
		h = gdk_pixbuf_get_height (priv->pixbuf);
		
		clutter_texture_set_pixbuf (CLUTTER_TEXTURE (priv->texture),
					    priv->pixbuf);
		clutter_actor_set_position (priv->texture, 
					    (size/2) - (w/2),
					    (size/2) - (h/2));    
	}
	if (factor < 0.5) {
		factor *= 2;
		factor = 1.0 - factor;
	} else {
		factor -= 0.5;
		factor /= 0.5;
	}
	
	clutter_actor_set_opacity (CLUTTER_ACTOR (priv->texture), 255 * factor);
	clutter_actor_set_opacity (CLUTTER_ACTOR (priv->frame), 255 * factor);
	
	if (CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR(data)))
		clutter_actor_queue_redraw (CLUTTER_ACTOR(data));	
}


static gboolean                 
on_thread_abort_idle (FluttrPhoto *photo)
{
        g_return_val_if_fail (FLUTTR_IS_PHOTO (photo), FALSE);

	g_signal_emit (photo, _photo_signals[ERROR], 0, "Aborted");	

        return FALSE;
}

static gboolean                 
on_thread_ok_idle (FluttrPhoto *photo)
{
        FluttrPhotoPrivate *priv;
        GdkPixbuf *pixbuf;
        
        g_return_val_if_fail (FLUTTR_IS_PHOTO (photo), FALSE);
        priv = FLUTTR_PHOTO_GET_PRIVATE(photo);
        
        /* Get pixbuf from worker */
        g_object_get (G_OBJECT (priv->worker), "pixbuf", &pixbuf, NULL);
        priv->pixbuf = pixbuf;        
        
        /*g_print ("%d %d\n", gdk_pixbuf_get_width (pixbuf), 
        		    gdk_pixbuf_get_height (pixbuf));
	*/
	if (clutter_timeline_is_playing (priv->swap_time))
        	;//clutter_timeline_rewind (priv->text_time);
       
        else
        	clutter_timeline_start (priv->swap_time);


	g_signal_emit (photo, _photo_signals[LOADED], 0, "");
        
        return FALSE;
}

static gboolean                 
on_thread_error_idle (FluttrPhoto *photo)
{
        FluttrPhotoPrivate *priv;
        gchar *error = NULL;
        
        g_return_val_if_fail (FLUTTR_IS_PHOTO (photo), FALSE);
        priv = FLUTTR_PHOTO_GET_PRIVATE(photo);
        
        /* Get the actual error */
        g_object_get (G_OBJECT (priv->worker), "error", &error, NULL);
        if (error == NULL) {
                error = g_strdup_printf (gettext ("Internal error. "));
                g_warning ("No error set on worker!");
        }
	g_signal_emit (photo, _photo_signals[ERROR], 0, error);

        g_free (error);

        return FALSE;
}

static gboolean                 
on_thread_msg_change_idle (FluttrPhoto *photo)
{
        FluttrPhotoPrivate *priv;
        gchar *msg = NULL;
        
        g_return_val_if_fail (FLUTTR_IS_PHOTO (photo), FALSE);
        priv = FLUTTR_PHOTO_GET_PRIVATE(photo);
        
        /* Get the new message */
        g_object_get (G_OBJECT (priv->worker), "message", &msg, NULL);
        if (msg != NULL) {
                g_print ("%s", msg);
        }
        
        g_free (msg);

        return FALSE;
}


/* Start the pixbuf worker */
void
fluttr_photo_fetch_pixbuf (FluttrPhoto *photo)
{
        FluttrPhotoPrivate *priv;
        FluttrSettings *settings = fluttr_settings_get_default ();
	NFlickWorker *worker;
        NFlickWorkerStatus status;
              
        gchar *token = NULL;
	guint size = fluttr_photo_get_default_size ();
	size *= 1.5;
        
        g_return_if_fail (FLUTTR_IS_PHOTO (photo));
        priv = FLUTTR_PHOTO_GET_PRIVATE(photo);	
        
        if (priv->pixbuf != NULL)
        	return;
	
	g_object_get (G_OBJECT (settings), "token", &token, NULL);
	
	worker = (NFlickWorker *)nflick_show_worker_new (priv->photoid, 
							       size, 
							       size, 
							       token);
        /* Check if the worker is in the right state */
        g_object_get (G_OBJECT (worker), "status", &status, NULL);
        
        if (status != NFLICK_WORKER_STATUS_IDLE) {
                g_warning ("Bad worker status"); 
                return;
        }
        
        g_object_ref (worker);
        priv->worker = worker;
        
        /* Get the initial message */
        gchar *msg = NULL;
        g_object_get (G_OBJECT (priv->worker), "message", &msg, NULL);
        if (msg != NULL) {
                /* FIXME Escape markup */
        	g_print ("%s", msg);
        }
        
        /* Set the callback functions */
        nflick_worker_set_custom_data (worker, photo);
        nflick_worker_set_aborted_idle (worker, 
        			   (NFlickWorkerIdleFunc) on_thread_abort_idle);
        
        nflick_worker_set_error_idle (worker, 
        			   (NFlickWorkerIdleFunc) on_thread_error_idle);
        
        nflick_worker_set_ok_idle (worker, 
        			      (NFlickWorkerIdleFunc) on_thread_ok_idle);
        
        nflick_worker_set_msg_change_idle (worker, 
        		      (NFlickWorkerIdleFunc) on_thread_msg_change_idle);
                                        
        nflick_worker_start (priv->worker);
        
        /* Free */
        g_free (msg);
}

/* Will return the default size of the FluttrPhoto square for the current stage */
guint
fluttr_photo_get_default_size (void)
{
	guint width = CLUTTER_STAGE_WIDTH ();
	guint height = CLUTTER_STAGE_HEIGHT ();
	
	if (width > height)
		return height/4;
	else
		return width /4;
}

/* GObject Stuff */

static void
fluttr_photo_set_property (GObject      *object, 
			  guint         prop_id,
			  const GValue *value, 
			  GParamSpec   *pspec)
{
	FluttrPhotoPrivate *priv;

	g_return_if_fail (FLUTTR_IS_PHOTO (object));
	priv = FLUTTR_PHOTO_GET_PRIVATE(object);

	switch (prop_id) {
		case PROP_ID:
			if (priv->photoid != NULL)
				g_free (priv->photoid);
			priv->photoid = g_strdup (g_value_get_string (value));
			break;
		case PROP_NAME:
			if (priv->name != NULL)
				g_free (priv->name);
			priv->name =g_strdup (g_value_get_string (value));
			break;	
		case PROP_PIXBUF:
			if (priv->pixbuf != NULL)
				g_object_unref (G_OBJECT (priv->pixbuf));
			priv->pixbuf = g_value_get_object (value);
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
fluttr_photo_get_property (GObject    *object, 
			  guint       prop_id,
			  GValue     *value, 
			  GParamSpec *pspec)
{
	FluttrPhotoPrivate *priv;
	
	g_return_if_fail (FLUTTR_IS_PHOTO (object));
	priv = FLUTTR_PHOTO_GET_PRIVATE(object);

	switch (prop_id) {
		case PROP_ID:
			g_value_set_string (value, priv->photoid);
			break;
		
		case PROP_NAME:
			g_value_set_string (value, priv->name);
			
		case PROP_PIXBUF:
			g_value_set_object (value, G_OBJECT (priv->pixbuf));
			break;
			
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
fluttr_photo_paint (ClutterActor *actor)
{
	FluttrPhoto        *photo;
	FluttrPhotoPrivate *priv;

	photo = FLUTTR_PHOTO(actor);

	priv = FLUTTR_PHOTO_GET_PRIVATE(photo);

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
fluttr_photo_dispose (GObject *object)
{
	FluttrPhoto         *self = FLUTTR_PHOTO(object);
	FluttrPhotoPrivate  *priv;  

	priv = self->priv;
  
	G_OBJECT_CLASS (fluttr_photo_parent_class)->dispose (object);
}

static void 
fluttr_photo_finalize (GObject *object)
{
	G_OBJECT_CLASS (fluttr_photo_parent_class)->finalize (object);
}

static void
fluttr_photo_class_init (FluttrPhotoClass *klass)
{
	GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
	ClutterActorClass   *actor_class = CLUTTER_ACTOR_CLASS (klass);
	ClutterActorClass   *parent_class; 

	parent_class = CLUTTER_ACTOR_CLASS (fluttr_photo_parent_class);

	actor_class->paint           = fluttr_photo_paint;
	
	gobject_class->finalize     = fluttr_photo_finalize;
	gobject_class->dispose      = fluttr_photo_dispose;
	gobject_class->get_property = fluttr_photo_get_property;
	gobject_class->set_property = fluttr_photo_set_property;	

	g_type_class_add_private (gobject_class, sizeof (FluttrPhotoPrivate));
	
	/* Class properties */
	g_object_class_install_property 
		(gobject_class,
		 PROP_ID,
		 g_param_spec_string ("photoid",
		 "PhotoID",
		 "The Flickr photo id",
		 NULL,
		 G_PARAM_CONSTRUCT|G_PARAM_READWRITE));	
		 
	g_object_class_install_property 
		(gobject_class,
		 PROP_NAME,
		 g_param_spec_string ("name",
		 "Name",
		 "The Flickr photo name",
		 NULL,
		 G_PARAM_CONSTRUCT|G_PARAM_READWRITE));	
		 	
	g_object_class_install_property 
		(gobject_class,
		 PROP_PIXBUF,
		 g_param_spec_object ("pixbuf",
		 "Pixbuf",
		 "The GdkPixbuf of the photo",
		 GDK_TYPE_PIXBUF,
		 G_PARAM_CONSTRUCT|G_PARAM_READWRITE));		

	g_object_class_install_property 
		(gobject_class,
		 PROP_SET,
		 g_param_spec_object ("set",
		 "Set",
		 "The Flickr set",
		 NFLICK_TYPE_PHOTO_SET,
		 G_PARAM_CONSTRUCT|G_PARAM_READWRITE));		 		 		 

	/* Class signals */
	_photo_signals[LOADED] =
		g_signal_new ("pixbuf-loaded",
			     G_OBJECT_CLASS_TYPE (gobject_class),
			     G_SIGNAL_RUN_FIRST,
			     G_STRUCT_OFFSET (FluttrPhotoClass, pixbuf_loaded),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__STRING,
			     G_TYPE_NONE, 1, G_TYPE_STRING);
			     
	_photo_signals[ERROR] =
		g_signal_new ("error",
			     G_OBJECT_CLASS_TYPE (gobject_class),
			     G_SIGNAL_RUN_FIRST,
			     G_STRUCT_OFFSET (FluttrPhotoClass, fetch_error),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__STRING,
			     G_TYPE_NONE, 1, G_TYPE_STRING);			     

}

static void
fluttr_photo_init (FluttrPhoto *self)
{
	FluttrPhotoPrivate *priv;
	ClutterColor rect_col   = { 0xff, 0xff, 0xff, 0xff };
	guint size = fluttr_photo_get_default_size ();
	
	priv = FLUTTR_PHOTO_GET_PRIVATE (self);
	
	priv->pixbuf = NULL;
	
	/* The white frame */
	priv->frame = clutter_rectangle_new_with_color (&rect_col);
	clutter_group_add (CLUTTER_GROUP (self), priv->frame);	
	clutter_actor_set_size (priv->frame, size, size);
	clutter_actor_set_position (priv->frame, 0, 0);
	
	/*Load the default pixbuf */
	if (default_pic == NULL) {
		default_pic = gdk_pixbuf_new_from_file_at_scale (PKGDATADIR \
  						    	"/picture.svg",
  						        size -(FRAME*2),
  						        size -(FRAME*2),
  						        FALSE,
  						        NULL);
	}
	/* The picture clip region */
	priv->clip = clutter_group_new ();
	clutter_group_add (CLUTTER_GROUP (self),priv->clip);
	clutter_actor_set_size (priv->clip, 
				size -(FRAME*2), 
				size -(FRAME*2));
	clutter_actor_set_position (priv->clip, 0, 0);
	clutter_actor_set_clip (priv->clip,
				FRAME, FRAME,
				size -(FRAME*2),
				size -(FRAME*2));
	
	/* The pixture texture */
	priv->texture = clutter_texture_new_from_pixbuf (default_pic);
	clutter_group_add (CLUTTER_GROUP (priv->clip), priv->texture);
	clutter_actor_set_size (priv->texture, 
				size -(FRAME*2), 
				size -(FRAME*2));
	clutter_actor_set_position (priv->texture, FRAME, FRAME);
	
	/* Setup the transformation */
	priv->new_x = priv->new_y = priv->new_scale = 0;
	priv->trans_time = clutter_timeline_new (40, 80);
	priv->trans_alpha = clutter_alpha_new_full (priv->trans_time,
					      alpha_linear_inc_func,
					      NULL, NULL);
	priv->trans_behave = fluttr_behave_new (priv->trans_alpha,
					  fluttr_photo_trans_alpha_func,
					  (gpointer)self);	
					  
	/* Setup the pixbuf swap */
	priv->pixbuf = NULL;
	priv->swap_time = clutter_timeline_new (40, 40);
	priv->swap_alpha = clutter_alpha_new_full (priv->swap_time,
					      alpha_linear_inc_func,
					      NULL, NULL);
	priv->swap_behave = fluttr_behave_new (priv->swap_alpha,
					  fluttr_photo_swap_alpha_func,
					  (gpointer)self);	
}

ClutterActor*
fluttr_photo_new (void)
{
	ClutterGroup         *photo;

	photo = g_object_new (FLUTTR_TYPE_PHOTO, 
			     NULL);
	return CLUTTER_ACTOR (photo);
}

