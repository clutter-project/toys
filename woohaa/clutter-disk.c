#include <clutter/clutter.h>
#include "clutter-disk.h"

#define DISC_SIZE 100
#define DISC_SIZE 100


G_DEFINE_TYPE (ClutterDisk, clutter_disk, CLUTTER_TYPE_ACTOR);

enum
{
  PROP_0,
};

#define CLUTTER_DISK_GET_PRIVATE(obj) \
(G_TYPE_INSTANCE_GET_PRIVATE ((obj), CLUTTER_TYPE_DISK, ClutterDiskPrivate))

typedef struct ClutterDiskSegment
{
  ClutterActor *actor;
  guint8        opacity;
  gint          angle;
}
ClutterDiskSegment;

struct _ClutterDiskPrivate
{
  gint           size;
  gint           n_segments;
  GSList        *segments;
  gint           active_angle;  
};

static void
clutter_disk_set_property (GObject      *object, 
			    guint         prop_id,
			    const GValue *value, 
			    GParamSpec   *pspec)
{
  ClutterDisk        *disk;
  ClutterDiskPrivate *priv;

  disk = CLUTTER_DISK(object);
  priv = disk->priv;

  switch (prop_id) 
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
clutter_disk_get_property (GObject    *object, 
			    guint       prop_id,
			    GValue     *value, 
			    GParamSpec *pspec)
{
  ClutterDisk        *disk;
  ClutterDiskPrivate *priv;

  disk = CLUTTER_DISK(object);
  priv = disk->priv;

  switch (prop_id) 
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    } 
}

static void
clutter_disk_paint (ClutterActor *self)
{
  ClutterDisk        *disk;
  ClutterDiskPrivate *priv;
  GSList             *segment_item;
  ClutterGeometry     geom;      

  disk = CLUTTER_DISK(self);
  priv = disk->priv;

  glPushMatrix();

  clutter_actor_get_geometry (self, &geom);

#if 0
  if (geom.x != 0 || geom.y != 0)
    glTranslatef(geom.x, geom.y, 0.0);
#endif

  for (segment_item = priv->segments;
       segment_item != NULL;
       segment_item = segment_item->next)
    {
      ClutterDiskSegment *segment = segment_item->data;
      guint8 opacity;

      clutter_actor_set_size (segment->actor, 
			      priv->size/priv->n_segments, priv->size/3); 
      clutter_actor_set_position (segment->actor, 
				  priv->size/2, 0);
      clutter_actor_rotate_z (segment->actor, segment->angle, 0, priv->size/2); 
      if (disk->priv->active_angle < segment->angle)
	clutter_actor_set_opacity (segment->actor, segment->opacity);
      else
	clutter_actor_set_opacity (segment->actor, 0xff);

      if (segment->angle < disk->priv->active_angle)
	{
	  opacity = 51 + ((disk->priv->active_angle * 204) / 360);
	}
      else opacity = 0x33;

      clutter_actor_set_opacity (segment->actor, opacity);

      clutter_actor_paint (segment->actor);
    }

  glPopMatrix();
}

void 
clutter_disk_input (ClutterStage *stage, 
		    ClutterEvent *event,
		    ClutterDisk  *disk)
{

  switch (event->type)
    {
    case CLUTTER_MOTION:
      {
	ClutterMotionEvent *mev = (ClutterMotionEvent *) event;
	double  angle;
        double  dx, dy;
	int      x, y;

	static double last_angle = 0;

	clutter_actor_get_abs_position (CLUTTER_ACTOR(disk), &x, &y);

	dx = mev->x - (x + disk->priv->size/2);
	dy = mev->y - (y  + disk->priv->size/2);

	angle = (atan2 (dy, dx) * 57.29) + 90;
	if (angle < 0) angle += 360;

	if (last_angle && last_angle - angle > 300)
	  angle = 360;

	disk->priv->active_angle = (gint)angle;

	clutter_actor_queue_redraw (CLUTTER_ACTOR(disk));

	last_angle = angle;

	break;
      }
    default:
      break;
    }
}

static void
clutter_disk_realize (ClutterActor *self)
{
  ClutterDisk        *disk;
  ClutterDiskPrivate *priv;
  gint                i;
  ClutterColor        color = { 0x0, 0x11, 0xde, 0xff } ;

  disk = CLUTTER_DISK(self);
  priv = disk->priv;

  for (i=0; i<priv->n_segments; i++) 
    {
      ClutterDiskSegment *segment;

      segment = g_new0(ClutterDiskSegment, 1);
      segment->actor   = clutter_rectangle_new_with_color (&color);
      segment->angle   = (360/priv->n_segments)*i;
      segment->opacity = 127;

      priv->segments = g_slist_append (priv->segments, segment);

      clutter_actor_set_parent (segment->actor, CLUTTER_ACTOR (self));
    }

  g_signal_connect (clutter_stage_get_default(), 
		    "input-event",
		    G_CALLBACK (clutter_disk_input), 
		    disk);
}

static void 
clutter_disk_dispose (GObject *object)
{
  ClutterDisk         *self = CLUTTER_DISK(object);
  ClutterDiskPrivate  *priv;  

  priv = self->priv;

  
  G_OBJECT_CLASS (clutter_disk_parent_class)->dispose (object);
}

static void 
clutter_disk_finalize (GObject *object)
{
  G_OBJECT_CLASS (clutter_disk_parent_class)->finalize (object);
}

static void
clutter_disk_class_init (ClutterDiskClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass   *actor_class = CLUTTER_ACTOR_CLASS (klass);
  ClutterActorClass   *parent_class 
                          = CLUTTER_ACTOR_CLASS (clutter_disk_parent_class);

  actor_class->paint      = clutter_disk_paint;
  actor_class->realize    = clutter_disk_realize;
  actor_class->unrealize  = parent_class->unrealize;
  actor_class->show       = parent_class->show;
  actor_class->hide       = parent_class->hide;

  gobject_class->finalize     = clutter_disk_finalize;
  gobject_class->dispose      = clutter_disk_dispose;
  gobject_class->set_property = clutter_disk_set_property;
  gobject_class->get_property = clutter_disk_get_property;

  g_type_class_add_private (gobject_class, sizeof (ClutterDiskPrivate));
}

static void
clutter_disk_init (ClutterDisk *self)
{
  ClutterDiskPrivate *priv;

  self->priv = priv = CLUTTER_DISK_GET_PRIVATE (self);
}

/**
 * clutter_disk_new
 *
 * Creates a new #ClutterDisk displaying @text using @font_name.
 *
 * Return value: a #ClutterDisk
 */
ClutterActor*
clutter_disk_new (gint size, gint n_segments)
{
  ClutterActor        *disk;
  ClutterDiskPrivate  *priv;

  disk = g_object_new (CLUTTER_TYPE_DISK, NULL);
  priv = CLUTTER_DISK_GET_PRIVATE (disk);

  priv->n_segments = n_segments;
  priv->size       = size;

  return disk;
}

