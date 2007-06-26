/*
 * Clutter.
 *
 * An OpenGL based 'interactive canvas' library.
 *
 * Authored By Tomas Frydrych tf@openedhand.com>
 *
 * Copyright (C) 2007 OpenedHand
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _HAVE_CLUTTER_DOMINATRIX_H
#define _HAVE_CLUTTER_DOMINATRIX_H

/* clutter-dominatrix.h */

#include <glib-object.h>
#include <clutter/clutter-fixed.h>
#include <clutter/clutter-actor.h>

G_BEGIN_DECLS

#define CLUTTER_TYPE_DOMINATRIX clutter_dominatrix_get_type()

#define CLUTTER_DOMINATRIX(obj) \
 (G_TYPE_CHECK_INSTANCE_CAST ((obj), CLUTTER_TYPE_DOMINATRIX, ClutterDominatrix))
#define CLUTTER_DOMINATRIX_CLASS(klass) \
 (G_TYPE_CHECK_CLASS_CAST ((klass), CLUTTER_TYPE_DOMINATRIX, ClutterDominatrixClass))
#define CLUTTER_IS_DOMINATRIX(obj) \
 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CLUTTER_TYPE_DOMINATRIX))
#define CLUTTER_IS_DOMINATRIX_CLASS(klass) \
 (G_TYPE_CHECK_CLASS_TYPE ((klass), CLUTTER_TYPE_DOMINATRIX))
#define CLUTTER_DOMINATRIX_GET_CLASS(obj) \
 (G_TYPE_INSTANCE_GET_CLASS ((obj), CLUTTER_TYPE_DOMINATRIX, ClutterDominatrixClass))

typedef struct _ClutterDominatrix         ClutterDominatrix;
typedef struct _ClutterDominatrixClass    ClutterDominatrixClass;
typedef struct _ClutterDominatrixPrivate  ClutterDominatrixPrivate;

struct _ClutterDominatrix
{
  /*< public >*/
  GObject parent_instance;

  /*< private >*/
  ClutterDominatrixPrivate *priv;
};

struct _ClutterDominatrixClass
{
  GObjectClass parent_class;
};

GType                 clutter_dominatrix_get_type         (void) G_GNUC_CONST;

ClutterDominatrix *   clutter_dominatrix_new              (ClutterActor *actor);

G_END_DECLS

#endif /* _HAVE_CLUTTER_DOMINATRIX_H */
