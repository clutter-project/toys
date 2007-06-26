#include <clutter/clutter.h>
#include <clutter/clutter-container.h>
#include <stdio.h>
#include <stdlib.h>
#include "clutter-dominatrix.h"


int
main (int argc, char *argv[])
{
  ClutterActor      * stage;
  ClutterActor      * label;
  ClutterActor      * rect;
  ClutterDominatrix * dmx;
  
  ClutterColor        stage_color = { 0x0, 0x0, 0x0, 0xff }, 
                      red         = { 0xff, 0, 0, 0xff },
	              white       = { 0xff, 0xff, 0xff, 0xff };

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();

  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);
  clutter_actor_set_size (stage, 640, 480);

  rect = clutter_rectangle_new ();
  clutter_actor_set_position (rect, 180, 120);
  clutter_actor_set_size (rect,
                          240,
                          240);
  clutter_rectangle_set_color (CLUTTER_RECTANGLE (rect),
                               &white);
  clutter_rectangle_set_border_width (CLUTTER_RECTANGLE (rect), 5);
  clutter_rectangle_set_border_color (CLUTTER_RECTANGLE (rect),
                                      &red);
  clutter_actor_show (rect);

  clutter_container_add (CLUTTER_CONTAINER (stage), rect, NULL);
  dmx = clutter_dominatrix_new (rect);
  
  label = clutter_label_new_with_text ("Mono 8pt",
				       "Test dragging on the rectangle");
  clutter_label_set_color (CLUTTER_LABEL (label), &white);
  
  clutter_actor_set_position (label, 10, 10);
  clutter_group_add (CLUTTER_GROUP(stage), label);
  
  clutter_actor_show_all (stage);
  
  clutter_main();

  return EXIT_SUCCESS;
}
