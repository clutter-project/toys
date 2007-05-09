/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-hand.com>
 */

#include "fluttr-viewer.h"

#include "fluttr-spinner.h"
#include "fluttr-behave.h"
#include "fluttr-settings.h"

G_DEFINE_TYPE (FluttrViewer, fluttr_viewer, CLUTTER_TYPE_GROUP);

#define FLUTTR_VIEWER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
	FLUTTR_TYPE_VIEWER, \
	FluttrViewerPrivate))
	
#define FONT "DejaVu Sans Book"


struct _FluttrViewerPrivate
{
	gchar 			*mini_token;
	gchar 			*username;
	gchar			*fullname;
	gchar			*token;
	gchar			*usernsid;
	
	NFlickWorker		*worker;

	GdkPixbuf		*logo;
	ClutterActor		*group;
	ClutterActor		*texture;
	ClutterActor		*spinner;

	gchar			*msg;

	gboolean		 popping;
	gboolean		 show;
	
	ClutterTimeline		*timeline;
	ClutterAlpha		*alpha;
	ClutterBehaviour 	*behave;
	
	/* Swap pixbuf code */
	GdkPixbuf		*pixbuf;
	ClutterTimeline		*swap_time;
	ClutterAlpha		*swap_alpha;
	ClutterBehaviour	*swap_behave;		

};

enum
{
	PROP_0,
	PROP_PIXBUF
};

enum
{
	SUCCESSFUL,
	ERROR,
	LAST_SIGNAL
};

static guint _viewer_signals[LAST_SIGNAL] = { 0 };


void
fluttr_viewer_show (FluttrViewer *viewer, gboolean show)
{
        FluttrViewerPrivate *priv;
        
        g_return_if_fail (FLUTTR_IS_VIEWER (viewer));
        priv = FLUTTR_VIEWER_GET_PRIVATE(viewer);
        
        priv->popping = show;
        if (!clutter_timeline_is_playing (priv->timeline))
        	clutter_timeline_start (priv->timeline);
}	

static void
close_message_window (FluttrViewer *viewer)
{
        FluttrViewerPrivate *priv;
        
        g_return_if_fail (FLUTTR_IS_VIEWER (viewer));
        priv = FLUTTR_VIEWER_GET_PRIVATE(viewer);
        
}

static gboolean                 
on_thread_abort_idle (FluttrViewer *viewer)
{
        g_return_val_if_fail (FLUTTR_IS_VIEWER (viewer), FALSE);
        
        close_message_window (viewer);

	g_signal_emit (viewer, _viewer_signals[ERROR], 0, "Aborted");	
	
	g_print ("Aborted\n");	

        return FALSE;
}

static gboolean                 
on_thread_ok_idle (FluttrViewer *viewer)
{
        FluttrViewerPrivate *priv;
        GdkPixbuf *pixbuf;
        
        g_return_val_if_fail (FLUTTR_IS_VIEWER (viewer), FALSE);
        priv = FLUTTR_VIEWER_GET_PRIVATE(viewer);
        
        close_message_window (viewer);
               
       /* Get pixbuf from worker */
        g_object_get (G_OBJECT (priv->worker), "pixbuf", &pixbuf, NULL);
        priv->pixbuf = pixbuf;        
        
	if (!clutter_timeline_is_playing (priv->swap_time))
        	clutter_timeline_start (priv->swap_time);
        
        g_signal_emit (viewer, _viewer_signals[SUCCESSFUL], 0, priv->worker);
        
        return FALSE;
}

static gboolean                 
on_thread_error_idle (FluttrViewer *viewer)
{
        FluttrViewerPrivate *priv;
        gchar *error = NULL;
        
        g_return_val_if_fail (FLUTTR_IS_VIEWER (viewer), FALSE);
        priv = FLUTTR_VIEWER_GET_PRIVATE(viewer);
        
        close_message_window (viewer);
        
        /* Get the actual error */
        g_object_get (G_OBJECT (priv->worker), "error", &error, NULL);
        if (error == NULL) {
                error = g_strdup_printf (gettext ("Internal error. "));
                g_warning ("No error set on worker!");
        }
	g_signal_emit (viewer, _viewer_signals[ERROR], 0, error);
	
	g_print ("%s\n", error);
	
        g_free (error);

        return FALSE;
}

/* Copy the new message and start the fade effect if not already started */
static gboolean                 
on_thread_msg_change_idle (FluttrViewer *viewer)
{
        FluttrViewerPrivate *priv;
        gchar *msg;
        
        g_return_val_if_fail (FLUTTR_IS_VIEWER (viewer), FALSE);
        priv = FLUTTR_VIEWER_GET_PRIVATE(viewer);
        
        /* Get the new message */
        g_object_get (G_OBJECT (priv->worker), "message", &msg, NULL);
        if (msg != NULL) {
                g_print ("%s\n", msg);
        }
        priv->msg = g_strdup (msg);

	return FALSE;
}


