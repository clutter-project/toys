#include <stdio.h>
#include <stdlib.h>

#include <glib.h>
#include <clutter/clutter.h>

#include <libastro-desktop/astro.h>

#include "astro-desktop.h"

/* forwards */
static ClutterActor * load_background (void);

/* Command line options */
static gint     width      = 640;
static gint     height     = 480;
static gboolean fullscreen = FALSE;

static GOptionEntry entries[] =
{
  {
    "width",
    'w', 0,
    G_OPTION_ARG_INT,
    &width,
    "Width of application window. Default: 640",
    NULL
  },
  {
    "height",
    'h', 0,
    G_OPTION_ARG_INT,
    &height,
    "Height of application window. Default: 480",
    NULL
  },
  {
    "fullscreen",
    'f', 0,
    G_OPTION_ARG_NONE,
    &fullscreen,
    "Whether the application window should be fullscreen.",
    NULL
  },  
  {
    NULL
  }
};

gint
main (gint argc, gchar *argv[])
{
  ClutterActor *stage, *bg, *desktop;
  ClutterColor black = { 0xff, 0xff, 0xff, 0xff };
  GError *error = NULL;

  g_thread_init (NULL);
  gdk_init (&argc, &argv);
  
  clutter_init_with_args (&argc, &argv,
                          " - Astro Desktop", entries,
                          NULL, &error);
  if (error)
    {
      g_error ("Unable to run Astro Desktop: %s", error->message);
      g_error_free (error);
      return EXIT_FAILURE;
    }

  /* Set up the stage */
  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, width, height);
  
  if (fullscreen)
    clutter_stage_fullscreen (CLUTTER_STAGE (stage));

  /* Draw the background */
  bg = load_background ();
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), bg);
  clutter_actor_set_position (bg, 0, 0);

  /* Load the desktop */
  desktop = astro_desktop_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), desktop);
  clutter_actor_set_size (desktop, CSW (), CSH ());
  clutter_actor_set_position (desktop, 0, 0);
  
  clutter_actor_show_all (stage);
  
  clutter_main ();

  return EXIT_SUCCESS;
}

static ClutterActor * 
load_background (void)
{
  ClutterActor *texture;
  GdkPixbuf *pixbuf;

  texture = clutter_texture_new ();
  pixbuf = gdk_pixbuf_new_from_file_at_scale (PKGDATADIR "/background.svg",
                                              CSW (),
                                              CSH (),
                                              FALSE,
                                              NULL);
  if (pixbuf)
    clutter_texture_set_pixbuf (CLUTTER_TEXTURE (texture), pixbuf, NULL);

  return texture;
}
