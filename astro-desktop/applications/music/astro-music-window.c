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


#include "astro-music-window.h"

#include <math.h>
#include <string.h>
#include <libastro-desktop/astro-defines.h>
#include <libastro-desktop/astro-application.h>
#include <libastro-desktop/astro-window.h>
#include <libastro-desktop/astro-behave.h>
#include <libastro-desktop/astro-utils.h>

#include "astro-reflection.h"

G_DEFINE_TYPE (AstroMusicWindow, astro_music_window, ASTRO_TYPE_WINDOW);

#define ASTRO_MUSIC_WINDOW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
        ASTRO_TYPE_MUSIC_WINDOW, AstroMusicWindowPrivate))

#define ALBUM_DIR PKGDATADIR"/albums"

struct _AstroMusicWindowPrivate
{
  GList *covers;

  ClutterActor *albums;
  ClutterActor *label;

  ClutterActor *player;

  gint active;
  gboolean activated;

  ClutterTimeline  *timeline;
  ClutterAlpha     *alpha;
  ClutterBehaviour *behave;
};

/* Public Functions */

/* Private functions */
typedef struct
{
  gint x;
  gfloat scale;

} CoverTrans;

static void
ensure_layout (AstroMusicWindow *window)
{
  AstroMusicWindowPrivate *priv;
  GList *c;
  gint i = 0;

  priv = window->priv;

  c = priv->covers;
  for (c=c; c; c = c->next)
    {
      ClutterActor *cover = c->data;
      CoverTrans *trans = g_object_get_data (G_OBJECT (cover), "trans");

      if (i == priv->active)
        {
          trans->x = CSW ()/2;
          trans->scale = 1.0;
        }
      else if (i > priv->active)
        {
          gint diff;

          diff = i - priv->active;
          trans->x = (CSW()/2) + ((CSW()/4)*diff);
          if (diff > 3)
            trans->scale = 0.4;
          else
            trans->scale = 0.4 + (0.4 * (3-diff)/3);
        }
      else
        {
          gint diff;

          diff = priv->active - i;
          trans->x = (CSW()/2) - ((CSW()/4)*diff);
          if (diff > 3)
            trans->scale = 0.4;
          else
            trans->scale = 0.4 + (0.4 * (3-diff)/3);        
        }

      i++;
    }
}

static void
astro_music_window_advance (AstroMusicWindow *window, gint n)
{
  AstroMusicWindowPrivate *priv;
  gint new_active;

  g_return_if_fail (ASTRO_IS_MUSIC_WINDOW (window));
  priv = window->priv;
  
  new_active = priv->active + n;
  if (new_active < 0 || 
   new_active > (clutter_group_get_n_children (CLUTTER_GROUP (priv->albums))-1))
    return;

  priv->active += n;
  ensure_layout (window);

  if (clutter_timeline_is_playing (priv->timeline))
    clutter_timeline_rewind (priv->timeline);
  else
    clutter_timeline_start (priv->timeline);

}

static void
on_cover_active_completed (ClutterTimeline *timeline,
                           AstroReflection *reflection)
{
  astro_reflection_set_active (reflection, TRUE);

  g_signal_handlers_disconnect_by_func (timeline, 
                                        on_cover_active_completed,
                                        reflection);
}

static void
on_cover_activated (AstroMusicWindow *window)
{
  AstroMusicWindowPrivate *priv;
  ClutterActor *cover;
  GList *children;
  CoverTrans *trans;

  g_return_if_fail (ASTRO_IS_MUSIC_WINDOW (window));
  priv = window->priv;

  children = priv->covers;
  cover = g_list_nth_data (children, priv->active);

  if (!CLUTTER_IS_ACTOR (cover))
    return;

  trans = g_object_get_data (G_OBJECT (cover), "trans");
  if (!trans)
    return;

  priv->activated = TRUE;

  trans->scale = ALBUM_SCALE;
  trans->x = (CSW()/2) - ((ALBUM_SIZE * ALBUM_SCALE) * 0.5);

  clutter_actor_raise_top (cover);

  if (clutter_timeline_is_playing (priv->timeline))
    clutter_timeline_rewind (priv->timeline);
  else
    clutter_timeline_start (priv->timeline);

  g_signal_connect (priv->timeline, "completed",
                    G_CALLBACK (on_cover_active_completed), cover);
}

