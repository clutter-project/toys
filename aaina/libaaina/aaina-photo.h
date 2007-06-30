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

#ifndef _HAVE_AAINA_PHOTO_H
#define _HAVE_AAINA_PHOTO_H

G_BEGIN_DECLS

#define AAINA_TYPE_PHOTO aaina_photo_get_type()

#define AAINA_PHOTO(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	AAINA_TYPE_PHOTO, \
	AainaPhoto))

#define AAINA_PHOTO_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	AAINA_TYPE_PHOTO, \
	AainaPhotoClass))

#define AAINA_IS_PHOTO(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	AAINA_TYPE_PHOTO))

#define AAINA_IS_PHOTO_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
	AAINA_TYPE_PHOTO))

#define AAINA_PHOTO_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	AAINA_TYPE_PHOTO, \
	AainaPhotoClass))

typedef struct _AainaPhoto AainaPhoto;
typedef struct _AainaPhotoClass AainaPhotoClass;
typedef struct _AainaPhotoPrivate AainaPhotoPrivate;

struct _AainaPhoto
{
	ClutterGroup         parent;
	
	/* private */
	AainaPhotoPrivate   *priv;
};

struct _AainaPhotoClass 
{
	/*< private >*/
	ClutterGroupClass parent_class;

  void (*_aaina_photo_1) (void);
  void (*_aaina_photo_2) (void);
  void (*_aaina_photo_3) (void);
  void (*_aaina_photo_4) (void);
}; 

GType aaina_photo_get_type (void) G_GNUC_CONST;

ClutterActor* 
aaina_photo_new (void);

void
aaina_photo_set_pixbuf (AainaPhoto *photo, GdkPixbuf *pixbuf);

void
aaina_photo_save (AainaPhoto *photo);
void
aaina_photo_restore (AainaPhoto *photo);

gdouble
aaina_photo_get_scale (AainaPhoto *photo);
void
aaina_photo_set_scale (AainaPhoto *photo, gdouble scale);

gboolean
aaina_photo_get_viewed (AainaPhoto *photo);
void
aaina_photo_set_viewed (AainaPhoto *photo, gboolean viewed);

void
aaina_photo_set_active (AainaPhoto *photo, gboolean active);

void
aaina_photo_set_visible (AainaPhoto *photo, gboolean visible);


G_END_DECLS

#endif
