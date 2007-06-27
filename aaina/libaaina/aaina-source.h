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


#include <config.h>
#include <glib.h>
#include <clutter/clutter.h>

#include "aaina-library.h"

#ifndef _HAVE_AAINA_SOURCE_H
#define _HAVE_AAINA_SOURCE_H

G_BEGIN_DECLS

#define AAINA_TYPE_SOURCE aaina_source_get_type()

#define AAINA_SOURCE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	AAINA_TYPE_SOURCE, \
	AainaSource))

#define AAINA_SOURCE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	AAINA_TYPE_SOURCE, \
	AainaSourceClass))

#define AAINA_IS_SOURCE(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	AAINA_TYPE_SOURCE))

#define AAINA_IS_SOURCE_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
	AAINA_TYPE_SOURCE))

#define AAINA_SOURCE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	AAINA_TYPE_SOURCE, \
	AainaSourceClass))

typedef struct _AainaSource AainaSource;
typedef struct _AainaSourceClass AainaSourceClass;

struct _AainaSource
{
	ClutterGroup         parent;
};

struct _AainaSourceClass 
{
	
	ClutterGroupClass parent_class;

  void (*_aaina_source_1) (void);
  void (*_aaina_source_2) (void);
  void (*_aaina_source_3) (void);
  void (*_aaina_source_4) (void);
}; 

GType aaina_source_get_type (void) G_GNUC_CONST;

AainaSource* 
aaina_source_new (AainaLibrary *library);

G_END_DECLS

#endif