/* This function does th emain work of creating and configuring the worker 
   thread. the majority of this code is taken from
   NFlick the n800 Flickr photo browser by MDK (see: README) */
void
fluttr_viewer_go (FluttrViewer *viewer, FluttrPhoto *photo)
{
        FluttrViewerPrivate *priv;
        FluttrSettings *settings = fluttr_settings_get_default ();
	NFlickWorker *worker;
        NFlickWorkerStatus status;
        gint width = CLUTTER_STAGE_WIDTH ();
        gint height = CLUTTER_STAGE_HEIGHT();
              
        gchar *token = NULL;
        gchar *photoid = NULL;
	
        g_return_if_fail (FLUTTR_IS_VIEWER (viewer));
        priv = FLUTTR_VIEWER_GET_PRIVATE(viewer);
        
        if (priv->worker)
        	nflick_worker_request_abort (priv->worker);
        
        //fluttr_spinner_spin (FLUTTR_SPINNER (priv->spinner), TRUE);
        
		
	g_object_get (G_OBJECT (settings), "token", &token, NULL);
	g_object_get (G_OBJECT (photo), "photoid", &photoid, NULL);
	
	worker = (NFlickWorker *)nflick_show_worker_new (photoid, 
							 width, 
							 height, 
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
        	//g_print ("%s", msg);
        }
        
        /* Set the callback functions */
        nflick_worker_set_custom_data (worker, viewer);
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


/* Slide in or out the notification popp, depending on priv->pop_visible */
static void
fluttr_viewer_alpha_func (ClutterBehaviour *behave,
		       	      guint		alpha_value,
		              gpointer		data)
{
	FluttrViewerPrivate *priv;
	gfloat factor;

       	g_return_if_fail (FLUTTR_IS_VIEWER (data));
	priv = FLUTTR_VIEWER_GET_PRIVATE(data);
	
	factor = (gfloat)alpha_value / CLUTTER_ALPHA_MAX_ALPHA;
	
	if (priv->popping) 
		clutter_actor_set_opacity (CLUTTER_ACTOR (data), 255 * factor);
	else
		clutter_actor_set_opacity (CLUTTER_ACTOR (data), 
					   255- (255*factor));
	
	if (CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR(data)))
		clutter_actor_queue_redraw (CLUTTER_ACTOR(data));	
}

/* Fade out text, change text, then fade in, all within one play of the timeline
   just to keep things interesting :) */
static void
fluttr_viewer_swap_alpha_func (ClutterBehaviour *behave,
		       	      guint		alpha_value,
		              gpointer		data)
{
	FluttrViewerPrivate *priv;
	gfloat factor;
	guint width = CLUTTER_STAGE_WIDTH ();
	guint height = CLUTTER_STAGE_HEIGHT ();
	guint w, h;
	
       	g_return_if_fail (FLUTTR_IS_VIEWER (data));
	priv = FLUTTR_VIEWER_GET_PRIVATE(data);
	
	factor = (gfloat) alpha_value / CLUTTER_ALPHA_MAX_ALPHA;
	
	if (priv->pixbuf != NULL && factor > 0.5) {
		clutter_texture_set_pixbuf (CLUTTER_TEXTURE (priv->texture),
					    priv->pixbuf);
		clutter_actor_get_size (priv->texture, &w, &h);
		
		clutter_actor_set_position (priv->texture, 
					    (width/2) - (w/2),
					    (height/2) - (h/2));    
	}
	if (factor < 0.5) {
		factor *= 2;
		factor = 1.0 - factor;
	} else {
		factor -= 0.5;
		factor /= 0.5;
	}
	
	if (priv->popping)
		clutter_actor_set_opacity (CLUTTER_ACTOR (priv->texture), 
					   255 * factor);
	
	if (CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR(data)))
		clutter_actor_queue_redraw (CLUTTER_ACTOR(data));	
}

	
/* GObject Stuff */

static void
fluttr_viewer_set_property (GObject      *object, 
			  guint         prop_id,
			  const GValue *value, 
			  GParamSpec   *pspec)
{
	FluttrViewerPrivate *priv;

	g_return_if_fail (FLUTTR_IS_VIEWER (object));
	priv = FLUTTR_VIEWER_GET_PRIVATE(object);

	switch (prop_id) {
		case PROP_PIXBUF:
			if (priv->pixbuf != NULL)
				g_object_unref (G_OBJECT (priv->pixbuf));
			priv->pixbuf = g_value_get_object (value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, 
							   pspec);
			break;
	}
}

static void
fluttr_viewer_get_property (GObject    *object, 
			  guint       prop_id,
			  GValue     *value, 
			  GParamSpec *pspec)
{
	FluttrViewerPrivate *priv;
	
	g_return_if_fail (FLUTTR_IS_VIEWER (object));
	priv = FLUTTR_VIEWER_GET_PRIVATE(object);

	switch (prop_id) {
		case PROP_PIXBUF:
			g_value_set_object (value, priv->pixbuf);
			break;		
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id,
							   pspec);
		break;
	} 
}

