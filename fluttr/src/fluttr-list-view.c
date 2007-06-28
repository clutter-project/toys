/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-hand.com>
 */

#include <GL/gl.h>

#include "fluttr-list-view.h"

G_DEFINE_TYPE (FluttrListView, fluttr_list_view, CLUTTER_TYPE_GROUP);

#define FLUTTR_LIST_VIEW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
	FLUTTR_TYPE_LIST_VIEW, \
	FluttrListViewPrivate))
	
struct _FluttrListViewPrivate
{
	FluttrLibrary		*library;
	FluttrSet		*set;
	GList			*photos;
	
	gint 			 active_photo;
	ClutterActor		*active_actor;
	gint			 active_col;
	
	gint                     n_cols;
};

enum
{
	PROP_0,
	PROP_LIBRARY,
	PROP_SET,
	PROP_COLS
};

static ClutterGroupClass	*parent_class = NULL;

FluttrPhoto*
fluttr_list_view_get_active (FluttrListView *list_view)
{
	FluttrListViewPrivate *priv;
		
	g_return_val_if_fail (FLUTTR_IS_LIST_VIEW (list_view), NULL);
	priv = FLUTTR_LIST_VIEW_GET_PRIVATE(list_view);
	
	return FLUTTR_PHOTO (priv->active_actor);
}

void 
fluttr_list_view_advance (FluttrListView *list_view, gint n)
{
	FluttrListViewPrivate *priv;
	gint len;
	gint i = 0;
	ClutterActor *photo = NULL;
	guint width = fluttr_photo_get_default_width ();
	guint height = fluttr_photo_get_default_height ();
	ClutterActor *stage = clutter_stage_get_default ();
	gint stage_height;
	gint min = -1 * fluttr_photo_get_default_height ();
	gint x1;
	gint active_row = 0;
	gint offset = height/2;
	gint padding = width /6;
		
	g_return_if_fail (FLUTTR_IS_LIST_VIEW (list_view));
	priv = FLUTTR_LIST_VIEW_GET_PRIVATE(list_view);

	len = g_list_length (priv->photos);
	g_object_get (G_OBJECT (stage), "height", &stage_height, NULL);
	stage_height += fluttr_photo_get_default_height ();
	
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
		if (col > (priv->n_cols-1)) {
			col = 0;
			row++;
		}
	}
	
	/* Figure out the base x value */
	x1 = ((width) * priv->n_cols ) + (padding*(priv->n_cols-1));
	x1 = (CLUTTER_STAGE_WIDTH ()/2)-(x1/2);
	
	/* Iterate through actors, calculating their new x positions, and make
	   sure they are on the right place (left, right or center) */
	col = 0;
	row = 0;
	gint less = priv->active_photo - (priv->n_cols * 2);
	gint more = priv->active_photo + (priv->n_cols * 3);
	
	offset = -1 * ((height) + padding) * active_row;
	offset += (CLUTTER_STAGE_HEIGHT () /2) - (height/2);
	
	for (i = 0; i < len; i++) {
		photo = (ClutterActor*)g_list_nth_data (priv->photos, i);
		 
		gint x = x1 + (col * (width + padding));
		gint y = offset;
		if (y > stage_height)
		        y = stage_height;
		else if (y < min)
		        y = min;
		
		fluttr_photo_update_position (FLUTTR_PHOTO (photo), x, y);
		
		col++;
		if (col > (priv->n_cols-1)) {
			col = 0;
			row++;
			offset += height + padding;
		}	
		if ((i > less) && (i < more)) {
			GdkPixbuf *pixbuf = NULL;
			g_object_get (G_OBJECT (photo), 
				      "pixbuf", &pixbuf, NULL);
			
			if (!pixbuf) {
				fluttr_photo_fetch_pixbuf (FLUTTR_PHOTO 
						 		(photo));
			}
		}
		
		if (i == priv->active_photo) {
			fluttr_photo_set_active (FLUTTR_PHOTO (photo), TRUE);
			priv->active_actor = photo;
			
		} else
			fluttr_photo_set_active (FLUTTR_PHOTO (photo), FALSE);
	}
	clutter_actor_raise_top (priv->active_actor);
}

