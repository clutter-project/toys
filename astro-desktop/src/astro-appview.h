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

#ifndef _HAVE_ASTRO_APPVIEW_H
#define _HAVE_ASTRO_APPVIEW_H

G_BEGIN_DECLS

#define ASTRO_TYPE_APPVIEW astro_appview_get_type()

#define ASTRO_APPVIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  ASTRO_TYPE_APPVIEW, \
  AstroAppview))

#define ASTRO_APPVIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
  ASTRO_TYPE_APPVIEW, \
  AstroAppviewClass))

#define ASTRO_IS_APPVIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  ASTRO_TYPE_APPVIEW))

#define ASTRO_IS_APPVIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  ASTRO_TYPE_APPVIEW))

#define ASTRO_APPVIEW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
  ASTRO_TYPE_APPVIEW, \
  AstroAppviewClass))

typedef struct _AstroAppview AstroAppview;
typedef struct _AstroAppviewClass AstroAppviewClass;
typedef struct _AstroAppviewPrivate AstroAppviewPrivate;

struct _AstroAppview
{
  ClutterGroup         parent;
	
  /*< private >*/
  AstroAppviewPrivate   *priv;
};

struct _AstroAppviewClass 
{
  /*< private >*/
  ClutterGroupClass parent_class;

  /*< signals >*/
  void (*launch_app) (AstroAppview *view, AstroApplication *application);
}; 

GType astro_appview_get_type (void) G_GNUC_CONST;

ClutterActor *  astro_appview_new          (void);

void            astro_appview_set_app_list (AstroAppview *view,
                                            GList        *apps);
void            astro_appview_advance      (AstroAppview *view,
                                            gint          n);


G_END_DECLS

#endif
