#include <clutter/clutter.h>
#include "custom-cursor.h"
#include <cogl/cogl.h>
#include <string.h>

G_DEFINE_TYPE (CustomCursor, custom_cursor, CLUTTER_TYPE_ACTOR);

enum
{
  PROP_0,
  PROP_NORMAL,
  PROP_PRESSED,
  PROP_HOT_X,
  PROP_HOT_Y
};


#define CUSTOM_CURSOR_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CUSTOM_TYPE_CURSOR, CustomCursorPrivate))


typedef struct
{
  gint            id;
  gint            x;
  gint            y;
  gint            pressed;
  CustomCursorState state;
} DeviceInfo;

#define MAX_DEVICES 40

struct _CustomCursorPrivate
{
  gint          hot_x;
  gint          hot_y;

  ClutterActor *normal;
  ClutterActor *pressed;

  DeviceInfo    device[MAX_DEVICES];
  gint          device_count;
};

static void
custom_cursor_allocate (ClutterActor          *self,
                      const ClutterActorBox *box,
                      gboolean               origin_changed)
{
  CustomCursorPrivate *priv = CUSTOM_CURSOR (self)->priv;

  /* chain up to set actor->allocation */
  CLUTTER_ACTOR_CLASS (custom_cursor_parent_class)->allocate (self, box,
                                                            origin_changed);

  /* Make sure children also get there allocations */
  if (priv->normal)
    clutter_actor_allocate_preferred_size (priv->normal, origin_changed);

  if (priv->pressed)
    clutter_actor_allocate_preferred_size (priv->pressed, origin_changed);

}


static void
custom_cursor_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  CustomCursor        *cursor;
  CustomCursorPrivate *priv;

  cursor = CUSTOM_CURSOR(object);
  priv = cursor->priv;

  switch (prop_id) 
    {
    case PROP_HOT_X:
      priv->hot_x = g_value_get_int (value);
      break;
    case PROP_HOT_Y:
      priv->hot_y = g_value_get_int (value);
      break;
    case PROP_NORMAL:
      if (priv->normal)
        g_object_unref (priv->normal);
      priv->normal = g_value_dup_object (value);
      clutter_actor_set_parent (priv->normal, CLUTTER_ACTOR (cursor));
      break;
    case PROP_PRESSED:
      if (priv->pressed)
        g_object_unref (priv->pressed);
      priv->pressed = g_value_dup_object (value);
      clutter_actor_set_parent (priv->pressed, CLUTTER_ACTOR (cursor));
      clutter_actor_queue_relayout (CLUTTER_ACTOR (cursor));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
custom_cursor_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  CustomCursorPrivate *priv = CUSTOM_CURSOR (object)->priv;

  switch (prop_id) 
    {
    case PROP_HOT_X:
      g_value_set_int (value, priv->hot_x);
      break;
    case PROP_HOT_Y:
      g_value_set_int (value, priv->hot_y);
      break;
    case PROP_NORMAL:
      g_value_set_object (value, priv->normal);
      break;
    case PROP_PRESSED:
      g_value_set_object (value, priv->pressed);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    } 
}

static void
custom_cursor_pick (ClutterActor       *self,
                  const ClutterColor *color)
{
  /* The cursor is not pickable at all 
   */
}

static void
custom_cursor_paint (ClutterActor       *self)
{
  CustomCursorPrivate *priv = CUSTOM_CURSOR (self)->priv;
  gint no;


  for (no=0; no<priv->device_count; no++)
    {
      cogl_push_matrix ();

      cogl_translate (priv->device[no].x, priv->device[no].y, 0);
      cogl_translate (-priv->hot_x, -priv->hot_y, 0);

      switch (priv->device[no].state)
        {
          case CUSTOM_CURSOR_NORMAL:
            if (priv->normal)
              clutter_actor_paint (priv->normal);
            else
              {
#define LENGTH 30
#define GAP 15
                cogl_color (&((ClutterColor){0x00, 0x00, 0x00, 0x77}));
                cogl_rectangle (-LENGTH-GAP, -2, LENGTH, 4);
                cogl_rectangle (GAP, -2, LENGTH, 4);
                cogl_rectangle (-2, -LENGTH-GAP, 4, LENGTH);
                cogl_rectangle (-2, GAP, 4, LENGTH);

                cogl_color (&((ClutterColor){0xff, 0xff, 0xff, 0xaa}));
                cogl_rectangle (-LENGTH-GAP, -1, LENGTH, 2);
                cogl_rectangle (GAP, -1, LENGTH, 2);
                cogl_rectangle (-1, -LENGTH-GAP, 2, LENGTH);
                cogl_rectangle (-1, GAP, 2, LENGTH);
            }
            break;
          case CUSTOM_CURSOR_PRESSED:
            if (priv->pressed)
              clutter_actor_paint (priv->pressed);
            else
              {
                cogl_color (&((ClutterColor){0xff, 0xff, 0xff, 0x77}));
                cogl_path_ellipse (0, 0, CLUTTER_UNITS_FROM_INT (52),
                                         CLUTTER_UNITS_FROM_INT (52));
                cogl_path_arc (0, 0, CLUTTER_UNITS_FROM_INT (38),
                                     CLUTTER_UNITS_FROM_INT (38),
                                     0, 1027);
                cogl_path_fill ();

                cogl_color (&((ClutterColor){0x45, 0x66, 0xff, 0x55}));
                cogl_path_ellipse (0, 0, CLUTTER_UNITS_FROM_INT (50),
                                         CLUTTER_UNITS_FROM_INT (50));
                cogl_path_arc (0, 0, CLUTTER_UNITS_FROM_INT (40),
                                     CLUTTER_UNITS_FROM_INT (40),
                                     0, 1027);
                cogl_path_fill ();
              }
        }
      cogl_pop_matrix ();
    }
}



