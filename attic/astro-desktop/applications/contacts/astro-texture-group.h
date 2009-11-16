/*
 * Copyright (C) 2007 OpenedHand Limited
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Neil Jagdish Patel <njp@o-hand.com>
 */


#include <glib.h>
#include <clutter/clutter.h>

#include "astro-contacts-window.h"

#ifndef _HAVE_ASTRO_TEXTURE_GROUP_H
#define _HAVE_ASTRO_TEXTURE_GROUP_H

G_BEGIN_DECLS

#define ASTRO_TYPE_TEXTURE_GROUP astro_texture_group_get_type()

#define ASTRO_TEXTURE_GROUP(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  ASTRO_TYPE_TEXTURE_GROUP, \
  AstroTextureGroup))

#define ASTRO_TEXTURE_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
  ASTRO_TYPE_TEXTURE_GROUP, \
  AstroTextureGroupClass))

#define ASTRO_IS_TEXTURE_GROUP(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  ASTRO_TYPE_TEXTURE_GROUP))

#define ASTRO_IS_TEXTURE_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  ASTRO_TYPE_TEXTURE_GROUP))

#define ASTRO_TEXTURE_GROUP_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
  ASTRO_TYPE_TEXTURE_GROUP, \
  AstroTextureGroupClass))

#define ROW_HEIGHT (CSH()/11)

typedef struct _AstroTextureGroup AstroTextureGroup;
typedef struct _AstroTextureGroupClass AstroTextureGroupClass;
typedef struct _AstroTextureGroupPrivate AstroTextureGroupPrivate;

struct _AstroTextureGroup
{
  ClutterGroup       parent;
	
  /*< private >*/
  AstroTextureGroupPrivate   *priv;
};

struct _AstroTextureGroupClass 
{
  /*< private >*/
  ClutterGroupClass parent_class;
}; 

GType astro_texture_group_get_type (void) G_GNUC_CONST;

ClutterActor *  astro_texture_group_new       ();
void            astro_texture_group_set_text (AstroTextureGroup *group,
                                              const gchar       *text);


G_END_DECLS

#endif
