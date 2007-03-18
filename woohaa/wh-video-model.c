 #include "wh-video-model.h"
#include <string.h>

G_DEFINE_TYPE (WHVideoModel, wh_video_model, G_TYPE_OBJECT);

#define VIDEO_MODEL_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), WH_TYPE_VIDEO_MODEL, WHVideoModelPrivate))

typedef struct _WHVideoModelPrivate WHVideoModelPrivate;

enum
{
  REORDERED,
  ROW_CHANGED,
  ROW_ADDED,
  FILTER,
  LAST_SIGNAL
};

static guint _model_signals[LAST_SIGNAL] = { 0 };

struct _WHVideoModelPrivate
{
  WHFilterRowFunc  filter;
  gpointer         filter_data;
  WHCompareRowFunc sort;
  gpointer         sort_data;
  EggSequence     *rows;
};

static void
wh_video_model_get_property (GObject *object, guint property_id,
			     GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
wh_video_model_set_property (GObject *object, guint property_id,
				  const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
wh_video_model_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (wh_video_model_parent_class)->dispose)
    G_OBJECT_CLASS (wh_video_model_parent_class)->dispose (object);
}

static void
wh_video_model_finalize (GObject *object)
{
  G_OBJECT_CLASS (wh_video_model_parent_class)->finalize (object);
}

static void
wh_video_model_class_init (WHVideoModelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (WHVideoModelPrivate));

  object_class->get_property = wh_video_model_get_property;
  object_class->set_property = wh_video_model_set_property;
  object_class->dispose = wh_video_model_dispose;
  object_class->finalize = wh_video_model_finalize;

  _model_signals[REORDERED] =
    g_signal_new ("rows-reordered",
		  G_OBJECT_CLASS_TYPE (object_class),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (WHVideoModelClass, reordered),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  _model_signals[FILTER] =
    g_signal_new ("filter-changed",
		  G_OBJECT_CLASS_TYPE (object_class),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (WHVideoModelClass, filter_change),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  _model_signals[ROW_CHANGED] =
    g_signal_new ("row-changed",
		  G_OBJECT_CLASS_TYPE (object_class),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (WHVideoModelClass, row_change),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__OBJECT,
		  G_TYPE_NONE, 1, WH_TYPE_VIDEO_MODEL_ROW);

  _model_signals[ROW_ADDED] =
    g_signal_new ("row-added",
		  G_OBJECT_CLASS_TYPE (object_class),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (WHVideoModelClass, row_added),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__OBJECT,
		  G_TYPE_NONE, 1, WH_TYPE_VIDEO_MODEL_ROW);

}

static void
wh_video_model_init (WHVideoModel *self)
{
  WHVideoModelPrivate *priv = VIDEO_MODEL_PRIVATE(self);

  priv->rows = egg_sequence_new (NULL);
}

static gboolean 
check_filter (WHVideoModel *model, EggSequenceIter *iter)
{
  WHVideoModelPrivate *priv = VIDEO_MODEL_PRIVATE(model);  
  gboolean res;

  if (priv->filter == NULL)
    return TRUE;

  res = priv->filter(model, 
		     (WHVideoModelRow*)egg_sequence_get (iter), 
		     priv->filter_data); 
  return res;
}

guint
wh_video_model_row_count (WHVideoModel *model)
{
  WHVideoModelPrivate *priv = VIDEO_MODEL_PRIVATE(model);  
  EggSequenceIter     *iter;
  gint                 n = 0;

  if (priv->filter == NULL)
    return egg_sequence_get_length (priv->rows);    

  iter = egg_sequence_get_begin_iter (priv->rows);

  while (!egg_sequence_iter_is_end (iter))
    {
      if (check_filter (model, iter))
	n++;
      iter = egg_sequence_iter_next (iter);
    }

  return n;
}

WHVideoModelRow*
wh_video_model_get_row (WHVideoModel *model, gint index)
{
  WHVideoModelPrivate *priv = VIDEO_MODEL_PRIVATE(model);
  EggSequenceIter     *iter;
  gint                 n = 0;

  if (priv->filter == NULL)
    return (WHVideoModelRow*)egg_sequence_get 
      (egg_sequence_get_iter_at_pos (priv->rows, index));

  iter = egg_sequence_get_begin_iter (priv->rows);

  while (!egg_sequence_iter_is_end (iter))
    {
      if (check_filter (model, iter))
	{
	  if (n == index)
	    return (WHVideoModelRow*)egg_sequence_get (iter);
	  n++;
	}
      iter = egg_sequence_iter_next (iter);
    }

  return NULL;
}

static void
on_row_changed (GObject      *obj,
		GParamSpec   *arg1,
		gpointer      data)
{
  WHVideoModel        *model = WH_VIDEO_MODEL(data);
  WHVideoModelPrivate *priv;

  priv = VIDEO_MODEL_PRIVATE(model);

  if (priv->sort)
    {
      egg_sequence_sort (priv->rows, 
			 (GCompareDataFunc)priv->sort, priv->sort_data);
      g_signal_emit (model, _model_signals[REORDERED], 0);
    }

  g_signal_emit (model, _model_signals[ROW_CHANGED], 0, 
		 WH_VIDEO_MODEL_ROW(obj));
} 

void
wh_video_model_append_row (WHVideoModel *model, WHVideoModelRow *row)
{
  WHVideoModelPrivate *priv = VIDEO_MODEL_PRIVATE(model);
  EggSequenceIter     *iter;

  g_signal_connect (row,
		    "notify",
		    G_CALLBACK (on_row_changed),
		    model);

  g_object_ref (row);

  if (priv->sort)
    iter = egg_sequence_insert_sorted (priv->rows,
				       (gpointer)row,
				       priv->sort,
				       priv->sort_data);
  else
    iter = egg_sequence_append (priv->rows, (gpointer)row);

  if (check_filter (model, iter))
    g_signal_emit (model, _model_signals[ROW_ADDED], 0, row);
}


void
wh_video_model_foreach (WHVideoModel      *model, 
		        WHForeachRowFunc   func,
			gpointer           data)
{
  WHVideoModelPrivate *priv = VIDEO_MODEL_PRIVATE(model);
  EggSequenceIter     *iter;

  iter = egg_sequence_get_begin_iter (priv->rows);

    while (!egg_sequence_iter_is_end (iter))
    {
      if (check_filter (model, iter))
	func (model, 
	      (WHVideoModelRow*)egg_sequence_get (iter),
	      data);
	
      iter = egg_sequence_iter_next (iter);
    }
}

void
wh_video_model_set_sort_func (WHVideoModel     *model, 
			      WHCompareRowFunc  func, 
			      gpointer          userdata)
{
  WHVideoModelPrivate *priv = VIDEO_MODEL_PRIVATE(model);

  priv->sort      = func;
  priv->sort_data = userdata;

  if (func)
    {
      egg_sequence_sort (priv->rows, (GCompareDataFunc)func, userdata);
      g_signal_emit (model, _model_signals[REORDERED], 0);
    }
}

void
wh_video_model_set_filter (WHVideoModel    *model,
			   WHFilterRowFunc  filter, 
			   gpointer         data)
{
  WHVideoModelPrivate *priv = VIDEO_MODEL_PRIVATE(model);
  WHFilterRowFunc      prev_filter;

  prev_filter = priv->filter;

  priv->filter      = filter;
  priv->filter_data = data;

  if (prev_filter != priv->filter)
    g_signal_emit (model, _model_signals[FILTER], 0);
}

WHVideoModel*
wh_video_model_new ()
{
  return g_object_new (WH_TYPE_VIDEO_MODEL, NULL);
}

