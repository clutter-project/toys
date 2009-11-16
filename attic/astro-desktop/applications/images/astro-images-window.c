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


#include "astro-images-window.h"

#include <math.h>
#include <string.h>
#include <libastro-desktop/astro.h>
#include <libastro-desktop/astro-application.h>
#include <libastro-desktop/astro-window.h>
#include <libastro-desktop/astro-behave.h>
#include <libastro-desktop/tidy-texture-frame.h>

G_DEFINE_TYPE (AstroImagesWindow, astro_images_window, ASTRO_TYPE_WINDOW);

#define ASTRO_IMAGES_WINDOW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), ASTRO_TYPE_IMAGES_WINDOW, AstroImagesWindowPrivate))

struct _AstroImagesWindowPrivate
{
  gint i;
};



/* GObject stuff */
static void
astro_images_window_class_init (AstroImagesWindowClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (gobject_class, sizeof (AstroImagesWindowPrivate));
}

static void
astro_images_window_init (AstroImagesWindow *window)
{
  AstroImagesWindowPrivate *priv;
    
  priv = window->priv = ASTRO_IMAGES_WINDOW_GET_PRIVATE (window);

  clutter_actor_set_position (CLUTTER_ACTOR (window), 0, 0);
  clutter_actor_show_all (CLUTTER_ACTOR (window));
}

AstroWindow * 
astro_images_window_new (void)
{
  AstroWindow *images_window =  g_object_new (ASTRO_TYPE_IMAGES_WINDOW,
									                       NULL);

  return images_window;
}
