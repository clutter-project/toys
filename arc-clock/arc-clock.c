#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <glib.h>

#include <clutter/clutter.h>
#include <clutter/x11/clutter-x11.h>

#define HAND_WIDTH      32

#define N_HANDS         3

enum
{
  HOURS,
  MINUTES,
  SECONDS
};

static double slices[N_HANDS] = { 0.0, };

static ClutterActor *hands[N_HANDS] = { NULL, };

static ClutterBehaviour *behaviours[N_HANDS] = { NULL, };

static const char *colors[N_HANDS] = {
  "#edd400",
  "#ce5c00",
  "#cc0000"
};

static gboolean hide_seconds = FALSE;
static gboolean enable_animations = FALSE;

static void
hand_paint (ClutterActor *hand)
{
  guint hand_id;

  hand_id = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (hand), "hand-id"));

  double end_angle = slices[hand_id];

  float radius = clutter_actor_get_width (hand) / 2.0 + 0.5;

  ClutterColor hand_color;
  clutter_rectangle_get_color (CLUTTER_RECTANGLE (hand), &hand_color);

  CoglColor fill_color;
  cogl_color_init_from_4ub (&fill_color,
                            hand_color.red,
                            hand_color.green,
                            hand_color.blue,
                            hand_color.alpha);

  cogl_set_source_color (&fill_color);

  cogl_path_move_to (radius, 0.0f);
  cogl_path_arc (radius, radius,
                 radius, radius,
                 -90.0,
                 -90.0 + end_angle * 180.0 / G_PI);
  cogl_path_arc (radius, radius,
                 radius - HAND_WIDTH, radius - HAND_WIDTH,
                 -90.0 + end_angle * 180.0 / G_PI,
                 -90.0);
  cogl_path_close ();
  cogl_path_fill ();

  g_signal_stop_emission_by_name (hand, "paint");
}

static void
on_stage_pre_paint (ClutterActor *actor)
{
  cogl_set_depth_test_enabled (TRUE);
}

static void
on_stage_post_paint (ClutterActor *actor)
{
  cogl_set_depth_test_enabled (FALSE);
}

static gboolean
update_slices (gpointer data G_GNUC_UNUSED)
{
  struct tm *now_tm;
  time_t now;

  time (&now);
  now_tm = localtime (&now);

  slices[SECONDS] = now_tm->tm_sec * G_PI / 30;
  slices[MINUTES] = now_tm->tm_min * G_PI / 30;
  slices[HOURS]   = (now_tm->tm_hour > 12 ? now_tm->tm_hour - 12
                                          : now_tm->tm_hour) * G_PI / 6;

  g_print ("%.3f hrs :: %.3f min :: %.3f sec\r",
           slices[HOURS],
           slices[MINUTES],
           slices[SECONDS]);

  for (int i = 0; i < N_HANDS; i++)
    clutter_actor_queue_redraw (hands[i]);

  return TRUE;
}

static GOptionEntry entries[] = {
  {
    "hide-seconds", 's',
    0,
    G_OPTION_ARG_NONE, &hide_seconds,
    "Hide the seconds hand", NULL
  },
  {
    "enable-animations", 'a',
    0,
    G_OPTION_ARG_NONE, &enable_animations,
    "Animate the arcs", NULL
  },
  { NULL }
};

