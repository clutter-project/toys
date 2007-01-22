#include "clutter-video-view.h"
#include "util.h"

#define CLUTTER_TYPE_BEHAVIOUR_SPINNER (clutter_behaviour_spinner_get_type ())

#define CLUTTER_BEHAVIOUR_SPINNER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  CLUTTER_TYPE_BEHAVIOUR_SPINNER, ClutterBehaviourSpinner))

#define CLUTTER_BEHAVIOUR_SPINNER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  CLUTTER_TYPE_BEHAVIOUR_SPINNER, ClutterBehaviourSpinnerClass))

#define CLUTTER_IS_BEHAVIOUR_SPINNER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  CLUTTER_TYPE_BEHAVIOUR_SPINNER))

#define CLUTTER_IS_BEHAVIOUR_SPINNER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  CLUTTER_TYPE_BEHAVIOUR_SPINNER))

#define CLUTTER_BEHAVIOUR_SPINNER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  CLUTTER_TYPE_BEHAVIOUR_SPINNER, ClutterBehaviourSpinnerClass))

typedef struct _ClutterBehaviourSpinner        ClutterBehaviourSpinner;
typedef struct _ClutterBehaviourSpinnerClass   ClutterBehaviourSpinnerClass;
 
struct _ClutterBehaviourSpinner
{
  ClutterBehaviour             parent;
  gdouble                      start;
  gdouble                      end;
};

struct _ClutterBehaviourSpinnerClass
{
  ClutterBehaviourClass   parent_class;
};

GType clutter_behaviour_spinner_get_type (void) G_GNUC_CONST;

G_DEFINE_TYPE (ClutterBehaviourSpinner, clutter_behaviour_spinner, CLUTTER_TYPE_BEHAVIOUR);

static void
clutter_behaviour_alpha_notify (ClutterBehaviour *behave,
                                guint32           alpha_value)
{
  ClutterBehaviourSpinner *spin = CLUTTER_BEHAVIOUR_SPINNER(behave);
  gint                     i, n_actors;
  ClutterActor            *actor;
  gdouble                  ang;

  ang = (gdouble) alpha_value * (spin->end - spin->start) / (gdouble)CLUTTER_ALPHA_MAX_ALPHA;

  ang += spin->start;

  n_actors = clutter_behaviour_get_n_actors(behave);

  for (i=0; i < n_actors; i++)
    {
      actor = clutter_behaviour_get_nth_actor (behave, i);

      clutter_actor_rotate_y(actor, ang, 
			     clutter_actor_get_width (actor)/2, 
			     0);
    }
}

static void
clutter_behaviour_spinner_class_init (ClutterBehaviourSpinnerClass *klass)
{
  ClutterBehaviourClass *behave_class = CLUTTER_BEHAVIOUR_CLASS (klass);

  behave_class->alpha_notify = clutter_behaviour_alpha_notify;
}

static void
clutter_behaviour_spinner_init (ClutterBehaviourSpinner *self)
{
  ;
}

static ClutterBehaviour*
clutter_behaviour_spinner_new (ClutterAlpha       *alpha,
			       gdouble             angle_start,
			       gdouble             angle_end)
{
  ClutterBehaviourSpinner *spin_behave;

  spin_behave = g_object_new (CLUTTER_TYPE_BEHAVIOUR_SPINNER, 
			      "alpha", alpha,
			      NULL);

  spin_behave->start = angle_start;
  spin_behave->end   = angle_end;

  return CLUTTER_BEHAVIOUR(spin_behave);
}



G_DEFINE_TYPE (ClutterVideoView, clutter_video_view, CLUTTER_TYPE_ACTOR);

enum
{
  PROP_0,
};

#define CLUTTER_VIDEO_VIEW_GET_PRIVATE(obj) \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CLUTTER_TYPE_VIDEO_VIEW, ClutterVideoViewPrivate))

