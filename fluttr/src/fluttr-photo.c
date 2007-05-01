/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-hand.com>
 */

#include "fluttr-photo.h"

#include "nflick-worker.h"

G_DEFINE_TYPE (FluttrPhoto, fluttr_photo, CLUTTER_TYPE_GROUP);

#define FLUTTR_PHOTO_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
	FLUTTR_TYPE_PHOTO, \
	FluttrPhotoPrivate))
	
#define FONT "DejaVu Sans Book"


struct _FluttrPhotoPrivate
{
	gchar			*id;
	gchar			*name;
	gchar 			*mini_token;
	gchar 			*username;
	gchar			*fullname;
	gchar			*token;
	gchar			*usernsid;
	NFlickPhotoSet		*set;
	
	NFlickWorker		*worker;

	GdkPixbuf		*pixbuf;
	
	ClutterTimeline		*timeline;
	ClutterAlpha		*alpha;
	ClutterBehaviour 	*behave;
};

enum
{
	PROP_0,
	PROP_ID,
	PROP_NAME,
	PROP_MINI_TOKEN,
	PROP_USERNAME,
	PROP_FULLNAME,
	PROP_TOKEN,
	PROP_USERNSID,
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
        
        g_return_val_if_fail (FLUTTR_IS_PHOTO (photo), FALSE);
        priv = FLUTTR_PHOTO_GET_PRIVATE(photo);
        
        g_object_get (G_OBJECT (priv->worker), 
                      "username", &priv->username, 
                      "fullname", &priv->fullname, 
                      "token", &priv->token, 
                      "usernsid", &priv->usernsid, 
                      NULL);

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
	;
}


/* Slide in or out the notification popp, depending on priv->pop_visible */
static void
fluttr_photo_alpha_func (ClutterBehaviour *behave,
		       	      guint		alpha_value,
		              gpointer		data)
{
	return;
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
			if (priv->id != NULL)
				g_free (priv->id);
			priv->id = g_strdup (g_value_get_string (value));
			break;
		case PROP_NAME:
			if (priv->name != NULL)
				g_free (priv->name);
			priv->name =g_strdup (g_value_get_string (value));
			break;	
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
			g_value_set_string (value, priv->id);
			break;
		
		case PROP_NAME:
			g_value_set_string (value, priv->name);
			
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
		 g_param_spec_string ("id",
		 "ID",
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
		 "The pixbuf of the photo",
		 NULL,
		 G_PARAM_CONSTRUCT|G_PARAM_READWRITE));		

	g_object_class_install_property 
		(gobject_class,
		 PROP_USERNSID,
		 g_param_spec_object ("pixbuf",
		 "Pixbuf",
		 "The Flickr usernsid",
		 GDK_TYPE_PIXBUF,
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
	priv = FLUTTR_PHOTO_GET_PRIVATE (self);
	
	priv->mini_token = NULL;
}

ClutterActor*
fluttr_photo_new (const char *mini_token)
{
	ClutterGroup         *photo;

	photo = g_object_new (FLUTTR_TYPE_PHOTO, 
			     "mini-token", mini_token,
			      NULL);
	if (0) fluttr_photo_alpha_func (NULL, 0, NULL);
	return CLUTTER_ACTOR (photo);
}

