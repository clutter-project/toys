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

#include <GL/gl.h>

#include "aaina-photo.h"

G_DEFINE_TYPE (AainaPhoto, aaina_photo, CLUTTER_TYPE_GROUP);

#define AAINA_PHOTO_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
	AAINA_TYPE_PHOTO, \
	AainaPhotoPrivate))
	
static GdkPixbuf	*default_pic = NULL;

struct _AainaPhotoPrivate
{
  gboolean      visible;
	
  GdkPixbuf    *pixbuf;

  gchar        *title;
  gchar        *author;
  gchar        *date;

  gboolean      viewed;

  ClutterActor *texture; 
  ClutterActor *bg;

  gdouble       scale;

  /* Variables for aaina_photo_save/restore */
  gdouble       save_scale;
  gint          save_x;
  gint          save_y;

};

enum
{
  PROP_0,
  PROP_PIXBUF,
  PROP_TITLE,
  PROP_DATE,
  PROP_AUTHOR,
  PROP_VIEWED
};

enum
{
  NAME,
  LAST_SIGNAL
};

static guint _photo_signals[LAST_SIGNAL] = { 0 };

void
aaina_photo_save (AainaPhoto *photo)
{
  AainaPhotoPrivate *priv;

  g_return_if_fail (AAINA_IS_PHOTO (photo));
  priv = photo->priv;

  priv->save_x = clutter_actor_get_x (CLUTTER_ACTOR (photo));
  priv->save_y = clutter_actor_get_y (CLUTTER_ACTOR (photo));
  clutter_actor_get_scale (CLUTTER_ACTOR (photo), 
                           &priv->save_scale, 
                           &priv->save_scale);
}

void
aaina_photo_restore (AainaPhoto *photo)
{
  AainaPhotoPrivate *priv;

  g_return_if_fail (AAINA_IS_PHOTO (photo));
  priv = photo->priv;

  clutter_actor_set_scale (CLUTTER_ACTOR (photo), 
                           priv->save_scale, 
                           priv->save_scale);
  clutter_actor_set_position (CLUTTER_ACTOR (photo), 
                             priv->save_x, 
                             priv->save_y);
}

gdouble
aaina_photo_get_scale (AainaPhoto *photo)
{
  g_return_val_if_fail (AAINA_IS_PHOTO (photo), 1.0);
  return photo->priv->scale;
}
void
aaina_photo_set_scale (AainaPhoto *photo, gdouble scale)
{
  g_return_if_fail (AAINA_IS_PHOTO (photo));
  photo->priv->scale = scale;

  clutter_actor_queue_redraw (CLUTTER_ACTOR (photo));
}

gboolean
aaina_photo_get_viewed (AainaPhoto *photo)
{
  g_return_val_if_fail (AAINA_IS_PHOTO (photo), TRUE);

  return photo->priv->viewed;
}

void
aaina_photo_set_viewed (AainaPhoto *photo, gboolean viewed)
{
  g_return_if_fail (AAINA_IS_PHOTO (photo));

  photo->priv->viewed = viewed;
}

void
aaina_photo_set_pixbuf (AainaPhoto *photo, GdkPixbuf *pixbuf)
{
  AainaPhotoPrivate *priv;
  gint width, height;
  gint x, y;

  g_return_if_fail (AAINA_IS_PHOTO (photo));
  g_return_if_fail (GDK_IS_PIXBUF (pixbuf));
  priv = photo->priv;

  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);
  
  clutter_texture_set_pixbuf (CLUTTER_TEXTURE (priv->texture), pixbuf, NULL);
  clutter_actor_set_size (priv->texture, width, height);
  clutter_actor_set_position (priv->texture, 0, 0);
}

/* GObject stuff */
/*
static void
aaina_photo_paint (ClutterActor *actor)
{
  AainaPhotoPrivate *priv;

  priv = AAINA_PHOTO (actor)->priv;

  glPushMatrix ();

  gfloat x, y;
  guint width = CLUTTER_STAGE_WIDTH ()/2;
  guint height = CLUTTER_STAGE_HEIGHT ()/2;

  x = (priv->scale *width) - (width);
  x /= 2;
  x *= -1;

  y = (priv->scale *height) - (height);
  y /= 2;
  y *= -1;

  glTranslatef (x, y, 0);
  glScalef (priv->scale, priv->scale, 1);

  gint i;
  gint len = clutter_group_get_n_children (CLUTTER_GROUP (actor));
  for (i = 0; i <len; i++)
  {
    ClutterActor *child;
    child = clutter_group_get_nth_child (CLUTTER_GROUP (actor), i);

    if (child)
      clutter_actor_paint (child);
  }
  glPopMatrix ();
}
*/
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
      if (priv->pixbuf)
        clutter_texture_set_pixbuf (CLUTTER_TEXTURE (priv->texture),
                                    priv->pixbuf, 
                                    NULL);
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
    case PROP_VIEWED:
        priv->viewed = g_value_get_boolean (value);
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
    case PROP_VIEWED:
      g_value_set_boolean (value, priv->viewed);
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

  //actor_class->paint          = aaina_photo_paint;

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
  
  g_object_class_install_property (
    gobject_class,
    PROP_VIEWED,
    g_param_spec_boolean ("viewed",
                         "If viewed",
                         "The photo has been view",
                         FALSE,
                         G_PARAM_CONSTRUCT|G_PARAM_READWRITE));
}

static void
aaina_photo_init (AainaPhoto *photo)
{
  AainaPhotoPrivate *priv;
  ClutterColor white = {0xff, 0xff, 0xff, 0xff};
  gint width, height;

  g_return_if_fail (AAINA_IS_PHOTO (photo));
  priv = AAINA_PHOTO_GET_PRIVATE (photo);

  photo->priv = priv;

  priv->pixbuf = NULL;
  priv->title = priv->author = priv->date = NULL;

  width = CLUTTER_STAGE_WIDTH ()/2;
  height = CLUTTER_STAGE_HEIGHT ()/2;

  priv->bg = clutter_rectangle_new_with_color (&white);
  clutter_actor_set_size (priv->bg, width, height);
  clutter_actor_set_position (priv->bg, -10, -10);
  //clutter_group_add (CLUTTER_GROUP (photo), priv->bg);

  priv->texture = clutter_texture_new ();
  clutter_actor_set_size (priv->texture, width, height);
  clutter_actor_set_position (priv->texture, 0, 0);
  clutter_group_add (CLUTTER_GROUP (photo), priv->texture);

  
  clutter_actor_show_all (CLUTTER_ACTOR (photo));
}

ClutterActor*
aaina_photo_new (void)
{
  AainaPhoto         *photo;

  photo = g_object_new (AAINA_TYPE_PHOTO, NULL);
  
  return CLUTTER_ACTOR (photo);
}

