#include "opt.h"
#include <stdlib.h> 		/* for exit() */

void 
input_cb (ClutterStage *stage,
	  ClutterEvent *event,
	  gpointer      user_data)
{
  OptShow  *show = (OptShow*)user_data;

  if (event->type == CLUTTER_KEY_RELEASE)
    {
      ClutterKeyEvent* kev = (ClutterKeyEvent*)event;

      if (clutter_key_event_symbol (kev) == CLUTTER_q)
	exit(0); 		/* FIXME: need clutter exit func */

      if (clutter_key_event_symbol (kev) == CLUTTER_r
	  || clutter_key_event_symbol (kev) == CLUTTER_Left)
	opt_show_retreat (show);
      else
	opt_show_advance (show);
    }
}


int 
main(int argc, char **argv)
{
  OptShow  *show;
  GError   *error = NULL; 

  clutter_init(NULL, NULL);

  show = opt_show_new();

  if (!opt_config_load (show, "test.xml", &error))
    {
      /* Cleanup */
      g_warning ("Could not load presentation: %s", error->message);
      g_error_free (error);
      exit(-1);
    }

#if 0
  if (!opt_show_export (show, "/tmp", &error))
    {
      /* Cleanup */
      g_warning ("Could not export presentation: %s", error->message);
      g_error_free (error);
      exit(-1);
    }
  
  return 0;
#endif

  /* Connect up for input event */
  g_signal_connect (clutter_stage(), 
		    "input-event",
                    G_CALLBACK (input_cb),
                    show);

  opt_show_run (show);

  return 0;
}
