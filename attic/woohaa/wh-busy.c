#include <glib.h>
#include "wh-busy.h"

#define CSW() CLUTTER_STAGE_WIDTH()
#define CSH() CLUTTER_STAGE_HEIGHT()

#define WOOHAA_TYPE_BEHAVIOUR_BUSY (clutter_behaviour_busy_get_type ())

#define WOOHAA_BEHAVIOUR_BUSY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  WOOHAA_TYPE_BEHAVIOUR_BUSY, WoohaaBehaviourBusy))

#define WOOHAA_BEHAVIOUR_BUSY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  WOOHAA_TYPE_BEHAVIOUR_BUSY, WoohaaBehaviourBusyClass))

#define CLUTTER_IS_BEHAVIOUR_BUSY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  WOOHAA_TYPE_BEHAVIOUR_BUSY))

#define CLUTTER_IS_BEHAVIOUR_BUSY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  WOOHAA_TYPE_BEHAVIOUR_BUSY))

#define WOOHAA_BEHAVIOUR_BUSY_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  WOOHAA_TYPE_BEHAVIOUR_BUSY, WoohaaBehaviourBusyClass))

typedef struct _WoohaaBehaviourBusy        WoohaaBehaviourBusy;
typedef struct _WoohaaBehaviourBusyClass   WoohaaBehaviourBusyClass;
 
struct _WoohaaBehaviourBusy
{
  ClutterBehaviour parent;
  WoohaaBusy      *busy;
};

struct _WoohaaBehaviourBusyClass
{
  ClutterBehaviourClass   parent_class;
};

GType clutter_behaviour_busy_get_type (void) G_GNUC_CONST;

G_DEFINE_TYPE (WoohaaBehaviourBusy, clutter_behaviour_busy, CLUTTER_TYPE_BEHAVIOUR);

static ClutterBehaviour*
clutter_behaviour_busy_new (WoohaaBusy *menu, ClutterAlpha *alpha);

static void
clutter_behaviour_alpha_notify (ClutterBehaviour *behave,
                                guint32           alpha_value);

enum
{
  PROP_0,
  PROP_LABEL
};

struct _WoohaaBusyPrivate
{
  ClutterActor     *spinner;
  ClutterActor     *label;
  gchar            *label_text;
  ClutterTimeline  *timeline;
  guint             spinner_alpha;
};

G_DEFINE_TYPE (WoohaaBusy, woohaa_busy, CLUTTER_TYPE_ACTOR);

static void
woohaa_busy_dispose (GObject *object)
{
  WoohaaBusy        *self;
  WoohaaBusyPrivate *priv; 

  self = WOOHAA_BUSY(object); 
  priv = self->priv;

  G_OBJECT_CLASS (woohaa_busy_parent_class)->dispose (object);
}

static void
woohaa_busy_finalize (GObject *object)
{
  WoohaaBusy        *self;
  WoohaaBusyPrivate *priv; 

  self = WOOHAA_BUSY(object); 
  priv = self->priv;

  G_OBJECT_CLASS (woohaa_busy_parent_class)->finalize (object);
}

static void
woohaa_busy_show_cb (ClutterActor *actor, ClutterTimeline *timeline)
{
  clutter_timeline_start (timeline);
}

static void
woohaa_busy_hide_cb (ClutterActor *actor, ClutterTimeline *timeline)
{
  clutter_timeline_stop (timeline);
  clutter_actor_set_opacity (CLUTTER_ACTOR (actor), 0xFF);
}

static void
newframe_cb (ClutterTimeline *timeline, 
	     gint             frame_num, 
	     ClutterActor    *spinner)
{
  gint x, y;

  clutter_actor_get_position (spinner, &x, &y);
  clutter_actor_set_position (spinner, x + 10 , y + 10);

  clutter_actor_set_rotation (spinner, 
			      CLUTTER_Z_AXIS,
                              (float)frame_num * 4.0,
                              clutter_actor_get_width (spinner) / 2,
                              clutter_actor_get_height (spinner) / 2,
                              0);
}

static void
woohaa_busy_paint (ClutterActor *actor)
{
  WoohaaBusyPrivate *priv = (WOOHAA_BUSY (actor))->priv;

  clutter_actor_paint (priv->spinner);
  clutter_actor_paint (priv->label);
}

static void
woohaa_busy_get_preferred_width  (ClutterActor          *actor,
				  ClutterUnit            for_height,
				  ClutterUnit           *min_width_p,
				  ClutterUnit           *natural_width_p)
{
  *min_width_p = CLUTTER_UNITS_FROM_INT (100);
  *natural_width_p = CLUTTER_UNITS_FROM_INT (500);
}

static void
woohaa_busy_get_preferred_height (ClutterActor          *actor,
				  ClutterUnit            for_width,
				  ClutterUnit           *min_height_p,
				  ClutterUnit           *natural_height_p)
{
  *min_height_p = CLUTTER_UNITS_FROM_INT (100);
  *natural_height_p = CLUTTER_UNITS_FROM_INT (500);
}

