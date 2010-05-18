/*
 * Clutter PLY - A library for displaying PLY models in a Clutter scene
 * Copyright (C) 2010  Intel Corporation
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

/**
 * SECTION:clutter-ply-data
 * @short_description: An object that contains the data for a PLY model.
 *
 * #ClutterPlyData is an object that can represent the data contained
 * in a PLY file. The data is internally converted to a
 * Cogl vertex buffer so that it can be rendered efficiently.
 *
 * The #ClutterPlyData object is usually associated with a
 * #ClutterPlyModel so that it can be animated as a regular actor. The
 * data is separated from the actor in this way to make it easy to
 * share data with multiple actors without having to keep two copies
 * of the data.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib-object.h>
#include <string.h>
#include <cogl/cogl.h>
#include <clutter/clutter.h>

#include "clutter-ply-data.h"
#include "rply/rply.h"

static void clutter_ply_data_finalize (GObject *object);

G_DEFINE_TYPE (ClutterPlyData, clutter_ply_data, G_TYPE_OBJECT);

#define CLUTTER_PLY_DATA_GET_PRIVATE(obj)                       \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CLUTTER_PLY_TYPE_DATA,   \
                                ClutterPlyDataPrivate))

static const struct
{
  const gchar *name;
  int size;
}
clutter_ply_data_properties[] =
{
  /* These should be sorted in descending order of size so that it
     never ends doing an unaligned write */
  { "x", sizeof (gfloat) },
  { "y", sizeof (gfloat) },
  { "z", sizeof (gfloat) },
  { "nx", sizeof (gfloat) },
  { "ny", sizeof (gfloat) },
  { "nz", sizeof (gfloat) },
  { "s", sizeof (gfloat) },
  { "t", sizeof (gfloat) },
  { "red", sizeof (guint8) },
  { "green", sizeof (guint8) },
  { "blue", sizeof (guint8) }
};

#define CLUTTER_PLY_DATA_VERTEX_PROPS    7
#define CLUTTER_PLY_DATA_NORMAL_PROPS    (7 << 3)
#define CLUTTER_PLY_DATA_TEX_COORD_PROPS (3 << 6)
#define CLUTTER_PLY_DATA_COLOR_PROPS     (7 << 8)

typedef struct _ClutterPlyDataLoadData ClutterPlyDataLoadData;

struct _ClutterPlyDataLoadData
{
  ClutterPlyData *model;
  p_ply ply;
  GError *error;
  /* Data for the current vertex */
  guint8 current_vertex[G_N_ELEMENTS (clutter_ply_data_properties) * 4];
  /* Map from property number to byte offset in the current_vertex array */
  gint prop_map[G_N_ELEMENTS (clutter_ply_data_properties)];
  /* Number of bytes for a vertex */
  guint n_vertex_bytes;
  gint available_props, got_props;
  guint first_vertex, last_vertex;
  GByteArray *vertices;
  GArray *faces;
  CoglIndicesType indices_type;

  /* Bounding cuboid of the data */
  ClutterVertex min_vertex, max_vertex;

  /* Range of indices used */
  guint min_index, max_index;
};

struct _ClutterPlyDataPrivate
{
  CoglHandle vertices_vbo;
  CoglHandle indices;
  guint min_index, max_index;
  guint n_triangles;

  /* Bounding cuboid of the data */
  ClutterVertex min_vertex, max_vertex;
};

static void
clutter_ply_data_class_init (ClutterPlyDataClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;

  gobject_class->finalize = clutter_ply_data_finalize;

  g_type_class_add_private (klass, sizeof (ClutterPlyDataPrivate));
}

static void
clutter_ply_data_init (ClutterPlyData *self)
{
  self->priv = CLUTTER_PLY_DATA_GET_PRIVATE (self);
}

static void
clutter_ply_data_free_vbos (ClutterPlyData *self)
{
  ClutterPlyDataPrivate *priv = self->priv;

  if (priv->vertices_vbo)
    {
      cogl_handle_unref (priv->vertices_vbo);
      priv->vertices_vbo = NULL;
    }

  if (priv->indices)
    {
      cogl_handle_unref (priv->indices);
      priv->indices = NULL;
    }
}