static gboolean
_peg (ClutterActor *photo)
{
	guint size = fluttr_photo_get_default_size ();
	fluttr_photo_update_position (FLUTTR_PHOTO (photo), 
				      clutter_actor_get_x (photo), 
				      CLUTTER_STAGE_HEIGHT () + size);
	return FALSE;
}

/* We make all the 'viewable' photos fall down, leaving just the main one */
void
fluttr_list_view_activate (FluttrListView *list_view)
{
	FluttrListViewPrivate *priv;
	gint len;
	gint i = 0;
	ClutterActor *photo = NULL;
	gint active_row = 0;
	guint size = fluttr_photo_get_default_size ();
	gint x_center = (CLUTTER_STAGE_WIDTH () /2) - (size /2);
	gint y_center = (CLUTTER_STAGE_HEIGHT ()/2) - (size /2);
		
	g_return_if_fail (FLUTTR_IS_LIST_VIEW (list_view));
	priv = FLUTTR_LIST_VIEW_GET_PRIVATE(list_view);

	len =  g_list_length (priv->photos);
	
	/* Find the active row */	
	active_row = 0;
	gint row = 0;
	gint col = 0;
	
	for (i = 0; i < len; i++) {
		if (i == priv->active_photo) {
			active_row = row;
			break;
		}
		col++;
		if (col > (priv->n_cols-1)) {
			col = 0;
			row++;
		}
	}
	
	/* Iterate through actors, calculating their new x positions, and make
	   sure they are on the right place (left, right or center) */
	col = 0;
	row = 0;
	
	for (i = 0; i < len; i++) {
		photo = (ClutterActor*)g_list_nth_data (priv->photos, i);
		 
		if (i == priv->active_photo) {
			fluttr_photo_update_position (FLUTTR_PHOTO (photo),
						      x_center, y_center);
		
		} else {
			if ((row >= active_row-2) && (row <= active_row +3)) {
							
												
				fluttr_photo_update_position 
					(FLUTTR_PHOTO (photo), 
					 clutter_actor_get_x (photo), 
					 clutter_actor_get_y (photo) - 20);

				/*fluttr_photo_update_position 
					(FLUTTR_PHOTO (photo), 
					 clutter_actor_get_x (photo), 
					 CLUTTER_STAGE_HEIGHT () + size);*/
				g_timeout_add (300, (GSourceFunc)_peg, photo);

			}
		}
		col++;
		if (col > (priv->n_cols-1)) {
			col = 0;
			row++;
		}		
	}
}

void
fluttr_list_view_advance_row (FluttrListView *view, gint n)
{
	FluttrListViewPrivate *priv;
	
	g_return_if_fail (FLUTTR_IS_LIST_VIEW (view));
	priv = FLUTTR_LIST_VIEW_GET_PRIVATE(view);
	
	fluttr_list_view_advance (view, (priv->n_cols * n));
}

void
fluttr_list_view_advance_col (FluttrListView *list_view, gint n)
{
	fluttr_list_view_advance (list_view, n);
}

/* Empty the group*/
static void
fluttr_list_view_empty (FluttrListView *view)
{
	FluttrListViewPrivate *priv;
	gint i;
	ClutterActor* child;
	gint len;
	
	g_return_if_fail (FLUTTR_IS_LIST_VIEW (view));
	priv = FLUTTR_LIST_VIEW_GET_PRIVATE(view);
	
	len = g_list_length (priv->photos);
	
	for (i = 0; i < len; i++) {
		child = (ClutterActor*)g_list_nth_data (priv->photos, i);
		clutter_group_remove (CLUTTER_GROUP (view), child);
		
	}
	g_list_free (priv->photos);
}
			
