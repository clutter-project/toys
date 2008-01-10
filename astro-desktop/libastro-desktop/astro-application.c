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


#include "astro-application.h"

#include "astro-defines.h"

G_DEFINE_ABSTRACT_TYPE (AstroApplication, astro_application, G_TYPE_OBJECT);
	
enum
{
  PROP_0,
  PROP_TITLE,
  PROP_ICON
};

/* Public Functions */
const gchar *   
astro_application_get_title (AstroApplication *application)
{
  AstroApplicationClass *klass;

  g_return_val_if_fail (ASTRO_IS_APPLICATION (application), NULL);
  
  klass = ASTRO_APPLICATION_GET_CLASS (application);
  g_return_val_if_fail (klass->get_title != NULL, NULL);

  return klass->get_title (application);
}

void            
astro_application_set_title (AstroApplication *application, const gchar *title)
{
  AstroApplicationClass *klass;

  g_return_if_fail (ASTRO_IS_APPLICATION (application));
  g_return_if_fail (title);
  
  klass = ASTRO_APPLICATION_GET_CLASS (application);
  g_return_if_fail (klass->set_title != NULL);

  klass->set_title (application, title);
  /* FIXME: emit signal */
}

GdkPixbuf *     
astro_application_get_icon  (AstroApplication *application)
{
  AstroApplicationClass *klass;

  g_return_val_if_fail (ASTRO_IS_APPLICATION (application), NULL);
  
  klass = ASTRO_APPLICATION_GET_CLASS (application);
  g_return_val_if_fail (klass->get_icon != NULL, NULL);

  return klass->get_icon (application);
}

void            
astro_application_set_icon  (AstroApplication *application, 
                             GdkPixbuf        *icon)
{
  AstroApplicationClass *klass;

  g_return_if_fail (ASTRO_IS_APPLICATION (application));
  g_return_if_fail (GDK_IS_PIXBUF (icon));
  
  klass = ASTRO_APPLICATION_GET_CLASS (application);
  g_return_if_fail (klass->set_icon != NULL);

  klass->set_icon (application, icon);
  /* FIXME: emit signal */
}

AstroWindow *     
astro_application_get_window (AstroApplication *application)
{
  AstroApplicationClass *klass;

  g_return_val_if_fail (ASTRO_IS_APPLICATION (application), NULL);
  
  klass = ASTRO_APPLICATION_GET_CLASS (application);
  g_return_val_if_fail (klass->get_window != NULL, NULL);

  return klass->get_window (application);
}

void 
astro_application_close (AstroApplication *application)
{
  AstroApplicationClass *klass;

  g_return_if_fail (ASTRO_IS_APPLICATION (application));
    
  klass = ASTRO_APPLICATION_GET_CLASS (application);
  g_return_if_fail (klass->close != NULL);

  klass->close (application);
}
 
/* GObject stuff */
static void
astro_application_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  AstroApplication *app = ASTRO_APPLICATION (object);

  g_return_if_fail (ASTRO_IS_APPLICATION (object));

  switch (prop_id)
  {
   case PROP_TITLE:
      g_value_set_string (value, astro_application_get_title (app));
      break;
    case PROP_ICON:
      g_value_set_object (value, astro_application_get_icon (app));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
astro_application_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  AstroApplication *app = ASTRO_APPLICATION (object);

  g_return_if_fail (ASTRO_IS_APPLICATION (object));
  
  switch (prop_id)
  {
   case PROP_TITLE:
      astro_application_set_title (app, g_value_get_string (value));
      break;
    case PROP_ICON:
      astro_application_set_icon (app, g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
astro_application_class_init (AstroApplicationClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = astro_application_set_property;
  gobject_class->get_property = astro_application_get_property;

  /* Class properties */
  g_object_class_install_property (gobject_class,
    PROP_TITLE,
    g_param_spec_string ("title",
                         "Title",
                         "The title of the application",
                         "Untitled",
                         G_PARAM_CONSTRUCT|G_PARAM_READWRITE));
  
  g_object_class_install_property (gobject_class,
    PROP_ICON,
    g_param_spec_object ("icon",
                         "Icon",
                         "The icon of the application",
                         CLUTTER_TYPE_ACTOR,
                         G_PARAM_READWRITE));
}

static void
astro_application_init (AstroApplication *application)
{
  ;
}

