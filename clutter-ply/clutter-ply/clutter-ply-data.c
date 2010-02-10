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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib-object.h>
#include <string.h>
#include <cogl/cogl.h>

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
}
clutter_ply_data_properties[] =
{
  { "x" },
  { "y" },
  { "z" },
  { "nx" },
  { "ny" },
  { "nz" },
  { "s" },
  { "t" },
  { "red" },
  { "green" },
  { "blue" }
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
  gfloat current_vertex[G_N_ELEMENTS (clutter_ply_data_properties)];
  gint prop_map[G_N_ELEMENTS (clutter_ply_data_properties)];
  gint n_props, available_props, got_props;
  gushort first_vertex, last_vertex;
  GArray *vertices;
  GArray *faces;
};

struct _ClutterPlyDataPrivate
{
  CoglHandle vertices_vbo;
  CoglHandle indices;
  guint min_index, max_index;
  guint n_triangles;
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
  /* Colors are specified as a byte so we need to normalize them to a
     float */
  if (((1 << prop_num) & CLUTTER_PLY_DATA_COLOR_PROPS))
    value /= 255.0;

  data->current_vertex[data->prop_map[prop_num]] = value;
  data->got_props |= 1 << prop_num;

  /* If we've got enough properties for a complete vertex then add it
     to the array */
  if (data->got_props == data->available_props)
    {
      g_array_append_vals (data->vertices, data->current_vertex, data->n_props);
      data->got_props = 0;
    }

  return 1;
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
      gushort new_vertex = ply_get_argument_value (argument);

      /* Add a triangle with the first vertex, the last vertex and
         this new vertex */
      g_array_append_val (data->faces, data->first_vertex);
      g_array_append_val (data->faces, data->last_vertex);
      g_array_append_val (data->faces, new_vertex);

      /* Use the new vertex as one of the vertices next time around */
      data->last_vertex = new_vertex;
    }

  return 1;
}

gboolean
clutter_ply_data_load (ClutterPlyData *self,
                       const gchar *filename,
                       GError **error)
{
  ClutterPlyDataPrivate *priv;
  ClutterPlyDataLoadData data;
  gchar *utf8_filename;
  gboolean ret;

  g_return_val_if_fail (CLUTTER_PLY_IS_DATA (self), FALSE);

  priv = self->priv;

  data.model = self;
  data.error = NULL;
  data.n_props = 0;
  data.available_props = 0;
  data.got_props = 0;
  data.vertices = g_array_new (FALSE, FALSE, sizeof (gfloat));
  data.faces = g_array_new (FALSE, FALSE, sizeof (gushort));

  utf8_filename = g_filename_to_utf8 (filename, -1, NULL, NULL, NULL);
  if (utf8_filename == NULL)
    utf8_filename = g_strdup ("?");

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
                data.prop_map[i] = data.n_props++;
                data.available_props |= 1 << i;
              }

          if ((data.available_props & CLUTTER_PLY_DATA_VERTEX_PROPS)
              != CLUTTER_PLY_DATA_VERTEX_PROPS)
            g_set_error (&data.error, CLUTTER_PLY_DATA_ERROR,
                         CLUTTER_PLY_DATA_ERROR_MISSING_PROPERTY,
                         "PLY file %s is missing the vertex properties",
                         utf8_filename);
          else if (!ply_set_read_cb (data.ply, "face", "vertex_indices",
                                     clutter_ply_data_face_read_cb,
                                     &data, i))
            g_set_error (&data.error, CLUTTER_PLY_DATA_ERROR,
                         CLUTTER_PLY_DATA_ERROR_MISSING_PROPERTY,
                         "PLY file %s is missing face property "
                         "'vertex_indices'",
                         utf8_filename);
          else if (!ply_read (data.ply))
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
                   utf8_filename);
      ret = FALSE;
    }
  else
    {
      GLuint min_index = G_MAXUINT;
      GLuint max_index = 0;
      int i;

      /* Make sure all of the indices are valid and calculate the
         range of used vertices */
      for (i = 0; i < data.faces->len; i++)
        {
          gushort index = g_array_index (data.faces, gushort, i);
          if (index >= data.vertices->len / data.n_props)
            break;
          else if (index < min_index)
            min_index = index;
          else if (index > max_index)
            max_index = index;
        }
      if (i < data.faces->len)
        {
          g_set_error (error, CLUTTER_PLY_DATA_ERROR,
                       CLUTTER_PLY_DATA_ERROR_INVALID,
                       "Index out of range in %s",
                       utf8_filename);
          ret = FALSE;
        }
      else
        {
          guint offset = 0;
          /* Get rid of the old VBOs (if any) */
          clutter_ply_data_free_vbos (self);

          /* Create a new VBO for the vertices */
          priv->vertices_vbo = cogl_vertex_buffer_new (data.vertices->len
                                                       / data.n_props);

          /* Upload the data */
          if ((data.available_props & CLUTTER_PLY_DATA_VERTEX_PROPS)
              == CLUTTER_PLY_DATA_VERTEX_PROPS)
            {
              cogl_vertex_buffer_add (priv->vertices_vbo,
                                      "gl_Vertex",
                                      3, COGL_ATTRIBUTE_TYPE_FLOAT,
                                      FALSE, sizeof (gfloat) * data.n_props,
                                      data.vertices->data
                                      + sizeof (gfloat) * offset);
              offset += 3;
            }

          if ((data.available_props & CLUTTER_PLY_DATA_NORMAL_PROPS)
              == CLUTTER_PLY_DATA_NORMAL_PROPS)
            {
              cogl_vertex_buffer_add (priv->vertices_vbo,
                                      "gl_Normal",
                                      3, COGL_ATTRIBUTE_TYPE_FLOAT,
                                      FALSE, sizeof (gfloat) * data.n_props,
                                      data.vertices->data
                                      + sizeof (gfloat) * offset);
              offset += 3;
            }

          if ((data.available_props & CLUTTER_PLY_DATA_TEX_COORD_PROPS)
              == CLUTTER_PLY_DATA_TEX_COORD_PROPS)
            {
              cogl_vertex_buffer_add (priv->vertices_vbo,
                                      "gl_MultiTexCoord0",
                                      2, COGL_ATTRIBUTE_TYPE_FLOAT,
                                      FALSE, sizeof (gfloat) * data.n_props,
                                      data.vertices->data
                                      + sizeof (gfloat) * offset);
              offset += 2;
            }

          if ((data.available_props & CLUTTER_PLY_DATA_COLOR_PROPS)
              == CLUTTER_PLY_DATA_COLOR_PROPS)
            {
              cogl_vertex_buffer_add (priv->vertices_vbo,
                                      "gl_Color",
                                      3, COGL_ATTRIBUTE_TYPE_FLOAT,
                                      FALSE, sizeof (gfloat) * data.n_props,
                                      data.vertices->data
                                      + sizeof (gfloat) * offset);
              offset += 3;
            }

          cogl_vertex_buffer_submit (priv->vertices_vbo);

          /* Create a VBO for the indices */
          priv->indices
            = cogl_vertex_buffer_indices_new (COGL_INDICES_TYPE_UNSIGNED_SHORT,
                                              data.faces->data,
                                              data.faces->len);

          priv->min_index = min_index;
          priv->max_index = max_index;
          priv->n_triangles = data.faces->len / 3;

          ret = TRUE;
        }
    }

  g_free (utf8_filename);
  g_array_free (data.vertices, TRUE);
  g_array_free (data.faces, TRUE);

  return ret;
}

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

GQuark
clutter_ply_data_error_quark (void)
{
  return g_quark_from_static_string ("clutter-ply-data-error-quark");
}
