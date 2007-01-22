/*
 *
 *
 * Authored By XXXXX
 *
 * Copyright (C) 2006 XXXXXX
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _HAVE_CLUTTER_DISK_H
#define _HAVE_CLUTTER_DISK_H

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define CLUTTER_TYPE_DISK clutter_disk_get_type()

#define CLUTTER_DISK(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  CLUTTER_TYPE_DISK, ClutterDisk))

#define CLUTTER_DISK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  CLUTTER_TYPE_DISK, ClutterDiskClass))

#define CLUTTER_IS_TEMPLATE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  CLUTTER_TYPE_DISK))

#define CLUTTER_IS_TEMPLATE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  CLUTTER_TYPE_DISK))

#define CLUTTER_DISK_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  CLUTTER_TYPE_DISK, ClutterDiskClass))

typedef struct _ClutterDisk ClutterDisk;
typedef struct _ClutterDiskClass ClutterDiskClass;
typedef struct _ClutterDiskPrivate ClutterDiskPrivate;

struct _ClutterDisk
{
  ClutterActor         parent;
  /*< private >*/
  ClutterDiskPrivate   *priv;
};

struct _ClutterDiskClass 
{
  /*< private >*/
  ClutterActorClass parent_class;

  void (*_clutter_disk_1) (void);
  void (*_clutter_disk_2) (void);
  void (*_clutter_disk_3) (void);
  void (*_clutter_disk_4) (void);
}; 

GType clutter_disk_get_type (void) G_GNUC_CONST;

ClutterActor*
clutter_disk_new ();

G_END_DECLS

#endif