static void
clutter_ply_data_finalize (GObject *object)
{
  ClutterPlyData *self = (ClutterPlyData *) object;

  clutter_ply_data_free_vbos (self);

  G_OBJECT_CLASS (clutter_ply_data_parent_class)->finalize (object);
}

/**
 * clutter_ply_data_new:
 *
 * Constructs a new #ClutterPlyData instance. The object initially has
 * no data so nothing will be drawn when clutter_ply_data_render() is
 * called. To load data into the object, call clutter_ply_data_load().
 *
 * Return value: a new #ClutterPlyData.
 */
ClutterPlyData *
clutter_ply_data_new (void)
{
  ClutterPlyData *self = g_object_new (CLUTTER_PLY_TYPE_DATA, NULL);

  return self;
}

static void
clutter_ply_data_error_cb (const char *message, gpointer data)
{
  ClutterPlyDataLoadData *load_data = data;

  if (load_data->error == NULL)
    g_set_error_literal (&load_data->error, CLUTTER_PLY_DATA_ERROR,
                         CLUTTER_PLY_DATA_ERROR_PLY, message);
}

static void
clutter_ply_data_check_unknown_error (ClutterPlyDataLoadData *data)
{
  if (data->error == NULL)
    g_set_error_literal (&data->error,
                         CLUTTER_PLY_DATA_ERROR,
                         CLUTTER_PLY_DATA_ERROR_PLY,
                         "Unknown error loading PLY file");
}

static int
clutter_ply_data_vertex_read_cb (p_ply_argument argument)
{
  long prop_num;
  ClutterPlyDataLoadData *data;
  gint32 length, index;
  double value;

  ply_get_argument_user_data (argument, (void **) &data, &prop_num);
  ply_get_argument_property (argument, NULL, &length, &index);

  if (length != 1 || index != 0)
    {
      g_set_error (&data->error, CLUTTER_PLY_DATA_ERROR,
                   CLUTTER_PLY_DATA_ERROR_INVALID,
                   "List type property not supported for vertex element '%s'",
                   clutter_ply_data_properties[prop_num].name);

      return 0;
    }

  value = ply_get_argument_value (argument);

  /* Colors are specified as a byte so we need to treat them specially */
  if (((1 << prop_num) & CLUTTER_PLY_DATA_COLOR_PROPS))
    data->current_vertex[data->prop_map[prop_num]] = value;
  else
    *(gfloat *) (data->current_vertex + data->prop_map[prop_num]) = value;

  data->got_props |= 1 << prop_num;

  /* If we've got enough properties for a complete vertex then add it
     to the array */
  if (data->got_props == data->available_props)
    {
      int i;

      g_byte_array_append (data->vertices, data->current_vertex,
                           data->n_vertex_bytes);
      data->got_props = 0;

      /* Update the bounding box for the data */
      for (i = 0; i < 3; i++)
        {
          gfloat *min = &data->min_vertex.x + i;
          gfloat *max = &data->max_vertex.x + i;
          gfloat value = *(gfloat *) (data->current_vertex + data->prop_map[i]);

          if (value < *min)
            *min = value;
          if (value > *max)
            *max = value;
        }
    }

  return 1;
}

static void
clutter_ply_data_add_face_index (ClutterPlyDataLoadData *data,
                                 guint index)
{
  if (index > data->max_index)
    data->max_index = index;
  if (index < data->min_index)
    data->min_index = index;

  switch (data->indices_type)
    {
    case COGL_INDICES_TYPE_UNSIGNED_BYTE:
      {
        guint8 value = index;
        g_array_append_val (data->faces, value);
      }
      break;
    case COGL_INDICES_TYPE_UNSIGNED_SHORT:
      {
        guint16 value = index;
        g_array_append_val (data->faces, value);
      }
      break;
    case COGL_INDICES_TYPE_UNSIGNED_INT:
      {
        guint32 value = index;
        g_array_append_val (data->faces, value);
      }
      break;
    }
}

