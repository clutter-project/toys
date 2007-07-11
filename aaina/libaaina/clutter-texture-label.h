/*
 * Clutter.
 *
 * An OpenGL based 'interactive canvas' library.
 *
 * Authored By Matthew Allum  <mallum@openedhand.com>
 *
 * Copyright (C) 2006 OpenedHand
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

#ifndef _HAVE_CLUTTER_TEXTURE_LABEL_H
#define _HAVE_CLUTTER_TEXTURE_LABEL_H

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define CLUTTER_TYPE_TEXTURE_LABEL clutter_texture_label_get_type()

#define CLUTTER_TEXTURE_LABEL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  CLUTTER_TYPE_TEXTURE_LABEL, ClutterTextureLabel))

#define CLUTTER_TEXTURE_LABEL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  CLUTTER_TYPE_TEXTURE_LABEL, ClutterTextureLabelClass))

#define CLUTTER_IS_TEXTURE_LABEL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  CLUTTER_TYPE_TEXTURE_LABEL))

#define CLUTTER_IS_TEXTURE_LABEL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  CLUTTER_TYPE_TEXTURE_LABEL))

#define CLUTTER_TEXTURE_LABEL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  CLUTTER_TYPE_TEXTURE_LABEL, ClutterTextureLabelClass))

typedef struct _ClutterTextureLabel ClutterTextureLabel;
typedef struct _ClutterTextureLabelClass ClutterTextureLabelClass;
typedef struct _ClutterTextureLabelPrivate ClutterTextureLabelPrivate;

struct _ClutterTextureLabel
{
  ClutterTexture         parent;

  /*< private >*/
  ClutterTextureLabelPrivate   *priv;
};

struct _ClutterTextureLabelClass 
{
  /*< private >*/
  ClutterTextureClass parent_class;

  void (*_clutter_texture_label_1) (void);
  void (*_clutter_texture_label_2) (void);
  void (*_clutter_texture_label_3) (void);
  void (*_clutter_texture_label_4) (void);
}; 

GType clutter_texture_label_get_type (void) G_GNUC_CONST;

ClutterActor *      clutter_texture_label_new              (void);
ClutterActor *      clutter_texture_label_new_with_text    (const gchar        *font_name,
							    const gchar        *text);

void                  clutter_texture_label_set_text         (ClutterTextureLabel       *label,
							      const gchar        *text);
G_CONST_RETURN gchar *clutter_texture_label_get_text         (ClutterTextureLabel       *label);
void                  clutter_texture_label_set_font_name    (ClutterTextureLabel       *label,
							      const gchar        *font_name);
G_CONST_RETURN gchar *clutter_texture_label_get_font_name    (ClutterTextureLabel       *label);
void                  clutter_texture_label_set_color        (ClutterTextureLabel       *label,
							      const ClutterColor *color);
void                  clutter_texture_label_get_color        (ClutterTextureLabel       *label,
							      ClutterColor       *color);
void                  clutter_texture_label_set_text_extents (ClutterTextureLabel       *label,
							      gint                width,
							      gint                height);
void                  clutter_texture_label_get_text_extents (ClutterTextureLabel       *label,
							      gint               *width,
							      gint               *height);

G_END_DECLS

#endif
