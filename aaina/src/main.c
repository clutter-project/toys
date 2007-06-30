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
#include <string.h>

#include <glib.h>

#include <clutter/clutter.h>

#include <libaaina/aaina-library.h>
#include <libaaina/aaina-source.h>
#include <sources/aaina-source-directory.h>

#include "aaina-slide-show.h"

/* Command line options */
static gboolean fullscreen = FALSE;
static gchar   *directory = NULL;

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
    G_OPTION_ARG_STRING,
    &directory,
    "The directory to load pictures from",
    NULL
  },
  {
    NULL
  }
};

int 
main (int argc, char **argv)
{
  GOptionContext *context;
  AainaLibrary *library;
  AainaSource *source;
  ClutterActor *stage, *show;
  ClutterColor black = {0x00, 0x00, 0x00, 0xff};

  g_thread_init (NULL);
  clutter_init (&argc, &argv);

  /* Options */
  context = g_option_context_new (" - Aaina Options");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_parse (context, &argc, &argv, NULL);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 800, 600);
  clutter_stage_fullscreen (CLUTTER_STAGE (stage));
  clutter_stage_set_color (CLUTTER_STAGE (stage), &black);

  /* Load the test source */
  library = aaina_library_new ();
  source = aaina_source_directory_new (library, directory);

  g_print ("%d\n", aaina_library_photo_count (library));

  show = aaina_slide_show_new ();
  clutter_group_add (CLUTTER_GROUP (stage), show);
  clutter_actor_set_size (show,CLUTTER_STAGE_WIDTH (),CLUTTER_STAGE_HEIGHT ());
  g_object_set (G_OBJECT (show), "library", library, NULL); 
  clutter_actor_set_position (show, 0, -400);

  clutter_actor_show_all (show);
  
  clutter_actor_show_all (stage);

 // clutter_actor_set_scale (stage, 0.25, 0.25);

  clutter_main ();

  return 0;
}
