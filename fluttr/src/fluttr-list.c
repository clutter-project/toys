/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-hand.com>
 */

#include "fluttr-list.h"

#include "fluttr-spinner.h"
#include "fluttr-behave.h"

#include "nflick-worker.h"
#include "nflick-set-list-worker.h"

G_DEFINE_TYPE (FluttrList, fluttr_list, CLUTTER_TYPE_GROUP);

#define FLUTTR_LIST_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
	FLUTTR_TYPE_LIST, \
	FluttrListPrivate))
	
#define FONT "DejaVu Sans Book"


struct _FluttrListPrivate
{
	gchar 			*mini_token;
	gchar 			*username;
	gchar			*fullname;
	gchar			*token;
	gchar			*usernsid;
	
	NFlickWorker		*worker;

	GdkPixbuf		*logo;
	ClutterActor		*group;
	ClutterActor		*message;
	ClutterActor		*spinner;
	ClutterActor		*text;
	gchar			*msg;
	gboolean		 popping;
	
	ClutterTimeline		*timeline;
	ClutterAlpha		*alpha;
	ClutterBehaviour 	*behave;
	
	ClutterTimeline		*text_time;
	ClutterAlpha		*text_alpha;
	ClutterBehaviour 	*text_behave;	
};

enum
{
	PROP_0,
	PROP_MINI_TOKEN,
	PROP_USERNAME,
	PROP_FULLNAME,
	PROP_TOKEN,
	PROP_USERNSID
};

enum
{
	SUCCESSFUL,
	ERROR,
	LAST_SIGNAL
};

static guint _list_signals[LAST_SIGNAL] = { 0 };

static void
close_message_window (FluttrList *list)
{
        FluttrListPrivate *priv;
        
        g_return_if_fail (FLUTTR_IS_LIST (list));
        priv = FLUTTR_LIST_GET_PRIVATE(list);
        
        priv->popping = FALSE;
        clutter_timeline_start (priv->timeline);
	fluttr_spinner_spin (FLUTTR_SPINNER (priv->spinner), FALSE);
}

static gboolean                 
on_thread_abort_idle (FluttrList *list)
{
        g_return_val_if_fail (FLUTTR_IS_LIST (list), FALSE);
        
        close_message_window (list);

	g_signal_emit (list, _list_signals[ERROR], 0, "Aborted");	

        return FALSE;
}

static gboolean                 
on_thread_ok_idle (FluttrList *list)
{
        FluttrListPrivate *priv;
        
        g_return_val_if_fail (FLUTTR_IS_LIST (list), FALSE);
        priv = FLUTTR_LIST_GET_PRIVATE(list);
        
        close_message_window (list);
                
        g_signal_emit (list, _list_signals[SUCCESSFUL], 0, priv->worker);

        return FALSE;
}

static gboolean                 
on_thread_error_idle (FluttrList *list)
{
        FluttrListPrivate *priv;
        gchar *error = NULL;
        
        g_return_val_if_fail (FLUTTR_IS_LIST (list), FALSE);
        priv = FLUTTR_LIST_GET_PRIVATE(list);
        
        close_message_window (list);
        
        /* Get the actual error */
        g_object_get (G_OBJECT (priv->worker), "error", &error, NULL);
        if (error == NULL) {
                error = g_strdup_printf (gettext ("Internal error. "));
                g_warning ("No error set on worker!");
        }
	g_signal_emit (list, _list_signals[ERROR], 0, error);

        g_free (error);

        return FALSE;
}

/* Copy the new message and start the fade effect if not already started */
static gboolean                 
on_thread_msg_change_idle (FluttrList *list)
{
        FluttrListPrivate *priv;
        gchar *msg;
        
        g_return_val_if_fail (FLUTTR_IS_LIST (list), FALSE);
        priv = FLUTTR_LIST_GET_PRIVATE(list);
        
        /* Get the new message */
        g_object_get (G_OBJECT (priv->worker), "message", &msg, NULL);
        if (msg != NULL) {
                g_print ("%s", msg);
        }
        priv->msg = g_strdup (msg);
        
        if (clutter_timeline_is_playing (priv->text_time))
        	;//clutter_timeline_rewind (priv->text_time);
       
        else
        	clutter_timeline_start (priv->text_time);

	return FALSE;
}


/* This function does th emain work of creating and configuring the worker 
   thread. the majority of this code is taken from
   NFlick the n800 Flickr photo browser by MDK (see: README) */
