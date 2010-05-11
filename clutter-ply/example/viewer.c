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

#include <clutter/clutter.h>
#include <clutter-ply/clutter-ply.h>
#include <stdio.h>
#include <stdlib.h>

#define Z_RANGE 600

typedef struct _Data Data;

struct _Data
{
  gfloat scale;
};

static void
pre_paint_cb (ClutterActor *actor,
              Data *data)
{
  /* Enable depth testing */
  cogl_set_depth_test_enabled (TRUE);

  /* Scale the actor. We can't just use clutter_actor_set_scale
     because we also need to scale the z axes */
  cogl_push_matrix ();
  cogl_scale (data->scale, data->scale, data->scale);
}

static void
post_paint_cb (ClutterActor *actor,
               Data *data)
{
  cogl_set_depth_test_enabled (FALSE);

  cogl_pop_matrix ();
}

static gfloat
calculate_scale (gfloat target_extents,
                 gfloat min,
                 gfloat max)
{
  if (min == max)
    return G_MAXFLOAT;
  else
    return target_extents / (max - min);
}

int
main (int argc, char **argv)
{
  ClutterActor *stage = stage, *model, *group;
  ClutterAnimation *anim;
  Data data;
  GError *error = NULL;

  clutter_init (&argc, &argv);

  if (argc != 2)
    {
      fprintf (stderr, "usage: %s <ply-file>\n", argv[0]);
      exit (1);
    }

  stage = clutter_stage_get_default ();

  /* The model is put in a group so that we can put the center of the
     model at the origin */
  group = clutter_group_new ();
  clutter_actor_set_position (group,
                              clutter_actor_get_width (stage) / 2.0f,
                              clutter_actor_get_height (stage) / 2.0f);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), group);

  if ((model = clutter_ply_model_new_from_file (argv[1], &error))
      == NULL)
    {
      g_warning ("%s", error->message);
      g_clear_error (&error);
    }
  else
    {
      ClutterPlyData *ply_data;
      ClutterVertex min_vertex, max_vertex;
      gfloat scale, min_scale;

      /* Get the extents of the model */
      ply_data = clutter_ply_model_get_data (CLUTTER_PLY_MODEL (model));
      clutter_ply_data_get_extents (ply_data, &min_vertex, &max_vertex);

      /* Calculate the smallest scale needed to fit to the full
         extents of one the axes */
      min_scale = calculate_scale (clutter_actor_get_width (stage) * 0.7f,
                                   min_vertex.x, max_vertex.x);
      scale = calculate_scale (clutter_actor_get_height (stage) * 0.7f,
                               min_vertex.y, max_vertex.y);
      if (min_scale > scale)
        min_scale = scale;
      scale = calculate_scale (Z_RANGE,
                               min_vertex.z, max_vertex.z);
      if (min_scale > scale)
        min_scale = scale;

      data.scale = min_scale;

      /* Position the model so its center is at the origin of the
         group */
      clutter_actor_set_position (model,
                                  -(min_vertex.x + max_vertex.x) / 2.0f
                                  * min_scale,
                                  -(min_vertex.y + max_vertex.y) / 2.0f
                                  * min_scale);
      clutter_actor_set_depth (model,
                               -(min_vertex.z + max_vertex.z) / 2.0f
                               * min_scale);

      clutter_container_add_actor (CLUTTER_CONTAINER (group), model);

      /* Enable depth testing and set the scale only for this actor */
      g_signal_connect (model, "paint",
                        G_CALLBACK (pre_paint_cb),
                        &data);
      g_signal_connect_after (model, "paint",
                              G_CALLBACK (post_paint_cb),
                              &data);
    }

  anim = clutter_actor_animate (group,
                                CLUTTER_LINEAR, 3000,
                                "rotation-angle-y", 360.0f,
                                NULL);
  clutter_animation_set_loop (anim, TRUE);

  clutter_actor_show (stage);

  clutter_main ();

  return 0;
}