static void
woohaa_busy_allocate (ClutterActor *actor, 
		      const ClutterActorBox *box,
		      gboolean absolute_origin_changed)
{
  WoohaaBusyPrivate *priv = (WOOHAA_BUSY (actor))->priv;
  ClutterUnit min_width, spinner_natural_width, label_natural_width;
  ClutterUnit min_height, spinner_natural_height, label_natural_height;
  ClutterActorBox spinner_box, label_box;
  
  /* query spinner width and height */
  clutter_actor_get_preferred_width (priv->spinner, 
				     box->y2 - box->y1,
				     &min_width,
				     &spinner_natural_width);
  clutter_actor_get_preferred_height (priv->spinner, 
				      box->x2 - box->x1,
				      &min_height,
				      &spinner_natural_height);

  /* query label width and height */
  clutter_actor_get_preferred_width (priv->label, 
				     box->y2 - box->y1,
				     &min_width,
				     &label_natural_width);
  clutter_actor_get_preferred_height (priv->label, 
				      box->x2 - box->x1,
				      &min_height,
				      &label_natural_height);

  spinner_box.x1 = CLUTTER_UNITS_FROM_INT (CSW()/2) - spinner_natural_width/2;
  spinner_box.y1 = CLUTTER_UNITS_FROM_INT (CSH()/2) 
    - spinner_natural_height/2 
    - label_natural_height/2  
    + (priv->spinner_alpha * 100);
  spinner_box.x2 = spinner_box.x1 + spinner_natural_width;
  spinner_box.y2 = spinner_box.y1 + spinner_natural_height;

  label_box.x1 = CLUTTER_UNITS_FROM_INT (CSW()/2) - label_natural_width/2;
  label_box.y1 = CLUTTER_UNITS_FROM_INT (CSH()/2) 
    - spinner_natural_height/2 
    - label_natural_height/2  
    + spinner_natural_height;
  label_box.x2 = label_box.x1 + label_natural_width;
  label_box.y2 = label_box.y1 + label_natural_height;

  clutter_actor_allocate (priv->spinner, 
			  &spinner_box, 
			  absolute_origin_changed);

  clutter_actor_allocate (priv->label, 
			  &label_box, 
			  absolute_origin_changed);

  CLUTTER_ACTOR_CLASS (woohaa_busy_parent_class)->
  	  allocate (actor, box, absolute_origin_changed);
}

