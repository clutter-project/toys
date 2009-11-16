#include "wh-video-view.h"
#include "wh-video-model.h"
#include "util.h"

#include <cogl/cogl.h>

G_DEFINE_TYPE (WHVideoView, wh_video_view, CLUTTER_TYPE_ACTOR);

#define WH_VIDEO_VIEW_GET_PRIVATE(obj)                \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj),              \
                                  WH_TYPE_VIDEO_VIEW, \
                                  WHVideoViewPrivate))

struct _WHVideoViewPrivate
{
  WHVideoModel      *model;
  gint               n_rows_visible;
  gint               active_item_num;
  gint               n_rows;

  ClutterActor      *rows;

  ClutterActor      *selection_indicator;
  ClutterActor      *selection;
  ClutterActor      *up_arrow; 
  ClutterActor      *down_arrow;

  gboolean           animation_running;
};

enum
{
  PROP_0,
  PROP_MODEL,
  PROP_N_ROWS
};

void wh_video_view_activate (WHVideoView *view, gint entry_num);

static gboolean 
populate_rows (WHVideoModel    *model,
	       WHVideoModelRow *row,
	       gpointer         data)
{
  WHVideoView          *view;
  WHVideoViewPrivate    *priv;
  WHVideoRowRenderer    *renderer;
  gint                   r_width, r_height, position;
  ClutterEffectTemplate *template;

  view = WH_VIDEO_VIEW (data);
  priv = WH_VIDEO_VIEW_GET_PRIVATE (view);

  template = clutter_effect_template_new_for_duration (500, 
						       CLUTTER_ALPHA_SINE_INC);

  r_height = clutter_actor_get_height (CLUTTER_ACTOR (view)) /
    priv->n_rows_visible;
  r_width = clutter_actor_get_width (CLUTTER_ACTOR (view));

  if (priv->n_rows == 0)
    {
      ClutterUnit width, height;
      double scale;

      /* 
       * Scale the up and down indication arrows 
       */
      clutter_actor_get_preferred_size (priv->up_arrow,
					NULL,
					NULL,
					&width,
					&height);	
      scale = (double)CLUTTER_UNITS_FROM_INT (r_height/4) / (double) height;
      height = CLUTTER_UNITS_FROM_INT (r_height/4);
      width = width * scale;
      clutter_actor_set_sizeu (priv->up_arrow, width, height);
      clutter_actor_set_sizeu (priv->down_arrow, width, height);

      clutter_actor_set_size (priv->selection, r_width, r_height);

      width = CLUTTER_UNITS_FROM_INT (r_width - r_width/100) - width;
      clutter_actor_set_positionu (priv->up_arrow, width, 
				   CLUTTER_UNITS_FROM_INT (r_height/10));

      height = CLUTTER_UNITS_FROM_INT (r_height - r_height/10) - height;
      clutter_actor_set_positionu (priv->down_arrow, width, height);

      clutter_actor_set_opacity (priv->selection_indicator, 0);
    }

  position = priv->n_rows++ + priv->active_item_num;

  renderer = wh_video_model_row_get_renderer (row);

  clutter_actor_set_size (CLUTTER_ACTOR (renderer), 
			  clutter_actor_get_width (CLUTTER_ACTOR (view)), 
			  r_height);
  clutter_actor_show (CLUTTER_ACTOR (renderer));

  clutter_effect_move (template,
		       CLUTTER_ACTOR (renderer), 
		       0, 
		       position * r_height,
		       NULL,
		       NULL);

  clutter_effect_fade (template,
		       CLUTTER_ACTOR (renderer),
		       position >= priv->n_rows_visible ? 0x00 : 0xff,
		       NULL,
		       NULL);

  if (priv->n_rows == 1)
    wh_video_row_renderer_set_active (renderer, TRUE);
  else
    wh_video_row_renderer_set_active (renderer, FALSE);

  clutter_group_add (priv->rows, CLUTTER_ACTOR (renderer));

  return TRUE;
}

