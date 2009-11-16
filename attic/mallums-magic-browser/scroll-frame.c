#include <tidy/tidy.h>

#include "scroll-frame.h"

static void scrollable_iface_init (TidyScrollableInterface *iface);

static void scrollable_set_adjustments (TidyScrollable *scrollable,
					TidyAdjustment *hadjustment,
					TidyAdjustment *vadjustment);
static void scrollable_get_adjustments (TidyScrollable  *scrollable,
					TidyAdjustment **hadjustment,
					TidyAdjustment **vadjustment);

enum {
  PROP_0,
  PROP_HADJUST,
  PROP_VADJUST
};

G_DEFINE_TYPE_WITH_CODE (ScrollFrame, scroll_frame, TIDY_TYPE_FRAME,
			 G_IMPLEMENT_INTERFACE (TIDY_TYPE_SCROLLABLE,
						scrollable_iface_init));
#define FRAME_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), SCROLL_TYPE_FRAME, ScrollFramePrivate))

struct _ScrollFramePrivate {
  TidyAdjustment *hadj;
  TidyAdjustment *vadj;
  
  WebkitAdjustment *wk_hadj;
  WebkitAdjustment *wk_vadj;
  
  guint           hadj_idle;
  gdouble         hadj_value;

  guint           vadj_idle;
  gdouble         vadj_value;  
};

static void
scroll_frame_finalize (GObject *object)
{
  G_OBJECT_CLASS (scroll_frame_parent_class)->finalize (object);
}

static void
scroll_frame_dispose (GObject *object)
{
  ScrollFramePrivate *priv = SCROLL_FRAME (object)->priv;
  
  if (priv->hadj_idle) {
    g_source_remove (priv->hadj_idle);
    priv->hadj_idle = 0;
  }

  if (priv->vadj_idle) {
    g_source_remove (priv->vadj_idle);
    priv->vadj_idle = 0;
  }
  
  G_OBJECT_CLASS (scroll_frame_parent_class)->dispose (object);
}

static void
scroll_frame_set_property (GObject      *object,
			   guint         prop_id,
			   const GValue *value,
			   GParamSpec   *pspec)
{
  ScrollFrame *frame = SCROLL_FRAME (object);

  switch (prop_id) {
  case PROP_HADJUST:
    scrollable_set_adjustments (TIDY_SCROLLABLE (object),
				g_value_get_object (value),
				frame->priv->vadj);
    break;

  case PROP_VADJUST:
    scrollable_set_adjustments (TIDY_SCROLLABLE (object),
				frame->priv->hadj,
				g_value_get_object (value));
    break;

  default:
    break;
  }
}

static void
scroll_frame_get_property (GObject    *object,
			   guint       prop_id,
			   GValue     *value,
			   GParamSpec *pspec)
{
  ScrollFrame *frame = SCROLL_FRAME (object);
  TidyAdjustment *adj;

  switch (prop_id) {
  case PROP_HADJUST:
    scrollable_get_adjustments (TIDY_SCROLLABLE (object), &adj, NULL);
    g_value_set_object (value, adj);
    break;

  case PROP_VADJUST:
    scrollable_get_adjustments (TIDY_SCROLLABLE (object), NULL, &adj);
    g_value_set_object (value, adj);
    break;

  default:
    break;
  }
}

static gboolean
scroll_frame_button_release_event (ClutterActor       *actor,
				   ClutterButtonEvent *event)
{
  ClutterActor *child = tidy_frame_get_child (TIDY_FRAME (actor));

  if (child) {
    return clutter_actor_event (child, (ClutterEvent *) event, FALSE);
  }

  return FALSE;
}

