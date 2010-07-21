/*
 * Poinpoint: A small-ish presentation tool
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
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cairo.h>
#include <cairo-pdf.h>
#include <clutter/clutter.h>
#include <gio/gio.h>
#ifdef HAVE_PDF
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <pango/pango.h>
#include <pango/pangocairo.h>
#endif
#ifdef USE_CLUTTER_GST
#include <clutter-gst/clutter-gst.h>
#endif
#ifdef USE_DAX
#include <dax/dax.h>
#include "pp-super-aa.h"
#endif
#include <string.h>
#include <stdlib.h>

#define RESTDEPTH   -9000.0
#define RESTX        4600.0
#define STARTPOS    -3000.0

typedef struct _PinPointPoint PinPointPoint;
typedef struct _PinPointRenderer PinPointRenderer;

static ClutterColor black = {0x00,0x00,0x00,0xff};
static char *output_filename;

typedef enum
{
  PP_BG_COLOR,
  PP_BG_IMAGE,
  PP_BG_VIDEO,
  PP_BG_SVG
} PPBackgroundType;

#define PINPOINT_RENDERER(renderer) ((PinPointRenderer *) renderer)

struct _PinPointRenderer
{
  void      (*init)              (PinPointRenderer  *renderer,
                                  char              *pinpoint_file);
  void      (*run)                (PinPointRenderer *renderer);
  void      (*finalize)           (PinPointRenderer *renderer);
  gboolean  (*create_background)  (PinPointRenderer *renderer,
                                   PinPointPoint    *point,
                                   PPBackgroundType  type);
  void      (*create_text)        (PinPointRenderer *renderer,
                                   PinPointPoint    *point,
                                   const char       *text);
  void *    (*allocate_data)      (PinPointRenderer *renderer);
  void      (*free_data)          (PinPointRenderer *renderer,
                                   void             *datap);
};

struct _PinPointPoint
{
  ClutterGravity  position;
  const char     *font;
  const char     *stage_color;
  const char     *text_color;
  const char     *shading_color;
  float           shading_opacity;
  const gchar    *bg;
  const char     *command;

  void           *data;               /* the renderer can attach data here */
};

static PinPointPoint default_point = 
{
  CLUTTER_GRAVITY_CENTER, "Sans 60px", "black", "white",
  "black", 0.66, "black", NULL,
};

static GList *slides      = NULL; /* list of slide texts */
static GList *slidep      = NULL; /* current slide */

/*
 * Cross-renderers helpers
 */

static void
get_padding (float  stage_width,
             float  stage_height,
             float *padding)
{
  *padding = stage_width * 0.01;
}

static void
get_text_position_scale (PinPointPoint *point,
                         float          stage_width,
                         float          stage_height,
                         float          text_width,
                         float          text_height,
                         float         *text_x,
                         float         *text_y,
                         float         *text_scale)
{
  float w, h;
  float x, y;
  float sx = 1.0;
  float sy = 1.0;
  float padding;

  get_padding (stage_width, stage_height, &padding);

  w = text_width;
  h = text_height;

  sx = stage_width / w * 0.8;
  sy = stage_height / h * 0.8;

  if (sy < sx)
    sx = sy;
  if (sx > 1.0) /* avoid enlarging text */
    sx = 1.0;

  switch (point->position)
    {
  case CLUTTER_GRAVITY_EAST:
  case CLUTTER_GRAVITY_NORTH_EAST:
  case CLUTTER_GRAVITY_SOUTH_EAST:
    x = stage_width * 0.95 - w * sx;
    break;
  case CLUTTER_GRAVITY_WEST:
  case CLUTTER_GRAVITY_NORTH_WEST:
  case CLUTTER_GRAVITY_SOUTH_WEST:
    x = stage_width * 0.05;
    break;
  case CLUTTER_GRAVITY_CENTER:
  default:
    x = (stage_width - w * sx) / 2;
    break;
    }
  switch (point->position)
    {
  case CLUTTER_GRAVITY_SOUTH:
  case CLUTTER_GRAVITY_SOUTH_EAST:
  case CLUTTER_GRAVITY_SOUTH_WEST:
    y = stage_height * 0.95 - h * sx;
    break;
  case CLUTTER_GRAVITY_NORTH:
  case CLUTTER_GRAVITY_NORTH_EAST:
  case CLUTTER_GRAVITY_NORTH_WEST:
    y = stage_height * 0.05;
    break;
  default:
    y = (stage_height- h * sx) / 2;
    break;
    }

  *text_scale = sx;
  *text_x = x;
  *text_y = y;
}