static gboolean
on_cover_clicked (ClutterActor      *cover, 
                  ClutterEvent      *event,
                  AstroMusicWindow  *window)
{
  AstroMusicWindowPrivate *priv;
  GList *children;
  gint n;

  g_return_val_if_fail (ASTRO_IS_MUSIC_WINDOW (window), FALSE);
  priv = window->priv;

  children = priv->covers;
  n = g_list_index (children, cover);

  if (priv->activated)
    {
      if (event->button.x > CSW()/2)
        return FALSE;
      astro_reflection_set_active (g_list_nth_data (priv->covers,
                                   priv->active), FALSE);
      priv->activated = FALSE;
      
      astro_music_window_advance (window, 0);
      return FALSE;
    }

  if (n == priv->active)
    on_cover_activated (window);
  else
    {
      gint diff;
      if (n > priv->active)
        diff = (n-priv->active);
      else
        diff = (priv->active - n) * -1;
      astro_music_window_advance (window, diff);
    }

  return FALSE;
}

static ClutterActor *
make_cover (const gchar *filename)
{
  GdkPixbuf *pixbuf;
  ClutterActor *texture;

  pixbuf = gdk_pixbuf_new_from_file_at_size (filename,
                                             ALBUM_SIZE, ALBUM_SIZE,
                                             NULL);
  if (!pixbuf)
    return NULL;

  texture = astro_reflection_new (pixbuf);

  g_object_set_data (G_OBJECT (texture), "trans", g_new0 (CoverTrans, 1));
  return texture;
}

static void
load_details (ClutterActor *cover, const gchar *leaf)
{
  gchar *details;
  gint i;

  details = g_strndup (leaf, strlen (leaf)-4);

  for (i = 0; i < strlen (details); i++)
    if (details[i] == '_') details[i] = ' ';

  clutter_actor_set_name (cover, details);
  g_free (details);
}

static void
load_albums (AstroMusicWindow *window)
{
  AstroMusicWindowPrivate *priv;
  GDir *dir;
  const gchar *leaf;
  GError *error = NULL;
  gint offset = CSW()*2;

  priv = window->priv;

  dir = g_dir_open (ALBUM_DIR, 0, &error);
  if (error)
    {
      g_warning ("Cannot load albums: %s", error->message);
      g_error_free (error);
      return;
    }
  
  while ((leaf = g_dir_read_name (dir)))
    {
      ClutterActor *cover;
      gchar *filename;

      if (!g_str_has_suffix (leaf, ".jpg"))
        continue;

      filename = g_build_filename (ALBUM_DIR, leaf, NULL);
      cover = make_cover (filename);

      if (!CLUTTER_IS_ACTOR (cover))
        {
          g_free (filename);
          continue;
        }
      load_details (cover, leaf);
      clutter_container_add_actor (CLUTTER_CONTAINER (priv->albums), cover);
      clutter_actor_set_position (cover, offset, 0);
      clutter_actor_show_all (cover);
      clutter_actor_set_reactive (cover, TRUE);
      g_signal_connect (cover, "button-release-event",
                        G_CALLBACK (on_cover_clicked), window);

      priv->covers = g_list_append (priv->covers, cover);

      g_free (filename);

      offset += ALBUM_SIZE * 0.9;
    }
}

static void
astro_music_alpha (ClutterBehaviour *behave,
                   guint32           alpha_value,
                   AstroMusicWindow *window)
{
  AstroMusicWindowPrivate *priv;
  GList *c;

  g_return_if_fail (ASTRO_IS_MUSIC_WINDOW (window));
  priv = window->priv;

  c = priv->covers;
  for (c=c; c; c = c->next)
    {
      ClutterActor *cover = c->data;
      CoverTrans *trans = g_object_get_data (G_OBJECT (cover), "trans");
      gdouble cscale, dscale;
      gint currentx, diffx;
      
      currentx = clutter_actor_get_x (cover);
      if (currentx > trans->x)
        diffx = (currentx - trans->x) * -1;
      else
        diffx = trans->x - currentx;

      clutter_actor_set_x (cover, currentx 
        + (gint)((diffx*alpha_value)/CLUTTER_ALPHA_MAX_ALPHA));

      clutter_actor_get_scale (cover, &cscale, &cscale);
      if (cscale > trans->scale)
        dscale = (cscale - trans->scale) * -1;
      else
        dscale = trans->scale - cscale;

      clutter_actor_set_scale (cover, 
        cscale + ((dscale*alpha_value)/CLUTTER_ALPHA_MAX_ALPHA),
        cscale + ((dscale*alpha_value)/CLUTTER_ALPHA_MAX_ALPHA));
    }
}

