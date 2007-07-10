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

#include "aaina-behave.h"

#include "aaina-photo.h"

G_DEFINE_TYPE (AainaPhoto, aaina_photo, CLUTTER_TYPE_GROUP);

#define AAINA_PHOTO_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
	AAINA_TYPE_PHOTO, \
	AainaPhotoPrivate))
	
static GdkPixbuf	*default_pic = NULL;

struct _AainaPhotoPrivate
{
  GdkPixbuf    *pixbuf;
  gboolean      visible;

  gchar        *id;
  gchar        *title;
  gchar        *author;
  gchar        *date;

  gboolean      viewed;

  ClutterActor *dim;
  ClutterActor *texture; 
  ClutterActor *bg;

  gdouble       scale;

  /* Variables for aaina_photo_save/restore */
  gdouble       save_scale;
  gint          save_x;
  gint          save_y;
  guint8        save_dim;
  gint          save_depth;

  
  ClutterTimeline *zoom_time;
  ClutterTimeline *restore_time;
  gint          temp_x;
  gint          temp_y;
};

enum
{
  PROP_0,
  PROP_PIXBUF,
  PROP_ID,
  PROP_TITLE,
  PROP_DATE,
  PROP_AUTHOR,
  PROP_VIEWED
};

enum
{
  PHOTO_ZOOMED,
  PHOTO_RESTORED,

  LAST_SIGNAL
};

static guint _photo_signals[LAST_SIGNAL] = { 0 };

guint8
aaina_photo_get_dim (AainaPhoto *photo)
{
  AainaPhotoPrivate *priv;

  g_return_if_fail (AAINA_IS_PHOTO (photo));
  priv = photo->priv;

  return clutter_actor_get_opacity (priv->dim);
}

void
aaina_photo_set_dim (AainaPhoto *photo, guint8 dim_level)
{
  AainaPhotoPrivate *priv;

  g_return_if_fail (AAINA_IS_PHOTO (photo));
  priv = photo->priv;

  clutter_actor_set_opacity (priv->dim, dim_level);
}

void
aaina_photo_save (AainaPhoto *photo)
{
  AainaPhotoPrivate *priv;

  g_return_if_fail (AAINA_IS_PHOTO (photo));
  priv = photo->priv;
  
  /* Make the x value slightly more the the left as it is constantly moving */
  priv->save_x = clutter_actor_get_x (CLUTTER_ACTOR (photo)) - 150;
  priv->save_y = clutter_actor_get_y (CLUTTER_ACTOR (photo));
  clutter_actor_get_scale (CLUTTER_ACTOR (photo), 
                           &priv->save_scale, 
                           &priv->save_scale);
  priv->save_dim = clutter_actor_get_opacity (priv->dim);
  priv->save_depth = clutter_actor_get_depth (CLUTTER_ACTOR (photo));
}

void
aaina_photo_restore (AainaPhoto *photo)
{
  AainaPhotoPrivate *priv;

  g_return_if_fail (AAINA_IS_PHOTO (photo));
  priv = photo->priv;

  priv->temp_x = clutter_actor_get_x (CLUTTER_ACTOR (photo));
  priv->temp_y = clutter_actor_get_y (CLUTTER_ACTOR (photo));

  clutter_timeline_start (priv->restore_time);
}

