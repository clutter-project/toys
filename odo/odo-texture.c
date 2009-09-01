/* odo-texture.c */

#include "odo-texture.h"

G_DEFINE_TYPE (OdoTexture, odo_texture, CLUTTER_TYPE_ACTOR)

#define TEXTURE_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), ODO_TYPE_TEXTURE, OdoTexturePrivate))

struct _OdoTexturePrivate
{
  gint                tiles_x;
  gint                tiles_y;
  OdoTextureCallback  callback;
  gpointer            user_data;

  CoglHandle          vbo;
  gint                n_indices;
  CoglHandle         *indices;
  CoglHandle         *bf_indices;
  CoglTextureVertex  *vertices;

  ClutterTexture     *front_face;
  ClutterTexture     *back_face;

  gboolean            dirty;
};

enum
{
  PROP_0,

  PROP_TILES_X,
  PROP_TILES_Y,
  PROP_FRONT_FACE,
  PROP_BACK_FACE
};

static void
odo_texture_get_property (GObject *object, guint property_id,
                          GValue *value, GParamSpec *pspec)
{
  OdoTexturePrivate *priv = ODO_TEXTURE (object)->priv;

  switch (property_id)
    {
    case PROP_TILES_X:
      g_value_set_int (value, priv->tiles_x);
      break;

    case PROP_TILES_Y:
      g_value_set_int (value, priv->tiles_y);
      break;

    case PROP_FRONT_FACE:
      g_value_set_object (value, priv->front_face);
      break;

    case PROP_BACK_FACE:
      g_value_set_object (value, priv->back_face);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
odo_texture_set_property (GObject *object, guint property_id,
                          const GValue *value, GParamSpec *pspec)
{
  OdoTexture *texture = ODO_TEXTURE (object);
  OdoTexturePrivate *priv = texture->priv;

  switch (property_id)
    {
    case PROP_TILES_X:
      odo_texture_set_resolution (texture,
                                  g_value_get_int (value),
                                  priv->tiles_y);
      break;

    case PROP_TILES_Y:
      odo_texture_set_resolution (texture,
                                  priv->tiles_x,
                                  g_value_get_int (value));
      break;

    case PROP_FRONT_FACE:
      odo_texture_set_textures (texture,
                                g_value_get_object (value),
                                priv->back_face);
      break;

    case PROP_BACK_FACE:
      odo_texture_set_textures (texture,
                                priv->front_face,
                                g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
odo_texture_free_arrays (OdoTexture *self)
{
  OdoTexturePrivate *priv = self->priv;

  if (priv->vbo)
    {
      cogl_handle_unref (priv->vbo);
      priv->vbo = NULL;
    }

  if (priv->indices)
    {
      cogl_handle_unref (priv->indices);
      priv->indices = NULL;
    }

  g_free (priv->vertices);
  priv->vertices = NULL;
}

static void
odo_texture_dispose (GObject *object)
{
  OdoTexture *self = ODO_TEXTURE (object);
  OdoTexturePrivate *priv = self->priv;

  odo_texture_free_arrays (self);

  if (priv->front_face)
    {
      g_object_unref (priv->front_face);
      priv->front_face = NULL;
    }

  if (priv->back_face)
    {
      g_object_unref (priv->back_face);
      priv->back_face = NULL;
    }

  G_OBJECT_CLASS (odo_texture_parent_class)->dispose (object);
}

static void
odo_texture_finalize (GObject *object)
{
  G_OBJECT_CLASS (odo_texture_parent_class)->finalize (object);
}

static void
odo_texture_paint (ClutterActor *actor)
{
  gint i, j;
  CoglHandle material;
  gboolean depth, cull;

  OdoTexture *self = ODO_TEXTURE (actor);
  OdoTexturePrivate *priv = self->priv;

  if (priv->dirty)
    {
      guint opacity;
      gfloat width, height;
      ClutterActorBox box;

      opacity = clutter_actor_get_paint_opacity (actor);
      clutter_actor_get_allocation_box (actor, &box);
      width = box.x2 - box.x1;
      height = box.y2 - box.y1;

      for (i = 0; i <= priv->tiles_y; i++)
        {
          for (j = 0; j <= priv->tiles_x; j++)
            {
              CoglTextureVertex *vertex =
                &priv->vertices[(i * (priv->tiles_x + 1)) + j];

              vertex->tx = j/(gfloat)priv->tiles_x;
              vertex->ty = i/(gfloat)priv->tiles_y;
              vertex->x = width * vertex->tx;
              vertex->y = height * vertex->ty;
              vertex->z = 0;
              cogl_color_set_from_4ub (&vertex->color,
                                       0xff, 0xff, 0xff, opacity);

              if (priv->callback)
                priv->callback (self, vertex, width, height, priv->user_data);
            }
        }

      /* We add all three attributes again, although in an ideal case,
       * we'd add only those that had changed. Because we provide the
       * ability to change each, unless we had a 'changed' gboolean * in
       * the function prototype, we have to upload all of it.
       */
      cogl_vertex_buffer_add (priv->vbo,
                              "gl_Vertex",
                              3,
                              COGL_ATTRIBUTE_TYPE_FLOAT,
                              FALSE,
                              sizeof (CoglTextureVertex),
                              &priv->vertices->x);
      cogl_vertex_buffer_add (priv->vbo,
                              "gl_MultiTexCoord0",
                              2,
                              COGL_ATTRIBUTE_TYPE_FLOAT,
                              FALSE,
                              sizeof (CoglTextureVertex),
                              &priv->vertices->tx);
      cogl_vertex_buffer_add (priv->vbo,
                              "gl_Color",
                              4,
                              COGL_ATTRIBUTE_TYPE_UNSIGNED_BYTE,
                              FALSE,
                              sizeof (CoglTextureVertex),
                              &priv->vertices->color);
      cogl_vertex_buffer_submit (priv->vbo);

      priv->dirty = FALSE;
    }

  depth = cogl_get_depth_test_enabled ();
  if (!depth)
    cogl_set_depth_test_enabled (TRUE);

  cull = cogl_get_backface_culling_enabled ();
  if (priv->back_face && !cull)
    cogl_set_backface_culling_enabled (TRUE);
  else if (!priv->back_face && cull)
    cogl_set_backface_culling_enabled (FALSE);

  if (priv->front_face)
    {
      material = clutter_texture_get_cogl_material (priv->front_face);
      cogl_set_source (material);
      cogl_vertex_buffer_draw_elements (priv->vbo,
                                        COGL_VERTICES_MODE_TRIANGLE_STRIP,
                                        priv->indices,
                                        0,
                                        (priv->tiles_x + 1) *
                                        (priv->tiles_y + 1),
                                        0,
                                        priv->n_indices);
    }

  if (priv->back_face)
    {
      material = clutter_texture_get_cogl_material (priv->back_face);
      cogl_set_source (material);
      cogl_vertex_buffer_draw_elements (priv->vbo,
                                        COGL_VERTICES_MODE_TRIANGLE_STRIP,
                                        priv->bf_indices,
                                        0,
                                        (priv->tiles_x + 1) *
                                        (priv->tiles_y + 1),
                                        0,
                                        priv->n_indices);
    }

  if (!depth)
    cogl_set_depth_test_enabled (FALSE);
  if (priv->back_face && !cull)
    cogl_set_backface_culling_enabled (FALSE);
  else if (!priv->back_face && cull)
    cogl_set_backface_culling_enabled (TRUE);
}

static void
odo_texture_get_preferred_width (ClutterActor *actor,
                                 gfloat        for_height,
                                 gfloat       *min_width_p,
                                 gfloat       *natural_width_p)
{
  ClutterActor *proxy;
  OdoTexturePrivate *priv = ODO_TEXTURE (actor)->priv;

  if (priv->front_face)
    proxy = CLUTTER_ACTOR (priv->front_face);
  else if (priv->back_face)
    proxy = CLUTTER_ACTOR (priv->back_face);
  else
    {
      if (min_width_p)
        *min_width_p = 0;
      if (natural_width_p)
        *natural_width_p = 0;

      return;
    }

  clutter_actor_get_preferred_width (proxy,
                                     for_height,
                                     min_width_p,
                                     natural_width_p);
}

static void
odo_texture_get_preferred_height (ClutterActor *actor,
                                  gfloat        for_width,
                                  gfloat       *min_height_p,
                                  gfloat       *natural_height_p)
{
  ClutterActor *proxy;
  OdoTexturePrivate *priv = ODO_TEXTURE (actor)->priv;

  if (priv->front_face)
    proxy = CLUTTER_ACTOR (priv->front_face);
  else if (priv->back_face)
    proxy = CLUTTER_ACTOR (priv->back_face);
  else
    {
      if (min_height_p)
        *min_height_p = 0;
      if (natural_height_p)
        *natural_height_p = 0;

      return;
    }

  clutter_actor_get_preferred_height (proxy,
                                      for_width,
                                      min_height_p,
                                      natural_height_p);
}
/*
static void
odo_texture_allocate (ClutterActor           *actor,
                      const ClutterActorBox  *box,
                      ClutterAllocationFlags  flags)
{
}*/

static void
odo_texture_class_init (OdoTextureClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (OdoTexturePrivate));

  object_class->get_property = odo_texture_get_property;
  object_class->set_property = odo_texture_set_property;
  object_class->dispose = odo_texture_dispose;
  object_class->finalize = odo_texture_finalize;

  actor_class->get_preferred_width = odo_texture_get_preferred_width;
  actor_class->get_preferred_height = odo_texture_get_preferred_height;
  /*actor_class->allocate = odo_texture_allocate;*/
  actor_class->paint = odo_texture_paint;

  g_object_class_install_property (object_class,
                                   PROP_TILES_X,
                                   g_param_spec_int ("tiles-x",
                                                     "Horizontal tiles",
                                                     "Amount of horizontal "
                                                     "tiles to split the "
                                                     "texture into.",
                                                     1, G_MAXINT, 32,
                                                     G_PARAM_READWRITE |
                                                     G_PARAM_STATIC_NAME |
                                                     G_PARAM_STATIC_NICK |
                                                     G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_TILES_Y,
                                   g_param_spec_int ("tiles-y",
                                                     "Vertical tiles",
                                                     "Amount of vertical "
                                                     "tiles to split the "
                                                     "texture into.",
                                                     1, G_MAXINT, 32,
                                                     G_PARAM_READWRITE |
                                                     G_PARAM_STATIC_NAME |
                                                     G_PARAM_STATIC_NICK |
                                                     G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_FRONT_FACE,
                                   g_param_spec_object ("front-face",
                                                        "Front-face",
                                                        "Front-face texture.",
                                                        CLUTTER_TYPE_TEXTURE,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_BACK_FACE,
                                   g_param_spec_object ("back-face",
                                                        "Back-face",
                                                        "Back-face texture.",
                                                        CLUTTER_TYPE_TEXTURE,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_BLURB));
}

static void
odo_texture_init_arrays (OdoTexture *self)
{
  GLushort *idx, *bf_idx;
  gint x, y, direction;
  GLushort *static_indices, *static_bf_indices;
  OdoTexturePrivate *priv = self->priv;

  odo_texture_free_arrays (self);

  priv->n_indices = (2 + 2 * priv->tiles_x) *
                    priv->tiles_y +
                    (priv->tiles_y - 1);
  static_indices = g_new (GLushort, priv->n_indices);
  static_bf_indices = g_new (GLushort, priv->n_indices);

#define MESH_INDEX(X, Y) (Y) * (priv->tiles_x + 1) + (X)

  direction = 1;

  idx = static_indices;
  idx[0] = MESH_INDEX (0, 0);
  idx[1] = MESH_INDEX (0, 1);
  idx += 2;

  bf_idx = static_bf_indices;
  bf_idx[0] = MESH_INDEX (priv->tiles_x, 0);
  bf_idx[1] = MESH_INDEX (priv->tiles_x, 1);
  bf_idx += 2;

  for (y = 0; y < priv->tiles_y; y++)
    {
      for (x = 0; x < priv->tiles_x; x++)
        {
          /* Add 2 triangles for a quad */
          if (direction)
            {
              idx[0] = MESH_INDEX (x + 1, y);
              idx[1] = MESH_INDEX (x + 1, y + 1);
              bf_idx[0] = MESH_INDEX (priv->tiles_x - (x + 1), y);
              bf_idx[1] = MESH_INDEX (priv->tiles_x - (x + 1), y + 1);
            }
          else
            {
              idx[0] = MESH_INDEX (priv->tiles_x - x - 1, y);
              idx[1] = MESH_INDEX (priv->tiles_x - x - 1, y + 1);
              bf_idx[0] = MESH_INDEX (x + 1, y);
              bf_idx[1] = MESH_INDEX (x + 1, y + 1);
            }
          idx += 2;
          bf_idx += 2;
        }

      /* Link rows together to draw in one call */
      if (y == (priv->tiles_y - 1))
        break;

      if (direction)
        {
          idx[0] = MESH_INDEX (priv->tiles_x, y + 1);
          idx[1] = MESH_INDEX (priv->tiles_x, y + 1);
          idx[2] = MESH_INDEX (priv->tiles_x, y + 2);
          bf_idx[0] = MESH_INDEX (0, y + 1);
          bf_idx[1] = MESH_INDEX (0, y + 1);
          bf_idx[2] = MESH_INDEX (0, y + 2);
        }
      else
        {
          idx[0] = MESH_INDEX (0, y + 1);
          idx[1] = MESH_INDEX (0, y + 1);
          idx[2] = MESH_INDEX (0, y + 2);
          bf_idx[0] = MESH_INDEX (priv->tiles_x, y + 1);
          bf_idx[1] = MESH_INDEX (priv->tiles_x, y + 1);
          bf_idx[2] = MESH_INDEX (priv->tiles_x, y + 2);
        }

      idx += 3;
      bf_idx += 3;
      direction = !direction;
    }

  priv->indices =
    cogl_vertex_buffer_indices_new (COGL_INDICES_TYPE_UNSIGNED_SHORT,
                                    static_indices,
                                    priv->n_indices);
  priv->bf_indices =
    cogl_vertex_buffer_indices_new (COGL_INDICES_TYPE_UNSIGNED_SHORT,
                                    static_bf_indices,
                                    priv->n_indices);
  g_free (static_indices);
  g_free (static_bf_indices);

  priv->vertices = g_new (CoglTextureVertex,
                          (priv->tiles_x + 1) * (priv->tiles_y + 1));

  priv->vbo = cogl_vertex_buffer_new ((priv->tiles_x + 1) *
                                      (priv->tiles_y + 1));
}

static void
odo_texture_init (OdoTexture *self)
{
  OdoTexturePrivate *priv = self->priv = TEXTURE_PRIVATE (self);

  priv->tiles_x = 32;
  priv->tiles_y = 32;
  odo_texture_init_arrays (self);
}

ClutterActor *
odo_texture_new (void)
{
  return g_object_new (ODO_TYPE_TEXTURE, NULL);
}

ClutterActor *
odo_texture_new_from_files (const gchar *front_face_filename,
                            const gchar *back_face_filename)
{
  ClutterTexture *front_face, *back_face;

  if (front_face_filename)
    front_face = g_object_new (CLUTTER_TYPE_TEXTURE,
                               "disable-slicing", TRUE,
                               "filename", front_face_filename,
                               NULL);
  else
    front_face = NULL;

  if (back_face_filename)
    back_face = g_object_new (CLUTTER_TYPE_TEXTURE,
                              "disable-slicing", TRUE,
                              "filename", back_face_filename,
                              NULL);
  else
    back_face = NULL;

  return odo_texture_new_with_textures (front_face, back_face);
}

ClutterActor *
odo_texture_new_with_textures (ClutterTexture *front_face,
                               ClutterTexture *back_face)
{
  return g_object_new (ODO_TYPE_TEXTURE,
                       "front-face", front_face,
                       "back-face", back_face,
                       NULL);
}

void
odo_texture_set_textures (OdoTexture     *texture,
                          ClutterTexture *front_face,
                          ClutterTexture *back_face)
{
  ClutterTexture *old_texture;
  OdoTexturePrivate *priv = texture->priv;

  old_texture = priv->front_face;
  priv->front_face = front_face ? g_object_ref_sink (front_face) : NULL;
  if (old_texture)
    g_object_unref (old_texture);

  old_texture = priv->back_face;
  priv->back_face = back_face ? g_object_ref_sink (back_face) : NULL;
  if (old_texture)
    g_object_unref (old_texture);

  clutter_actor_queue_redraw (CLUTTER_ACTOR (texture));
}

void
odo_texture_get_resolution (OdoTexture *texture,
                            gint       *tiles_x,
                            gint       *tiles_y)
{
  OdoTexturePrivate *priv = texture->priv;

  if (tiles_x)
    *tiles_x = priv->tiles_x;
  if (tiles_y)
    *tiles_y = priv->tiles_y;
}

void
odo_texture_set_resolution (OdoTexture *texture,
                            gint        tiles_x,
                            gint        tiles_y)
{
  OdoTexturePrivate *priv = texture->priv;
  gboolean changed = FALSE;

  g_return_if_fail ((tiles_x > 0) && (tiles_y > 0));

  if (priv->tiles_x != tiles_x)
    {
      priv->tiles_x = tiles_x;
      changed = TRUE;
      g_object_notify (G_OBJECT (texture), "tiles-x");
    }

  if (priv->tiles_y != tiles_y)
    {
      priv->tiles_y = tiles_y;
      changed = TRUE;
      g_object_notify (G_OBJECT (texture), "tiles-y");
    }

  if (changed)
    {
      odo_texture_init_arrays (texture);
      odo_texture_invalidate (texture);
    }
}

void
odo_texture_set_callback (OdoTexture         *texture,
                          OdoTextureCallback  callback,
                          gpointer            user_data)
{
  OdoTexturePrivate *priv = texture->priv;

  priv->callback = callback;
  priv->user_data = user_data;

  odo_texture_invalidate (texture);
}

void
odo_texture_invalidate (OdoTexture *texture)
{
  OdoTexturePrivate *priv = texture->priv;
  priv->dirty = TRUE;
  clutter_actor_queue_redraw (CLUTTER_ACTOR (texture));
}

