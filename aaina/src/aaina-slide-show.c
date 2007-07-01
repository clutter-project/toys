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

#include <libaaina/aaina-behave.h>

#include "aaina-slide-show.h"

G_DEFINE_TYPE (AainaSlideShow, aaina_slide_show, G_TYPE_OBJECT);

#define AAINA_SLIDE_SHOW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
	AAINA_TYPE_SLIDE_SHOW, \
	AainaSlideShowPrivate))

#define VIEW_PHOTO_TIMEOUT 4000
#define N_LANES 4
static gint lane_frames[N_LANES] = {60, 40, 60, 40};
static gint lane_speed[N_LANES]  = {120, 40, 320, 60};

struct _AainaSlideShowPrivate
{
  AainaLibrary      *library;
  ClutterTexture    *texture;

  GList             *lanes[N_LANES];
  ClutterTimeline   *timelines[N_LANES];
  gint               lanesx[N_LANES];

  AainaPhoto        *zoomed;
};

enum
{
  PROP_0,

  PROP_LIBRARY
};

static gboolean zoom_photo (AainaSlideShow *slide_show);

static gboolean
restore_photo (AainaSlideShow *slide_show)
{
  AainaSlideShowPrivate *priv;
  static GRand *rand = NULL;
  gint i;

  g_return_val_if_fail (AAINA_IS_SLIDE_SHOW (slide_show), FALSE);
  priv = slide_show->priv;
  
  if (rand == NULL)
    rand = g_rand_new ();

  aaina_photo_set_viewed (priv->zoomed, TRUE);
  aaina_photo_restore (priv->zoomed);

  for (i = 0; i < N_LANES; i++)
    clutter_timeline_start (priv->timelines[i]);

  g_timeout_add (g_rand_int_range (rand, 4000, 10000), 
                 (GSourceFunc)zoom_photo, 
                 (gpointer)slide_show);
  return FALSE;
}

static gboolean
zoom_photo (AainaSlideShow *slide_show)
{
  AainaSlideShowPrivate *priv;
  static GRand *rand = NULL;
  GList *l, *photos = NULL;
  gint lane, i;
  gint stage_width = CLUTTER_STAGE_WIDTH ();
  AainaPhoto *photo;
  
  g_return_val_if_fail (AAINA_IS_SLIDE_SHOW (slide_show), FALSE);
  priv = slide_show->priv;

  if (rand == NULL)
    rand = g_rand_new ();
  
  /* Get a random lane to choose the picture from */
  lane = g_rand_int_range (rand, 0, N_LANES);
  
  /* Create a list of possible photos to zoom (those which are 'visible' to the
   * user)
   */
  for (l = priv->lanes[lane]; l != NULL; l = l->next)
  {
    ClutterActor *actor = CLUTTER_ACTOR (l->data);
    gint x;

    if (!AAINA_IS_PHOTO (actor))
      continue;

    x = clutter_actor_get_x (actor);

    if (x > 0 && x < stage_width)
      photos = g_list_append (photos, actor);
  }
  
  /* Choose a random photo in the list */
  i = g_rand_int_range (rand, 0, g_list_length (photos));
  photo = AAINA_PHOTO (g_list_nth_data (photos, i));

  /* Pause all the timelines*/
  for (i = 0; i < N_LANES; i++)
    clutter_timeline_pause (priv->timelines[i]);

  /* Save the photos current 'state' (x, y, and scale) */
  aaina_photo_save (photo);
  
  /* FIXME: This needs to be a behaviour */
  clutter_actor_raise_top (CLUTTER_ACTOR (photo));
  clutter_actor_set_scale (CLUTTER_ACTOR (photo), 1, 1);
  clutter_actor_set_position (CLUTTER_ACTOR (photo),
                              CLUTTER_STAGE_WIDTH () /4,
                              CLUTTER_STAGE_HEIGHT ()/4);
  
  /* Keep a pointer to the photo, and finally add a timeout for when the 
   * slideshow should be restored.
   */
  priv->zoomed = photo;
  g_timeout_add (VIEW_PHOTO_TIMEOUT, 
                 (GSourceFunc)restore_photo, 
                 (gpointer)slide_show);

  return FALSE;
}

