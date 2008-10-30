#include <clutter/clutter.h>
#include <clutter-gst/clutter-gst.h>
#include <cogl/cogl.h>
#include <gst/gst.h>
#include <unistd.h>
#include <stdio.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "totem-resources.h"

int
main (int argc, char *argv[])
{
  ClutterActor   *video;
  GdkPixbuf      *shot = NULL;
  gint            duration;
  CoglHandle      tex_id;
  CoglPixelFormat format;
  gint            size;
  gint            width;
  gint            height; 
  gint            rowstride;
  guchar         *data = NULL;

  gst_init (&argc, &argv);
  clutter_init (&argc, &argv);

  if (argc < 3)
    {
      g_print ("Usage: %s <path to movie file> <output png>\n", argv[0]);
      exit(-1);
    }

  totem_resources_monitor_start (argv[1], 60 * G_USEC_PER_SEC);

  video = clutter_gst_video_texture_new ();

  if (argv[1][0] == '/')
    clutter_media_set_filename(CLUTTER_MEDIA(video), argv[1]);
  else
    clutter_media_set_uri(CLUTTER_MEDIA(video), argv[1]);
  clutter_media_set_volume (CLUTTER_MEDIA(video), 0);
  clutter_media_set_playing (CLUTTER_MEDIA(video), TRUE);

  do {

    while (g_main_context_pending (NULL))
      g_main_context_iteration (NULL, FALSE);

    duration = clutter_media_get_duration (CLUTTER_MEDIA(video));

  } while (duration == 0);

  clutter_actor_realize (video);

  clutter_media_set_position (CLUTTER_MEDIA(video), duration/3);

  do {

    while (g_main_context_pending (NULL))
      g_main_context_iteration (NULL, FALSE);

  } while (clutter_media_get_position (CLUTTER_MEDIA(video)) <= duration/3);

  tex_id = clutter_texture_get_cogl_texture (CLUTTER_TEXTURE (video));
  if (tex_id)
    {
      format = cogl_texture_get_format (tex_id);
      size = cogl_texture_get_data (tex_id, format, 0, NULL);
      width = cogl_texture_get_width (tex_id);
      height = cogl_texture_get_height (tex_id);
      rowstride = cogl_texture_get_rowstride (tex_id);
      
      data = (guchar*) g_malloc (sizeof(guchar) * size);
      if (data)
	shot = gdk_pixbuf_new_from_data (data, 
					 GDK_COLORSPACE_RGB, 
					 TRUE, 
					 8,
					 width, 
					 height, 
					 rowstride, 
					 NULL, 
					 NULL);
	    
    }

  totem_resources_monitor_stop ();

  if (shot)
    {
      GdkPixbuf *thumb, *pic;
      gint       x, y, nw, nh, w, h, size;
      
      size = 128;

      /* FIXME swap RGB pixels */

      w = clutter_actor_get_width (video);
      h = clutter_actor_get_height (video);
      
      nh = ( h * size) / w;

      if (nh <= size)
	{
	  nw = size;
	  x = 0;
	  y = (size - nh) / 2;
	}
      else
	{
	  nw  = ( w * size ) / h;
	  nh = size;
	  x = (size - nw) / 2;
	  y = 0;
	}

      thumb = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, size, size);
      gdk_pixbuf_fill (thumb, 0x000000FF);

      pic = gdk_pixbuf_scale_simple (shot, nw, nh, GDK_INTERP_BILINEAR);
      gdk_pixbuf_copy_area  (pic, 0, 0, nw, nh, thumb, x, y);
      
      if (!gdk_pixbuf_save (thumb, argv[2], "png", NULL, NULL))
	{
	  g_error ("%s: Pixbuf save failed\n", argv[0]);
	  exit(-1);
	}

      g_object_unref (shot);
      g_object_unref (thumb);
      g_object_unref (pic);

      exit(0);
    }

  exit (-1);
}
