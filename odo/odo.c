#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <clutter/clutter.h>
#include "clutter-texture-odo.h"

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

    clutter_actor_queue_redraw (d->tex);
}

/* skew paramter t <0, 0.7>, the greater t, the greater distortion */
static void
distort_func1 (ClutterTexture * texture,
	      gint x, gint y, gint z,
	      gint * x2, gint * y2, gint * z2,
	      gpointer data)
{
    struct distort_data * d = data;
    gint w, h;

    clutter_texture_get_base_size (CLUTTER_TEXTURE (d->tex), &w, &h);

    *x2 = x;
    *y2 = y;
    *z2 =  z + CFX_INT (CFX_MUL (d->t, w * clutter_sini (x*1024/w)));
}

/* skew paramter t <0, 0.7>, the greater t, the greater distortion */
static void
distort_func2 (ClutterTexture * texture,
	       gint x, gint y, gint z,
	       gint * x2, gint * y2, gint * z2,
	       gpointer data)
{
  struct distort_data * d = data;
  gint w, h;

  clutter_texture_get_base_size (CLUTTER_TEXTURE (d->tex), &w, &h);

  *x2 =
    x + CFX_INT(CFX_MUL(d->t * w/2, clutter_cosi(x*512/w)) - (d->t * w/2));
    
  *y2 =
    y + CFX_INT(CFX_MUL(d->t * h/2, clutter_cosi(y*512/h)) - (d->t * h/2));

  *z2 =  z - CFX_INT (CFX_MUL (d->t, w * clutter_sini (x*512/w)));
}

/* skew paramter t <0, 0.7>, the greater t, the greater distortion */
static void
distort_func3 (ClutterTexture * texture,
	       gint x, gint y, gint z,
	       gint * x2, gint * y2, gint * z2,
	       gpointer data)
{
  struct distort_data * d = data;
  gint w, h;

  clutter_texture_get_base_size (CLUTTER_TEXTURE (d->tex), &w, &h);

  *x2 =
    x + CFX_INT(CFX_MUL(d->t * w/2, clutter_cosi(x*512/w)) - (d->t * w/2));
    
  *y2 =
    y + CFX_INT(CFX_MUL(d->t * h/2, clutter_cosi(y*512/h)) - (d->t * h/2));

  *z2 =  z;
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
  clutter_actor_rotate_x (odo, rotate_x, 0, 0);
  clutter_actor_rotate_y (odo, rotate_y, 0, 0);
  
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
  GdkPixbuf        *pixbuf;
  struct distort_data data;
  ClutterTimeline  *timeline;
  
  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();

  g_signal_connect (stage, "key-press-event",
                    G_CALLBACK (on_event),
                    NULL);

  pixbuf = gdk_pixbuf_new_from_file ("redhand.png", NULL);

  if (!pixbuf)
    g_error("pixbuf load failed");

  clutter_stage_set_color (CLUTTER_STAGE (stage),
		           &stage_color);

  clutter_actor_set_size (stage, 800, 600);

  /* Make a hand */
  hand = clutter_texture_new_from_pixbuf (pixbuf);
  clutter_actor_show (hand);

  data.t = CLUTTER_FLOAT_TO_FIXED (0.7);
  data.tex = hand;
  
  odo = beget_odo (CLUTTER_TEXTURE (hand),
		   20, 120,
		   15.0, 30.0,
		   distort_func1,
		   &data);
		   
  clutter_container_add (CLUTTER_CONTAINER (stage), odo, NULL);

  odo = beget_odo (CLUTTER_TEXTURE (hand),
		   400, 400,
		   340.0, 0.0,
		   distort_func2,
		   &data);
		   
  clutter_container_add (CLUTTER_CONTAINER (stage), odo, NULL);

  odo = beget_odo (CLUTTER_TEXTURE (hand),
		   350, 20,
		   0.0, 0.0,
		   distort_func3,
		   &data);
		   
  clutter_container_add (CLUTTER_CONTAINER (stage), odo, NULL);
  
  timeline = clutter_timeline_new (104, 26);
  g_object_set (timeline, "loop", TRUE, NULL);
  
  g_signal_connect (timeline, "new-frame", G_CALLBACK (new_frame_cb), &data);
  
  clutter_actor_show_all (stage);

  clutter_timeline_start (timeline);
  
  clutter_main();

  return 0;
}
