#include <math.h>
#include "odo-distort-funcs.h"

void
cloth_func (OdoTexture *otex,
            CoglTextureVertex *vertex,
            gfloat             width,
            gfloat             height,
            gpointer           data)
{
  OdoDistortData *d = data;
  gfloat cx, cy, rx, turn_angle, height_radius;
  guint shade;

  /* Rotate the point around the centre of the curl ray to align it with
   * the y-axis.
   */

  cx = (1.f - d->turn) * width;
  cy = (1.f - d->turn) * height;

  rx = ((vertex->x - cx) * cos (-d->angle)) -
       ((vertex->y - cy) * sin (-d->angle)) - d->radius;

  /* Calculate the angle as a function of the distance from the curl ray */
  turn_angle = ((rx / d->radius) * G_PI_2) - G_PI_2;

  /* Add a gradient that makes it look like lighting and hides the switch
   * between textures.
   */
  shade = (255 * (1.f - d->amplitude)) +
          (((sin (turn_angle) * 96) + 159) * d->amplitude);
  vertex->color.red = shade;
  vertex->color.green = shade;
  vertex->color.blue = shade;

  /* Make the wave amplitude lower as its distance from the curl ray increases.
   * Not really necessary, but looks a little nicer I think.
   */
  height_radius = (1 - rx / width) * d->radius;
  vertex->z = height_radius * sin (turn_angle) * d->amplitude;
}

void
bowtie_func (OdoTexture        *otex,
             CoglTextureVertex *vertex,
             gfloat             width,
             gfloat             height,
             gpointer           data)
{
  OdoDistortData *d = data;
  gfloat cx, cy, rx, ry, turn_angle, height_radius;
  guint shade;

  cx = d->turn * (width + width/2);
  cy = height/2;

  rx = ((vertex->x - cx) * cos (0)) -
       ((vertex->y - cy) * sin (0));
  ry = ((vertex->x - cx) * sin (0)) +
       ((vertex->y - cy) * cos (0));

  /* Make angle as a function of distance from the curl ray */
  turn_angle = MAX (-G_PI, MIN (0, (rx / (width/4)) * G_PI_2));

  /* Add a gradient that makes it look like lighting */
  shade = (cos (turn_angle * 2) * 96) + 159;
  vertex->color.red = shade;
  vertex->color.green = shade;
  vertex->color.blue = shade;

  /* Calculate the point on a cone (note, a cone, not a right cone) */
  height_radius = ry;
  /*ClutterFixed height_radius =
    clutter_qmulx (clutter_qdivx (ry, height/2), height/2);*/

  ry = height_radius * cos (turn_angle);
  vertex->x = (rx * cos (0)) - (ry * sin (0)) + cx;
  vertex->y = (rx * sin (0)) + (ry * cos (0)) + cy;
  vertex->z = height_radius * sin (turn_angle);
}

void
page_turn_func (OdoTexture        *otex,
                CoglTextureVertex *vertex,
                gfloat             width,
                gfloat             height,
                gpointer           data)
{
  OdoDistortData *d = data;
  gfloat cx, cy, rx, ry;
  gfloat turn_angle;
  guint shade;

  /* Rotate the point around the centre of the page-curl ray to align it with
   * the y-axis.
   */
  cx = (1.f - d->turn) * width;
  cy = (1.f - d->turn) * height;

  rx = ((vertex->x - cx) * cos (-d->angle)) -
       ((vertex->y - cy) * sin (-d->angle)) - d->radius;
  ry = ((vertex->x - cx) * sin (-d->angle)) +
       ((vertex->y - cy) * cos (-d->angle));

  if (rx > -d->radius * 2)
    {
      /* Calculate the curl angle as a function from the distance of the curl
       * ray (i.e. the page crease)
       */
      turn_angle = (rx / d->radius * G_PI_2) - G_PI_2;
      shade = (sin (turn_angle) * 96) + 159;

      /* Add a gradient that makes it look like lighting and hides the switch
       * between textures.
       */
      vertex->color.red = shade;
      vertex->color.green = shade;
      vertex->color.blue = shade;
    }

  if (rx > 0)
    {
      /* Make the curl radius smaller as more circles are formed (stops
       * z-fighting and looks cool)
       */
      gfloat small_radius = d->radius - (turn_angle * 2) / G_PI;

      /* Calculate a point on a cylinder (maybe make this a cone at some point)
       * and rotate it by the specified angle.
       */
      rx = (small_radius * cos (turn_angle)) + d->radius;
      vertex->x = (rx * cos (d->angle)) - (ry * sin (d->angle)) + cx;
      vertex->y = (rx * sin (d->angle)) + (ry * cos (d->angle)) + cy;
      vertex->z = (small_radius * sin (turn_angle)) + d->radius;
    }
}