static void
aaina_photo_alpha_restore (ClutterBehaviour *behave, 
                           guint32 alpha_value, 
                           AainaPhoto *photo)
{
  AainaPhotoPrivate *priv;
  gfloat factor;
  gdouble scale, new_scale;
  gint x, y;
  guint width, height;
  gint new_x, new_y;
  
  g_return_if_fail (AAINA_IS_PHOTO (photo));
  priv = photo->priv;

  factor = (gfloat)alpha_value / CLUTTER_ALPHA_MAX_ALPHA;

  x = priv->temp_x; //clutter_actor_get_x (CLUTTER_ACTOR (photo));
  y = priv->temp_y; //clutter_actor_get_y (CLUTTER_ACTOR (photo));
  clutter_actor_get_size (CLUTTER_ACTOR (photo), &width, &height);
  clutter_actor_get_scale (CLUTTER_ACTOR (photo), &scale, &scale);

  new_x = priv->save_x;
  new_y = priv->save_y;

  if (x > new_x)
    new_x = x - ((x - new_x) * factor);
  else
    new_x = x + ((new_x - x) * factor);

  if (y > new_y)
    new_y = y - ((y - new_y) * factor);
  else
    new_y = y + ((new_y - y) * factor);

  //new_scale = scale - ((scale - priv->save_scale) * factor);
  new_scale = 1.0 - ((1-priv->save_scale) * factor);

  clutter_actor_set_position (CLUTTER_ACTOR (photo), new_x, new_y);
  clutter_actor_set_scale (CLUTTER_ACTOR (photo), new_scale, new_scale);
  
  if (factor == 1)
  {
    clutter_actor_set_opacity (priv->dim, priv->save_dim);
    clutter_actor_set_depth (CLUTTER_ACTOR (photo), priv->save_depth);
    g_signal_emit (G_OBJECT (photo), _photo_signals[PHOTO_RESTORED], 0);
  }
  clutter_actor_queue_redraw (CLUTTER_ACTOR (photo));
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
  GError *err = NULL;
    
  g_return_if_fail (AAINA_IS_PHOTO (photo));
  g_return_if_fail (GDK_IS_PIXBUF (pixbuf));
  priv = photo->priv;

  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);

  clutter_actor_set_size (priv->dim, width+20, height+20);
  clutter_actor_set_position (priv->dim, 0, 0);
  
  clutter_actor_set_size (priv->bg, width+20, height+20);
  clutter_actor_set_position (priv->bg, 0, 0);
  
  clutter_texture_set_pixbuf (CLUTTER_TEXTURE (priv->texture), pixbuf, &err);
  if (err)
    g_warning ("%s\n", err->message);
  clutter_actor_set_position (priv->texture, 10, 10);
  clutter_actor_show (priv->texture);
}

void
aaina_photo_zoom (AainaPhoto *photo)
{
  AainaPhotoPrivate *priv;

  g_return_if_fail (AAINA_IS_PHOTO (photo));
  priv = photo->priv;

  clutter_timeline_start (priv->zoom_time);
}

