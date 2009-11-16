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


#include "astro-contacts.h"

#include <libastro-desktop/astro-defines.h>
#include <libastro-desktop/astro-application.h>
#include <libastro-desktop/astro-window.h>

#include "astro-contacts-window.h"

G_DEFINE_TYPE (AstroContacts, astro_contacts, ASTRO_TYPE_APPLICATION);

#define ASTRO_CONTACTS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
        ASTRO_TYPE_CONTACTS, AstroContactsPrivate))
	
struct _AstroContactsPrivate
{
  const gchar *title;
  GdkPixbuf *icon;
  ClutterActor *window;
};

/* Public Functions */

/* Private functions */
static const gchar *
get_title (AstroApplication *app)
{
  g_return_val_if_fail (ASTRO_IS_CONTACTS (app), NULL);

  return ASTRO_CONTACTS (app)->priv->title;
}

static void
set_title (AstroApplication *app, const gchar *title)
{
  g_return_if_fail (ASTRO_IS_CONTACTS (app));
  g_return_if_fail (title);

  ASTRO_CONTACTS (app)->priv->title = g_strdup (title);
}

static GdkPixbuf *
get_icon (AstroApplication *app)
{
  g_return_val_if_fail (ASTRO_IS_CONTACTS (app), NULL);

  return ASTRO_CONTACTS (app)->priv->icon;
}

static void
set_icon (AstroApplication *app, GdkPixbuf *icon)
{
  g_return_if_fail (ASTRO_IS_CONTACTS (app));
  g_return_if_fail (GDK_IS_PIXBUF (icon));

  ASTRO_CONTACTS (app)->priv->icon = icon;
}

static AstroWindow *
get_window (AstroApplication *app)
{
  AstroContactsPrivate *priv;
  ClutterActor *window = NULL;

  g_return_val_if_fail (ASTRO_IS_CONTACTS (app), NULL);
  priv = ASTRO_CONTACTS (app)->priv;

  if (CLUTTER_IS_ACTOR (priv->window))
    window = priv->window;
  else
    {
      window = CLUTTER_ACTOR (astro_contacts_window_new ());
    }

  ASTRO_CONTACTS (app)->priv->window = window;

  return ASTRO_WINDOW (window);
}

static void
close (AstroApplication *app)
{
  AstroContactsPrivate *priv;
  
  g_return_if_fail (ASTRO_IS_CONTACTS (app));
  priv = ASTRO_CONTACTS (app)->priv;
  
  if (CLUTTER_IS_ACTOR (priv->window))
    clutter_actor_destroy (priv->window);
}

/* GObject stuff */
static void
astro_contacts_class_init (AstroContactsClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
  AstroApplicationClass *app_class = ASTRO_APPLICATION_CLASS (klass);

  app_class->get_title = get_title;
  app_class->set_title = set_title;
  app_class->get_icon = get_icon;
  app_class->set_icon = set_icon;
  app_class->get_window = get_window;
  app_class->close = close;

  g_type_class_add_private (gobject_class, sizeof (AstroContactsPrivate));
}

static void
astro_contacts_init (AstroContacts *contacts)
{
  AstroContactsPrivate *priv;
  priv = contacts->priv = ASTRO_CONTACTS_GET_PRIVATE (contacts);

  priv->title = NULL;
  priv->icon = NULL;
  priv->window = NULL;
}

AstroApplication * 
astro_contacts_new (const gchar *title, GdkPixbuf *icon)
{
  AstroApplication *contacts =  g_object_new (ASTRO_TYPE_CONTACTS,
									                       NULL);

  astro_application_set_title (contacts, title);
  astro_application_set_icon (contacts, icon);

  return contacts;
}

