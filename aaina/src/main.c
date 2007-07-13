/*
 * Authored By Neil Jagdish Patel <njp@o-hand.com>
 *
 * Copyright (C) 2007 OpenedHand
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <clutter/clutter.h>

#include <libaaina/aaina-library.h>
#include <libaaina/aaina-source.h>
#include <libaaina/aaina-behave.h>

#include <sources/aaina-source-directory.h>
#include <sources/aaina-source-flickr.h>

#include "aaina-slide-show.h"

/* Command line options */
static gboolean   fullscreen  = FALSE;
static gchar    **directories = NULL;
static gchar     *flickr_tags = NULL;

static GOptionEntry entries[] =
{
  {
    "fullscreen",
    'f', 0,
    G_OPTION_ARG_NONE,
    &fullscreen,
    "Launch Aaina in fullscreen mode",
    NULL
  },
  {
    "directory",
    'd', 0,
    G_OPTION_ARG_FILENAME_ARRAY,
    &directories,
    "The directory to load pictures from",
    "PATH"
  },
  {
    "tag",
    't', 0,
    G_OPTION_ARG_STRING,
    &flickr_tags,
    "A set of comma-separated tags to search flickr with",
    "TAG"
  },
  {
    NULL
  }
};
  
static void on_key_release_event (ClutterStage *stage, 
                                  ClutterEvent *event,
                                  gpointer      null);

static void spin_me (ClutterBehaviour *behave, 
                     guint32           alpha_value,
                     gpointer          null);

static gboolean
im_spinning_around (ClutterTimeline *timeline)
{
  clutter_timeline_start (timeline);
  return TRUE;
}
  
int 
main (int argc, char **argv)
{
  AainaLibrary *library;
  AainaSource *source;
  ClutterActor *stage;
  AainaSlideShow *show;
  ClutterTimeline *timeline;
  ClutterAlpha *alpha;
  ClutterBehaviour *behave;
  ClutterColor black = { 0x00, 0x00, 0x00, 0xff };
  GError *error = NULL;

  g_thread_init (NULL);

  g_set_application_name ("Aaina Image Slideshow");
  clutter_init_with_args (&argc, &argv,
                          " - Aaina Image Slideshow", entries,
                          NULL,
                          &error);
  if (error)
    {
      g_print ("Unable to run Aaina: %s", error->message);
      g_error_free (error);
      return EXIT_FAILURE;
    }

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 720, 480);
  clutter_stage_hide_cursor (CLUTTER_STAGE (stage));
  
  if (fullscreen)
    clutter_stage_fullscreen (CLUTTER_STAGE (stage));

  clutter_stage_set_color (CLUTTER_STAGE (stage), &black);

  /* Load the test source */
  library = aaina_library_new ();

  if (directories && directories[0])
    {
      gint n_directories, i;

      n_directories = g_strv_length (directories);
      for (i = 0; i < n_directories; i++)
        source = aaina_source_directory_new (library, directories[i]);
    }
  else if (flickr_tags)
    source = aaina_source_flickr_new (library, flickr_tags);
  else
    {
      g_print ("Usage: aaina -d <path>\n"
               "       aaina -t <tag>[,<tag>,....]\n");
      return EXIT_FAILURE;
    }

  show = aaina_slide_show_get_default ();
  g_object_set (G_OBJECT (show), "library", library, NULL);

  clutter_actor_show_all (stage);

  /*clutter_actor_set_scale (stage, 0.25, 0.25);*/

  g_signal_connect (G_OBJECT (stage), "key-release-event",
                    G_CALLBACK (on_key_release_event), (gpointer)stage);

  
  timeline = clutter_timeline_new (60, 30);
  alpha = clutter_alpha_new_full (timeline,
                                  alpha_sine_inc_func,
                                  NULL, NULL);
  behave = aaina_behave_new (alpha, 
                             (AainaBehaveAlphaFunc)spin_me,
                             (gpointer)stage);
  
  clutter_actor_rotate_y (stage, 0, 
                          CLUTTER_STAGE_WIDTH ()/2,
                          CLUTTER_STAGE_HEIGHT ());

  g_timeout_add (10000, (GSourceFunc)im_spinning_around, timeline);
  clutter_main ();

  return EXIT_SUCCESS;
}

static void
on_key_release_event (ClutterStage *stage, 
                      ClutterEvent *event,
                      gpointer      null)
{
  static gint i = 0;
  
  switch (clutter_key_event_symbol ((ClutterKeyEvent*)event))
  {
    case CLUTTER_Escape:
      clutter_main_quit ();
      break;
    case CLUTTER_Left:
       i--;
       if (i == 0)
         i = 359;
       clutter_actor_rotate_y (CLUTTER_ACTOR (stage), i, 
                               CLUTTER_STAGE_WIDTH ()/2,
                               CLUTTER_STAGE_HEIGHT ());
       break;
    case CLUTTER_Right:
       i++;
       if (i == 360)
         i = 0;
       clutter_actor_rotate_y (CLUTTER_ACTOR (stage), i, 
                               CLUTTER_STAGE_WIDTH ()/2,
                               CLUTTER_STAGE_HEIGHT ());
       break;

    default:
      break;
  }
} 


static void
spin_me (ClutterBehaviour *behave, 
         guint32           alpha_value,
         gpointer          null)
{
 gfloat factor = (gfloat)alpha_value / CLUTTER_ALPHA_MAX_ALPHA;
 
 clutter_actor_rotate_y (clutter_stage_get_default (),
                         factor * 360,
                         CLUTTER_STAGE_WIDTH ()/2,
                         CLUTTER_STAGE_HEIGHT ());

}
