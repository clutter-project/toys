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

#ifndef _HAVE_WH_SLIDER_MENU_H
#define _HAVE_WH_SLIDER_MENU_H

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define WH_TYPE_SLIDER_MENU wh_slider_menu_get_type()

#define WH_SLIDER_MENU(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  WH_TYPE_SLIDER_MENU, WHSliderMenu))

#define WH_SLIDER_MENU_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  WH_TYPE_SLIDER_MENU, WHSliderMenuClass))

#define WH_IS_SLIDER_MENU(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  WH_TYPE_SLIDER_MENU))

#define WH_IS_SLIDER_MENU_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  WH_TYPE_SLIDER_MENU))

#define WH_SLIDER_MENU_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  WH_TYPE_SLIDER_MENU, WHSliderMenuClass))

typedef struct _WHSliderMenu WHSliderMenu;
typedef struct _WHSliderMenuClass WHSliderMenuClass;
typedef struct _WHSliderMenuPrivate WHSliderMenuPrivate;

struct _WHSliderMenu
{
  ClutterActor         parent;
  /*< private >*/
  WHSliderMenuPrivate   *priv;
};

struct _WHSliderMenuClass 
{
  /*< private >*/
  ClutterActorClass parent_class;

  void (*_wh_slider_menu_1) (void);
  void (*_wh_slider_menu_2) (void);
  void (*_wh_slider_menu_3) (void);
  void (*_wh_slider_menu_4) (void);
}; 

GType wh_slider_menu_get_type (void) G_GNUC_CONST;

typedef void (*WHSliderMenuSelectedFunc) (WHSliderMenu *menu,
					  ClutterActor      *actor,
					  gpointer           userdata);

ClutterActor*
wh_slider_menu_new (const gchar *font);

void
wh_slider_menu_add_option (WHSliderMenu            *menu, 
			   const gchar             *text,
			   WHSliderMenuSelectedFunc selected,
			   gpointer                 userdata);

void
wh_slider_menu_activate (WHSliderMenu *menu,
			 gint          entry_num);

void
wh_slider_menu_advance (WHSliderMenu *menu, gint n);

G_END_DECLS

#endif