static void
get_shading_position_size (float stage_width,
                           float stage_height,
                           float text_x,
                           float text_y,
                           float text_width,
                           float text_height,
                           float text_scale,
                           float *shading_x,
                           float *shading_y,
                           float *shading_width,
                           float *shading_height)
{
  float padding;

  get_padding (stage_width, stage_height, &padding);

  *shading_x = text_x - padding;
  *shading_y = text_y - padding;
  *shading_width = text_width * text_scale + padding * 2;
  *shading_height = text_height * text_scale + padding * 2;
}

/*
 * ClutterRenderer
 */

#define CLUTTER_RENDERER(renderer)  ((ClutterRenderer *) renderer)

typedef struct _ClutterRenderer
{
  PinPointRenderer renderer;
  ClutterActor *stage;
  ClutterActor *background;
  ClutterActor *shading;
  ClutterActor *foreground;

  char *path;               /* path of the file of the GFileMonitor callback */
  int rest_y;               /* where the text can rest */
} ClutterRenderer;

typedef struct
{
  ClutterActor   *background;
  ClutterActor   *text;
} ClutterPointData;

static void     leave_slide   (ClutterRenderer  *renderer);
static void     show_slide    (ClutterRenderer  *renderer);
static void     parse_slides  (PinPointRenderer *renderer,
                               const char       *slide_src);
static void     file_changed  (GFileMonitor     *monitor,
                               GFile            *file,
                               GFile            *other_file,
                               GFileMonitorEvent event_type,
                               ClutterRenderer  *renderer);
static void     stage_resized (ClutterActor     *actor,
                               GParamSpec       *pspec,
                               ClutterRenderer  *renderer);
static gboolean key_pressed   (ClutterActor     *actor,
                               ClutterEvent     *event,
                               ClutterRenderer  *renderer);

static void
clutter_renderer_init (PinPointRenderer   *pp_renderer,
                       char               *pinpoint_file)
{
  ClutterRenderer *renderer = CLUTTER_RENDERER (pp_renderer);
  GFileMonitor *monitor;
  ClutterActor *stage;

  renderer->rest_y = STARTPOS;

  renderer->stage = stage = clutter_stage_get_default ();

  renderer->background = clutter_group_new ();
  renderer->foreground = clutter_group_new ();
  renderer->shading = clutter_rectangle_new_with_color (&black);
  clutter_actor_set_opacity (renderer->shading, 0x77);

  clutter_container_add (CLUTTER_CONTAINER (stage),
                         renderer->background, renderer->shading,
                         renderer->foreground, NULL);

  clutter_actor_show (stage);
  clutter_stage_set_color (CLUTTER_STAGE (stage), &black);
  g_signal_connect (stage, "key-press-event",
                    G_CALLBACK (key_pressed), renderer);
  g_signal_connect (stage, "notify::width",
                    G_CALLBACK (stage_resized), renderer);
  g_signal_connect (stage, "notify::height",
                    G_CALLBACK (stage_resized), renderer);

  clutter_stage_set_user_resizable (CLUTTER_STAGE (stage), TRUE);

  renderer->path = pinpoint_file;
  if (renderer->path)
    {
      monitor = g_file_monitor (g_file_new_for_commandline_arg (pinpoint_file),
                                G_FILE_MONITOR_NONE, NULL, NULL);
      g_signal_connect (monitor, "changed", G_CALLBACK (file_changed),
                                            renderer);
    }
}

