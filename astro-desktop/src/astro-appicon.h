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
#include <libastro-desktop/astro-application.h>

#ifndef _HAVE_ASTRO_APPICON_H
#define _HAVE_ASTRO_APPICON_H

G_BEGIN_DECLS

#define ASTRO_TYPE_APPICON astro_appicon_get_type()

#define ASTRO_APPICON(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  ASTRO_TYPE_APPICON, \
  AstroAppicon))

#define ASTRO_APPICON_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
  ASTRO_TYPE_APPICON, \
  AstroAppiconClass))

#define ASTRO_IS_APPICON(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  ASTRO_TYPE_APPICON))

#define ASTRO_IS_APPICON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  ASTRO_TYPE_APPICON))

#define ASTRO_APPICON_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
  ASTRO_TYPE_APPICON, \
  AstroAppiconClass))

typedef struct _AstroAppicon AstroAppicon;
typedef struct _AstroAppiconClass AstroAppiconClass;
typedef struct _AstroAppiconPrivate AstroAppiconPrivate;

struct _AstroAppicon
{
  ClutterGroup         parent;
	
  /*< private >*/
  AstroAppiconPrivate   *priv;
};

struct _AstroAppiconClass 
{
  /*< private >*/
  ClutterGroupClass parent_class;

  /*< signals >*/
  void (*clicked)    (AstroAppicon *appicon, AstroApplication *application);
}; 

GType astro_appicon_get_type (void) G_GNUC_CONST;

ClutterActor *  astro_appicon_new       (AstroApplication *application);
const gchar  *  astro_appicon_get_title (AstroAppicon     *icon);
void            astro_appicon_set_blur  (AstroAppicon     *icon,
                                         gfloat            blur);
  
G_END_DECLS

#endif
