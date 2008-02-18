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
#include <libastro-desktop/astro.h>
#include <libastro-desktop/astro-application.h>
#include <libastro-desktop/astro-window.h>
#include <libastro-desktop/astro-behave.h>
#include <libastro-desktop/tidy-texture-frame.h>

#include "astro-contact-row.h"
#include "astro-contacts-details.h"

G_DEFINE_TYPE (AstroContactsWindow, astro_contacts_window, ASTRO_TYPE_WINDOW);

#define ASTRO_CONTACTS_WINDOW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
        ASTRO_TYPE_CONTACTS_WINDOW, AstroContactsWindowPrivate))

#define ALBUM_SIZE (CSW()/4)
#define OH_ADDRESS "Unit R, Homesdale Business Centre\n216-218 Homesdale Road\nBromley, BR12QZ"

#define OH_TEL "01923 820 124"

struct _AstroContactsWindowPrivate
{
  GList *contacts_list;

  ClutterActor *contacts;
  ClutterActor *contacts_eventbox;
  ClutterActor *label;

  ClutterActor *details;

  gint active;
  gboolean activated;

  ClutterTimeline  *timeline;
  ClutterAlpha     *alpha;
  ClutterBehaviour *behave;

  gint     starty;
  gint     endy;
  gint     lasty;
  guint32  start_time;
  gboolean mousedown;
};

static AstroContact contacts[] = {
  {"Andrew Zaborowski", OH_ADDRESS, OH_TEL, "andrew@o-hand.com"},
  {"Chris Lord", OH_ADDRESS, OH_TEL, "chris@o-hand.com"},
  {"Dodji Seketeli", OH_ADDRESS, OH_TEL, "dodji@o-hand.com"},
  {"Emmanuele Bassi", OH_ADDRESS, OH_TEL, "ebassi@o-hand.com"},
  {"Iain Holmes", OH_ADDRESS, OH_TEL, "iain@o-hand.com"},
  {"Jorn Baayen", OH_ADDRESS, OH_TEL, "jorn@o-hand.com"},
  {"Jussi Kukkonen", OH_ADDRESS, OH_TEL, "jku@o-hand.com"},
  {"Marcin Juszkiewicz", OH_ADDRESS, OH_TEL, "hrw@o-hand.com"},
  {"Matthew Allum", OH_ADDRESS, OH_TEL, "mallum@o-hand.com"},
  {"Neil J. Patel", OH_ADDRESS, OH_TEL, "njp@o-hand.com"},
  {"Øyvind Kolås", OH_ADDRESS, OH_TEL, "pippin@o-hand.com"},
  {"Paul Cooper", OH_ADDRESS, OH_TEL, "pgc@o-hand.com"},
  {"Richard Purdie", OH_ADDRESS, OH_TEL, "rp@o-hand.com"},
  {"Robert Bradford", OH_ADDRESS, OH_TEL, "rob@o-hand.com"},
  {"Ross Burton", OH_ADDRESS, OH_TEL, "ross@o-hand.com"},
  {"Samuel Ortiz", OH_ADDRESS, OH_TEL, "sameo@o-hand.com"},
  {"Sidske Allum", OH_ADDRESS, OH_TEL, "sid@o-hand.com"},
  {"Thomas Wood", OH_ADDRESS, OH_TEL, "thomas@o-hand.com"},
  {"Tomas Frydrych", OH_ADDRESS, OH_TEL, "tf@o-hand.com"}
};



static void on_main_timeline_completed (ClutterTimeline  *timeline,
                                        AstroContactsWindow *window);

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
#define SPACING (ROW_HEIGHT * 1.5)
  AstroContactsWindowPrivate *priv;
  GList *c;
  gint i = 0;

  priv = window->priv;

  c = priv->contacts_list;
  for (c=c; c; c = c->next)
    {
      ClutterActor *contact = c->data;
      ContactTrans *trans = g_object_get_data (G_OBJECT (contact), "trans");
      gboolean active = FALSE;

      if (i == priv->active)
        {
          trans->y = CSH ()/2;
          trans->scale = 1.0;          
        }
      else if (i > priv->active)
        {
          gint diff;

          diff = i - priv->active;
          trans->y = (CSH()/2) + (SPACING * diff);
          if (diff > MAX_DIST)
            trans->scale = 0.4;
          else
            trans->scale = 0.4 + (0.4 * (MAX_DIST-diff)/MAX_DIST);

        }
      else
        {
          gint diff;

          diff = priv->active - i;
          trans->y = (CSH()/2) - (SPACING * diff);
          if (diff > MAX_DIST)
            trans->scale = 0.4;
          else
            trans->scale = 0.4 + (0.4 * (MAX_DIST-diff)/MAX_DIST);        
        }

      astro_contact_row_set_active (ASTRO_CONTACT_ROW (contact), active);

      i++;
    }
}