static void
clutter_renderer_run (PinPointRenderer *renderer)
{
  show_slide (CLUTTER_RENDERER (renderer));
  clutter_main ();
}

static void
clutter_renderer_finalize (PinPointRenderer *pp_renderer)
{
  ClutterRenderer *renderer = CLUTTER_RENDERER (pp_renderer);

  clutter_actor_destroy (renderer->stage);
}

static gboolean
clutter_renderer_create_background (PinPointRenderer *pp_renderer,
                                    PinPointPoint    *point,
                                    PPBackgroundType  type)
{
  ClutterRenderer *renderer = CLUTTER_RENDERER (pp_renderer);
  ClutterPointData *data = point->data;
  gboolean ret;

  switch (type)
    {
    case PP_BG_COLOR:
      {
        ClutterColor color;

        ret = clutter_color_from_string (&color, point->bg);
        if (ret)
          data->background = g_object_new (CLUTTER_TYPE_RECTANGLE,
                                           "color", &color,
                                           "width", 100.0,
                                           "height", 100.0,
                                           NULL);
      }
      break;
    case PP_BG_IMAGE:
      data->background = g_object_new (CLUTTER_TYPE_TEXTURE,
                                       "filename", point->bg,
                                       "load-data-async", TRUE,
                                       NULL);
      ret = TRUE;
      break;
    case PP_BG_VIDEO:
#ifdef USE_CLUTTER_GST
      data->background = clutter_gst_video_texture_new ();
      clutter_media_set_filename (CLUTTER_MEDIA (data->background), point->bg);
      /* should pre-roll the video and set the size */
      clutter_actor_set_size (data->background, 400, 300);
      ret = TRUE;
#endif
      break;
    case PP_BG_SVG:
#ifdef USE_DAX
      {
        ClutterActor *aa, *svg;
        GError *error = NULL;

        aa = pp_super_aa_new ();
        pp_super_aa_set_resolution (PP_SUPER_AA (aa), 2, 2);
        svg = dax_actor_new_from_file (point->bg, &error);
        mx_offscreen_set_pick_child (MX_OFFSCREEN (aa), TRUE);
        clutter_container_add_actor (CLUTTER_CONTAINER (aa),
                                     svg);

        data->background = aa;

        if (data->background == NULL)
          {
            g_warning ("Could not open SVG file %s: %s",
                       point->bg, error->message);
            g_clear_error (&error);
          }
        ret = data->background != NULL;
      }
#endif
      break;
    default:
      g_assert_not_reached();
    }

  if (data->background)
    {
      clutter_container_add_actor (CLUTTER_CONTAINER (renderer->background),
                                   data->background);
      clutter_actor_set_opacity (data->background, 0);
    }

    return ret;
}

static void
clutter_renderer_create_text (PinPointRenderer *pp_renderer,
                              PinPointPoint    *point,
                              const char       *text)
{
  ClutterRenderer *renderer = CLUTTER_RENDERER (pp_renderer);
  ClutterPointData *data = point->data;
  ClutterColor color;

  clutter_color_from_string (&color, point->text_color);
  data->text = g_object_new (CLUTTER_TYPE_TEXT,
                             "font-name", point->font,
                             "text", text,
                             "color", &color,
                             NULL);

  clutter_container_add_actor (CLUTTER_CONTAINER (renderer->foreground),
                               data->text);

  clutter_actor_set_position (data->text, RESTX, renderer->rest_y);
  g_object_set_data (G_OBJECT (data->text), "rest-y",
                     GINT_TO_POINTER (renderer->rest_y));
  renderer->rest_y += clutter_actor_get_height (data->text);
  clutter_actor_set_depth (data->text, RESTDEPTH);
}

static void *
clutter_renderer_allocate_data (PinPointRenderer *renderer)
{
  return g_slice_new0 (ClutterPointData);
}

static void
clutter_renderer_free_data (PinPointRenderer *renderer,
                            void             *datap)
{
  ClutterPointData *data = datap;

  if (data->background)
    clutter_actor_destroy (data->background);
  if (data->text)
    clutter_actor_destroy (data->text);
  g_slice_free (ClutterPointData, data);
}

