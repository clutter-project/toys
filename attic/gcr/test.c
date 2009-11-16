#include <clutter/clutter.h>

#include "custom-cursor.h"

#include "gcr.h"


gint
main (int   argc,
      char *argv[])
{
  ClutterActor *stage;

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();

  custom_cursor (0, 0, 0);

  gcr_prepare ("/tmp/test.mpg");
  clutter_actor_show (stage);
  gcr_start ();
  clutter_main ();
  gcr_stop ();

  return 0;
}
