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
 * Library General Public License for more group.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Neil Jagdish Patel <njp@o-hand.com>
 */


#include "astro-texture-group.h"

#include <libastro-desktop/astro-defines.h>
#include <libastro-desktop/astro-behave.h>

#include <libastro-desktop/tidy-texture-frame.h>


G_DEFINE_TYPE (AstroTextureGroup, astro_texture_group, CLUTTER_TYPE_GROUP);

#define ASTRO_TEXTURE_GROUP_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
        ASTRO_TYPE_TEXTURE_GROUP, AstroTextureGroupPrivate))

#define PADDING 8
#define GROUP_WIDTH (CSW()*0.4)
#define GROUP_HEIGHT (CSH()/10) 

static GdkPixbuf    *bg_pixbuf = NULL;
static ClutterActor *bg_texture = NULL;

struct _AstroTextureGroupPrivate
{
  ClutterActor *bg;
  ClutterActor *label;
};

enum
{
  PROP_0,

  PROP_TEXT,
};


/* Public Functions */
void
astro_texture_group_set_text (AstroTextureGroup *group, const gchar *text)
{
  AstroTextureGroupPrivate *priv;

  g_return_if_fail (ASTRO_IS_TEXTURE_GROUP (group));
  g_return_if_fail (text);
  priv = group->priv;

  clutter_label_set_text (CLUTTER_LABEL (priv->label), text);

  clutter_actor_set_position (priv->label, PADDING, PADDING);
  
  clutter_actor_set_size (priv->bg, 
                          GROUP_WIDTH,
                          clutter_actor_get_height (priv->label) + (2*PADDING));
  clutter_actor_set_position (priv->bg, 0, 0);
}

/* GObject stuff */
static void
astro_texture_group_set_property (GObject      *object, 
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  AstroTextureGroupPrivate *priv;
  
  g_return_if_fail (ASTRO_IS_TEXTURE_GROUP (object));
  priv = ASTRO_TEXTURE_GROUP (object)->priv;

  switch (prop_id)
  {
    case PROP_TEXT:
      astro_texture_group_set_text (ASTRO_TEXTURE_GROUP (object),
                                  g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
astro_texture_group_get_property (GObject    *object, 
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  AstroTextureGroupPrivate *priv;
  
  g_return_if_fail (ASTRO_IS_TEXTURE_GROUP (object));
  priv = ASTRO_TEXTURE_GROUP (object)->priv;

  switch (prop_id)
  {
    case PROP_TEXT:
      g_value_set_string (value, 
                          clutter_label_get_text (CLUTTER_LABEL (priv->label)));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}
  
static void
astro_texture_group_class_init (AstroTextureGroupClass *klass)
{
  GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = astro_texture_group_set_property;
  gobject_class->get_property = astro_texture_group_get_property;

  g_object_class_install_property (
    gobject_class,
    PROP_TEXT,
    g_param_spec_string ("text",
                         "Text",
                         "The text to display",
                         " ",
                         G_PARAM_READWRITE));

  g_type_class_add_private (gobject_class, sizeof (AstroTextureGroupPrivate));
}

static void
astro_texture_group_init (AstroTextureGroup *group)
{
  AstroTextureGroupPrivate *priv;
  ClutterColor white = { 0xff, 0xff, 0xff, 0xff };
  gchar *font = NULL;

  
  priv = group->priv = ASTRO_TEXTURE_GROUP_GET_PRIVATE (group);

  /* The background texture */
  if (!GDK_IS_PIXBUF (bg_pixbuf))
    bg_pixbuf = gdk_pixbuf_new_from_file (PKGDATADIR"/info_bg.png", NULL);
  if (!CLUTTER_IS_ACTOR (bg_texture))
    {
      bg_texture = clutter_texture_new_from_pixbuf (bg_pixbuf);
      clutter_actor_show (bg_texture);
    }
  
  priv->bg = tidy_texture_frame_new (CLUTTER_TEXTURE (bg_texture), 
                                     15, 15, 15, 15);
  clutter_container_add_actor (CLUTTER_CONTAINER (group), priv->bg);
  clutter_actor_set_position (priv->bg, 0, 0);
  clutter_actor_set_size (priv->bg, GROUP_WIDTH, GROUP_HEIGHT);


  /* The label */
  font = g_strdup_printf ("Sans %d", (gint)(GROUP_HEIGHT * 0.3));
  priv->label = clutter_label_new_full (font, " ", &white);
  clutter_label_set_line_wrap (CLUTTER_LABEL (priv->label), TRUE);
  clutter_actor_set_width (priv->label, GROUP_WIDTH);
  clutter_container_add_actor (CLUTTER_CONTAINER (group), priv->label);
  clutter_actor_set_position (priv->label, PADDING, GROUP_HEIGHT /2);
  g_free (font);

  clutter_actor_show_all (CLUTTER_ACTOR (group));
}

ClutterActor * 
astro_texture_group_new ()
{
  ClutterActor *group =  g_object_new (ASTRO_TYPE_TEXTURE_GROUP,
         					                     NULL);
  return group;
}

