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

/**
 * SECTION:clutter-label
 * @short_description: Actor for displaying text
 *
 * #ClutterTextureLabel is a #ClutterTexture that displays text.
 */

#include "clutter-texture-label.h"

#include <pango/pangoft2.h>

#define DEFAULT_FONT_NAME	"Sans 10"

G_DEFINE_TYPE (ClutterTextureLabel, clutter_texture_label, CLUTTER_TYPE_TEXTURE);

enum
{
  PROP_0,
  PROP_FONT_NAME,
  PROP_TEXT,
  PROP_COLOR
};

#define CLUTTER_TEXTURE_LABEL_GET_PRIVATE(obj) \
(G_TYPE_INSTANCE_GET_PRIVATE ((obj), CLUTTER_TYPE_TEXTURE_LABEL, ClutterTextureLabelPrivate))

struct _ClutterTextureLabelPrivate
{
  PangoLayout          *layout;
  PangoContext         *context;
  PangoFontDescription *desc;
  
  ClutterColor          fgcol;
  
  gchar                *text;
  gchar                *font_name;
  
  gint                  extents_width;
  gint                  extents_height;

  gint                  detail;
  gint                  detail_direction;
  ClutterTimeline      *timeline;
};

static void
clutter_texture_label_make_pixbuf (ClutterTextureLabel *label)
{
  gint                 bx, by, w, h;
  FT_Bitmap            ft_bitmap;
  guint8 const         *ps;
  guint8               *pd;
  ClutterTextureLabelPrivate  *priv;
  ClutterTexture       *texture;
  GdkPixbuf            *pixbuf;
  
  priv  = label->priv;

  texture = CLUTTER_TEXTURE(label);

  if (priv->layout == NULL || priv->desc == NULL || priv->text == NULL)
    {
      //g_debug("*** FAIL: layout: %p , desc: %p, text %p ***",
	    //  priv->layout, priv->desc, priv->text);
      return;
    }

  pango_layout_set_font_description (priv->layout, priv->desc);
  pango_layout_set_text (priv->layout, priv->text, -1);

  if (priv->extents_width != 0)
    {
      pango_layout_set_width (priv->layout, PANGO_SCALE * priv->extents_width);
      pango_layout_set_wrap  (priv->layout, PANGO_WRAP_WORD);
    }

  pango_layout_get_pixel_size (priv->layout, 
			       &w, 
			       &h);

  if (w == 0 || h == 0)
    {
      //g_debug("aborting w:%i , h:%i", w, h);
      return;
    }

  ft_bitmap.rows         = h;
  ft_bitmap.width        = w;
  ft_bitmap.pitch        = (w+3) & ~3;
  ft_bitmap.buffer       = g_malloc0 (ft_bitmap.rows * ft_bitmap.pitch);
  ft_bitmap.num_grays    = 256;
  ft_bitmap.pixel_mode   = ft_pixel_mode_grays;
  ft_bitmap.palette_mode = 0;
  ft_bitmap.palette      = NULL;

  pango_ft2_render_layout (&ft_bitmap, priv->layout, 0, 0);

  pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, 
			   TRUE, 
			   8,
			   ft_bitmap.width, 
			   ft_bitmap.rows);

  for (by = 0; by < ft_bitmap.rows; by++) 
    {
      guint8  alpha;
      gint    i = 0;

      pd = gdk_pixbuf_get_pixels (pixbuf)
	+ by * gdk_pixbuf_get_rowstride (pixbuf);
      ps = ft_bitmap.buffer + by * ft_bitmap.pitch;

      alpha = *ps;

      for (bx = 0; bx < ft_bitmap.width; bx++) 
	{
	  *pd++ = priv->fgcol.red;
	  *pd++ = priv->fgcol.green;
	  *pd++ = priv->fgcol.blue;
	  //*pd++ = *ps++;
	  *pd++ = alpha;
	  ps++;
	  if (++i >= priv->detail)
	    {
	      i = 0; alpha = *ps;
	    }
	}
    }

  g_free (ft_bitmap.buffer);

  /*g_debug("Calling set_pixbuf with text : '%s' , pixb %ix%i"
	  " rendered with color %i,%i,%i,%i", 
	  priv->text, w, h, 
	  priv->fgcol.red,
	  priv->fgcol.green,
	  priv->fgcol.blue,
	  priv->fgcol.alpha);
*/
  clutter_texture_set_pixbuf (CLUTTER_TEXTURE (label), pixbuf, NULL);
  
  /* Texture has the ref now */
  g_object_unref (pixbuf); 
}

