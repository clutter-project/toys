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

#include <clutter/clutter.h>
#include <gio/gio.h>
#ifdef USE_CLUTTER_GST
#include <clutter-gst/clutter-gst.h>
#endif
#include <string.h>
#include <stdlib.h>

#define RESTDEPTH   -9000.0
#define RESTX        4600.0
#define STARTPOS    -3000.0

static ClutterColor black = {0x00,0x00,0x00,0xff};

typedef struct PinPointPoint
{
  ClutterGravity  position;
  const char     *font;
  const char     *text_color;
  const char     *shading_color;
  float           shading_opacity;
  const gchar    *bg;
  const char     *command;

  ClutterActor   *background;
  ClutterActor   *text;
} PinPointPoint;

static PinPointPoint default_point = 
{
  CLUTTER_GRAVITY_CENTER, "Sans 60px", "white", "black", 0.66, "black", NULL,
};

static GList *slides      = NULL; /* list of slide texts */
static GList *slidep      = NULL; /* current slide */

static ClutterActor *stage;       /* stage */
static ClutterActor *background;
static ClutterActor *shading;
static ClutterActor *foreground;

static void     leave_slide   (void);
static void     show_slide    (void);
static void     parse_slides  (const char       *slide_src);
static void     file_changed  (GFileMonitor     *monitor,
                               GFile            *file,
                               GFile            *other_file,
                               GFileMonitorEvent event_type,
                               char             *path);
static void     stage_resized (ClutterActor     *actor,
                               GParamSpec       *pspec,
                               gpointer          data);
static gboolean key_pressed   (ClutterActor     *actor,
                               ClutterEvent     *event,
                               gpointer          data);

int main (int argc, char **argv)
{
  GFileMonitor *monitor;
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
  stage = clutter_stage_get_default ();

  background = clutter_group_new ();
  foreground = clutter_group_new ();
  shading = clutter_rectangle_new_with_color (&black);
  clutter_actor_set_opacity (shading, 0x77);

  clutter_container_add (CLUTTER_CONTAINER (stage),
                         background, shading, foreground, NULL);

  clutter_actor_show (stage);
  clutter_stage_set_color (CLUTTER_STAGE (stage), &black);
  g_signal_connect (stage, "key-press-event", G_CALLBACK (key_pressed), NULL);
  g_signal_connect (stage, "notify::width", G_CALLBACK (stage_resized), NULL);
  g_signal_connect (stage, "notify::height", G_CALLBACK (stage_resized), NULL);

  clutter_stage_set_user_resizable (CLUTTER_STAGE (stage), TRUE);

  if (argv[1])
    {
      monitor = g_file_monitor (g_file_new_for_commandline_arg (argv[1]),
                                G_FILE_MONITOR_NONE, NULL, NULL);
      g_signal_connect (monitor, "changed", G_CALLBACK (file_changed),
                                            argv[1]);
    }
  parse_slides (text);
  g_free (text);
  clutter_main ();

  clutter_actor_destroy (stage);
  g_list_free (slides);

  return 0;
}


static gboolean key_pressed (ClutterActor *actor,
                             ClutterEvent *event,
                             gpointer      data)
{


  if (event) /* There is no event for the first triggering */
  switch (clutter_event_get_key_symbol (event))
    {
      case CLUTTER_Left:
      case CLUTTER_Up:
      case CLUTTER_BackSpace:
        if (slidep && slidep->prev)
          {
            leave_slide ();
            slidep = slidep->prev;
            show_slide ();
          }
        break;
      case CLUTTER_Right:
      case CLUTTER_space:
        if (slidep && slidep->next)
          {
            leave_slide ();
            slidep = slidep->next;
            show_slide ();
          }
        break;
      case CLUTTER_Escape:
        clutter_main_quit ();
        break;
      case CLUTTER_F11:
        clutter_stage_set_fullscreen (CLUTTER_STAGE (stage),
                 !clutter_stage_get_fullscreen (CLUTTER_STAGE (stage)));
        break;
    }
  return TRUE;
}


static void leave_slide (void)
{
  PinPointPoint *point = slidep->data;
  float resty;

  resty = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (point->text),
                                              "rest-y"));
  clutter_actor_animate (point->text,
                         CLUTTER_LINEAR, 2000,
                         "depth", RESTDEPTH,
                         "scale-x", 1.0,
                         "scale-y", 1.0,
                         "x", RESTX,
                         "y", resty,
                         NULL);
  if (point->background)
    {
      clutter_actor_animate (point->background,
                             CLUTTER_LINEAR, 1000,
                             "opacity", 0x0,
                             NULL);
#ifdef USE_CLUTTER_GST
      if (CLUTTER_GST_IS_VIDEO_TEXTURE (point->background))
        {
          clutter_media_set_playing (CLUTTER_MEDIA (point->background), FALSE);
        }
#endif
    }
}


