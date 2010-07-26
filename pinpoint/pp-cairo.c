/*
 * Pinpoint: A small-ish presentation tool
 *
 * Copyright (C) 2010 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option0 any later version.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Written by: Øyvind Kolås <pippin@linux.intel.com>
 *             Damien Lespiau <damien.lespiau@intel.com>
 *             Emmanuele Bassi <ebassi@linux.intel.com>
 */

#include "pinpoint.h"

#ifdef HAVE_PDF
#include <cairo.h>
#include <cairo-pdf.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <pango/pango.h>
#include <pango/pangocairo.h>

#define CAIRO_RENDERER(renderer)  ((CairoRenderer *) renderer)

typedef struct _CairoRenderer
{
  PinPointRenderer renderer;
  GHashTable *surfaces;         /* keep cairo_surface_t around for source
                                   images as we wantt to only include one
                                   instance of the image when using it in
                                   several slides */
  cairo_surface_t  *surface;
  cairo_t *ctx;
} CairoRenderer;

typedef struct
{
} CairoPointData;

static void
_destroy_surface (gpointer data)
{
  cairo_surface_t *surface = data;

  cairo_surface_destroy (surface);
}

#define A4_LS_WIDTH  841.88976378
#define A4_LS_HEIGHT 595.275590551

static void
cairo_renderer_init (PinPointRenderer *pp_renderer,
                     char             *pinpoint_file)
{
  CairoRenderer *renderer = CAIRO_RENDERER (pp_renderer);

  /* A4, landscape */
  renderer->surface = cairo_pdf_surface_create (pp_output_filename,
                                                A4_LS_WIDTH, A4_LS_HEIGHT);

  renderer->ctx = cairo_create (renderer->surface);
  renderer->surfaces = g_hash_table_new_full (g_str_hash, g_str_equal,
                                              NULL, _destroy_surface);
}

/* This function is adapted from Gtk's gdk_cairo_set_source_pixbuf() you can
 * find in gdk/gdkcairo.c.
 * Copyright (C) Red Had, Inc.
 * LGPLv2+ */
static cairo_surface_t *
_cairo_new_surface_from_pixbuf (const GdkPixbuf *pixbuf)
{
  gint width = gdk_pixbuf_get_width (pixbuf);
  gint height = gdk_pixbuf_get_height (pixbuf);
  guchar *gdk_pixels = gdk_pixbuf_get_pixels (pixbuf);
  int gdk_rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  int n_channels = gdk_pixbuf_get_n_channels (pixbuf);
  int cairo_stride;
  guchar *cairo_pixels;
  cairo_format_t format;
  cairo_surface_t *surface;
  static const cairo_user_data_key_t key;
  int j;

  if (n_channels == 3)
    format = CAIRO_FORMAT_RGB24;
  else
    format = CAIRO_FORMAT_ARGB32;

  cairo_stride = cairo_format_stride_for_width (format, width);
  cairo_pixels = g_malloc (height * cairo_stride);
  surface = cairo_image_surface_create_for_data ((unsigned char *)cairo_pixels,
                                                 format,
                                                 width, height, cairo_stride);

  cairo_surface_set_user_data (surface, &key,
			       cairo_pixels, (cairo_destroy_func_t)g_free);

  for (j = height; j; j--)
    {
      guchar *p = gdk_pixels;
      guchar *q = cairo_pixels;

      if (n_channels == 3)
	{
	  guchar *end = p + 3 * width;

	  while (p < end)
	    {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
	      q[0] = p[2];
	      q[1] = p[1];
	      q[2] = p[0];
#else
	      q[1] = p[0];
	      q[2] = p[1];
	      q[3] = p[2];
#endif
	      p += 3;
	      q += 4;
	    }
	}
      else
	{
	  guchar *end = p + 4 * width;
	  guint t1,t2,t3;

#define MULT(d,c,a,t) G_STMT_START { t = c * a + 0x7f; d = ((t >> 8) + t) >> 8; } G_STMT_END

	  while (p < end)
	    {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
	      MULT(q[0], p[2], p[3], t1);
	      MULT(q[1], p[1], p[3], t2);
	      MULT(q[2], p[0], p[3], t3);
	      q[3] = p[3];
#else
	      q[0] = p[3];
	      MULT(q[1], p[0], p[3], t1);
	      MULT(q[2], p[1], p[3], t2);
	      MULT(q[3], p[2], p[3], t3);
#endif

	      p += 4;
	      q += 4;
	    }

#undef MULT
	}

      gdk_pixels += gdk_rowstride;
      cairo_pixels += cairo_stride;
    }

  return surface;
}

