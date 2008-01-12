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


#include "astro-contacts-window.h"

#include <math.h>
#include <string.h>
#include <libastro-desktop/astro-defines.h>
#include <libastro-desktop/astro-application.h>
#include <libastro-desktop/astro-window.h>
#include <libastro-desktop/astro-behave.h>
#include <tidy/tidy-texture-frame.h>

#include "astro-reflection.h"

G_DEFINE_TYPE (AstroContactsWindow, astro_contacts_window, ASTRO_TYPE_WINDOW);

#define ASTRO_CONTACTS_WINDOW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
        ASTRO_TYPE_CONTACTS_WINDOW, AstroContactsWindowPrivate))

#define ALBUM_SIZE (CSW()/4)

static GdkPixbuf    *row_pixbuf = NULL;
static ClutterActor *row_texture = NULL;

struct _AstroContactsWindowPrivate
{
  GList *contacts_list;

  ClutterActor *row;

  ClutterActor *contacts;
  ClutterActor *label;

  ClutterActor *player;

  gint active;
  gboolean activated;

  ClutterTimeline  *timeline;
  ClutterAlpha     *alpha;
  ClutterBehaviour *behave;
};

static gchar *names[] = {
  "Andrew Zaborowski",
  "Chris Lord",
  "Dodji Seketeli",
  "Emmanuele Bassi",
  "Iain Holmes",
  "Jorn Baayen",
  "Jussi Kukkonen",
  "Marcin Juszkiewicz",
  "Matthew Allum",
  "Neil J. Patel",
  "Øyvind Kolås",
  "Paul Cooper",
  "Richard Purdie",
  "Robert Bradford",
  "Ross Burton",
  "Samuel Ortiz",
  "Sidske Allum",
  "Thomas Wood",
  "Tomas Frydrych"
};

/* Public Functions */

/* Private functions */
typedef struct
{
  gint y;
  gfloat scale;

} ContactTrans;

static void
ensure_layout (AstroContactsWindow *window)
{
#define MAX_DIST 4
  AstroContactsWindowPrivate *priv;
  GList *c;
  gint i = 0;

  priv = window->priv;

  c = priv->contacts_list;
  for (c=c; c; c = c->next)
    {
      ClutterActor *contact = c->data;
      ContactTrans *trans = g_object_get_data (G_OBJECT (contact), "trans");

      if (i == priv->active)
        {
          trans->y = CSH ()/2;
          trans->scale = 1.0;
        }
      else if (i > priv->active)
        {
          gint diff;

          diff = i - priv->active;
          trans->y = (CSH()/2) + ((CSH()/5)*diff);
          if (diff > MAX_DIST)
            trans->scale = 0.4;
          else
            trans->scale = 0.4 + (0.4 * (MAX_DIST-diff)/MAX_DIST);
        }
      else
        {
          gint diff;

          diff = priv->active - i;
          trans->y = (CSH()/2) - ((CSH()/5)*diff);
          if (diff > MAX_DIST)
            trans->scale = 0.4;
          else
            trans->scale = 0.4 + (0.4 * (MAX_DIST-diff)/MAX_DIST);        
        }

      i++;
    }
}

static void
astro_contacts_list_window_advance (AstroContactsWindow *window, gint n)
{
  AstroContactsWindowPrivate *priv;
  gint new_active;

  g_return_if_fail (ASTRO_IS_CONTACTS_WINDOW (window));
  priv = window->priv;
  
  new_active = priv->active + n;
  if (new_active < 0 || 
   new_active > (clutter_group_get_n_children (CLUTTER_GROUP (priv->contacts))-1))
    return;

  priv->active += n;
  ensure_layout (window);

  if (clutter_timeline_is_playing (priv->timeline))
    clutter_timeline_rewind (priv->timeline);
  else
    clutter_timeline_start (priv->timeline);

}

static void
on_contact_active_completed (ClutterTimeline *timeline,
                           AstroReflection *reflection)
{
  astro_reflection_set_active (reflection, TRUE);

  g_signal_handlers_disconnect_by_func (timeline, 
                                        on_contact_active_completed,
                                        reflection);
}

