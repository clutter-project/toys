#include "util.h"
#include "math.h"

#include <gdk/gdk.h>

typedef struct AnimClosure
{
  ClutterActor        *actor;
  ClutterTimeline     *timeline;
  ClutterAlpha        *alpha;
  ClutterBehaviour    *behave;
  gulong               signal_id;
  UtilAnimCompleteFunc completed_func;
  gpointer             completed_data;
}
AnimClosure;

static void
util_anim_closure_destroy (AnimClosure *f)
{
  g_signal_handler_disconnect (f->timeline, f->signal_id);
  clutter_behaviour_remove (f->behave, f->actor);

  g_object_unref(f->actor);
  g_object_unref(f->behave);
  g_object_unref(f->alpha);
  g_object_unref(f->timeline);

  g_free(f);
}

static AnimClosure*
util_anim_closure_new (ClutterActor *actor, GCallback complete)
{
  AnimClosure *f;

  f = g_new0(AnimClosure, 1);

  g_object_ref (actor);

  f->actor    = actor;
  f->timeline = clutter_timeline_new (40, 120);
  f->alpha    = clutter_alpha_new_full (f->timeline,
					alpha_sine_inc_func,
					NULL, NULL);

  f->signal_id  = g_signal_connect (f->timeline,
				    "completed",
				    G_CALLBACK(complete),
				    f);
  return f;
}

static void
util_actor_fade_in_complete (ClutterTimeline *timeline,
			     gpointer         user_data)
{
  AnimClosure *f =  (AnimClosure*)user_data;

  clutter_actor_show_all (f->actor);

  if (f->completed_func)
    f->completed_func(f->actor, f->completed_data);

  util_anim_closure_destroy (f);
}

static void
util_actor_fade_out_complete (ClutterTimeline *timeline,
			     gpointer         user_data)
{
  AnimClosure *f =  (AnimClosure*)user_data;

  clutter_actor_hide_all (f->actor);

  if (f->completed_func)
    f->completed_func(f->actor, f->completed_data);

  util_anim_closure_destroy (f);
}

ClutterTimeline*
util_actor_fade_in (ClutterActor        *actor, 
		    UtilAnimCompleteFunc func, 
		    gpointer             data)
{
  AnimClosure *f;
  
  f = util_anim_closure_new (actor, G_CALLBACK (util_actor_fade_in_complete));

  f->completed_func = func;
  f->completed_data = data;

  f->behave = clutter_behaviour_opacity_new (f->alpha, 0, 0xff);
  
  clutter_actor_set_opacity (actor, 0);
  clutter_actor_show_all (actor);
  
  clutter_behaviour_apply (f->behave, actor);
  clutter_timeline_start (f->timeline);
  
  return f->timeline;
}

static void
util_actor_fade_complete (ClutterTimeline *timeline,
			  gpointer         user_data)
{
  AnimClosure *f =  (AnimClosure*)user_data;

  if (f->completed_func)
    f->completed_func(f->actor, f->completed_data);

  util_anim_closure_destroy (f);
}

ClutterTimeline*
util_actor_fade (ClutterActor        *actor, 
		 UtilAnimCompleteFunc func,
		 guint8               start_opacity,
		 guint8               end_opacity,
		 gpointer             data)
{
  AnimClosure *f;

  f = util_anim_closure_new (actor, G_CALLBACK (util_actor_fade_complete));

  f->completed_func = func;
  f->completed_data = data;

  f->behave = clutter_behaviour_opacity_new (f->alpha, 
					     start_opacity, 
					     end_opacity);
  
  clutter_actor_set_opacity (actor, start_opacity);
  clutter_actor_show_all (actor);
  
  clutter_behaviour_apply (f->behave, actor);
  clutter_timeline_start (f->timeline);
  
  return f->timeline;
}

static void
util_actor_zoom_complete (ClutterTimeline *timeline,
			  gpointer         user_data)
{
  AnimClosure *f =  (AnimClosure*)user_data;

  if (f->completed_func)
    f->completed_func(f->actor, f->completed_data);

  util_anim_closure_destroy (f);
}

ClutterTimeline*
util_actor_zoom (ClutterActor        *actor, 
		 UtilAnimCompleteFunc func,
		 gdouble              start_scale,
		 gdouble              end_scale,
		 gpointer             data)
{
  AnimClosure *f;

  f = util_anim_closure_new (actor, G_CALLBACK (util_actor_zoom_complete));

  f->completed_func = func;
  f->completed_data = data;

  f->behave = clutter_behaviour_scale_new (f->alpha,
					   start_scale,
					   end_scale,
					   CLUTTER_GRAVITY_CENTER);

  clutter_behaviour_apply (f->behave, actor);
  clutter_timeline_start (f->timeline);
  
  return f->timeline;
}

ClutterTimeline*
util_actor_fade_out (ClutterActor        *actor,
		     UtilAnimCompleteFunc func, 
		     gpointer             data)
{
  AnimClosure *f;

  f = util_anim_closure_new (actor, G_CALLBACK (util_actor_fade_out_complete));

  f->completed_func = func;
  f->completed_data = data;

  f->behave = clutter_behaviour_opacity_new (f->alpha, 0xff, 0);

  clutter_behaviour_apply (f->behave, actor);
  clutter_timeline_start (f->timeline);

  return f->timeline;
}

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

