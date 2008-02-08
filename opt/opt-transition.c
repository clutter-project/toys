#include "opt.h"

#define FPS    90
#define FRAMES 30

G_DEFINE_TYPE (OptTransition, opt_transition, CLUTTER_TYPE_TIMELINE);

struct OptTransitionPrivate
{
  OptTransitionStyle      style;
  OptSlide               *from, *to;
  gulong                  signal_id;
  OptTransitionDirection  direction;
};

static void
yz_flip_transition_frame_cb (OptTransition   *trans,
			     gint             frame_num,
			     gpointer         data)
{
  OptSlide             *from, *to;
  OptTransitionPrivate *priv;
  ClutterActor         *stage;
  gint                  n_frames = 0; 

  priv = trans->priv;

  from  = opt_transition_get_from (trans);
  to    = opt_transition_get_to (trans);
  stage = clutter_stage_get_default();

  n_frames = clutter_timeline_get_n_frames (CLUTTER_TIMELINE(trans));

  if (frame_num > n_frames/2)
    {
      clutter_actor_hide (CLUTTER_ACTOR(from));
      clutter_actor_show_all (CLUTTER_ACTOR(to));
      clutter_actor_set_depth (CLUTTER_ACTOR(to), 
			       -1 * ((n_frames * 2000) - (frame_num * 2000)));

      clutter_actor_set_rotation (CLUTTER_ACTOR(to),
            CLUTTER_Z_AXIS,
            frame_num * (360/n_frames/2),
            CLUTTER_STAGE_WIDTH()/2,
            CLUTTER_STAGE_HEIGHT()/2,
            0);
    }
  else
    {
      clutter_actor_hide (CLUTTER_ACTOR(to));
      clutter_actor_set_depth (CLUTTER_ACTOR(from), -2000 * frame_num);

      clutter_actor_set_rotation (CLUTTER_ACTOR(from),
            CLUTTER_Z_AXIS,
            frame_num * (360/n_frames/2),
            CLUTTER_STAGE_WIDTH()/2,
            CLUTTER_STAGE_HEIGHT()/2,
            0);
    }
}

static void
zoom_transition_frame_cb (OptTransition   *trans,
			  gint             frame_num,
			  gpointer         data)
{
  OptSlide             *from, *to;
  OptTransitionPrivate *priv;
  ClutterActor         *stage;
  gint                  n_frames = 0; 

  priv = trans->priv;

  from  = opt_transition_get_from (trans);
  to    = opt_transition_get_to (trans);
  stage = clutter_stage_get_default();

  n_frames = clutter_timeline_get_n_frames (CLUTTER_TIMELINE(trans));

  if (frame_num > n_frames/2)
    {
      clutter_actor_hide (CLUTTER_ACTOR(from));
      clutter_actor_show_all (CLUTTER_ACTOR(to));
      clutter_actor_set_depth (CLUTTER_ACTOR(to), 
			       -1 * ((n_frames * 2000) - (frame_num * 2000)));

      clutter_actor_set_rotation (CLUTTER_ACTOR(to),
            CLUTTER_Z_AXIS,
            frame_num * ((360*2)/n_frames),
            CLUTTER_STAGE_WIDTH()/2,
            CLUTTER_STAGE_HEIGHT()/2,
            0);
    }
  else
    {
      clutter_actor_hide (CLUTTER_ACTOR(to));
      clutter_actor_set_depth (CLUTTER_ACTOR(from), -2000 * frame_num);

      clutter_actor_set_rotation (CLUTTER_ACTOR(from),
            CLUTTER_Z_AXIS,
            frame_num * ((360*2)/n_frames),
            CLUTTER_STAGE_WIDTH()/2,
            CLUTTER_STAGE_HEIGHT()/2,
            0);
    }
}

static void
flip_transition_frame_cb (OptTransition   *trans,
			  gint             frame_num,
			  gpointer         data)
{
  OptSlide             *from, *to;
  OptTransitionPrivate *priv;
  ClutterColor          color = { 0x22, 0x22, 0x22, 0xff };
  ClutterActor       *stage;
  gint                  mult, n_frames;

  priv = trans->priv;

  from = opt_transition_get_from (trans);
  to   = opt_transition_get_to (trans);
  stage = clutter_stage_get_default();

  clutter_actor_show_all (CLUTTER_ACTOR(to));

  mult = priv->direction ? 1 : -1;

  n_frames = clutter_timeline_get_n_frames (CLUTTER_TIMELINE(trans));

  if (frame_num > n_frames/2)
    {
      /* Fix Z ordering */
      clutter_actor_lower_bottom (CLUTTER_ACTOR(from));
    }

  clutter_stage_set_color (CLUTTER_STAGE(stage), &color);

  clutter_actor_set_rotation (CLUTTER_ACTOR(from),
                              CLUTTER_Y_AXIS,
                              - (float)frame_num * 6 * mult,
                              CLUTTER_STAGE_WIDTH ()/2,
                              0,
                              0);

  clutter_actor_set_rotation (CLUTTER_ACTOR(to),
                              CLUTTER_Y_AXIS,
                              180 - (frame_num * 6) * mult,
                              CLUTTER_STAGE_WIDTH()/2,
                              0,
                              0);
}

