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
      if (clutter_key_event_symbol ((ClutterKeyEvent*)event) == CLUTTER_q)
	exit(0); 		/* FIXME: need clutter exit func */
      opt_show_advance (show, 0);
    }
}

void
opt_init (void)
{
  clutter_init(NULL, NULL);
}

int 
main(int argc, char **argv)
{
  OptShow  *show;
  GError        *error = NULL; 

  opt_init ();

  show = opt_show_new();

  if (!opt_config_load (show, "test.xml", &error))
    {
      /* Cleanup */
      g_warning ("Could not load presentation: %s", error->message);
      g_error_free (error);
      exit(-1);
    }

  /* Connect up for input event */

  g_signal_connect (clutter_stage(), "input-event",
                    G_CALLBACK (input_cb),
                    show);

  opt_show_run (show);

  return 0;
}