static void
scroll_frame_class_init (ScrollFrameClass *klass)
{
  GObjectClass *o_class = (GObjectClass *) klass;
  ClutterActorClass *a_class = (ClutterActorClass *) klass;

  o_class->finalize = scroll_frame_finalize;
  o_class->dispose = scroll_frame_dispose;
  o_class->set_property = scroll_frame_set_property;
  o_class->get_property = scroll_frame_get_property;

  a_class->button_release_event = scroll_frame_button_release_event;

  g_type_class_add_private (klass, sizeof (ScrollFramePrivate));

  g_object_class_override_property (o_class, PROP_HADJUST, "hadjustment");
  g_object_class_override_property (o_class, PROP_VADJUST, "vadjustment");
}

static void
vadj_wk_changed (WebkitAdjustment *adjustment,
		 ScrollFrame      *frame)
{
  ScrollFramePrivate *priv = frame->priv;
  double value, lower, upper, step, page_inc, page_size;

  webkit_adjustment_get_values (adjustment, &value, &lower, &upper,
				&step, &page_inc, &page_size);
  tidy_adjustment_set_values (priv->vadj, value, lower, upper, step,
			      page_inc, page_size);
}

static void
vadj_wk_value_changed (WebkitAdjustment *adjustment,
		       GParamSpec       *pspec,
		       ScrollFrame      *frame)
{
  vadj_wk_changed (adjustment, frame);
}

static void
hadj_wk_changed (WebkitAdjustment *adjustment,
		 ScrollFrame      *frame)
{
  ScrollFramePrivate *priv = frame->priv;
  double value, lower, upper, step, page_inc, page_size;

  webkit_adjustment_get_values (adjustment, &value, &lower, &upper,
				&step, &page_inc, &page_size);
  tidy_adjustment_set_values (priv->hadj, value, lower, upper, step,
			      page_inc, page_size);
}

static void
hadj_wk_value_changed (WebkitAdjustment *adjustment,
		       GParamSpec       *pspec,
		       ScrollFrame      *frame)
{
  hadj_wk_changed (adjustment, frame);
}

static gboolean
vadj_idle_cb (ScrollFrame *frame)
{
  ScrollFramePrivate *priv = frame->priv;
  
  webkit_adjustment_set_value (priv->wk_vadj, priv->vadj_value);
  priv->vadj_idle = 0;
  
  return FALSE;
}

static gboolean
hadj_idle_cb (ScrollFrame *frame)
{
  ScrollFramePrivate *priv = frame->priv;
  
  webkit_adjustment_set_value (priv->wk_hadj, priv->hadj_value);
  priv->hadj_idle = 0;
  
  return FALSE;
}

static void 
vadj_tidy_changed (TidyAdjustment *adjustment,
		   GParamSpec     *pspec,
		   ScrollFrame    *frame)
{
  ScrollFramePrivate *priv = frame->priv;
  double value;

  value = tidy_adjustment_get_value (adjustment);
  priv->vadj_value = value;
  if (!priv->vadj_idle)
    priv->vadj_idle = g_idle_add_full (G_PRIORITY_DEFAULT,
                                       (GSourceFunc)vadj_idle_cb, frame, NULL);
}

static void
hadj_tidy_changed (TidyAdjustment *adjustment,
		   GParamSpec     *pspec,
		   ScrollFrame    *frame)
{
  ScrollFramePrivate *priv = frame->priv;
  double value;

  value = tidy_adjustment_get_value (adjustment);
  priv->hadj_value = value;
  if (!priv->hadj_idle)
    priv->hadj_idle = g_idle_add_full (G_PRIORITY_DEFAULT,
                                       (GSourceFunc)hadj_idle_cb, frame, NULL);
}