static void
woohaa_busy_set_property (GObject      *object, 
			  guint         prop_id,
			  const GValue *value, 
			  GParamSpec   *pspec)
{
  WoohaaBusy        *busy;
  WoohaaBusyPrivate *priv;

  busy = WOOHAA_BUSY(object);
  priv = busy->priv;

  switch (prop_id) 
    {
    case PROP_LABEL:
      if (priv->label_text)
	g_free (priv->label_text);
      priv->label_text = g_strdup(g_value_get_string (value));
      clutter_label_set_text (CLUTTER_LABEL (priv->label), priv->label_text);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
woohaa_busy_get_property (GObject    *object, 
			  guint       prop_id,
			  GValue     *value, 
			  GParamSpec *pspec)
{
  WoohaaBusy        *busy;
  WoohaaBusyPrivate *priv;

  busy = WOOHAA_BUSY(object);
  priv = busy->priv;

  switch (prop_id) 
    {
    case PROP_LABEL:
      g_value_set_string (value, priv->label_text);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    } 
}

static void
woohaa_busy_class_init (WoohaaBusyClass *klass)
{
  GObjectClass *object_class     = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  
  g_type_class_add_private (klass, sizeof (WoohaaBusyPrivate));

  object_class->dispose      = woohaa_busy_dispose;
  object_class->finalize     = woohaa_busy_finalize;
  object_class->set_property = woohaa_busy_set_property;
  object_class->get_property = woohaa_busy_get_property;

  actor_class->get_preferred_width = woohaa_busy_get_preferred_width;
  actor_class->get_preferred_height = woohaa_busy_get_preferred_height;
  actor_class->allocate = woohaa_busy_allocate;
  actor_class->paint    = woohaa_busy_paint;

  g_object_class_install_property 
    (object_class, PROP_LABEL,
     g_param_spec_string ("label",
			  "Label Text",
			  "Label Text",
			  NULL,
			  G_PARAM_READWRITE));

}

static void
woohaa_busy_init (WoohaaBusy *woohaa_busy)
{
  WoohaaBusyPrivate   *priv;
  gchar               *font;
  ClutterColor grey = { 0x72, 0x9f, 0xcf, 0xff};

  woohaa_busy->priv = priv =
    G_TYPE_INSTANCE_GET_PRIVATE (woohaa_busy,
                                 WOOHAA_TYPE_BUSY,
                                 WoohaaBusyPrivate);

  priv->timeline = clutter_timeline_new (90, 20);
  clutter_timeline_set_loop (priv->timeline, TRUE);

  priv->spinner = clutter_texture_new_from_file (PKGDATADIR "/spinner.svg", 
						 NULL);
  clutter_actor_set_position 
    (priv->spinner, 
     (CSW() - clutter_actor_get_width (priv->spinner))/2,
     (CSH() - clutter_actor_get_height (priv->spinner))/2);
  clutter_actor_show (priv->spinner);

  priv->label_text = g_strdup("One moment please...");
  font = g_strdup_printf("Sans %ipx", (CSH()/6)/2);
  priv->label = clutter_label_new_full (font, priv->label_text,  &grey);
  clutter_actor_set_position (priv->label,
			      (CSW() - clutter_actor_get_width (priv->label))/2,
			      CSH() - (3*clutter_actor_get_height (priv->label)));  
  clutter_actor_show (priv->label);

  clutter_actor_set_parent (priv->spinner, CLUTTER_ACTOR (woohaa_busy));
  clutter_actor_set_parent (priv->label, CLUTTER_ACTOR (woohaa_busy));

  g_signal_connect (priv->timeline, 
		    "new-frame", 
		    G_CALLBACK (newframe_cb), 
		    priv->spinner);

  g_signal_connect (woohaa_busy,
		    "show",
		    G_CALLBACK (woohaa_busy_show_cb),
		    priv->timeline);
  g_signal_connect (woohaa_busy,
		    "hide",
		    G_CALLBACK (woohaa_busy_hide_cb),
		    priv->timeline);
}

void 
fade_complete_cb (ClutterActor *actor, gpointer data)
{
  clutter_actor_hide(actor);
}

void
woohaa_busy_fade_out (WoohaaBusy *busy, gint timeout)
{
  ClutterEffectTemplate *template;
  ClutterTimeline *timeline;

  timeline = clutter_timeline_new_for_duration (timeout);
  template = clutter_effect_template_new (timeline, CLUTTER_ALPHA_SINE_INC);

  clutter_effect_fade (template,
		       CLUTTER_ACTOR (busy),
		       0,
		       fade_complete_cb,
		       NULL);
}

void
woohaa_busy_fade_in (WoohaaBusy *busy, gint timeout)
{
  ClutterEffectTemplate *template;
  ClutterTimeline *timeline;

  timeline = clutter_timeline_new_for_duration (timeout);
  template = clutter_effect_template_new (timeline, CLUTTER_ALPHA_SINE_INC);

  clutter_actor_set_opacity (CLUTTER_ACTOR (busy), 0);
  clutter_actor_show (CLUTTER_ACTOR (busy));

  clutter_effect_fade (template,
		       CLUTTER_ACTOR (busy),
		       0xFF,
		       NULL,
		       NULL);
}

void
woohaa_busy_bounce (WoohaaBusy *busy)
{
  WoohaaBusyPrivate *priv;
  ClutterTimeline *timeline;
  ClutterAlpha *alpha;
  ClutterGeometry geo;
  ClutterBehaviour *behave;
  priv = busy->priv;

  timeline = clutter_timeline_new_for_duration (500);
  alpha = clutter_alpha_new_full (timeline, CLUTTER_ALPHA_SINE, NULL, NULL);

  behave = clutter_behaviour_busy_new (busy, alpha);

  clutter_timeline_start (timeline);

  clutter_actor_get_geometry (CLUTTER_ACTOR (busy), &geo);
}

ClutterActor*
woohaa_busy_new (void)
{
  return g_object_new (WOOHAA_TYPE_BUSY, NULL);
}

/* Custom Behavior */

static void
clutter_behaviour_alpha_notify (ClutterBehaviour *behave,
                                guint32           alpha_value)
{
  WoohaaBusy *busy;

  busy = (WOOHAA_BEHAVIOUR_BUSY (behave))->busy;
  
  busy->priv->spinner_alpha = alpha_value;
  clutter_actor_queue_relayout (CLUTTER_ACTOR (busy));
}

static void
clutter_behaviour_busy_class_init (WoohaaBehaviourBusyClass *klass)
{
  ClutterBehaviourClass *behave_class = CLUTTER_BEHAVIOUR_CLASS (klass);

  behave_class->alpha_notify = clutter_behaviour_alpha_notify;
}

static void
clutter_behaviour_busy_init (WoohaaBehaviourBusy *self)
{
}

static ClutterBehaviour*
clutter_behaviour_busy_new (WoohaaBusy *busy, ClutterAlpha *alpha)
{
  WoohaaBehaviourBusy *busy_behave;

  busy_behave = g_object_new (WOOHAA_TYPE_BEHAVIOUR_BUSY, 
			      "alpha", alpha, NULL);
  busy_behave->busy  = busy;

  return CLUTTER_BEHAVIOUR(busy_behave);
}
