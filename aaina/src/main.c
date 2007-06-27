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

  g_thread_init (NULL);
  clutter_init (&argc, &argv);

  /* Options */
  context = g_option_context_new (" - Aaina Options");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_parse (context, &argc, &argv, NULL);

  /* Load the test source */
  library = aaina_library_new ();
  source = aaina_source_directory_new (library, directory);

  g_print ("%d\n", aaina_library_photo_count (library));

  return 0;
}
