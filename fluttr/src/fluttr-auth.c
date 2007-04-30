/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-hand.com>
 */

#include "fluttr-auth.h"

#include "nflick-worker.h"
#include "nflick-auth-worker.h"

G_DEFINE_TYPE (FluttrAuth, fluttr_auth, CLUTTER_TYPE_GROUP);

#define FLUTTR_AUTH_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
	FLUTTR_TYPE_AUTH, \
	FluttrAuthPrivate))
	
#define FONT "DejaVu Sans Book"


struct _FluttrAuthPrivate
{
	gchar 			*mini_token;
	gchar 			*username;
	gchar			*fullname;
	gchar			*token;
	gchar			*usernsid;
	
	NFlickWorker		*worker;

	GdkPixbuf		*logo;
	ClutterActor		*text;
	ClutterActor		*throbber;
	
	ClutterTimeline		*timeline;
	ClutterAlpha		*alpha;
	ClutterBehaviour 	*behave;
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

static guint _auth_signals[LAST_SIGNAL] = { 0 };

static gboolean                 
on_thread_abort_idle (FluttrAuth *auth)
{
        g_return_val_if_fail (FLUTTR_IS_AUTH (auth), FALSE);

	g_signal_emit (auth, _auth_signals[ERROR], 0, "Aborted");	

        return FALSE;
}

static gboolean                 
on_thread_ok_idle (FluttrAuth *auth)
{
        FluttrAuthPrivate *priv;
        
        g_return_val_if_fail (FLUTTR_IS_AUTH (auth), FALSE);
        priv = FLUTTR_AUTH_GET_PRIVATE(auth);
        
        g_object_get (G_OBJECT (priv->worker), 
                      "username", &priv->username, 
                      "fullname", &priv->fullname, 
                      "token", &priv->token, 
                      "usernsid", &priv->usernsid, 
                      NULL);

        g_signal_emit (auth, _auth_signals[SUCCESSFUL], 0, "");

        return FALSE;
}

static gboolean                 
on_thread_error_idle (FluttrAuth *auth)
{
        FluttrAuthPrivate *priv;
        gchar *error = NULL;
        
        g_return_val_if_fail (FLUTTR_IS_AUTH (auth), FALSE);
        priv = FLUTTR_AUTH_GET_PRIVATE(auth);
        
        /* Get the actual error */
        g_object_get (G_OBJECT (priv->worker), "error", &error, NULL);
        if (error == NULL) {
                error = g_strdup_printf (gettext ("Internal error. "));
                g_warning ("No error set on worker!");
        }
	g_signal_emit (auth, _auth_signals[ERROR], 0, error);

        g_free (error);

        return FALSE;
}

static gboolean                 
on_thread_msg_change_idle (FluttrAuth *auth)
{
        FluttrAuthPrivate *priv;
        gchar *msg = NULL;
        
        g_return_val_if_fail (FLUTTR_IS_AUTH (auth), FALSE);
        priv = FLUTTR_AUTH_GET_PRIVATE(auth);
        
        /* Get the new message */
        g_object_get (G_OBJECT (priv->worker), "message", &msg, NULL);
        if (msg != NULL) {
                g_print ("%s", msg);
        }
        
        g_free (msg);

        return FALSE;
}


/* This function does th emain work of creating and configuring the worker 
   thread. the majority of this code is taken from
   NFlick the n800 Flickr photo browser by MDK (see: README) */
void
fluttr_auth_go (FluttrAuth *auth)
{
	FluttrAuthPrivate *priv;
	NFlickWorker *worker;
        NFlickWorkerStatus status;
        
       	g_return_if_fail (FLUTTR_IS_AUTH (auth));
	priv = FLUTTR_AUTH_GET_PRIVATE(auth);
	
	/* Create the worker */
        worker = (NFlickWorker*)nflick_auth_worker_new (priv->mini_token);
        
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
        nflick_worker_set_custom_data (worker, auth);
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
fluttr_auth_alpha_func (ClutterBehaviour *behave,
		       	      guint		alpha_value,
		              gpointer		data)
{
	return;
} 		
		
/* GObject Stuff */

static void
fluttr_auth_set_property (GObject      *object, 
			  guint         prop_id,
			  const GValue *value, 
			  GParamSpec   *pspec)
{
	FluttrAuthPrivate *priv;

	g_return_if_fail (FLUTTR_IS_AUTH (object));
	priv = FLUTTR_AUTH_GET_PRIVATE(object);

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
fluttr_auth_get_property (GObject    *object, 
			  guint       prop_id,
			  GValue     *value, 
			  GParamSpec *pspec)
{
	FluttrAuthPrivate *priv;
	
	g_return_if_fail (FLUTTR_IS_AUTH (object));
	priv = FLUTTR_AUTH_GET_PRIVATE(object);

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
fluttr_auth_paint (ClutterActor *actor)
{
	FluttrAuth        *auth;
	FluttrAuthPrivate *priv;

	auth = FLUTTR_AUTH(actor);

	priv = FLUTTR_AUTH_GET_PRIVATE(auth);

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
fluttr_auth_dispose (GObject *object)
{
	FluttrAuth         *self = FLUTTR_AUTH(object);
	FluttrAuthPrivate  *priv;  

	priv = self->priv;
  
	G_OBJECT_CLASS (fluttr_auth_parent_class)->dispose (object);
}

static void 
fluttr_auth_finalize (GObject *object)
{
	G_OBJECT_CLASS (fluttr_auth_parent_class)->finalize (object);
}

static void
fluttr_auth_class_init (FluttrAuthClass *klass)
{
	GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
	ClutterActorClass   *actor_class = CLUTTER_ACTOR_CLASS (klass);
	ClutterActorClass   *parent_class; 

	parent_class = CLUTTER_ACTOR_CLASS (fluttr_auth_parent_class);

	actor_class->paint           = fluttr_auth_paint;
	
	gobject_class->finalize     = fluttr_auth_finalize;
	gobject_class->dispose      = fluttr_auth_dispose;
	gobject_class->get_property = fluttr_auth_get_property;
	gobject_class->set_property = fluttr_auth_set_property;	

	g_type_class_add_private (gobject_class, sizeof (FluttrAuthPrivate));
	
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
	_auth_signals[SUCCESSFUL] =
		g_signal_new ("successful",
			     G_OBJECT_CLASS_TYPE (gobject_class),
			     G_SIGNAL_RUN_FIRST,
			     G_STRUCT_OFFSET (FluttrAuthClass, successful),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__STRING,
			     G_TYPE_NONE, 1, G_TYPE_STRING);
			     
	_auth_signals[ERROR] =
		g_signal_new ("error",
			     G_OBJECT_CLASS_TYPE (gobject_class),
			     G_SIGNAL_RUN_FIRST,
			     G_STRUCT_OFFSET (FluttrAuthClass, error),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__STRING,
			     G_TYPE_NONE, 1, G_TYPE_STRING);			     

}

static void
fluttr_auth_init (FluttrAuth *self)
{
	FluttrAuthPrivate *priv;
	priv = FLUTTR_AUTH_GET_PRIVATE (self);
	
	priv->mini_token = NULL;
}

ClutterActor*
fluttr_auth_new (const char *mini_token)
{
	ClutterGroup         *auth;

	auth = g_object_new (FLUTTR_TYPE_AUTH, 
			     "mini-token", mini_token,
			      NULL);
	if (0) fluttr_auth_alpha_func (NULL, 0, NULL);
	return CLUTTER_ACTOR (auth);
}

