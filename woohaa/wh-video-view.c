#include "wh-video-view.h"
#include "wh-video-model.h"
#include "util.h"

G_DEFINE_TYPE (WHVideoView, wh_video_view, CLUTTER_TYPE_ACTOR);

#define WH_VIDEO_VIEW_GET_PRIVATE(obj)                \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj),              \
                                  WH_TYPE_VIDEO_VIEW, \
                                  WHVideoViewPrivate))

struct _WHVideoViewPrivate
{
  WHVideoModel      *model;
  ClutterActor      *rows; 
  ClutterActor      *selector; 
  gint               row_height;
  gint               n_rows_visible;
  gboolean           enable_model_change_anim;
  
  /* For vertical scroll */
  ClutterTimeline   *timeline;
  ClutterAlpha      *alpha;
  ClutterBehaviour  *behave_up;
  ClutterBehaviour  *behave_down;
  ClutterKnot        path_down[3], path_up[3];

  /* For model changed animation */
  ClutterTimeline   *timeline_switch;
  ClutterAlpha      *alpha_switch;
  ClutterBehaviour  *behave_switch_out;
  ClutterBehaviour  *behave_switch_in;
  gulong             switch_signal;

  /* Selection tracking */
  gint               active_item_num;
  gint               pending_item_num;
  gint               current_offset;
  gint               n_items; 	

  /* up/down 'buttons */
  ClutterActor *up;
  ClutterActor *down;
};

enum
{
  PROP_0,
  PROP_MODEL,
  PROP_N_ROWS
};

static void
switch_timeline_completed_1 (ClutterTimeline *timeline,
			     gpointer         user_data);

static void
wh_video_view_add_rows (WHVideoView *view);

static void
wh_video_view_remove_rows (WHVideoView *view);

static void
ensure_layout (WHVideoView *view,
	       gint         width,
	       gint         height,
	       gint         n_rows);

static void
on_model_rows_change (WHVideoModel *model, gpointer *userdata);

static void
on_model_row_added (WHVideoModel    *model, 
		    WHVideoModelRow *row,
		    gpointer         *userdata);

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
	g_object_unref (priv->model);
      priv->model = g_value_get_object (value);
      wh_video_view_remove_rows (view);
      if (priv->model)
	{
	  g_object_ref (priv->model);
	  wh_video_view_add_rows (view);

	  g_signal_connect(priv->model, 
			   "rows-reordered",
			   G_CALLBACK(on_model_rows_change), 
			   view);

	  g_signal_connect(priv->model, 
			   "filter-changed",
			   G_CALLBACK(on_model_rows_change), 
			   view);

	  /* FIXME: borked
	  g_signal_connect(priv->model, 
			   "row-added",
			   G_CALLBACK(on_model_row_added), 
			   view);
	  */
	}
      break;
    case PROP_N_ROWS:
      ensure_layout (view,
		     clutter_actor_get_width(CLUTTER_ACTOR(view)),
		     clutter_actor_get_height(CLUTTER_ACTOR(view)),
		     g_value_get_int (value));
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
ensure_layout (WHVideoView *view,
	       gint         width,
	       gint         height,
	       gint         n_rows)
{
  WHVideoViewPrivate *priv;
  WHVideoModelRow    *row;
  gint                i, offset = 0;
  gboolean            size_change, n_rows_change;

  priv = WH_VIDEO_VIEW_GET_PRIVATE(view);

  n_rows_change = (priv->n_rows_visible != n_rows);
  size_change = (clutter_actor_get_width(CLUTTER_ACTOR(view)) != width 
		 || clutter_actor_get_width(CLUTTER_ACTOR(view)) != height);

  priv->n_rows_visible = n_rows;

  if (width == 0 || height == 0)
    return;

  priv->row_height = height / n_rows;

  for (i = 0; i < clutter_group_get_n_children(CLUTTER_GROUP(priv->rows)); i++)
    {
      ClutterActor *renderer;

      renderer = clutter_group_get_nth_child (CLUTTER_GROUP(priv->rows), i);

      clutter_actor_set_position (renderer, 0, offset); 
      clutter_actor_set_size (renderer, 
			      width - clutter_actor_get_height (priv->up)- 10, 
			      priv->row_height); 
      clutter_actor_show_all (renderer);  

      offset += priv->row_height;
    }

  row = wh_video_model_get_row (priv->model, priv->active_item_num);
  if (row)
    wh_video_row_renderer_set_active (wh_video_model_row_get_renderer(row), 
				      TRUE); 

  clutter_actor_show_all (priv->rows);

  if (size_change)
    {
      /* Selector and buttons - */
      clutter_actor_set_size (priv->selector, width, priv->row_height);
      clutter_actor_set_position (priv->selector, 0, 0);  

      clutter_actor_set_position 
	(priv->up, 
	 width - clutter_actor_get_width (priv->up) - 10, 
	 0);  

      clutter_actor_set_position 
	(priv->down, 
	 width - clutter_actor_get_width (priv->down) - 10, 
	 priv->row_height - clutter_actor_get_height (priv->down));  

      /* Set up new scroll path */
#if 0
      priv->path_up[0].x = 0;   priv->path_up[0].y = 0; 
      priv->path_up[1].x = 0;   
      priv->path_up[1].y = -1 * (priv->row_height + (priv->row_height/4)); 
      priv->path_up[2].x = 0;   priv->path_up[2].y = -1 * priv->row_height; 
      
      priv->path_down[0].x = 0;   priv->path_down[0].y = 0; 
      priv->path_down[1].x = 0;   
      priv->path_down[1].y = (priv->row_height + (priv->row_height/4)); 
      priv->path_down[2].x = 0;   priv->path_down[2].y = priv->row_height; 
#endif
      priv->path_up[0].x = 0;   priv->path_up[0].y = 0; 
      priv->path_up[1].x = 0;   priv->path_up[1].y = -1 * priv->row_height; 
      
      priv->path_down[0].x = 0;   priv->path_down[0].y = 0; 
      priv->path_down[1].x = 0;   priv->path_down[1].y = priv->row_height; 
      
      /* FIXME: unapply to any actors..? */
      if (priv->behave_up) g_object_unref (priv->behave_up);
      priv->behave_up = clutter_behaviour_path_new (priv->alpha, 
						    priv->path_up, 2);
      
      if (priv->behave_down) g_object_unref (priv->behave_down);
      priv->behave_down = clutter_behaviour_path_new (priv->alpha,  
						      priv->path_down, 2);
    }
}

