#include "clutter-simple-layout.h"

G_DEFINE_TYPE (ClutterSimpleLayout, clutter_simple_layout, CLUTTER_TYPE_GROUP);

#define CLUTTER_SIMPLE_LAYOUT_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj),     \
   CLUTTER_TYPE_SIMPLE_LAYOUT,             \
   ClutterSimpleLayoutPrivate))

typedef struct _ClutterSimpleLayoutPrivate
{
  int               allocated_width, allocated_height;
  ClutterBehaviour *opacity_behave, *scale_behave;
  gulong            behave_signal_id;

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

  priv->allocated_width  = box->x2 - box->x1;
  priv->allocated_height = box->y2 - box->y1;

  /*
  printf("setting clip to %ix%i\n", 
	 priv->allocated_width, priv->allocated_height);
  */
  clutter_actor_set_clip (self, 0, 0, 
			  priv->allocated_width, priv->allocated_height);
}

static void
clutter_simple_layout_allocate_coords (ClutterActor    *self,
				       ClutterActorBox *box)
{
  ClutterSimpleLayoutPrivate *priv = CLUTTER_SIMPLE_LAYOUT_PRIVATE(self);

  box->x2 = box->x1 + priv->allocated_width;
  box->y2 = box->y1 + priv->allocated_height;
}

static void
clutter_simple_layout_class_init (ClutterSimpleLayoutClass *klass)
{
  GObjectClass      *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  actor_class->request_coords  = clutter_simple_layout_request_coords;
  actor_class->allocate_coords = clutter_simple_layout_allocate_coords;

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
  ClutterSimpleLayoutPrivate *priv = CLUTTER_SIMPLE_LAYOUT_PRIVATE(self);

  g_signal_connect (self,
		    "add",
		    G_CALLBACK (add_handler),
		    NULL);

  g_signal_connect (self,
		    "remove",
		    G_CALLBACK (remove_handler),
		    NULL);

  priv->scale_behave 
    = CLUTTER_BEHAVIOUR(g_object_new(CLUTTER_TYPE_BEHAVIOUR_SCALE, NULL));

  priv->opacity_behave 
    = CLUTTER_BEHAVIOUR(g_object_new(CLUTTER_TYPE_BEHAVIOUR_OPACITY, NULL));

}

ClutterActor*
clutter_simple_layout_new (void)
{
  return CLUTTER_ACTOR(g_object_new (CLUTTER_TYPE_SIMPLE_LAYOUT, NULL));
}

static void
fade_complete (ClutterTimeline *timeline,
	       gpointer         user_data)
{
  ClutterSimpleLayout        *self = CLUTTER_SIMPLE_LAYOUT(user_data);
  ClutterSimpleLayoutPrivate *priv = CLUTTER_SIMPLE_LAYOUT_PRIVATE(self);

  if (priv->behave_signal_id)
    {
      g_signal_handler_disconnect (timeline, priv->behave_signal_id);
      clutter_behaviour_remove (priv->scale_behave, CLUTTER_ACTOR(self));
      clutter_behaviour_remove (priv->opacity_behave, CLUTTER_ACTOR(self));
    }

  priv->behave_signal_id = 0;
}

void
clutter_simple_layout_fade_out (ClutterSimpleLayout *self, 
				ClutterAlpha        *alpha)
{
  ClutterSimpleLayoutPrivate *priv = CLUTTER_SIMPLE_LAYOUT_PRIVATE(self);

  if (priv->behave_signal_id)
    return;

  clutter_behaviour_set_alpha (priv->scale_behave, alpha);

  g_object_set (priv->scale_behave, 
		"scale-begin", 1.0f,
		"scale-end", 0.5f,
		"scale-gravity", CLUTTER_GRAVITY_CENTER,
		NULL);

  clutter_behaviour_apply (priv->scale_behave, CLUTTER_ACTOR(self));

  clutter_behaviour_set_alpha (priv->opacity_behave, alpha);

  g_object_set (priv->opacity_behave, 
		"opacity-start", 0xff,
		"opacity-end", 0x10,
		NULL);

  clutter_behaviour_apply (priv->opacity_behave, CLUTTER_ACTOR(self));

  priv->behave_signal_id 
    = g_signal_connect (clutter_alpha_get_timeline (alpha), 
			"completed",
			G_CALLBACK (fade_complete),
			self);

  clutter_timeline_start(clutter_alpha_get_timeline (alpha));
}

void
clutter_simple_layout_fade_in (ClutterSimpleLayout *self, 
			       ClutterAlpha        *alpha)
{
  ClutterSimpleLayoutPrivate *priv = CLUTTER_SIMPLE_LAYOUT_PRIVATE(self);

  if (priv->behave_signal_id)
    return;

  clutter_behaviour_set_alpha (priv->scale_behave, alpha);

  g_object_set (priv->scale_behave, 
		"scale-begin", 0.5f,
		"scale-end", 1.0f,
		"scale-gravity", CLUTTER_GRAVITY_CENTER,
		NULL);

  clutter_behaviour_apply (priv->scale_behave, CLUTTER_ACTOR(self));

  clutter_behaviour_set_alpha (priv->opacity_behave, alpha);

  g_object_set (priv->opacity_behave, 
		"opacity-start", 0x10,
		"opacity-end", 0xff,
		NULL);

  clutter_behaviour_apply (priv->opacity_behave, CLUTTER_ACTOR(self));

  priv->behave_signal_id 
    = g_signal_connect (clutter_alpha_get_timeline (alpha), 
			"completed",
			G_CALLBACK (fade_complete),
			self);

  clutter_timeline_start(clutter_alpha_get_timeline (alpha));
}
