#include "opt.h"
#include <stdlib.h> 		/* for exit() */

static OptShow *opt_show = NULL;

static gboolean 
key_release_cb (ClutterStage           *stage,
                ClutterKeyEvent        *kev,
                gpointer                user_data)
{
  OptShow *show = OPT_SHOW (user_data);

  switch (clutter_key_event_symbol (kev))
    {
      case CLUTTER_m:
          opt_show_pop_menu (show);
          break;
      case CLUTTER_s:
          opt_show_toggle_position (show);
          break;
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

      case CLUTTER_Up:
      case CLUTTER_Down:
      case CLUTTER_Return:
          /* menu keys -- ignore */
          break;

      case CLUTTER_Right:
      default:
          opt_show_advance (show);
          break;
    }

  return FALSE;
}

static gboolean
button_release_cb (ClutterStage        *stage,
                   ClutterButtonEvent  *bev,
                   gpointer             user_data)
{
  OptShow  *show = OPT_SHOW (user_data);

  if (bev->button == 1)
    opt_show_advance (show);
  else if (bev->button == 3)
    opt_show_retreat (show);

  return FALSE;
}

static void
on_fullscreen (ClutterStage *stage,
               const gchar  *filename)
{
  GError *error = NULL;

  if (opt_show)
    return;

  opt_show = opt_show_new ();

  if (!opt_config_load (opt_show, filename, &error))
    {
      /* Cleanup */
      g_printerr ("Could not load presentation:\n\t%s\n", error->message);
      g_error_free (error);
      exit (EXIT_FAILURE);
    }

  opt_show_run (opt_show);

  /* Connect up for input event */
  g_signal_connect (stage,
                    "key-release-event", G_CALLBACK (key_release_cb),
                    opt_show);
  g_signal_connect (stage,
                    "button-release-event", G_CALLBACK (button_release_cb),
                    opt_show);
}

static int
usage (const char *msg)
{
  g_printerr ("Usage: %s [OPTIONS..] <FILE>\n", msg);

  return EXIT_FAILURE;
}

int 
main(int argc, char **argv)
{
  GError        *error = NULL; 
  ClutterActor  *stage;
  gchar        **opt_filename = NULL;
  gchar         *opt_export = NULL;
  gchar         *opt_size = NULL;

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
      "WxH" },

    { G_OPTION_REMAINING, 
      0, 
      0, 
      G_OPTION_ARG_FILENAME_ARRAY, 
      &opt_filename, 
      "Presentation XML filename to load", 
      "FILE" },

    { NULL }
  };

  if (argc == 1)
    return usage (argv[0]);

  clutter_init_with_args (&argc, &argv, "- OH Presentation tool",
                          options, NULL,
                          NULL);

  stage = clutter_stage_get_default();

  /* Need to set this early on */
  if (opt_export != NULL)
    {
      gboolean offscreen_supported;

      g_object_set (stage, "offscreen", TRUE, NULL);

      /* Actually check offscreen works - recent Mesas appear not to
       * like rendering to Pixmaps. 
      */
      g_object_get (stage, "offscreen", &offscreen_supported, NULL);
      if (offscreen_supported == FALSE)
	{
	  g_print ("Could not export presentation:\n"
		   "\tOffscreen rendering not supported by Clutter backend\n");
	  return EXIT_FAILURE;
	}
    }

  if (opt_size != NULL)
    {
      gint w, h;

      if (!sscanf (opt_size, "%dx%d", &w, &h) || w <= 0 || h <= 0)
	return usage (argv[0]);

      opt_show = opt_show_new ();

      clutter_actor_set_size (stage, w, h);

      if (!opt_config_load (opt_show, opt_filename[0], &error))
        {
          /* Cleanup */
          g_printerr ("Could not load presentation:\n\t%s\n", error->message);
          g_error_free (error);
          return EXIT_FAILURE;
        }

      /* Connect up for input event */
      g_signal_connect (stage,
                        "key-release-event", G_CALLBACK (key_release_cb),
                        opt_show);
      g_signal_connect (stage,
                        "button-release-event", G_CALLBACK (button_release_cb),
                        opt_show);

      opt_show_run (opt_show);
    }
  else
    {
      g_signal_connect (stage,
                        "fullscreen", G_CALLBACK (on_fullscreen),
                        opt_filename[0]);

      clutter_stage_fullscreen (CLUTTER_STAGE (stage));
      clutter_stage_hide_cursor (CLUTTER_STAGE (stage));
      clutter_actor_show (stage);
    }

  if (opt_export)
    {
      if (!opt_show_export (opt_show, opt_export, &error))
        {
          /* Cleanup */
          g_printerr ("Could not export presentation:\n\t%s\n",
                      error->message);
          g_error_free (error);
	  return EXIT_FAILURE;
	}

      return EXIT_SUCCESS;
    }
  else
    clutter_main ();

  return EXIT_SUCCESS;
}
