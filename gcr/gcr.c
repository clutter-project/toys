/* Minimal screencast recorder for clutter (GL) using GEGL.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Copyright (C) 2008 Øyvind Kolås
 * Copyright (C) 2008 OpenedHand
 */

#include <gegl.h>
#include <clutter/clutter.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <string.h>

/* some basic configuration */
#define FPS 25
#define KBITRATE (7000*8)  

/* this needs to be called before clutter_main()  */
void gcr (const gchar *path);


/* TODO: Move away from pixbufs, the original code used a pixbuf but
 * with newer GEGL it should be better to use a linear buffer directly
 */

static GThread      *encode_thread   = NULL;
static GStaticMutex  mutex           = G_STATIC_MUTEX_INIT;
static gint          prev_width      = 0;
static gint          prev_height     = 0;
static gint          frames          = 0;
static guchar       *pixels          = NULL;
static guchar       *pixels_inverted = NULL;
static gboolean      got_data        = FALSE;
static GdkPixbuf    *pixbuf          = NULL;
static GeglNode     *gegl            = NULL;
static GeglNode     *load_pixbuf     = NULL;
static GeglNode     *ff_save;
static long          prev_stored     = 0;

static gpointer encoder (gpointer data);
static void save_frame  (gpointer data);

long babl_ticks (void);

void gcr (const gchar *path)
{
  ClutterActor *stage = clutter_stage_get_default ();

  if (!g_thread_supported ()) g_thread_init (NULL);

  g_signal_connect_after (stage, "paint", G_CALLBACK (save_frame), NULL);

  encode_thread = g_thread_create (encoder, NULL, FALSE, NULL);

  gegl_init (NULL, NULL);
  gegl = gegl_node_new ();

  ff_save = gegl_node_new_child (gegl,
     "operation", "ff-save",
     "bitrate",   KBITRATE *1000.0,
     "fps",       (FPS * 1.0),
     "path",      path,
     NULL
    );
  load_pixbuf = gegl_node_create_child (gegl, "pixbuf");
  gegl_node_link (load_pixbuf, ff_save);
  prev_stored = babl_ticks ();
}

/* this is called by clutter each time a stage has been rendered */
static void save_frame (gpointer data)
{

  GLint      viewport[4];
  gint       x, y, width, height;
  glong      delta;

  /* issue commands that might make sure we're fully rendered ... */
  glFinish ();
  glFlush ();

  glGetIntegerv(GL_VIEWPORT, viewport);
 
  x         = viewport[0]; 
  y         = viewport[1]; 
  width     = viewport[2] - x;
  height    = viewport[3] - y;

  /* by locking here we wait until the encoder thread is finished encoding */
  g_static_mutex_lock (&mutex);

  /* we only create the pixbuf on the first frame (hopefully) */

  if (prev_width  != width ||
      prev_height != height)
    {
      if (pixels)
        g_free (pixels);
      if (pixels_inverted)
        g_free (pixels_inverted);
      if (pixbuf)
        g_object_unref (pixbuf);
      pixels_inverted = g_malloc (width * height * 4);
      pixels = g_malloc (width * height * 4);

      pixbuf = gdk_pixbuf_new_from_data (pixels, GDK_COLORSPACE_RGB, TRUE, 8,
                                         width, height, width * 4, NULL, NULL);

      prev_width = width;
      prev_height = height;
    }

  /* figure out the time elapsed since the previously stored encoded frame */
  delta = babl_ticks () - prev_stored;

  if (got_data == FALSE && delta >= 1000000.0/FPS)
    {
      prev_stored += delta;
      glReadPixels (x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels_inverted);
      /* we only do the read pixels here, the encoder thread does the
       * inversion
       */
      got_data = TRUE;

      frames = delta / (1000000.0/FPS);
      /* FIXME: * precision loss, we need to keep a remainder around to produce
       * the correct framerate */
    }
  g_static_mutex_unlock (&mutex);
}

static gpointer encoder (gpointer data)
{
  while (TRUE)
    {
      gint rowstride;
      gint width, height;
      gint repeats = 0;

      gboolean action = FALSE;

      /* critical section */
      g_static_mutex_lock (&mutex);
      if (got_data)
        {
          gint y;
          height = prev_height;
          width = prev_width;
          rowstride = prev_width * 4;

          /* flip the image right side up when copying it */
          for (y=0; y<height; y++)
            {
              memcpy (pixels + ((height-y-1) * rowstride),
                      pixels_inverted + y * rowstride, rowstride);
            }
          got_data = FALSE;
          action = TRUE;  /* store that we've got work to do */
          repeats = frames;
        }
      g_static_mutex_unlock (&mutex);

      if (action)
        {
          static     gint dump_no = -1;

          if (dump_no<=0)
            {
              dump_no++;
            }
          else
            {
              /* encode the current frame for the number of frames
               * that is needed
               */
              while (repeats--)
                {
                  gegl_node_set (load_pixbuf, "pixbuf", pixbuf, NULL);
                  gegl_node_process (ff_save);
                }
            }
        }
      else
        {
          g_usleep (100);
        }
    }
}
