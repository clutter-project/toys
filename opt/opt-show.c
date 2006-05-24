#include "opt-show.h"

G_DEFINE_TYPE (OptShow, opt_show, G_TYPE_OBJECT);

typedef struct OptTransition
{
  OptTransitionType type;
  OptSlide         *from, *to;


}
OptTransition;

struct OptShowPrivate
{
  GList           *slides;
  gint             current_slide_num;
  guint            num_slides;
  ClutterTimeline *transition;
  ClutterElement   *bg;
};

static void 
opt_show_dispose (GObject *object)
{
  OptShow *self = OPT_SHOW(object); 

  if (self->priv)
    {

    }

  G_OBJECT_CLASS (opt_show_parent_class)->dispose (object);
}

static void 
opt_show_finalize (GObject *object)
{
  OptShow *self = OPT_SHOW(object); 

  if (self->priv)
    {
      g_free(self->priv);
      self->priv = NULL;
    }

  G_OBJECT_CLASS (opt_show_parent_class)->finalize (object);
}

static void
opt_show_class_init (OptShowClass *klass)
{
  GObjectClass        *object_class;

  object_class = (GObjectClass*) klass;

  /*
  element_class->request_coords  = opt_show_request_coords;
  element_class->allocate_coords = opt_show_allocate_coords;
  */

  /* GObject */
  object_class->finalize     = opt_show_finalize;
  object_class->dispose      = opt_show_dispose;
}

static void
opt_show_init (OptShow *self)
{
  OptShowPrivate *priv;

  priv           = g_new0 (OptShowPrivate, 1);

  self->priv  = priv;

  g_object_set (clutter_stage(),
		"fullscreen", TRUE,
		"hide-cursor", TRUE,
		NULL);

  /* FIXME: should be prop */
  priv->bg = clutter_texture_new_from_pixbuf 
              (gdk_pixbuf_new_from_file ("bg.png", NULL));

  /* FIXME: construct only prop to set bullet style */
}

OptShow*
opt_show_new (void)
{
  return g_object_new (OPT_TYPE_SHOW, NULL);
}


void
opt_show_add_slide (OptShow *self, OptSlide *slide)
{
  ClutterElement *clone;

  self->priv->slides = g_list_append(self->priv->slides, slide);
  self->priv->num_slides++;

  clone = clutter_clone_texture_new(CLUTTER_TEXTURE(self->priv->bg));
  clutter_element_set_size (clone, 
			    CLUTTER_STAGE_WIDTH(), CLUTTER_STAGE_HEIGHT()); 
  clutter_group_add (CLUTTER_GROUP(slide), clone);
  clutter_element_lower_bottom(clone);
  clutter_element_show(clone);
}

void
opt_show_run (OptShow *self)
{
  OptSlide       *slide;
  OptShowPrivate *priv;

  priv = self->priv;
  priv->current_slide_num = 0;

  slide = g_list_nth_data (priv->slides, 0);

  clutter_stage_set_color (CLUTTER_STAGE(clutter_stage()), 0x222222ff);
  clutter_group_add (clutter_stage(), CLUTTER_ELEMENT(slide));
  clutter_group_show_all(clutter_stage());

  clutter_main();
}

void
cube_transition_frame_cb (ClutterTimeline *timeline,
			  gint             frame_num,
			  gpointer         data)
{
  OptShow        *show = (OptShow *)data;
  OptSlide       *from, *to;
  OptShowPrivate *priv;

  priv = show->priv;

  from = g_list_nth_data (priv->slides, priv->current_slide_num);
  to   = g_list_nth_data (priv->slides, priv->current_slide_num+1);

  clutter_group_show_all (CLUTTER_GROUP(to));

  if (frame_num > 15)
    {
      /* Fix Z ordering */
      clutter_element_lower_bottom (CLUTTER_ELEMENT(from));
    }

  clutter_element_rotate_y (CLUTTER_ELEMENT(from),
                            - (float)frame_num * 3,
                            CLUTTER_STAGE_WIDTH()/2,
                            -1 * (CLUTTER_STAGE_WIDTH()/2));

  clutter_element_rotate_y (CLUTTER_ELEMENT(to),
                            90 - (frame_num * 3),
                            CLUTTER_STAGE_WIDTH()/2,
                            -1 * (CLUTTER_STAGE_WIDTH()/2));
}

void
fade_transition_frame_cb (ClutterTimeline *timeline,
			  gint             frame_num,
			  gpointer         data)
{
  OptShow        *show = (OptShow *)data;
  OptSlide       *from, *to;
  OptShowPrivate *priv;

  priv = show->priv;

  from = g_list_nth_data (priv->slides, priv->current_slide_num);
  to   = g_list_nth_data (priv->slides, priv->current_slide_num+1);

  clutter_group_show_all (CLUTTER_GROUP(to));

  clutter_element_set_opacity (CLUTTER_ELEMENT(from), 255 - frame_num);
  clutter_element_set_opacity (CLUTTER_ELEMENT(to), frame_num);
}

void
transition_completed_cb (ClutterTimeline *timeline,
			 gpointer         data)
{
  OptShow        *show = (OptShow *)data;
  OptSlide       *from;
  OptShowPrivate *priv;

  priv = show->priv;

  from = g_list_nth_data (priv->slides, priv->current_slide_num);

  /* Off screen now */
  clutter_group_remove (clutter_stage(), CLUTTER_ELEMENT(from));
  clutter_group_hide_all (CLUTTER_GROUP(from));

  /* advance to next slide */
  priv->current_slide_num++;

  if (priv->current_slide_num > priv->num_slides)
    priv->current_slide_num = 0;

  /* Get rid of the timeline, we've played out now */
  g_object_unref(timeline);
  priv->transition = NULL;
}

void
opt_show_advance (OptShow *self, OptTransitionType transition)
{
  OptSlide       *from, *to;
  OptShowPrivate *priv;

  priv = self->priv;

  /* In a transition */
  if (priv->transition != NULL)
    return;

  from = g_list_nth_data (priv->slides, priv->current_slide_num);
  to   = g_list_nth_data (priv->slides, priv->current_slide_num+1);

  if (to == NULL)
    return;

  clutter_group_add (clutter_stage(), CLUTTER_ELEMENT(to));
  clutter_element_lower_bottom (CLUTTER_ELEMENT(to));

  if (/*priv->current_slide_num % 2 == */ 0)
    {
      priv->transition = clutter_timeline_new (100, 60); /* num frames, fps */

      g_signal_connect (priv->transition, 
			"new-frame",  
			G_CALLBACK (fade_transition_frame_cb), 
			self);
    }
  else
    {
      priv->transition = clutter_timeline_new (30, 60); /* num frames, fps */

      g_signal_connect (priv->transition, 
			"new-frame",  
			G_CALLBACK (cube_transition_frame_cb), 
			self);
    }

  g_signal_connect (priv->transition, 
		    "completed",  
		    G_CALLBACK (transition_completed_cb), 
		    self);

  clutter_timeline_start (priv->transition);
}

