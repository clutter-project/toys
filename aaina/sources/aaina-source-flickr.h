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

#include <libaaina/aaina-library.h>
#include <libaaina/aaina-source.h>

#ifndef _HAVE_AAINA_SOURCE_FLICKR_H
#define _HAVE_AAINA_SOURCE_FLICKR_H

G_BEGIN_DECLS

#define AAINA_TYPE_SOURCE_FLICKR aaina_source_flickr_get_type()

#define AAINA_SOURCE_FLICKR(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	AAINA_TYPE_SOURCE_FLICKR, \
	AainaSourceFlickr))

#define AAINA_SOURCE_FLICKR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	AAINA_TYPE_SOURCE_FLICKR, \
	AainaSourceFlickrClass))

#define AAINA_IS_SOURCE_FLICKR(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	AAINA_TYPE_SOURCE_FLICKR))

#define AAINA_IS_SOURCE_FLICKR_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
	AAINA_TYPE_SOURCE_FLICKR))

#define AAINA_SOURCE_FLICKR_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	AAINA_TYPE_SOURCE_FLICKR, \
	AainaSourceFlickrClass))

typedef struct _AainaSourceFlickr AainaSourceFlickr;
typedef struct _AainaSourceFlickrClass AainaSourceFlickrClass;
typedef struct _AainaSourceFlickrPrivate AainaSourceFlickrPrivate;

struct _AainaSourceFlickr
{
	AainaSource         parent;

  AainaSourceFlickrPrivate *priv;
};

struct _AainaSourceFlickrClass 
{
	
	AainaSourceClass parent_class;

  void (*_aaina_source_flickr_1) (void);
  void (*_aaina_source_flickr_2) (void);
  void (*_aaina_source_flickr_3) (void);
  void (*_aaina_source_flickr_4) (void);
}; 

GType aaina_source_flickr_get_type (void) G_GNUC_CONST;

AainaSource* 
aaina_source_flickr_new (AainaLibrary *library, const gchar *tags);

G_END_DECLS

#endif