static ClutterRenderer clutter_renderer =
{
  .renderer =
    {
      .init = clutter_renderer_init,
      .run = clutter_renderer_run,
      .finalize = clutter_renderer_finalize,
      .create_background = clutter_renderer_create_background,
      .create_text = clutter_renderer_create_text,
      .allocate_data = clutter_renderer_allocate_data,
      .free_data = clutter_renderer_free_data
    }
};

static gboolean
key_pressed (ClutterActor    *actor,
             ClutterEvent    *event,
             ClutterRenderer *renderer)
{
  if (event) /* There is no event for the first triggering */
  switch (clutter_event_get_key_symbol (event))
    {
      case CLUTTER_Left:
      case CLUTTER_Up:
      case CLUTTER_BackSpace:
        if (slidep && slidep->prev)
          {
            leave_slide (renderer);
            slidep = slidep->prev;
            show_slide (renderer);
          }
        break;
      case CLUTTER_Right:
      case CLUTTER_space:
        if (slidep && slidep->next)
          {
            leave_slide (renderer);
            slidep = slidep->next;
            show_slide (renderer);
          }
        break;
      case CLUTTER_Escape:
        clutter_main_quit ();
        break;
      case CLUTTER_F11:
        clutter_stage_set_fullscreen (CLUTTER_STAGE (renderer->stage),
            !clutter_stage_get_fullscreen (CLUTTER_STAGE (renderer->stage)));
        break;
    }
  return TRUE;
}


static void leave_slide (ClutterRenderer *renderer)
{
  PinPointPoint *point = slidep->data;
  ClutterPointData *data = point->data;
  float resty;

  resty = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (data->text),
                                              "rest-y"));
  clutter_actor_animate (data->text,
                         CLUTTER_LINEAR, 2000,
                         "depth", RESTDEPTH,
                         "scale-x", 1.0,
                         "scale-y", 1.0,
                         "x", RESTX,
                         "y", resty,
                         NULL);
  if (data->background)
    {
      clutter_actor_animate (data->background,
                             CLUTTER_LINEAR, 1000,
                             "opacity", 0x0,
                             NULL);
#ifdef USE_CLUTTER_GST
      if (CLUTTER_GST_IS_VIDEO_TEXTURE (data->background))
        {
          clutter_media_set_playing (CLUTTER_MEDIA (data->background), FALSE);
        }
#endif
#ifdef USE_DAX
      if (DAX_IS_ACTOR (data->background))
        {
          dax_actor_set_playing (DAX_ACTOR (data->background), FALSE);
        }
#endif
    }
}


