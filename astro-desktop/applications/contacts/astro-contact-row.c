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
 * Library General Public License for more row.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Neil Jagdish Patel <njp@o-hand.com>
 */


#include "astro-contact-row.h"

#include <libastro-desktop/astro-defines.h>
#include <libastro-desktop/astro-behave.h>

#include <tidy/tidy-texture-frame.h>


G_DEFINE_TYPE (AstroContactRow, astro_contact_row, CLUTTER_TYPE_GROUP);

#define ASTRO_CONTACT_ROW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
        ASTRO_TYPE_CONTACT_ROW, AstroContactRowPrivate))

#define PADDING 8
#define ICON_SIZE (ROW_HEIGHT - (PADDING * 2))

struct _AstroContactRowPrivate
{
  gchar     *name;
  GdkPixbuf *icon;
  gboolean   active;

  /* Actors */
  ClutterActor *bg;
  ClutterActor *texture;
  ClutterActor *label;
  ClutterActor *bar;

  /* Timelines */
  ClutterEffectTemplate *active_temp;
  ClutterTimeline       *active_time;
  ClutterEffectTemplate *bar_temp;
  ClutterTimeline       *bar_time;
  ClutterAlpha          *alpha;
  ClutterBehaviour      *behave;
};

enum
{
  PROP_0,

  PROP_NAME,
  PROP_ICON
};

static GdkPixbuf    *bg_pixbuf = NULL;
static ClutterActor *bg_texture = NULL;

/* Public Functions */
static void
on_active_completed (ClutterActor *actor, gpointer data)
{
  AstroContactRowPrivate *priv;

  g_return_if_fail (ASTRO_IS_CONTACT_ROW (data));
  priv = ASTRO_CONTACT_ROW_GET_PRIVATE (data);

  if (clutter_timeline_is_playing (priv->bar_time))
    return;

   priv->bar_time = clutter_effect_fade (priv->bar_temp,
                       priv->bar,
                       255,
                       NULL, NULL);
}

static void
on_inactive_completed (ClutterActor *actor, gpointer data)
{
  AstroContactRowPrivate *priv;

  g_return_if_fail (ASTRO_IS_CONTACT_ROW (data));
  priv = ASTRO_CONTACT_ROW_GET_PRIVATE (data);

  clutter_actor_set_height (priv->bg, ROW_HEIGHT);
  clutter_actor_set_opacity (priv->bar, 0);
}

void            
astro_contact_row_set_active (AstroContactRow *row,
                              gboolean         active)
{
  AstroContactRowPrivate *priv;
  static ClutterTimeline *active_time = NULL;
 
  g_return_if_fail (ASTRO_IS_CONTACT_ROW (row));
  priv = row->priv;

  if (priv->active == active)
    return;
   
  priv->active = active;

  if (active)
    {
      active_time = clutter_effect_fade (priv->active_temp,
                                         priv->bg,
                                         255,
                                         on_active_completed, row);
      clutter_timeline_start (priv->active_time);    
    }
  else
    {
      active_time = clutter_effect_fade (priv->active_temp,
                                         priv->bg,
                                         0,
                                         on_inactive_completed, row);
      if (clutter_timeline_is_playing (priv->bar_time))
        clutter_timeline_stop (priv->bar_time);

      priv->bar_time = clutter_effect_fade (priv->active_temp,
                                     priv->bar,
                                     0,
                                     NULL, NULL);
    }
}

/* Private functions */
static void
astro_contact_row_set_name (AstroContactRow *row, const gchar *name)
{
  AstroContactRowPrivate *priv;

  g_return_if_fail (ASTRO_IS_CONTACT_ROW (row));
  g_return_if_fail (name);
  priv = row->priv;

  if (priv->name)
    g_free (priv->name);
  priv->name = g_strdup (name);

  clutter_label_set_text (CLUTTER_LABEL (priv->label), name);

  clutter_actor_set_position (priv->label,PADDING, 
                    (ROW_HEIGHT /2)-(clutter_actor_get_height (priv->label)/2));
}

static void
astro_contact_row_set_icon (AstroContactRow *row, GdkPixbuf *icon)
{
  AstroContactRowPrivate *priv;

  g_return_if_fail (ASTRO_IS_CONTACT_ROW (row));
  g_return_if_fail (GDK_IS_PIXBUF (icon));
  priv = row->priv;

  priv->icon = icon;

  g_object_set (G_OBJECT (priv->texture), "pixbuf", icon, NULL);
  clutter_actor_set_size (priv->texture, ICON_SIZE, ICON_SIZE);
}

static void
_resize_alpha (ClutterBehaviour *behave, 
               guint32           alpha,
               AstroContactRow  *row)
{
  AstroContactRowPrivate *priv;
  gfloat factor;
  gint dest_height = ROW_HEIGHT;
  gint current_height, diff_height;

  g_return_if_fail (ASTRO_IS_CONTACT_ROW (row));
  priv = row->priv;

  factor = (gfloat)alpha/CLUTTER_ALPHA_MAX_ALPHA;

  if (priv->active)
    dest_height = (ROW_HEIGHT * 2) + PADDING;

  current_height = clutter_actor_get_height (priv->bg);

  if (current_height > dest_height)
    diff_height = (current_height - dest_height) * -1;
  else
    diff_height = dest_height - current_height;

  clutter_actor_set_height (priv->bg, 
            current_height + ((diff_height * alpha)/CLUTTER_ALPHA_MAX_ALPHA));
} 

