/* ClutterScript viewer, a viewer for displaying clutter scripts or fragments
 * of clutterscript.
 *
 * Copyright 2007 OpenedHand Ltd
 * Authored by Øyvind Kolås <pippin@o-hand.com>
 * Licensed under the GPL v2 or greater.
 *
 */

#include <clutter/clutter.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>


/* Global structure containing information parsed from commandline parameters */
static struct
{
  gboolean  fullscreen;
  gchar    *bg_color;
  gint      width, height;
  gchar    *path;
  gchar    *id;
  gchar    *timeline;
  gchar    *png;
}
args =
{
  FALSE,
  "gray",
  640,    480,
  NULL,
  "root",
  NULL,
  NULL
};

/* using global variables, this is needed at least for the ClutterScript to avoid
 * possible behaviours to be destroyed when the script is destroyed.
 */
static ClutterActor    *stage;
static ClutterActor    *actor    = NULL;
static ClutterTimeline *timeline = NULL;
static ClutterScript   *script   = NULL;

gboolean
parse_args (gchar **argv)
{
  gchar **arg = argv + 1;

  while (*arg)
    {
      if (g_str_equal (*arg, "-h") ||
          g_str_equal (*arg, "--help"))
        {
usage:
          g_print ("\nUsage: %s [options] %s\n\n",
                   argv[0], args.path ? args.path : "<clutterscript>");
          g_print ("  -s <widthXheight>       stage size                  (%ix%i)\n",
                   args.width, args.height);
          g_print ("  -fs                     run fullscreen              (%s)\n",
                   args.fullscreen ? "TRUE" : "FALSE");
          g_print ("  -bg <color>             stage color                 (%s)\n",
                   args.bg_color);
          g_print ("  -id <actor id>          which actor id to show      (%s)\n",
                   args.id ? args.id : "NULL");
          g_print ("  -timeline <timeline id> a timeline to play          (%s)\n",
                   args.timeline ? args.timeline : "NULL");
          g_print ("  -o <file.png>           write screenshot, then quit (%s)\n",
                   args.png? args.png: "NULL");
          g_print ("  -h                      this help\n\n");
          return FALSE;
        }
      else if (g_str_equal (*arg, "-s"))
        {
          arg++; g_assert (*arg);
          args.width = atoi (*arg);
          if (strstr (*arg, "x"))
            args.height = atoi (strstr (*arg, "x") + 1);
        }
      else if (g_str_equal (*arg, "-bg"))
        {
          arg++; g_assert (*arg);
          args.bg_color = *arg;
        }
      else if (g_str_equal (*arg, "-id"))
        {
          arg++; g_assert (*arg);
          args.id = *arg;
        }
      else if (g_str_equal (*arg, "-timeline"))
        {
          arg++; g_assert (*arg);
          args.timeline = *arg;
        }
      else if (g_str_equal (*arg, "-o"))
        {
          arg++; g_assert (*arg);
          args.png = *arg;
        }
      else if (g_str_equal (*arg, "-fs"))
        {
          args.fullscreen = TRUE;
        }
      else
        {
          args.path = *arg;
        }
      arg++;
    }
  if (args.path == NULL)
    {
      g_print ("Error parsing commandline: no clutterscript provided\n");
      goto usage;
    }
  return TRUE;
}

static ClutterActor *
initialize_stage ()
{
  ClutterActor *stage;
  ClutterColor  color;

  stage = clutter_stage_get_default ();
  clutter_color_parse (args.bg_color, &color);
  clutter_stage_set_color (CLUTTER_STAGE (stage), &color);
  clutter_actor_set_size (stage, args.width, args.height);

  if (args.fullscreen)
    clutter_stage_fullscreen (CLUTTER_STAGE (stage));

  return stage;
}


static void
load_script (const gchar *path)
{
  GError        *error = NULL;

  g_assert (CLUTTER_IS_SCRIPT (script));
  clutter_script_load_from_file (script, path, &error);

  if (error)
    {
      ClutterColor error_color = { 0xff, 0, 0, 0xff };
      actor = clutter_label_new_with_text ("Sans 25px", error->message);
      clutter_label_set_color (CLUTTER_LABEL (actor), &error_color);
      g_error_free (error);
      return;
    }

  actor = CLUTTER_ACTOR (clutter_script_get_object (script, args.id));

  if (actor == NULL)
    {
      ClutterColor error_color = { 0xff, 0, 0, 0xff };
      gchar        message[256];

      g_sprintf (message, "No actor with \"id\"=\"%s\" found", args.id);
      actor = clutter_label_new_with_text ("Sans 30px", message);
      clutter_label_set_color (CLUTTER_LABEL (actor), &error_color);
    }
  else 
    {
      clutter_group_add (CLUTTER_GROUP (stage), actor);
      clutter_actor_show_all (stage);

      if (args.png != NULL)
        { 
          /* write a screenshot to file */
          GdkPixbuf *snapshot;
          gint i;

          for (i=0;i<10;i++) /* evil hack to force stage to be updated,
                              * before screenshot
                              */
            {
              g_usleep (10000);
              while (g_main_context_pending (NULL))
                g_main_context_iteration (NULL, FALSE);
            }

          snapshot = clutter_stage_snapshot (CLUTTER_STAGE (stage), 0, 0,
                                             args.width, args.height);
          gdk_pixbuf_save (snapshot, args.png, "png", NULL, NULL);
          clutter_main_quit ();
        }
      else if (args.timeline != NULL)
        {
          timeline = CLUTTER_TIMELINE (clutter_script_get_object (
                                       script, args.timeline));
          if (timeline)
            clutter_timeline_start (timeline);
        }
    }
}

static gboolean watch_file (gpointer data)
{
  static struct stat   stat_buf;
  static time_t        previous_ctime = 0;

  g_stat (args.path, &stat_buf);

  if (stat_buf.st_ctime != previous_ctime)
    {
      if (script != NULL)
        g_object_unref (script);

      script = clutter_script_new ();
      if (actor != NULL)
        {
          clutter_actor_destroy (actor);
        }
      if (timeline != NULL)
        {
          timeline = NULL;
        }

      load_script (args.path);
    }

  previous_ctime = stat_buf.st_ctime;
  return TRUE;
}

gint
main (gint    argc,
      gchar **argv)
{
  clutter_init (&argc, &argv);

  if (!parse_args (argv))
    return -1;

  stage = initialize_stage ();

  g_timeout_add (1000, watch_file, NULL);

  clutter_main ();
  return 0;
}
