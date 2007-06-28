#include <clutter/clutter.h>
#include <clutter/clutter-container.h>
#include <stdio.h>
#include <stdlib.h>
#include "clutter-dominatrix.h"

#define RECT_SIZE 120
#define DIMX 3
#define DIMY 3

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

	for (i = 0; i < DIMX * DIMY; ++i)
	  {
	    clutter_dominatrix_restore (*dmx);
	    dmx++;
	  }
      }
      
    default:
      break;
    }
}

int
main (int argc, char *argv[])
{
  ClutterActor      * stage;
  ClutterActor      * label;
  ClutterActor      * rect[DIMX * DIMY];
  ClutterDominatrix * dmx[DIMX * DIMY];
  struct on_event_data d;
  gint i,j;

  ClutterColor        clr[DIMX * DIMY] = { {0xff,0xff,0xff,0xff},
 					   {0xff,0,0,0xff},
 					   {0xff,0xff,0,0xff},
 					   {0xff,0,0xff,0xff},
 					   {0,0xff,0xff,0xff},
 					   {0,0xff,0,0xff},
					   {0x7f,0x7f,0x7f,0xff},
					   {0xff,0x5a,0,0xff},
					   {0,0,0xff,0xff}};
	  
  ClutterColor        stage_color = { 0x0, 0x0, 0x0, 0xff }, 
	              white       = { 0xff, 0xff, 0xff, 0xff };

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();

  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);
  clutter_actor_set_size (stage, 360, 410);

  for (i = 0; i < DIMY; ++i)
    for (j = 0; j < DIMX; ++j)
      {
	gint k = i * DIMX + j;
	ClutterGravity gravity = CLUTTER_GRAVITY_CENTER;

	rect[k] = clutter_rectangle_new ();
	clutter_actor_set_position (rect[k],
				    j*RECT_SIZE, 50 + i*RECT_SIZE);
	clutter_actor_set_size (rect[k], RECT_SIZE, RECT_SIZE);
	clutter_rectangle_set_color (CLUTTER_RECTANGLE (rect[k]), &clr[k]);
  	clutter_actor_show (rect[k]);
	clutter_container_add (CLUTTER_CONTAINER (stage), rect[k], NULL);

	if (i == 0 && j == 0)
	    gravity = CLUTTER_GRAVITY_NORTH_WEST;
	else if (i == DIMY - 1 && j == DIMX - 1)
	    gravity = CLUTTER_GRAVITY_SOUTH_EAST;
	else if (i == 0)
	    gravity = CLUTTER_GRAVITY_NORTH;
	else if (i == DIMY - 1)
	    gravity = CLUTTER_GRAVITY_SOUTH;
	else if (j == 0)
	    gravity = CLUTTER_GRAVITY_WEST;
	else if (j == DIMX - 1)
	    gravity = CLUTTER_GRAVITY_EAST;
	
	dmx[k] = clutter_dominatrix_new_with_gravity (rect[k], gravity);
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
