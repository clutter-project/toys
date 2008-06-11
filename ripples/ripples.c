
#include <clutter/clutter.h>

#define RIPPLE_S 3000    /* speed */
#define RIPPLE_W 8       /* width */
#define RIPPLE_G 2       /* gap */
#define RIPPLE_N 6       /* Max amount of ripple circles */
#define RIPPLE_MIND 150  /* Minimum delay between ripples */
#define RIPPLE_MAXD 2000 /* Maximum delay */
#define RIPPLE_WX CLUTTER_INT_TO_FIXED(RIPPLE_W)

#define SCREEN_W 640
#define SCREEN_H 480

static ClutterEffectTemplate *template;

static void
circle_paint_cb (ClutterActor *actor)
{
  ClutterColor fill_color = { 0xff, 0xff, 0xff, 0xff };
  ClutterFixed radius =
    CLUTTER_UNITS_TO_FIXED (clutter_actor_get_widthu (actor))/2;

  fill_color.alpha = clutter_actor_get_opacity (actor);
  cogl_color (&fill_color);
  cogl_path_move_to (radius, radius);
  cogl_path_arc (radius, radius, radius, radius,
                 CLUTTER_ANGLE_FROM_DEG(0),
                 CLUTTER_ANGLE_FROM_DEG(360));
  cogl_path_line_to (radius-RIPPLE_WX/2, radius);
  cogl_path_arc (radius, radius, radius-RIPPLE_WX/2, radius-RIPPLE_WX/2,
                 CLUTTER_ANGLE_FROM_DEG(0),
                 CLUTTER_ANGLE_FROM_DEG(360));
  cogl_path_close ();
  cogl_path_fill ();
}

void
ripple (ClutterActor *stage, gint x, gint y)
{
  const ClutterColor transp = { 0x00, 0x00, 0x00, 0x00 };
  gint i, n;
  
  n = g_random_int_range (1, RIPPLE_N);
  for (i = 0; i < n; i++)
    {
      gint size;
      ClutterActor *actor = clutter_rectangle_new_with_color (&transp);
      
      size = ((RIPPLE_W * 2) * (i + 1)) + (RIPPLE_G * i);
      clutter_actor_set_size (actor, size, size);
      clutter_actor_set_anchor_point_from_gravity (actor,
                                                   CLUTTER_GRAVITY_CENTER);
      clutter_actor_set_position (actor, x, y);
      clutter_actor_set_opacity (actor, 0x80);
      
      g_signal_connect (actor, "paint", G_CALLBACK (circle_paint_cb), NULL);

      clutter_container_add_actor (CLUTTER_CONTAINER (stage), actor);
      
      clutter_effect_scale (template, actor,
                            CLUTTER_STAGE_WIDTH () / RIPPLE_W,
                            CLUTTER_STAGE_WIDTH () / RIPPLE_W,
                            NULL, NULL);
      clutter_effect_fade (template, actor,
                           0x00,
                           (ClutterEffectCompleteFunc)clutter_actor_destroy,
                           NULL);
    }
}

static gboolean
stage_clicked_cb (ClutterActor *stage, ClutterButtonEvent *event)
{
  ripple (stage, event->x, event->y);
  return TRUE;
}

static gboolean
random_ripple_cb (ClutterActor *stage)
{
  ripple (stage,
          g_random_int_range (0, CLUTTER_STAGE_WIDTH ()),
          g_random_int_range (0, CLUTTER_STAGE_HEIGHT ()));
  g_timeout_add (g_random_int_range (RIPPLE_MIND, RIPPLE_MAXD),
                 (GSourceFunc)random_ripple_cb, stage);
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
  
  template = clutter_effect_template_new_for_duration (RIPPLE_S,
                                                       CLUTTER_ALPHA_SINE_INC);
  random_ripple_cb (stage);
  
  g_signal_connect (stage, "button-press-event",
                    G_CALLBACK (stage_clicked_cb), NULL);
  
  clutter_main ();
  
  return 0;
}

