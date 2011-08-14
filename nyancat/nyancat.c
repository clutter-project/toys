#include <string.h>

#include <clutter/clutter.h>

#define SCALE_X_VARIANCE 0.3
#define SCALE_Y_VARIANCE 0.2
#define ROT_Z_VARIANCE 0.1
#define ALPHA_VARIANCE 0.5
#define ANIM_LENGTH 10000
#define ANIM_VARIANCE 0.3
#define OPACITY 0.60
#define OPACITY_VARIANCE 0.33
#define HEIGHT 0.85
#define HEIGHT_VARIANCE 0.2
#define HEIGHT_DROP 1.7

static void
legs_state_completed (ClutterState *state, gpointer data)
{
  const gchar *state_name = clutter_state_get_state (state);

  if (!strcmp (state_name, "legs-up"))
    clutter_state_set_state (state, "legs-down");
  else
    clutter_state_set_state (state, "legs-up");
}

static void
head_state_completed (ClutterState *state, gpointer data)
{
  const gchar *state_name = clutter_state_get_state (state);

  if (!strcmp (state_name, "head-up"))
    clutter_state_set_state (state, "head-right");
  else if (!strcmp (state_name, "head-right"))
    clutter_state_set_state (state, "head-down");
  else if (!strcmp (state_name, "head-down"))
    clutter_state_set_state (state, "head-left");
  else if (!strcmp (state_name, "head-left"))
    clutter_state_set_state (state, "head-up");
}

static inline gdouble
get_variance (gdouble value, gdouble constant)
{
  return value + value * (2.0 * g_random_double () - 1.0) * constant;
}

static gboolean
create_cloud(ClutterActor *cloud_texture)
{
  ClutterPerspective perspective;
  ClutterAnimator *animator;
  ClutterTimeline *timeline;
  guint8 opacity;
  gfloat cloud_width;
  gfloat stage_width, stage_height;

  ClutterActor *stage = clutter_stage_get_default ();
  ClutterActor *cloud = clutter_clone_new (cloud_texture);

  stage_width = clutter_actor_get_width (stage);
  stage_height = clutter_actor_get_height (stage);
  cloud_width = clutter_actor_get_width (cloud);

  clutter_actor_set_scale_with_gravity (cloud,
                                        get_variance (1, SCALE_X_VARIANCE),
                                        get_variance (1, SCALE_Y_VARIANCE),
                                        CLUTTER_GRAVITY_CENTER);
  clutter_actor_set_z_rotation_from_gravity (cloud,
                                             get_variance (360, ROT_Z_VARIANCE),
                                             CLUTTER_GRAVITY_CENTER);
  clutter_actor_set_y (cloud,
    g_random_double_range (-clutter_actor_get_height (cloud),
                           stage_height));

  clutter_container_add_actor (CLUTTER_CONTAINER (stage), cloud);
  clutter_actor_lower_bottom (cloud);

  clutter_actor_show (cloud);

  clutter_stage_get_perspective (CLUTTER_STAGE (stage), &perspective);

  animator = clutter_animator_new();
  opacity = (guint8)(get_variance (OPACITY, OPACITY_VARIANCE) * 255.0);
  clutter_animator_set (animator,
                        cloud, "x", CLUTTER_LINEAR, 0.0, stage_width,
                        cloud, "opacity", CLUTTER_LINEAR, 0.0, 0,
                        cloud, "opacity", CLUTTER_EASE_IN_CUBIC, 0.15, opacity,
                        cloud, "opacity", CLUTTER_LINEAR, 0.85, opacity,
                        cloud, "opacity", CLUTTER_EASE_OUT_CUBIC, 1.0, 0,
                        cloud, "x", CLUTTER_LINEAR, 1.0, -cloud_width,
                        NULL);

  clutter_animator_set_duration (animator,
                                 get_variance (ANIM_LENGTH, ANIM_VARIANCE));
  timeline = clutter_animator_get_timeline (animator);
  g_signal_connect_swapped (timeline, "completed",
                            G_CALLBACK (clutter_actor_destroy), cloud);

  g_object_weak_ref (G_OBJECT (cloud), (GWeakNotify)g_object_unref, animator);

  clutter_animator_start (animator);

  return TRUE;
}

int
main (int argc, char *argv[])
{
  ClutterColor stage_color = { 0x1C, 0x41, 0x70, 0xff };
  ClutterActor *nyancat, *stage, *cloud;
  ClutterState *legs_states, *head_states;
  ClutterScript *script;
  guint source;
  GError *error = NULL;

  if (argc < 2)
    {
      g_warning ("%s file.json", argv[0]);
      return 1;
    }

  if (clutter_init (&argc, &argv) != CLUTTER_INIT_SUCCESS)
    return 1;

  stage = clutter_stage_get_default ();
  clutter_stage_set_title (CLUTTER_STAGE (stage), "Clutter Nyan Cat");
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);
  clutter_actor_set_size (stage, 800, 600);

  script = clutter_script_new ();

  clutter_script_load_from_file (script, argv[1], &error);
  if (error)
    {
      g_warning ("Error while loading '%s': %s", argv[1], error->message);
      g_error_free (error);
      return 1;
    }

  /**/
  nyancat = CLUTTER_ACTOR (clutter_script_get_object (script, "nyancat"));
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), nyancat);
  clutter_actor_raise_top (nyancat);
  legs_states = CLUTTER_STATE (clutter_script_get_object (script,
                                                          "legs-tail-states"));
  head_states = CLUTTER_STATE (clutter_script_get_object (script,
                                                          "head-states"));
  g_signal_connect (legs_states, "completed",
                    G_CALLBACK (legs_state_completed), NULL);
  g_signal_connect (head_states, "completed",
                    G_CALLBACK (head_state_completed), NULL);

  /**/
  if (!(cloud = clutter_texture_new_from_file ("star.png", &error)))
    {
      g_warning ("Error loading image: %s", error->message);
      g_error_free (error);
      return -1;
    }
  clutter_actor_set_position (cloud,
                              -clutter_actor_get_width (cloud),
                              -clutter_actor_get_height (cloud));
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), cloud);

  source = g_timeout_add_full (CLUTTER_PRIORITY_REDRAW, 1000,
                               (GSourceFunc)create_cloud,
                               cloud, NULL);

  clutter_actor_show_all (stage);

  clutter_state_set_state (legs_states, "legs-up");
  clutter_state_set_state (head_states, "head-up");

  clutter_main ();

  g_source_remove (source);

  return 0;
}