static void
on_model_rows_change (WHVideoModel *model, gpointer *userdata)
{
  WHVideoView        *view;
  WHVideoViewPrivate *priv;

  view = WH_VIDEO_VIEW(userdata);
  priv = WH_VIDEO_VIEW_GET_PRIVATE(view);

  clutter_actor_set_opacity (priv->selection_indicator, 0);
  clutter_group_remove_all (CLUTTER_GROUP (priv->rows));
  priv->n_rows = 0;
  priv->active_item_num = 0;

  wh_video_model_foreach (model, 
			  populate_rows,
			  view);

  wh_video_view_activate(view, 0);
}

static void
wh_video_view_set_property (GObject      *object, 
			    guint         prop_id,
			    const GValue *value, 
			    GParamSpec   *pspec)
{
  WHVideoView        *view;
  WHVideoViewPrivate *priv;

  view = WH_VIDEO_VIEW(object);
  priv = WH_VIDEO_VIEW_GET_PRIVATE(view);

  switch (prop_id) 
    {
    case PROP_MODEL:
      if (priv->model)
        {
	  clutter_group_remove_all (CLUTTER_GROUP (priv->rows));
	  g_object_unref (priv->model);
	}
      priv->model = g_value_get_object (value);

      wh_video_model_foreach (priv->model, 
			      populate_rows,
			      priv->rows);

      g_signal_connect(priv->model, 
		       "rows-reordered",
		       G_CALLBACK(on_model_rows_change), 
		       object);
      break;
    case PROP_N_ROWS:
      priv->n_rows_visible = g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
wh_video_view_get_property (GObject    *object, 
			    guint       prop_id,
			    GValue     *value, 
			    GParamSpec *pspec)
{
  WHVideoView        *view;
  WHVideoViewPrivate *priv;

  view = WH_VIDEO_VIEW(object);
  priv = WH_VIDEO_VIEW_GET_PRIVATE(view);

  switch (prop_id) 
    {
    case PROP_MODEL:
      g_value_set_object (value, priv->model);
      break;
    case PROP_N_ROWS:
      g_value_set_int (value, priv->n_rows_visible);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    } 
}

static void
wh_video_view_get_preferred_width  (ClutterActor          *actor,
				    ClutterUnit            for_height,
				    ClutterUnit           *min_width_p,
				    ClutterUnit           *natural_width_p)
{
  WHVideoView         *self = WH_VIDEO_VIEW(actor);
  WHVideoViewPrivate  *priv;
  
  priv = self->priv;
  
  clutter_actor_get_preferred_height (priv->rows, 
				      for_height,
				      min_width_p,
				      natural_width_p);
}

static void
wh_video_view_get_preferred_height (ClutterActor          *actor,
				    ClutterUnit            for_width,
				    ClutterUnit           *min_height_p,
				    ClutterUnit           *natural_height_p)
{
  WHVideoView         *self = WH_VIDEO_VIEW(actor);
  WHVideoViewPrivate  *priv;  
  
  priv = self->priv;

  clutter_actor_get_preferred_height (priv->rows, 
				      for_width,
				      min_height_p,
				      natural_height_p);
}

static void
wh_video_view_allocate (ClutterActor    *actor,
			const ClutterActorBox *box,
			gboolean absolute_origin_changed)
{
  WHVideoView         *self = WH_VIDEO_VIEW(actor);
  WHVideoViewPrivate  *priv;  
  ClutterActorBox      child_box;
  ClutterUnit          width, height;

  priv = self->priv;

  CLUTTER_ACTOR_CLASS (wh_video_view_parent_class)->
	  allocate (actor, box, absolute_origin_changed);

  clutter_actor_get_preferred_size (priv->rows,
				    NULL,
				    NULL,
				    &width,
				    &height);
  
  child_box.x1 = 0;
  child_box.y1 = 0;
  child_box.x2 = box->x1 + width;
  child_box.y2 = box->y1 + height;
  clutter_actor_allocate (priv->rows, 
			  &child_box,
			  absolute_origin_changed);

  clutter_actor_get_preferred_size (priv->selection_indicator,
				    NULL,
				    NULL,
				    &width,
				    &height);
  
  child_box.x1 = 0;
  child_box.y1 = 0;
  child_box.x2 = box->x1 + width;
  child_box.y2 = box->y1 + height;
  clutter_actor_allocate (priv->selection_indicator, 
			  &child_box,
			  absolute_origin_changed);
}

static void
wh_video_view_paint (ClutterActor *actor)
{
  WHVideoView         *self = WH_VIDEO_VIEW(actor);
  WHVideoViewPrivate  *priv = self->priv;  

  priv = self->priv;

  clutter_actor_paint (priv->selection_indicator);
  clutter_actor_paint (priv->rows);
}

static void 
wh_video_view_dispose (GObject *object)
{
  WHVideoView         *self = WH_VIDEO_VIEW(object);
  WHVideoViewPrivate  *priv;

  priv = self->priv;
  
  G_OBJECT_CLASS (wh_video_view_parent_class)->dispose (object);
}

static void 
wh_video_view_finalize (GObject *object)
{
  G_OBJECT_CLASS (wh_video_view_parent_class)->finalize (object);
}

static void
wh_video_view_class_init (WHVideoViewClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass   *actor_class = CLUTTER_ACTOR_CLASS (klass);
  ClutterActorClass   *parent_class; 

  parent_class = CLUTTER_ACTOR_CLASS (wh_video_view_parent_class);

  actor_class->paint           = wh_video_view_paint;
  actor_class->allocate        = wh_video_view_allocate;
  actor_class->get_preferred_width  = wh_video_view_get_preferred_width;
  actor_class->get_preferred_height = wh_video_view_get_preferred_height;

  gobject_class->finalize     = wh_video_view_finalize;
  gobject_class->dispose      = wh_video_view_dispose;
  gobject_class->set_property = wh_video_view_set_property;
  gobject_class->get_property = wh_video_view_get_property;

  g_type_class_add_private (gobject_class, sizeof (WHVideoViewPrivate));

  g_object_class_install_property 
    (gobject_class,
     PROP_MODEL,
     g_param_spec_object ("model",
			  "Model",
			  "Underlying video model",
			  WH_TYPE_VIDEO_MODEL,
			  G_PARAM_CONSTRUCT|G_PARAM_READWRITE));

  g_object_class_install_property 
    (gobject_class,
     PROP_N_ROWS,
     g_param_spec_int ("n-rows-visible",
		       "n-rows-visible",
		       "Number of row to display",
		       0, G_MAXINT,
		       0,
		       G_PARAM_READWRITE));
}

static void
row_move_complete (ClutterActor *actor, gpointer data)
{
  WHVideoView        *view = WH_VIDEO_VIEW (data);   
  WHVideoViewPrivate *priv = WH_VIDEO_VIEW_GET_PRIVATE(view);

  priv->animation_running = FALSE;
}

void
wh_video_view_activate (WHVideoView  *view,
			gint          entry_num)
{
  WHVideoViewPrivate    *priv = WH_VIDEO_VIEW_GET_PRIVATE(view);
  ClutterActor          *child;
  gint                   i, r_height, position;
  ClutterEffectTemplate *template;
  guint8                 opacity;

  priv->animation_running = TRUE;

  r_height = clutter_actor_get_height (CLUTTER_ACTOR (view)) /
    priv->n_rows_visible;

  template = clutter_effect_template_new_for_duration (250, 
						       CLUTTER_ALPHA_SINE_INC);

  for ( i = 0; i < priv->n_rows + 1; i++ )
    {
      child = clutter_group_get_nth_child (CLUTTER_GROUP (priv->rows), i);
      if (!child)
	return;

      position = i - priv->active_item_num;

      if (position < -1 || position >= priv->n_rows_visible)
	opacity = 0;
      else
	opacity = 0xff;
	
      clutter_effect_move (template,
			   child, 
			   0, 
			   position * r_height,
			   row_move_complete,
			   view);
      clutter_effect_fade (template,
			   child,
			   opacity,
			   NULL,
			   NULL);

      if (priv->active_item_num == i)
        {
	  wh_video_row_renderer_set_active (WH_VIDEO_ROW_RENDERER (child), 
					    TRUE);

	  if (i > 0)
	    {
	      if (clutter_actor_get_opacity (priv->up_arrow) != 0xff)
		clutter_effect_fade (template, 
				     priv->up_arrow, 
				     0xff, 
				     NULL, 
				     NULL);
	    }
	  else if (clutter_actor_get_opacity (priv->up_arrow) != 0)
	    clutter_effect_fade (template, priv->up_arrow, 0, NULL, NULL);

	  if (i < priv->n_rows - 1)
	    {
	      if (clutter_actor_get_opacity (priv->down_arrow) != 0xff)
		clutter_effect_fade (template, 
				     priv->down_arrow, 
				     0xff, 
				     NULL, 
				     NULL);
	    }
	  else if (clutter_actor_get_opacity (priv->down_arrow) != 0)
	    clutter_effect_fade (template, priv->down_arrow, 0, NULL, NULL);

	  clutter_actor_set_opacity (priv->selection_indicator, 0xff);
	}
      else
	wh_video_row_renderer_set_active (WH_VIDEO_ROW_RENDERER (child), 
					  FALSE);
    }
}

void
wh_video_view_advance (WHVideoView *view, gint n)
{
  WHVideoViewPrivate *priv = WH_VIDEO_VIEW_GET_PRIVATE(view);
  gint new_index;

  if (priv->animation_running)
    return;

  new_index = priv->active_item_num + n;
  
  if (new_index > priv->n_rows - 1)
    new_index = priv->n_rows - 1;
  else if (new_index < 0)
    new_index = 0;

  if (new_index == priv->active_item_num)
    return;

  priv->active_item_num = new_index;

  wh_video_view_activate (view, priv->active_item_num);
}

WHVideoModelRow*
wh_video_view_get_selected (WHVideoView *view)
{
  WHVideoViewPrivate *priv = WH_VIDEO_VIEW_GET_PRIVATE(view);

  return wh_video_model_get_row (priv->model, priv->active_item_num);
}

void
wh_video_view_enable_animation (WHVideoView *view, gboolean active)
{
}

static void
wh_video_view_init (WHVideoView *self)
{
  WHVideoViewPrivate *priv;

  self->priv = priv = WH_VIDEO_VIEW_GET_PRIVATE (self);

  priv->rows = clutter_group_new ();

  priv->selection_indicator = clutter_group_new ();
  clutter_actor_set_opacity (priv->selection_indicator, 0);
  clutter_actor_set_parent (priv->rows, CLUTTER_ACTOR (self));

  /* Load the position backgroud image */
  priv->selection = clutter_texture_new_from_file (PKGDATADIR "/selected.svg",
						   NULL);
  if (!priv->selection)
    g_warning ("Unable to load %s\n",  PKGDATADIR "/selected.svg");

  clutter_actor_show (priv->selection);
  clutter_group_add (CLUTTER_GROUP (priv->selection_indicator), 
		     priv->selection);

  /* Load the up arrow image */
  priv->up_arrow = clutter_texture_new_from_file (PKGDATADIR "/arrow-up.svg",
						  NULL);
  if (!priv->up_arrow)
    g_warning ("Unable to load %s\n",  PKGDATADIR "/arrow-up.svg");

  clutter_actor_show (priv->up_arrow);
  clutter_group_add (CLUTTER_GROUP (priv->selection_indicator), 
		     priv->up_arrow);
  
  /* Load the down arrow image */
  priv->down_arrow = clutter_texture_new_from_file (PKGDATADIR "/arrow-down.svg",
						    NULL);
  if (!priv->down_arrow)
    g_warning ("Unable to load %s\n",  PKGDATADIR "/arrow-down.svg");

  clutter_actor_show (priv->down_arrow);
  clutter_group_add (CLUTTER_GROUP (priv->selection_indicator), 
		     priv->down_arrow);

}

ClutterActor*
wh_video_view_new (WHVideoModel      *model, 
		   gint               n_rows_visible)
{
  ClutterActor         *view;

  view = g_object_new (WH_TYPE_VIDEO_VIEW, 
		       "n-rows-visible", n_rows_visible,
		       "model", model,
		       NULL);

  return view;
}

