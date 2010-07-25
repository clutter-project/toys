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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdlib.h>

#include "pinpoint.h"

GList *pp_slides      = NULL; /* list of slide texts */
GList *pp_slidep      = NULL; /* current slide */

typedef struct
{
  const char *name;
  int value;
} EnumDescription;

static EnumDescription PPBackgroundScale_desc[] =
{
  { "fit",  PP_BG_FIT },
  { "fill", PP_BG_FILL},
  { NULL,   0}
};

static EnumDescription PPTextAlign_desc[] =
{
  { "left",   PP_TEXT_LEFT },
  { "center", PP_TEXT_CENTER },
  { "right",  PP_TEXT_RIGHT },
  { NULL,     0 }
};

#define PINPOINT_RENDERER(renderer) ((PinPointRenderer *) renderer)

static PinPointPoint default_point = {
  .stage_color = "black",

  .bg = "NULL",
  .bg_type = PP_BG_NONE,
  .bg_scale = PP_BG_FIT,

  .text = NULL,
  .position = CLUTTER_GRAVITY_CENTER,
  .font = "Sans 60px",
  .text_color = "white",
  .text_align = PP_TEXT_LEFT,
  .use_markup = TRUE,

  .shading_color = "black",
  .shading_opacity = 0.66,
  .transition = NULL,

  .command = NULL,
  .data = NULL,
};

char      *pp_output_filename;
gboolean   pp_fullscreen = FALSE;
gboolean   pp_maximized = FALSE;

static GOptionEntry entries[] =
{
    { "maximized", 'm', 0, G_OPTION_ARG_NONE, &pp_maximized,
    "Maximize witohout decorations, instead\n"
"                                         of fullscreening, this is useful\n"
"                                         to enable windowmanagement when running\n"
"                                         [command=] spawned apps.", NULL},
    { "fullscreen", 'f', 0, G_OPTION_ARG_NONE, &pp_fullscreen,
    "Start in fullscreen mode", NULL},
    { "output", 'o', 0, G_OPTION_ARG_STRING, &pp_output_filename,
      "Output presentation to FILE\n"
"                                         (formats supported: pdf)", "FILE" },
    { NULL }
};

PinPointRenderer *pp_clutter_renderer (void);
#ifdef HAVE_PDF
PinPointRenderer *pp_cairo_renderer   (void);
#endif

