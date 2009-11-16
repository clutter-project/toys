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

#include "aaina-source.h"

G_DEFINE_ABSTRACT_TYPE (AainaSource, aaina_source, G_TYPE_OBJECT);
	
/* GObject stuff */
static void
aaina_source_class_init (AainaSourceClass *klass)
{
  ;
}


static void
aaina_source_init (AainaSource *source)
{
  ;
}

AainaSource*
aaina_source_new (AainaLibrary *library)
{
  AainaSource         *source;

  source = g_object_new (AAINA_TYPE_SOURCE, NULL);
  
  return source;
}

