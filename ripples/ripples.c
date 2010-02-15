#include <stdlib.h>
#include <cogl/cogl.h>
#include <clutter/clutter.h>

#define RIPPLE_S 3000    /* speed */
#define RIPPLE_W 8       /* width */
#define RIPPLE_G 2       /* gap */
#define RIPPLE_N 6       /* Max amount of ripple circles */
#define RIPPLE_MIND 500  /* Minimum delay between ripples */
#define RIPPLE_MAXD 2000 /* Maximum delay */

#define SCREEN_W 640
#define SCREEN_H 480

static void
circle_paint_cb (ClutterActor *actor)
{
  CoglColor fill_color;
  float radius = clutter_actor_get_width (actor) / 2;

  cogl_color_set_from_4ub (&fill_color,
                           255,
                           255,
                           255,
                           clutter_actor_get_paint_opacity (actor));

  cogl_set_source_color (&fill_color);
  cogl_path_move_to (radius, radius);
  cogl_path_arc (radius, radius,
                 radius, radius,
                 0.0, 360.0);
  cogl_path_line_to (radius - RIPPLE_W / 2, radius);
  cogl_path_arc (radius, radius,
                 radius - RIPPLE_W / 2, radius - RIPPLE_W / 2,
                 0.0,
                 360.0);
  cogl_path_close ();
  cogl_path_fill ();
}

void
ripple (ClutterActor *stage,
        gfloat        x,
        gfloat        y)
{
  const ClutterColor transp = { 0x00, 0x00, 0x00, 0x00 };
  gfloat scale_x, scale_y;
  gint i, n;

  n = g_random_int_range (1, RIPPLE_N);

  scale_x = clutter_actor_get_width (stage) / RIPPLE_W,
  scale_y = clutter_actor_get_width (stage) / RIPPLE_W;

  for (i = 0; i < n; i++)
    {
      ClutterActor *actor = clutter_rectangle_new_with_color (&transp);
      gfloat size;

      size = ((RIPPLE_W * 2) * (i + 1)) + (RIPPLE_G * i);
      clutter_actor_set_size (actor, size, size);
      clutter_actor_set_anchor_point_from_gravity (actor, CLUTTER_GRAVITY_CENTER);
      clutter_actor_set_position (actor, x, y);
      clutter_actor_set_opacity (actor, 0x80);

      g_signal_connect (actor, "paint", G_CALLBACK (circle_paint_cb), NULL);

      clutter_container_add_actor (CLUTTER_CONTAINER (stage), actor);

      clutter_actor_animate (actor, CLUTTER_EASE_OUT_CUBIC, RIPPLE_S / 2,
                             "scale-x", scale_x,
                             "scale-y", scale_y,
                             "opacity", 0,
                             "signal-swapped-after::completed",
                               clutter_actor_destroy, actor,
                             NULL);
    }
}

static gboolean
stage_clicked_cb (ClutterActor *stage, ClutterEvent *event)
{
  gfloat event_x, event_y;

  clutter_event_get_coords (event, &event_x, &event_y);
  ripple (stage, event_x, event_y);

  return TRUE;
}

static gboolean
random_ripple_cb (gpointer data)
{
  ClutterActor *stage = data;

  ripple (stage,
          g_random_double_range (0, clutter_actor_get_width (stage)),
          g_random_double_range (0, clutter_actor_get_height (stage)));

  g_timeout_add (g_random_int_range (RIPPLE_MIND, RIPPLE_MAXD),
                 random_ripple_cb,
                 stage);

  return FALSE;
}

int
main (int argc, char **argv)
{
  const ClutterColor bg_color = { 0xe0, 0xf2, 0xfc, 0xff };
  ClutterActor *stage;

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, SCREEN_W, SCREEN_H);
  clutter_stage_set_color (CLUTTER_STAGE (stage), &bg_color);

  clutter_actor_show (stage);

  random_ripple_cb (stage);

  g_signal_connect (stage,
                    "button-press-event", G_CALLBACK (stage_clicked_cb),
                    NULL);

  clutter_main ();
  
  return EXIT_SUCCESS;
}