int
main (int    argc,
      char **argv)
{
  PinPointRenderer *renderer;
  GOptionContext *context;
  GError *error = NULL;
  char   *text = NULL;

  renderer = pp_clutter_renderer ();

  context = g_option_context_new ("- Presentations made easy");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_add_group (context, clutter_get_option_group_without_init ());
  g_option_context_add_group (context, cogl_get_option_group ());
  if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      g_print ("option parsing failed: %s\n", error->message);
      return EXIT_FAILURE;
    }

  if (!argv[1])
    {
      g_print ("usage: %s [options] <presentation>\n", argv[0]);
      text = g_strdup ("[transition=sheet][red]\n--\nusage: pinpoint [options] <presentation.txt>\n");
    }
  else
    {
      if (!g_file_get_contents (argv[1], &text, NULL, NULL))
        {
          g_print ("failed to load presentation from %s\n", argv[1]);
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
  if (pp_output_filename && g_str_has_suffix (pp_output_filename, ".pdf"))
    {
#ifdef HAVE_PDF
      renderer = pp_cairo_renderer ();
      /* makes more sense to default to white for "stage" color in PDFs*/
      default_point.stage_color = "white";
#else
      g_warning ("Pinpoint was built without PDF support");
      return EXIT_FAILURE;
#endif
    }

  renderer->init (renderer, argv[1]);

  pp_parse_slides (renderer, text);
  g_free (text);

  renderer->run (renderer);
  renderer->finalize (renderer);
  if (renderer->source)
    g_free (renderer->source);

  g_list_free (pp_slides);

  return 0;
}

/*********************/


/*
 * Cross-renderers helpers
 */

void
pp_get_padding (float  stage_width,
                float  stage_height,
                float *padding)
{
  *padding = stage_width * 0.01;
}

void
pp_get_background_position_scale (PinPointPoint *point,
                                  float          stage_width,
                                  float          stage_height,
                                  float          bg_width,
                                  float          bg_height,
                                  float         *bg_x,
                                  float         *bg_y,
                                  float         *bg_scale)
{
  float w_scale = stage_width / bg_width;
  float h_scale = stage_height / bg_height;

  switch (point->bg_scale)
    {
    case PP_BG_FILL:
      *bg_scale = (w_scale > h_scale) ? w_scale : h_scale;
      break;
    case PP_BG_FIT:
      *bg_scale = (w_scale < h_scale) ? w_scale : h_scale;
      break;
    }
  *bg_x = (stage_width - bg_width * *bg_scale) / 2;
  *bg_y = (stage_height - bg_height * *bg_scale) / 2;
}

void
pp_get_text_position_scale (PinPointPoint *point,
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

  pp_get_padding (stage_width, stage_height, &padding);

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

void
pp_get_shading_position_size (float stage_width,
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

  pp_get_padding (stage_width, stage_height, &padding);

  *shading_x = text_x - padding;
  *shading_y = text_y - padding;
  *shading_width = text_width * text_scale + padding * 2;
  *shading_height = text_height * text_scale + padding * 2;
}

void     pp_parse_slides  (PinPointRenderer *renderer,
                           const char       *slide_src);
/*
 * Parsing
 */

static void
parse_setting (PinPointPoint *point,
               const gchar   *setting)
{
/* C Preprocessor macros implemeting a mini language for interpreting
 * pinpoint key=value pairs
 */

#define START_PARSER if (0) {
#define DEFAULT      } else {
#define END_PARSER   }
#define IF_PREFIX(prefix) } else if (g_str_has_prefix (setting, prefix)) {
#define IF_EQUAL(string) } else if (g_str_equal (setting, string)) {
#define char g_intern_string (strrchr (setting, '=') + 1)
#define float g_ascii_strtod (strrchr (setting, '=') + 1, NULL);
#define enum(r,t,s) \
  do { \
      int _i; \
      EnumDescription *_d = t##_desc; \
      r = _d[0].value; \
      for (_i = 0; _d[_i].name; _i++) \
        if (g_strcmp0 (_d[_i].name, s) == 0) \
          r = _d[_i].value; \
  } while (0)

  START_PARSER
  IF_PREFIX("stage-color=") point->stage_color = char;
  IF_PREFIX("font=")        point->font = char;
  IF_PREFIX("text-color=")  point->text_color = char;
  IF_PREFIX("text-align=")  enum(point->text_align, PPTextAlign, char);
  IF_PREFIX("shading-color=") point->shading_color = char;
  IF_PREFIX("shading-opacity=") point->shading_opacity = float;
  IF_PREFIX("command=")    point->command = char;
  IF_PREFIX("transition=") point->transition = char;
  IF_EQUAL("fill")         point->bg_scale = PP_BG_FILL;
  IF_EQUAL("fit")          point->bg_scale = PP_BG_FIT;
  IF_EQUAL("center")       point->position = CLUTTER_GRAVITY_CENTER;
  IF_EQUAL("top")          point->position = CLUTTER_GRAVITY_NORTH;
  IF_EQUAL("bottom")       point->position = CLUTTER_GRAVITY_SOUTH;
  IF_EQUAL("left")         point->position = CLUTTER_GRAVITY_WEST;
  IF_EQUAL("right")        point->position = CLUTTER_GRAVITY_EAST;
  IF_EQUAL("top-left")     point->position = CLUTTER_GRAVITY_NORTH_WEST;
  IF_EQUAL("top-right")    point->position = CLUTTER_GRAVITY_NORTH_EAST;
  IF_EQUAL("bottom-left")  point->position = CLUTTER_GRAVITY_SOUTH_WEST;
  IF_EQUAL("bottom-right") point->position = CLUTTER_GRAVITY_SOUTH_EAST;
  IF_EQUAL("no-markup")    point->use_markup = FALSE;
  IF_EQUAL("markup")       point->use_markup = TRUE;
  DEFAULT                  point->bg = g_intern_string (setting);
  END_PARSER

/* undefine all the overrides, returning us to regular C */
#undef START_PARSER
#undef END_PARSER
#undef DEFAULT
#undef IF_PREFIX
#undef IF_EQUAL
#undef float
#undef char
#undef enum
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

static gboolean
pp_is_color (const char *string)
{
  ClutterColor color;
  return clutter_color_from_string (&color, string);
}

void
pp_parse_slides (PinPointRenderer *renderer,
                 const char       *slide_src)
{
  const char *p;
  int         slideno = 0;
  gboolean    done = FALSE;
  gboolean    startofline = TRUE;
  gboolean    gotconfig = FALSE;
  GString    *slide_str = g_string_new ("");
  GString    *setting_str = g_string_new ("");
  GList      *s;
  PinPointPoint *point, *next_point;

  if (renderer->source)
    {
      gboolean start_of_line = TRUE;
      int pos;
      int lineno=0;
      /* compute slide no that has changed */
      for (pos = 0, slideno = 0;
           slide_src[pos] && renderer->source[pos] && slide_src[pos]==renderer->source[pos]
           ; pos ++)
        {
          switch (slide_src[pos])
            {
              case '\n':
                start_of_line = TRUE;
                lineno++;
                break;
              case '-':
                if (start_of_line)
                  slideno++;
              default:
                start_of_line = FALSE;
            }
        }
      slideno--;
      g_free (renderer->source);
    }
  renderer->source = g_strdup (slide_src);

  for (s = pp_slides; s; s = s->next)
    pin_point_free (renderer, s->data);

  g_list_free (pp_slides);
  pp_slides = NULL;
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
          close_last_slide:
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
                  parse_config (point, setting_str->str);
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
                        point->bg_type = PP_BG_VIDEO;
                      else if (g_str_has_suffix (point->bg, ".svg"))
                        point->bg_type = PP_BG_SVG;
                      else if (pp_is_color (point->bg))
                        point->bg_type = PP_BG_COLOR;
                      else
                        point->bg_type = PP_BG_IMAGE;
                    }

                  {
                    char *str = slide_str->str;

                  /* trim newlines from start and end. ' ' can be used in the
                   * insane case you actually want blank lines before or after
                   * the text of a slide */
                    while (*str == '\n') str++;
                    while ( slide_str->str[strlen(slide_str->str)-1]=='\n')
                      slide_str->str[strlen(slide_str->str)-1]='\0';

                    point->text = g_intern_string (str);
                  }

                  renderer->make_point (renderer, point);

                  g_string_assign (slide_str, "");
                  g_string_assign (setting_str, "");

                  pp_slides = g_list_append (pp_slides, point);
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
  if (!done)
    {
      done = TRUE;
      goto close_last_slide;
    }

  g_string_free (slide_str, TRUE);
  g_string_free (setting_str, TRUE);

  if (g_list_nth (pp_slides, slideno))
    pp_slidep = g_list_nth (pp_slides, slideno);
  else
    pp_slidep = pp_slides;
}
