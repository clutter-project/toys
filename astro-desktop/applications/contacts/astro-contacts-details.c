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


#include "astro-contacts-details.h"

#include <libastro-desktop/astro-defines.h>
#include <libastro-desktop/astro-utils.h>

#include "astro-texture-group.h"

G_DEFINE_TYPE (AstroContactDetails, astro_contact_details, CLUTTER_TYPE_GROUP);

#define ASTRO_CONTACT_DETAILS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
        ASTRO_TYPE_CONTACT_DETAILS, AstroContactDetailsPrivate))

#define PADDING 12

struct _AstroContactDetailsPrivate
{
  AstroContact *contact;
  ClutterActor *address;
  ClutterActor *tel;
  ClutterActor *email;
  
  ClutterEffectTemplate *details_temp;
  ClutterTimeline       *details_time;
  ClutterTimeline       *timeline;
};

void 
on_fade_out (AstroContactDetails *details)
{
  AstroContactDetailsPrivate *priv;

  g_return_if_fail (ASTRO_IS_CONTACT_DETAILS (details));
  priv = details->priv;
  
  astro_texture_group_set_text (ASTRO_TEXTURE_GROUP (priv->address),
                                priv->contact->address);
  astro_texture_group_set_text (ASTRO_TEXTURE_GROUP (priv->tel),
                                priv->contact->tel);
  astro_texture_group_set_text (ASTRO_TEXTURE_GROUP (priv->email),
                                priv->contact->email);

  clutter_actor_set_y (priv->address, 0);
  clutter_actor_set_y (priv->tel,   
                       clutter_actor_get_height (priv->address) + PADDING);
  clutter_actor_set_y (priv->email,
                       clutter_actor_get_y (priv->tel) +
                       clutter_actor_get_height (priv->tel) + PADDING);

  clutter_actor_set_y (CLUTTER_ACTOR (details), 
                       (CSH()/2)-(clutter_actor_get_height (CLUTTER_ACTOR (details))/2));

  priv->timeline = clutter_effect_fade (priv->details_temp,
                                  CLUTTER_ACTOR (details),
                                  255,
                                  NULL, NULL);

  g_debug ("on_fade_out\n");
}

void
astro_contact_details_set_active (AstroContactDetails *details,
                                  AstroContact        *contact)
{
  AstroContactDetailsPrivate *priv;
  
  g_return_if_fail (ASTRO_IS_CONTACT_DETAILS (details));
  priv = details->priv;

  priv->contact = contact;
  
  if (CLUTTER_IS_TIMELINE (priv->timeline))
    g_object_unref (G_OBJECT (priv->timeline));

  priv->timeline = clutter_effect_fade (priv->details_temp,
                                  CLUTTER_ACTOR (details),
                                  0,
                                 (ClutterEffectCompleteFunc)on_fade_out,
                                 details);

 g_debug ("set_active"); 
}


/* GObject stuff */
static void
astro_contact_details_class_init (AstroContactDetailsClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (gobject_class, sizeof (AstroContactDetailsPrivate));
}

static void
astro_contact_details_init (AstroContactDetails *details)
{
  AstroContactDetailsPrivate *priv;
  priv = details->priv = ASTRO_CONTACT_DETAILS_GET_PRIVATE (details);
  
  priv->address = astro_texture_group_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (details), priv->address);
  
  priv->tel = astro_texture_group_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (details), priv->tel);

  priv->email = astro_texture_group_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (details), priv->email);

  priv->details_time = clutter_timeline_new_for_duration (500);
  priv->details_temp = clutter_effect_template_new (priv->details_time, 
                                                    clutter_sine_inc_func);

  clutter_actor_set_opacity (CLUTTER_ACTOR (details), 0);
  clutter_actor_show_all (CLUTTER_ACTOR (details));
}

ClutterActor *
astro_contact_details_new (void)
{
  ClutterActor *details =  g_object_new (ASTRO_TYPE_CONTACT_DETAILS,
                                                 NULL);
  return CLUTTER_ACTOR (details);
}