static gboolean
clutter_ply_data_get_indices_type (ClutterPlyDataLoadData *data,
                                   GError **error)
{
  p_ply_element elem = NULL;

  /* Look for the 'vertices' element */
  while ((elem = ply_get_next_element (data->ply, elem)))
    {
      const char *name;
      gint32 n_instances;

      if (ply_get_element_info (elem, &name, &n_instances))
        {
          if (!strcmp (name, "vertex"))
            {
              if (n_instances <= 0x100)
                {
                  data->indices_type = COGL_INDICES_TYPE_UNSIGNED_BYTE;
                  data->faces = g_array_new (FALSE, FALSE, sizeof (guint8));
                }
              else if (n_instances <= 0x10000)
                {
                  data->indices_type = COGL_INDICES_TYPE_UNSIGNED_SHORT;
                  data->faces = g_array_new (FALSE, FALSE, sizeof (guint16));
                }
              else if (cogl_features_available
                       (COGL_FEATURE_UNSIGNED_INT_INDICES))
                {
                  data->indices_type = COGL_INDICES_TYPE_UNSIGNED_INT;
                  data->faces = g_array_new (FALSE, FALSE, sizeof (guint32));
                }
              else
                {
                  g_set_error (error, CLUTTER_PLY_DATA_ERROR,
                               CLUTTER_PLY_DATA_ERROR_UNSUPPORTED,
                               "The PLY file requires unsigned int indices "
                               "but this is not supported by your GL driver");
                  return FALSE;
                }

              return TRUE;
            }
        }
      else
        {
          g_set_error (error, CLUTTER_PLY_DATA_ERROR,
                       CLUTTER_PLY_DATA_ERROR_PLY,
                       "Error getting element info");
          return FALSE;
        }
    }

  g_set_error (error, CLUTTER_PLY_DATA_ERROR,
               CLUTTER_PLY_DATA_ERROR_MISSING_PROPERTY,
               "PLY file is missing the vertex element");

  return FALSE;
}

static int
clutter_ply_data_face_read_cb (p_ply_argument argument)
{
  long prop_num;
  ClutterPlyDataLoadData *data;
  gint32 length, index;

  ply_get_argument_user_data (argument, (void **) &data, &prop_num);
  ply_get_argument_property (argument, NULL, &length, &index);

  if (index == 0)
    data->first_vertex = ply_get_argument_value (argument);
  else if (index == 1)
    data->last_vertex = ply_get_argument_value (argument);
  else if (index != -1)
    {
      guint new_vertex = ply_get_argument_value (argument);

      /* Add a triangle with the first vertex, the last vertex and
         this new vertex */
      clutter_ply_data_add_face_index (data, data->first_vertex);
      clutter_ply_data_add_face_index (data, data->last_vertex);
      clutter_ply_data_add_face_index (data, new_vertex);

      /* Use the new vertex as one of the vertices next time around */
      data->last_vertex = new_vertex;
    }

  return 1;
}

/**
 * clutter_ply_data_load:
 * @self: The #ClutterPlyData instance
 * @filename: The name of a PLY file to load
 * @error: Return location for an error or %NULL
 *
 * Loads the data from the PLY file called @filename into @self. The
 * model can then be rendered using clutter_ply_data_render(). If
 * there is an error loading the file it will return %FALSE and @error
 * will be set to a GError instance.
 *
 * Return value: %TRUE if the load succeeded or %FALSE otherwise.
 */
