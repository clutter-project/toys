/*
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

#ifndef _HAVE_CLUTTER_SLIDER_MENU_H
#define _HAVE_CLUTTER_SLIDER_MENU_H

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define CLUTTER_TYPE_SLIDER_MENU clutter_slider_menu_get_type()

#define CLUTTER_SLIDER_MENU(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  CLUTTER_TYPE_SLIDER_MENU, ClutterSliderMenu))

#define CLUTTER_SLIDER_MENU_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  CLUTTER_TYPE_SLIDER_MENU, ClutterSliderMenuClass))

#define CLUTTER_IS_TEMPLATE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  CLUTTER_TYPE_SLIDER_MENU))

#define CLUTTER_IS_TEMPLATE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  CLUTTER_TYPE_SLIDER_MENU))

#define CLUTTER_SLIDER_MENU_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  CLUTTER_TYPE_SLIDER_MENU, ClutterSliderMenuClass))

typedef struct _ClutterSliderMenu ClutterSliderMenu;
typedef struct _ClutterSliderMenuClass ClutterSliderMenuClass;
typedef struct _ClutterSliderMenuPrivate ClutterSliderMenuPrivate;

struct _ClutterSliderMenu
{
  ClutterActor         parent;
  /*< private >*/
  ClutterSliderMenuPrivate   *priv;
};

struct _ClutterSliderMenuClass 
{
  /*< private >*/
  ClutterActorClass parent_class;

  void (*_clutter_slider_menu_1) (void);
  void (*_clutter_slider_menu_2) (void);
  void (*_clutter_slider_menu_3) (void);
  void (*_clutter_slider_menu_4) (void);
}; 

GType clutter_slider_menu_get_type (void) G_GNUC_CONST;

typedef void (*ClutterSliderMenuSelectedFunc) (ClutterSliderMenu *menu,
					       ClutterActor      *actor,
					       gpointer           userdata);

ClutterActor*
clutter_slider_menu_new (void);

void
clutter_slider_menu_append_actor (ClutterSliderMenu            *menu, 
				  ClutterActor                 *actor,
				  ClutterSliderMenuSelectedFunc selected,
				  gpointer                      userdata);

void
clutter_slider_menu_activate (ClutterSliderMenu *menu,
			      gint               entry_num);

void
clutter_slider_menu_advance (ClutterSliderMenu *menu, gint n);

G_END_DECLS

#endif
