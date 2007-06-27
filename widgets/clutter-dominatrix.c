/*
 * Clutter.
 *
 * An OpenGL based 'interactive canvas' library.
 *
 * Authored By Tomas Frydrych tf@openedhand.com>
 *
 * Copyright (C) 2007 OpenedHand
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:clutter-dominatrix
 * @short_description: An actor manipulation proxy.
 * 
 * #ClutterDominatrix is a proxy object for manipulation for actors via a
 * pointer: the slave actor can be rotated by dragging one of it's corners,
 * moved by dragging it's center and resizes by dragging the rest of it.
 */

#include "clutter-dominatrix.h"
#include <clutter/clutter.h>
#include <stdlib.h>

#ifndef CLUTTER_PARAM_READWRITE
#define CLUTTER_PARAM_READWRITE \
        G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |G_PARAM_STATIC_BLURB
#endif

G_DEFINE_TYPE (ClutterDominatrix,
	       clutter_dominatrix,
	       G_TYPE_OBJECT);

#define CLUTTER_DOMINATRIX_GET_PRIVATE(obj) \
(G_TYPE_INSTANCE_GET_PRIVATE ((obj), CLUTTER_TYPE_DOMINATRIX, ClutterDominatrixPrivate))

static void clutter_dominatrix_on_event (ClutterStage *stage,
					 ClutterEvent *event,
					 gpointer      data);

typedef enum {
  DRAG_NONE = 0,
  DRAG_ROTATE,
  DRAG_MOVE,
  DRAG_RESIZE_TL,
  DRAG_RESIZE_TR,
  DRAG_RESIZE_BL,
  DRAG_RESIZE_BR,
  DRAG_SCALE,
} DragType;

struct _ClutterDominatrixPrivate
{
  ClutterActor * slave;

  guint rhandle_width;
  guint rhandle_height;
  
  guint mhandle_width;
  guint mhandle_height;

  DragType  dragging;
  gint      prev_x;
  gint      prev_y;
  gint      center_x;
  gint      center_y;
  
  gulong    handler_id;

  gboolean  scale : 1;
  gboolean  dont_rotate : 1;
  gboolean  dont_resize : 1;
  gboolean  dont_move : 1;

  ClutterGravity gravity;

  ClutterActorBox orig_box;
  ClutterFixed    orig_scale_x;
  ClutterFixed    orig_scale_y;
  ClutterFixed    orig_zang;
};

enum
{
  MANIPULATION_STARTED,
  MANIPULATION_ENDED,
  
  LAST_SIGNAL
};

static guint dmx_signals[LAST_SIGNAL] = { 0, };

enum
{
  PROP_0,
  PROP_ROTATE_HANDLE_WIDTH,
  PROP_ROTATE_HANDLE_HEIGHT,
  PROP_MOVE_HANDLE_WIDTH,
  PROP_MOVE_HANDLE_HEIGHT,
  PROP_SLAVE,
  PROP_SCALE,
  PROP_DISABLE_ROTATION,
  PROP_DISABLE_RESIZING,
  PROP_DISABLE_MOVEMENT,
  PROP_GRAVITY,
};


