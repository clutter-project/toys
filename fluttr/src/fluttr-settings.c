/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-hand.com>
 */

#include "fluttr-settings.h"


G_DEFINE_TYPE (FluttrSettings, fluttr_settings, G_TYPE_OBJECT);

#define FLUTTR_SETTINGS_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), FLUTTR_TYPE_SETTINGS, \
	FluttrSettingsPrivate))
	

struct _FluttrSettingsPrivate
{	
	gchar		*username;
	gchar		*fullname;
	gchar 		*token;
	gchar		*usernsid;
};

enum
{
	PROP_0,
	PROP_USERNAME,
	PROP_FULLNAME,
	PROP_TOKEN,
	PROP_USERNSID
};

static FluttrSettings* global_settings	= NULL;

/* GObject Stuff */

static void
fluttr_settings_set_property (GObject      *object, 
			  	 guint         prop_id,
			  	 const GValue *value, 
			  	 GParamSpec   *pspec)
{
	FluttrSettingsPrivate *priv;

	g_return_if_fail (FLUTTR_IS_SETTINGS (object));
	priv = FLUTTR_SETTINGS_GET_PRIVATE(object);

	switch (prop_id) {
		case PROP_USERNAME:
			priv->username = g_strdup (g_value_get_string (value));
			break;
		case PROP_FULLNAME:
			priv->fullname = g_strdup (g_value_get_string (value));
			break;	
	
		case PROP_TOKEN:
			priv->token = g_strdup (g_value_get_string (value));
			break;
		
		case PROP_USERNSID:
			priv->usernsid = g_strdup (g_value_get_string (value));
			break;	
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, 
							   pspec);
			break;
	}
}

static void
fluttr_settings_get_property (GObject    *object, 
			  guint       prop_id,
			  GValue     *value, 
			  GParamSpec *pspec)
{
	FluttrSettingsPrivate *priv;
	
	g_return_if_fail (FLUTTR_IS_SETTINGS (object));
	priv = FLUTTR_SETTINGS_GET_PRIVATE(object);

	switch (prop_id) {
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
fluttr_settings_dispose (GObject *object)
{
	FluttrSettings         *self = FLUTTR_SETTINGS(object);
	FluttrSettingsPrivate  *priv;  

	priv = self->priv;
  
	G_OBJECT_CLASS (fluttr_settings_parent_class)->dispose (object);
}

static void 
fluttr_settings_finalize (GObject *object)
{
	G_OBJECT_CLASS (fluttr_settings_parent_class)->finalize (object);
}

static void
fluttr_settings_class_init (FluttrSettingsClass *klass)
{
	GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->finalize     = fluttr_settings_finalize;
	gobject_class->dispose      = fluttr_settings_dispose;
	gobject_class->get_property = fluttr_settings_get_property;
	gobject_class->set_property = fluttr_settings_set_property;	

	g_type_class_add_private (gobject_class, 
				  sizeof (FluttrSettingsPrivate));
		
	/* Class properties */
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
		 "The Flickr fullname",
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
}

static void
fluttr_settings_init (FluttrSettings *self)
{
	FluttrSettingsPrivate *priv;
	priv = FLUTTR_SETTINGS_GET_PRIVATE (self);
}

FluttrSettings*
fluttr_settings_new (void)
{
	global_settings = (FluttrSettings*) g_object_new (FLUTTR_TYPE_SETTINGS, 
			       		    NULL);
	
	return global_settings;
}

FluttrSettings*
fluttr_settings_get_default (void)
{
	if (global_settings == NULL)
		global_settings = fluttr_settings_new ();
	
	return global_settings;
}

