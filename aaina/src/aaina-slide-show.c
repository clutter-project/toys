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
#define N_LANES 7

static gint lane_frames[N_LANES] = {60, 60, 60, 60, 60, 60, 60};
static gint lane_speed[N_LANES]  = {180, 150, 120, 90, 60, 30, 15};

struct _AainaSlideShowPrivate
{
  AainaLibrary      *library;
  gint               count;
  
  GList             *lanes[N_LANES];
  ClutterTimeline   *timelines[N_LANES];
  gint               lanesx[N_LANES];

  /* Viewing single photo */
  AainaPhoto        *zoomed;

};

enum
{
  PROP_0,

  PROP_LIBRARY
};

static void     on_photo_added (AainaLibrary    *library, 
                                AainaPhoto      *photo, 
                                AainaSlideShow  *data);
static gboolean zoom_photo (AainaSlideShow *slide_show);

static void
on_photo_zoomed (AainaPhoto *photo, AainaSlideShow *slide_show)
{
  AainaSlideShowPrivate *priv;

  g_return_if_fail (AAINA_IS_SLIDE_SHOW (slide_show));
  priv = slide_show->priv;

  priv->zoomed = NULL;
}

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

  if (!AAINA_IS_PHOTO (priv->zoomed))
    return FALSE;

  aaina_photo_set_viewed (priv->zoomed, TRUE);
  aaina_photo_restore (priv->zoomed);

  for (i = 0; i < N_LANES; i++)
    clutter_timeline_start (priv->timelines[i]);

  g_signal_connect (G_OBJECT (priv->zoomed), "photo_restored",
                    G_CALLBACK (on_photo_zoomed), (gpointer)slide_show);

  g_timeout_add (g_rand_int_range (rand, 4000, 10000), 
                 (GSourceFunc)zoom_photo, 
                 (gpointer)slide_show);
  return FALSE;
}
/*
static void
on_photo_zoomed (AainaPhoto *photo, AainaSlideShow *slide_show)
{
  AainaSlideShowPrivate *priv;
  gint i;

  g_return_if_fail (AAINA_IS_SLIDE_SHOW (slide_show));
  priv = slide_show->priv;

   Pause all the timelines
  for (i = 0; i < N_LANES; i++)
    clutter_timeline_pause (priv->timelines[i]);
}
*/
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
    guint w, h;
    
    if (!AAINA_IS_PHOTO (actor))
      continue;

    x = clutter_actor_get_x (actor);
    clutter_actor_get_abs_size (actor, &w, &h);
    x += w;

    if (x > 0 && x < stage_width 
            && !aaina_photo_get_viewed (AAINA_PHOTO (actor)))
      photos = g_list_append (photos, actor);
  }
  
  /* This should work, right? */
  if (photos == NULL)
  {
    //zoom_photo (slide_show);
    return TRUE;
  }

  /* Choose a random photo in the list */
  i = g_rand_int_range (rand, 0, g_list_length (photos));
  photo = AAINA_PHOTO (g_list_nth_data (photos, i));

  /* Connect to 'zoomed' signal, swhen the photo has finished, we stop the
   * timelines
   
  g_signal_connect (G_OBJECT (photo), "photo_zoomed", 
                     G_CALLBACK (on_photo_zoomed), (gpointer)slide_show);
  */
  /* Save the photos current 'state' (x, y, and scale) */
  aaina_photo_save (photo);
  
  clutter_actor_raise_top (CLUTTER_ACTOR (photo));                              
  aaina_photo_zoom (photo);

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
  AainaSlideShow *slide_show = aaina_slide_show_get_default ();
  AainaSlideShowPrivate *priv = slide_show->priv;
  GList *l;
  gint leftmost = 0 - (CLUTTER_STAGE_WIDTH () /4);

  for (l = *lane; l != NULL; l = l->next)
  {
    gint x;
    guint width;
    gboolean viewed;

    if (!AAINA_IS_PHOTO (l->data))
      continue;

    g_object_get (G_OBJECT (l->data), 
                  "x", &x,
                  "width", &width,
                  "viewed", &viewed,
                  NULL);
    if (l->data == priv->zoomed)
      continue;
    else if (viewed)
    {
      if (x < leftmost)
      {
        if (aaina_library_get_pending (priv->library))
        {
          aaina_library_remove_photo (priv->library, AAINA_PHOTO (l->data));
          clutter_actor_destroy (CLUTTER_ACTOR (l->data));
          l->data = NULL;
          g_print ("Deleteing\n");
        }
        else
        {
          aaina_photo_set_viewed (AAINA_PHOTO (l->data), FALSE);
          on_photo_added (NULL, l->data, slide_show);
          l->data = NULL;
          g_print ("Re-adding\n");
        }
      }
      else
        g_object_set (G_OBJECT (l->data), "x", x - 1, NULL); 
    }
    else
    {
      if (x > leftmost)
        g_object_set (G_OBJECT (l->data), "x", x - 1, NULL);
      else
      {
        on_photo_added (NULL, l->data, slide_show);
        l->data = NULL;
      }
    }
  }
}