struct _ClutterVideoViewPrivate
{
  ClutterVideoModel *model;
  ClutterActor      *list; 
  ClutterActor      *selector; 
  gint               row_width, row_height;
  gint               n_items_visible;

  /* For vertical scroll */
  ClutterTimeline   *timeline;
  ClutterAlpha      *alpha;
  ClutterBehaviour  *behave_up;
  ClutterBehaviour  *behave_down;
  ClutterKnot        path_down[3], path_up[3];

  /* For switch animation */
  ClutterTimeline   *timeline_switch;
  ClutterAlpha      *alpha_switch;
  ClutterBehaviour  *behave_switch_out;
  ClutterBehaviour  *behave_switch_in;
  gulong             switch_signal;

  gint               active_item_num;
  gint               pending_item_num;
  gint               current_offset;
};

static void
switch_timeline_completed_1 (ClutterTimeline *timeline,
			     gpointer         user_data);

static void 
sync_view_foreach  (ClutterVideoModel     *model,
		    ClutterVideoModelItem *item,
		    gpointer               user_data)
{
  ClutterVideoView *view = CLUTTER_VIDEO_VIEW(user_data);

  clutter_group_add (CLUTTER_GROUP(view->priv->list),
		     CLUTTER_ACTOR(item->group));

  clutter_actor_set_position (CLUTTER_ACTOR(item->group), 
			      0, view->priv->current_offset);

  clutter_actor_show_all (CLUTTER_ACTOR(item->group));  

  view->priv->current_offset += view->priv->row_height;
}

static void
sync_view (ClutterVideoView *view)
{
  view->priv->current_offset = 0;

  clutter_video_model_view_foreach (view->priv->model, 
				    sync_view_foreach, 
				    view);  
}

static void
clutter_video_view_set_property (GObject      *object, 
				 guint         prop_id,
				 const GValue *value, 
				 GParamSpec   *pspec)
{
  ClutterVideoView        *disk;
  ClutterVideoViewPrivate *priv;

  disk = CLUTTER_VIDEO_VIEW(object);
  priv = disk->priv;

  switch (prop_id) 
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
clutter_video_view_get_property (GObject    *object, 
				  guint       prop_id,
				  GValue     *value, 
				  GParamSpec *pspec)
{
  ClutterVideoView        *disk;
  ClutterVideoViewPrivate *priv;

  disk = CLUTTER_VIDEO_VIEW(object);
  priv = disk->priv;

  switch (prop_id) 
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    } 
}

static void
clutter_video_view_request_coords (ClutterActor    *self,
				    ClutterActorBox *box)
{
  ClutterVideoViewPrivate *priv;

  priv = CLUTTER_VIDEO_VIEW(self)->priv;

  if (box->x2 - box->x1 != priv->row_width)
    {
      priv->row_height = (box->y2 - box->y1) / priv->n_items_visible;
      priv->row_width  = box->x2 - box->x1;

      clutter_video_model_set_cell_size (priv->model,
					 priv->row_width,
					 priv->row_height);


      sync_view (CLUTTER_VIDEO_VIEW(self));
    }

  if (priv->selector)
    {
      clutter_actor_set_size (priv->selector, priv->row_width, priv->row_height);
      clutter_actor_set_position (priv->selector, 0, priv->row_height);  
    }

  clutter_actor_set_clip (self, 0, 0, 
			  priv->row_width, box->y2 - box->y1);
}

static void
clutter_video_view_paint (ClutterActor *actor)
{
  ClutterVideoView *view = CLUTTER_VIDEO_VIEW(actor);

  glPushMatrix();

  if (view->priv->selector)
    clutter_actor_paint (view->priv->selector);

  glTranslatef(0.0, 
	       (float)-1.0 * (view->priv->active_item_num - 1) * view->priv->row_height, 
	       0.0);

  clutter_actor_paint (view->priv->list);

  glPopMatrix();
}

static void
clutter_video_view_realize (ClutterActor *self)
{
  ClutterVideoView        *menu;

  menu = CLUTTER_VIDEO_VIEW(self);
}

