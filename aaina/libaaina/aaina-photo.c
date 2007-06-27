/*
* Authored By Neil Jagdish Patel <njp@o-hand.com>
 *
 * Copyright (C) 2007 OpenedHand
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "aaina-photo.h"

G_DEFINE_TYPE (AainaPhoto, aaina_photo, CLUTTER_TYPE_GROUP);

#define AAINA_PHOTO_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
	AAINA_TYPE_PHOTO, \
	AainaPhotoPrivate))
	
static GdkPixbuf	*default_pic = NULL;

struct _AainaPhotoPrivate
{
  gboolean   visible;
	
  GdkPixbuf		*pixbuf;

  gchar       *title;
  gchar       *author;
  gchar       *date;
};

enum
{
  PROP_0,
  PROP_PIXBUF,
  PROP_TITLE,
  PROP_DATE,
  PROP_AUTHOR
};

enum
{
  NAME,
  LAST_SIGNAL
};

static guint _photo_signals[LAST_SIGNAL] = { 0 };

/* GObject stuff */
static void
aaina_photo_paint (ClutterActor *actor)
{
  ;
}

static void
aaina_photo_set_property (GObject      *object, 
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  AainaPhotoPrivate *priv;
  
  g_return_if_fail (AAINA_IS_PHOTO (object));
  priv = AAINA_PHOTO (object)->priv;

  switch (prop_id)
  {
    case PROP_PIXBUF:
      priv->pixbuf = g_value_get_object (value);
      break;
    case PROP_TITLE:
      if (priv->title)
        g_free (priv->title);
      priv->title = g_strdup (g_value_get_string (value));
      break;
    case PROP_AUTHOR:
      if (priv->author)
        g_free (priv->author);
      priv->author = g_strdup (g_value_get_string (value));
      break;
    case PROP_DATE:
      if (priv->date)
        g_free (priv->date);
      priv->date = g_strdup (g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
aaina_photo_get_property (GObject    *object, 
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  AainaPhotoPrivate *priv;
  
  g_return_if_fail (AAINA_IS_PHOTO (object));
  priv = AAINA_PHOTO (object)->priv;

  switch (prop_id)
  {
    case PROP_PIXBUF:
      g_value_set_object (value, G_OBJECT (priv->pixbuf));
      break;
    case PROP_TITLE:
      g_value_set_string (value, priv->title);
      break;
    case PROP_AUTHOR:
      g_value_set_string (value, priv->author);
      break;
    case PROP_DATE:
      g_value_set_string (value, priv->date);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

  static void
aaina_photo_dispose (GObject *object)
{
 G_OBJECT_CLASS (aaina_photo_parent_class)->dispose (object);
}

static void
aaina_photo_finalize (GObject *object)
{
  G_OBJECT_CLASS (aaina_photo_parent_class)->finalize (object);
}

static void
aaina_photo_class_init (AainaPhotoClass *klass)
{
  GObjectClass    *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  actor_class->paint          = aaina_photo_paint;

  gobject_class->finalize     = aaina_photo_finalize;
  gobject_class->dispose      = aaina_photo_dispose;
  gobject_class->get_property = aaina_photo_get_property;
  gobject_class->set_property = aaina_photo_set_property;

  g_type_class_add_private (gobject_class, sizeof (AainaPhotoPrivate));

  g_object_class_install_property (
    gobject_class,
    PROP_PIXBUF,
    g_param_spec_object ("pixbuf",
                         "The pixbuf!",
                         "The GdkPixbuf to be shown",
                         GDK_TYPE_PIXBUF,
                         G_PARAM_CONSTRUCT|G_PARAM_READWRITE));

  g_object_class_install_property (
    gobject_class,
    PROP_TITLE,
    g_param_spec_string ("title",
                         "The title",
                         "The title of the photo",
                         NULL,
                         G_PARAM_CONSTRUCT|G_PARAM_READWRITE));

  g_object_class_install_property (
    gobject_class,
    PROP_DATE,
    g_param_spec_string ("date",
                         "The date",
                         "The date the photo was taken",
                         NULL,
                         G_PARAM_CONSTRUCT|G_PARAM_READWRITE));
  g_object_class_install_property (
    gobject_class,
    PROP_AUTHOR,
    g_param_spec_string ("author",
                         "The author",
                         "The athor of the photo",
                         NULL,
                         G_PARAM_CONSTRUCT|G_PARAM_READWRITE));

}

static void
aaina_photo_init (AainaPhoto *photo)
{
  AainaPhotoPrivate *priv;

  g_return_if_fail (AAINA_IS_PHOTO (photo));
  priv = AAINA_PHOTO_GET_PRIVATE (photo);

  photo->priv = priv;

  priv->pixbuf = NULL;
  priv->title = priv->author = priv->date = NULL;
}

ClutterActor*
aaina_photo_new (void)
{
  ClutterGroup         *photo;

  photo = g_object_new (AAINA_TYPE_PHOTO, NULL);
  
  return CLUTTER_ACTOR (photo);
}