static void
fluttr_viewer_paint (ClutterActor *actor)
{
	FluttrViewer        *viewer;
	FluttrViewerPrivate *priv;

	viewer = FLUTTR_VIEWER(actor);

	priv = FLUTTR_VIEWER_GET_PRIVATE(viewer);

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
fluttr_viewer_dispose (GObject *object)
{
	FluttrViewer         *self = FLUTTR_VIEWER(object);
	FluttrViewerPrivate  *priv;  

	priv = self->priv;
  
	G_OBJECT_CLASS (fluttr_viewer_parent_class)->dispose (object);
}

static void 
fluttr_viewer_finalize (GObject *object)
{
	G_OBJECT_CLASS (fluttr_viewer_parent_class)->finalize (object);
}

static void
fluttr_viewer_class_init (FluttrViewerClass *klass)
{
	GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
	ClutterActorClass   *actor_class = CLUTTER_ACTOR_CLASS (klass);
	ClutterActorClass   *parent_class; 

	parent_class = CLUTTER_ACTOR_CLASS (fluttr_viewer_parent_class);

	actor_class->paint           = fluttr_viewer_paint;
	
	gobject_class->finalize     = fluttr_viewer_finalize;
	gobject_class->dispose      = fluttr_viewer_dispose;
	gobject_class->get_property = fluttr_viewer_get_property;
	gobject_class->set_property = fluttr_viewer_set_property;	

	g_type_class_add_private (gobject_class, sizeof (FluttrViewerPrivate));
	
	/* Class properties */
	g_object_class_install_property 
		(gobject_class,
		 PROP_PIXBUF,
		 g_param_spec_object ("pixbuf",
		 "Pixbuf",
		 "The current pixbuf",
		 GDK_TYPE_PIXBUF,
		 G_PARAM_CONSTRUCT|G_PARAM_READWRITE));	
		 		 		 		 

	/* Class signals */
	_viewer_signals[SUCCESSFUL] =
		g_signal_new ("successful",
			     G_OBJECT_CLASS_TYPE (gobject_class),
			     G_SIGNAL_RUN_FIRST,
			     G_STRUCT_OFFSET (FluttrViewerClass, successful),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__OBJECT,
			     G_TYPE_NONE, 1, NFLICK_TYPE_WORKER);
			     
	_viewer_signals[ERROR] =
		g_signal_new ("error",
			     G_OBJECT_CLASS_TYPE (gobject_class),
			     G_SIGNAL_RUN_FIRST,
			     G_STRUCT_OFFSET (FluttrViewerClass, error),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__STRING,
			     G_TYPE_NONE, 1, G_TYPE_STRING);			     

}

static void
fluttr_viewer_init (FluttrViewer *self)
{
	FluttrViewerPrivate *priv;
	gint width, height;
	ClutterActor *message;
	
	priv = FLUTTR_VIEWER_GET_PRIVATE (self);
	
	priv->mini_token = NULL;
	priv->popping = FALSE;

	width = CLUTTER_STAGE_WIDTH ();
	height = CLUTTER_STAGE_HEIGHT ();
		
	/* Group */
	priv->group = clutter_group_new ();
	clutter_group_add (CLUTTER_GROUP (self),priv->group); 
	clutter_actor_set_size (priv->group, width, height);
	clutter_actor_set_position (priv->group, 
				    (CLUTTER_STAGE_WIDTH ()/2) - (width/2),
				    (CLUTTER_STAGE_HEIGHT ()/2) - (height/2));	
	
	/* message box */
	message = clutter_texture_new ();
	priv->texture = message;
	clutter_group_add (CLUTTER_GROUP (priv->group),message); 
	clutter_actor_set_size (message, width, height);
	clutter_actor_set_position (message, -(width/2),-(height/2));
	
				    
				    
	/* Setup the pixbuf swap */
	priv->pixbuf = NULL;
	priv->swap_time = clutter_timeline_new (40, 40);
	priv->swap_alpha = clutter_alpha_new_full (priv->swap_time,
					           alpha_linear_inc_func,
					           NULL, NULL);
	priv->swap_behave = fluttr_behave_new (priv->swap_alpha,
					       fluttr_viewer_swap_alpha_func,
					       (gpointer)self);
					  				    
	priv->timeline = clutter_timeline_new (40, 80);
	priv->alpha = clutter_alpha_new_full (priv->timeline,
					      alpha_sine_inc_func,
					      NULL, NULL);
	priv->behave = fluttr_behave_new (priv->alpha,
					  fluttr_viewer_alpha_func,
					  (gpointer)self);
		
}

ClutterActor*
fluttr_viewer_new (void)
{
	ClutterGroup         *viewer;

	viewer = g_object_new (FLUTTR_TYPE_VIEWER, 
			     NULL);
	
	clutter_actor_set_opacity (CLUTTER_ACTOR (viewer), 0);
	
	return CLUTTER_ACTOR (viewer);
}

