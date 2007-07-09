#include "util.h"
#include "math.h"

#include <gdk/gdk.h>

ClutterActor*
util_actor_from_file (const gchar *path, int width, int height)
{
  ClutterActor  *actor;
  GdkPixbuf     *pixbuf;

  pixbuf = gdk_pixbuf_new_from_file_at_size (path, width, height, NULL); 

  if (pixbuf)
    {
      actor = clutter_texture_new_from_pixbuf (pixbuf);
      g_object_unref (pixbuf);
      return actor;
    }
  
  return NULL;
}

ClutterActor*
util_texture_from_root_window (void)
{
  ClutterActor *texture = NULL;
  GdkWindow    *root;
  GdkPixbuf    *pixbuf;

  gdk_init(NULL, NULL);

  root = gdk_get_default_root_window ();
  
  pixbuf = gdk_pixbuf_get_from_drawable (NULL, 
					 root, 
					 NULL,
					 0, 
					 0, 
					 0, 
					 0, 
					 gdk_screen_width(), 
					 gdk_screen_height());

  if (pixbuf)
    {
      texture = clutter_texture_new_from_pixbuf (pixbuf);
      g_object_unref (pixbuf);
      return texture;
    }

  return NULL;
}

guint32 
alpha_sine_inc_func (ClutterAlpha *alpha,
		     gpointer      dummy)
{
  ClutterTimeline *timeline;
  gint current_frame_num, n_frames;
  gdouble x, sine;
  
  timeline = clutter_alpha_get_timeline (alpha);

  current_frame_num = clutter_timeline_get_current_frame (timeline);
  n_frames = clutter_timeline_get_n_frames (timeline);

  x = (gdouble) (current_frame_num * 0.5f * M_PI) / n_frames ;
  /* sine = (sin (x - (M_PI / 0.5f)) + 1.0f) * 0.5f; */
  sine = (sin (x - (M_PI / 0.5f))) ;

  return (guint32) (sine * (gdouble) CLUTTER_ALPHA_MAX_ALPHA);
}

