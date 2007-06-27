#include <clutter/clutter.h>
#include <clutter/clutter-container.h>
#include <stdio.h>
#include <stdlib.h>
#include "clutter-dominatrix.h"

struct on_event_data 
{
  ClutterDominatrix ** dmx;
  ClutterActor       * reset;
};

static void 
on_event (ClutterStage *stage,
	  ClutterEvent *event,
	  gpointer      data)
{
  struct on_event_data * d = data;
  
  switch (event->type)
    {
    case CLUTTER_BUTTON_PRESS:
      {
	gint x, y, i;
	ClutterActor   * actor;
	ClutterDominatrix ** dmx = d->dmx;
	
        clutter_event_get_coords (event, &x, &y);

	actor = clutter_stage_get_actor_at_pos (stage, x, y);
	
	if (actor != d->reset)
	    return;

	for (i = 0; i < 4; ++i)
	  {
	    clutter_dominatrix_restore (*dmx);
	    dmx++;
	  }
      }
      
    default:
      break;
    }
}


#define RECT_SIZE 180

int
main (int argc, char *argv[])
{
  ClutterActor      * stage;
  ClutterActor      * label;
  ClutterActor      * rect[4];
  ClutterDominatrix * dmx[4];
  ClutterGravity      gvt[4];
  struct on_event_data d;
  gint i,j;

  ClutterColor        clr[4] = { {0xff,0xff,0xff,0xff},
				 {0xff,0,0,0xff},
				 {0,0xff,0,0xff},
				 {0,0,0xff,0xff}};
	  
  ClutterColor        stage_color = { 0x0, 0x0, 0x0, 0xff }, 
                      red         = { 0xff, 0, 0, 0xff },
	              white       = { 0xff, 0xff, 0xff, 0xff };

  gvt[0] = CLUTTER_GRAVITY_NORTH_WEST;
  gvt[1] = CLUTTER_GRAVITY_SOUTH_WEST;
  gvt[2] = CLUTTER_GRAVITY_NORTH_EAST;
  gvt[3] = CLUTTER_GRAVITY_SOUTH_EAST;
  
  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();

  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);
  clutter_actor_set_size (stage, 800, 600);

  for (i = 0; i < 2; ++i)
    for (j = 0; j < 2; ++j)
      {
	gint k = i*2 + j;
	
	rect[k] = clutter_rectangle_new ();
	clutter_actor_set_position (rect[k],
				    150 + i*RECT_SIZE, 100 + j*RECT_SIZE);
	clutter_actor_set_size (rect[k], RECT_SIZE, RECT_SIZE);
	clutter_rectangle_set_color (CLUTTER_RECTANGLE (rect[k]), &clr[k]);
  	clutter_actor_show (rect[k]);
	clutter_container_add (CLUTTER_CONTAINER (stage), rect[k], NULL);

	dmx[k] = clutter_dominatrix_new_with_gravity (rect[k], gvt[k]);
      }
  
  label = clutter_label_new_with_text ("Mono 8pt",
	       "Test dragging on the rectangles; click here to reset");
  
  clutter_label_set_color (CLUTTER_LABEL (label), &white);
  
  clutter_actor_set_position (label, 10, 10);
  clutter_group_add (CLUTTER_GROUP(stage), label);
  
  clutter_actor_show_all (stage);

  d.reset = label;
  d.dmx = &dmx[0];

  g_signal_connect (stage, "event",
		    G_CALLBACK (on_event), &d);
  
  clutter_main();

  return EXIT_SUCCESS;
}