static void
aaina_slide_show_move (ClutterBehaviour *behave, 
                       guint32 alpha_value, 
                       GList **lane)
{
  GList *l;

  for (l = *lane; l != NULL; l = l->next)
  {
    g_object_set (G_OBJECT (l->data), "x", 
                  clutter_actor_get_x (l->data) - 1, NULL); 
  }
}

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
  AainaSlideShowPrivate *priv;
  static GRand *rand = NULL;
  static gint count = 0;
  gint x, y;
  gdouble scale;
 
  g_return_val_if_fail (AAINA_IS_SLIDE_SHOW (data), TRUE);
  priv = AAINA_SLIDE_SHOW (data)->priv;

  if (!rand)
    rand = g_rand_new ();

  /* We want the scale of the photos to be random */
  scale = g_rand_double_range (rand, 0.15, 0.35); 

  /* We want 'random' spacing of the photos, but we don't want to overlap two
   * photos from the same lane, hence we only randomise the gap between two
   * photos. That also prevents photos from being too far apart
   */
  x = g_rand_int_range (rand, 0, CLUTTER_STAGE_WIDTH ()/4);
  x += priv->lanesx[count];

  /* Set the new x value for the lane */
  priv->lanesx[count] = x + (CLUTTER_STAGE_WIDTH() * scale);

  /* Each lane has a set 'base y value, as calculated below, in addition to 
   * this, we add a random value between -30 and 30, which makes sure the photos
   * look randomised.
   */
  y = ((CLUTTER_STAGE_HEIGHT () / N_LANES +1) * count) + 30;
  y += g_rand_int_range (rand, -30, 30);
         
	/* Use AainaPhoto's scale feature as it makes sure gravity is center */
  //aaina_photo_set_scale (AAINA_PHOTO (photo), scale);
  clutter_actor_set_scale (CLUTTER_ACTOR (photo), scale, scale);
	clutter_actor_set_position (CLUTTER_ACTOR (photo), x, y);
 
  clutter_group_add (CLUTTER_GROUP (clutter_stage_get_default ()), 
                                    CLUTTER_ACTOR (photo));
  clutter_actor_show_all (CLUTTER_ACTOR (photo));

  priv->lanes[count] = g_list_append (priv->lanes[count], (gpointer)photo);

  count++;
  if (count == N_LANES)
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
  gint i;

  g_return_if_fail (AAINA_IS_SLIDE_SHOW (slide_show));
  if (!AAINA_IS_LIBRARY (library))
    return;
  priv = slide_show->priv;

  if (priv->library)
  {
    aaina_slide_show_remove_rows (slide_show);
    g_object_unref (priv->library);
  }
  priv->library = library;
  if (!library)
    return;
  g_signal_connect (G_OBJECT (priv->library), "photo-added",
                    G_CALLBACK (on_photo_added), slide_show);
  
  /* Sort each photo into a 'lane', and give it a 'randomised' x and y value */
  aaina_library_foreach (priv->library, 
                         aaina_slide_show_row_foreach,
                         (gpointer)slide_show);

  /* Now all the photos have a lane and position, we start each lanes timeline,
   * with randomised speed
   */
  for (i = 0; i < N_LANES; i++)
  {
    
    clutter_timeline_set_speed (priv->timelines[i], lane_speed[i]);
    clutter_timeline_set_n_frames (priv->timelines[i], lane_frames[i]);
    clutter_timeline_set_loop (priv->timelines[i], TRUE);
    
    if (!clutter_timeline_is_playing (priv->timelines[i]))
      clutter_timeline_start (priv->timelines[i]);
  }

  /* Finally, set the timeout for zooming the pictures */
  g_timeout_add (3000, (GSourceFunc)zoom_photo, (gpointer)slide_show);
}


/* GObject stuff */

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
  
  gint i;
  for (i=0; i < N_LANES;  i++)
  {
    ClutterAlpha *alpha;
    ClutterBehaviour *behave;

    priv->lanes[i] = NULL;

    priv->timelines[i] = clutter_timeline_new (40, 120);
    alpha = clutter_alpha_new_full (priv->timelines[i], 
                                    alpha_sine_inc_func,
                                    NULL, NULL);
    behave = aaina_behave_new (alpha, 
                               (AainaBehaveAlphaFunc)aaina_slide_show_move, 
                               (gpointer)&priv->lanes[i]);
    priv->lanesx[i] = g_random_int_range (0, CLUTTER_STAGE_WIDTH () /2);
  }
}

AainaSlideShow*
aaina_slide_show_new (void)
{
  AainaSlideShow         *slide_show;

  slide_show = g_object_new (AAINA_TYPE_SLIDE_SHOW, NULL);
  
  return slide_show;
}