static void show_slide (void)
{
  PinPointPoint *point;
  if (!slidep)
    return;
 
  point = slidep->data;
  if (point->background)
    {
      float w,h;
      float x,y;
      float s = 1.0;
      if (CLUTTER_IS_RECTANGLE (point->background))
        {
          clutter_actor_get_size (stage, &w, &h);
          clutter_actor_set_size (point->background, w, h);
        }
      else
        {
          clutter_actor_get_size (point->background, &w, &h);
        }
      s = clutter_actor_get_width (stage)/w;
      if (s > 1.0 || clutter_actor_get_height (stage) < s * h)
        s = clutter_actor_get_height (stage)/h;
      clutter_actor_set_scale (point->background, s, s);
      x = (clutter_actor_get_width (stage)-w*s)/2;
      y = (clutter_actor_get_height (stage)-h*s)/2;
      clutter_actor_set_position (point->background, x, y);
      clutter_actor_animate (point->background,
                             CLUTTER_LINEAR, 1000,
                             "opacity", 0xff,
                             NULL);

#ifdef USE_CLUTTER_GST
      if (CLUTTER_GST_IS_VIDEO_TEXTURE (point->background))
        {
          clutter_media_set_progress (CLUTTER_MEDIA (point->background), 0.0);
          clutter_media_set_playing (CLUTTER_MEDIA (point->background), TRUE);
        }
#endif
    }

  {
    float w, h;
    float x, y;
    float sx = 1.0;
    float sy = 1.0;
    float padding = clutter_actor_get_width (stage) * 0.01;

    clutter_actor_get_size (point->text, &w, &h);

    sx = clutter_actor_get_width (stage) / w * 0.8;
    sy = clutter_actor_get_height (stage) / h * 0.8;

    if (sy < sx)
      sx = sy;
    if (sx > 1.0) /* avoid enlarging text */
      sx = 1.0;

    switch (point->position)
      {
        case CLUTTER_GRAVITY_EAST:
        case CLUTTER_GRAVITY_NORTH_EAST:
        case CLUTTER_GRAVITY_SOUTH_EAST:
          x = clutter_actor_get_width (stage) * 0.95 - clutter_actor_get_width (point->text) * sx;
          break;
        case CLUTTER_GRAVITY_WEST:
        case CLUTTER_GRAVITY_NORTH_WEST:
        case CLUTTER_GRAVITY_SOUTH_WEST:
          x = clutter_actor_get_width (stage) * 0.05;
          break;
        case CLUTTER_GRAVITY_CENTER:
        default:
          x = (clutter_actor_get_width (stage)-w*sx)/2;
          break;
      }
    switch (point->position)
      {
        case CLUTTER_GRAVITY_SOUTH:
        case CLUTTER_GRAVITY_SOUTH_EAST:
        case CLUTTER_GRAVITY_SOUTH_WEST:
          y = clutter_actor_get_height (stage) * 0.95 - clutter_actor_get_height (point->text) * sx;
          break;
        case CLUTTER_GRAVITY_NORTH:
        case CLUTTER_GRAVITY_NORTH_EAST:
        case CLUTTER_GRAVITY_NORTH_WEST:
          y = clutter_actor_get_height (stage) * 0.05;
          break;
        default:
          y = (clutter_actor_get_height (stage)-h*sx)/2;
          break;
      }

    clutter_actor_animate (point->text,
                           CLUTTER_EASE_OUT_QUINT, 1000,
                           "depth", 0.0,
                           "scale-x", sx,
                           "scale-y", sx,
                           "x", x,
                           "y", y,
                           NULL);

    if (clutter_actor_get_width (point->text) > 1.0)
      {
        ClutterColor color;
        clutter_color_from_string (&color, point->shading_color);
        clutter_actor_animate (shading,
               CLUTTER_LINEAR, 500,
               "x", x - padding,
               "y", y - padding,
               "opacity", (int)(point->shading_opacity*255),
               "color", &color,
               "width", clutter_actor_get_width (point->text) * sx + padding*2,
               "height", clutter_actor_get_height (point->text) * sx + padding*2,
               NULL);
      }
    else /* no text, fade out shading */
      clutter_actor_animate (shading,
             CLUTTER_LINEAR, 500,
             "opacity", 0,
             "x", x - padding,
             "y", y - padding,
             "width", clutter_actor_get_width (point->text) * sx + padding*2,
             "height", clutter_actor_get_height (point->text) * sx + padding*2,
             NULL);
  }

  if (point->command)
    {
      g_print ("running: %s\n", point->command);
      system (point->command);
    }
}