static void
cube_transition_frame_cb (OptTransition   *trans,
			  gint             frame_num,
			  gpointer         data)
{
  OptSlide             *from, *to;
  ClutterActor       *stage;
  ClutterColor          color = { 0x22, 0x22, 0x22, 0xff };
  OptTransitionPrivate *priv;
  gint                  mult, n_frames;

  priv = trans->priv;

  from  = opt_transition_get_from (trans);
  to    = opt_transition_get_to (trans);
  stage = clutter_stage_get_default();

  clutter_actor_show_all (CLUTTER_ACTOR(to));

  mult = priv->direction ? -1 : 1;

  n_frames = clutter_timeline_get_n_frames (CLUTTER_TIMELINE(trans));

  if (frame_num > n_frames/2)
    {
      /* Fix Z ordering */
      clutter_actor_lower_bottom (CLUTTER_ACTOR(from));
    }

  clutter_stage_set_color (CLUTTER_STAGE(stage), &color);

  clutter_actor_set_rotation (CLUTTER_ACTOR(from),
                              CLUTTER_Y_AXIS,
                              - (float)frame_num * 3 * mult,
                              CLUTTER_STAGE_WIDTH()/2,
                              0,
                              -1 * (CLUTTER_STAGE_WIDTH()/2));

  clutter_actor_set_rotation (CLUTTER_ACTOR(to),
                              CLUTTER_Y_AXIS,
                              (mult * 90) - (frame_num * 3 * mult),
                              CLUTTER_STAGE_WIDTH()/2,
                              0,
                              -1 * (CLUTTER_STAGE_WIDTH()/2));
}

static void
page_transition_frame_cb (OptTransition   *trans,
			     gint             frame_num,
			     gpointer         data)
{
  OptSlide             *from, *to;
  ClutterActor       *stage;
  ClutterColor          color = { 0x22, 0x22, 0x22, 0xff };
  OptTransitionPrivate *priv;
  gint                  mult, n_frames;

  priv = trans->priv;

  from  = opt_transition_get_from (trans);
  to    = opt_transition_get_to (trans);
  stage = clutter_stage_get_default();

  clutter_actor_show_all (CLUTTER_ACTOR(to));

  mult = priv->direction ? -1 : 1;

  n_frames = clutter_timeline_get_n_frames (CLUTTER_TIMELINE(trans));

  if (frame_num > n_frames/2)
    {
      /* Fix Z ordering */
      clutter_actor_lower_bottom (CLUTTER_ACTOR(from));
    }

  clutter_stage_set_color (CLUTTER_STAGE(stage), &color);

  clutter_actor_set_rotation (CLUTTER_ACTOR(from),
                              CLUTTER_Y_AXIS,
                              - (float)frame_num * 2 * mult,
                              CLUTTER_STAGE_WIDTH()*3/2,
                              0,
                              -1 * (CLUTTER_STAGE_WIDTH()/2));

  clutter_actor_set_rotation (CLUTTER_ACTOR(to),
                              CLUTTER_Y_AXIS,
                              (mult * 60) - (frame_num * 2 * mult),
                              CLUTTER_STAGE_WIDTH()*3/2,
                              0,
                              -1 * (CLUTTER_STAGE_WIDTH()/2));
}

static void
fade_transition_frame_cb (OptTransition   *trans,
			  gint             frame_num,
			  gpointer         data)
{
  OptSlide             *from, *to;
  OptTransitionPrivate *priv;
  gint                  opacity;

  priv = trans->priv;

  from = opt_transition_get_from (trans);
  to   = opt_transition_get_to (trans);

  if (frame_num == 0)
    {
      clutter_actor_show_all (CLUTTER_ACTOR(to));
      clutter_actor_raise_top (CLUTTER_ACTOR(to));
    }

  opacity = (frame_num * 255 ) 
                  / clutter_timeline_get_n_frames (CLUTTER_TIMELINE(trans));

  clutter_actor_set_opacity (CLUTTER_ACTOR(to), opacity);

  /* clutter_actor_set_depth (CLUTTER_ACTOR(from), - opacity/10 ); */
}