static void
aaina_photo_alpha_zoom (ClutterBehaviour *behave, 
                        guint32 alpha_value, 
                        AainaPhoto *photo)
{
  AainaPhotoPrivate *priv;
  gfloat factor;
  gdouble scale, new_scale;
  gint x, y;
  guint width, height;
  gint new_x, new_y;
  
  g_return_if_fail (AAINA_IS_PHOTO (photo));
  priv = photo->priv;

  factor = (gfloat)alpha_value / CLUTTER_ALPHA_MAX_ALPHA;

  x = clutter_actor_get_x (CLUTTER_ACTOR (photo));
  y = clutter_actor_get_y (CLUTTER_ACTOR (photo));
  clutter_actor_get_size (CLUTTER_ACTOR (photo), &width, &height);
  clutter_actor_get_scale (CLUTTER_ACTOR (photo), &scale, &scale);

  new_x = CLUTTER_STAGE_WIDTH () / 4;
  new_y = CLUTTER_STAGE_HEIGHT () /4;


  if (x > new_x)
    new_x = x - ((x - new_x) * factor);
  else
    new_x = x + ((new_x - x) * factor);

  if (y > new_y)
    new_y = y - ((y - new_y) * factor);
  else
    new_y = y + ((new_y - y) * factor);

  new_scale = scale + ((1 - scale) * factor);
  if (new_scale < scale)
    new_scale = scale;

  clutter_actor_set_position (CLUTTER_ACTOR (photo), new_x, new_y);
  clutter_actor_set_scale (CLUTTER_ACTOR (photo), new_scale, new_scale);
  clutter_actor_set_opacity (priv->dim,
                            clutter_actor_get_opacity (priv->dim)
      -(clutter_actor_get_opacity (priv->dim))*factor);

  if (factor == 1)
    g_signal_emit (G_OBJECT (photo), _photo_signals[PHOTO_ZOOMED], 0);
  
  clutter_actor_queue_redraw (CLUTTER_ACTOR (photo));
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

  clutter_actor_get_scale (actor, &priv->scale, &priv->scale);

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
        aaina_photo_set_pixbuf (AAINA_PHOTO (object), priv->pixbuf);
      break;

    case PROP_ID:
      if (priv->id)
        g_free (priv->id);
      priv->id = g_strdup (g_value_get_string (value));
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
    case PROP_ID:
      g_value_set_string (value, priv->id);
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
    PROP_ID,
    g_param_spec_string ("id",
                         "The id",
                         "The id of the photo",
                         NULL,
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

  _photo_signals[PHOTO_ZOOMED] = 
    g_signal_new ("photo_zoomed",
                  G_OBJECT_CLASS_TYPE (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (AainaPhotoClass, photo_zoomed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  
  _photo_signals[PHOTO_RESTORED] = 
    g_signal_new ("photo_restored",
                  G_OBJECT_CLASS_TYPE (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (AainaPhotoClass, photo_restored),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
aaina_photo_init (AainaPhoto *photo)
{
  AainaPhotoPrivate *priv;
  ClutterColor white = {0xff, 0xff, 0xff, 0xff};
  ClutterColor black = {0x00, 0x00, 0x00, 0x00};
  gint width, height;
  ClutterAlpha *alpha;
  ClutterBehaviour *behave;
  GdkPixbuf *pixbuf;

  g_return_if_fail (AAINA_IS_PHOTO (photo));
  priv = AAINA_PHOTO_GET_PRIVATE (photo);

  photo->priv = priv;

  priv->pixbuf = NULL;
  priv->title = priv->author = priv->date = NULL;
  priv->visible = TRUE;

  width = CLUTTER_STAGE_WIDTH ()/2;
  height = CLUTTER_STAGE_HEIGHT ()/2;

  priv->bg = clutter_rectangle_new_with_color (&white);
  clutter_group_add (CLUTTER_GROUP (photo), priv->bg);
  clutter_actor_show (priv->bg);
    
  priv->texture = clutter_texture_new ();
  clutter_actor_set_size (priv->texture, width, height);
  clutter_actor_set_position (priv->texture, 0, 0);
  clutter_group_add (CLUTTER_GROUP (photo), priv->texture);

  priv->dim = clutter_rectangle_new_with_color (&black);
  clutter_group_add (CLUTTER_GROUP (photo), priv->dim);
  clutter_actor_show (priv->dim);

  clutter_actor_show (CLUTTER_ACTOR (photo));

  priv->zoom_time = clutter_timeline_new (60, 30);
  alpha = clutter_alpha_new_full (priv->zoom_time,
                                  alpha_sine_inc_func,
                                  NULL, NULL);
  behave = aaina_behave_new (alpha, 
                             (AainaBehaveAlphaFunc)aaina_photo_alpha_zoom,
                             (gpointer)photo);

  priv->restore_time = clutter_timeline_new (120, 30);
  alpha = clutter_alpha_new_full (priv->restore_time,
                                  alpha_sine_inc_func,
                                  NULL, NULL);
  behave = aaina_behave_new (alpha, 
                             (AainaBehaveAlphaFunc)aaina_photo_alpha_restore,
                             (gpointer)photo);
}

ClutterActor*
aaina_photo_new (void)
{
  AainaPhoto         *photo;

  photo = g_object_new (AAINA_TYPE_PHOTO, NULL);
  
  return CLUTTER_ACTOR (photo);
}

