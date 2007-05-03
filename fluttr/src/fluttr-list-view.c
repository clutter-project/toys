/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-hand.com>
 */

#include "fluttr-list-view.h"

G_DEFINE_TYPE (FluttrListView, fluttr_list_view, CLUTTER_TYPE_GROUP);

#define FLUTTR_LIST_VIEW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
	FLUTTR_TYPE_LIST_VIEW, \
	FluttrListViewPrivate))
	
struct _FluttrListViewPrivate
{
	FluttrLibrary		*library;
	
	gint 			 active_photo;
	gint			 active_col;
};

enum
{
	PROP_0,
	PROP_LIBRARY
};

#define N_COLS 5

static ClutterGroupClass	*parent_class = NULL;

FluttrPhoto*
fluttr_list_view_get_active (FluttrListView *list_view)
{
	FluttrListViewPrivate *priv;
	ClutterActor *actor;
		
	g_return_if_fail (FLUTTR_IS_LIST_VIEW (list_view));
	priv = FLUTTR_LIST_VIEW_GET_PRIVATE(list_view);
	
	actor = clutter_group_get_nth_child (CLUTTER_GROUP (list_view),
					     priv->active_photo);
	
	return FLUTTR_PHOTO (actor);
}

static void
_pixbuf_loaded (FluttrPhoto *photo, gchar *null, FluttrLibraryRow *row)
{
	GdkPixbuf *pixbuf = NULL;
	g_object_get (G_OBJECT (photo), "pixbuf", &pixbuf, NULL);
	g_object_set (G_OBJECT (row), "pixbuf", pixbuf, NULL);
}

void 
fluttr_list_view_advance (FluttrListView *list_view, gint n)
{
	FluttrListViewPrivate *priv;
	gint len;
	gint i = 0;
	FluttrLibraryRow *lrow = NULL;
	ClutterActor *photo = NULL;
	ClutterActor *active = NULL;
	guint size = fluttr_photo_get_default_size ();
	gint x1;
	gint active_row = 0;
	gint offset = size/2;
	gint x_center = CLUTTER_STAGE_WIDTH () /2;
	gint y_center = CLUTTER_STAGE_HEIGHT ()/2;
	gint padding = size /4;
		
	g_return_if_fail (FLUTTR_IS_LIST_VIEW (list_view));
	priv = FLUTTR_LIST_VIEW_GET_PRIVATE(list_view);

	len = fluttr_library_row_count (priv->library);
	
	/* Make sure we are within the bounds of the number of albums */
	priv->active_photo+= n;
	if (priv->active_photo < 0) {
		priv->active_photo  = 0;
	} else if (priv->active_photo > len-1) {
		priv->active_photo = len -1;
	} else
		;
	/* Find the magic row */	
	active_row = 0;
	gint row = 0;
	gint col = 0;
	
	for (i = 0; i < len; i++) {
		if (i == priv->active_photo) {
			active_row = row;
			break;
		}
		col++;
		if (col > (N_COLS-1)) {
			col = 0;
			row++;
		}
	}
	
	/* Figure out the base x value */
	x1 = ((size) * N_COLS ) + (padding*(N_COLS-1));
	x1 = (CLUTTER_STAGE_WIDTH ()/2)-(x1/2);
	
	/* Iterate through actors, calculating their new x positions, and make
	   sure they are on the right place (left, right or center)
	*/
	col = 0;
	row = 0;
	gint less = priv->active_photo - 10;
	gint more = priv->active_photo + 12;
	
	offset = -1 * ((size) + padding) * active_row;
	
	for (i = 0; i < len; i++) {
		lrow = fluttr_library_get_library_row (priv->library, i);
		photo = NULL;
		g_object_get (G_OBJECT (lrow), "photo", &photo, NULL);
		 
		if (!photo) {
		 	gchar *id;
		 	g_object_get(G_OBJECT (lrow), "id", &id, NULL);
		 	
		 	photo = fluttr_photo_new ();
		 	clutter_group_add (CLUTTER_GROUP (list_view), photo);
		 	clutter_actor_set_size (photo, size, size);
		 	clutter_actor_set_position (photo, x_center, y_center);
		 	clutter_actor_show_all (CLUTTER_ACTOR (photo	));
		 	
		 	if (photo) {
		 		g_object_set (G_OBJECT (lrow), "photo", 
		 			      photo, NULL);
				g_object_set (G_OBJECT (photo),"photoid", 
					      id, NULL);
			}
			
				
		}
		gint x = x1 + (col * (size + padding));
		gint y = offset;
		fluttr_photo_update_position (FLUTTR_PHOTO (photo), x, y);
		
		col++;
		if (col > (N_COLS-1)) {
			col = 0;
			row++;
			offset += size + padding;
		}	
		if ((i > less) && (i < more)) {
			GdkPixbuf *pixbuf = NULL;
			g_object_get (G_OBJECT (lrow), "pixbuf", &pixbuf, NULL);
			
			if (pixbuf != NULL) {
				g_object_set (G_OBJECT (photo), "pixbuf",
					      pixbuf, NULL);
				/*g_print ("Got pixbuf\n");*/
			} else {
				fluttr_photo_fetch_pixbuf (FLUTTR_PHOTO (photo));
				g_signal_connect (G_OBJECT (photo), 
						  "pixbuf-loaded",
				  		  G_CALLBACK (_pixbuf_loaded),
				  		  lrow);
			}
		}
	}
	
}

