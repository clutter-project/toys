#include "clutter-simple-layout.h"

G_DEFINE_TYPE (ClutterSimpleLayout, clutter_simple_layout, CLUTTER_TYPE_GROUP);

#define CLUTTER_SIMPLE_LAYOUT_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj),     \
   CLUTTER_TYPE_SIMPLE_LAYOUT,             \
   ClutterSimpleLayoutPrivate))

typedef struct _ClutterSimpleLayoutPrivate
{
  int               allocated_width, allocated_height;
} 
ClutterSimpleLayoutPrivate;

static void
clutter_simple_layout_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (clutter_simple_layout_parent_class)->dispose)
    G_OBJECT_CLASS (clutter_simple_layout_parent_class)->dispose (object);
}

static void
clutter_simple_layout_finalize (GObject *object)
{
  G_OBJECT_CLASS (clutter_simple_layout_parent_class)->finalize (object);
}

static void
clutter_simple_layout_request_coords (ClutterActor    *self,
				    ClutterActorBox *box)
{
  ClutterSimpleLayoutPrivate *priv = CLUTTER_SIMPLE_LAYOUT_PRIVATE(self);

  priv->allocated_width  = CLUTTER_UNITS_TO_INT(box->x2 - box->x1);
  priv->allocated_height = CLUTTER_UNITS_TO_INT(box->y2 - box->y1);

  /*
  printf("setting clip to %ix%i\n", 
	 priv->allocated_width, priv->allocated_height);
  */
  clutter_actor_set_clip (self, 0, 0, 
			  priv->allocated_width, priv->allocated_height);
}

static void
clutter_simple_layout_query_coords (ClutterActor    *self,
				       ClutterActorBox *box)
{
  ClutterSimpleLayoutPrivate *priv = CLUTTER_SIMPLE_LAYOUT_PRIVATE(self);

  box->x2 = box->x1 + CLUTTER_UNITS_FROM_INT(priv->allocated_width);
  box->y2 = box->y1 + CLUTTER_UNITS_FROM_INT(priv->allocated_height);
}

static void
clutter_simple_layout_class_init (ClutterSimpleLayoutClass *klass)
{
  GObjectClass      *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  actor_class->request_coords  = clutter_simple_layout_request_coords;
  actor_class->query_coords = clutter_simple_layout_query_coords;

  object_class->dispose = clutter_simple_layout_dispose;
  object_class->finalize = clutter_simple_layout_finalize;

  g_type_class_add_private (klass, sizeof (ClutterSimpleLayoutPrivate));
}

static void 
add_handler (ClutterGroup *group,
	     ClutterActor *actor,
	     gpointer      user_data)
{
  ClutterSimpleLayoutPrivate *priv = CLUTTER_SIMPLE_LAYOUT_PRIVATE(group);
  
  g_object_set(actor, "x", 
	       (priv->allocated_width - clutter_actor_get_width(actor))/2, 
	       NULL);
  /*
  printf("x -> %i - %i / 2 = %i\n",
	 priv->allocated_width, 
	 clutter_actor_get_width(actor),
	 (priv->allocated_width - clutter_actor_get_width(actor))/2);
  */
}

static void 
remove_handler (ClutterGroup *group,
		ClutterActor *actor,
		gpointer      user_data)
{

  /* ClutterSimpleLayoutPrivate *priv = CLUTTER_SIMPLE_LAYOUT_PRIVATE(group); */

}


static void
clutter_simple_layout_init (ClutterSimpleLayout *self)
{
  g_signal_connect (self,
		    "add",
		    G_CALLBACK (add_handler),
		    NULL);

  g_signal_connect (self,
		    "remove",
		    G_CALLBACK (remove_handler),
		    NULL);

}

ClutterActor*
clutter_simple_layout_new (void)
{
  return CLUTTER_ACTOR(g_object_new (CLUTTER_TYPE_SIMPLE_LAYOUT, NULL));
}