void
fluttr_list_go (FluttrList *list)
{
	FluttrListPrivate *priv;
	NFlickWorker *worker;
        NFlickWorkerStatus status;
        
       	g_return_if_fail (FLUTTR_IS_LIST (list));
	priv = FLUTTR_LIST_GET_PRIVATE(list);
	
	/* Set to opaque and start spinner */
	fluttr_spinner_spin (FLUTTR_SPINNER (priv->spinner), TRUE);
	clutter_timeline_start (priv->timeline);
	clutter_actor_set_opacity (CLUTTER_ACTOR (list), 255);		
	
	/* Create the worker */
        worker = (NFlickWorker*)nflick_set_list_worker_new (priv->usernsid,
        						    priv->token);
        
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
        nflick_worker_set_custom_data (worker, list);
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
fluttr_list_alpha_func (ClutterBehaviour *behave,
		       	      guint		alpha_value,
		              gpointer		data)
{
	FluttrListPrivate *priv;
	gfloat scale;
	gfloat factor;
	guint width, height;	
	gint x, y;
	
       	g_return_if_fail (FLUTTR_IS_LIST (data));
	priv = FLUTTR_LIST_GET_PRIVATE(data);
	
	factor = (gfloat)alpha_value / CLUTTER_ALPHA_MAX_ALPHA;
	
	if (priv->popping) 
		scale = factor;
	else
		scale = 1.0 - factor;
	
	clutter_actor_set_scale (CLUTTER_ACTOR (priv->group), scale, scale);
	
	/* Set new size */
	clutter_actor_get_size (CLUTTER_ACTOR (priv->group), &width, &height);
	width *= scale;
	height *= scale;
	
	x = (CLUTTER_STAGE_WIDTH () /2) - (width /2);
	y = (CLUTTER_STAGE_HEIGHT () /2) - (height /2);
	
	g_object_set (G_OBJECT (priv->group), 
		      "y", y, 
		      "x", x,
		      NULL);
	clutter_actor_set_opacity (CLUTTER_ACTOR (priv->group), 255 * scale);
	clutter_actor_set_opacity (CLUTTER_ACTOR (priv->text), 255 * scale);
	
	if (CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR(data)))
		clutter_actor_queue_redraw (CLUTTER_ACTOR(data));	
}

/* Fade out text, change text, then fade in, all within one play of the timeline
   just to keep things interesting :) */
static void
fluttr_list_text_alpha_func (ClutterBehaviour *behave,
		       	     guint		alpha_value,
		             gpointer		data)
{
	FluttrListPrivate *priv;
	gfloat factor;
	
       	g_return_if_fail (FLUTTR_IS_LIST (data));
	priv = FLUTTR_LIST_GET_PRIVATE(data);
	
	factor = (gfloat) alpha_value / CLUTTER_ALPHA_MAX_ALPHA;
	
	if (priv->msg != NULL && factor > 0.5) {
		gchar *text = priv->msg;
		gint x, y;
		clutter_label_set_text (CLUTTER_LABEL (priv->text),
					text);
	
		/* Calculate the new position */
		x = (CLUTTER_STAGE_WIDTH () /2) 
				- (clutter_actor_get_width (priv->text)/2);
		y = (CLUTTER_STAGE_HEIGHT () /20) * 18;
			clutter_actor_set_position (priv->text, x, y);
			clutter_actor_set_position (priv->text, x, y);
		g_free (priv->msg);
		priv->msg = NULL;
		
	}
	if (factor < 0.5) {
		factor *= 2;
	} else {
		factor -= 0.5;
		factor /= 0.5;
	}
	clutter_actor_set_opacity (CLUTTER_ACTOR (priv->text), 255 * factor);
	/*clutter_actor_rotate_x (CLUTTER_ACTOR (priv->text), 
				360 * factor,
			clutter_actor_get_height (CLUTTER_ACTOR (priv->text))/2,
			0);
	*/
	if (CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR(data)))
		clutter_actor_queue_redraw (CLUTTER_ACTOR(data));	
} 	
		
/* GObject Stuff */

