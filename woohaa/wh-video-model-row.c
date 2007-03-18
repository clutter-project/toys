/* wh-video-model-row.c */

#include "wh-video-model-row.h"
#include "wh-video-row-renderer.h"

G_DEFINE_TYPE (WHVideoModelRow, wh_video_model_row, G_TYPE_OBJECT);

#define VIDEO_MODEL_ROW_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), WH_TYPE_VIDEO_MODEL_ROW, WHVideoModelRowPrivate))

typedef struct _WHVideoModelRowPrivate WHVideoModelRowPrivate;

struct _WHVideoModelRowPrivate
{
  gchar              *path;
  gchar              *title;
  gchar              *series;
  gchar              *episode;
  gint                n_views;
  time_t              age;
  time_t              vtime;
  GdkPixbuf          *thumbnail;
  WHVideoRowRenderer *renderer; 
};

enum
{
  PROP_0,
  PROP_PATH,
  PROP_TITLE,
  PROP_N_VIEWS,
  PROP_AGE,
  PROP_RENDERER,
  PROP_VTIME,
  PROP_SERIES,
  PROP_EPISODE,
  PROP_THUMBNAIL
};

static void
wh_video_model_row_get_property (GObject *object, guint property_id,
				 GValue *value, GParamSpec *pspec)
{
  WHVideoModelRow        *row = WH_VIDEO_MODEL_ROW(object);
  WHVideoModelRowPrivate *priv;  
  
  priv = VIDEO_MODEL_ROW_PRIVATE(row);
  
  switch (property_id) 
    {
    case PROP_PATH:
      g_value_set_string (value, priv->path);
      break;
    case PROP_TITLE:
      g_value_set_string (value, priv->title);
      break;
    case PROP_SERIES:
      g_value_set_string (value, priv->series);
      break;
    case PROP_EPISODE:
      g_value_set_string (value, priv->episode);
      break;
    case PROP_N_VIEWS:
      g_value_set_int (value, wh_video_model_row_get_n_views (row));
      break;
    case PROP_AGE:
      g_value_set_int (value, wh_video_model_row_get_age (row));
      break;
    case PROP_VTIME:
      g_value_set_int (value, wh_video_model_row_get_vtime (row));
      break;
    case PROP_RENDERER:
      g_value_set_object (value, priv->renderer);
      break;
    case PROP_THUMBNAIL:
      g_value_set_object (value, priv->thumbnail);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
wh_video_model_row_set_property (GObject *object, guint property_id,
				 const GValue *value, GParamSpec *pspec)
{
 WHVideoModelRow *row = WH_VIDEO_MODEL_ROW(object);
  WHVideoModelRowPrivate *priv;  
  
  priv = VIDEO_MODEL_ROW_PRIVATE(row);

  switch (property_id) 
    {
    case PROP_PATH:
      wh_video_model_row_set_path (row, g_value_get_string (value));
      break;
    case PROP_TITLE:
      wh_video_model_row_set_title (row, g_value_get_string (value));
      break;
    case PROP_SERIES:
      wh_video_model_row_set_extended_info (row, 
					    g_value_get_string (value),
					    priv->episode);
      break;
    case PROP_EPISODE:
      wh_video_model_row_set_extended_info (row,
					    priv->series,
					    g_value_get_string (value));
      break;
    case PROP_N_VIEWS:
      wh_video_model_row_set_n_views (row, g_value_get_int (value));
      break;
    case PROP_AGE:
      wh_video_model_row_set_age (row, g_value_get_int (value));
      break;
    case PROP_VTIME:
      wh_video_model_row_set_vtime (row, g_value_get_int (value));
      break;
    case PROP_RENDERER:
      wh_video_model_row_set_renderer (row, g_value_get_object (value));
      break;
    case PROP_THUMBNAIL:
      wh_video_model_row_set_thumbnail (row, 
					g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
wh_video_model_row_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (wh_video_model_row_parent_class)->dispose)
    G_OBJECT_CLASS (wh_video_model_row_parent_class)->dispose (object);
}

static void
wh_video_model_row_finalize (GObject *object)
{
  G_OBJECT_CLASS (wh_video_model_row_parent_class)->finalize (object);
}

static void
wh_video_model_row_class_init (WHVideoModelRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (WHVideoModelRowPrivate));

  object_class->get_property = wh_video_model_row_get_property;
  object_class->set_property = wh_video_model_row_set_property;
  object_class->dispose = wh_video_model_row_dispose;
  object_class->finalize = wh_video_model_row_finalize;

  g_object_class_install_property 
    (object_class,
     PROP_PATH,
     g_param_spec_string ("path",
			  "Path",
			  "path to rows video file",
			  NULL,
			  G_PARAM_READWRITE));

  g_object_class_install_property 
    (object_class,
     PROP_TITLE,
     g_param_spec_string ("title",
			  "title",
			  "Title of row entry",
			  NULL,
			  G_PARAM_READWRITE));

  g_object_class_install_property 
    (object_class,
     PROP_SERIES,
     g_param_spec_string ("series",
			  "series",
			  "Series",
			  NULL,
			  G_PARAM_READWRITE));

  g_object_class_install_property 
    (object_class,
     PROP_EPISODE,
     g_param_spec_string ("episode",
			  "episide",
			  "Episode",
			  NULL,
			  G_PARAM_READWRITE));

  g_object_class_install_property 
    (object_class,
     PROP_N_VIEWS,
     g_param_spec_int ("n-views",
		       "n-views",
		       "Numberof times video file has been watched",
		       0, G_MAXINT,
		       0,
		       G_PARAM_READWRITE));

  g_object_class_install_property 
    (object_class,
     PROP_AGE,
     g_param_spec_int ("age",
		       "Age",
		       "Age in seconds",
		       0, G_MAXINT,
		       0,
		       G_PARAM_READWRITE));

  g_object_class_install_property 
    (object_class,
     PROP_VTIME,
     g_param_spec_int ("last-viewed-time",
		       "Last-Viewed-Time",
		       "When file was last viewed",
		       0, G_MAXINT,
		       0,
		       G_PARAM_READWRITE));


  g_object_class_install_property 
    (object_class,
     PROP_RENDERER,
     g_param_spec_object ("renderer",
			  "Renderer",
			  "Renderer Object used to paint the row",
			  WH_TYPE_VIDEO_ROW_RENDERER,
			  G_PARAM_READWRITE));

  g_object_class_install_property 
    (object_class,
     PROP_THUMBNAIL,
     g_param_spec_object ("thumbnail",
			  "Thumbnail",
			  "Thumbnail image of video file",
			  GDK_TYPE_PIXBUF,
			  G_PARAM_READWRITE));

}

static void
wh_video_model_row_init (WHVideoModelRow *self)
{
}

WHVideoModelRow*
wh_video_model_row_new (void)
{
  return g_object_new (WH_TYPE_VIDEO_MODEL_ROW, NULL);
}

G_CONST_RETURN gchar*
wh_video_model_row_get_path (WHVideoModelRow *row)
{
  WHVideoModelRowPrivate *priv = VIDEO_MODEL_ROW_PRIVATE(row);

  return priv->path;
}

void
wh_video_model_row_set_path (WHVideoModelRow *row, const gchar *path)
{
  WHVideoModelRowPrivate *priv = VIDEO_MODEL_ROW_PRIVATE(row);

  g_object_ref (row);

  g_free (priv->path);
  
  if (path && path[0] != '\0')
    priv->path = g_strdup(path);

  g_object_notify (G_OBJECT (row), "path");
  g_object_unref (row);
}

G_CONST_RETURN gchar*
wh_video_model_row_get_title (WHVideoModelRow *row)
{
  WHVideoModelRowPrivate *priv = VIDEO_MODEL_ROW_PRIVATE(row);

  return priv->title;
}

void
wh_video_model_row_set_title (WHVideoModelRow *row, const gchar *title)
{
  WHVideoModelRowPrivate *priv = VIDEO_MODEL_ROW_PRIVATE(row);

  g_object_ref (row);

  g_free (priv->title);
  priv->title = NULL;

  if (title && title[0] != '\0')
    priv->title = g_strdup(title);

  g_object_notify (G_OBJECT (row), "title");
  g_object_unref (row);
}

void
wh_video_model_row_set_extended_info (WHVideoModelRow *row,
				      const gchar     *series,
				      const gchar     *episode)
{
  WHVideoModelRowPrivate *priv = VIDEO_MODEL_ROW_PRIVATE(row);

  g_object_ref (row);

  g_free (priv->series);
  priv->series = NULL;
  
  if (series && series[0] != '\0')
    priv->series = g_strdup(series);

  g_object_notify (G_OBJECT (row), "series");

  g_free (priv->episode);
  priv->episode = NULL;
  
  if (episode && episode[0] != '\0')
    priv->episode = g_strdup(episode);

  g_object_notify (G_OBJECT (row), "episode");

  g_object_unref (row);
}

void
wh_video_model_row_get_extended_info (WHVideoModelRow *row,
				      gchar          **series,
				      gchar          **episode)
{
  WHVideoModelRowPrivate *priv = VIDEO_MODEL_ROW_PRIVATE(row);

  if (priv->series)
    *series = g_strdup (priv->series);
  else
    *series = NULL;

  if (priv->episode)
    *episode = g_strdup (priv->episode);
  else
    *episode = NULL;
}


gint
wh_video_model_row_get_age (WHVideoModelRow *row)
{
  WHVideoModelRowPrivate *priv = VIDEO_MODEL_ROW_PRIVATE(row);

  return priv->age;
}

void
wh_video_model_row_set_age (WHVideoModelRow *row, gint age)
{
  WHVideoModelRowPrivate *priv = VIDEO_MODEL_ROW_PRIVATE(row);

  g_object_ref (row);

  priv->age = age;

  g_object_notify (G_OBJECT (row), "age");
  g_object_unref (row);
}

gint
wh_video_model_row_get_vtime (WHVideoModelRow *row)
{
  WHVideoModelRowPrivate *priv = VIDEO_MODEL_ROW_PRIVATE(row);

  return priv->vtime;
}

void
wh_video_model_row_set_vtime (WHVideoModelRow *row, gint vtime)
{
  WHVideoModelRowPrivate *priv = VIDEO_MODEL_ROW_PRIVATE(row);

  g_object_ref (row);

  priv->vtime = vtime;

  g_object_notify (G_OBJECT (row), "last-viewed-time");
  g_object_unref (row);
}

gint
wh_video_model_row_get_n_views (WHVideoModelRow *row)
{
  WHVideoModelRowPrivate *priv = VIDEO_MODEL_ROW_PRIVATE(row);

  return priv->n_views;
}

void
wh_video_model_row_set_renderer (WHVideoModelRow    *row, 
				 WHVideoRowRenderer *renderer)
{
  WHVideoModelRowPrivate *priv = VIDEO_MODEL_ROW_PRIVATE(row);

  g_object_ref (row);

  if (priv->renderer)
    g_object_unref (priv->renderer);

  priv->renderer = renderer;

  if (priv->renderer)
    g_object_ref (priv->renderer);

  g_object_notify (G_OBJECT (row), "renderer");
  g_object_unref (row);
}

WHVideoRowRenderer*
wh_video_model_row_get_renderer (WHVideoModelRow    *row)
{
  WHVideoModelRowPrivate *priv = VIDEO_MODEL_ROW_PRIVATE(row);

  return priv->renderer;
}

void
wh_video_model_row_set_n_views (WHVideoModelRow *row, gint n_views)
{
  WHVideoModelRowPrivate *priv = VIDEO_MODEL_ROW_PRIVATE(row);

  g_object_ref (row);

  priv->n_views = n_views;

  g_object_notify (G_OBJECT (row), "n-views");
  g_object_unref (row);
}

GdkPixbuf*
wh_video_model_row_get_thumbnail (WHVideoModelRow *row)
{
  WHVideoModelRowPrivate *priv = VIDEO_MODEL_ROW_PRIVATE(row);

  return priv->thumbnail;
}

void
wh_video_model_row_set_thumbnail (WHVideoModelRow *row,
				  GdkPixbuf       *pixbuf)
{
  WHVideoModelRowPrivate *priv = VIDEO_MODEL_ROW_PRIVATE(row);

  g_object_ref (row);

  if (priv->thumbnail)
    g_object_unref (priv->thumbnail);

  priv->thumbnail = pixbuf;
  g_object_ref (pixbuf);

  g_object_notify (G_OBJECT (row), "thumbnail");
  g_object_unref (row);
}
