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


#include <glib.h>
#include <clutter/clutter.h>
#include <libastro-desktop/astro-application.h>

#include "astro-contacts-window.h"

#ifndef _HAVE_ASTRO_CONTACT_DETAILS_H
#define _HAVE_ASTRO_CONTACT_DETAILS_H

G_BEGIN_DECLS

#define ASTRO_TYPE_CONTACT_DETAILS astro_contact_details_get_type()

#define ASTRO_CONTACT_DETAILS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  ASTRO_TYPE_CONTACT_DETAILS, \
  AstroContactDetails))

#define ASTRO_CONTACT_DETAILS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
  ASTRO_TYPE_CONTACT_DETAILS, \
  AstroContactDetailsClass))

#define ASTRO_IS_CONTACT_DETAILS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  ASTRO_TYPE_CONTACT_DETAILS))

#define ASTRO_IS_CONTACT_DETAILS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  ASTRO_TYPE_CONTACT_DETAILS))

#define ASTRO_CONTACT_DETAILS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
  ASTRO_TYPE_CONTACT_DETAILS, \
  AstroContactDetailsClass))

typedef struct _AstroContactDetails AstroContactDetails;
typedef struct _AstroContactDetailsClass AstroContactDetailsClass;
typedef struct _AstroContactDetailsPrivate AstroContactDetailsPrivate;

struct _AstroContactDetails
{
  ClutterGroup       parent;
	
  /*< private >*/
  AstroContactDetailsPrivate   *priv;
};

struct _AstroContactDetailsClass 
{
  /*< private >*/
  ClutterGroupClass parent_class;
}; 

GType astro_contact_details_get_type (void) G_GNUC_CONST;

ClutterActor *  astro_contact_details_new        (void);

void            astro_contact_details_set_active (AstroContactDetails *details,
                                                  AstroContact        *contact);
  
G_END_DECLS

#endif
