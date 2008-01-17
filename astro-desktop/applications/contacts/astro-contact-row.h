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


#ifndef _HAVE_ASTRO_CONTACT_ROW_H
#define _HAVE_ASTRO_CONTACT_ROW_H

G_BEGIN_DECLS

#define ASTRO_TYPE_CONTACT_ROW astro_contact_row_get_type()

#define ASTRO_CONTACT_ROW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  ASTRO_TYPE_CONTACT_ROW, \
  AstroContactRow))

#define ASTRO_CONTACT_ROW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
  ASTRO_TYPE_CONTACT_ROW, \
  AstroContactRowClass))

#define ASTRO_IS_CONTACT_ROW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  ASTRO_TYPE_CONTACT_ROW))

#define ASTRO_IS_CONTACT_ROW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  ASTRO_TYPE_CONTACT_ROW))

#define ASTRO_CONTACT_ROW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
  ASTRO_TYPE_CONTACT_ROW, \
  AstroContactRowClass))

#define ROW_HEIGHT (CSH()/11)

typedef struct _AstroContactRow AstroContactRow;
typedef struct _AstroContactRowClass AstroContactRowClass;
typedef struct _AstroContactRowPrivate AstroContactRowPrivate;

struct _AstroContactRow
{
  ClutterGroup       parent;
	
  /*< private >*/
  AstroContactRowPrivate   *priv;
};

struct _AstroContactRowClass 
{
  /*< private >*/
  ClutterGroupClass parent_class;
}; 

GType astro_contact_row_get_type (void) G_GNUC_CONST;

ClutterActor *  astro_contact_row_new        (const gchar  *name,
                                              GdkPixbuf    *icon);
void            astro_contact_row_set_active (AstroContactRow *row,
                                              gboolean         axtive);

G_END_DECLS

#endif
