/*
Copyright (c) 2010, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

• Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
• Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
• Neither the name of Intel Corporation nor the names of its contributors may
  be used to endorse or promote products derived from this software without
  specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <clutter/clutter.h>
#include <gio/gio.h>
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
  void      (*init)              (PinPointRenderer    *renderer,
                                  int                 *argc,
                                  char             ***argv);
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
                       int                *argc,
                       char             ***argv)
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

  renderer->path = *argv[1];
  if (renderer->path)
    {
      monitor = g_file_monitor (g_file_new_for_commandline_arg (*argv[1]),
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
    float w, h;
    float x, y;
    float sx = 1.0;
    float sy = 1.0;
    float padding = clutter_actor_get_width (renderer->stage) * 0.01;

    clutter_actor_get_size (data->text, &w, &h);

    sx = clutter_actor_get_width (renderer->stage) / w * 0.8;
    sy = clutter_actor_get_height (renderer->stage) / h * 0.8;

    if (sy < sx)
      sx = sy;
    if (sx > 1.0) /* avoid enlarging text */
      sx = 1.0;

    switch (point->position)
      {
        case CLUTTER_GRAVITY_EAST:
        case CLUTTER_GRAVITY_NORTH_EAST:
        case CLUTTER_GRAVITY_SOUTH_EAST:
          x = clutter_actor_get_width (renderer->stage) * 0.95
              - clutter_actor_get_width (data->text) * sx;
          break;
        case CLUTTER_GRAVITY_WEST:
        case CLUTTER_GRAVITY_NORTH_WEST:
        case CLUTTER_GRAVITY_SOUTH_WEST:
          x = clutter_actor_get_width (renderer->stage) * 0.05;
          break;
        case CLUTTER_GRAVITY_CENTER:
        default:
          x = (clutter_actor_get_width (renderer->stage) - w * sx) / 2;
          break;
      }
    switch (point->position)
      {
        case CLUTTER_GRAVITY_SOUTH:
        case CLUTTER_GRAVITY_SOUTH_EAST:
        case CLUTTER_GRAVITY_SOUTH_WEST:
          y = clutter_actor_get_height (renderer->stage) * 0.95
              - clutter_actor_get_height (data->text) * sx;
          break;
        case CLUTTER_GRAVITY_NORTH:
        case CLUTTER_GRAVITY_NORTH_EAST:
        case CLUTTER_GRAVITY_NORTH_WEST:
          y = clutter_actor_get_height (renderer->stage) * 0.05;
          break;
        default:
          y = (clutter_actor_get_height (renderer->stage)- h * sx) / 2;
          break;
      }

    clutter_actor_animate (data->text,
                           CLUTTER_EASE_OUT_QUINT, 1000,
                           "depth", 0.0,
                           "scale-x", sx,
                           "scale-y", sx,
                           "x", x,
                           "y", y,
                           NULL);

    if (clutter_actor_get_width (data->text) > 1.0)
      {
        ClutterColor color;
        clutter_color_from_string (&color, point->shading_color);
        clutter_actor_animate (renderer->shading,
               CLUTTER_LINEAR, 500,
               "x", x - padding,
               "y", y - padding,
               "opacity", (int)(point->shading_opacity*255),
               "color", &color,
               "width", clutter_actor_get_width (data->text) * sx + padding*2,
               "height", clutter_actor_get_height (data->text) * sx + padding*2,
               NULL);
      }
    else /* no text, fade out shading */
      clutter_actor_animate (renderer->shading,
             CLUTTER_LINEAR, 500,
             "opacity", 0,
             "x", x - padding,
             "y", y - padding,
             "width", clutter_actor_get_width (data->text) * sx + padding*2,
             "height", clutter_actor_get_height (data->text) * sx + padding*2,
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
}

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
parse_config (const char *config)
{
  GString *str = g_string_new ("");
  const char *p;
  for (p = config; *p; p++)
    {
      switch (*p)
        {
          case '\n':
            parse_setting (&default_point, str->str);
            g_string_truncate (str, 0);
            break;
          default:
            g_string_append_c (str, *p);
            break;
        }
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
  PinPointPoint *point;
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
        case '[': 
          p++;
          g_string_assign (setting_str, "");
          for (;*p && *p!='\n' && *p!=']'; p++) /* until ] or endof line/buf */
              g_string_append_c (setting_str, *p);

          parse_setting (point, setting_str->str);
          break;
        case '-': /* slide seperator */
          if (startofline)
            {
              while (*p && *p!='\n')  /* until newline */
                p++;

              if (!gotconfig)
                {
                  parse_config (slide_str->str);
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
                  point = pin_point_new (renderer);
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

int
main (int    argc,
      char **argv)
{
  PinPointRenderer *renderer = PINPOINT_RENDERER (&clutter_renderer);
  char   *text = NULL;

  if (!argv[1])
    {
      g_print ("usage: %s <slides>\n", argv[0]);
      text = g_strdup ("red\n--\nusage: pinpoint <slides.txt>\n--");
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

  renderer->init (renderer, &argc, &argv);

  parse_slides (renderer, text);
  g_free (text);

  renderer->run (renderer);

  renderer->finalize (renderer);

  g_list_free (slides);

  return 0;
}
