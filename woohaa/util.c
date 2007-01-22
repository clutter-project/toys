#include "util.h"
#include "math.h"

typedef struct FadeClosure
{
  ClutterActor     *actor;
  ClutterTimeline  *timeline;
  ClutterAlpha     *alpha;
  ClutterBehaviour *behave;
  gulong            signal_id;
}
FadeClosure;


static void
util_fade_closure_destroy (FadeClosure *f)
{
  g_signal_handler_disconnect (f->timeline, f->signal_id);
  clutter_behaviour_remove (f->behave, f->actor);

  g_object_unref(f->actor);
  g_object_unref(f->behave);
  g_object_unref(f->alpha);
  g_object_unref(f->timeline);

  g_free(f);
}

static FadeClosure*
util_fade_closure_new (ClutterActor *actor, GCallback complete)
{
  FadeClosure *f;

  f = g_new0(FadeClosure, 1);

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
  FadeClosure *f =  (FadeClosure*)user_data;

  clutter_actor_show_all (f->actor);

  util_fade_closure_destroy (f);
}

static void
util_actor_fade_out_complete (ClutterTimeline *timeline,
			     gpointer         user_data)
{
  FadeClosure *f =  (FadeClosure*)user_data;

  clutter_actor_hide_all (f->actor);

  util_fade_closure_destroy (f);
}


ClutterTimeline*
util_actor_fade_in (ClutterActor *actor)
{
  FadeClosure *f;

  f = util_fade_closure_new (actor, G_CALLBACK (util_actor_fade_in_complete));

  f->behave = clutter_behaviour_opacity_new (f->alpha, 0, 0xff);

  clutter_actor_set_opacity (actor, 0);
  clutter_actor_show_all (actor);

  clutter_behaviour_apply (f->behave, actor);
  clutter_timeline_start (f->timeline);

  return f->timeline;
}

ClutterTimeline*
util_actor_fade_out (ClutterActor *actor)
{
  FadeClosure *f;

  f = util_fade_closure_new (actor, G_CALLBACK (util_actor_fade_out_complete));

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
      actor =  clutter_texture_new_from_pixbuf (pixbuf);
      g_object_unref (pixbuf);
      return actor;
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