static void
aaina_slide_show_remove_rows (AainaSlideShow *slide_show)
{
	clutter_group_remove_all (CLUTTER_GROUP(slide_show));
}

static void
on_photo_added (AainaLibrary    *library, 
                AainaPhoto      *photo, 
                AainaSlideShow  *data)
{
  AainaSlideShowPrivate *priv;
  static GRand *rand = NULL;
  gint count;
  gint x, y, dim;
  gdouble scale;
 
  g_return_if_fail (AAINA_IS_SLIDE_SHOW (data));
  priv = AAINA_SLIDE_SHOW (data)->priv;

  if (!rand)
    rand = g_rand_new ();

  count = priv->count;

  /* We want the scale of the photos to be random 0.15, 0.35*/
  scale = (0.35 - 0.15)/N_LANES;
  scale = g_rand_double_range (rand,
                               0.15 + (scale * count), 
                               0.15 + (scale * (count+1))); 

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
  y = ((CLUTTER_STAGE_HEIGHT () / (N_LANES +2)) * count) + 30;
  y = g_rand_int_range (rand, 
                        0, 
                        CLUTTER_STAGE_HEIGHT ()-(CLUTTER_STAGE_HEIGHT()/5));
         
	/* Use AainaPhoto's scale feature as it makes sure gravity is center */
  clutter_actor_set_scale (CLUTTER_ACTOR (photo), scale, scale);
	clutter_actor_set_position (CLUTTER_ACTOR (photo), x, y);
  clutter_actor_set_depth (CLUTTER_ACTOR (photo), count);

  dim = 255/N_LANES;
  dim = dim * (N_LANES - (count+1));
  aaina_photo_set_dim (photo, dim);

  if (!clutter_actor_get_parent (CLUTTER_ACTOR (photo)))
    clutter_group_add (CLUTTER_GROUP (clutter_stage_get_default ()), 
                       CLUTTER_ACTOR (photo));
  clutter_actor_show_all (CLUTTER_ACTOR (photo));

  priv->lanes[count] = g_list_append (priv->lanes[count], (gpointer)photo);

  priv->count++;
  if (priv->count == N_LANES)
    priv->count = 0;
}
  
static gboolean
aaina_slide_show_row_foreach (AainaLibrary     *library,
		 	                        AainaPhoto	     *photo,
		 	                        gpointer          data)
{
  on_photo_added (library, photo, AAINA_SLIDE_SHOW (data));
  return TRUE;
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
  priv->count = 0;
  
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
aaina_slide_show_get_default (void)
{
  static AainaSlideShow *slide_show = NULL;

  if (!slide_show)
    slide_show = g_object_new (AAINA_TYPE_SLIDE_SHOW, NULL);
  
  return slide_show;
}