static void 
clutter_video_view_dispose (GObject *object)
{
  ClutterVideoView         *self = CLUTTER_VIDEO_VIEW(object);
  ClutterVideoViewPrivate  *priv;  

  priv = self->priv;
  
  G_OBJECT_CLASS (clutter_video_view_parent_class)->dispose (object);
}

static void 
clutter_video_view_finalize (GObject *object)
{
  G_OBJECT_CLASS (clutter_video_view_parent_class)->finalize (object);
}

static void
clutter_video_view_class_init (ClutterVideoViewClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass   *actor_class = CLUTTER_ACTOR_CLASS (klass);
  ClutterActorClass   *parent_class; 

  parent_class = CLUTTER_ACTOR_CLASS (clutter_video_view_parent_class);

  actor_class->paint      = clutter_video_view_paint;
  actor_class->realize    = clutter_video_view_realize;
  actor_class->request_coords  = clutter_video_view_request_coords;
  actor_class->unrealize  = parent_class->unrealize;
  actor_class->show       = parent_class->show;
  actor_class->hide       = parent_class->hide;

  gobject_class->finalize     = clutter_video_view_finalize;
  gobject_class->dispose      = clutter_video_view_dispose;
  gobject_class->set_property = clutter_video_view_set_property;
  gobject_class->get_property = clutter_video_view_get_property;

  g_type_class_add_private (gobject_class, sizeof (ClutterVideoViewPrivate));
}

static void
clutter_video_view_init (ClutterVideoView *self)
{
  ClutterVideoViewPrivate *priv;

  self->priv = priv = CLUTTER_VIDEO_VIEW_GET_PRIVATE (self);
}


void
clutter_video_view_activate (ClutterVideoView  *view,
			     gint               entry_num)
{
  if (entry_num < 0 
      || entry_num >= clutter_video_model_item_count(view->priv->model))
    return;

  if (clutter_timeline_is_playing(view->priv->timeline))
    {
      // clutter_timeline_skip(view->priv->timeline, 5);
      return;
    }

  view->priv->pending_item_num = entry_num;

  if (view->priv->active_item_num > view->priv->pending_item_num)
      clutter_behaviour_apply (view->priv->behave_down, view->priv->list);
  else
    clutter_behaviour_apply (view->priv->behave_up, view->priv->list);

  clutter_timeline_start (view->priv->timeline);
}

void
clutter_video_view_advance (ClutterVideoView *view, gint n)
{
  clutter_video_view_activate (view, view->priv->active_item_num + n);
}

void
clutter_video_view_switch (ClutterVideoView *view)
{
  clutter_behaviour_apply (view->priv->behave_switch_in, CLUTTER_ACTOR(view));
  clutter_timeline_start (view->priv->timeline_switch);
}


static void
scroll_timeline_completed (ClutterTimeline *timeline,
			   gpointer         user_data)
{
  ClutterVideoView *view = CLUTTER_VIDEO_VIEW(user_data);

  if (view->priv->active_item_num > view->priv->pending_item_num)
    clutter_behaviour_remove (view->priv->behave_down, view->priv->list);
  else
    clutter_behaviour_remove (view->priv->behave_up, view->priv->list);

  view->priv->active_item_num = view->priv->pending_item_num;

  clutter_actor_set_position (view->priv->list, 0, 0);
}

static void
switch_timeline_completed_2 (ClutterTimeline *timeline,
			     gpointer         user_data)
{
  ClutterVideoView *view = CLUTTER_VIDEO_VIEW(user_data);

  clutter_behaviour_remove (view->priv->behave_switch_out, 
			    CLUTTER_ACTOR(view));

  g_signal_handler_disconnect (timeline, view->priv->switch_signal);

  view->priv->switch_signal = g_signal_connect (view->priv->timeline_switch, 
						"completed",
						G_CALLBACK (switch_timeline_completed_1),
						view);
}