static void
show_slide (ClutterRenderer *renderer)
{
  PinPointPoint *point;
  ClutterPointData *data;

  if (!slidep)
    return;
 
  point = slidep->data;
  data = point->data;

  if (point->stage_color)
    {
      ClutterColor color;

      clutter_color_from_string (&color, point->stage_color);
      clutter_stage_set_color (CLUTTER_STAGE (renderer->stage), &color);
    }

  if (data->background)
    {
      float w,h;
      float x,y;
      float s = 1.0;
      if (CLUTTER_IS_RECTANGLE (data->background))
        {
          clutter_actor_get_size (renderer->stage, &w, &h);
          clutter_actor_set_size (data->background, w, h);
        }
      else
        {
          clutter_actor_get_size (data->background, &w, &h);
        }
      s = clutter_actor_get_width (renderer->stage) / w;
      if (s > 1.0 || clutter_actor_get_height (renderer->stage) < s * h)
        s = clutter_actor_get_height (renderer->stage)/h;
      clutter_actor_set_scale (data->background, s, s);
      x = (clutter_actor_get_width (renderer->stage) - w * s) / 2;
      y = (clutter_actor_get_height (renderer->stage) - h * s) / 2;
      clutter_actor_set_position (data->background, x, y);
      clutter_actor_animate (data->background,
                             CLUTTER_LINEAR, 1000,
                             "opacity", 0xff,
                             NULL);

#ifdef USE_CLUTTER_GST
      if (CLUTTER_GST_IS_VIDEO_TEXTURE (data->background))
        {
          clutter_media_set_progress (CLUTTER_MEDIA (data->background), 0.0);
          clutter_media_set_playing (CLUTTER_MEDIA (data->background), TRUE);
        }
      else
#endif
#ifdef USE_DAX
      if (DAX_IS_ACTOR (data->background))
        {
          dax_actor_set_playing (DAX_ACTOR (data->background), TRUE);
        }
      else
#endif
        {
        }
    }

  {
    float text_x, text_y, text_width, text_height, text_scale;
    float shading_x, shading_y, shading_width, shading_height;

    clutter_actor_get_size (data->text, &text_width, &text_height);

    get_text_position_scale (point,
                             clutter_actor_get_width (renderer->stage),
                             clutter_actor_get_height (renderer->stage),
                             text_width, text_height,
                             &text_x, &text_y,
                             &text_scale);

    clutter_actor_animate (data->text,
                           CLUTTER_EASE_OUT_QUINT, 1000,
                           "depth", 0.0,
                           "scale-x", text_scale,
                           "scale-y", text_scale,
                           "x", text_x,
                           "y", text_y,
                           NULL);

    get_shading_position_size (clutter_actor_get_width (renderer->stage),
                               clutter_actor_get_height (renderer->stage),
                               text_x, text_y,
                               text_width, text_height,
                               text_scale,
                               &shading_x, &shading_y,
                               &shading_width, &shading_height);

    if (clutter_actor_get_width (data->text) > 1.0)
      {
        ClutterColor color;
        clutter_color_from_string (&color, point->shading_color);

        clutter_actor_animate (renderer->shading,
               CLUTTER_LINEAR, 500,
               "x", shading_x,
               "y", shading_y,
               "opacity", (int)(point->shading_opacity*255),
               "color", &color,
               "width", shading_width,
               "height", shading_height,
               NULL);
      }
    else /* no text, fade out shading */
      clutter_actor_animate (renderer->shading,
             CLUTTER_LINEAR, 500,
             "opacity", 0,
             "x", shading_x,
             "y", shading_y,
             "width", shading_width,
             "height", shading_height,
             NULL);
  }

  if (point->command)
    {
      g_print ("running: %s\n", point->command);
      system (point->command);
    }
}

static void
stage_resized (ClutterActor    *actor,
               GParamSpec      *pspec,
               ClutterRenderer *renderer)
{
  show_slide (renderer); /* redisplay the current slide */
}

static void
file_changed (GFileMonitor      *monitor,
              GFile             *file,
              GFile             *other_file,
              GFileMonitorEvent  event_type,
              ClutterRenderer   *renderer)
{
  char   *text = NULL;
  if (!g_file_get_contents (renderer->path, &text, NULL, NULL))
    g_error ("failed to load slides from %s\n", renderer->path);
  parse_slides (PINPOINT_RENDERER (renderer), text);
  g_free (text);
  show_slide(renderer);
}

/*
 * Cairo renderer
 */

#ifdef HAVE_PDF

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
  PPBackgroundType bg_type;
  char *text;
} CairoPointData;

