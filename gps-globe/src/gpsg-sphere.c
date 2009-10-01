/*
 * gps-globe - A little app showing your position on a globe
 * Copyright (C) 2009  Intel Corporation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <clutter/clutter.h>
#include <math.h>
#include <string.h>

#include "gpsg-sphere.h"
#include "gpsg-enum-types.h"
#include "gpsg-sphere-vertex-shader.h"

#define GPSG_SPHERE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GPSG_TYPE_SPHERE, GpsgSpherePrivate))

G_DEFINE_TYPE (GpsgSphere, gpsg_sphere, CLUTTER_TYPE_TEXTURE);

static void gpsg_sphere_paint (ClutterActor *self);
static void gpsg_sphere_dispose (GObject *self);

static void gpsg_sphere_set_property (GObject      *self,
				      guint         property_id,
				      const GValue *value,
				      GParamSpec   *pspec);
static void gpsg_sphere_get_property (GObject    *self,
				      guint       property_id,
				      GValue     *value,
				      GParamSpec *pspec);

#define GPSG_SPHERE_GOLDEN_RATIO      1.61803398874989  /* φ = (1+√5) ÷ 2 */
/* Amount to scale a vertex using the golden ratio so that it will
   have a radius of one. */
#define GPSG_SPHERE_NORM_ONE          0.525731112119134 /* = √(1 / (1 + φ²)) */
#define GPSG_SPHERE_NORM_GOLDEN_RATIO (GPSG_SPHERE_GOLDEN_RATIO \
                                       * GPSG_SPHERE_NORM_ONE)

struct _GpsgSpherePrivate
{
  guint depth;
  guint n_vertices, n_indices;
  CoglHandle vertices, indices;
  ClutterColor lines_color;
  GpsgSpherePaintFlags paint_flags;

  /* A shader program use to render the transition between a flat
     texture and a sphere */
  CoglHandle flat_program;
  /* Set to true if shaders aren't available or it won't compile */
  gboolean shader_failed;

  gint flatness_uniform;
  gint flat_width_uniform;
  gint flat_height_uniform;
  gint sphere_radius_uniform;

  gfloat flatness;
};

enum
{
  PROP_0,

  PROP_PAINT_FLAGS,
  PROP_LINES_COLOR,
  PROP_DEPTH,
  PROP_FLATNESS
};

static void
gpsg_sphere_class_init (GpsgSphereClass *klass)
{
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  actor_class->paint = gpsg_sphere_paint;

  object_class->dispose = gpsg_sphere_dispose;
  object_class->set_property = gpsg_sphere_set_property;
  object_class->get_property = gpsg_sphere_get_property;

  g_type_class_add_private (klass, sizeof (GpsgSpherePrivate));

  pspec = g_param_spec_uint ("depth", "Depth",
                             "The number of times to subdivide each "
                             "triangle. Higher numbers mean better looking "
                             "spheres that consume more resources.",
                             0, G_MAXUINT, 4,
                             G_PARAM_READWRITE
                             | G_PARAM_STATIC_NAME
                             | G_PARAM_STATIC_NICK
                             | G_PARAM_STATIC_BLURB);
  g_object_class_install_property (object_class, PROP_DEPTH, pspec);

  pspec = g_param_spec_float ("flatness", "Flatness",
                              "A value between 0.0 and 1.0. Zero paints the "
                              "texture normally and 1.0 paints it as a "
                              "sphere. Other values mix between the two.",
                              0.0f, 1.0f, 0.0f,
                              G_PARAM_READWRITE
                              | G_PARAM_STATIC_NAME
                              | G_PARAM_STATIC_NICK
                              | G_PARAM_STATIC_BLURB);
  g_object_class_install_property (object_class, PROP_FLATNESS, pspec);

  pspec = g_param_spec_flags ("paint-flags", "Paint flags",
                              "A set of flags describing what parts of the "
                              "sphere to paint.",
                              GPSG_TYPE_SPHERE_PAINT_FLAGS,
                              GPSG_SPHERE_PAINT_TEXTURE,
                              G_PARAM_READWRITE
                              | G_PARAM_STATIC_NAME
                              | G_PARAM_STATIC_NICK
                              | G_PARAM_STATIC_BLURB);
  g_object_class_install_property (object_class, PROP_PAINT_FLAGS, pspec);

  pspec = g_param_spec_boxed ("lines-color", "Lines color",
                              "Color to use when painting a wireframe of the "
                              "sphere.",
                              CLUTTER_TYPE_COLOR,
                              G_PARAM_READWRITE
                              | G_PARAM_STATIC_NAME
                              | G_PARAM_STATIC_NICK
                              | G_PARAM_STATIC_BLURB);
  g_object_class_install_property (object_class, PROP_LINES_COLOR, pspec);
}

