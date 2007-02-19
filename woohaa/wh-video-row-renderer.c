#include "wh-video-row-renderer.h"
#include "wh-video-model.h"
#include "wh-video-model-row.h"
#include "util.h"

G_DEFINE_TYPE (WHVideoRowRenderer, wh_video_row_renderer, CLUTTER_TYPE_ACTOR);

#define VIDEO_ROW_RENDERER_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), WH_TYPE_VIDEO_ROW_RENDERER, WHVideoRowRendererPrivate))

typedef struct _WHVideoRowRendererPrivate WHVideoRowRendererPrivate;

struct _WHVideoRowRendererPrivate
{
  WHVideoModelRow *row;
  ClutterActor    *container;
  ClutterActor    *thumbnail;
  ClutterActor    *title_label;
  gint             width, height;
};

enum
{
  PROP_0,
  PROP_ROW
};

static ClutterActor *_default_video_thumbnail = NULL;

static void
wh_video_row_renderer_get_property (GObject *object, guint property_id,
				    GValue *value, GParamSpec *pspec)
{
  WHVideoRowRenderer        *row = WH_VIDEO_ROW_RENDERER(object);
  WHVideoRowRendererPrivate *priv;  
  
  priv = VIDEO_ROW_RENDERER_PRIVATE(row);

  switch (property_id) 
    {
    case PROP_ROW:
      g_value_set_object (value, priv->row);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
wh_video_row_renderer_set_property (GObject *object, guint property_id,
				    const GValue *value, GParamSpec *pspec)
{
  WHVideoRowRenderer        *row = WH_VIDEO_ROW_RENDERER(object);
  WHVideoRowRendererPrivate *priv;  
  
  priv = VIDEO_ROW_RENDERER_PRIVATE(row);

  switch (property_id) 
    {
    case PROP_ROW:
      if (priv->row)
	g_object_unref(priv->row);
      priv->row = g_value_get_object (value);
      g_object_ref(priv->row);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
wh_video_row_renderer_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (wh_video_row_renderer_parent_class)->dispose)
    G_OBJECT_CLASS (wh_video_row_renderer_parent_class)->dispose (object);
}

static void
wh_video_row_renderer_finalize (GObject *object)
{
  G_OBJECT_CLASS (wh_video_row_renderer_parent_class)->finalize (object);
}

static void
wh_video_row_renderer_request_coords (ClutterActor    *self,
				      ClutterActorBox *box)
{
  WHVideoRowRenderer        *row = WH_VIDEO_ROW_RENDERER(self);
  WHVideoRowRendererPrivate *priv;  
  
  priv = VIDEO_ROW_RENDERER_PRIVATE(row);

  if ( ((box->x2 - box->x1) != priv->width) 
       || ((box->y2 - box->y1) != priv->height))
    {
#define PAD 10

      gint  w,h;
      gchar font_desc[32];

      /* Keep a simple cache to avoid setting fonts up too much */
      w = priv->width  = (box->x2 - box->x1);
      h = priv->height = (box->y2 - box->y1);

      clutter_actor_set_position (priv->thumbnail, 0, 0);
      clutter_actor_set_size (priv->thumbnail, h, h);

      g_snprintf(font_desc, 32, "Sans Bold %ipx", h/3); 

      clutter_label_set_text (CLUTTER_LABEL(priv->title_label),
			      wh_video_model_row_get_title (priv->row));
      clutter_label_set_font_name (CLUTTER_LABEL(priv->title_label), 
				   font_desc); 
      clutter_label_set_line_wrap (CLUTTER_LABEL(priv->title_label), FALSE);
      clutter_label_set_ellipsize  (CLUTTER_LABEL(priv->title_label), 
				    PANGO_ELLIPSIZE_END);

      clutter_actor_set_width (priv->title_label, w - (h + (2*PAD)));
      clutter_actor_set_position (priv->title_label, h + PAD, PAD); 
    }
}

static void
wh_video_row_renderer_paint (ClutterActor *actor)
{
  WHVideoRowRenderer        *row = WH_VIDEO_ROW_RENDERER(actor);
  WHVideoRowRendererPrivate *priv;  
  
  priv = VIDEO_ROW_RENDERER_PRIVATE(row);

  if (priv->width == 0 || priv->height ==0)
    return;

  glPushMatrix();

  clutter_actor_paint (CLUTTER_ACTOR(priv->container));

  glPopMatrix();
}

static void
wh_video_row_renderer_class_init (WHVideoRowRendererClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass   *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (WHVideoRowRendererPrivate));

  object_class->get_property = wh_video_row_renderer_get_property;
  object_class->set_property = wh_video_row_renderer_set_property;
  object_class->dispose      = wh_video_row_renderer_dispose;
  object_class->finalize     = wh_video_row_renderer_finalize;

  actor_class->paint          = wh_video_row_renderer_paint;
  actor_class->request_coords = wh_video_row_renderer_request_coords;
  /* 
   *  actor_class->realize    = wh_video_row_renderer__realize;
   *  actor_class->unrealize  = parent_class->unrealize;
  */  

  g_object_class_install_property 
    (object_class,
     PROP_ROW,
     g_param_spec_object ("row",
			  "Row",
			  "Row to render",
			  WH_TYPE_VIDEO_MODEL_ROW,
			  G_PARAM_CONSTRUCT|G_PARAM_READWRITE));
}

static void
wh_video_row_renderer_init (WHVideoRowRenderer *self)
{
  WHVideoRowRendererPrivate *priv;  
  
  priv = VIDEO_ROW_RENDERER_PRIVATE(self);

  if (_default_video_thumbnail == NULL)
    _default_video_thumbnail = util_actor_from_file ("video.png", -1, -1);

  if (_default_video_thumbnail == NULL)
    g_error ("Unable to load video.png");

  /* FIXME: have below set on realize/unrealize */

  priv->thumbnail 
    = clutter_clone_texture_new (CLUTTER_TEXTURE(_default_video_thumbnail));

  priv->title_label = clutter_label_new();

  priv->container = clutter_group_new();
  clutter_actor_set_parent (priv->container, CLUTTER_ACTOR(self));

  clutter_group_add_many (CLUTTER_GROUP(priv->container), 
			  priv->thumbnail,
			  priv->title_label,
			  NULL);

  clutter_actor_show_all (priv->container);
}

WHVideoRowRenderer*
wh_video_row_renderer_new (WHVideoModelRow *row)
{
  return g_object_new (WH_TYPE_VIDEO_ROW_RENDERER, "row", row, NULL);
}