static void
ensure_layout_proper (AstroContactsWindow *window)
{
#define MAX_DIST 4
#define SPACING (ROW_HEIGHT * 1.5)
  AstroContactsWindowPrivate *priv;
  GList *c;
  gint i = 0;

  priv = window->priv;

  c = priv->contacts_list;
  for (c=c; c; c = c->next)
    {
      ClutterActor *contact = c->data;
      ContactTrans *trans = g_object_get_data (G_OBJECT (contact), "trans");
      gboolean active = FALSE;

      if (i == priv->active)
        {
          trans->y = CSH ()/2;
          trans->scale = 1.0;          
          active = TRUE;

          astro_contact_details_set_active (ASTRO_CONTACT_DETAILS (priv->details), 
              &contacts[g_list_index (priv->contacts_list, contact)]);
        }
      else if (i > priv->active)
        {
          gint diff;

          diff = i - priv->active;
          trans->y = (CSH()/2) + (SPACING * diff);
          trans->y += ROW_HEIGHT * 1;
          if (diff > MAX_DIST)
            trans->scale = 0.4;
          else
            trans->scale = 0.4 + (0.4 * (MAX_DIST-diff)/MAX_DIST);
          }
      else
        {
          gint diff;

          diff = priv->active - i;
          trans->y = (CSH()/2) - (SPACING * diff);
          if (diff > MAX_DIST)
            trans->scale = 0.4;
          else
            trans->scale = 0.4 + (0.4 * (MAX_DIST-diff)/MAX_DIST);        
        }

      astro_contact_row_set_active (ASTRO_CONTACT_ROW (contact), active);

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

  g_signal_connect (priv->timeline, "completed",
                    G_CALLBACK (on_main_timeline_completed), window);

  if (clutter_timeline_is_playing (priv->timeline))
    clutter_timeline_stop (priv->timeline);

  clutter_timeline_start (priv->timeline);

}

static gboolean
on_event (ClutterActor        *contacts, 
          ClutterEvent        *event,
          AstroContactsWindow *window)
{
  AstroContactsWindowPrivate *priv;

  g_return_val_if_fail (ASTRO_IS_CONTACTS_WINDOW (window), FALSE);
  priv = window->priv;

  if (event->type == CLUTTER_BUTTON_PRESS)
    {   
      priv->mousedown = TRUE;
      priv->starty = priv->lasty = event->button.y;
      priv->start_time = event->button.time;

      priv->active = -1;
      clutter_timeline_start (priv->timeline);

      g_debug ("button-press\n");
    }
  else if (event->type == CLUTTER_MOTION)
    {
      gint offset;
     
      if (!priv->mousedown)
        return FALSE;

      if (event->motion.y > priv->lasty)
        offset = event->motion.y - priv->lasty;
      else
        offset = -1 * (priv->lasty - event->motion.y);

      priv->lasty = event->motion.y;

      clutter_actor_set_y (priv->contacts, 
                           clutter_actor_get_y (priv->contacts) + offset);

      g_debug ("button-motion\n");
      return TRUE;
    }
  else if (event->type == CLUTTER_BUTTON_RELEASE)
    {
      gint endy;

      endy = event->button.y - priv->starty;
     

      g_print ("endy = %d\n", endy);

      priv->mousedown = FALSE;
      g_debug ("button-release\n");
    } 
  return FALSE;
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
  ClutterActor *row;
  static GdkPixbuf *face = NULL;

  if (!face)
    face = gdk_pixbuf_new_from_file (PKGDATADIR"/face.png", NULL);

  row = astro_contact_row_new (name, face);

  clutter_actor_set_anchor_point_from_gravity (row, CLUTTER_GRAVITY_WEST);

  g_object_set_data (G_OBJECT (row), "trans", g_new0 (ContactTrans, 1));

  return row;
}

static void
load_contacts (AstroContactsWindow *window)
{
#define PADDING 10
  AstroContactsWindowPrivate *priv;
  gint i = 0;
  
  priv = window->priv;

  for (i = 0; i < G_N_ELEMENTS (contacts); i++)
     {
      ClutterActor *contact;

      contact = make_contact (contacts[i].name);
      clutter_container_add_actor (CLUTTER_CONTAINER (priv->contacts), contact);
      clutter_actor_set_position (contact, PADDING, CSH());
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

      //clutter_actor_set_y (contact, currenty + (gint)(diffy*factor));
      clutter_actor_set_y (contact, 
                           currenty + 
                           (gint)((diffy*alpha_value)/CLUTTER_ALPHA_MAX_ALPHA));

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

  g_signal_handlers_disconnect_by_func (timeline, 
                                        on_main_timeline_completed,
                                        window);
  
  ensure_layout_proper (window);
  clutter_timeline_start (priv->timeline);
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
            priv->activated = FALSE;
          }
        astro_contacts_list_window_advance (window, -1);
        break;
      case CLUTTER_Down:
      case CLUTTER_KP_Down:
        if (priv->activated)
          {
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

  priv->contacts = clutter_group_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (window), priv->contacts);
  clutter_actor_set_size (priv->contacts, CSW(), CSH());
  clutter_actor_set_position (priv->contacts, 0, 0);

  priv->contacts_eventbox = clutter_rectangle_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (window),
                               priv->contacts_eventbox);
  clutter_actor_set_position (priv->contacts_eventbox, 0, 0);
  clutter_actor_set_size (priv->contacts_eventbox, CSW()/2, CSH());
  clutter_actor_set_opacity (priv->contacts_eventbox, 0);
  clutter_actor_set_reactive (priv->contacts_eventbox, TRUE);

  priv->details = astro_contact_details_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (window), priv->details);
  clutter_actor_set_position (priv->details, CSW()*0.54, 0);

  load_contacts (window);
  
  ensure_layout (window);

  priv->timeline = clutter_timeline_new_for_duration (800);
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

  astro_utils_set_clip (CLUTTER_ACTOR (window), 0, ASTRO_PANEL_HEIGHT (),
                        CSW(), CSH());

  g_signal_connect (priv->contacts_eventbox, "event",
                    G_CALLBACK (on_event), window);

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
