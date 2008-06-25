#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <clutter/clutter.h>
#include "clutter-texture-odo.h"

static int tile_width  = 8;
static int tile_height = 8;
static char * image = "redhand.png";

struct distort_data
{
    ClutterFixed t;
    ClutterActor * tex;
};

void new_frame_cb (ClutterTimeline *timeline,
                   gint             frame_num,
                   gpointer         data)
{
    struct distort_data * d = data;
    guint frames = clutter_timeline_get_n_frames (timeline);
    gdouble t = 0.7 - ((gdouble)abs(frames/2-frame_num))*1.4 / (gdouble)frames;

    d->t = CLUTTER_FLOAT_TO_FIXED (t);

    clutter_actor_queue_redraw (clutter_stage_get_default ());
}

/* skew paramter t <0, 0.7>, the greater t, the greater distortion */
static gboolean
distort_func1 (ClutterTextureOdo * texture,
               ClutterFixed x, ClutterFixed y, ClutterFixed z,
               ClutterFixed *x2, ClutterFixed *y2, ClutterFixed *z2,
               ClutterColor *color, gpointer data)
{
    struct distort_data * d = data;
    gint w, h;

    clutter_texture_get_base_size (CLUTTER_TEXTURE (d->tex), &w, &h);

    *x2 = x;
    *y2 = y;
    *z2 =  z + CFX_MUL (d->t,
                        w * clutter_sini (CLUTTER_FIXED_TO_INT (x)*1024/w));
    
    return TRUE;
}

/* skew paramter t <0, 0.7>, the greater t, the greater distortion */
static gboolean
distort_func2 (ClutterTextureOdo * texture,
               ClutterFixed x, ClutterFixed y, ClutterFixed z,
               ClutterFixed *x2, ClutterFixed *y2, ClutterFixed *z2,
               ClutterColor *color, gpointer data)
{
  struct distort_data * d = data;
  gint w, h, xi, yi, zi;

  clutter_texture_get_base_size (CLUTTER_TEXTURE (d->tex), &w, &h);

  xi = CLUTTER_FIXED_TO_INT (x);
  yi = CLUTTER_FIXED_TO_INT (y);
  zi = CLUTTER_FIXED_TO_INT (z);
  
  *x2 = x + CFX_MUL(d->t * w/2, clutter_cosi(xi*512/w)) - (d->t * w/2);
    
  *y2 = y + CFX_MUL(d->t * h/2, clutter_cosi(yi*512/h)) - (d->t * h/2);

  *z2 = z - CFX_MUL (d->t, w * clutter_sini (xi*512/w));
  
  return TRUE;
}

/* skew paramter t <0, 0.7>, the greater t, the greater distortion */
static gboolean
distort_func3 (ClutterTextureOdo * texture,
               ClutterFixed x, ClutterFixed y, ClutterFixed z,
               ClutterFixed *x2, ClutterFixed *y2, ClutterFixed *z2,
               ClutterColor *color, gpointer data)
{
  struct distort_data * d = data;
  gint w, h, xi, yi;

  clutter_texture_get_base_size (CLUTTER_TEXTURE (d->tex), &w, &h);

  xi = CLUTTER_FIXED_TO_INT (x);
  yi = CLUTTER_FIXED_TO_INT (y);

  *x2 = x + CFX_MUL(d->t * w/2, clutter_cosi(xi*512/w)) - (d->t * w/2);
    
  *y2 = y + CFX_MUL(d->t * h/2, clutter_cosi(yi*512/h)) - (d->t * h/2);

  *z2 =  z;
  
  return TRUE;
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
  ClutterActor     *hand, *odo;
  ClutterColor      stage_color = { 0xcc, 0xcc, 0xcc, 0xff };
  struct distort_data data;
  ClutterTimeline  *timeline;
  gint              i;
  ClutterMesh       mesh;
  
  for (i = 1; i < argc; ++i)
    {
      if (!strncmp (argv[i], "--tile-height", 13))
        {
          tile_height = atoi (argv[i] + 14);

          if (!tile_height)
            tile_height = 8;
        }
      else if (!strncmp (argv[i], "--tile-width", 12))
        {
          tile_width = atoi (argv[i] + 13);

          if (!tile_width)
            tile_height = 8;
        }
      else if (!strncmp (argv[i], "--image", 7))
          image = argv[i] + 8;
    }
  
  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();

  g_signal_connect (stage, "key-press-event",
                    G_CALLBACK (on_event),
                    NULL);

  clutter_stage_set_color (CLUTTER_STAGE (stage),
                           &stage_color);

  clutter_actor_set_size (stage, 800, 600);

  /* Make a hand */
  hand = clutter_texture_new_from_file (image, NULL);
  clutter_actor_show (hand);

  data.t = CLUTTER_FLOAT_TO_FIXED (0.7);
  data.tex = hand;
  
  odo = beget_odo (CLUTTER_TEXTURE (hand),
                   20, 20,
                   15.0, 30.0,
                   distort_func1,
                   &data);
                   
  clutter_container_add (CLUTTER_CONTAINER (stage), odo, NULL);

  odo = beget_odo (CLUTTER_TEXTURE (hand),
                   570, 400,
                   340.0, 0.0,
                   distort_func2,
                   &data);
                   
  clutter_container_add (CLUTTER_CONTAINER (stage), odo, NULL);

  odo = beget_odo (CLUTTER_TEXTURE (hand),
                   600, 20,
                   0.0, 0.0,
                   distort_func3,
                   &data);
                   
  clutter_container_add (CLUTTER_CONTAINER (stage), odo, NULL);

  mesh.dimension_x = 4;
  mesh.dimension_y = 4;
  mesh.points = g_new (ClutterMeshPoint, 16);

#define P(a,b,_x,_y,_z) \
   mesh.points[b*mesh.dimension_x + a].x = CLUTTER_INT_TO_FIXED(_x); \
   mesh.points[b*mesh.dimension_x + a].y = CLUTTER_INT_TO_FIXED(_y); \
   mesh.points[b*mesh.dimension_x + a].z = CLUTTER_INT_TO_FIXED(_z)

  P (0,0,  0,0,0);
  P (0,1, 80,0,0);
  P (0,2,160,0,0);
  P (0,3,240,0,0);

  P (1,0,  0,80,0);
  P (1,1, 80,80,160);
  P (1,2,160,80,160);
  P (1,3,240,80,0);

  P (2,0,  0,160,0);
  P (2,1, 80,160,160);
  P (2,2,160,160,160);
  P (2,3,240,160,0);

  P (3,0,  0,240,0);
  P (3,1, 80,240,0);
  P (3,2,160,240,0);
  P (3,3,240,240,0);
#undef P
  
  odo = clutter_texture_odo_new (CLUTTER_TEXTURE (hand));
  clutter_actor_set_position (odo, 290, 200);

  g_object_set (G_OBJECT (odo), "mesh", &mesh, NULL);
  
  clutter_container_add (CLUTTER_CONTAINER (stage), odo, NULL);

  timeline = clutter_timeline_new (106, 26);
  clutter_timeline_set_loop (timeline, TRUE);
  
  g_signal_connect (timeline, "new-frame", G_CALLBACK (new_frame_cb), &data);
  
  clutter_actor_show_all (stage);

  clutter_timeline_start (timeline);
  
  clutter_main();

  return 0;
}
