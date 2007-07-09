#include "wh-busy.h"
#include "util.h"

G_DEFINE_TYPE (WHBusy, wh_busy, CLUTTER_TYPE_ACTOR);

#define BUSY_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), WH_TYPE_BUSY, WHBusyPrivate))

typedef struct _WHBusyPrivate WHBusyPrivate;

struct _WHBusyPrivate
{
  ClutterActor     *texture;
  ClutterTimeline  *timeline;
};

static void
timeline_cb (ClutterTimeline *timeline, 
	     gint             frame_num, 
	     WHBusy          *busy)
{
  WHBusyPrivate *priv  = BUSY_PRIVATE(busy);

  clutter_actor_rotate_z(priv->texture, (float)frame_num * 4.0, 
			 clutter_actor_get_width (priv->texture)/2, 
			 clutter_actor_get_height (priv->texture)/2);
}

static void
wh_busy_get_property (GObject *object, guint property_id,
		      GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
wh_busy_set_property (GObject *object, guint property_id,
		      const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
wh_busy_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (wh_busy_parent_class)->dispose)
    G_OBJECT_CLASS (wh_busy_parent_class)->dispose (object);
}

static void
wh_busy_finalize (GObject *object)
{
  G_OBJECT_CLASS (wh_busy_parent_class)->finalize (object);
}

static void
wh_busy_paint (ClutterActor *actor)
{
  WHBusy        *busy = WH_BUSY(actor);
  WHBusyPrivate *priv;

  priv = BUSY_PRIVATE(busy);

  clutter_actor_paint (priv->texture); 
}

static void
show_handler (ClutterActor *actor)
{
  WHBusy        *busy = WH_BUSY(actor);
  WHBusyPrivate *priv;

  priv = BUSY_PRIVATE(busy);

  clutter_timeline_start (priv->timeline);
}

static void
hide_handler (ClutterActor *actor)
{
  WHBusy        *busy = WH_BUSY(actor);
  WHBusyPrivate *priv;

  priv = BUSY_PRIVATE(busy);

  clutter_timeline_stop (priv->timeline);
}

static void
wh_busy_class_init (WHBusyClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
 ClutterActorClass *actor_class  = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (WHBusyPrivate));

  object_class->get_property = wh_busy_get_property;
  object_class->set_property = wh_busy_set_property;
  object_class->dispose = wh_busy_dispose;
  object_class->finalize = wh_busy_finalize;

  actor_class->paint = wh_busy_paint;
}

static void
wh_busy_init (WHBusy *self)
{
  WHBusyPrivate *priv;
  GdkPixbuf     *pixbuf;

  priv = BUSY_PRIVATE(self);

  pixbuf = gdk_pixbuf_new_from_file (PKGDATADIR "/spinner.svg", NULL); 

  if (pixbuf == NULL)
    g_error ("unable to load resource '%s'", PKGDATADIR "/spinner.svg");

  priv->texture = clutter_texture_new_from_pixbuf (pixbuf);
  clutter_actor_set_size (priv->texture, CSW()/6, CSW()/6);

  g_object_unref(pixbuf);

  clutter_actor_set_parent (priv->texture, CLUTTER_ACTOR(self)); 

  clutter_actor_set_position 
    (priv->texture, 
     (CSW() - clutter_actor_get_width (priv->texture))/2,
     (CSH() - clutter_actor_get_height (priv->texture))/2);

  priv->timeline = clutter_timeline_new (90, 120);
  clutter_timeline_set_loop (priv->timeline, TRUE);

  g_signal_connect (priv->timeline, 
		    "new-frame", 
		    G_CALLBACK (timeline_cb), 
		    self);

  g_signal_connect (self,
		    "show",
		    G_CALLBACK (show_handler),
		    NULL);

  g_signal_connect (self,
		    "hide",
		    G_CALLBACK (hide_handler),
		    NULL);
}

ClutterActor*
wh_busy_new (void)
{
  return CLUTTER_ACTOR(g_object_new (WH_TYPE_BUSY, NULL));
}