static void
on_contact_activated (AstroContactsWindow *window)
{
#define ACTIVE_SCALE 1.5
  AstroContactsWindowPrivate *priv;
  ClutterActor *contact;
  GList *children;
  ContactTrans *trans;

  g_return_if_fail (ASTRO_IS_CONTACTS_WINDOW (window));
  priv = window->priv;

  children = priv->contacts_list;
  contact = g_list_nth_data (children, priv->active);

  if (!CLUTTER_IS_ACTOR (contact))
    return;

  trans = g_object_get_data (G_OBJECT (contact), "trans");
  if (!trans)
    return;

  priv->activated = TRUE;

  trans->scale = ACTIVE_SCALE;
  trans->y = (CSW()/2) - ((ALBUM_SIZE * ACTIVE_SCALE) * 0.5);

  clutter_actor_raise_top (contact);

  if (clutter_timeline_is_playing (priv->timeline))
    clutter_timeline_rewind (priv->timeline);
  else
    clutter_timeline_start (priv->timeline);

  g_signal_connect (priv->timeline, "completed",
                    G_CALLBACK (on_contact_active_completed), contact);
}

static gboolean
on_contact_clicked (ClutterActor      *contact, 
                  ClutterEvent      *event,
                  AstroContactsWindow  *window)
{
  AstroContactsWindowPrivate *priv;
  GList *children;
  gint n;

  g_return_val_if_fail (ASTRO_IS_CONTACTS_WINDOW (window), FALSE);
  priv = window->priv;

  children = priv->contacts_list;
  n = g_list_index (children, contact);

  if (priv->activated)
    {
      astro_reflection_set_active (g_list_nth_data (priv->contacts_list,
                                   priv->active), FALSE);
      priv->activated = FALSE;
      
      astro_contacts_list_window_advance (window, 0);
      return FALSE;
    }
  if (n == priv->active)
    on_contact_activated (window);
  else
    {
      gint diff;
      if (n > priv->active)
        diff = (n-priv->active);
      else
        diff = (priv->active - n) * -1;
      astro_contacts_list_window_advance (window, diff);
    }

  return FALSE;
}

static ClutterActor *
make_contact (const gchar *name)
{
  ClutterActor *label;
  ClutterColor white = { 0xff, 0xff, 0xff, 0xff };

  label = clutter_label_new_full ("Sans 22", name, &white);
  clutter_label_set_line_wrap (CLUTTER_LABEL (label), FALSE);
  clutter_actor_set_width (label, CSW()/2);
  clutter_actor_set_anchor_point_from_gravity (label, CLUTTER_GRAVITY_WEST);

  g_object_set_data (G_OBJECT (label), "trans", g_new0 (ContactTrans, 1));

  return label;
}

#if 0
static void
load_details (ClutterActor *contact, const gchar *leaf)
{
  gchar *details;
  gint i;

  details = g_strndup (leaf, strlen (leaf)-4);

  for (i = 0; i < strlen (details); i++)
    if (details[i] == '_') details[i] = ' ';

  clutter_actor_set_name (contact, details);
  g_free (details);
}
#endif

static void
load_contacts (AstroContactsWindow *window)
{
#define PADDING 10
  AstroContactsWindowPrivate *priv;
  gint i = 0;
  
  priv = window->priv;

  for (i = 0; i < G_N_ELEMENTS (names); i++)
     {
      ClutterActor *contact;

      contact = make_contact (names[i]);
      clutter_container_add_actor (CLUTTER_CONTAINER (priv->contacts), contact);
      clutter_actor_set_x (contact, PADDING);
      clutter_actor_show_all (contact);
      clutter_actor_set_reactive (contact, TRUE);
      g_signal_connect (contact, "button-release-event",
                        G_CALLBACK (on_contact_clicked), window);

      priv->contacts_list = g_list_append (priv->contacts_list, contact);
    }
}

static void
astro_contacts_list_alpha (ClutterBehaviour *behave,
                   guint32           alpha_value,
                   AstroContactsWindow *window)
{
  AstroContactsWindowPrivate *priv;
  gfloat factor;
  GList *c;

  g_return_if_fail (ASTRO_IS_CONTACTS_WINDOW (window));
  priv = window->priv;

  factor = (gfloat)alpha_value / CLUTTER_ALPHA_MAX_ALPHA;
  
  c = priv->contacts_list;
  for (c=c; c; c = c->next)
    {
      ClutterActor *contact = c->data;
      ContactTrans *trans = g_object_get_data (G_OBJECT (contact), "trans");
      gdouble cscale, dscale;
      gint currenty, diffy;
      
      currenty = clutter_actor_get_y (contact);
      if (currenty > trans->y)
        diffy = (currenty - trans->y) * -1;
      else
        diffy = trans->y - currenty;

      clutter_actor_set_y (contact, currenty + (gint)(diffy*factor));

      clutter_actor_get_scale (contact, &cscale, &cscale);
      if (cscale > trans->scale)
        dscale = (cscale - trans->scale) * -1;
      else
        dscale = trans->scale - cscale;

      clutter_actor_set_scale (contact, 
                              cscale + (dscale*factor),
                              cscale + (dscale*factor));
    }
}

