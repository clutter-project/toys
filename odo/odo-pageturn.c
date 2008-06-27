#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <clutter/clutter.h>
#include "clutter-texture-odo.h"

static int tile_width  = 8;
static int tile_height = 8;
static char * image1 = "redhand.png";
static char * image2 = "neghand.png";

struct distort_data
{
    ClutterFixed radius;
    ClutterFixed angle;
    ClutterFixed turn;
};

static gboolean
distort_func (ClutterTextureOdo * otex,
              ClutterFixed x, ClutterFixed y, ClutterFixed z,
              ClutterFixed *x2, ClutterFixed *y2, ClutterFixed *z2,
              ClutterColor *color, gpointer data)
{
  struct distort_data * d = data;
  ClutterFixed cx, cy, rx, ry;
  guint w, h, shade;
  ClutterFixed width, height, turn_angle;
  
  clutter_actor_get_size (CLUTTER_ACTOR (otex), &w, &h);

  width = CLUTTER_INT_TO_FIXED (w);
  height = CLUTTER_INT_TO_FIXED (h);
  
  /* Rotate the point around the centre of the page-curl ray to align it with
   * the y-axis.
   */
  
  cx = clutter_qmulx (CFX_ONE - d->turn, width);
  cy = clutter_qmulx (CFX_ONE - d->turn, height);
  
  rx = clutter_qmulx (x - cx, clutter_cosx (-d->angle)) -
       clutter_qmulx (y - cy, clutter_sinx (-d->angle)) - d->radius;
  ry = clutter_qmulx (x - cx, clutter_sinx (-d->angle)) +
       clutter_qmulx (y - cy, clutter_cosx (-d->angle));
  
  if (rx > -d->radius)
    {
      /* Calculate the curl angle as a function from the distance of the curl
       * ray (i.e. the page crease)
       */
      turn_angle = (clutter_qmulx (clutter_qdivx (rx, d->radius),
                                   CFX_PI_2) - CFX_PI_2);

      /* Add a gradient that makes it look like lighting and hides the switch
       * between textures.
       */
      shade = CLUTTER_FIXED_TO_INT (
        clutter_qmulx (clutter_sinx (turn_angle), CLUTTER_INT_TO_FIXED (96))) +
        159;
      color->red = shade;
      color->green = shade;
      color->blue = shade;
    }
  else
    *color = (ClutterColor){ 0xff, 0xff, 0xff, 0xff };
  
  if (rx > 0)
    {
      /* Make the curl radius smaller as more circles are formed (stops
       * z-fighting and looks cool)
       */
      ClutterFixed small_radius = d->radius -
              clutter_qdivx (clutter_qmulx (turn_angle,
                                            CLUTTER_INT_TO_FIXED (2)), CFX_PI);
      
      /* Calculate a point on a cylinder (maybe make this a cone at some point)
       * and rotate it by the specified angle.
       */
      rx = clutter_qmulx (small_radius, clutter_cosx (turn_angle)) + d->radius;
      *x2 = clutter_qmulx (rx, clutter_cosx (d->angle)) -
            clutter_qmulx (ry, clutter_sinx (d->angle)) + cx;
      *y2 = clutter_qmulx (rx, clutter_sinx (d->angle)) +
            clutter_qmulx (ry, clutter_cosx (d->angle)) + cy;
      *z2 = clutter_qmulx (small_radius, clutter_sinx (turn_angle)) + d->radius;
    }
  else
    {
      *x2 = x;
      *y2 = y;
      *z2 = z;
    }
  
  return TRUE;
}

static void
new_frame_cb (ClutterTimeline *timeline,
              gint             frame_num,
              gpointer         data)
{
    struct distort_data * d = data;
    guint frames = clutter_timeline_get_n_frames (timeline);
    gdouble t = 1.0 - ((gdouble)abs(frames/2-frame_num))*2.0 / (gdouble)frames;

    d->turn = CLUTTER_FLOAT_TO_FIXED (t);

    clutter_actor_queue_redraw (clutter_stage_get_default ());
}