static void
fluttr_list_set_property (GObject      *object, 
			  guint         prop_id,
			  const GValue *value, 
			  GParamSpec   *pspec)
{
	FluttrListPrivate *priv;

	g_return_if_fail (FLUTTR_IS_LIST (object));
	priv = FLUTTR_LIST_GET_PRIVATE(object);

	switch (prop_id) {
		case PROP_MINI_TOKEN:
			if (priv->mini_token != NULL)
				g_free (priv->mini_token);
			priv->mini_token =g_strdup (g_value_get_string (value));
			break;
		case PROP_USERNAME:
			if (priv->username != NULL)
				g_free (priv->username);
			priv->username =g_strdup (g_value_get_string (value));
			break;

		case PROP_FULLNAME:
			if (priv->fullname != NULL)
				g_free (priv->fullname);
			priv->fullname =g_strdup (g_value_get_string (value));
			break;
		
		case PROP_TOKEN:
			if (priv->token != NULL)
				g_free (priv->token);
			priv->token =g_strdup (g_value_get_string (value));
			break;
		
		case PROP_USERNSID:
			if (priv->usernsid != NULL)
				g_free (priv->usernsid);
			priv->usernsid =g_strdup (g_value_get_string (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, 
							   pspec);
			break;
	}
}

static void
fluttr_list_get_property (GObject    *object, 
			  guint       prop_id,
			  GValue     *value, 
			  GParamSpec *pspec)
{
	FluttrListPrivate *priv;
	
	g_return_if_fail (FLUTTR_IS_LIST (object));
	priv = FLUTTR_LIST_GET_PRIVATE(object);

	switch (prop_id) {
		case PROP_MINI_TOKEN:
			g_value_set_string (value, priv->mini_token);
			break;
		case PROP_USERNAME:
			g_value_set_string (value, priv->username);
			break;

		case PROP_FULLNAME:
			g_value_set_string (value, priv->fullname);
			break;
		
		case PROP_TOKEN:
			g_value_set_string (value, priv->token);
			break;
		
		case PROP_USERNSID:
			g_value_set_string (value, priv->usernsid);
			break;			
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id,
							   pspec);
		break;
	} 
}

