
#include <clutter/clutter.h>
#include <math.h>

#define N_CIRCLES 3     /* number of circles */
#define CIRCLE_W 128    /* width */
#define CIRCLE_G 32     /* gap */
#define CIRCLE_S 3      /* segments */
#define SCREEN_W 640
#define SCREEN_H 480

#ifndef CLUTTER_ANGLE_FROM_RAD
#define CLUTTER_ANGLE_FROM_RAD(x) ((x) * 180.0 / M_PI)
#endif

static void
circle_paint_cb (ClutterActor *actor)
{
  const CoglColor fill_color = { 0xff, 0xff, 0xff, 0x80 };
  gint i;
  gdouble angle;
  guint radius = clutter_actor_get_width (actor)/2;

  cogl_set_source_color (&fill_color);

  angle = *((gdouble *)g_object_get_data (G_OBJECT (actor), "angle"));
  for (i = 0; i < CIRCLE_S; i++, angle += (2.0*M_PI)/(gdouble)CIRCLE_S)
    {
      gdouble angle2 = angle + ((2.0*M_PI)/(gdouble)CIRCLE_S)/2.0;
      cogl_path_move_to (((radius - CIRCLE_W) * cos (angle)) + radius,
                         ((radius - CIRCLE_W) * sin (angle)) + radius);
      cogl_path_arc (radius, radius, radius, radius,
                     CLUTTER_ANGLE_FROM_RAD (angle),
                     CLUTTER_ANGLE_FROM_RAD (angle2));
      cogl_path_line_to (((radius - CIRCLE_W) * cos (angle2)) + radius,
                         ((radius - CIRCLE_W) * sin (angle2)) + radius);
      cogl_path_arc (radius, radius, radius - CIRCLE_W, radius - CIRCLE_W,
                     CLUTTER_ANGLE_FROM_RAD (angle2),
                     CLUTTER_ANGLE_FROM_RAD (angle));
      cogl_path_close ();
      cogl_path_fill ();
    }
}

int
main (int argc, char **argv)
{
  const ClutterColor transp = { 0x00, 0x00, 0x00, 0x00 };
  const ClutterColor bg_color = { 0xe0, 0xf2, 0xfc, 0xff };
  ClutterTimeline *timeline;
  ClutterActor *stage;
  gint i;
  
  clutter_init (&argc, &argv);
  
  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, SCREEN_W, SCREEN_H);
  clutter_stage_set_color (CLUTTER_STAGE (stage), &bg_color);
  
  timeline = clutter_timeline_new (5000);
  clutter_timeline_set_loop (timeline, TRUE);

  for (i = 0; i < N_CIRCLES; i++)
    {
      gint size;
      gdouble *angle;
      ClutterActor *actor;
      ClutterAlpha *alpha;
      ClutterBehaviour *behaviour;
      
      actor = clutter_rectangle_new_with_color (&transp);
      
      size = (i+1) * (CIRCLE_W + CIRCLE_G) * 2;
      clutter_actor_set_size (actor, size, size);
      clutter_actor_set_position (actor, SCREEN_W - size/2,
                                  SCREEN_H - size/2);
      
      clutter_container_add_actor (CLUTTER_CONTAINER (stage), actor);
      
      angle = g_slice_new (gdouble);
      *angle = g_random_double_range (0.0, 90.0);
      g_object_set_data (G_OBJECT (actor), "angle", angle);
      g_signal_connect (actor, "paint", G_CALLBACK (circle_paint_cb), NULL);
      
      /* Animate */
      alpha = clutter_alpha_new_full (timeline, CLUTTER_LINEAR);
      behaviour = clutter_behaviour_rotate_new (alpha, CLUTTER_Z_AXIS,
                                                (i % 2) ? CLUTTER_ROTATE_CW
                                                        : CLUTTER_ROTATE_CCW,
                                                0.0, 0.0);
      clutter_behaviour_rotate_set_center (CLUTTER_BEHAVIOUR_ROTATE (behaviour),
                                           size/2, size/2, 0);
      clutter_behaviour_apply (behaviour, actor);
    }
  
  clutter_actor_show_all (stage);
  
  clutter_timeline_start (timeline);
  
  clutter_main ();
  
  return 0;
}