/* GObject stuff */
static void
astro_contact_row_set_property (GObject      *object, 
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  AstroContactRowPrivate *priv;
  
  g_return_if_fail (ASTRO_IS_CONTACT_ROW (object));
  priv = ASTRO_CONTACT_ROW (object)->priv;

  switch (prop_id)
  {
    case PROP_NAME:
      astro_contact_row_set_name (ASTRO_CONTACT_ROW (object),
                                  g_value_get_string (value));
      break;
    case PROP_ICON:
      astro_contact_row_set_icon (ASTRO_CONTACT_ROW (object),
                                  GDK_PIXBUF (g_value_get_object (value)));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
astro_contact_row_get_property (GObject    *object, 
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  AstroContactRowPrivate *priv;
  
  g_return_if_fail (ASTRO_IS_CONTACT_ROW (object));
  priv = ASTRO_CONTACT_ROW (object)->priv;

  switch (prop_id)
  {
    case PROP_NAME:
      g_value_set_string (value, priv->name);
    case PROP_ICON:
      g_value_set_object (value, G_OBJECT (priv->icon));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}
  
static void
astro_contact_row_class_init (AstroContactRowClass *klass)
{
  GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = astro_contact_row_set_property;
  gobject_class->get_property = astro_contact_row_get_property;

  g_object_class_install_property (
    gobject_class,
    PROP_NAME,
    g_param_spec_string ("contact_name",
                         "Contact Name",
                         "The contacts name",
                         " ",
                         G_PARAM_READWRITE));

  g_object_class_install_property (
    gobject_class,
    PROP_ICON,
    g_param_spec_object ("contact_icon",
                         "Contact Icon",
                         "The contacts icon",
                         GDK_TYPE_PIXBUF,
                         G_PARAM_READWRITE));


  g_type_class_add_private (gobject_class, sizeof (AstroContactRowPrivate));
}

static void
astro_contact_row_init (AstroContactRow *row)
{
  AstroContactRowPrivate *priv;
  ClutterColor white = { 0xff, 0xff, 0xff, 0xff };
  gchar *font = NULL;
  GdkPixbuf *pixbuf;
  
  priv = row->priv = ASTRO_CONTACT_ROW_GET_PRIVATE (row);

  priv->name = NULL;
  priv->icon = NULL;
  priv->active = FALSE;

  /* The background texture */
  if (!GDK_IS_PIXBUF (bg_pixbuf))
    bg_pixbuf = gdk_pixbuf_new_from_file (PKGDATADIR"/applet_bg.png", NULL);
  if (!CLUTTER_IS_ACTOR (bg_texture))
    {
      bg_texture = clutter_texture_new_from_pixbuf (bg_pixbuf);
      clutter_actor_show (bg_texture);
    }
  
  priv->bg = tidy_texture_frame_new (CLUTTER_TEXTURE (bg_texture), 
                                     15, 15, 15, 15);
  clutter_container_add_actor (CLUTTER_CONTAINER (row), priv->bg);
  clutter_actor_set_position (priv->bg, 0, 0);
  clutter_actor_set_size (priv->bg, CSW()*0.5, ROW_HEIGHT);
  clutter_actor_set_opacity (priv->bg, 0);

  /* The icon */
  priv->texture = clutter_texture_new ();
  //clutter_container_add_actor (CLUTTER_CONTAINER (row), priv->texture);
  clutter_actor_set_position (priv->texture, PADDING, PADDING);
  clutter_actor_set_size (priv->texture, ICON_SIZE, ICON_SIZE);

  /* The label */
  font = g_strdup_printf ("Sans %d", (gint)(ROW_HEIGHT * 0.5));
  priv->label = clutter_label_new_full (font, " ", &white);
  clutter_label_set_line_wrap (CLUTTER_LABEL (priv->label), FALSE);
  clutter_actor_set_width (priv->label, CSW()/2);
  clutter_container_add_actor (CLUTTER_CONTAINER (row), priv->label);
  clutter_actor_set_position (priv->label, (PADDING), 
                              ROW_HEIGHT /2);
  g_free (font);

  /* Contact bar */
  pixbuf = gdk_pixbuf_new_from_file_at_scale (PKGDATADIR"/contact-bar.svg", 
                                              -1, ROW_HEIGHT-(PADDING*2), TRUE,
                                              NULL);
  priv->bar = clutter_texture_new_from_pixbuf (pixbuf);
  clutter_container_add_actor (CLUTTER_CONTAINER (row), priv->bar);
  clutter_actor_set_position (priv->bar, 
                              PADDING,
                              ROW_HEIGHT + PADDING);
  clutter_actor_set_opacity (priv->bar, 0);

  /* Timelines */
  priv->active_time = clutter_timeline_new_for_duration (200);
  priv->active_temp = clutter_effect_template_new (priv->active_time,
                                                   clutter_sine_inc_func);
  priv->bar_time = clutter_timeline_new_for_duration (600);
  priv->bar_temp = clutter_effect_template_new (priv->bar_time,
                                                clutter_sine_inc_func);

  priv->active_time = clutter_timeline_new_for_duration (1200);
  priv->alpha = clutter_alpha_new_full (priv->active_time,
                                        clutter_sine_inc_func,
                                        NULL, NULL);
  priv->behave = astro_behave_new (priv->alpha,
                                   (AstroBehaveAlphaFunc)_resize_alpha,
                                   row);

  clutter_actor_show_all (CLUTTER_ACTOR (row));
}

ClutterActor * 
astro_contact_row_new (const gchar *name, GdkPixbuf *icon)
{
  ClutterActor *row =  g_object_new (ASTRO_TYPE_CONTACT_ROW,
                                     "contact_name", name,
                                     "contact_icon", icon,
								                     NULL);
  return row;
}