static void
gpsg_sphere_init (GpsgSphere *self)
{
  GpsgSpherePrivate *priv;

  self->priv = priv = GPSG_SPHERE_GET_PRIVATE (self);

  priv->depth = 4;
  priv->vertices = COGL_INVALID_HANDLE;
  priv->indices = COGL_INVALID_HANDLE;
  priv->lines_color.alpha = 255;
  priv->paint_flags = GPSG_SPHERE_PAINT_TEXTURE;
}

static void
gpsg_sphere_set_property (GObject      *self,
			  guint         property_id,
			  const GValue *value,
			  GParamSpec   *pspec)
{
  GpsgSphere *sphere = GPSG_SPHERE (self);

  switch (property_id)
    {
    case PROP_DEPTH:
      gpsg_sphere_set_depth (sphere, g_value_get_uint (value));
      break;

    case PROP_FLATNESS:
      gpsg_sphere_set_flatness (sphere, g_value_get_float (value));
      break;

    case PROP_PAINT_FLAGS:
      gpsg_sphere_set_paint_flags (sphere, g_value_get_flags (value));
      break;

    case PROP_LINES_COLOR:
      gpsg_sphere_set_lines_color (sphere, clutter_value_get_color (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (self, property_id, pspec);
      break;
    }
}

static void
gpsg_sphere_get_property (GObject    *self,
			  guint       property_id,
			  GValue     *value,
			  GParamSpec *pspec)
{
  GpsgSphere *sphere = GPSG_SPHERE (self);

  switch (property_id)
    {
    case PROP_DEPTH:
      g_value_set_uint (value, gpsg_sphere_get_depth (sphere));
      break;

    case PROP_FLATNESS:
      g_value_set_float (value, gpsg_sphere_get_flatness (sphere));
      break;

    case PROP_PAINT_FLAGS:
      g_value_set_flags (value, gpsg_sphere_get_paint_flags (sphere));
      break;

    case PROP_LINES_COLOR:
      {
        ClutterColor color;
        gpsg_sphere_get_lines_color (sphere, &color);
        clutter_value_set_color (value, &color);
      }
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (self, property_id, pspec);
      break;
    }
}

ClutterActor *
gpsg_sphere_new (void)
{
  return g_object_new (GPSG_TYPE_SPHERE, NULL);
}

typedef struct _GpsgSphereStackEntry
{
  /* Index of the three edges that make up the triangle */
  guint16 e[3];
  /* The recursion depth that was needed to add this entry */
  guint8 depth;
  /* A bit for each edge. If set then use v1, otherwise v0 */
  guint8 direction;
} GpsgSphereStackEntry;

typedef struct _GpsgSphereVertex
{
  /* Vertex coordinates */
  gfloat x, y, z;
  /* Texture coordinates */
  gfloat tx, ty;
} GpsgSphereVertex;

typedef struct _GpsgSphereEdge
{
  /* Index of the two vertices that make up this edge */
  guint16 v0, v1;
  /* Index of two edges that subdivide this edge, or -1 if it hasn't
     been divided yet */
  gint16 e0, e1;
} GpsgSphereEdge;

/* Initial edges needed to make an icosahedron */
static const GpsgSphereEdge
gpsg_sphere_ico_edges[] =
{
  { 0, 2, -1, -1 }, { 0, 4, -1, -1 }, { 0, 6, -1, -1 }, { 0, 8, -1, -1 },
  { 0, 9, -1, -1 }, { 1, 3, -1, -1 }, { 1, 4, -1, -1 }, { 1, 6, -1, -1 },
  { 1, 10, -1, -1 }, { 1, 11, -1, -1 }, { 2, 5, -1, -1 }, { 2, 7, -1, -1 },
  { 2, 8, -1, -1 }, { 2, 9, -1, -1 }, { 3, 5, -1, -1 }, { 3, 7, -1, -1 },
  { 3, 10, -1, -1 }, { 3, 11, -1, -1 }, { 4, 6, -1, -1 }, { 4, 8, -1, -1 },
  { 4, 10, -1, -1 }, { 5, 7, -1, -1 }, { 5, 8, -1, -1 }, { 5, 10, -1, -1 },
  { 6, 9, -1, -1 }, { 6, 11, -1, -1 }, { 7, 9, -1, -1 }, { 7, 11, -1, -1 },
  { 8, 10, -1, -1 }, { 9, 11, -1, -1 }
};

/* Initial triangles needed to make an icosahedron with all the
   vertices in anti-clockwise order */
static const GpsgSphereStackEntry
gpsg_sphere_ico_stack_entries[] =
{
  { { 17, 9, 5 }, 0, 2 }, { { 8, 16, 5 }, 0, 6 }, { { 14, 16, 23 }, 0, 5 },
  { { 21, 15, 14 }, 0, 2 }, { { 27, 17, 15 }, 0, 2 }, { { 11, 21, 10 }, 0, 6 },
  { { 12, 10, 22 }, 0, 1 }, { { 22, 23, 28 }, 0, 5 }, { { 19, 28, 20 }, 0, 4 },
  { { 20, 8, 6 }, 0, 2 }, { { 18, 6, 7 }, 0, 3 }, { { 7, 9, 25 }, 0, 5 },
  { { 24, 25, 29 }, 0, 5 }, { { 29, 27, 26 }, 0, 2 }, { { 26, 11, 13 }, 0, 3 },
  { { 0, 4, 13 }, 0, 5 }, { { 4, 2, 24 }, 0, 1 }, { { 1, 18, 2 }, 0, 4 },
  { { 3, 19, 1 }, 0, 6 }, { { 12, 3, 0 }, 0, 2 }
};

/* This helper macro is just used to abbreviate getting a vertex out
   of the GArray */
#define VERT(x) g_array_index (vertices, GpsgSphereVertex, (x))

static void
gpsg_sphere_ensure_vertices (GpsgSphere *sphere)
{
  GpsgSpherePrivate *priv = sphere->priv;
  guint n_triangles;
  guint n_indices;
  guint n_edges;
  GpsgSphereStackEntry *stack, *stack_pos, *max_stack_pos;
  guint stack_size;
  GArray *vertices;
  GpsgSphereVertex *vertices_pos;
  guint16 *indices, *indices_pos;
  GpsgSphereEdge *edges, *edges_pos;
  int i;

  /* Don't do anything if we've already got the vertices */
  if (priv->vertices != COGL_INVALID_HANDLE)
    return;

  n_triangles = 20 * powf (4, priv->depth) + 0.5f;
  n_edges = 0;
  for (i = 0; i <= priv->depth; i++)
    n_edges += 30 * powf (4, i) + 0.5f;
  n_indices = n_triangles * 3;
  stack_size = priv->depth * 3 + 20;
  stack = g_new (GpsgSphereStackEntry, stack_size);
  indices_pos = indices = g_new (guint16, n_indices);
  vertices = g_array_new (FALSE, FALSE, sizeof (GpsgSphereVertex));
  edges = g_new (GpsgSphereEdge, n_edges + 100);

  /* Add the initial 12 vertices needed to make an icosahedron */
  g_array_set_size (vertices, 12);
  vertices_pos = &VERT (0);
  {
    int unit, magic;

    for (unit = -1; unit <= 1; unit += 2)
      for (magic = -1; magic <= 1; magic += 2)
        {
          vertices_pos->x = 0;
          vertices_pos->y = unit * GPSG_SPHERE_NORM_ONE;
          vertices_pos->z = magic * GPSG_SPHERE_NORM_GOLDEN_RATIO;
          vertices_pos++;
        }
    for (unit = -1; unit <= 1; unit += 2)
      for (magic = -1; magic <= 1; magic += 2)
        {
          vertices_pos->x = unit * GPSG_SPHERE_NORM_ONE;
          vertices_pos->y = magic * GPSG_SPHERE_NORM_GOLDEN_RATIO;
          vertices_pos->z = 0;
          vertices_pos++;
        }
    for (unit = -1; unit <= 1; unit += 2)
      for (magic = -1; magic <= 1; magic += 2)
        {
          vertices_pos->x = magic * GPSG_SPHERE_NORM_GOLDEN_RATIO;
          vertices_pos->y = 0;
          vertices_pos->z = unit * GPSG_SPHERE_NORM_ONE;
          vertices_pos++;
        }
  }

  /* Add the initial edges */
  memcpy (edges, gpsg_sphere_ico_edges, sizeof (gpsg_sphere_ico_edges));
  edges_pos = edges + G_N_ELEMENTS (gpsg_sphere_ico_edges);
  /* and stack entries */
  memcpy (stack, gpsg_sphere_ico_stack_entries,
          sizeof (gpsg_sphere_ico_stack_entries));
  stack_pos = stack + G_N_ELEMENTS (gpsg_sphere_ico_stack_entries);

  max_stack_pos = stack_pos;

  /* While the stack is not empty */
  while (stack_pos > stack)
    {
      /* Pop an entry off the stack */
      GpsgSphereStackEntry entry = *(--stack_pos);

      /* If we've reached the depth limit.. */
      if (entry.depth >= priv->depth)
        /* Add the triangle to the vertices */
        for (i = 0; i < 3; i++)
          *(indices_pos++) = ((entry.direction & (1 << i))
                              ? edges[entry.e[i]].v1
                              : edges[entry.e[i]].v0);
      else
        {
          /* If the stack is not empty then add four more triangles
             to split this one up */

          /* Split each edge if it is not already split */
          for (i = 0; i < 3; i++)
            if (edges[entry.e[i]].e0 == -1)
              {
                g_array_set_size (vertices, vertices->len + 1);
                vertices_pos = &VERT (vertices->len - 1);
                vertices_pos->x = (VERT (edges[entry.e[i]].v0).x
                                   + VERT (edges[entry.e[i]].v1).x) / 2.0;
                vertices_pos->y = (VERT (edges[entry.e[i]].v0).y
                                   + VERT (edges[entry.e[i]].v1).y) / 2.0;
                vertices_pos->z = (VERT (edges[entry.e[i]].v0).z
                                   + VERT (edges[entry.e[i]].v1).z) / 2.0;
                edges[entry.e[i]].e0 = edges_pos - edges;
                edges_pos->v0 = edges[entry.e[i]].v0;
                edges_pos->v1 = vertices->len - 1;
                edges_pos->e0 = -1;
                edges_pos->e1 = -1;
                edges_pos++;
                edges[entry.e[i]].e1 = edges_pos - edges;
                edges_pos->v0 = edges[entry.e[i]].v1;
                edges_pos->v1 = vertices->len - 1;
                edges_pos->e0 = -1;
                edges_pos->e1 = -1;
                edges_pos++;
              }

          /* Add each triangle */

          /* Top triangle */
          if ((entry.direction & 1))
            stack_pos->e[0] = edges[entry.e[0]].e1;
          else
            stack_pos->e[0] = edges[entry.e[0]].e0;
          stack_pos->e[1] = edges_pos - edges;
          edges_pos->v0 = edges[edges[entry.e[0]].e0].v1;
          edges_pos->v1 = edges[edges[entry.e[2]].e0].v1;
          if (edges_pos->v0 > edges_pos->v1)
            {
              gint t = edges_pos->v0;
              edges_pos->v0 = edges_pos->v1;
              edges_pos->v1 = t;
              stack_pos->direction = 6;
            }
          else
            stack_pos->direction = 4;
          edges_pos->e0 = -1;
          edges_pos->e1 = -1;
          edges_pos++;
          if ((entry.direction & 4))
            stack_pos->e[2] = edges[entry.e[2]].e0;
          else
            stack_pos->e[2] = edges[entry.e[2]].e1;
          stack_pos->depth = entry.depth + 1;
          stack_pos++;

          /* Bottom left triangle */
          if ((entry.direction & 1))
            stack_pos->e[0] = edges[entry.e[0]].e0;
          else
            stack_pos->e[0] = edges[entry.e[0]].e1;
          if ((entry.direction & 2))
            stack_pos->e[1] = edges[entry.e[1]].e1;
          else
            stack_pos->e[1] = edges[entry.e[1]].e0;
          stack_pos->e[2] = edges_pos - edges;
          edges_pos->v0 = edges[edges[entry.e[1]].e0].v1;
          edges_pos->v1 = edges[edges[entry.e[0]].e0].v1;
          if (edges_pos->v0 > edges_pos->v1)
            {
              gint t = edges_pos->v0;
              edges_pos->v0 = edges_pos->v1;
              edges_pos->v1 = t;
              stack_pos->direction = 5;
            }
          else
            stack_pos->direction = 1;
          edges_pos->e0 = -1;
          edges_pos->e1 = -1;
          edges_pos++;
          stack_pos->depth = entry.depth + 1;
          stack_pos++;

          /* Bottom right triangle */
          stack_pos->e[0] = edges_pos - edges;
          edges_pos->v0 = edges[edges[entry.e[2]].e0].v1;
          edges_pos->v1 = edges[edges[entry.e[1]].e0].v1;
          if (edges_pos->v0 > edges_pos->v1)
            {
              gint t = edges_pos->v0;
              edges_pos->v0 = edges_pos->v1;
              edges_pos->v1 = t;
              stack_pos->direction = 3;
            }
          else
            stack_pos->direction = 2;
          edges_pos->e0 = -1;
          edges_pos->e1 = -1;
          edges_pos++;
          if ((entry.direction & 2))
            stack_pos->e[1] = edges[entry.e[1]].e0;
          else
            stack_pos->e[1] = edges[entry.e[1]].e1;
          if ((entry.direction & 4))
            stack_pos->e[2] = edges[entry.e[2]].e1;
          else
            stack_pos->e[2] = edges[entry.e[2]].e0;
          stack_pos->depth = entry.depth + 1;
          stack_pos++;

          /* Middle triangle */
          stack_pos->e[0] = stack_pos[-1].e[0];
          stack_pos->e[1] = stack_pos[-3].e[1];
          stack_pos->e[2] = stack_pos[-2].e[2];
          stack_pos->depth = entry.depth + 1;
          stack_pos->direction = ((stack_pos[-1].direction & 1)
                                  | (stack_pos[-3].direction & 2)
                                  | (stack_pos[-2].direction & 4)) ^ 7;
          stack_pos++;

          /* This is just used for the assert below */
          if (stack_pos > max_stack_pos)
            max_stack_pos = stack_pos;
        }
    }

  /* Normalise every vertex. The initial 12 are already normalised */
  for (i = 12; i < vertices->len; i++)
    {
      gfloat length;

      vertices_pos = &VERT (i);

      length = sqrt (vertices_pos->x * vertices_pos->x
                     + vertices_pos->y * vertices_pos->y
                     + vertices_pos->z * vertices_pos->z);
      vertices_pos->x /= length;
      vertices_pos->y /= length;
      vertices_pos->z /= length;
    }

  /* Calculate texture coordinates */
  for (i = 0; i < vertices->len; i++)
    {
      vertices_pos = &VERT (i);

      vertices_pos->tx = (atan2 (vertices_pos->x, vertices_pos->z)
                          / G_PI / 2.0 + 0.5);
      vertices_pos->ty = asin (vertices_pos->y) / G_PI + 0.5;
    }

  /* Fix all of the triangles along the seam. If a triangle contains
     vertices with texture coordinates that wrap the long way from
     0->1 then we need to duplicate one of them to extend the texture
     coordinate past 1 so that it will vary across the span
     correctly */
  for (i = 0; i < n_indices; i += 3)
    {
      gfloat min_tx = G_MAXDOUBLE, max_tx = -G_MAXDOUBLE;
      int v;

      for (v = 0; v < 3; v++)
        {
          gfloat tx = VERT (indices[i + v]).tx;
          if (tx < min_tx)
            min_tx = tx;
          if (tx > max_tx)
            max_tx = tx;
        }

      /* If the span is greater than half of the texture then it would
         be shorter to wrap around instead */
      if (max_tx - min_tx > 0.5f)
        {
          int n_left = 0, n_right = 0, left, right;
          gfloat tx_diff;

          /* Find the odd one out */
          for (v = 0; v < 3; v++)
            if (VERT (indices[i + v]).tx < 0.5f)
              {
                n_left++;
                left = v;
              }
            else
              {
                n_right++;
                right = v;
              }

          /* Duplicate whichever side is the odd one out */
          if (n_left == 1)
            {
              v = left;
              tx_diff = 1.0f;
            }
          else
            {
              v = right;
              tx_diff = -1.0f;
            }

          /* Duplicate it with a different tx */
          g_array_set_size (vertices, vertices->len + 1);
          vertices_pos = &VERT (vertices->len - 1);
          *vertices_pos = VERT (indices[i + v]);
          vertices_pos->tx += tx_diff;
          indices[i + v] = vertices->len - 1;
        }
    }

  /* Create the VBO */
  vertices_pos = &VERT (0);
  priv->vertices = cogl_vertex_buffer_new (vertices->len);
  cogl_vertex_buffer_add (priv->vertices, "gl_Vertex", 3,
                          COGL_ATTRIBUTE_TYPE_FLOAT, FALSE,
                          sizeof (GpsgSphereVertex),
                          &vertices_pos->x);
  cogl_vertex_buffer_add (priv->vertices, "gl_MultiTexCoord0", 2,
                          COGL_ATTRIBUTE_TYPE_FLOAT, FALSE,
                          sizeof (GpsgSphereVertex),
                          &vertices_pos->tx);
  /* The normals are the same as the untransformed vertices */
  cogl_vertex_buffer_add (priv->vertices, "gl_Normal", 3,
                          COGL_ATTRIBUTE_TYPE_FLOAT, FALSE,
                          sizeof (GpsgSphereVertex),
                          &vertices_pos->x);
  cogl_vertex_buffer_submit (priv->vertices);

  priv->n_vertices = vertices->len;
  priv->n_indices = n_indices;

  priv->indices
    = cogl_vertex_buffer_indices_new (COGL_INDICES_TYPE_UNSIGNED_SHORT,
                                      indices, n_indices);

  /* Make sure that we allocated exactly the right amount of memory */
  g_assert (edges_pos == edges + n_edges);
  g_assert (indices_pos == indices + n_indices);
  g_assert (max_stack_pos == stack + stack_size);

  g_free (edges);
  g_array_free (vertices, TRUE);
  g_free (indices);
  g_free (stack);
}

#undef VERT

static gboolean
gpsg_sphere_compile_program (GpsgSphere *sphere)
{
  GpsgSpherePrivate *priv = sphere->priv;
  CoglHandle shader;
  gboolean ret = TRUE;

  /* If we've previously failed to create a shader then don't try
     again */
  if (priv->shader_failed)
    ret = FALSE;
  /* If we've already got the program then we don't need to do
     anything */
  else if (priv->flat_program == COGL_INVALID_HANDLE)
    {
      shader = cogl_create_shader (COGL_SHADER_TYPE_VERTEX);

      if (shader == COGL_INVALID_HANDLE)
        {
          g_warning ("Failed to create shader");
          priv->shader_failed = TRUE;
          ret = FALSE;
        }
      else
        {
          cogl_shader_source (shader, gpsg_sphere_vertex_shader);
          cogl_shader_compile (shader);

          if (cogl_shader_is_compiled (shader))
            {
              priv->flat_program = cogl_create_program ();
              cogl_program_attach_shader (priv->flat_program, shader);
              cogl_program_link (priv->flat_program);

              if ((priv->flatness_uniform
                   = cogl_program_get_uniform_location (priv->flat_program,
                                                        "flatness")) == -1
                  || (priv->flat_width_uniform
                      = cogl_program_get_uniform_location (priv->flat_program,
                                                           "flat_width")) == -1
                  || (priv->flat_height_uniform
                      = cogl_program_get_uniform_location (priv->flat_program,
                                                           "flat_height")) == -1
                  || ((priv->sphere_radius_uniform
                       = cogl_program_get_uniform_location (priv->flat_program,
                                                            "sphere_radius"))
                      == -1))
                {
                  g_warning ("Shader is missing some uniforms");
                  cogl_handle_unref (priv->flat_program);
                  priv->flat_program = COGL_INVALID_HANDLE;
                  priv->shader_failed = TRUE;
                  ret = FALSE;
                }
            }
          else
            {
              gchar *info_log = cogl_shader_get_info_log (shader);
              g_warning ("%s", info_log);
              g_free (info_log);
              priv->shader_failed = TRUE;
              ret = FALSE;
            }

          cogl_handle_unref (shader);
        }
    }

  return ret;
}

static void
gpsg_sphere_paint (ClutterActor *self)
{
  GpsgSphere *sphere = GPSG_SPHERE (self);
  GpsgSpherePrivate *priv = sphere->priv;

  /* If the sphere is completely flat we can just use the regular
     texture paint method */
  if (priv->flatness >= 1.0f)
    {
      if (CLUTTER_ACTOR_CLASS (gpsg_sphere_parent_class)->paint)
        CLUTTER_ACTOR_CLASS (gpsg_sphere_parent_class)->paint (self);
    }
  else
    {
      ClutterGeometry geom;
      CoglHandle material;
      gboolean backface_culling_was_enabled;
      gfloat sphere_radius;
      gboolean use_shader;
      guint8 paint_opacity;

      clutter_actor_get_allocation_geometry (self, &geom);
      paint_opacity = clutter_actor_get_paint_opacity (self);

      sphere_radius = MIN (geom.width, geom.height) / 2.0f;

      gpsg_sphere_ensure_vertices (sphere);

      backface_culling_was_enabled = cogl_get_backface_culling_enabled ();
      cogl_set_backface_culling_enabled (TRUE);

      use_shader = (priv->flatness > 0.0f
                    && gpsg_sphere_compile_program (sphere));

      cogl_push_matrix ();
      cogl_translate (geom.width / 2.0f, geom.height / 2.0f, 0.0f);

      if (use_shader)
        {
          cogl_program_use (priv->flat_program);
          cogl_program_uniform_1f (priv->flatness_uniform, priv->flatness);
          cogl_program_uniform_1f (priv->flat_width_uniform, geom.width);
          cogl_program_uniform_1f (priv->flat_height_uniform, geom.height);
          cogl_program_uniform_1f (priv->sphere_radius_uniform, sphere_radius);
        }
      else
        cogl_scale (sphere_radius, sphere_radius, sphere_radius);

      if ((priv->paint_flags & GPSG_SPHERE_PAINT_TEXTURE))
        {
          material = clutter_texture_get_cogl_material (CLUTTER_TEXTURE (self));

          cogl_material_set_color4ub (material,
                                      paint_opacity,
                                      paint_opacity,
                                      paint_opacity,
                                      paint_opacity);

          if (material != COGL_INVALID_HANDLE)
            {
              cogl_set_source (material);

              cogl_vertex_buffer_draw_elements (priv->vertices,
                                                COGL_VERTICES_MODE_TRIANGLES,
                                                priv->indices,
                                                0, priv->n_vertices - 1,
                                                0, priv->n_indices);
            }
        }

      if ((priv->paint_flags & GPSG_SPHERE_PAINT_LINES))
        {
          int i;
          CoglColor color;

          cogl_color_set_from_4ub (&color,
                                   priv->lines_color.red,
                                   priv->lines_color.green,
                                   priv->lines_color.blue,
                                   priv->lines_color.alpha
                                   * paint_opacity / 255);
          cogl_color_premultiply (&color);
          cogl_set_source_color (&color);

          /* If we could set the polygon mode this could be done with
             triangles in a single call instead */
          for (i = 0; i + 3 <= priv->n_indices; i += 3)
            cogl_vertex_buffer_draw_elements (priv->vertices,
                                              COGL_VERTICES_MODE_LINE_LOOP,
                                              priv->indices,
                                              0, priv->n_vertices - 1,
                                              i, 3);
        }

      if (use_shader)
        cogl_program_use (COGL_INVALID_HANDLE);

      cogl_pop_matrix ();

     cogl_set_backface_culling_enabled (backface_culling_was_enabled);
    }
}

static void
gpsg_sphere_forget_vertices (GpsgSphere *sphere)
{
  GpsgSpherePrivate *priv = sphere->priv;

  if (priv->vertices != COGL_INVALID_HANDLE)
    {
      cogl_handle_unref (priv->vertices);
      cogl_handle_unref (priv->indices);
      priv->vertices = COGL_INVALID_HANDLE;
      priv->indices = COGL_INVALID_HANDLE;
    }
}

static void
gpsg_sphere_dispose (GObject *self)
{
  GpsgSphere *sphere = GPSG_SPHERE (self);
  GpsgSpherePrivate *priv = sphere->priv;

  gpsg_sphere_forget_vertices (sphere);

  if (priv->flat_program != COGL_INVALID_HANDLE)
    {
      cogl_handle_unref (priv->flat_program);
      priv->flat_program = COGL_INVALID_HANDLE;
    }

  G_OBJECT_CLASS (gpsg_sphere_parent_class)->dispose (self);
}

guint
gpsg_sphere_get_depth (GpsgSphere *sphere)
{
  g_return_val_if_fail (GPSG_IS_SPHERE (sphere), 0);

  return sphere->priv->depth;
}

void
gpsg_sphere_set_depth (GpsgSphere *sphere, guint depth)
{
  GpsgSpherePrivate *priv;

  g_return_if_fail (GPSG_IS_SPHERE (sphere));

  priv = sphere->priv;

  if (depth != priv->depth)
    {
      gpsg_sphere_forget_vertices (sphere);
      priv->depth = depth;
      clutter_actor_queue_redraw (CLUTTER_ACTOR (sphere));
      g_object_notify (G_OBJECT (sphere), "depth");
    }
}

gfloat
gpsg_sphere_get_flatness (GpsgSphere *sphere)
{
  g_return_val_if_fail (GPSG_IS_SPHERE (sphere), 0);

  return sphere->priv->flatness;
}

void
gpsg_sphere_set_flatness (GpsgSphere *sphere, gfloat flatness)
{
  GpsgSpherePrivate *priv;

  g_return_if_fail (GPSG_IS_SPHERE (sphere));

  priv = sphere->priv;

  if (flatness != priv->flatness)
    {
      priv->flatness = flatness;
      clutter_actor_queue_redraw (CLUTTER_ACTOR (sphere));
      g_object_notify (G_OBJECT (sphere), "flatness");
    }
}

GpsgSpherePaintFlags
gpsg_sphere_get_paint_flags (GpsgSphere *sphere)
{
  g_return_val_if_fail (GPSG_IS_SPHERE (sphere), 0);

  return sphere->priv->paint_flags;
}

void
gpsg_sphere_set_paint_flags (GpsgSphere *sphere,
                             GpsgSpherePaintFlags flags)
{
  GpsgSpherePrivate *priv;

  g_return_if_fail (GPSG_IS_SPHERE (sphere));

  priv = sphere->priv;

  if (flags != priv->paint_flags)
    {
      priv->paint_flags = flags;
      clutter_actor_queue_redraw (CLUTTER_ACTOR (sphere));
      g_object_notify (G_OBJECT (sphere), "paint-flags");
    }
}

void
gpsg_sphere_get_lines_color (GpsgSphere *sphere,
                             ClutterColor *color)
{
  g_return_if_fail (GPSG_IS_SPHERE (sphere));

  *color = sphere->priv->lines_color;
}

void
gpsg_sphere_set_lines_color (GpsgSphere *sphere,
                             const ClutterColor *color)
{
  GpsgSpherePrivate *priv;

  g_return_if_fail (GPSG_IS_SPHERE (sphere));

  priv = sphere->priv;

  if (!clutter_color_equal (color, &priv->lines_color))
    {
      priv->lines_color = *color;
      if (priv->paint_flags & GPSG_SPHERE_PAINT_LINES)
        clutter_actor_queue_redraw (CLUTTER_ACTOR (sphere));
      g_object_notify (G_OBJECT (sphere), "lines-color");
    }
}
