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

#ifndef _HAVE_ASTRO_APPLET_MANAGER_H
#define _HAVE_ASTRO_APPLET_MANAGER_H

G_BEGIN_DECLS

#define ASTRO_TYPE_APPLET_MANAGER astro_applet_manager_get_type()

#define ASTRO_APPLET_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  ASTRO_TYPE_APPLET_MANAGER, \
  AstroAppletManager))

#define ASTRO_APPLET_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
  ASTRO_TYPE_APPLET_MANAGER, \
  AstroAppletManagerClass))

#define ASTRO_IS_APPLET_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  ASTRO_TYPE_APPLET_MANAGER))

#define ASTRO_IS_APPLET_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  ASTRO_TYPE_APPLET_MANAGER))

#define ASTRO_APPLET_MANAGER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
  ASTRO_TYPE_APPLET_MANAGER, \
  AstroAppletManagerClass))

typedef struct _AstroAppletManager AstroAppletManager;
typedef struct _AstroAppletManagerClass AstroAppletManagerClass;
typedef struct _AstroAppletManagerPrivate AstroAppletManagerPrivate;

struct _AstroAppletManager
{
  ClutterGroup         parent;
	
  /*< private >*/
  AstroAppletManagerPrivate   *priv;
};

struct _AstroAppletManagerClass 
{
  /*< private >*/
  ClutterGroupClass parent_class;
}; 

GType astro_applet_manager_get_type (void) G_GNUC_CONST;

ClutterActor *  astro_applet_manager_new       (void);

G_END_DECLS

#endif
