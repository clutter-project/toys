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

#include <libaaina/aaina-library.h>
#include <libaaina/aaina-photo.h>

#ifndef _HAVE_AAINA_SLIDE_SHOW_H
#define _HAVE_AAINA_SLIDE_SHOW_H

G_BEGIN_DECLS

#define AAINA_TYPE_SLIDE_SHOW aaina_slide_show_get_type()

#define AAINA_SLIDE_SHOW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	AAINA_TYPE_SLIDE_SHOW, \
	AainaSlideShow))

#define AAINA_SLIDE_SHOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	AAINA_TYPE_SLIDE_SHOW, \
	AainaSlideShowClass))

#define AAINA_IS_SLIDE_SHOW(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	AAINA_TYPE_SLIDE_SHOW))

#define AAINA_IS_SLIDE_SHOW_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
	AAINA_TYPE_SLIDE_SHOW))

#define AAINA_SLIDE_SHOW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	AAINA_TYPE_SLIDE_SHOW, \
	AainaSlideShowClass))

typedef struct _AainaSlideShow AainaSlideShow;
typedef struct _AainaSlideShowClass AainaSlideShowClass;
typedef struct _AainaSlideShowPrivate AainaSlideShowPrivate;

struct _AainaSlideShow
{
	GObject         parent;
	
	/* private */
	AainaSlideShowPrivate   *priv;
};

struct _AainaSlideShowClass 
{
	/*< private >*/
	GObjectClass parent_class;

  void (*_aaina_slide_show_1) (void);
  void (*_aaina_slide_show_2) (void);
  void (*_aaina_slide_show_3) (void);
  void (*_aaina_slide_show_4) (void);
}; 

GType aaina_slide_show_get_type (void) G_GNUC_CONST;

AainaSlideShow*
aaina_slide_show_get_default (void);

void
aaina_slide_show_set_library (AainaSlideShow *slide_show, 
                              AainaLibrary *library);

G_END_DECLS

#endif