gboolean
clutter_ply_data_load (ClutterPlyData *self,
                       const gchar *filename,
                       GError **error)
{
  ClutterPlyDataPrivate *priv;
  ClutterPlyDataLoadData data;
  gchar *display_name;
  gboolean ret;

  g_return_val_if_fail (CLUTTER_PLY_IS_DATA (self), FALSE);

  priv = self->priv;

  data.model = self;
  data.error = NULL;
  data.n_vertex_bytes = 0;
  data.available_props = 0;
  data.got_props = 0;
  data.vertices = g_byte_array_new ();
  data.faces = NULL;
  data.min_vertex.x = G_MAXFLOAT;
  data.min_vertex.y = G_MAXFLOAT;
  data.min_vertex.z = G_MAXFLOAT;
  data.max_vertex.x = -G_MAXFLOAT;
  data.max_vertex.y = -G_MAXFLOAT;
  data.max_vertex.z = -G_MAXFLOAT;
  data.min_index = G_MAXUINT;
  data.max_index = 0;

  display_name = g_filename_display_name (filename);

  if ((data.ply = ply_open (filename,
                            clutter_ply_data_error_cb,
                            &data)) == NULL)
    clutter_ply_data_check_unknown_error (&data);
  else
    {
      if (!ply_read_header (data.ply))
        clutter_ply_data_check_unknown_error (&data);
      else
        {
          int i;

          for (i = 0; i < G_N_ELEMENTS (clutter_ply_data_properties); i++)
            if (ply_set_read_cb (data.ply, "vertex",
                                 clutter_ply_data_properties[i].name,
                                 clutter_ply_data_vertex_read_cb,
                                 &data, i))
              {
                data.prop_map[i] = data.n_vertex_bytes;
                data.n_vertex_bytes += clutter_ply_data_properties[i].size;
                data.available_props |= 1 << i;
              }

          /* Align the size of a vertex to 32 bits */
          data.n_vertex_bytes = (data.n_vertex_bytes + 3) & ~(guint) 3;

          if ((data.available_props & CLUTTER_PLY_DATA_VERTEX_PROPS)
              != CLUTTER_PLY_DATA_VERTEX_PROPS)
            g_set_error (&data.error, CLUTTER_PLY_DATA_ERROR,
                         CLUTTER_PLY_DATA_ERROR_MISSING_PROPERTY,
                         "PLY file %s is missing the vertex properties",
                         display_name);
          else if (!ply_set_read_cb (data.ply, "face", "vertex_indices",
                                     clutter_ply_data_face_read_cb,
                                     &data, i))
            g_set_error (&data.error, CLUTTER_PLY_DATA_ERROR,
                         CLUTTER_PLY_DATA_ERROR_MISSING_PROPERTY,
                         "PLY file %s is missing face property "
                         "'vertex_indices'",
                         display_name);
          else if (clutter_ply_data_get_indices_type (&data, &data.error)
                   && !ply_read (data.ply))
            clutter_ply_data_check_unknown_error (&data);
        }

      ply_close (data.ply);
    }

  if (data.error)
    {
      g_propagate_error (error, data.error);
      ret = FALSE;
    }
  else if (data.faces->len < 3)
    {
      g_set_error (error, CLUTTER_PLY_DATA_ERROR,
                   CLUTTER_PLY_DATA_ERROR_INVALID,
                   "No faces found in %s",
                   display_name);
      ret = FALSE;
    }
  else
    {
      /* Make sure all of the indices are valid */
      if (data.max_index >= data.vertices->len / data.n_vertex_bytes)
        {
          g_set_error (error, CLUTTER_PLY_DATA_ERROR,
                       CLUTTER_PLY_DATA_ERROR_INVALID,
                       "Index out of range in %s",
                       display_name);
          ret = FALSE;
        }
      else
        {
          /* Get rid of the old VBOs (if any) */
          clutter_ply_data_free_vbos (self);

          /* Create a new VBO for the vertices */
          priv->vertices_vbo = cogl_vertex_buffer_new (data.vertices->len
                                                       / data.n_vertex_bytes);

          /* Upload the data */
          if ((data.available_props & CLUTTER_PLY_DATA_VERTEX_PROPS)
              == CLUTTER_PLY_DATA_VERTEX_PROPS)
            cogl_vertex_buffer_add (priv->vertices_vbo,
                                    "gl_Vertex",
                                    3, COGL_ATTRIBUTE_TYPE_FLOAT,
                                    FALSE, data.n_vertex_bytes,
                                    data.vertices->data
                                    + data.prop_map[0]);

          if ((data.available_props & CLUTTER_PLY_DATA_NORMAL_PROPS)
              == CLUTTER_PLY_DATA_NORMAL_PROPS)
            cogl_vertex_buffer_add (priv->vertices_vbo,
                                    "gl_Normal",
                                    3, COGL_ATTRIBUTE_TYPE_FLOAT,
                                    FALSE, data.n_vertex_bytes,
                                    data.vertices->data
                                    + data.prop_map[3]);

          if ((data.available_props & CLUTTER_PLY_DATA_TEX_COORD_PROPS)
              == CLUTTER_PLY_DATA_TEX_COORD_PROPS)
            cogl_vertex_buffer_add (priv->vertices_vbo,
                                    "gl_MultiTexCoord0",
                                    2, COGL_ATTRIBUTE_TYPE_FLOAT,
                                    FALSE, data.n_vertex_bytes,
                                    data.vertices->data
                                    + data.prop_map[6]);

          if ((data.available_props & CLUTTER_PLY_DATA_COLOR_PROPS)
              == CLUTTER_PLY_DATA_COLOR_PROPS)
            cogl_vertex_buffer_add (priv->vertices_vbo,
                                    "gl_Color",
                                    3, COGL_ATTRIBUTE_TYPE_UNSIGNED_BYTE,
                                    FALSE, data.n_vertex_bytes,
                                    data.vertices->data
                                    + data.prop_map[8]);

          cogl_vertex_buffer_submit (priv->vertices_vbo);

          /* Create a VBO for the indices */
          priv->indices
            = cogl_vertex_buffer_indices_new (data.indices_type,
                                              data.faces->data,
                                              data.faces->len);

          priv->min_index = data.min_index;
          priv->max_index = data.max_index;
          priv->n_triangles = data.faces->len / 3;

          priv->min_vertex = data.min_vertex;
          priv->max_vertex = data.max_vertex;

          ret = TRUE;
        }
    }

  g_free (display_name);
  g_byte_array_free (data.vertices, TRUE);
  if (data.faces)
    g_array_free (data.faces, TRUE);

  return ret;
}

