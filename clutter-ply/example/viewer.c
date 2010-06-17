/*
 * Clutter PLY - A library for displaying PLY models in a Clutter scene
 * Copyright (C) 2010  Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include <clutter/clutter.h>
#include <clutter-ply/clutter-ply.h>
#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char **argv)
{
  ClutterActor *stage = stage, *model;
  GError *error = NULL;

  clutter_init (&argc, &argv);

  if (argc != 2 && argc != 3)
    {
      fprintf (stderr, "usage: %s <ply-file> [texture]\n", argv[0]);
      exit (1);
    }

  stage = clutter_stage_get_default ();

  if ((model = clutter_ply_model_new_from_file (argv[1], &error))
      == NULL)
    {
      g_warning ("%s", error->message);
      g_clear_error (&error);
    }
  else
    {
      ClutterAnimation *anim;
      ClutterVertex center_vertex;

      /* If a texture was specified then set that as a material */
      if (argc > 2)
        {
          CoglHandle texture =
            cogl_texture_new_from_file (argv[2],
                                        COGL_TEXTURE_NONE,
                                        COGL_PIXEL_FORMAT_ANY,
                                        &error);

          if (texture == COGL_INVALID_HANDLE)
            {
              g_warning ("%s", error->message);
              g_clear_error (&error);
            }
          else
            {
              CoglHandle material = cogl_material_new ();
              cogl_material_set_layer (material, 0, texture);
              cogl_handle_unref (texture);

              clutter_ply_model_set_material (CLUTTER_PLY_MODEL (model),
                                              material);

              cogl_handle_unref (material);
            }
        }

      clutter_actor_set_size (model,
                              clutter_actor_get_width (stage) * 0.7f,
                              clutter_actor_get_height (stage) * 0.7f);
      clutter_actor_set_position (model,
                                  clutter_actor_get_width (stage) * 0.15f,
                                  clutter_actor_get_height (stage) * 0.15f);

      center_vertex.x = clutter_actor_get_width (stage) * 0.35f;
      center_vertex.y = 0.0f;
      center_vertex.z = 0.0f;

      anim = clutter_actor_animate (model,
                                    CLUTTER_LINEAR, 3000,
                                    "rotation-angle-y", 360.0f,
                                    "fixed::rotation-center-y", &center_vertex,
                                    NULL);
      clutter_animation_set_loop (anim, TRUE);

      clutter_container_add_actor (CLUTTER_CONTAINER (stage), model);

      /* Enable depth testing only for this actor */
      g_signal_connect_swapped (model, "paint",
                                G_CALLBACK (cogl_set_depth_test_enabled),
                                GINT_TO_POINTER (TRUE));
      g_signal_connect_data (model, "paint",
                             G_CALLBACK (cogl_set_depth_test_enabled),
                             GINT_TO_POINTER (FALSE), NULL,
                             G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    }

  clutter_actor_show (stage);

  clutter_main ();

  return 0;
}
