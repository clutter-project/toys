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

#ifndef __GPSG_SPHERE_H__
#define __GPSG_SPHERE_H__

#include <clutter/clutter.h>

G_BEGIN_DECLS

#define GPSG_TYPE_SPHERE (gpsg_sphere_get_type ())

#define GPSG_SPHERE(obj)						\
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPSG_TYPE_SPHERE, GpsgSphere))
#define GPSG_SPHERE_CLASS(klass)					\
  (G_TYPE_CHECK_CLASS_CAST ((klass), GPSG_TYPE_SPHERE, GpsgSphereClass))
#define GPSG_IS_SPHERE(obj)					        \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPSG_TYPE_SPHERE))
#define GPSG_IS_SPHERE_CLASS(klass)			                \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), GPSG_TYPE_SPHERE))
#define GPSG_SPHERE_GET_CLASS(obj)					\
  (G_TYPE_INSTANCE_GET_CLASS ((obj), GPSG_TYPE_SPHERE, GpsgSphereClass))

typedef struct _GpsgSphere GpsgSphere;
typedef struct _GpsgSpherePrivate GpsgSpherePrivate;
typedef struct _GpsgSphereClass GpsgSphereClass;

struct _GpsgSphere
{
  ClutterTexture parent_instance;

  GpsgSpherePrivate *priv;
};

struct _GpsgSphereClass
{
  ClutterTextureClass parent_class;
};

typedef enum
{
  GPSG_SPHERE_PAINT_LINES   = (1 << 0),
  GPSG_SPHERE_PAINT_TEXTURE = (1 << 1)
} GpsgSpherePaintFlags;

GType gpsg_sphere_get_type (void) G_GNUC_CONST;

ClutterActor *gpsg_sphere_new (void);

guint gpsg_sphere_get_depth (GpsgSphere *sphere);
void gpsg_sphere_set_depth (GpsgSphere *sphere, guint depth);

gfloat gpsg_sphere_get_flatness (GpsgSphere *sphere);
void gpsg_sphere_set_flatness (GpsgSphere *sphere, gfloat flatness);

GpsgSpherePaintFlags gpsg_sphere_get_paint_flags (GpsgSphere *sphere);
void gpsg_sphere_set_paint_flags (GpsgSphere *sphere,
                                  GpsgSpherePaintFlags flags);

void gpsg_sphere_get_lines_color (GpsgSphere *sphere,
                                  ClutterColor *color);
void gpsg_sphere_set_lines_color (GpsgSphere *sphere,
                                  const ClutterColor *color);

G_END_DECLS


#endif /* __GPSG_SPHERE_H__ */