static void 
clutter_dominatrix_set_property (GObject      *object, 
				  guint         prop_id,
				  const GValue *value, 
				  GParamSpec   *pspec)
{

  ClutterDominatrix        *dominatrix;
  ClutterDominatrixPrivate *priv;

  dominatrix = CLUTTER_DOMINATRIX(object);
  priv = dominatrix->priv;

  switch (prop_id) 
    {
    case PROP_ROTATE_HANDLE_WIDTH:
      priv->rhandle_width = g_value_get_int (value);
      break;
    case PROP_ROTATE_HANDLE_HEIGHT:
      priv->rhandle_height = g_value_get_int (value);
      break;
    case PROP_MOVE_HANDLE_WIDTH:
      priv->mhandle_width = g_value_get_int (value);
      break;
    case PROP_MOVE_HANDLE_HEIGHT:
      priv->mhandle_height = g_value_get_int (value);
      break;
    case PROP_SLAVE:
      clutter_dominatrix_set_slave (dominatrix,
				CLUTTER_ACTOR (g_value_get_pointer (value)));
      break;
    case PROP_SCALE:
      priv->scale = g_value_get_boolean (value);
      break;
    case PROP_DISABLE_ROTATION:
      priv->dont_rotate = g_value_get_boolean (value);
      break;
    case PROP_DISABLE_RESIZING:
      priv->dont_resize = g_value_get_boolean (value);
      break;
    case PROP_DISABLE_MOVEMENT:
      priv->dont_move = g_value_get_boolean (value);
      break;
    case PROP_GRAVITY:
      priv->gravity = g_value_get_enum (value);
      break;
      
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void 
clutter_dominatrix_get_property (GObject    *object, 
				  guint       prop_id,
				  GValue     *value, 
				  GParamSpec *pspec)
{
  ClutterDominatrix        *dominatrix;
  ClutterDominatrixPrivate *priv;

  dominatrix = CLUTTER_DOMINATRIX(object);
  priv = dominatrix->priv;

  switch (prop_id) 
    {
    case PROP_ROTATE_HANDLE_WIDTH:
      g_value_set_int (value, priv->rhandle_width);
      break;
    case PROP_ROTATE_HANDLE_HEIGHT:
      g_value_set_int (value, priv->rhandle_height);
      break;
    case PROP_MOVE_HANDLE_WIDTH:
      g_value_set_int (value, priv->mhandle_width);
      break;
    case PROP_MOVE_HANDLE_HEIGHT:
      g_value_set_int (value, priv->mhandle_height);
      break;
    case PROP_SLAVE:
      g_value_set_pointer (value, priv->slave);
      break;
    case PROP_SCALE:
      g_value_set_boolean (value, priv->scale);
      break;
    case PROP_DISABLE_ROTATION:
      g_value_set_boolean (value, priv->dont_rotate);
      break;
    case PROP_DISABLE_RESIZING:
      g_value_set_boolean (value, priv->dont_resize);
      break;
    case PROP_DISABLE_MOVEMENT:
      g_value_set_boolean (value, priv->dont_move);
      break;
    case PROP_GRAVITY:
      g_value_set_enum (value, priv->gravity);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
clutter_dominatrix_release_slave (ClutterDominatrixPrivate * priv)
{
  priv->dragging = DRAG_NONE;
  
  if (priv->slave)
    {
      g_object_unref (priv->slave);
      priv->slave = NULL;
    }
}

static void 
clutter_dominatrix_finalize (GObject *object)
{
  ClutterDominatrix *dmx = CLUTTER_DOMINATRIX (object);

  clutter_dominatrix_release_slave (dmx->priv);
  
  if (dmx->priv->handler_id)
    g_signal_handler_disconnect (clutter_stage_get_default (),
				 dmx->priv->handler_id);
  
  G_OBJECT_CLASS (clutter_dominatrix_parent_class)->finalize (object);
}

static void
clutter_dominatrix_store_original_settings (ClutterDominatrixPrivate *priv)
{
  clutter_actor_query_coords (priv->slave, &priv->orig_box);
  
  clutter_actor_get_scalex (priv->slave,
			    &priv->orig_scale_x,
			    &priv->orig_scale_y);

  priv->orig_zang = clutter_actor_get_rzangx (priv->slave);
}

static GObject *
clutter_dominatrix_constructor (GType                  gtype,
				guint                  n_params,
				GObjectConstructParam *params)
{
  GObjectClass       * parent_class;
  GObject            * retval;
  ClutterDominatrix  * dmx;
  ClutterActor       * stage;
  
  parent_class = G_OBJECT_CLASS (clutter_dominatrix_parent_class);
  retval = parent_class->constructor (gtype, n_params, params);

  dmx = CLUTTER_DOMINATRIX (retval);

  stage = clutter_stage_get_default ();
  
  dmx->priv->handler_id =
    g_signal_connect (stage, "event",
		      G_CALLBACK (clutter_dominatrix_on_event), dmx);
  
  clutter_dominatrix_store_original_settings (dmx->priv);
  
  return retval;
}

static void
clutter_dominatrix_class_init (ClutterDominatrixClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructor = clutter_dominatrix_constructor;
  object_class->set_property = clutter_dominatrix_set_property;
  object_class->get_property = clutter_dominatrix_get_property;
  object_class->finalize     = clutter_dominatrix_finalize;

  g_type_class_add_private (klass, sizeof (ClutterDominatrixPrivate));

  /**
   * ClutterDominatrix:rotate-handle-width:
   *
   * Width of the rotation handle.
   */
  g_object_class_install_property (object_class,
                                   PROP_ROTATE_HANDLE_WIDTH,
                                   g_param_spec_int ("rotate-handle-width",
                                                "width of rotation handle",
                                                "width of rotation handle",
                                                0, G_MAXINT,
                                                0,
                                                CLUTTER_PARAM_READWRITE));

  /**
   * ClutterDominatrix:rotate-handle-height:
   *
   * Height of the rotation handle.
   */
  g_object_class_install_property (object_class,
                                   PROP_ROTATE_HANDLE_HEIGHT,
                                   g_param_spec_int ("rotate-handle-height",
                                                "height of rotation handle",
                                                "height of rotation handle",
                                                0, G_MAXINT,
                                                0,
                                                CLUTTER_PARAM_READWRITE));

  /**
   * ClutterDominatrix:move-handle-width:
   *
   * Width of the move handle.
   */
  g_object_class_install_property (object_class,
                                   PROP_MOVE_HANDLE_WIDTH,
                                   g_param_spec_int ("move-handle-width",
                                                "width of move handle",
                                                "width of move handle",
                                                0, G_MAXINT,
                                                0,
                                                CLUTTER_PARAM_READWRITE));

  /**
   * ClutterDominatrix:move-handle-height:
   *
   * Height of the move handle.
   */
  g_object_class_install_property (object_class,
                                   PROP_MOVE_HANDLE_HEIGHT,
                                   g_param_spec_int ("move-handle-height",
                                                "height of move handle",
                                                "height of move handle",
                                                0, G_MAXINT,
                                                0,
                                                CLUTTER_PARAM_READWRITE));

  
  /**
   * ClutterDominatrix:slave:
   *
   * Slave we are manipulating.
   */
  g_object_class_install_property (object_class,
                                   PROP_SLAVE,
                                   g_param_spec_pointer ("slave",
                                                "slave",
                                                "slave",
				                G_PARAM_CONSTRUCT |
						CLUTTER_PARAM_READWRITE));

  /**
   * ClutterDominatrix:scale:
   *
   * Whether dragging in the no-mans land should be translated to scaling
   * or resizing. Deafult TRUE
   */
  g_object_class_install_property (object_class,
                                   PROP_SCALE,
                                   g_param_spec_boolean ("scale",
                                                "whether to scale or resize",
                                                "whether to scale or resize",
						TRUE,
                                                CLUTTER_PARAM_READWRITE));

  /**
   * ClutterDominatrix:disable-rotation:
   *
   * Whether rotation should be disabled; default FALSE
   */
  g_object_class_install_property (object_class,
                                   PROP_DISABLE_ROTATION,
                                   g_param_spec_boolean ("disable-rotation",
                                                "whether to rotate",
                                                "whether to rotate",
						FALSE,
                                                CLUTTER_PARAM_READWRITE));
  

  /**
   * ClutterDominatrix:disable-resizing:
   *
   * Whether resizing should be disabled; default FALSE
   */
  g_object_class_install_property (object_class,
                                   PROP_DISABLE_RESIZING,
                                   g_param_spec_boolean ("disable-resizing",
                                                "whether to resize",
                                                "whether to resize",
						FALSE,
                                                CLUTTER_PARAM_READWRITE));
  
  /**
   * ClutterDominatrix:disable-movement:
   *
   * Whether moving should be disabled; default FALSE
   */
  g_object_class_install_property (object_class,
                                   PROP_DISABLE_MOVEMENT,
                                   g_param_spec_boolean ("disable-movement",
                                                "whether to move",
                                                "whether to move",
						FALSE,
                                                CLUTTER_PARAM_READWRITE));
  
  /**
   * ClutterDominatrix:gravity:
   *
   * Gravity to use when scaling; default CLUTTER_GRAVITY_CENTER
   */
  g_object_class_install_property (object_class,
                                   PROP_GRAVITY,
                                   g_param_spec_enum ("gravity",
                                            "which gravity to use for scaling",
                                            "which gravity to use for scaling",
					    CLUTTER_TYPE_GRAVITY,
					    CLUTTER_GRAVITY_CENTER,
			                    G_PARAM_CONSTRUCT |
                                            CLUTTER_PARAM_READWRITE));

  /**
   * ClutterDominatrix::manipulation-started:
   * @dmx: the object which received the signal
   *
   * This signal is emitted each time the users starts to manipulate the
   * actor.
   *
   */
  dmx_signals[MANIPULATION_STARTED] =
    g_signal_new ("manipulation-started",
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (ClutterDominatrixClass,
				   manipulation_started),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  
  /**
   * ClutterDominatrix::manipulation-ended:
   * @dmx: the object which received the signal
   *
   * This signal is emitted each time the users starts to manipulate the
   * actor.
   *
   */
  dmx_signals[MANIPULATION_ENDED] =
    g_signal_new ("manipulation-ended",
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (ClutterDominatrixClass,
				   manipulation_ended),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
clutter_dominatrix_init (ClutterDominatrix *self)
{
  self->priv = CLUTTER_DOMINATRIX_GET_PRIVATE (self);

  self->priv->rhandle_width  = 20;
  self->priv->rhandle_height = 20;
  self->priv->mhandle_width  = 20;
  self->priv->mhandle_height = 20;
  self->priv->scale = TRUE;
  self->priv->gravity = CLUTTER_GRAVITY_CENTER;
}

static void 
clutter_dominatrix_on_event (ClutterStage *stage,
			     ClutterEvent *event,
			     gpointer      data)
{
  ClutterDominatrix        * dominatrix = data;
  ClutterDominatrixPrivate * priv = dominatrix->priv;
  
  switch (event->type)
    {
    case CLUTTER_BUTTON_PRESS:
      {
	gint x, y;
	ClutterActor   * actor;
	ClutterVertex    v[4];
	ClutterFixed     zang;
	gint32           xp, yp, zp;
	gint i;
	gint width, height;
	gint mhandle_width  = priv->mhandle_width;
	gint mhandle_height = priv->mhandle_height;
	gint rhandle_width  = priv->rhandle_width;
	gint rhandle_height = priv->rhandle_height;
	
        clutter_event_get_coords (event, &x, &y);

	actor = clutter_stage_get_actor_at_pos (stage, x, y);
	
	while (actor &&
	       actor != priv->slave &&
	       (actor = clutter_actor_get_parent (actor)));
	
	if (actor != priv->slave)
	  return;

	clutter_actor_raise_top (priv->slave);

	g_signal_emit (dominatrix, dmx_signals[MANIPULATION_STARTED], 0);

	priv->prev_x = x;
	priv->prev_y = y;
	
	/* Check that the handle size are sensible in relationship to the
	 * projected size of our slave, otherwise if the user reduces the size
	 * of the slave too much, s/he will not be able to expand it again
	 * -- we practice safe bondage only in this house ;).
	 *
	 * Allow the movement handle to be at most half of the width/height
	 * and the rotation handles to be at most a quarter of width/height.
	 */
	clutter_actor_get_vertices (actor, v);

	width  = CLUTTER_FIXED_INT (abs (v[0].x - v[3].x));
	height = CLUTTER_FIXED_INT (abs (v[0].y - v[3].y));

	if (width < 2 * mhandle_width)
	  {
	    mhandle_width = width >> 1;
	    g_debug ("Adjusted mhandle width to %d", mhandle_width);
	  }

	if (height < 2 * mhandle_height)
	  {
	    mhandle_height = height >> 1;
	    g_debug ("Adjusted mhandle height to %d", mhandle_height);
	  }
	
	if (width < 4 * rhandle_width)
	  {
	    rhandle_width = width >> 2;
	    g_debug ("Adjusted rhandle width to %d", rhandle_width);
	  }
	
	if (height < 4 * rhandle_height)
	  {
	    rhandle_height = height >> 2;
	    g_debug ("Adjusted rhandle height to %d", rhandle_height);
	  }
	
	/*
	 * work out drag type
	 *
	 * First, check for movement
	 */
	xp = CLUTTER_INT_TO_FIXED (clutter_actor_get_width  (actor) / 2);
	yp = CLUTTER_INT_TO_FIXED (clutter_actor_get_height (actor) / 2);
	zp = 0;
  
	clutter_actor_apply_transform_to_point (actor, xp, yp, zp,
						&xp, &yp, &zp);

	xp = CLUTTER_FIXED_INT (xp);
	yp = CLUTTER_FIXED_INT (yp);

	/* Store these for later */
	priv->center_x = xp;
	priv->center_y = yp;
	
	if (abs (xp - x) < mhandle_width &&
	    abs (yp - y) < mhandle_height)
	  {
	    priv->dragging = DRAG_MOVE;
	    return;
	  }

	/*
	 * Next, we check for rotation
	 */
	for (i = 0; i < 4; ++i)
	  if (abs (CLUTTER_FIXED_INT (v[i].x) - x) < rhandle_width &&
	      abs (CLUTTER_FIXED_INT (v[i].y) - y) < rhandle_height)
	    {
	      priv->dragging = DRAG_ROTATE;
	      return;
	    }

	
	/*
	 *  Neither move or rotation, so we are resizing or scaling.
         */
	if (priv->scale)
	  {
	    priv->dragging = DRAG_SCALE;
	    return;
	  }
	else
	  {
	    /*
	     * We notionally divide the projected area into 2 x 2 grid,
	     * representing 4 types of resize.
	     *
	     * If the object is rotated, we need to unrotate the screen
	     * coords first.
	     */
	    zang = clutter_actor_get_rzangx (actor);

	    if (zang)
	      {
		gint x2 = x - xp;
		gint y2 = y - yp;
		ClutterFixed zang_rad = -CFX_MUL (zang, CFX_PI) / 180;
	    
		x = CLUTTER_FIXED_INT (x2 * clutter_cosx (zang_rad) -
				       y2 * clutter_sinx (zang_rad)) + xp;

		y = CLUTTER_FIXED_INT (y2 * clutter_cosx (zang_rad) +
				       x2 * clutter_sinx (zang_rad)) + yp;
	      }
	
	    if (x < xp && y < yp)
	      {
		priv->dragging = DRAG_RESIZE_TL;
		g_debug ("Resize TL");
		return;
	      }

	    if (x < xp && y >= yp)
	      {
		priv->dragging = DRAG_RESIZE_BL;
		g_debug ("Resize BL");
		return;
	      }

	    if (x >= xp && y < yp)
	      {
		priv->dragging = DRAG_RESIZE_TR;
		g_debug ("Resize TR");
		return;
	      }

	    if (x >= xp && y >= yp)
	      {
		priv->dragging = DRAG_RESIZE_BR;
		g_debug ("Resize BR");
		return;
	      }
	  }
	
	g_warning ("Error calculating drag type");
	priv->dragging = DRAG_NONE;
      }
      break;

    case CLUTTER_MOTION:
      {
	gint x, y;
	ClutterFixed zang;
	ClutterActorBox box;

	if (priv->dragging == DRAG_NONE)
	  return;
	
        clutter_event_get_coords (event, &x, &y);
	
	/* We intentionally do not test here if the pointer is within
	 * our slave since we want to be able to manipulate the objects with
	 * the point outwith the object (i.e., for greater precission when
	 * rotating)
	 */
	clutter_actor_query_coords (priv->slave, &box);

	zang = clutter_actor_get_rzangx (priv->slave);

	if (priv->dragging == DRAG_MOVE)
	  {
	    if (priv->dont_move)
	      return;
	    
	    clutter_actor_move_by (priv->slave,
				   x - priv->prev_x,
				   y - priv->prev_y);
	  }
	else if (priv->dragging >= DRAG_RESIZE_TL &&
		 priv->dragging <= DRAG_RESIZE_BR)
	  {
	    ClutterFixed xp, yp;

	    if (priv->dont_resize)
	      return;
	    
	    
	    xp = CLUTTER_INT_TO_FIXED (x - priv->prev_x);
	    yp = CLUTTER_INT_TO_FIXED (y - priv->prev_y);

	    if (zang)
	      {
		gint x2 = x - priv->prev_x;
		gint y2 = y - priv->prev_y;
		
		ClutterFixed zang_rad = -CFX_MUL (zang, CFX_PI) / 180;
	    
		xp = x2 * clutter_cosx (zang_rad) -
		  y2 * clutter_sinx (zang_rad);

		yp = y2 * clutter_cosx (zang_rad) +
		  x2 * clutter_sinx (zang_rad);
	      }
	    
	    switch (priv->dragging)
	      {
	      case DRAG_RESIZE_TL:
		box.x1 += xp;
		box.y1 += yp;
		break;
	      case DRAG_RESIZE_TR:
		box.x2 += xp;
		box.y1 += yp;
		break;
	      case DRAG_RESIZE_BL:
		box.x1 += xp;
		box.y2 += yp;
		break;
	      case DRAG_RESIZE_BR:
		box.x2 += xp;
		box.y2 += yp;
		break;
	      default:
		break;
	      }
	    
	    clutter_actor_request_coords (priv->slave, &box);
	  }
	else if (priv->dragging == DRAG_ROTATE)
	  {
	    ClutterFixed a;
	    gint x1, x2, y1, y2;
	    
	    if (priv->dont_rotate)
	      return;
	    
	    x1 = priv->prev_x;
	    y1 = priv->prev_y;
	    x2 = x;
	    y2 = y;
	    
	    /*
	     * For the incremental angle a,
	     * 
	     * sin(a) = (x1*y2 - x2*y1) / (x1^2 + y1^2)
	     *
	     * where x1,y1 and x2,y2 are coords relative to the center of
	     * rotation.
	     * 
	     * For very small a, we can assume sin(a) == a,
	     * and after converting from rad to deg and to ClutterFixed,
	     * we get,
	     *
	     * a = 0x394bb8 * (x1*y2 - x2*y1) / (x1^2 + y1^2),
	     *
	     */

	    /* We work out the rotation on screen, not in the actor space.
	     * This is not entirely acurate, but considerably easier, and
	     * since the angles are very small should be generally enough for
	     * the rotatated actor not to get out of sync with the position
	     * of the pointer even if it is somewhat rotated around x and/or y
	     * axes.
	     *
	     * FIXME: if the actor has been rotated around the Z axis prior to
	     * start of our dragging, and the center of rotation was not the
	     * center of the actor, the actor will move from it's current
	     * position, since we will preserve the rotation angle, but change
	     * the pivot point. This is probably not a great deal for the
	     * kinds of application the dominatrix is intended for.
	     *
	     * First, project the center of the actor, which will be our
	     * reference point.
	     */
	    x1 -= priv->center_x;
	    y1 -= priv->center_y;
	    x2 -= priv->center_x;
	    y2 -= priv->center_y;
	    
	    a = (((x1 * y2 - x2 * y1) * 0x32000) / (x1 * x1 + y1 * y1)) << 4;

	    /*
	     * For anything above 0.7 rad, we tweak the value a bit
	     */
	    if (a >= 0xb333)
	      a = CFX_MUL (a, 0x14000);

	    clutter_actor_rotate_zx (priv->slave, zang + a,
				     clutter_actor_get_width (priv->slave)>>1,
				     clutter_actor_get_height(priv->slave)>>1);
	  }
	else if (priv->dragging == DRAG_SCALE)
	  {
	    /*
	     * for each pixle of movement from the center increase scale by
	     * some sensible step.
	     */
#define SCALE_STEP 40
	    ClutterFixed sx, sy;
	    gint d1, d2, diff, x1, y1, x2, y2;

	    if (priv->dont_resize)
	      return;
	    
	    x1 = abs (priv->prev_x - priv->center_x);
	    y1 = abs (priv->prev_y - priv->center_y);
	    x2 = abs (x - priv->center_x);
	    y2 = abs (y - priv->center_y);
	    
	    clutter_actor_get_scalex (priv->slave, &sx, &sy);

	    d1 = x1*x1 + y1*y1;
	    d2 = x2*x2 + y2*y2;

	    /* What we should do now is to sqrt d1 and d2 and substract the
	     * results but d1 and d2 can be quite big and sqrti does not
	     * handle very big numbers well, while ClutterFixed range is
	     * limited, ruling out sqrtx. We do not want to use sqrt for
	     * performance reasons, and all we need is reasonable speed of
	     * scaling and semblance of constancy. So, we substract the
	     * numbers first, then sqrti the difference, give it appropriate
	     * sign and choose a suitable step to go with what that produces.
	     */
	    diff = clutter_sqrti (abs (d2 - d1));

	    if (d1 > d2)
	      diff = -diff;
	    
	    sx += SCALE_STEP * diff;
	    sy += SCALE_STEP * diff;
	    
	    clutter_actor_set_scale_with_gravityx (priv->slave, sx, sy,
						   priv->gravity);
#undef SCALE_STEP
	  }

	priv->prev_x = x;
	priv->prev_y = y;

      }
      break;
      
    case CLUTTER_BUTTON_RELEASE:
      {
	if (priv->dragging != DRAG_NONE)
	  {
	    g_signal_emit (dominatrix, dmx_signals[MANIPULATION_ENDED], 0);
	    priv->dragging = DRAG_NONE;
	  }
      }
      break;
      
    default:
      break;
    }
}


/**
 * clutter_dominatrix_new:
 * @slave: a #ClutterActor to manipulate
 *
 * Creates a ClutterDominatrix proxy for the given actor that allows
 * the user to be manipulated via a pointer.
 * 
 * When you are done with the proxy, release the references to it.
 */
ClutterDominatrix *
clutter_dominatrix_new (ClutterActor *slave)
{
  return g_object_new (CLUTTER_TYPE_DOMINATRIX, "slave", slave, NULL);
}

/**
 * clutter_dominatrix_new_with_gravity:
 * @slave: a #ClutterActor to manipulate
 * @gravity: a #ClutterGravity to use when the actor is being scaled.
 * 
 * Creates a ClutterDominatrix proxy for the given actor that allows
 * the user to be manipulated via a pointer, and sets the gravity for scaling
 * to the provided value.
 * 
 * When you are done with the proxy, release the references to it.
 */
ClutterDominatrix *
clutter_dominatrix_new_with_gravity (ClutterActor   * slave,
				     ClutterGravity   gravity)
{
  return g_object_new (CLUTTER_TYPE_DOMINATRIX,
		       "slave", slave,
		       "gravity", gravity,
		       NULL);
}

/**
 * clutter_dominatrix_set_slave:
 * @dmx: a #ClutterDominatrix
 * @slave: a #ClutterActor that the proxy should operate on.
 * 
 */
void
clutter_dominatrix_set_slave (ClutterDominatrix *dmx, ClutterActor *slave)
{
  clutter_dominatrix_release_slave (dmx->priv);

  g_object_ref (slave);
  dmx->priv->slave = slave;

  clutter_dominatrix_store_original_settings (dmx->priv);
}

/**
 * clutter_dominatrix_get_slave:
 * @dmx: a #ClutterDominatrix
 *
 * Returns the #ClutterActor that the proxy currently operates on. When you
 * are done with it, you have to unref the slave.
 */
ClutterActor *
clutter_dominatrix_get_slave (ClutterDominatrix *dmx)
{
  g_return_val_if_fail (CLUTTER_IS_DOMINATRIX (dmx), NULL);
  
  if (dmx->priv->slave)
    g_object_ref (dmx->priv->slave);

  return dmx->priv->slave;
}

/**
 * clutter_dominatrix_restore:
 * @dmx: a #ClutterDominatrix
 *
 * Restores the slave the proxy manipulates to it's original state.
 */
void
clutter_dominatrix_restore (ClutterDominatrix *dmx)
{
  ClutterDominatrixPrivate * priv;
  g_return_if_fail (CLUTTER_IS_DOMINATRIX (dmx));

  priv = dmx->priv;
  
  clutter_actor_rotate_zx (priv->slave, priv->orig_zang, 0, 0);
  
  clutter_actor_set_scalex (priv->slave,
			    priv->orig_scale_x, priv->orig_scale_y);
  
  clutter_actor_request_coords (priv->slave, &priv->orig_box);
}