static void 
on_event (ClutterStage *stage,
          ClutterEvent *event,
          gpointer      data)
{
  switch (event->type)
    {
    case CLUTTER_KEY_PRESS:
      {
        guint sym = clutter_key_event_symbol ((ClutterKeyEvent*)event);

        switch (sym)
          {
          case CLUTTER_Escape:
          case CLUTTER_q:
          case CLUTTER_Q:
             clutter_main_quit ();
             break;
             
          default:
            break;
          }
      }
      break;
    default:
      break;
    }
}


static ClutterActor * beget_odo (ClutterTexture * tex,
                                 gint x,
                                 gint y,
                                 gdouble rotate_x,
                                 gdouble rotate_y,
                                 ClutterTextureDistortFunc func,
                                 gpointer data)
{
  ClutterActor * odo = clutter_texture_odo_new (tex);
  clutter_actor_set_position (odo, x, y);
  clutter_actor_set_rotation (odo, CLUTTER_X_AXIS, rotate_x, 0, 0, 0);
  clutter_actor_set_rotation (odo, CLUTTER_Y_AXIS, rotate_y, 0, 0, 0);

  g_object_set (G_OBJECT (odo), "tile-width", tile_width, NULL);
  g_object_set (G_OBJECT (odo), "tile-height", tile_height, NULL);
  g_object_set (G_OBJECT (odo), "distort-func-data", data, NULL);
  g_object_set (G_OBJECT (odo), "distort-func", func, NULL);
  return odo;
}

int
main (int argc, char *argv[])
{
  ClutterActor     *stage;
  ClutterActor     *tex1, *tex2, *odo;
  ClutterColor      stage_color = { 0xcc, 0xcc, 0xcc, 0xff };
  struct distort_data data;
  ClutterTimeline  *timeline;
  gint              i;
  
  for (i = 1; i < argc; ++i)
    {
      if (!strncmp (argv[i], "--tile-height", 13))
        {
          tile_height = atoi (argv[i] + 14);

          if (!tile_height)
            tile_height = 4;
        }
      else if (!strncmp (argv[i], "--tile-width", 12))
        {
          tile_width = atoi (argv[i] + 13);

          if (!tile_width)
            tile_width = 4;
        }
      else if (!strncmp (argv[i], "--image1", 8))
          image1 = argv[i] + 9;
      else if (!strncmp (argv[i], "--image2", 8))
          image2 = argv[i] + 9;
    }
  
  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();

  g_signal_connect (stage, "key-press-event",
                    G_CALLBACK (on_event),
                    NULL);

  clutter_stage_set_color (CLUTTER_STAGE (stage),
                           &stage_color);

  clutter_actor_set_size (stage, 800, 600);
  
  /* Make textures */
  tex1 = g_object_new (CLUTTER_TYPE_TEXTURE,
                       "disable-slicing", TRUE,
                       "filename", image1,
                       NULL);
  clutter_actor_show (tex1);
  tex2 = g_object_new (CLUTTER_TYPE_TEXTURE,
                       "disable-slicing", TRUE,
                       "filename", image2,
                       NULL);
  clutter_actor_show (tex2);

  data.turn = 0;
  data.radius = CLUTTER_INT_TO_FIXED (20);
  data.angle = clutter_qdivx (CFX_PI, CLUTTER_INT_TO_FIXED (6));
  
  odo = beget_odo (CLUTTER_TEXTURE (tex1),
                   240, 120,
                   0.0, 0.0,
                   distort_func,
                   &data);
  clutter_texture_odo_set_backface_texture (CLUTTER_TEXTURE_ODO (odo),
                                            CLUTTER_TEXTURE (tex2));
  clutter_texture_odo_set_cull_mode (CLUTTER_TEXTURE_ODO (odo), ODO_CULL_BACK);
                   
  clutter_container_add (CLUTTER_CONTAINER (stage), odo, NULL);

  timeline = clutter_timeline_new_for_duration (3000);
  clutter_timeline_set_loop (timeline, TRUE);
  
  g_signal_connect (timeline, "new-frame", G_CALLBACK (new_frame_cb), &data);
  
  clutter_actor_show_all (stage);

  clutter_timeline_start (timeline);
  
  cogl_enable_depth_test (TRUE);
  clutter_main();

  return 0;
}

