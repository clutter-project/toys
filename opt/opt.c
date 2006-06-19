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
      ClutterKeyEvent* kev = (ClutterKeyEvent *) event;

      switch (clutter_key_event_symbol (kev))
	{
        case CLUTTER_q:
          clutter_main_quit ();
	  break;
	case CLUTTER_r:
	case CLUTTER_Left:
	  opt_show_retreat (show);
	  break;
	case CLUTTER_Page_Down:
	  opt_show_skip (show, 5);
	  break;
	case CLUTTER_Page_Up:
	  opt_show_skip (show, -5);
	  break;
	default:
	  opt_show_advance (show);
	  break;
	}
    }
  else if (event->type == CLUTTER_BUTTON_RELEASE)
    {
      ClutterButtonEvent* bev = (ClutterButtonEvent*)event;

      if (bev->button == 1)
	opt_show_advance (show);
      else
	opt_show_retreat (show);
    }
}

static void
usage (const char *msg)
{
  g_print("\nUsage: %s [OPTIONS..] <FILE>\n\n", msg);
  exit(-1);
}

int 
main(int argc, char **argv)
{
  GError         *error = NULL; 
  ClutterActor *stage;
  OptShow        *show;
  gchar         **opt_filename = NULL;
  gchar          *opt_export = NULL;
  gchar          *opt_size = NULL;
  GOptionContext *ctx;

  GOptionEntry options[] = {
    { "export", 
      'e', 
      0, 
      G_OPTION_ARG_STRING, 
      &opt_export, 
      "Export PNG slides to PATH", 
      "PATH" },

    { "size", 
      's', 
      0, 
      G_OPTION_ARG_STRING, 
      &opt_size, 
      "Presentation display dimentions.", 
      "<WxH>" },

    { G_OPTION_REMAINING, 
      0, 
      0, 
      G_OPTION_ARG_FILENAME_ARRAY, 
      &opt_filename, 
      "Presentation XML filename to load", 
      "<PRESENTATION FILE>" },

    { NULL }
  };

  if (argc == 1)
    usage(argv[0]);

  ctx = g_option_context_new("- OH Presentation Tool");
  g_option_context_add_main_entries(ctx, options, "OPT");

  if (!g_option_context_parse(ctx, &argc, &argv, NULL))
    usage(argv[0]);

  if (opt_filename  == NULL)
    usage(argv[0]);

  g_option_context_free(ctx);

  /* FIXME: Need clutter_init_with_args() */
  clutter_init(NULL, NULL);

  stage = clutter_stage_get_default();

  /* Need to set this early on */
  if (opt_export != NULL)
    g_object_set (stage, "offscreen", TRUE, NULL);

  if (opt_size != NULL)
    {
      gint w, h;
      if (!sscanf(opt_size, "%dx%d", &w, &h) || w <= 0 || h <= 0)
	usage(argv[0]);

      g_object_set (stage, "fullscreen", FALSE, NULL);

      clutter_actor_set_size (stage, w, h);
    }
  else
    {
      g_object_set (stage,
		    "fullscreen",  TRUE, 
		    "hide-cursor", TRUE,
		    NULL);
    }

  show = opt_show_new();

  if (!opt_config_load (show, opt_filename[0], &error))
    {
      /* Cleanup */
      g_print ("Could not load presentation:\n\t%s\n", error->message);
      g_error_free (error);
      exit(-1);
    }

  if (opt_export)
    {
      if (!opt_show_export (show, opt_export, &error))
	{
	  /* Cleanup */
	  g_print ("Could not export presentation:\n\t%s\n", error->message);
	  g_error_free (error);
	  exit(-1);
	}
      return 0;
    }
  else
    {
      /* Connect up for input event */
      g_signal_connect (stage, 
			"input-event",
			G_CALLBACK (input_cb),
			show);

      opt_show_run (show);
    }


  return 0;
}
