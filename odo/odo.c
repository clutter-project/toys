#include <math.h>
#include <clutter/clutter.h>
#include "odo-distort-funcs.h"
#include "odo-texture.h"

struct distort_data
{
  ClutterActor    *odo;
  OdoDistortData   data;
  ClutterTimeline *timeline;
  ClutterAlpha    *alpha;
};

static gint func = 0;

static void
new_frame_cb (ClutterTimeline *timeline,
              gint             msecs,
              gpointer         data)
{
  /* Set the turn value to the alpha value and redraw */
  struct distort_data *d = data;
  d->data.turn = clutter_alpha_get_alpha (d->alpha);
  d->data.amplitude = d->data.turn;

  odo_texture_invalidate (ODO_TEXTURE (d->odo));
}

static void
completed_cb (ClutterTimeline *timeline,
              gint             msecs,
              gpointer         data)
{
  struct distort_data *d = data;

  /* Reverse direction and start again */
  ClutterTimelineDirection dir = clutter_timeline_get_direction (timeline);
  clutter_timeline_set_direction (timeline, 1 - dir);

  if (dir == CLUTTER_TIMELINE_BACKWARD)
    {
      switch (func)
        {
        case 0:
          odo_texture_set_callback (ODO_TEXTURE (d->odo),
                                    bowtie_func,
                                    &d->data);
          func = 1;
          break;

        case 1:
          odo_texture_set_callback (ODO_TEXTURE (d->odo),
                                    cloth_func,
                                    &d->data);
          func = 2;
          break;

        case 2:
          odo_texture_set_callback (ODO_TEXTURE (d->odo),
                                    page_turn_func,
                                    &d->data);
          func = 0;
          break;
        }
    }

  clutter_timeline_start (timeline);
}

int
main (int argc, char *argv[])
{
  ClutterActor        *stage;
  ClutterColor         stage_color = { 0xcc, 0xcc, 0xcc, 0xff };
  struct distort_data  data;

  if (argc < 2)
    {
      printf ("Usage: %s <filename> [filename]\n", argv[0]);
      return 1;
    }

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();

  /* Quit on key-press */
  g_signal_connect (stage, "key-press-event",
                    G_CALLBACK (clutter_main_quit), NULL);

  /* Make a fullscreen stage */
  clutter_stage_set_color (CLUTTER_STAGE (stage),
                           &stage_color);
  clutter_stage_set_fullscreen (CLUTTER_STAGE (stage), TRUE);

  /* Create the texture and set the deformation callback */
  data.odo = odo_texture_new_from_files (argv[1], (argc > 2) ? argv[2] : NULL);
  odo_texture_set_callback (ODO_TEXTURE (data.odo), page_turn_func, &data.data);

  /* Make the subdivision dependent on image size */
  odo_texture_set_resolution (ODO_TEXTURE (data.odo),
                              clutter_actor_get_width (data.odo) / 10,
                              clutter_actor_get_height (data.odo) / 10);

  /* Put it in the centre of the stage and add a jaunty angle */
  clutter_actor_set_rotation (data.odo, CLUTTER_Y_AXIS, 15, 0, 0, 0);
  clutter_actor_set_rotation (data.odo, CLUTTER_X_AXIS, 15, 0, 0, 0);
  clutter_actor_set_position (data.odo,
                              (clutter_actor_get_width (stage) -
                               clutter_actor_get_width (data.odo)) / 2,
                              (clutter_actor_get_height (stage) -
                               clutter_actor_get_height (data.odo)) / 2);
  clutter_actor_set_depth (data.odo, -300.0);

  /* Add it to the stage */
  clutter_container_add (CLUTTER_CONTAINER (stage), data.odo, NULL);

  /* Fill in the data required for the animation */
  data.timeline = clutter_timeline_new (5000);
  data.alpha = clutter_alpha_new_full (data.timeline,
                                       CLUTTER_EASE_IN_OUT_SINE);
  data.data.turn = 0;
  data.data.radius = clutter_actor_get_width (data.odo) / 18;
  data.data.angle = G_PI/6;
  data.data.amplitude = 1.0;

  /* Connect to timeline signals for progressing animation */
  g_signal_connect (data.timeline, "new-frame",
                    G_CALLBACK (new_frame_cb), &data);
  g_signal_connect (data.timeline, "completed",
                    G_CALLBACK (completed_cb), &data);

  /* Begin */
  clutter_actor_show (stage);
  clutter_timeline_start (data.timeline);
  clutter_main();

  return 0;
}