static gboolean
_cairo_read_file (const char     *file,
                  unsigned char **data,
                  unsigned int   *len)
{
    FILE *fp;

    fp = fopen (file, "rb");
    if (fp == NULL)
      return FALSE;

    fseek (fp, 0, SEEK_END);
    *len = ftell(fp);
    fseek (fp, 0, SEEK_SET);
    *data = g_malloc (*len);

    if (fread(*data, *len, 1, fp) != 1)
	return FALSE;

    fclose(fp);
    return TRUE;
}

static cairo_surface_t *
_cairo_get_surface (CairoRenderer *renderer,
                    const char    *file)
{
  cairo_surface_t *surface;
  GdkPixbuf *pixbuf;
  GError *error = NULL;

  surface = g_hash_table_lookup (renderer->surfaces, file);
  if (surface)
    return surface;

  pixbuf = gdk_pixbuf_new_from_file (file, &error);
  if (pixbuf == NULL)
    {
      if (error)
        {
          g_warning ("could not load file %s: %s", file, error->message);
          g_clear_error (&error);
        }
      return NULL;
    }

  surface = _cairo_new_surface_from_pixbuf (pixbuf);
  g_hash_table_insert (renderer->surfaces, (char *) file, surface);

  /* If we embed a JPEG, we can actually insert the coded data into the PDF in
   * a lossless fashion (no recompression of the JPEG) */
  if (g_str_has_suffix (file, ".jpg") || g_str_has_suffix (file, ".jpeg"))
      {
        unsigned char *data = NULL;
        guint len;

        _cairo_read_file (file, &data, &len);
        cairo_surface_set_mime_data (surface, CAIRO_MIME_TYPE_JPEG,
                                     data, len,
                                     g_free, data);
      }

  return surface;
}

static void
_cairo_render_background (CairoRenderer *renderer,
                          PinPointPoint *point)
{
  if (point->stage_color)
    {
      ClutterColor color;

      clutter_color_from_string (&color, point->stage_color);
      cairo_set_source_rgba (renderer->ctx,
                             color.red / 255.f,
                             color.green / 255.f,
                             color.blue / 255.f,
                             color.alpha / 255.f);
      cairo_paint (renderer->ctx);
    }

  switch (point->bg_type)
    {
    case PP_BG_COLOR:
      {
        ClutterColor color;

        clutter_color_from_string (&color, point->bg);
        cairo_set_source_rgba (renderer->ctx,
                               color.red / 255.f,
                               color.green / 255.f,
                               color.blue / 255.f,
                               color.alpha / 255.f);
        cairo_paint (renderer->ctx);
      }
      break;
    case PP_BG_IMAGE:
      {
        cairo_surface_t *surface;
        float bg_x, bg_y, bg_width, bg_height, bg_scale;

        surface = _cairo_get_surface (renderer, point->bg);
        if (surface == NULL)
          break;


        bg_width = cairo_image_surface_get_width (surface);
        bg_height = cairo_image_surface_get_height (surface);

        pp_get_background_position_scale (point,
                                          A4_LS_WIDTH, A4_LS_HEIGHT,
                                          bg_width, bg_height,
                                          &bg_x, &bg_y,
                                          &bg_scale);

        cairo_save (renderer->ctx);
        cairo_translate (renderer->ctx, bg_x, bg_y);
        cairo_scale (renderer->ctx, bg_scale, bg_scale);
        cairo_set_source_surface (renderer->ctx, surface, 0., 0.);
        cairo_paint (renderer->ctx);
        cairo_restore (renderer->ctx);
      }
      break;
    case PP_BG_VIDEO:
#ifdef USE_CLUTTER_GST
      /* TODO */
#endif
      break;
    case PP_BG_SVG:
#ifdef USE_DAX
      /* TODO */
#endif
      break;
    default:
      g_assert_not_reached();
    }
}

