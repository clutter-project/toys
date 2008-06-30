/*
 * CustomCursor.
 *
 * Copyright (C) 2008 OpenedHand, authored by Øyvind Kolås.
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

#ifndef __CUSTOM_CURSOR_H__
#define __CUSTOM_CURSOR_H__

#include <glib-object.h>
#include <clutter/clutter-actor.h>


G_BEGIN_DECLS

#define CUSTOM_TYPE_CURSOR              (custom_cursor_get_type ())
#define CUSTOM_CURSOR(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), CUSTOM_TYPE_CURSOR, CustomCursor))
#define CUSTOM_CURSORCLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), CUSTOM_TYPE_CURSOR, CustomCursorClass))
#define CLUTTER_IS_CAIRO(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CUSTOM_TYPE_CURSOR))
#define CLUTTER_IS_CAIRO_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), CUSTOM_TYPE_CURSOR))
#define CUSTOM_CURSORGET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), CUSTOM_TYPE_CURSOR, CustomCursorClass))

typedef struct _CustomCursor            CustomCursor;
typedef struct _CustomCursorClass       CustomCursorClass;
typedef struct _CustomCursorPrivate     CustomCursorPrivate;

typedef enum {
  CUSTOM_CURSOR_NORMAL,
  CUSTOM_CURSOR_PRESSED
}CustomCursorState;

struct _CustomCursor
{
  ClutterActor       parent;
  CustomCursorPrivate  *priv;
};

struct _CustomCursorClass 
{
  /*< private >*/
  ClutterActorClass parent_class;

  void (*_custom_cursor_1) (void);
  void (*_custom_cursor_2) (void);
  void (*_custom_cursor_3) (void);
  void (*_custom_cursor_4) (void);
}; 

GType  custom_cursor_get_type (void) G_GNUC_CONST;


/* update positional data for cursor, first call to this function enables custom cursors,
 * subsequent calls will update the position of the given device_id (needs to be called
 * during pointer grabs) */

ClutterActor * custom_cursor (gint x,
                              gint y,
                              gint device_id);

G_END_DECLS

#endif /* __CUSTOM_CURSORH__ */