static void
timeline_cb (ClutterTimeline     *timeline, 
	     gint                 frame_num, 
	     ClutterTextureLabel *label)
{
  ClutterTextureLabelPrivate *priv;

  priv = label->priv;

  if (priv->detail_direction > 0)
    priv->detail /= 2;
  else
    priv->detail *= 2;

  clutter_texture_label_make_pixbuf (label);
}

static void
clutter_texture_label_show (ClutterActor *actor)
{
  ClutterTextureLabel        *label;
  ClutterTextureLabelPrivate *priv;

  label = CLUTTER_TEXTURE_LABEL(actor);
  priv = label->priv;

  priv->detail = 512;
  priv->detail_direction = 1;

  clutter_timeline_start (priv->timeline);

  CLUTTER_ACTOR_CLASS (clutter_texture_label_parent_class)->show (actor);
}

static void
clutter_texture_label_hide (ClutterActor *actor)
{
  ClutterTextureLabel        *label;
  ClutterTextureLabelPrivate *priv;

  label = CLUTTER_TEXTURE_LABEL(actor);
  priv = label->priv;

  priv->detail = 1;
  priv->detail_direction = -1;

  clutter_timeline_rewind (priv->timeline);
  clutter_timeline_start (priv->timeline);

  // CLUTTER_ACTOR_CLASS (clutter_texture_label_parent_class)->hide (actor);
}