static void
custom_cursor_class_init (CustomCursorClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  gobject_class->set_property = custom_cursor_set_property;
  gobject_class->get_property = custom_cursor_get_property;
  actor_class->pick = custom_cursor_pick;
  actor_class->paint = custom_cursor_paint;
  actor_class->allocate = custom_cursor_allocate;

  g_type_class_add_private (gobject_class, sizeof (CustomCursorPrivate));

#define PARAM_FLAGS (G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |\
                     G_PARAM_STATIC_BLURB |                     \
                     G_PARAM_READABLE | G_PARAM_WRITABLE)

  g_object_class_install_property (gobject_class,
                                   PROP_HOT_X,
                                   g_param_spec_int ("hot-x",
                                                     "hot-Y",
                                                     "x coordinate of cursor tip",
                                                     0, G_MAXINT,
                                                     0,
                                                     PARAM_FLAGS));

  g_object_class_install_property (gobject_class,
                                   PROP_HOT_Y,
                                   g_param_spec_int ("hot-y",
                                                     "hot Y",
                                                     "Y coordinate of cursor tip",
                                                     0, G_MAXINT,
                                                     0,
                                                     PARAM_FLAGS));


  g_object_class_install_property (gobject_class,
                                   PROP_NORMAL,
                                   g_param_spec_object ("normal",
                                                     "Normal",
                                                     "Actor to show when moving the mouse around",
                                                     CLUTTER_TYPE_ACTOR,
                                                     PARAM_FLAGS));

  g_object_class_install_property (gobject_class,
                                   PROP_PRESSED,
                                   g_param_spec_object ("pressed",
                                                     "Pressed",
                                                     "Actor to show when a mouse button is pressed",
                                                     CLUTTER_TYPE_ACTOR,
                                                     PARAM_FLAGS));

#undef PARAM_FLAGS
}

static void
custom_cursor_init (CustomCursor *self)
{
  CustomCursorPrivate *priv;

  self->priv = priv = CUSTOM_CURSOR_GET_PRIVATE (self);

  priv->normal = NULL;
  priv->pressed = NULL;
  priv->hot_x = 0;
  priv->hot_y = 0;
  priv->device_count = 0;
}

static gint
get_device_no (CustomCursorPrivate *priv,
               gint device_id)
{
  gint i;
  for (i=0; i<priv->device_count; i++)
    {
      if (priv->device[i].id == device_id)
          return i;
    }
  g_assert (priv->device_count+1 < MAX_DEVICES);
  priv->device[priv->device_count].id = device_id;
  priv->device_count++;
  return priv->device_count-1;
}

static gboolean soft_cursor_capture (ClutterActor *stage,
                                     ClutterEvent *event,
                                     gpointer      data)
{
  CustomCursor        *self = CUSTOM_CURSOR (data);
  CustomCursorPrivate *priv = self->priv;

  switch (clutter_event_type (event))
    {
      case CLUTTER_MOTION:
      case CLUTTER_BUTTON_PRESS:
      case CLUTTER_BUTTON_RELEASE:
        {
          gint x, y;
          gint id = clutter_event_get_device_id (event);
          gint no = get_device_no (priv, id);
          clutter_event_get_coords (event, &x, &y);

#if 0
          gchar c;

          switch (clutter_event_type (event))
            {
              case CLUTTER_MOTION:
                c = 'm';
                break;
              case CLUTTER_BUTTON_PRESS:
                c = 'p';
                break;
              case CLUTTER_BUTTON_RELEASE:
                c = 'r';
                break;
              default:
                c = '?';
            }
  
          g_print ("%c%c%c%c %i,%i\n", 
              id!=0?' ':c,
              id!=1?' ':c,
              id!=2?' ':c,
              id!=3?' ':c, 
              id!=4?' ':c, 
              x, y);
#endif
          
          if (clutter_event_type (event) == CLUTTER_BUTTON_PRESS)
            {
              priv->device[no].state = CUSTOM_CURSOR_PRESSED;
            }
          else if (clutter_event_type (event) == CLUTTER_BUTTON_RELEASE)
            {
              priv->device[no].state = CUSTOM_CURSOR_NORMAL;
            }
          custom_cursor (x,y, id);
        }
      default:
        break;
    }
  return FALSE;
}

/* always returns the same cursor */
ClutterActor *
custom_cursor (gint x,
             gint y,
             gint device_id)
{
  static ClutterActor *cursor = NULL;
  CustomCursorPrivate *priv;

  if (!cursor)
    {
      ClutterActor *stage = clutter_stage_get_default ();
      cursor = g_object_new (CUSTOM_TYPE_CURSOR, NULL);

/*if(0)      clutter_x11_enable_xinput ();*/
      priv = CUSTOM_CURSOR (cursor)->priv;
      priv->device_count = 0;

      clutter_container_add_actor (CLUTTER_CONTAINER (stage), cursor);
      clutter_actor_show (cursor);
      g_signal_connect (stage, "captured-event",
                        G_CALLBACK (soft_cursor_capture), cursor);
      if(0)clutter_stage_hide_cursor (CLUTTER_STAGE (stage));
    }
  else
    {
      priv = CUSTOM_CURSOR (cursor)->priv;
    }

  {
    gint no = get_device_no (priv, device_id);
    priv->device[no].x = x;
    priv->device[no].y = y;
  }

  clutter_actor_queue_redraw (cursor);
  clutter_actor_raise_top (cursor);
  return cursor;
}