void
fluttr_list_view_advance_row (FluttrListView *list_view, gint n)
{
	fluttr_list_view_advance (list_view, (N_COLS * n));
}

void
fluttr_list_view_advance_col (FluttrListView *list_view, gint n)
{
	fluttr_list_view_advance (list_view, n);
}

/* GObject Stuff */

static void
fluttr_list_view_set_property (GObject      *object, 
			  guint         prop_id,
			  const GValue *value, 
			  GParamSpec   *pspec)
{
	FluttrListViewPrivate *priv;

	g_return_if_fail (FLUTTR_IS_LIST_VIEW (object));
	priv = FLUTTR_LIST_VIEW_GET_PRIVATE(object);

	switch (prop_id) {
		case PROP_LIBRARY:
			if (priv->library != NULL)
				g_object_unref (priv->library);
			priv->library =g_value_get_object (value);
			g_object_ref (priv->library);
			/* Connect to the library signals */
			break;
		
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, 
							   pspec);
			break;
	}
}

static void
fluttr_list_view_get_property (GObject    *object, 
			  guint       prop_id,
			  GValue     *value, 
			  GParamSpec *pspec)
{
	FluttrListViewPrivate *priv;
	
	g_return_if_fail (FLUTTR_IS_LIST_VIEW (object));
	priv = FLUTTR_LIST_VIEW_GET_PRIVATE(object);

	switch (prop_id) {
		case PROP_LIBRARY:
			g_value_set_object (value, priv->library);
			break;			
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id,
							   pspec);
		break;
	} 
}

static void
fluttr_list_view_paint (ClutterActor *actor)
{
	FluttrListView        *list;
	FluttrListViewPrivate *priv;

	list = FLUTTR_LIST_VIEW(actor);

	priv = FLUTTR_LIST_VIEW_GET_PRIVATE(list);

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
fluttr_list_view_dispose (GObject *object)
{
	FluttrListView         *self = FLUTTR_LIST_VIEW(object);
	FluttrListViewPrivate  *priv;  

	priv = self->priv;
  
	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void 
fluttr_list_view_finalize (GObject *object)
{
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
fluttr_list_view_class_init (FluttrListViewClass *klass)
{
	GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
	ClutterActorClass   *actor_class = CLUTTER_ACTOR_CLASS (klass);
	
	parent_class = CLUTTER_GROUP_CLASS (klass);

	actor_class->paint           = fluttr_list_view_paint;
	
	gobject_class->finalize     = fluttr_list_view_finalize;
	gobject_class->dispose      = fluttr_list_view_dispose;
	gobject_class->get_property = fluttr_list_view_get_property;
	gobject_class->set_property = fluttr_list_view_set_property;	

	g_type_class_add_private (gobject_class, sizeof (FluttrListViewPrivate));
		
	/* Class properties */
	g_object_class_install_property 
		(gobject_class,
		 PROP_LIBRARY,
		 g_param_spec_object ("library",
		 "Library",
		 "The underlying Fluttr Library",
		 FLUTTR_TYPE_LIBRARY,
		 G_PARAM_CONSTRUCT|G_PARAM_READWRITE));	

}

static void
fluttr_list_view_init (FluttrListView *self)
{
	FluttrListViewPrivate *priv;

	priv = FLUTTR_LIST_VIEW_GET_PRIVATE (self);
	
	priv->active_photo = 0;
	priv->active_col = 0;
	
}

ClutterActor*
fluttr_list_view_new (FluttrLibrary *library)
{
	ClutterGroup         *list_view;

	list_view = g_object_new (FLUTTR_TYPE_LIST_VIEW, 
			          "library", library,
			          NULL);
	
	return CLUTTER_ACTOR (list_view);
}

