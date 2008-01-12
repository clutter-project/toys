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

#ifndef _HAVE_ASTRO_PANEL_H
#define _HAVE_ASTRO_PANEL_H

G_BEGIN_DECLS

#define ASTRO_TYPE_PANEL astro_panel_get_type()

#define ASTRO_PANEL(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  ASTRO_TYPE_PANEL, \
  AstroPanel))

#define ASTRO_PANEL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
  ASTRO_TYPE_PANEL, \
  AstroPanelClass))

#define ASTRO_IS_PANEL(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  ASTRO_TYPE_PANEL))

#define ASTRO_IS_PANEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  ASTRO_TYPE_PANEL))

#define ASTRO_PANEL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
  ASTRO_TYPE_PANEL, \
  AstroPanelClass))

typedef struct _AstroPanel AstroPanel;
typedef struct _AstroPanelClass AstroPanelClass;
typedef struct _AstroPanelPrivate AstroPanelPrivate;

struct _AstroPanel
{
  ClutterGroup         parent;
	
  /*< private >*/
  AstroPanelPrivate   *priv;
};

struct _AstroPanelClass 
{
  /*< private >*/
  ClutterGroupClass parent_class;

  /*< signals >*/
  void (*show_home)    (AstroPanel *panel);
  void (*close_window) (AstroPanel *panel);
}; 

GType astro_panel_get_type (void) G_GNUC_CONST;

ClutterActor *  astro_panel_new       (void);

void            astro_panel_set_header (AstroPanel  *panel,
                                        const gchar *title,
                                        GdkPixbuf   *icon);
  
G_END_DECLS

#endif