static void
switch_timeline_completed_1 (ClutterTimeline *timeline,
			   gpointer         user_data)
{
  ClutterVideoView *view = CLUTTER_VIDEO_VIEW(user_data);

  clutter_behaviour_remove (view->priv->behave_switch_in, CLUTTER_ACTOR(view));

  view->priv->active_item_num = 0;

  clutter_behaviour_apply (view->priv->behave_switch_out, CLUTTER_ACTOR(view));

  g_signal_handler_disconnect (timeline, view->priv->switch_signal);

  view->priv->switch_signal = g_signal_connect (view->priv->timeline_switch, 
						"completed",
						G_CALLBACK (switch_timeline_completed_2),
						view);

  clutter_timeline_start (view->priv->timeline_switch);

}

ClutterVideoModelItem*
clutter_video_view_get_selected (ClutterVideoView *view)
{
  return clutter_video_model_get_item (view->priv->model, 
				       view->priv->active_item_num);
}

ClutterActor*
clutter_video_view_new (ClutterVideoModel *model, 
			gint               width, 
			gint               height,
			gint               n_items_visible)
{
  ClutterActor              *view;
  ClutterVideoViewPrivate   *priv;

  view = g_object_new (CLUTTER_TYPE_VIDEO_VIEW, NULL);

  priv = CLUTTER_VIDEO_VIEW_GET_PRIVATE (view);

  priv->model = model;

  /* Add models items to our list */
  priv->list = clutter_group_new();
  clutter_actor_set_parent(priv->list, view);
  clutter_actor_show_all (priv->list);

  /* Set size - syncs model + sets row sizes - FIXME: messy */
  priv->n_items_visible = n_items_visible;
  clutter_actor_set_size (view, width, height);

  /* Selector */
  priv->selector = util_actor_from_file ("selector.svg", 
					 priv->row_width, 
					 priv->row_height);
  clutter_actor_set_parent(priv->selector, view);


  /* Scrolling */

  priv->timeline = clutter_timeline_new (15, 60);

  priv->alpha = clutter_alpha_new_full (priv->timeline,
					alpha_sine_inc_func,
					NULL, NULL);

  priv->path_up[0].x = 0;   priv->path_up[0].y = 0; 
  priv->path_up[1].x = 0;   
  priv->path_up[1].y = -1 * (priv->row_height + (priv->row_height/4)); 
  priv->path_up[2].x = 0;   priv->path_up[2].y = -1 * priv->row_height; 

  priv->path_down[0].x = 0;   priv->path_down[0].y = 0; 
  priv->path_down[1].x = 0;   
  priv->path_down[1].y = (priv->row_height + (priv->row_height/4)); 
  priv->path_down[2].x = 0;   priv->path_down[2].y = priv->row_height; 

  priv->behave_up = clutter_behaviour_path_new (priv->alpha, 
						priv->path_up, 3);
  priv->behave_down = clutter_behaviour_path_new (priv->alpha, 
						  priv->path_down, 3);

  g_signal_connect (priv->timeline, 
		    "completed",
		    G_CALLBACK (scroll_timeline_completed),
		    view);

  /* Underlying view change */

  priv->timeline_switch = clutter_timeline_new (10, 120);

  priv->alpha_switch = clutter_alpha_new_full (priv->timeline_switch,
					       alpha_sine_inc_func,
					       NULL, NULL);

  priv->behave_switch_in 
    = clutter_behaviour_opacity_new (priv->alpha_switch, 0xff, 0);
  priv->behave_switch_out 
    = clutter_behaviour_opacity_new (priv->alpha_switch, 0, 0xff);


  /*
  priv->behave_switch_in 
    = clutter_behaviour_spinner_new (priv->alpha_switch, 0.0, 90.0);

  priv->behave_switch_out 
    = clutter_behaviour_spinner_new (priv->alpha_switch, 270.0, 360.0);
  */

  priv->switch_signal 
    = g_signal_connect (priv->timeline_switch, 
			"completed",
			G_CALLBACK (switch_timeline_completed_1),
			view);

  return view;
}

