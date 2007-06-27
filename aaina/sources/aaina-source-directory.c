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

#include <libaaina/aaina-photo.h>

#include "aaina-source-directory.h"

G_DEFINE_TYPE (AainaSourceDirectory, aaina_source_directory, AAINA_TYPE_SOURCE);
	
static void
_load_photos (AainaLibrary *library, const gchar *directory)
{
  GDir *dir;
  const gchar *name;
  
  g_print ("Scanning : %s\n", directory);

  dir = g_dir_open (directory, 0, NULL);
  while ((name = g_dir_read_name (dir)))
  {
    gchar *path = g_build_filename (directory, name, NULL);

    if (g_file_test (path, G_FILE_TEST_IS_DIR))
    {
      _load_photos (library, path);
    }
    else
    {
      GdkPixbuf *pixbuf = NULL;
      pixbuf = gdk_pixbuf_new_from_file (path, NULL);
      if (pixbuf)
      {
        ClutterActor *photo = aaina_photo_new ();
        g_object_set (G_OBJECT (photo), "pixbuf", pixbuf, NULL);
        aaina_library_append_photo (library, AAINA_PHOTO (photo));

      }
    }
    g_free (path);
  }
  g_dir_close (dir);
}

/* GObject stuff */
static void
aaina_source_directory_class_init (AainaSourceDirectoryClass *klass)
{
  ;
}


static void
aaina_source_directory_init (AainaSourceDirectory *source_directory)
{
  ;
}

AainaSource*
aaina_source_directory_new (AainaLibrary *library, const gchar *dir)
{
  AainaSourceDirectory         *source_directory;

  source_directory = g_object_new (AAINA_TYPE_SOURCE_DIRECTORY, 
                                   NULL);

  _load_photos (library, dir);
  
  return AAINA_SOURCE (source_directory);
}

