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

/* 'flatness' should be a number from 0.0 -> 1.0. If it is 0.0 the
   model will be rendered as a sphere, or if it is 1.0 it will be
   rendered as a flat model. Otherwise it will be rendered somewhere
   in-between the two. */
uniform float flatness;

/* size of the model when it is rendered completely flat */
uniform float flat_width;
uniform float flat_height;

/* radius of the sphere */
uniform float sphere_radius;

void
main ()
{
  vec3 sphere_position = gl_Vertex.xyz * sphere_radius;

  vec3 flat_position;
  flat_position.xyz = vec3 (vec2 (flat_width, flat_height)
                            * (gl_MultiTexCoord0.xy - vec2 (0.5, 0.5)),
                            0.0);

  /* Linear interpolation between the flat position and sphere position */
  vec4 lerp_position;
  lerp_position = vec4 (mix (sphere_position, flat_position, flatness), 1.0);

  gl_Position = gl_ModelViewProjectionMatrix * vec4 (lerp_position, 1.0);
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_FrontColor = gl_Color;
}
