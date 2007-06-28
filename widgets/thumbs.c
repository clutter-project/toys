#include <clutter/clutter.h>
#include <clutter/clutter-container.h>
#include <stdio.h>
#include <stdlib.h>
#include "clutter-dominatrix.h"

#define IMG_SIZE 120
#define DIMX 3
#define DIMY 3

static void
on_manipulation (ClutterDominatrix * dmx1, gpointer data)
{
  ClutterDominatrix ** dmx = data;
  gint i;

  for (i = 0; i < DIMX * DIMY; ++i, ++dmx)
    {
      if (dmx1 == *dmx)
	continue;

      clutter_dominatrix_restore (*dmx);
    }
}

int
main (int argc, char *argv[])
{
  ClutterActor      * stage;
  ClutterActor      * img[DIMX * DIMY];
  ClutterDominatrix * dmx[DIMX * DIMY];
  gint i,j;
  GValue              val_true = {0};
  
  ClutterColor        stage_color = { 0x0, 0x0, 0x0, 0xff };

  clutter_init (&argc, &argv);

  g_value_init (&val_true, G_TYPE_BOOLEAN);
  g_value_set_boolean (&val_true, TRUE);
  
  stage = clutter_stage_get_default ();

  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);
  clutter_actor_set_size (stage, DIMX*IMG_SIZE, DIMY*IMG_SIZE);

  for (i = 0; i < DIMY; ++i)
    for (j = 0; j < DIMX; ++j)
      {
	GdkPixbuf * pixbuf;
	gint k = i * DIMX + j;
	ClutterGravity gravity = CLUTTER_GRAVITY_CENTER;
	gchar * img_name = g_strdup_printf ("hand%d.png", k);

	pixbuf = gdk_pixbuf_new_from_file (img_name, NULL);

	if (!pixbuf)
	  continue;
	
	img[k] = clutter_texture_new_from_pixbuf (pixbuf);
	clutter_actor_set_position (img[k], j*IMG_SIZE, i*IMG_SIZE);
	clutter_actor_set_size (img[k], IMG_SIZE, IMG_SIZE);
  	clutter_actor_show (img[k]);
	clutter_container_add (CLUTTER_CONTAINER (stage), img[k], NULL);

	if (i == 0 && j == 0)
	    gravity = CLUTTER_GRAVITY_NORTH_WEST;
	else if (i == DIMY - 1 && j == DIMX - 1)
	    gravity = CLUTTER_GRAVITY_SOUTH_EAST;
	else if (i == 0 && j == DIMX-1)
	    gravity = CLUTTER_GRAVITY_NORTH_EAST;
	else if (i == DIMY-1 && j == 0)
	    gravity = CLUTTER_GRAVITY_SOUTH_WEST;
	else if (i == 0)
	    gravity = CLUTTER_GRAVITY_NORTH;
	else if (i == DIMY - 1)
	    gravity = CLUTTER_GRAVITY_SOUTH;
	else if (j == 0)
	    gravity = CLUTTER_GRAVITY_WEST;
	else if (j == DIMX - 1)
	    gravity = CLUTTER_GRAVITY_EAST;
	
	dmx[k] = clutter_dominatrix_new_with_gravity (img[k], gravity);

	g_object_set_property (G_OBJECT (dmx[k]),
			       "disable-movement", &val_true);
	
	g_signal_connect (dmx[k], "manipulation-started",
			  G_CALLBACK (on_manipulation), &dmx[0]);

	g_free (img_name);
      }
  
  clutter_actor_show_all (stage);

  clutter_main();

  return EXIT_SUCCESS;
}
