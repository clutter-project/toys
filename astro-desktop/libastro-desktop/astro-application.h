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

#include "astro-window.h"

#ifndef _HAVE_ASTRO_APPLICATION_H
#define _HAVE_ASTRO_APPLICATION_H

G_BEGIN_DECLS

#define ASTRO_TYPE_APPLICATION astro_application_get_type()

#define ASTRO_APPLICATION(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  ASTRO_TYPE_APPLICATION, \
  AstroApplication))

#define ASTRO_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
  ASTRO_TYPE_APPLICATION, \
  AstroApplicationClass))

#define ASTRO_IS_APPLICATION(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  ASTRO_TYPE_APPLICATION))

#define ASTRO_IS_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  ASTRO_TYPE_APPLICATION))

#define ASTRO_APPLICATION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
  ASTRO_TYPE_APPLICATION, \
  AstroApplicationClass))

typedef struct _AstroApplication AstroApplication;
typedef struct _AstroApplicationClass AstroApplicationClass;

struct _AstroApplication
{
  GObject         parent;
};

struct _AstroApplicationClass 
{
  /*< private >*/
  GObjectClass parent_class;

  /*< VTable, not signals >*/
  const gchar  * (*get_title)  (AstroApplication *application);
  void           (*set_title)  (AstroApplication *application, 
                                const gchar      *title);
  GdkPixbuf    * (*get_icon)   (AstroApplication *application);
  void           (*set_icon)   (AstroApplication *application,
                                GdkPixbuf        *icon);
  AstroWindow  * (*get_window) (AstroApplication *application);

  void           (*close)      (AstroApplication *application);
  
};

typedef GObject * (*AstroApplicationInitFunc) ();

GType astro_application_get_type (void) G_GNUC_CONST;

const gchar *      astro_application_get_title  (AstroApplication *application);
void               astro_application_set_title  (AstroApplication *application,
                                                 const gchar      *title);
GdkPixbuf   *      astro_application_get_icon   (AstroApplication *application);
void               astro_application_set_icon   (AstroApplication *application,
                                                 GdkPixbuf        *pixbuf);
AstroWindow *      astro_application_get_window (AstroApplication *application);
void               astro_application_close      (AstroApplication *application);
  
G_END_DECLS

#endif
