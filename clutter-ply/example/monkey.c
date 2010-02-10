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

int
main (int argc, char **argv)
{
  ClutterActor *stage = stage, *model, *label;
  GError *error = NULL;

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();

  if ((model = clutter_ply_model_new_from_file ("TheMonkey.ply", &error))
      == NULL)
    {
      g_warning ("%s", error->message);
      g_clear_error (&error);
    }
  else
    {
      CoglHandle material = cogl_material_new ();
      ClutterAnimation *anim;

      cogl_material_set_color4ub (material, 255, 0, 0, 255);
      clutter_ply_model_set_material (CLUTTER_PLY_MODEL (model), material);
      clutter_actor_set_scale (model, 80, 80);
      clutter_actor_set_rotation (model, CLUTTER_X_AXIS, 90.0f,
                                  0.0f, 0.0f, 0.0f);
      clutter_actor_set_position (model,
                                  clutter_actor_get_width (stage) / 2,
                                  clutter_actor_get_height (stage) * 0.9);
      clutter_container_add (CLUTTER_CONTAINER (stage), model, NULL);

      /* Enable depth testing only for this actor */
      g_signal_connect_swapped (model, "paint",
                                G_CALLBACK (cogl_set_depth_test_enabled),
                                (gpointer) TRUE);
      g_signal_connect_data (model, "paint",
                             G_CALLBACK (cogl_set_depth_test_enabled),
                             (gpointer) TRUE,
                             NULL,
                             G_CONNECT_AFTER | G_CONNECT_SWAPPED);

      anim = clutter_actor_animate (model,
                                    CLUTTER_LINEAR, 3000,
                                    "rotation-angle-y", 360.0f,
                                    NULL);
      clutter_animation_set_loop (anim, TRUE);
    }

  label = clutter_text_new_with_text ("Sans 15px",
                                      "The monkey model is Copyright "
                                      "Kursad Karatas "
                                      "and distributed under the "
                                      "Attribution-Share Alike 3.0 Unported "
                                      "license.");
  clutter_text_set_line_wrap (CLUTTER_TEXT (label), TRUE);
  clutter_actor_set_width (label, clutter_actor_get_width (stage));
  clutter_actor_set_position (label, 0,
                              clutter_actor_get_height (stage)
                              - clutter_actor_get_height (label));
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), label);

  clutter_actor_show (stage);

  clutter_main ();

  return 0;
}
