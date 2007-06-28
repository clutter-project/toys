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

#include "aaina-slide-show.h"

G_DEFINE_TYPE (AainaSlideShow, aaina_slide_show, CLUTTER_TYPE_GROUP);

#define AAINA_SLIDE_SHOW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
	AAINA_TYPE_SLIDE_SHOW, \
	AainaSlideShowPrivate))
	
struct _AainaSlideShowPrivate
{
  AainaLibrary      *library;
  ClutterTexture    *texture;
};

enum
{
  PROP_0,

  PROP_LIBRARY
};

static void
aaina_slide_show_remove_rows (AainaSlideShow *slide_show)
{
	clutter_group_remove_all (CLUTTER_GROUP(slide_show));
}

static gboolean
aaina_slide_show_row_foreach (AainaLibrary     *library,
		 	                        AainaPhoto	     *photo,
		 	                        gpointer          data)
{
  static GRand *rand = NULL;
  static gint count = 0;

  if (!rand)
    rand = g_rand_new ();
  gint x = g_rand_int_range (rand, 
                             0, 
                             CLUTTER_STAGE_WIDTH () *4);
  gint y;
  if (count == 1)
    y = 30;
  else if (count == 2)
    y = 200;
  else
    y  = 400;
  gdouble scale = g_random_double_range (0.1, 0.3);

  g_return_val_if_fail (AAINA_IS_SLIDE_SHOW (data), TRUE);
        
	clutter_group_add (CLUTTER_GROUP(data), CLUTTER_ACTOR(photo));
  aaina_photo_set_scale (AAINA_PHOTO (photo), scale);
	clutter_actor_set_position (CLUTTER_ACTOR (photo), x, y);
 
  clutter_actor_show_all (CLUTTER_ACTOR (photo));

  count++;
  if (count == 3)
    count = 0;
	return TRUE;
}

static void
on_photo_added (AainaLibrary    *library, 
                AainaPhoto      *photo, 
                AainaSlideShow  *slide_show)
{
  ;
}

void
aaina_slide_show_set_library (AainaSlideShow *slide_show, 
                              AainaLibrary *library)
{
  AainaSlideShowPrivate *priv;

  g_return_if_fail (AAINA_IS_SLIDE_SHOW (slide_show));
  if (!AAINA_IS_LIBRARY (library))
    return;
  priv = slide_show->priv;

  priv->library = library;
  
  if (priv->library)
  {
    aaina_slide_show_remove_rows (slide_show);
    g_object_unref (priv->library);
  
    g_signal_connect (G_OBJECT (priv->library), "photo-added",
                    G_CALLBACK (on_photo_added), slide_show);
    aaina_library_foreach (priv->library, 
                         aaina_slide_show_row_foreach,
                         (gpointer)slide_show);
  }
}


/* GObject stuff */

/*
static void
aaina_slide_show_paint (ClutterActor *actor)
{
  ;
}
*/

static void
aaina_slide_show_set_property (GObject      *object, 
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  AainaSlideShowPrivate *priv;
  
  g_return_if_fail (AAINA_IS_SLIDE_SHOW (object));
  priv = AAINA_SLIDE_SHOW (object)->priv;

  switch (prop_id)
  {
    case PROP_LIBRARY:
      aaina_slide_show_set_library (AAINA_SLIDE_SHOW (object), 
                                    AAINA_LIBRARY (g_value_get_object (value)));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
aaina_slide_show_get_property (GObject    *object, 
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  AainaSlideShowPrivate *priv;
  
  g_return_if_fail (AAINA_IS_SLIDE_SHOW (object));
  priv = AAINA_SLIDE_SHOW (object)->priv;

  switch (prop_id)
  {
    case PROP_LIBRARY:
      g_value_set_object (value, G_OBJECT (priv->library));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

  static void
aaina_slide_show_dispose (GObject *object)
{
 G_OBJECT_CLASS (aaina_slide_show_parent_class)->dispose (object);
}

static void
aaina_slide_show_finalize (GObject *object)
{
  G_OBJECT_CLASS (aaina_slide_show_parent_class)->finalize (object);
}

static void
aaina_slide_show_class_init (AainaSlideShowClass *klass)
{
  GObjectClass    *gobject_class = G_OBJECT_CLASS (klass);
  /*ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  actor_class->paint          = aaina_slide_show_paint;*/

  gobject_class->finalize     = aaina_slide_show_finalize;
  gobject_class->dispose      = aaina_slide_show_dispose;
  gobject_class->get_property = aaina_slide_show_get_property;
  gobject_class->set_property = aaina_slide_show_set_property;

  g_type_class_add_private (gobject_class, sizeof (AainaSlideShowPrivate));

  g_object_class_install_property (
    gobject_class,
    PROP_LIBRARY,
    g_param_spec_object ("library",
                         "The Library",
                         "The AainaLibrary",
                         AAINA_TYPE_LIBRARY,
                         G_PARAM_CONSTRUCT|G_PARAM_READWRITE));

}

static void
aaina_slide_show_init (AainaSlideShow *slide_show)
{
  AainaSlideShowPrivate *priv;

  g_return_if_fail (AAINA_IS_SLIDE_SHOW (slide_show));
  priv = AAINA_SLIDE_SHOW_GET_PRIVATE (slide_show);

  slide_show->priv = priv;
}

ClutterActor*
aaina_slide_show_new (void)
{
  ClutterGroup         *slide_show;

  slide_show = g_object_new (AAINA_TYPE_SLIDE_SHOW, NULL);
  
  return CLUTTER_ACTOR (slide_show);
}