static void
clutter_texture_label_set_property (GObject      *object, 
				    guint         prop_id,
				    const GValue *value, 
				    GParamSpec   *pspec)
{
  ClutterTextureLabel        *label;
  ClutterTextureLabelPrivate *priv;

  label = CLUTTER_TEXTURE_LABEL(object);
  priv = label->priv;

  switch (prop_id) 
    {
    case PROP_FONT_NAME:
      clutter_texture_label_set_font_name (label, g_value_get_string (value));
      break;
    case PROP_TEXT:
      clutter_texture_label_set_text (label, g_value_get_string (value));
      break;
    case PROP_COLOR:
      clutter_texture_label_set_color (label, g_value_get_boxed (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
clutter_texture_label_get_property (GObject    *object, 
			    guint       prop_id,
			    GValue     *value, 
			    GParamSpec *pspec)
{
  ClutterTextureLabel        *label;
  ClutterTextureLabelPrivate *priv;
  ClutterColor         color;

  label = CLUTTER_TEXTURE_LABEL(object);
  priv = label->priv;

  switch (prop_id) 
    {
    case PROP_FONT_NAME:
      g_value_set_string (value, priv->font_name);
      break;
    case PROP_TEXT:
      g_value_set_string (value, priv->text);
      break;
    case PROP_COLOR:
      clutter_texture_label_get_color (label, &color);
      g_value_set_boxed (value, &color);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    } 
}


static void 
clutter_texture_label_dispose (GObject *object)
{
  ClutterTextureLabel         *self = CLUTTER_TEXTURE_LABEL(object);
  ClutterTextureLabelPrivate  *priv;  

  priv = self->priv;
  
  if (priv->layout)
    {
      g_object_unref (priv->layout);
      priv->layout = NULL;
    }
  
  if (priv->desc)
    {
      pango_font_description_free (priv->desc);    
      priv->desc = NULL;
    }

  g_free (priv->text);
  priv->text = NULL;

  g_free (priv->font_name);
  priv->font_name = NULL;
      
  if (priv->context)
    {
      g_object_unref (priv->context);
      priv->context = NULL;
    }

  G_OBJECT_CLASS (clutter_texture_label_parent_class)->dispose (object);
}

static void 
clutter_texture_label_finalize (GObject *object)
{
  G_OBJECT_CLASS (clutter_texture_label_parent_class)->finalize (object);
}

static void
clutter_texture_label_class_init (ClutterTextureLabelClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass   *actor_class = CLUTTER_ACTOR_CLASS (klass);
  ClutterActorClass   *parent_class = CLUTTER_ACTOR_CLASS (clutter_texture_label_parent_class);

  actor_class->paint      = parent_class->paint;
  actor_class->realize    = parent_class->realize;
  actor_class->unrealize  = parent_class->unrealize;

  actor_class->show       = clutter_texture_label_show;
  actor_class->hide       = clutter_texture_label_hide;

  gobject_class->finalize   = clutter_texture_label_finalize;
  gobject_class->dispose    = clutter_texture_label_dispose;
  gobject_class->set_property = clutter_texture_label_set_property;
  gobject_class->get_property = clutter_texture_label_get_property;

  g_object_class_install_property
    (gobject_class, PROP_FONT_NAME,
     g_param_spec_string ("font-name",
			  "Font Name",
			  "Pango font description",
			  NULL,
			  G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_TEXT,
     g_param_spec_string ("text",
			  "Text",
			  "Text to render",
			  NULL,
			  G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_COLOR,
     g_param_spec_boxed ("color",
			 "Font Colour",
			 "Font Colour",
			 CLUTTER_TYPE_COLOR,
			 G_PARAM_READWRITE));

  g_type_class_add_private (gobject_class, sizeof (ClutterTextureLabelPrivate));
}

static void
clutter_texture_label_init (ClutterTextureLabel *self)
{
  ClutterTextureLabelPrivate *priv;
  PangoFT2FontMap     *font_map;

  self->priv = priv = CLUTTER_TEXTURE_LABEL_GET_PRIVATE (self);

  priv->fgcol.red   = 0;
  priv->fgcol.green = 0;
  priv->fgcol.blue  = 0;
  priv->fgcol.alpha = 255;

  priv->text = NULL;
  priv->font_name = g_strdup (DEFAULT_FONT_NAME);
  priv->desc = pango_font_description_from_string (priv->font_name);
  
  font_map = PANGO_FT2_FONT_MAP (pango_ft2_font_map_new ());
  pango_ft2_font_map_set_resolution (font_map, 96.0, 96.0);
  priv->context = pango_ft2_font_map_create_context (font_map);

  priv->layout  = pango_layout_new (priv->context);

  /* See http://bugzilla.gnome.org/show_bug.cgi?id=143542  ?? 
  pango_ft2_font_map_substitute_changed (font_map);
  g_object_unref (font_map);
  */

  priv->timeline = clutter_timeline_new (8, 20);

  g_signal_connect (priv->timeline, 
		    "new-frame", 
		    G_CALLBACK (timeline_cb), 
		    self);
#if 0
  g_signal_connect (self,
		    "show",
		    G_CALLBACK (show_handler),
		    NULL);

  g_signal_connect (self,
		    "hide",
		    G_CALLBACK (hide_handler),
		    NULL);
#endif

  priv->detail = 512;
}

/**
 * clutter_texture_label_new_with_text:
 * @font_name: the name (and size) of the font to be used
 * @text: the text to be displayed
 *
 * Creates a new #ClutterTextureLabel displaying @text using @font_name.
 *
 * Return value: a #ClutterTextureLabel
 */
ClutterActor*
clutter_texture_label_new_with_text (const gchar *font_name,
				     const gchar *text)
{
  ClutterActor *label;

  label = clutter_texture_label_new ();
  clutter_texture_label_set_font_name (CLUTTER_TEXTURE_LABEL(label), font_name);
  clutter_texture_label_set_text (CLUTTER_TEXTURE_LABEL(label), text);

  /*  FIXME: Why does calling like;
   *  return g_object_new (CLUTTER_TYPE_TEXTURE_LABEL, 
   *		       "font-name", font_name,
   *		       "text", text,
   *		       NULL);
   *  mean text does not get rendered without color being set
   *  ( seems to need extra clutter_texture_label_make_pixbuf() call )
  */

  return label;
}

/**
 * clutter_texture_label_new:
 *
 * Creates a new, empty #ClutterTextureLabel.
 *
 * Returns: the newly created #ClutterTextureLabel
 */
ClutterActor *
clutter_texture_label_new (void)
{
  return g_object_new (CLUTTER_TYPE_TEXTURE_LABEL, NULL);
}

/**
 * clutter_texture_label_get_text:
 * @label: a #ClutterTextureLabel
 *
 * Retrieves the text displayed by @label
 *
 * Return value: the text of the label.  The returned string is
 *   owned by #ClutterTextureLabel and should not be modified or freed.
 */
G_CONST_RETURN gchar *
clutter_texture_label_get_text (ClutterTextureLabel *label)
{
  g_return_val_if_fail (CLUTTER_IS_TEXTURE_LABEL (label), NULL);

  return label->priv->text;
}

/**
 * clutter_texture_label_set_text:
 * @label: a #ClutterTextureLabel
 * @text: the text to be displayed
 *
 * Sets @text as the text to be displayed by @label.
 */
void
clutter_texture_label_set_text (ClutterTextureLabel *label,
		        const gchar  *text)
{
  ClutterTextureLabelPrivate  *priv;  

  g_return_if_fail (CLUTTER_IS_TEXTURE_LABEL (label));

  priv = label->priv;
  
  g_free (priv->text);
  priv->text = g_strdup (text);

  clutter_texture_label_make_pixbuf (label);

  if (CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR(label)))
    clutter_actor_queue_redraw (CLUTTER_ACTOR(label));

  g_object_notify (G_OBJECT (label), "text");
}

/**
 * clutter_texture_label_get_font_name:
 * @label: a #ClutterTextureLabel
 *
 * Retrieves the font used by @label.
 *
 * Return value: a string containing the font name, in a format
 *   understandable by pango_font_description_from_string().  The
 *   string is owned by #ClutterTextureLabel and should not be modified
 *   or freed.
 */
G_CONST_RETURN gchar *
clutter_texture_label_get_font_name (ClutterTextureLabel *label)
{
  g_return_val_if_fail (CLUTTER_IS_TEXTURE_LABEL (label), NULL);
  
  return label->priv->font_name;
}

/**
 * clutter_texture_label_set_font_name:
 * @label: a #ClutterTextureLabel
 * @font_name: a font name and size, or %NULL for the default font
 *
 * Sets @font_name as the font used by @label.
 *
 * @font_name must be a string containing the font name and its
 * size, similarly to what you would feed to the
 * pango_font_description_from_string() function.
 */
void
clutter_texture_label_set_font_name (ClutterTextureLabel *label,
				     const gchar  *font_name)
{
  ClutterTextureLabelPrivate  *priv;  

  g_return_if_fail (CLUTTER_IS_TEXTURE_LABEL (label));
  
  if (!font_name || font_name[0] == '\0')
    font_name = DEFAULT_FONT_NAME;

  priv = label->priv;

  if (priv->desc)
    pango_font_description_free (priv->desc);

  g_free (priv->font_name);
  priv->font_name = g_strdup (font_name);

  priv->desc = pango_font_description_from_string (priv->font_name);
  if (!priv->desc)
    {
      g_warning ("Attempting to create a PangoFontDescription for "
		 "font name `%s', but failed.",
		 priv->font_name);
      return;
    }

  if (label->priv->text && label->priv->text[0] != '\0')
    {
      clutter_texture_label_make_pixbuf (label);

      if (CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR(label)))
	clutter_actor_queue_redraw (CLUTTER_ACTOR(label));
    }
  
  g_object_notify (G_OBJECT (label), "font-name");
}

/**
 * clutter_texture_label_set_text_extents:
 * @label: a #ClutterTextureLabel
 * @width: the width of the text
 * @height: the height of the text
 *
 * Sets the maximum extents of the label's text.
 */
void
clutter_texture_label_set_text_extents (ClutterTextureLabel *label, 
				gint          width,
				gint          height)
{
  /* FIXME: height extents is broken.... 
  */

  label->priv->extents_width = width;
  label->priv->extents_height = height;

  clutter_texture_label_make_pixbuf (label);

  if (CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR(label)))
    clutter_actor_queue_redraw (CLUTTER_ACTOR(label));
}

/**
 * clutter_texture_label_get_text_extents:
 * @label: a #ClutterTextureLabel
 * @width: return location for the width of the extents or %NULL
 * @height: return location for the height of the extents or %NULL
 *
 * Gets the extents of the label.
 */
void
clutter_texture_label_get_text_extents (ClutterTextureLabel *label,
					gint         *width,
					gint         *height)
{
  g_return_if_fail (CLUTTER_IS_TEXTURE_LABEL (label));

  if (width)
    *width = label->priv->extents_width;

  if (height)
    *height = label->priv->extents_height;
}

/**
 * clutter_texture_label_set_color:
 * @label: a #ClutterTextureLabel
 * @color: a #ClutterColor
 *
 * Sets the color of @label.
 */
void
clutter_texture_label_set_color (ClutterTextureLabel       *label,
				 const ClutterColor        *color)
{
  ClutterActor *actor;
  ClutterTextureLabelPrivate *priv;

  g_return_if_fail (CLUTTER_IS_TEXTURE_LABEL (label));
  g_return_if_fail (color != NULL);

  priv = label->priv;
  priv->fgcol.red = color->red;
  priv->fgcol.green = color->green;
  priv->fgcol.blue = color->blue;
  priv->fgcol.alpha = color->alpha;

  clutter_texture_label_make_pixbuf (label);

  actor = CLUTTER_ACTOR (label);
  clutter_actor_set_opacity (actor, priv->fgcol.alpha);

  if (CLUTTER_ACTOR_IS_VISIBLE (actor))
    clutter_actor_queue_redraw (actor);

  g_object_notify (G_OBJECT (label), "color");

}

/**
 * clutter_texture_label_get_color:
 * @label: a #ClutterTextureLabel
 * @color: return location for a #ClutterColor
 *
 * Retrieves the color of @label.
 */
void
clutter_texture_label_get_color (ClutterTextureLabel *label,
				 ClutterColor        *color)
{
  ClutterTextureLabelPrivate *priv;

  g_return_if_fail (CLUTTER_IS_TEXTURE_LABEL (label));
  g_return_if_fail (color != NULL);

  priv = label->priv;

  color->red = priv->fgcol.red;
  color->green = priv->fgcol.green;
  color->blue = priv->fgcol.blue;
  color->alpha = priv->fgcol.alpha;
}