static void stage_resized (ClutterActor *actor,
                           GParamSpec   *pspec,
                           gpointer      data)
{
  show_slide (); /* redisplay the current slide */
}

static void file_changed (GFileMonitor     *monitor,
                          GFile            *file,
                          GFile            *other_file,
                          GFileMonitorEvent event_type,
                          char             *path)
{
  char   *text = NULL;
  if (!g_file_get_contents (path, &text, NULL, NULL))
    g_error ("failed to load slides from %s\n", path);
  parse_slides (text);
  g_free (text);
}

static void
parse_setting(PinPointPoint *point,
              const gchar   *setting)
{
#define IF_PREFIX(prefix) } else if (g_str_has_prefix (setting, prefix)) {
#define IF_EQUAL(string) } else if (g_str_equal (setting, string)) {
  if (0) {
    IF_PREFIX("font=") point->font = g_intern_string (strrchr (setting, '=') + 1);
    IF_PREFIX("text-color=") point->text_color = g_intern_string (strrchr (setting, '=') + 1);
    IF_PREFIX("shading-color=") point->shading_color = g_intern_string (strrchr (setting, '=') + 1);
    IF_PREFIX("shading-opacity=") point->shading_opacity = g_ascii_strtod (strrchr (setting, '=') + 1, NULL);
    IF_PREFIX("command=") point->command = g_intern_string (strrchr (setting, '=') + 1);
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

static void point_free (PinPointPoint *point)
{
  if (point->text)
    clutter_actor_destroy (point->text);
  if (point->background)
    clutter_actor_destroy (point->background);
  g_free (point);
}

static
PinPointPoint *pin_point_new (void)
{
  PinPointPoint *point;
  point = g_new0 (PinPointPoint, 1);
  *point = default_point;

  return point;
}

static void
parse_slides (const char *slide_src)
{
  const char *p;
  int      y;
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
    point_free (s->data);
  g_list_free (slides);
  slides = NULL;
  point = pin_point_new ();

  y = STARTPOS;

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
                  *point = default_point;
                  gotconfig = TRUE;
                  g_string_assign (slide_str, "");
                  g_string_assign (setting_str, "");
                }
              else
                {
                  if (point->bg && point->bg[0])
                    {
#ifdef USE_CLUTTER_GST
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
                          point->background = clutter_gst_video_texture_new ();
                          clutter_media_set_filename (CLUTTER_MEDIA (point->background), point->bg);
                          /* should pre-roll the video and set the size */
                          clutter_actor_set_size (point->background, 400, 300);
                        }
                      else
#endif
                        {
                          ClutterColor color;
                          if (clutter_color_from_string (&color, point->bg))
                            point->background = g_object_new (CLUTTER_TYPE_RECTANGLE,
                                           "color", &color,
                                           "width", 100.0,
                                           "height", 100.0,
                                           NULL);
                          else
                             point->background = g_object_new (
                                     CLUTTER_TYPE_TEXTURE,
                                     "filename", point->bg,
                                     "load-data-async", TRUE,
                                     NULL);
                        }
                    }

                  /* trim newlines and spaces from end */
                  while ( slide_str->str[strlen(slide_str->str)-1]=='\n'
                       || slide_str->str[strlen(slide_str->str)-1]==' ')
                          slide_str->str[strlen(slide_str->str)-1]='\0';

                  {
                    ClutterColor color;
                    clutter_color_from_string (&color, point->text_color);
                    point->text = g_object_new (CLUTTER_TYPE_TEXT,
                                                "font-name", point->font,
                                                "text", slide_str->str,
                                                "color", &color,
                                                NULL);
                  }

                  clutter_container_add_actor (CLUTTER_CONTAINER (foreground),
                                               point->text);

                  if (point->background)
                    {
                      clutter_container_add_actor (
                         CLUTTER_CONTAINER (background), point->background);
                      clutter_actor_set_opacity (point->background, 0);
                    }

                  clutter_actor_set_position (point->text, RESTX, y);
                  g_object_set_data (G_OBJECT (point->text), "rest-y",
                                     GINT_TO_POINTER (y));
                  y += clutter_actor_get_height (point->text);
                  clutter_actor_set_depth (point->text, RESTDEPTH);

                  g_string_assign (slide_str, "");
                  g_string_assign (setting_str, "");

                  slides = g_list_append (slides, point);
                  point = pin_point_new ();
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
  show_slide ();
}