static void 
opt_transition_dispose (GObject *object)
{
  OptTransition *self = OPT_TRANSITION(object); 

  if (self->priv)
    {
      opt_transition_set_from (self, NULL);
      opt_transition_set_to (self, NULL);
    }

  G_OBJECT_CLASS (opt_transition_parent_class)->dispose (object);
}

static void 
opt_transition_finalize (GObject *object)
{
  OptTransition *self = OPT_TRANSITION(object); 

  if (self->priv)
    {
      g_free(self->priv);
      self->priv = NULL;
    }

  G_OBJECT_CLASS (opt_transition_parent_class)->finalize (object);
}

static void
opt_transition_class_init (OptTransitionClass *klass)
{
  GObjectClass        *object_class;

  object_class = (GObjectClass*) klass;

  object_class->finalize     = opt_transition_finalize;
  object_class->dispose      = opt_transition_dispose;
}

static void
opt_transition_init (OptTransition *self)
{
  OptTransitionPrivate *priv;

  priv  = g_new0 (OptTransitionPrivate, 1);

  self->priv = priv;

}

OptTransition*
opt_transition_new (OptTransitionStyle style)
{
  OptTransition *trans;

  trans = g_object_new (OPT_TYPE_TRANSITION, 
		       "fps", FPS, 
		       "num-frames", FRAMES, 
			NULL);

  opt_transition_set_style (trans, style);

  return trans;
}

OptTransitionStyle
opt_transition_get_style (OptTransition     *trans)
{
  return trans->priv->style;
}

void
opt_transition_set_style (OptTransition     *trans,
			  OptTransitionStyle style)
{
  OptTransitionPrivate *priv;

  priv = trans->priv;

  if (priv->signal_id)
    g_signal_handler_disconnect (trans, priv->signal_id);

  switch (style)
    {
    case OPT_TRANSITION_CUBE:
      priv->signal_id 
	= g_signal_connect (trans,
			    "new-frame",  
			    G_CALLBACK (cube_transition_frame_cb), 
			    trans);
      break;
    case OPT_TRANSITION_PAGE:
      priv->signal_id 
	= g_signal_connect (trans,
			    "new-frame",  
			    G_CALLBACK (page_transition_frame_cb), 
			    trans);
      break;
    case OPT_TRANSITION_FLIP:
      priv->signal_id 
	= g_signal_connect (trans,
			    "new-frame",  
			    G_CALLBACK (flip_transition_frame_cb), 
			    trans);
      break;
    case OPT_TRANSITION_YZ_FLIP:
      priv->signal_id 
	= g_signal_connect (trans,
			    "new-frame",  
			    G_CALLBACK (yz_flip_transition_frame_cb), 
			    trans);
      break;
    case OPT_TRANSITION_ZOOM:
      priv->signal_id 
	= g_signal_connect (trans,
			    "new-frame",  
			    G_CALLBACK (zoom_transition_frame_cb), 
			    trans);
      break;
    case OPT_TRANSITION_FADE:
    default:
      priv->signal_id 
	= g_signal_connect (trans,
			    "new-frame",  
			    G_CALLBACK (fade_transition_frame_cb), 
			    trans);
      break;
    }

  trans->priv->style = style;
}

void
opt_transition_set_from (OptTransition *trans, OptSlide *slide)
{
  OptTransitionPrivate *priv;

  priv = trans->priv;

  if (priv->from == slide)
    return;

  if (priv->from != NULL)
    g_object_unref(priv->from);

  priv->from = slide;
  if (slide != NULL)
    g_object_ref(slide);
}

void
opt_transition_set_to (OptTransition *trans, OptSlide *slide)
{
  OptTransitionPrivate *priv;

  priv = trans->priv;

  if (priv->to == slide)
    return;

  if (priv->to != NULL)
    g_object_unref(priv->to);

  priv->to = slide;
  if (slide != NULL)
    g_object_ref(slide);
}

OptSlide*
opt_transition_get_from (OptTransition *trans)
{
  return trans->priv->from;
}

OptSlide*
opt_transition_get_to (OptTransition *trans)
{
  return trans->priv->to;
}

void
opt_transition_set_direction (OptTransition           *trans, 
			      OptTransitionDirection   direction)
{
  trans->priv->direction = direction;
}