/* Populate the group */
static void
fluttr_list_view_populate (FluttrListView *view)
{
	FluttrListViewPrivate *priv;
	GList *photos = NULL;
	GList *p;
	gint x =(CLUTTER_STAGE_WIDTH ()/2)-(fluttr_photo_get_default_width()/2);
	gint y =(CLUTTER_STAGE_HEIGHT()/2)
		-(fluttr_photo_get_default_height()/2);
	
	g_return_if_fail (FLUTTR_IS_LIST_VIEW (view));
	priv = FLUTTR_LIST_VIEW_GET_PRIVATE(view);
	
	photos = fluttr_set_get_photos (FLUTTR_SET (priv->set));
	priv->photos = NULL;
	
	/* Go through each photodata in the list, creating a FluttrPhoto, and 
	   adding it to the group */
	
	for (p = photos; p != NULL; p = p->next) {
		FluttrPhotoData *data = (FluttrPhotoData*)p->data;
		ClutterActor *photo = fluttr_photo_new ();
		clutter_actor_set_size (photo, 
					fluttr_photo_get_default_width (),
					fluttr_photo_get_default_height ());
		clutter_actor_set_position (photo, x, y);
		clutter_group_add (CLUTTER_GROUP (view), photo);
		
		g_object_set (G_OBJECT (photo), 
			      "photoid", data->id,
			      "name", data->name,
			      NULL);
		
		/* Now lets set the pixbuf if we have it */
		if (data->pixbuf)
			g_object_set (G_OBJECT (photo), "pixbuf", data->pixbuf,
			      NULL);
		
		clutter_actor_show_all (photo);
		priv->photos = g_list_append (priv->photos, photo);
	}
	priv->active_photo = 0;
	priv->active_actor = NULL;
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
			if (priv->library)
                                g_object_ref (priv->library);
			/* Connect to the library signals */
			break;
			
		case PROP_SET:
			if (priv->set)
				g_object_unref (priv->set);
			priv->set = g_value_get_object (value);
			if (priv->set != NULL) {
				g_object_ref (priv->set);
				/* Empty the group*/
				fluttr_list_view_empty (
						     FLUTTR_LIST_VIEW (object));
				
				/* Populate the group */
				fluttr_list_view_populate (
						    FLUTTR_LIST_VIEW (object));
			}
			break;
		
		case PROP_COLS:
		        priv->n_cols = g_value_get_int (value);
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
		
		case PROP_SET:
			g_value_set_object (value, priv->library);
			break;		
		
		case PROP_COLS:
		        g_value_set_int (value, priv->n_cols);
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
	gint height;
	gint buf = -1 * fluttr_photo_get_default_width ();

	list = FLUTTR_LIST_VIEW(actor);

	priv = FLUTTR_LIST_VIEW_GET_PRIVATE(list);

	glPushMatrix();
	
	g_object_get (G_OBJECT (clutter_stage_get_default ()), "height", 
	              &height, NULL);
	gint i;
	gint len = clutter_group_get_n_children (CLUTTER_GROUP (actor)); 
	for (i = 0; i < len; i++) {
		ClutterActor* child;
                child = clutter_group_get_nth_child (CLUTTER_GROUP(actor), i);
		
		gint y;
		g_object_get (G_OBJECT (child), "y", &y, NULL);
                
                if (y < buf || y > height) {
                        fluttr_photo_set_visible (FLUTTR_PHOTO (child), FALSE);
                        continue;
                } else {
                        fluttr_photo_set_visible (FLUTTR_PHOTO (child), TRUE);
                }
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

	g_object_class_install_property 
		(gobject_class,
		 PROP_SET,
		 g_param_spec_object ("set",
		 "Set",
		 "The underlying Fluttr Photo set",
		 FLUTTR_TYPE_SET,
		 G_PARAM_CONSTRUCT|G_PARAM_READWRITE));	
		 
	g_object_class_install_property 
		(gobject_class,
		 PROP_COLS,
		 g_param_spec_int ("cols",
		 "Columns",
		 "The number of photo columns",
		 1, 10, 3,
		 G_PARAM_CONSTRUCT|G_PARAM_READWRITE));	

}

static void
fluttr_list_view_init (FluttrListView *self)
{
	FluttrListViewPrivate *priv;

	priv = FLUTTR_LIST_VIEW_GET_PRIVATE (self);
	
	priv->active_photo = 0;
	priv->active_col = 0;
	priv->set = NULL;
	
}

ClutterActor*
fluttr_list_view_new (void)
{
	ClutterGroup         *list_view;

	list_view = g_object_new (FLUTTR_TYPE_LIST_VIEW, 
			          NULL);
	
	return CLUTTER_ACTOR (list_view);
}