int
main (int argc, char *argv[])
{
#if !CLUTTER_CHECK_VERSION (1, 3, 6)
#error "You need Clutter >= 1.3.6 to compile arc-clock."
#endif

  clutter_x11_set_use_argb_visual (TRUE);

  GError *error = NULL;
  clutter_init_with_args (&argc, &argv,
                          "Arc Clock",
                          entries,
                          NULL,
                          &error);

  ClutterActor *stage = clutter_stage_new ();
  clutter_stage_set_title (CLUTTER_STAGE (stage), "Arc Clock");
  clutter_stage_set_user_resizable (CLUTTER_STAGE (stage), TRUE);
  clutter_stage_set_use_alpha (CLUTTER_STAGE (stage), TRUE);
  clutter_actor_set_size (stage, 512, 512);
  clutter_actor_set_opacity (stage, 0);

  g_signal_connect (stage, "paint", G_CALLBACK (on_stage_pre_paint), NULL);
  g_signal_connect_after (stage, "paint", G_CALLBACK (on_stage_post_paint), NULL);
  g_signal_connect (stage, "destroy", G_CALLBACK (clutter_main_quit), NULL);

  ClutterColor color;

  clutter_color_from_string (&color, colors[HOURS]);
  hands[HOURS] = clutter_rectangle_new_with_color (&color);
  clutter_actor_set_size (hands[HOURS], 384, 384);
  clutter_actor_add_constraint (hands[HOURS], clutter_align_constraint_new (stage, CLUTTER_ALIGN_X_AXIS, 0.5));
  clutter_actor_add_constraint (hands[HOURS], clutter_align_constraint_new (stage, CLUTTER_ALIGN_Y_AXIS, 0.5));
  g_signal_connect (hands[HOURS], "paint", G_CALLBACK (hand_paint), NULL);
  g_object_set_data (G_OBJECT (hands[HOURS]), "hand-id", GUINT_TO_POINTER (HOURS));

  clutter_color_from_string (&color, colors[MINUTES]);
  hands[MINUTES] = clutter_rectangle_new_with_color (&color);
  clutter_actor_set_size (hands[MINUTES], 256, 256);
  clutter_actor_add_constraint (hands[MINUTES], clutter_align_constraint_new (stage, CLUTTER_ALIGN_X_AXIS, 0.5));
  clutter_actor_add_constraint (hands[MINUTES], clutter_align_constraint_new (stage, CLUTTER_ALIGN_Y_AXIS, 0.5));
  g_signal_connect (hands[MINUTES], "paint", G_CALLBACK (hand_paint), NULL);
  g_object_set_data (G_OBJECT (hands[MINUTES]), "hand-id", GUINT_TO_POINTER (MINUTES));

  clutter_color_from_string (&color, colors[SECONDS]);
  hands[SECONDS] = clutter_rectangle_new_with_color (&color);
  clutter_actor_set_size (hands[SECONDS], 128, 128);
  clutter_actor_add_constraint (hands[SECONDS], clutter_align_constraint_new (stage, CLUTTER_ALIGN_X_AXIS, 0.5));
  clutter_actor_add_constraint (hands[SECONDS], clutter_align_constraint_new (stage, CLUTTER_ALIGN_Y_AXIS, 0.5));
  g_signal_connect (hands[SECONDS], "paint", G_CALLBACK (hand_paint), NULL);
  g_object_set_data (G_OBJECT (hands[SECONDS]), "hand-id", GUINT_TO_POINTER (SECONDS));

  ClutterTimeline *timeline = clutter_timeline_new (5000);
  clutter_timeline_set_loop (timeline, TRUE);

  if (enable_animations)
    {
      ClutterAlpha *alpha = clutter_alpha_new_full (timeline, CLUTTER_LINEAR);
      g_object_unref (timeline);

      behaviours[HOURS] = clutter_behaviour_rotate_new (alpha, CLUTTER_Y_AXIS,
                                                        CLUTTER_ROTATE_CW,
                                                        0.0, 0.0);
      clutter_behaviour_rotate_set_center (CLUTTER_BEHAVIOUR_ROTATE (behaviours[HOURS]), 196, 0, 0);
      clutter_behaviour_apply (behaviours[HOURS], hands[HOURS]);

      behaviours[MINUTES] = clutter_behaviour_rotate_new (alpha, CLUTTER_Y_AXIS,
                                                          CLUTTER_ROTATE_CCW,
                                                          0.0, 0.0);
      clutter_behaviour_rotate_set_center (CLUTTER_BEHAVIOUR_ROTATE (behaviours[MINUTES]), 128, 0, 0);
      clutter_behaviour_apply (behaviours[MINUTES], hands[MINUTES]);
    }

  clutter_container_add (CLUTTER_CONTAINER (stage),
                         hands[HOURS],
                         hands[MINUTES],
                         hands[SECONDS],
                         NULL);

  if (hide_seconds)
    clutter_actor_hide (hands[SECONDS]);

  g_timeout_add_seconds ((hide_seconds ? 60 : 1), update_slices, NULL);

  update_slices (NULL);

  clutter_actor_show (stage);

  if (enable_animations)
    clutter_timeline_start (timeline);

  clutter_main ();

  return EXIT_SUCCESS;
}
