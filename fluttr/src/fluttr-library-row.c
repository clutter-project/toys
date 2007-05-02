/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-hand.com>
 */

#include "fluttr-library-row.h"

#include "nflick-worker.h"

G_DEFINE_TYPE (FluttrLibraryRow, fluttr_library_row, G_TYPE_OBJECT);

#define FLUTTR_LIBRARY_ROW_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), FLUTTR_TYPE_LIBRARY_ROW, \
	FluttrLibraryRowPrivate))
	
#define FONT "DejaVu Sans Book"


struct _FluttrLibraryRowPrivate
{	
	gchar			*id;
	gchar			*name;
	NFlickPhotoSet		*set;
	
	ClutterActor		*photo;
};

enum
{
	PROP_0,
	PROP_ID,
	PROP_NAME,
	PROP_PHOTO,
	PROP_SET
};

/* GObject Stuff */

static void
fluttr_library_row_set_property (GObject      *object, 
			  	 guint         prop_id,
			  	 const GValue *value, 
			  	 GParamSpec   *pspec)
{
	FluttrLibraryRowPrivate *priv;

	g_return_if_fail (FLUTTR_IS_LIBRARY_ROW (object));
	priv = FLUTTR_LIBRARY_ROW_GET_PRIVATE(object);

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
		
		case PROP_PHOTO:
			priv->photo = g_value_get_object (value);
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
fluttr_library_row_get_property (GObject    *object, 
			  guint       prop_id,
			  GValue     *value, 
			  GParamSpec *pspec)
{
	FluttrLibraryRowPrivate *priv;
	
	g_return_if_fail (FLUTTR_IS_LIBRARY_ROW (object));
	priv = FLUTTR_LIBRARY_ROW_GET_PRIVATE(object);

	switch (prop_id) {
		case PROP_ID:
			g_value_set_string (value, priv->id);
			break;
		
		case PROP_NAME:
			g_value_set_string (value, priv->name);
			break;
			
		case PROP_PHOTO:
			g_value_set_object (value, G_OBJECT (priv->photo));
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
fluttr_library_row_dispose (GObject *object)
{
	FluttrLibraryRow         *self = FLUTTR_LIBRARY_ROW(object);
	FluttrLibraryRowPrivate  *priv;  

	priv = self->priv;
  
	G_OBJECT_CLASS (fluttr_library_row_parent_class)->dispose (object);
}

static void 
fluttr_library_row_finalize (GObject *object)
{
	G_OBJECT_CLASS (fluttr_library_row_parent_class)->finalize (object);
}

static void
fluttr_library_row_class_init (FluttrLibraryRowClass *klass)
{
	GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->finalize     = fluttr_library_row_finalize;
	gobject_class->dispose      = fluttr_library_row_dispose;
	gobject_class->get_property = fluttr_library_row_get_property;
	gobject_class->set_property = fluttr_library_row_set_property;	

	g_type_class_add_private (gobject_class, 
				  sizeof (FluttrLibraryRowPrivate));
		
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
		 PROP_PHOTO,
		 g_param_spec_object ("photo",
		 "Photo",
		 "The FluttrPhoto actor",
		 CLUTTER_TYPE_ACTOR,
		 G_PARAM_CONSTRUCT|G_PARAM_READWRITE));			 
		 	
	g_object_class_install_property 
		(gobject_class,
		 PROP_SET,
		 g_param_spec_object ("set",
		 "Set",
		 "The NFlick photo set",
		 NFLICK_TYPE_PHOTO_SET,
		 G_PARAM_CONSTRUCT|G_PARAM_READWRITE));	
}

static void
fluttr_library_row_init (FluttrLibraryRow *self)
{
	FluttrLibraryRowPrivate *priv;
	priv = FLUTTR_LIBRARY_ROW_GET_PRIVATE (self);
	
}

FluttrLibraryRow*
fluttr_library_row_new (gchar *id, gchar *name, NFlickPhotoSet *set)
{
	GObject         *row;

	row = g_object_new (FLUTTR_TYPE_LIBRARY_ROW, 
			    "id", id,
			    "name", name,
			    "set", set,
			    NULL);

	return FLUTTR_LIBRARY_ROW(row);
}