static void
_cairo_render_text (CairoRenderer *renderer,
                    PinPointPoint *point)
{
  CairoPointData *data = point->data;
  PangoLayout *layout;
  PangoFontDescription *desc;
  PangoRectangle logical_rect = { 0, };
  ClutterColor text_color, shading_color;

  float text_width, text_height, text_x, text_y, text_scale;
  float shading_x, shading_y, shading_width, shading_height;

  layout = pango_cairo_create_layout (renderer->ctx);
  desc = pango_font_description_from_string (point->font);
  pango_layout_set_font_description (layout, desc);
  if (point->use_markup)
    pango_layout_set_markup (layout, point->text, -1);
  else
    pango_layout_set_text (layout, point->text, -1);
  pango_layout_set_alignment (layout, point->text_align);

  pango_layout_get_extents (layout, NULL, &logical_rect);
  text_width = (logical_rect.x + logical_rect.width) / 1024;
  text_height = (logical_rect.y + logical_rect.height) / 1024;
  if (text_width < 1)
    goto out;

  pp_get_text_position_scale (point,
                              A4_LS_WIDTH, A4_LS_HEIGHT,
                              text_width, text_height,
                              &text_x, &text_y,
                              &text_scale);

  pp_get_shading_position_size (A4_LS_HEIGHT, A4_LS_WIDTH,
                                text_x, text_y,
                                text_width, text_height,
                                text_scale,
                                &shading_x, &shading_y,
                                &shading_width, &shading_height);

  clutter_color_from_string (&text_color, point->text_color);
  clutter_color_from_string (&shading_color, point->shading_color);

  cairo_set_source_rgba (renderer->ctx,
                         shading_color.red / 255.f,
                         shading_color.green / 255.f,
                         shading_color.blue / 255.f,
                         shading_color.alpha / 255.f * point->shading_opacity);
  cairo_rectangle (renderer->ctx,
                   shading_x, shading_y, shading_width, shading_height);
  cairo_fill (renderer->ctx);

  cairo_save (renderer->ctx);
  cairo_translate (renderer->ctx, text_x, text_y);
  cairo_scale (renderer->ctx, text_scale, text_scale);
  cairo_set_source_rgba (renderer->ctx,
                         text_color.red / 255.f,
                         text_color.green / 255.f,
                         text_color.blue / 255.f,
                         text_color.alpha / 255.f);
  pango_cairo_show_layout (renderer->ctx, layout);
  cairo_restore (renderer->ctx);

out:
  pango_font_description_free (desc);
  g_object_unref (layout);
}

static void
_cairo_render_page (CairoRenderer *renderer,
                    PinPointPoint *point)
{
  _cairo_render_background (renderer, point);
  _cairo_render_text (renderer, point);
  cairo_show_page (renderer->ctx);
}

static void
cairo_renderer_run (PinPointRenderer *pp_renderer)
{
  CairoRenderer *renderer = CAIRO_RENDERER (pp_renderer);
  GList *cur;

  for (cur = pp_slides; cur; cur = g_list_next (cur))
    _cairo_render_page (renderer, cur->data);
}

static void
cairo_renderer_finalize (PinPointRenderer *pp_renderer)
{
  CairoRenderer *renderer = CAIRO_RENDERER (pp_renderer);

  cairo_surface_destroy (renderer->surface);
  g_hash_table_unref (renderer->surfaces);
  cairo_destroy (renderer->ctx);
}

static gboolean
cairo_renderer_make_point (PinPointRenderer *pp_renderer,
                           PinPointPoint    *point)
{
  gboolean ret = TRUE;

  if (point->bg_type == PP_BG_COLOR)
    {
      ClutterColor color;

      ret =  clutter_color_from_string (&color, point->bg);
    }

  return ret;
}

static void *
cairo_renderer_allocate_data (PinPointRenderer *renderer)
{
  return NULL;
}

static void
cairo_renderer_free_data (PinPointRenderer *renderer,
                          void             *datap)
{
}

static CairoRenderer cairo_renderer_vtable =
{
  .renderer =
    {
      .init = cairo_renderer_init,
      .run = cairo_renderer_run,
      .finalize = cairo_renderer_finalize,
      .make_point = cairo_renderer_make_point,
      .allocate_data = cairo_renderer_allocate_data,
      .free_data = cairo_renderer_free_data
    }
};

PinPointRenderer *pp_cairo_renderer (void)
{
  return (void*)&cairo_renderer_vtable;
}

#endif /* HAVE_PDF */