static void
on_main_timeline_completed (ClutterTimeline  *timeline,
                            AstroMusicWindow *window)
{
  AstroMusicWindowPrivate *priv;
  const gchar *details;
  GList *children;

  g_return_if_fail (ASTRO_MUSIC_WINDOW (window));
  priv = window->priv;

  children = priv->covers;
  details = clutter_actor_get_name (g_list_nth_data (children, priv->active));

  clutter_label_set_text (CLUTTER_LABEL (priv->label), details);
}

static gboolean
on_key_release_event (ClutterActor     *actor, 
                      ClutterEvent     *event,
                      AstroMusicWindow *window)
{
  AstroMusicWindowPrivate *priv;
  
  g_return_val_if_fail (ASTRO_IS_WINDOW (window), FALSE);
  priv = window->priv;

  switch (event->key.keyval)
    {
      case CLUTTER_Return:
      case CLUTTER_KP_Enter:
      case CLUTTER_ISO_Enter:
        on_cover_activated (window);
        break;
      case CLUTTER_Left:
      case CLUTTER_KP_Left:
        if (priv->activated)
          {
            astro_reflection_set_active (g_list_nth_data (priv->covers,
                                                          priv->active), FALSE);  
            priv->activated = FALSE;
          }
        astro_music_window_advance (window, -1);
        break;
      case CLUTTER_Right:
      case CLUTTER_KP_Right:
        if (priv->activated)
          {
            astro_reflection_set_active (g_list_nth_data (priv->covers,
                                                        priv->active), FALSE);
            priv->activated = FALSE;
          }
          astro_music_window_advance (window, 1);
        break;
      default:
        ;
    }

  return FALSE;
}

/* GObject stuff */
static void
astro_music_window_class_init (AstroMusicWindowClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (gobject_class, sizeof (AstroMusicWindowPrivate));
}

static void
astro_music_window_init (AstroMusicWindow *window)
{
  AstroMusicWindowPrivate *priv;
  ClutterColor white = { 0xff, 0xff, 0xff, 0xff };

  priv = window->priv = ASTRO_MUSIC_WINDOW_GET_PRIVATE (window);

  priv->covers = NULL;
  priv->active = 0;
  priv->activated = FALSE;

  priv->albums = clutter_group_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (window), priv->albums);
  clutter_actor_set_anchor_point_from_gravity (priv->albums, 
                                               CLUTTER_GRAVITY_WEST);
  clutter_actor_set_position (priv->albums, 0, CSH() * 0.5);

  load_albums (window);
  
  priv->label = clutter_label_new_full ("Sans 18", 
                                        "Jay Z - American Gangster",
                                        &white);
  clutter_label_set_line_wrap (CLUTTER_LABEL (priv->label), FALSE);
  clutter_label_set_alignment (CLUTTER_LABEL (priv->label),
                               PANGO_ALIGN_CENTER);
  clutter_container_add_actor (CLUTTER_CONTAINER (window), priv->label);
  clutter_actor_set_size (priv->label, CSW(), CSH()/10);
  clutter_actor_set_anchor_point_from_gravity (priv->label, 
                                               CLUTTER_GRAVITY_CENTER);
  clutter_actor_set_position (priv->label, CSW()/2, CSH()*0.95);

  ensure_layout (window);

  priv->timeline = clutter_timeline_new_for_duration (1200);
  priv->alpha = clutter_alpha_new_full (priv->timeline,
                                        clutter_sine_inc_func,
                                        NULL, NULL);
  priv->behave = astro_behave_new (priv->alpha,
                                   (AstroBehaveAlphaFunc)astro_music_alpha,
                                   window);

  g_signal_connect (priv->timeline, "completed",
                    G_CALLBACK (on_main_timeline_completed), window);

  clutter_timeline_start (priv->timeline);

  g_signal_connect (window, "key-release-event",
                    G_CALLBACK (on_key_release_event), window);
  clutter_grab_keyboard (CLUTTER_ACTOR (window));


  clutter_actor_set_position (CLUTTER_ACTOR (window), 0, 0);
  clutter_actor_show_all (CLUTTER_ACTOR (window));
}

AstroWindow * 
astro_music_window_new (void)
{
  AstroWindow *music_window =  g_object_new (ASTRO_TYPE_MUSIC_WINDOW,
									                       NULL);

  return music_window;
}

