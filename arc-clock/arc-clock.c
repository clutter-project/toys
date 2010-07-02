#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <glib.h>

#include <clutter/clutter.h>
#include <clutter/x11/clutter-x11.h>

#define HAND_WIDTH      24

enum
{
  PM,

  SECONDS,
  MINUTES,
  HOURS,

  DAY,
  MONTH,

  N_HANDS
};

static double slices[N_HANDS] = { 0.0, };

static ClutterActor *hands[N_HANDS] = { NULL, };

static const char *colors[N_HANDS] = {
  "#4e9a06",

  "#edd400",
  "#ce5c00",
  "#cc0000",

  "#3465a4",
  "#75507b"
};

static gboolean hide_date = FALSE;
static gboolean hide_seconds = FALSE;

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
  slices[HOURS]   = (now_tm->tm_hour >= 12 ? now_tm->tm_hour - 12
                                           : now_tm->tm_hour) * G_PI / 6;
  slices[DAY]     = now_tm->tm_mday * G_PI / 15;
  slices[MONTH]   = (now_tm->tm_mon + 1) * G_PI / 6;

  for (int i = SECONDS; i < N_HANDS; i++)
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
    "hide-date", 'd',
    0,
    G_OPTION_ARG_NONE, &hide_date,
    "Hide the date hands", NULL
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
  clutter_actor_set_opacity (stage, 0);

  g_signal_connect (stage, "paint", G_CALLBACK (on_stage_pre_paint), NULL);
  g_signal_connect_after (stage, "paint", G_CALLBACK (on_stage_post_paint), NULL);
  g_signal_connect (stage, "destroy", G_CALLBACK (clutter_main_quit), NULL);

  for (int i = SECONDS; i < N_HANDS; i++)
    {
      ClutterColor color;

      clutter_color_from_string (&color, colors[i]);
      hands[i] = clutter_rectangle_new_with_color (&color);
      clutter_actor_set_size (hands[i], (HAND_WIDTH * 3.0) * i, (HAND_WIDTH * 3.0) * i);
      clutter_actor_add_constraint (hands[i], clutter_align_constraint_new (stage, CLUTTER_ALIGN_X_AXIS, 0.5));
      clutter_actor_add_constraint (hands[i], clutter_align_constraint_new (stage, CLUTTER_ALIGN_Y_AXIS, 0.5));
      g_signal_connect (hands[i], "paint", G_CALLBACK (hand_paint), NULL);
      g_object_set_data (G_OBJECT (hands[i]), "hand-id", GUINT_TO_POINTER (i));
      clutter_container_add_actor (CLUTTER_CONTAINER (stage), hands[i]);
    }

  if (hide_seconds)
    clutter_actor_hide (hands[SECONDS]);

  if (hide_date)
    {
      clutter_actor_hide (hands[DAY]);
      clutter_actor_hide (hands[MONTH]);
    }

  g_timeout_add_seconds ((hide_seconds ? 60 : 1), update_slices, NULL);

  update_slices (NULL);

  clutter_actor_show (stage);

  clutter_main ();

  return EXIT_SUCCESS;
}