static void
wh_video_view_request_coords (ClutterActor    *actor,
			      ClutterActorBox *box)
{
  WHVideoView        *view;
  WHVideoViewPrivate *priv;

  view = WH_VIDEO_VIEW(actor);
  priv = WH_VIDEO_VIEW_GET_PRIVATE(view);

  if (((box->x2 - box->x1) != clutter_actor_get_width(actor))
      || (box->y2 - box->y1) != clutter_actor_get_height(actor))
    {
      gint w,h;

      w = (box->x2 - box->x1);
      h = (box->y2 - box->y1);

      ensure_layout (view, w, h, priv->n_rows_visible);

      clutter_actor_set_clip (actor, 0, 0, w, h);
    }
}

static void
wh_video_view_paint (ClutterActor *actor)
{
  WHVideoView        *view;
  WHVideoViewPrivate *priv;
  int                 i, start_item, end_item;
  
  view = WH_VIDEO_VIEW(actor);
  priv = WH_VIDEO_VIEW_GET_PRIVATE(view);

  glPushMatrix();

  if (priv->selector && priv->n_items)
    {
      clutter_actor_paint (priv->selector);

      if (priv->active_item_num > 0)
	clutter_actor_paint (priv->up);
      if (priv->active_item_num < priv->n_items-1)
	clutter_actor_paint (priv->down);
    }
  else 
    {
      glPopMatrix();
      return;
    }

  /* Custom paint code to speed things up avoiding attempting to 
   * offscreen entrys.
  */

  glTranslatef(0.0, 
	       (float)-1.0 * (view->priv->active_item_num) 
	                         * priv->row_height, 
	       0.0);

  start_item = priv->active_item_num - 1;
  if (start_item < 0) start_item = 0;

  end_item = priv->active_item_num + 6;

  glTranslatef(0.0, clutter_actor_get_y (priv->rows), 0.0);

  for (i = start_item; i < end_item; i++)
    {
      ClutterActor* child;

      child = clutter_group_get_nth_child (CLUTTER_GROUP(priv->rows), i);
      if (child)
	clutter_actor_paint (child);
    }

  glPopMatrix();
}

