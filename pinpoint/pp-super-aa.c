/* pp-super-aa.c */

#include "pp-super-aa.h"

G_DEFINE_TYPE (PPSuperAA, pp_super_aa, MX_TYPE_OFFSCREEN)

#define SUPER_AA_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), PP_TYPE_SUPER_AA, PPSuperAAPrivate))

struct _PPSuperAAPrivate
{
  gfloat        x_res;
  gfloat        y_res;
};

enum
{
  PROP_0,

  PROP_X_RES,
  PROP_Y_RES
};

static void
pp_super_aa_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  PPSuperAA *self = PP_SUPER_AA (object);

  switch (property_id)
    {
    case PROP_X_RES:
      g_value_set_float (value, self->priv->x_res);
      break;

    case PROP_Y_RES:
      g_value_set_float (value, self->priv->y_res);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
pp_super_aa_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  PPSuperAA *self = PP_SUPER_AA (object);

  switch (property_id)
    {
    case PROP_X_RES:
      self->priv->x_res = g_value_get_float (value);
      clutter_actor_queue_redraw (CLUTTER_ACTOR (object));
      break;

    case PROP_Y_RES:
      self->priv->y_res = g_value_get_float (value);
      clutter_actor_queue_redraw (CLUTTER_ACTOR (object));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
pp_super_aa_dispose (GObject *object)
{
  G_OBJECT_CLASS (pp_super_aa_parent_class)->dispose (object);
}

static void
pp_super_aa_finalize (GObject *object)
{
  G_OBJECT_CLASS (pp_super_aa_parent_class)->finalize (object);
}

static void
pp_super_aa_paint (ClutterActor *actor)
{
  CoglHandle texture;
  gfloat width, height;

  PPSuperAAPrivate *priv = PP_SUPER_AA (actor)->priv;

  clutter_actor_get_size (actor, &width, &height);
  texture = clutter_texture_get_cogl_texture (CLUTTER_TEXTURE (actor));

  if (!texture ||
      (cogl_texture_get_width (texture) != (guint)(width * priv->x_res)) ||
      (cogl_texture_get_height (texture) != (guint)(height * priv->y_res)))
    {
      texture = cogl_texture_new_with_size ((guint)(width * priv->x_res),
                                            (guint)(height * priv->y_res),
                                            COGL_TEXTURE_NO_SLICING,
                                            COGL_PIXEL_FORMAT_RGBA_8888_PRE);
      clutter_texture_set_cogl_texture (CLUTTER_TEXTURE (actor), texture);
      cogl_handle_unref (texture);
    }

  CLUTTER_ACTOR_CLASS (pp_super_aa_parent_class)->paint (actor);
}

static void
pp_super_aa_paint_child (MxOffscreen *offscreen)
{
  PPSuperAAPrivate *priv = PP_SUPER_AA (offscreen)->priv;

  cogl_scale (priv->x_res, priv->y_res, 1.0);

  MX_OFFSCREEN_CLASS (pp_super_aa_parent_class)->paint_child (offscreen);
}

static void
pp_super_aa_class_init (PPSuperAAClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  MxOffscreenClass *offscreen_class = MX_OFFSCREEN_CLASS (klass);

  g_type_class_add_private (klass, sizeof (PPSuperAAPrivate));

  object_class->get_property = pp_super_aa_get_property;
  object_class->set_property = pp_super_aa_set_property;
  object_class->dispose = pp_super_aa_dispose;
  object_class->finalize = pp_super_aa_finalize;

  actor_class->paint = pp_super_aa_paint;

  offscreen_class->paint_child = pp_super_aa_paint_child;
}

static void
pp_super_aa_init (PPSuperAA *self)
{
  self->priv = SUPER_AA_PRIVATE (self);
  clutter_texture_set_sync_size (CLUTTER_TEXTURE (self), FALSE);
}

ClutterActor *
pp_super_aa_new (void)
{
  return g_object_new (PP_TYPE_SUPER_AA, NULL);
}

void
pp_super_aa_set_resolution (PPSuperAA *aa, gfloat x_res, gfloat y_res)
{
  aa->priv->x_res = x_res;
  aa->priv->y_res = y_res;
  clutter_actor_queue_redraw (CLUTTER_ACTOR (aa));
}

void
pp_super_aa_get_resolution (PPSuperAA *aa, gfloat *x_res, gfloat *y_res)
{
  if (x_res)
    *x_res = aa->priv->x_res;
  if (y_res)
    *y_res = aa->priv->y_res;
}