static void
_destroy_surface (gpointer data)
{
  cairo_surface_t *surface;

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
  renderer->surface = cairo_pdf_surface_create (output_filename,
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
  CairoPointData *data = point->data;

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

  switch (data->bg_type)
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
        float w, h, s, r, x, y;

        surface = _cairo_get_surface (renderer, point->bg);
        if (surface == NULL)
          break;

        cairo_save (renderer->ctx);

        w = cairo_image_surface_get_width (surface);
        h = cairo_image_surface_get_height (surface);

        s = A4_LS_WIDTH / w;
        if (s > 1.0 || A4_LS_HEIGHT < s * h)
          s = A4_LS_HEIGHT / h;
        x = (A4_LS_WIDTH - w * s) / 2;
        y = (A4_LS_HEIGHT - h * s) / 2;

        cairo_translate (renderer->ctx, x, y);
        cairo_scale (renderer->ctx, s, s);
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
  pango_layout_set_text (layout, data->text, -1);

  pango_layout_get_extents (layout, NULL, &logical_rect);

  text_width = (logical_rect.x + logical_rect.width) / 1024;
  text_height = (logical_rect.y + logical_rect.height) / 1024;
  get_text_position_scale (point,
                           A4_LS_WIDTH,
                           A4_LS_HEIGHT,
                           text_width,
                           text_height,
                           &text_x,
                           &text_y,
                           &text_scale);

  get_shading_position_size (A4_LS_HEIGHT,
                             A4_LS_WIDTH,
                             text_x,
                             text_y,
                             text_width,
                             text_height,
                             text_scale,
                             &shading_x,
                             &shading_y,
                             &shading_width,
                             &shading_height);

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

  pango_font_description_free (desc);
  g_object_unref (layout);
}

static void
_cairo_render_page (CairoRenderer *renderer,
                    PinPointPoint *point)
{
  CairoPointData *data = point->data;

  _cairo_render_background (renderer, point);
  _cairo_render_text (renderer, point);
  cairo_show_page (renderer->ctx);
}

static void
cairo_renderer_run (PinPointRenderer *pp_renderer)
{
  CairoRenderer *renderer = CAIRO_RENDERER (pp_renderer);
  GList *cur;

  for (cur = slides; cur; cur = g_list_next (cur))
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
cairo_renderer_create_background (PinPointRenderer *pp_renderer,
                                  PinPointPoint    *point,
                                  PPBackgroundType  type)
{
  CairoPointData *data = point->data;
  gboolean ret = TRUE;

  data->bg_type = type;

  if (type == PP_BG_COLOR)
    {
      ClutterColor color;

      ret =  clutter_color_from_string (&color, point->bg);
    }

  return ret;
}

static void
cairo_renderer_create_text (PinPointRenderer *pp_renderer,
                            PinPointPoint    *point,
                            const char       *text)
{
  CairoPointData *data = point->data;

  data->text = g_strdup (text);
}

static void *
cairo_renderer_allocate_data (PinPointRenderer *renderer)
{
  return g_slice_new0 (CairoPointData);
}

static void
cairo_renderer_free_data (PinPointRenderer *renderer,
                          void             *datap)
{
  CairoPointData *data = datap;

  g_free (data->text);
}

static ClutterRenderer cairo_renderer =
{
  .renderer =
    {
      .init = cairo_renderer_init,
      .run = cairo_renderer_run,
      .finalize = cairo_renderer_finalize,
      .create_background = cairo_renderer_create_background,
      .create_text = cairo_renderer_create_text,
      .allocate_data = cairo_renderer_allocate_data,
      .free_data = cairo_renderer_free_data
    }
};

#endif /* HAVE_PDF */

/*
 * Parsing
 */

static void
parse_setting (PinPointPoint *point,
               const gchar   *setting)
{
/* pippin started it */
#define IF_PREFIX(prefix) } else if (g_str_has_prefix (setting, prefix)) {
#define IF_EQUAL(string) } else if (g_str_equal (setting, string)) {
#define char g_intern_string (strrchr (setting, '=') + 1)
#define float g_ascii_strtod (strrchr (setting, '=') + 1, NULL);
  if (0) {
    IF_PREFIX("stage-color=") point->stage_color = char;
    IF_PREFIX("font=") point->font = char;
    IF_PREFIX("text-color=") point->text_color = char;
    IF_PREFIX("shading-color=") point->shading_color = char;
    IF_PREFIX("shading-opacity=") point->shading_opacity = float;
    IF_PREFIX("command=") point->command = char;
    IF_EQUAL("center") point->position = CLUTTER_GRAVITY_CENTER;
    IF_EQUAL("top") point->position = CLUTTER_GRAVITY_NORTH;
    IF_EQUAL("bottom") point->position = CLUTTER_GRAVITY_SOUTH;
    IF_EQUAL("left") point->position = CLUTTER_GRAVITY_WEST;
    IF_EQUAL("right") point->position = CLUTTER_GRAVITY_EAST;
    IF_EQUAL("top-left") point->position = CLUTTER_GRAVITY_NORTH_WEST;
    IF_EQUAL("top-right") point->position = CLUTTER_GRAVITY_NORTH_EAST;
    IF_EQUAL("bottom-left") point->position = CLUTTER_GRAVITY_SOUTH_WEST;
    IF_EQUAL("bottom-right") point->position = CLUTTER_GRAVITY_SOUTH_EAST;
  } else {
    point->bg = g_intern_string (setting);
  }
#undef IF_PREFIX
#undef IF_EQUAL
#undef float
#undef char
}

static void
parse_config (PinPointPoint *point,
              const char    *config)
{
  GString *str = g_string_new ("");
  const char *p;
  for (p = config; *p; p++)
    {
      if (*p != '[')
        continue;

      p++;
      g_string_truncate (str, 0);
      while (*p && *p != ']' && *p != '\n')
        {
          g_string_append_c (str, *p);
          p++;
        }

      if (*p == ']')
        parse_setting (point, str->str);
    }
  g_string_free (str, TRUE);
}

static void
pin_point_free (PinPointRenderer *renderer,
                PinPointPoint    *point)
{
  if (renderer->free_data)
    renderer->free_data (renderer, point->data);
  g_free (point);
}

static PinPointPoint *
pin_point_new (PinPointRenderer *renderer)
{
  PinPointPoint *point;

  point = g_new0 (PinPointPoint, 1);
  *point = default_point;

  if (renderer->allocate_data)
      point->data = renderer->allocate_data (renderer);

  return point;
}

static void
parse_slides (PinPointRenderer *renderer,
              const char       *slide_src)
{
  const char *p;
  int      slideno = 0;
  gboolean startofline = TRUE;
  gboolean gotconfig = FALSE;
  GString *slide_str = g_string_new ("");
  GString *setting_str = g_string_new ("");
  PinPointPoint *point, *next_point;
  GList *s;

  /* store current slideno */
  if (slidep)
  for (;slidep->prev; slidep = slidep->prev)
    slideno++;

  for (s = slides; s; s = s->next)
    pin_point_free (renderer, s->data);

  g_list_free (slides);
  slides = NULL;
  point = pin_point_new (renderer);

  /* parse the slides, constructing lists of objects, adding all generated
   * actors to the stage
   */
  for (p = slide_src; *p; p++)
    {
      switch (*p)
      {
        case '\\': /* escape the next char */
          p++;
          startofline = FALSE;
          if (*p)
            g_string_append_c (slide_str, *p);
          break;
        case '\n':
          startofline = TRUE;
          g_string_append_c (slide_str, *p);
          break;
        case '-': /* slide seperator */
          if (startofline)
            {
              next_point = pin_point_new (renderer);

              g_string_assign (setting_str, "");
              while (*p && *p!='\n')  /* until newline */
                {
                  g_string_append_c (setting_str, *p);
                  p++;
                }
              parse_config (next_point, setting_str->str);

              if (!gotconfig)
                {
                  parse_config (&default_point, slide_str->str);
                  /* copy the default point except the per-slide allocated
                   * data (void *) */
                  memcpy (point, &default_point,
                          sizeof (PinPointPoint) - sizeof (void *));
                  gotconfig = TRUE;
                  g_string_assign (slide_str, "");
                  g_string_assign (setting_str, "");
                }
              else
                {
                  if (point->bg && point->bg[0])
                    {
                      if (g_str_has_suffix (point->bg, ".avi")
                       || g_str_has_suffix (point->bg, ".ogg")
                       || g_str_has_suffix (point->bg, ".ogv")
                       || g_str_has_suffix (point->bg, ".mpg")
                       || g_str_has_suffix (point->bg, ".mov")
                       || g_str_has_suffix (point->bg, ".mp4")
                       || g_str_has_suffix (point->bg, ".wmv")
                       || g_str_has_suffix (point->bg, ".AVI")
                       || g_str_has_suffix (point->bg, ".OGG")
                       || g_str_has_suffix (point->bg, ".OGV")
                       || g_str_has_suffix (point->bg, ".MPG")
                       || g_str_has_suffix (point->bg, ".MOV")
                       || g_str_has_suffix (point->bg, ".MP4")
                       || g_str_has_suffix (point->bg, ".WMV"))
                        {
                          renderer->create_background (renderer, point,
                                                       PP_BG_VIDEO);
                        }
                      else if (g_str_has_suffix (point->bg, ".svg"))
                        {
                          renderer->create_background (renderer, point,
                                                       PP_BG_SVG);
                        }
                      else if (renderer->create_background (renderer, point,
                                                           PP_BG_COLOR))
                        {
                        }
                      else
                        {
                          renderer->create_background(renderer, point,
                                                      PP_BG_IMAGE);
                        }
                    }

                  /* trim newlines and spaces from end */
                  while ( slide_str->str[strlen(slide_str->str)-1]=='\n'
                       || slide_str->str[strlen(slide_str->str)-1]==' ')
                          slide_str->str[strlen(slide_str->str)-1]='\0';

                  renderer->create_text (renderer, point, slide_str->str);

                  g_string_assign (slide_str, "");
                  g_string_assign (setting_str, "");

                  slides = g_list_append (slides, point);
                  point = next_point;
                }
            }
          else
            {
              startofline = FALSE;
              g_string_append_c (slide_str, *p);
            }
          break;
        default:
          startofline = FALSE;
          g_string_append_c (slide_str, *p);
          break;
      }
    }

  g_string_free (slide_str, TRUE);
  g_string_free (setting_str, TRUE);

  if (g_list_nth (slides, slideno))
    slidep = g_list_nth (slides, slideno);
  else
    slidep = slides;
}

static GOptionEntry entries[] =
{
    { "output", 'o', 0, G_OPTION_ARG_STRING, &output_filename,
      "Output slides to FILE (formats supported: pdf)", "FILE" },
    { NULL }
};

int
main (int    argc,
      char **argv)
{
  PinPointRenderer *renderer = PINPOINT_RENDERER (&clutter_renderer);
  GOptionContext *context;
  GError *error = NULL;
  char   *text = NULL;

  context = g_option_context_new ("- Presentations made easy");
  g_option_context_add_main_entries (context, entries, NULL);
  if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      g_print ("option parsing failed: %s\n", error->message);
      return EXIT_FAILURE;
    }

  if (!argv[1])
    {
      g_print ("usage: %s <slides>\n", argv[0]);
      text = g_strdup ("[red]\n--\nusage: pinpoint <slides.txt>\n--");
    }
  else
    {
      if (!g_file_get_contents (argv[1], &text, NULL, NULL))
        {
          g_print ("failed to load slides from %s\n", argv[1]);
          return -1;
        }
    }

#ifdef USE_CLUTTER_GST
  clutter_gst_init (&argc, &argv);
#else
  clutter_init (&argc, &argv);
#endif
#ifdef USE_DAX
  dax_init (&argc, &argv);
#endif

  /* select the cairo renderer if we are requested a pdf output */
  if (output_filename && g_str_has_suffix (output_filename, ".pdf"))
    {
#ifdef HAVE_PDF
      renderer = PINPOINT_RENDERER (&cairo_renderer);
      /* makes more sense to default to white for "stage" color in PDFs*/
      default_point.stage_color = "white";
#else
      g_warning ("Pinpoint was built without PDF support");
      return EXIT_FAILURE;
#endif
    }

  renderer->init (renderer, argv[1]);

  parse_slides (renderer, text);
  g_free (text);

  renderer->run (renderer);

  renderer->finalize (renderer);

  g_list_free (slides);

  return 0;
}
