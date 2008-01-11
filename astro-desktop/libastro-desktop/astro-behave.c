/*
 * Copyright (C) 2007 Intel
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
 *
 * Authored by Neil Jagdish Patel <njp@o-hand.com>
 *
 */

#include "astro-behave.h"

G_DEFINE_TYPE (AstroBehave, astro_behave, CLUTTER_TYPE_BEHAVIOUR);

#define ASTRO_BEHAVE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
  ASTRO_TYPE_BEHAVE, \
  AstroBehavePrivate))

struct _AstroBehavePrivate
{
  AstroBehaveAlphaFunc     func;
  gpointer		data;
};

static void
astro_behave_alpha_notify (ClutterBehaviour *behave, guint32 alpha_value)
{
  AstroBehave *astro_behave = ASTRO_BEHAVE(behave);
  AstroBehavePrivate *priv;
	
  priv = ASTRO_BEHAVE_GET_PRIVATE (astro_behave);
	
  if (priv->func != NULL) 
    priv->func (behave, alpha_value, priv->data);
}

static void
astro_behave_class_init (AstroBehaveClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
  ClutterBehaviourClass *behave_class = CLUTTER_BEHAVIOUR_CLASS (klass);

  behave_class->alpha_notify = astro_behave_alpha_notify;
	
  g_type_class_add_private (gobject_class, sizeof (AstroBehavePrivate));
}

static void
astro_behave_init (AstroBehave *self)
{
  AstroBehavePrivate *priv;
	
  priv = ASTRO_BEHAVE_GET_PRIVATE (self);
	
  priv->func = NULL;
  priv->data = NULL;
}

ClutterBehaviour*
astro_behave_new (ClutterAlpha 		       *alpha,
                     AstroBehaveAlphaFunc 	func,
                     gpointer		              data)
{
  AstroBehave *behave;
  AstroBehavePrivate *priv;
	
  behave = g_object_new (ASTRO_TYPE_BEHAVE, 
                         "alpha", alpha,
                         NULL);

  priv = ASTRO_BEHAVE_GET_PRIVATE (behave);  
	
  priv->func = func;
  priv->data = data;
		
  return CLUTTER_BEHAVIOUR(behave);
}