static void
scrollable_set_adjustments (TidyScrollable *scrollable,
			    TidyAdjustment *hadjustment,
			    TidyAdjustment *vadjustment)
{
  ScrollFramePrivate *priv = SCROLL_FRAME (scrollable)->priv;

  if (hadjustment != priv->hadj) {
    if (priv->hadj) {
      g_signal_handlers_disconnect_by_func (priv->hadj,
					    hadj_tidy_changed,
					    scrollable);
      g_object_unref (priv->hadj);
    }

    if (hadjustment) {
      g_object_ref (hadjustment);
      g_signal_connect (hadjustment, "notify::value",
			G_CALLBACK (hadj_tidy_changed), scrollable);
    }

    priv->hadj = hadjustment;
  }

  if (vadjustment != priv->vadj) {
    if (priv->vadj) {
      g_signal_handlers_disconnect_by_func (priv->vadj,
					    vadj_tidy_changed,
					    scrollable);
      g_object_unref (priv->vadj);
    }

    if (vadjustment) {
      g_object_ref (vadjustment);
      g_signal_connect (vadjustment, "notify::value",
			G_CALLBACK (vadj_tidy_changed), scrollable);
    }

    priv->vadj = vadjustment;
  }
}

static void
scrollable_get_adjustments (TidyScrollable  *scrollable,
			    TidyAdjustment **hadjustment,
			    TidyAdjustment **vadjustment)
{
  ScrollFramePrivate *priv = SCROLL_FRAME (scrollable)->priv;

  if (hadjustment) {
    if (priv->hadj) {
      *hadjustment = priv->hadj;
    } else {
      TidyAdjustment *adjustment = tidy_adjustment_newx (0, 0, 0, 0, 0, 0);
      double value, lower, upper, step, page_inc, page_size;

      webkit_adjustment_get_values (priv->wk_hadj, &value, &lower, &upper,
				    &step, &page_inc, &page_size);
      tidy_adjustment_set_values (adjustment, value, lower, upper, step,
				  page_inc, page_size);

      scrollable_set_adjustments (scrollable, adjustment, priv->vadj);
      *hadjustment = adjustment;
    }

    g_signal_connect (priv->wk_hadj, "notify::value",
		      G_CALLBACK (hadj_wk_value_changed), scrollable);
    g_signal_connect (priv->wk_hadj, "changed",
		      G_CALLBACK (hadj_wk_changed), scrollable);
  }

  if (vadjustment) {
    if (priv->vadj) {
      *vadjustment = priv->vadj;
    } else {
      TidyAdjustment *adjustment = tidy_adjustment_newx (0, 0, 0, 0, 0, 0);
      double value, lower, upper, step, page_inc, page_size;

      webkit_adjustment_get_values (priv->wk_vadj, &value, &lower, &upper,
				    &step, &page_inc, &page_size);
      tidy_adjustment_set_values (adjustment, value, lower, upper, step,
				  page_inc, page_size);

      scrollable_set_adjustments (scrollable, priv->hadj, adjustment);
      *vadjustment = adjustment;
    }

    g_signal_connect (priv->wk_vadj, "notify::value",
		      G_CALLBACK (vadj_wk_value_changed), scrollable);
    g_signal_connect (priv->wk_vadj, "changed",
		      G_CALLBACK (vadj_wk_changed), scrollable);
  }
}
      
static void
scrollable_iface_init (TidyScrollableInterface *iface)
{
  iface->set_adjustments = scrollable_set_adjustments;
  iface->get_adjustments = scrollable_get_adjustments;
}

static void
scroll_frame_init (ScrollFrame *frame)
{
  ScrollFramePrivate *priv;
  ClutterColor bg = {0x00, 0x00, 0x00, 0x00};

  priv = frame->priv = FRAME_PRIVATE (frame);
  priv->wk_hadj = webkit_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  priv->wk_vadj = webkit_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
}

void
scroll_frame_add_webkit (ScrollFrame   *frame,
			 WebKitWebView *web_view)
{
  webkit_web_view_set_scroll_adjustments (web_view,
					  frame->priv->wk_hadj,
					  frame->priv->wk_vadj);
  clutter_container_add_actor (CLUTTER_CONTAINER (frame), 
			       CLUTTER_ACTOR (web_view));
}
