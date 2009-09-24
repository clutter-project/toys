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

#include "gpsg-sphere.h"

static int tex_num;
static int n_tex;
static char **tex;

static void
on_paint (ClutterActor *actor)
{
  ClutterGeometry geom;

  clutter_actor_get_allocation_geometry (actor, &geom);

  cogl_set_source_color4ub (0, 0, 255, 255);
}

static gboolean
set_tex (ClutterActor *sphere)
{
  if (n_tex > 0)
    {
      clutter_texture_set_from_file (CLUTTER_TEXTURE (sphere),
                                     tex[tex_num++],
                                     NULL);
      if (tex_num >= n_tex)
        tex_num = 0;
    }

  return FALSE;
}

int
main (int argc, char **argv)
{
  ClutterActor *stage;
  ClutterActor *sphere;
  ClutterAnimation *anim;
  ClutterVertex center = { 240, 240, 0 };

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();

  sphere = gpsg_sphere_new ();
  g_object_set (sphere,
                "depth", 3,
                "paint-flags", (GPSG_SPHERE_PAINT_LINES
                                | GPSG_SPHERE_PAINT_TEXTURE),
                NULL);
  clutter_actor_set_size (sphere, 480, 480);
  g_signal_connect_after (sphere, "paint",
                          G_CALLBACK (on_paint), NULL);
  clutter_actor_set_position (sphere,
                              clutter_actor_get_width (stage) / 2.0 - 240,
                              clutter_actor_get_height (stage) / 2.0 - 240);
  anim = clutter_actor_animate (sphere, CLUTTER_LINEAR, 8000,
                                "rotation-angle-y", 360.0,
                                "fixed::rotation-center-y", &center,
                                NULL);
  clutter_animation_set_loop (anim, TRUE);

  n_tex = argc - 1;
  tex = argv + 1;
  tex_num = 0;

  set_tex (sphere);

  g_signal_connect_swapped (stage, "button-press-event",
                            G_CALLBACK (set_tex), sphere);

  clutter_container_add_actor (CLUTTER_CONTAINER (stage), sphere);

  clutter_actor_show (stage);

  clutter_main ();

  return 0;
}
