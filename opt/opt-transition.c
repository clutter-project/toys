#include "opt.h"

#define FPS    60
#define FRAMES 30

G_DEFINE_TYPE (OptTransition, opt_transition, CLUTTER_TYPE_TIMELINE);

struct OptTransitionPrivate
{
  OptTransitionStyle  style;
  OptSlide           *from, *to;
  gulong              signal_id;
};

static void
yz_flip_transition_frame_cb (OptTransition   *trans,
			     gint             frame_num,
			     gpointer         data)
{
  OptSlide             *from, *to;
  OptTransitionPrivate *priv;
  ClutterElement       *stage;
  ClutterColor          color = { 0x22, 0x22, 0x22, 0xff };

  priv = trans->priv;

  from  = opt_transition_get_from (trans);
  to    = opt_transition_get_to (trans);
  stage = clutter_stage_get_default();

  clutter_group_show_all (CLUTTER_GROUP(to));

  if (frame_num > 15)
    {
      /* Fix Z ordering */
      clutter_element_lower_bottom (CLUTTER_ELEMENT(from));
    }

  clutter_stage_set_color (CLUTTER_STAGE(stage), &color);

  clutter_element_rotate_y (CLUTTER_ELEMENT(from),
                            - (float)frame_num * 6,
                            CLUTTER_STAGE_WIDTH()/2,
                            0);

  clutter_element_rotate_y (CLUTTER_ELEMENT(to),
                            180 - (frame_num * 6),
                            CLUTTER_STAGE_WIDTH()/2,
                            0);

  clutter_element_rotate_z (CLUTTER_ELEMENT(from),
                            - (float)frame_num * 6,
                            CLUTTER_STAGE_WIDTH()/2,
                            0);

  clutter_element_rotate_z (CLUTTER_ELEMENT(to),
                            180 - (frame_num * 6),
                            CLUTTER_STAGE_WIDTH()/2,
                            0);
}

static void
flip_transition_frame_cb (OptTransition   *trans,
			  gint             frame_num,
			  gpointer         data)
{
  OptSlide             *from, *to;
  OptTransitionPrivate *priv;
  ClutterColor          color = { 0x22, 0x22, 0x22, 0xff };
  ClutterElement       *stage;

  priv = trans->priv;

  from = opt_transition_get_from (trans);
  to   = opt_transition_get_to (trans);
  stage = clutter_stage_get_default();

  clutter_group_show_all (CLUTTER_GROUP(to));

  if (frame_num > 15)
    {
      /* Fix Z ordering */
      clutter_element_lower_bottom (CLUTTER_ELEMENT(from));
    }

  clutter_stage_set_color (CLUTTER_STAGE(stage), &color);

  clutter_element_rotate_y (CLUTTER_ELEMENT(from),
                            - (float)frame_num * 6,
                            CLUTTER_STAGE_WIDTH()/2,
                            0);

  clutter_element_rotate_y (CLUTTER_ELEMENT(to),
                            180 - (frame_num * 6),
                            CLUTTER_STAGE_WIDTH()/2,
                            0);
}

static void
cube_transition_frame_cb (OptTransition   *trans,
			  gint             frame_num,
			  gpointer         data)
{
  OptSlide             *from, *to;
  ClutterElement       *stage;
  ClutterColor          color = { 0x22, 0x22, 0x22, 0xff };
  OptTransitionPrivate *priv;

  priv = trans->priv;

  from  = opt_transition_get_from (trans);
  to    = opt_transition_get_to (trans);
  stage = clutter_stage_get_default();

  clutter_group_show_all (CLUTTER_GROUP(to));

  if (frame_num > 15)
    {
      /* Fix Z ordering */
      clutter_element_lower_bottom (CLUTTER_ELEMENT(from));
    }

  clutter_stage_set_color (CLUTTER_STAGE(stage), &color);

  clutter_element_rotate_y (CLUTTER_ELEMENT(from),
                            - (float)frame_num * 3,
                            CLUTTER_STAGE_WIDTH()/2,
                            -1 * (CLUTTER_STAGE_WIDTH()/2));

  clutter_element_rotate_y (CLUTTER_ELEMENT(to),
                            90 - (frame_num * 3),
                            CLUTTER_STAGE_WIDTH()/2,
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
      clutter_group_show_all (CLUTTER_GROUP(to));
      clutter_element_raise_top (CLUTTER_ELEMENT(to));
    }

  opacity = (frame_num * 255 ) 
                  / clutter_timeline_get_n_frames (CLUTTER_TIMELINE(trans));

  clutter_element_set_opacity (CLUTTER_ELEMENT(to), opacity);

  /* clutter_element_set_depth (CLUTTER_ELEMENT(from), - opacity/10 ); */
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