static void
wh_video_view_realize (ClutterActor *self)
{
  if (clutter_actor_get_width(self) == 0 
      || clutter_actor_get_height(self) == 0)
    {
      g_warning ("Unable to realize view, no size set..");
      CLUTTER_ACTOR_UNSET_FLAGS (self, CLUTTER_ACTOR_REALIZED);
      return;
    }
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
  actor_class->realize         = wh_video_view_realize;
  actor_class->request_coords  = wh_video_view_request_coords;

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


void
wh_video_view_activate (WHVideoView  *view,
			gint          entry_num)
{
  WHVideoViewPrivate *priv;
  WHVideoModelRow    *row;

  priv = WH_VIDEO_VIEW_GET_PRIVATE(view);

  if (clutter_timeline_is_playing(view->priv->timeline))
    return;

  if (entry_num < 0  || entry_num >= wh_video_model_row_count (priv->model))
      return;

  row = wh_video_model_get_row (priv->model, priv->active_item_num);
  wh_video_row_renderer_set_active (wh_video_model_row_get_renderer(row), 
				    FALSE); 

  priv->pending_item_num = entry_num;

  if (priv->active_item_num > priv->pending_item_num)
      clutter_behaviour_apply (priv->behave_down, priv->rows);
  else
    clutter_behaviour_apply (priv->behave_up, priv->rows);

  clutter_timeline_start (priv->timeline);
}

void
wh_video_view_advance (WHVideoView *view, gint n)
{
  WHVideoViewPrivate *priv = WH_VIDEO_VIEW_GET_PRIVATE(view);

  wh_video_view_activate (view, priv->active_item_num + n);
}

static void
scroll_timeline_completed (ClutterTimeline *timeline,
			   gpointer         user_data)
{
  WHVideoView        *view = WH_VIDEO_VIEW(user_data);
  WHVideoViewPrivate *priv;
  WHVideoModelRow    *row;

  priv = WH_VIDEO_VIEW_GET_PRIVATE(view);

  if (priv->active_item_num > priv->pending_item_num)
    clutter_behaviour_remove (priv->behave_down, priv->rows);
  else
    clutter_behaviour_remove (priv->behave_up, priv->rows);

  row = wh_video_model_get_row (priv->model, priv->active_item_num);
  wh_video_row_renderer_set_active (wh_video_model_row_get_renderer(row), 
				    FALSE); 

  priv->active_item_num = priv->pending_item_num;

  row = wh_video_model_get_row (priv->model, priv->active_item_num);
  wh_video_row_renderer_set_active (wh_video_model_row_get_renderer(row), 
				    TRUE); 

  clutter_actor_set_position (priv->rows, 0, 0);
}

static void
switch_timeline_completed_2 (ClutterTimeline *timeline,
			     gpointer         user_data)
{
  WHVideoView        *view = WH_VIDEO_VIEW(user_data);
  WHVideoViewPrivate *priv;

  priv = WH_VIDEO_VIEW_GET_PRIVATE(view);

  clutter_behaviour_remove (priv->behave_switch_out, CLUTTER_ACTOR(view));

  g_signal_handler_disconnect (timeline, priv->switch_signal);

  priv->switch_signal 
    = g_signal_connect (priv->timeline_switch, 
			"completed",
			G_CALLBACK (switch_timeline_completed_1),
			view);
}


static void
switch_timeline_completed_1 (ClutterTimeline *timeline,
			     gpointer         user_data)
{
  WHVideoView       *view = WH_VIDEO_VIEW(user_data);
  WHVideoViewPrivate *priv;

  priv = WH_VIDEO_VIEW_GET_PRIVATE(view);

  clutter_behaviour_remove (priv->behave_switch_in, CLUTTER_ACTOR(view));

  priv->active_item_num = 0;

  clutter_behaviour_apply (priv->behave_switch_out, CLUTTER_ACTOR(view));

  g_signal_handler_disconnect (timeline, priv->switch_signal);

  priv->switch_signal 
    = g_signal_connect (priv->timeline_switch, 
			"completed",
			G_CALLBACK (switch_timeline_completed_2),
			view);

  wh_video_view_remove_rows (view);
  wh_video_view_add_rows (view);

  ensure_layout (view,
		 clutter_actor_get_width(CLUTTER_ACTOR(view)),
		 clutter_actor_get_height(CLUTTER_ACTOR(view)),
		 priv->n_rows_visible);

  clutter_timeline_start (priv->timeline_switch);
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
  WHVideoViewPrivate *priv = WH_VIDEO_VIEW_GET_PRIVATE (view);

  priv->enable_model_change_anim = active;
}

static void
wh_video_view_remove_rows (WHVideoView *view)
{
  WHVideoViewPrivate *priv = WH_VIDEO_VIEW_GET_PRIVATE (view);

  clutter_group_remove_all (CLUTTER_GROUP(priv->rows));
}

static gboolean
add_row_foreach (WHVideoModel    *model,
		 WHVideoModelRow *row,
		 gpointer         data)
{
  WHVideoView *view = WH_VIDEO_VIEW(data); 
  WHVideoViewPrivate *priv; 
  WHVideoRowRenderer *renderer;
    
  priv = WH_VIDEO_VIEW_GET_PRIVATE (view);

  renderer = wh_video_model_row_get_renderer (row);

  /* Reset as could have been active previously */
  wh_video_row_renderer_set_active (renderer, FALSE); 

  clutter_group_add (CLUTTER_GROUP(priv->rows), CLUTTER_ACTOR(renderer));

  return TRUE;
}

static void
wh_video_view_add_rows (WHVideoView *view)
{
  WHVideoViewPrivate *priv = WH_VIDEO_VIEW_GET_PRIVATE (view);

  wh_video_model_foreach (priv->model, add_row_foreach, (gpointer)view);

  priv->n_items = wh_video_model_row_count (priv->model); /* cache */
}

#if 0
static void
on_model_row_added (WHVideoModel    *model, 
		    WHVideoModelRow *row,
		    gpointer         *userdata)
{
  WHVideoView        *view = WH_VIDEO_VIEW(userdata);
  WHVideoViewPrivate *priv;
  WHVideoRowRenderer *renderer;

  priv = WH_VIDEO_VIEW_GET_PRIVATE (view);

  renderer = wh_video_model_row_get_renderer (row);
  wh_video_row_renderer_set_active (renderer, FALSE); 

  clutter_group_add (CLUTTER_GROUP(priv->rows), CLUTTER_ACTOR(renderer));

  ensure_layout (view,
		 clutter_actor_get_width(CLUTTER_ACTOR(view)),
		 clutter_actor_get_height(CLUTTER_ACTOR(view)),
		 priv->n_rows_visible);
}
#endif

static void
on_model_rows_change (WHVideoModel *model, gpointer *userdata)
{
  WHVideoView        *view = WH_VIDEO_VIEW(userdata);
  WHVideoViewPrivate *priv;
  WHVideoModelRow    *row;

  priv = WH_VIDEO_VIEW_GET_PRIVATE (view);

  if (!priv->enable_model_change_anim)
    {
      row = wh_video_model_get_row (priv->model, priv->active_item_num);

      wh_video_view_remove_rows (view);
      wh_video_view_add_rows (view);

      /* If our row has dissapeared, just defualt to first */
      if (row != wh_video_model_get_row (priv->model, priv->active_item_num))
	priv->active_item_num = 0;

      ensure_layout (view,
		     clutter_actor_get_width(CLUTTER_ACTOR(view)),
		     clutter_actor_get_height(CLUTTER_ACTOR(view)),
		     priv->n_rows_visible);
      return;
    }

  /* FIXME: filter and sort each fire a signal giving us *2* signals 
   *        and thus we get called twice. Need to figure out a better
   *        fix.
  */
  if (clutter_timeline_is_playing (priv->timeline_switch))
    return;
  
  clutter_behaviour_apply (priv->behave_switch_in, CLUTTER_ACTOR(view));
  clutter_timeline_start (priv->timeline_switch);
}

static void
wh_video_view_init (WHVideoView *self)
{
  WHVideoViewPrivate *priv;
  ClutterColor        col = { 0xb4, 0xe2, 0xff, 0xff };
  ClutterColor rect_col   = { 0x88, 0x88, 0x97, 0xff };

  self->priv = priv = WH_VIDEO_VIEW_GET_PRIVATE (self);

  priv->rows = clutter_group_new();
  clutter_actor_set_parent(priv->rows, CLUTTER_ACTOR(self));

  /* Scrolling */

  priv->timeline = clutter_timeline_new (10, 60);

  priv->alpha = clutter_alpha_new_full (priv->timeline,
					alpha_sine_inc_func,
					NULL, NULL);

  g_signal_connect (priv->timeline, 
		    "completed",
		    G_CALLBACK (scroll_timeline_completed),
		    self);

  /* Underlying view change */

  priv->enable_model_change_anim = TRUE;

  priv->timeline_switch = clutter_timeline_new (10, 120);

  priv->alpha_switch = clutter_alpha_new_full (priv->timeline_switch,
					       alpha_sine_inc_func,
					       NULL, NULL);

  priv->behave_switch_in 
    = clutter_behaviour_opacity_new (priv->alpha_switch, 0xff, 0);
  priv->behave_switch_out 
    = clutter_behaviour_opacity_new (priv->alpha_switch, 0, 0xff);

  priv->switch_signal 
    = g_signal_connect (priv->timeline_switch, 
			"completed",
			G_CALLBACK (switch_timeline_completed_1),
			self);

  /* FIXME: global font/style settings  */
#define FONT "VistaSansBook 75px"

  priv->up = clutter_label_new_full (FONT, "<", &col);
  clutter_actor_rotate_z (priv->up, 
			90.0,
			clutter_actor_get_width (priv->up)/2, 
			clutter_actor_get_height (priv->up)/2);

  priv->down = clutter_label_new_full (FONT, "<", &col);
  clutter_actor_rotate_z (priv->down, 
			270.0,
			clutter_actor_get_width (priv->down)/2, 
			clutter_actor_get_height (priv->down)/2);

  clutter_actor_set_parent (priv->up, CLUTTER_ACTOR(self));
  clutter_actor_set_parent (priv->down, CLUTTER_ACTOR(self));

  priv->selector = clutter_rectangle_new_with_color (&rect_col);
  clutter_actor_set_parent (priv->selector, CLUTTER_ACTOR(self));
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