/**
 * clutter_ply_data_render:
 * @self: A #ClutterPlyData instance
 *
 * Renders the data contained in the PLY model to the Clutter
 * scene. The current Cogl source material will be used to affect the
 * appearance of the model. This function is not usually called
 * directly but instead the #ClutterPlyData instance is added to a
 * #ClutterPlyModel and this function will be automatically called by
 * the paint method of the model.
 */
void
clutter_ply_data_render (ClutterPlyData *self)
{
  ClutterPlyDataPrivate *priv;

  g_return_if_fail (CLUTTER_PLY_IS_DATA (self));

  priv = self->priv;

  /* Silently fail if we didn't load any data */
  if (priv->vertices_vbo == NULL || priv->indices == NULL)
    return;

  cogl_vertex_buffer_draw_elements (priv->vertices_vbo,
                                    COGL_VERTICES_MODE_TRIANGLES,
                                    priv->indices,
                                    priv->min_index,
                                    priv->max_index,
                                    0, priv->n_triangles * 3);
}

/**
 * clutter_ply_data_get_extents:
 * @self: A #ClutterPlyData instance
 * @min_vertex: A location to return the minimum vertex
 * @max_vertex: A location to return the maximum vertex
 *
 * Gets the bounding cuboid of the vertices in @self. The cuboid is
 * represented by two vertices representing the minimum and maximum
 * extents. The x, y and z components of @min_vertex will contain the
 * minimum x, y and z values of all the vertices and @max_vertex will
 * contain the maximum. The extents of the model are cached so it is
 * cheap to call this function.
 */
void
clutter_ply_data_get_extents (ClutterPlyData *self,
                              ClutterVertex *min_vertex,
                              ClutterVertex *max_vertex)
{
  ClutterPlyDataPrivate *priv = self->priv;

  *min_vertex = priv->min_vertex;
  *max_vertex = priv->max_vertex;
}

GQuark
clutter_ply_data_error_quark (void)
{
  return g_quark_from_static_string ("clutter-ply-data-error-quark");
}