static void
on_main_timeline_completed (ClutterTimeline  *timeline,
                            AstroContactsWindow *window)
{
  AstroContactsWindowPrivate *priv;
  
  g_return_if_fail (ASTRO_CONTACTS_WINDOW (window));
  priv = window->priv;

}

static gboolean
on_key_release_event (ClutterActor     *actor, 
                      ClutterEvent     *event,
                      AstroContactsWindow *window)
{
  AstroContactsWindowPrivate *priv;
  
  g_return_val_if_fail (ASTRO_IS_WINDOW (window), FALSE);
  priv = window->priv;

  switch (event->key.keyval)
    {
      case CLUTTER_Return:
      case CLUTTER_KP_Enter:
      case CLUTTER_ISO_Enter:
        on_contact_activated (window);
        break;
      case CLUTTER_Up:
      case CLUTTER_KP_Up:
        if (priv->activated)
          {
            astro_reflection_set_active (g_list_nth_data (priv->contacts_list,
                                                          priv->active), FALSE);  
            priv->activated = FALSE;
          }
        astro_contacts_list_window_advance (window, -1);
        break;
      case CLUTTER_Down:
      case CLUTTER_KP_Down:
        if (priv->activated)
          {
            astro_reflection_set_active (g_list_nth_data (priv->contacts_list,
                                                        priv->active), FALSE);
            priv->activated = FALSE;
          }
          astro_contacts_list_window_advance (window, 1);
        break;
      default:
        ;
    }

  return FALSE;
}

/* GObject stuff */
static void
astro_contacts_window_class_init (AstroContactsWindowClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (gobject_class, sizeof (AstroContactsWindowPrivate));
}

static void
astro_contacts_window_init (AstroContactsWindow *window)
{
  AstroContactsWindowPrivate *priv;
  
  priv = window->priv = ASTRO_CONTACTS_WINDOW_GET_PRIVATE (window);

  priv->contacts_list = NULL;
  priv->active = 0;
  priv->activated = FALSE;

  if (!CLUTTER_IS_TEXTURE (row_texture))
    {
      row_pixbuf = gdk_pixbuf_new_from_file (PKGDATADIR "/applet_bg.png", NULL);
      row_texture = g_object_new (CLUTTER_TYPE_TEXTURE,
                                  "pixbuf", row_pixbuf,
                                  "tiled", FALSE,
                                  NULL);

    }

  priv->row = tidy_texture_frame_new (CLUTTER_TEXTURE (row_texture), 
                                      15, 15, 15, 15);
  clutter_container_add_actor (CLUTTER_CONTAINER (window), priv->row);
  clutter_actor_set_size (priv->row, CSW ()*0.45, CSH()/10);
  clutter_actor_set_anchor_point_from_gravity (priv->row, 
                                               CLUTTER_GRAVITY_WEST);
  clutter_actor_set_position (priv->row, CSW ()/2, CSH()/2);


  priv->contacts = clutter_group_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (window), priv->contacts);
  clutter_actor_set_size (priv->contacts, CSW(), CSH());
  clutter_actor_set_position (priv->contacts, CSW()/2, 0);

  load_contacts (window);
  
  ensure_layout (window);

  priv->timeline = clutter_timeline_new_for_duration (600);
  priv->alpha = clutter_alpha_new_full (priv->timeline,
                                        clutter_sine_inc_func,
                                        NULL, NULL);
  priv->behave = astro_behave_new (priv->alpha,
                                   (AstroBehaveAlphaFunc)astro_contacts_list_alpha,
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
astro_contacts_window_new (void)
{
  AstroWindow *contacts_window =  g_object_new (ASTRO_TYPE_CONTACTS_WINDOW,
									                       NULL);

  return contacts_window;
}