static void
fluttr_list_paint (ClutterActor *actor)
{
	FluttrList        *list;
	FluttrListPrivate *priv;

	list = FLUTTR_LIST(actor);

	priv = FLUTTR_LIST_GET_PRIVATE(list);

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
fluttr_list_dispose (GObject *object)
{
	FluttrList         *self = FLUTTR_LIST(object);
	FluttrListPrivate  *priv;  

	priv = self->priv;
  
	G_OBJECT_CLASS (fluttr_list_parent_class)->dispose (object);
}

static void 
fluttr_list_finalize (GObject *object)
{
	G_OBJECT_CLASS (fluttr_list_parent_class)->finalize (object);
}

static void
fluttr_list_class_init (FluttrListClass *klass)
{
	GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
	ClutterActorClass   *actor_class = CLUTTER_ACTOR_CLASS (klass);
	ClutterActorClass   *parent_class; 

	parent_class = CLUTTER_ACTOR_CLASS (fluttr_list_parent_class);

	actor_class->paint           = fluttr_list_paint;
	
	gobject_class->finalize     = fluttr_list_finalize;
	gobject_class->dispose      = fluttr_list_dispose;
	gobject_class->get_property = fluttr_list_get_property;
	gobject_class->set_property = fluttr_list_set_property;	

	g_type_class_add_private (gobject_class, sizeof (FluttrListPrivate));
	
	/* Class properties */
	g_object_class_install_property 
		(gobject_class,
		 PROP_MINI_TOKEN,
		 g_param_spec_string ("mini-token",
		 "Mini Token",
		 "The Flickr mini-token",
		 NULL,
		 G_PARAM_CONSTRUCT|G_PARAM_READWRITE));	
		 
	g_object_class_install_property 
		(gobject_class,
		 PROP_USERNAME,
		 g_param_spec_string ("username",
		 "Username",
		 "The Flickr username",
		 NULL,
		 G_PARAM_CONSTRUCT|G_PARAM_READWRITE));			 
		 
	g_object_class_install_property 
		(gobject_class,
		 PROP_FULLNAME,
		 g_param_spec_string ("fullname",
		 "Fullname",
		 "The users full name",
		 NULL,
		 G_PARAM_CONSTRUCT|G_PARAM_READWRITE));	
		 
	g_object_class_install_property 
		(gobject_class,
		 PROP_TOKEN,
		 g_param_spec_string ("token",
		 "Token",
		 "The Flickr token",
		 NULL,
		 G_PARAM_CONSTRUCT|G_PARAM_READWRITE));	
		 
	g_object_class_install_property 
		(gobject_class,
		 PROP_USERNSID,
		 g_param_spec_string ("usernsid",
		 "Usernsid",
		 "The Flickr usernsid",
		 NULL,
		 G_PARAM_CONSTRUCT|G_PARAM_READWRITE));			 		 		 

	/* Class signals */
	_list_signals[SUCCESSFUL] =
		g_signal_new ("successful",
			     G_OBJECT_CLASS_TYPE (gobject_class),
			     G_SIGNAL_RUN_FIRST,
			     G_STRUCT_OFFSET (FluttrListClass, successful),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__OBJECT,
			     G_TYPE_NONE, 1, NFLICK_TYPE_WORKER);
			     
	_list_signals[ERROR] =
		g_signal_new ("error",
			     G_OBJECT_CLASS_TYPE (gobject_class),
			     G_SIGNAL_RUN_FIRST,
			     G_STRUCT_OFFSET (FluttrListClass, error),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__STRING,
			     G_TYPE_NONE, 1, G_TYPE_STRING);			     

}

static void
fluttr_list_init (FluttrList *self)
{
	FluttrListPrivate *priv;
	gint size = CLUTTER_STAGE_HEIGHT ()/9;
	gint width, height;
	ClutterActor *message;
	GdkPixbuf *msg_buf = NULL;
	guint font_size;
	gchar *font;
	ClutterColor text_color = { 0xff, 0xff, 0xff, 0xff };	
	
	priv = FLUTTR_LIST_GET_PRIVATE (self);
	
	priv->mini_token = NULL;

	width = CLUTTER_STAGE_WIDTH ()/4;
	height = CLUTTER_STAGE_HEIGHT ()/4;
		
	/* Group */
	priv->group = clutter_group_new ();
	clutter_group_add (CLUTTER_GROUP (self),priv->group); 
	clutter_actor_set_size (priv->group, width, height);
	clutter_actor_set_position (priv->group, 
				    (CLUTTER_STAGE_WIDTH ()/2) - (width/2),
				    (CLUTTER_STAGE_HEIGHT ()/2) - (height/2));	
	
	/* message box */
	message = clutter_texture_new ();
	priv->message = message;
	msg_buf = gdk_pixbuf_new_from_file_at_scale (PKGDATADIR \
  						    	"/message.svg",
  						    width,
  						    height,
  						    FALSE,
  						    NULL);
	clutter_texture_set_pixbuf (CLUTTER_TEXTURE (message), msg_buf);
	clutter_group_add (CLUTTER_GROUP (priv->group),message); 
	clutter_actor_set_size (message, width, height);
	clutter_actor_set_position (message, -(width/2),-(height/2));
	
	
	/* Spinner */
	priv->spinner = fluttr_spinner_new ();
	clutter_group_add (CLUTTER_GROUP (priv->group), priv->spinner);
	clutter_actor_set_size (priv->spinner, size, size);
	clutter_actor_set_position (priv->spinner, -(size/2), -(size/2));
				    
	priv->timeline = clutter_timeline_new (40, 80);
	priv->alpha = clutter_alpha_new_full (priv->timeline,
					      alpha_sine_inc_func,
					      NULL, NULL);
	priv->behave = fluttr_behave_new (priv->alpha,
					  fluttr_list_alpha_func,
					  (gpointer)self);
	priv->popping = TRUE;
	
	/* This is the msg label */
	font_size = CLUTTER_STAGE_HEIGHT () /20;
	font = g_strdup_printf ("%s %d", FONT, font_size);
	
	priv->text = clutter_label_new_full (font, " ", &text_color);
	clutter_actor_set_opacity (priv->text, 0);
	clutter_label_set_line_wrap (CLUTTER_LABEL (priv->text), FALSE);
	clutter_group_add (CLUTTER_GROUP (self), priv->text);
	clutter_actor_set_size (priv->text, 
				CLUTTER_STAGE_WIDTH (),
			        font_size * 2);	
	
	priv->text_time = clutter_timeline_new (40, 50);
	priv->text_alpha = clutter_alpha_new_full (priv->text_time,
					      alpha_sine_inc_func,
					      NULL, NULL);
	priv->text_behave = fluttr_behave_new (priv->text_alpha,
					  fluttr_list_text_alpha_func,
					  (gpointer)self);	
	g_free (font);	
}

ClutterActor*
fluttr_list_new (void)
{
	ClutterGroup         *list;

	list = g_object_new (FLUTTR_TYPE_LIST, 
			     NULL);
	
	clutter_actor_set_opacity (CLUTTER_ACTOR (list), 0);
	
	return CLUTTER_ACTOR (list);
}

