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

#ifndef __PINPOINT_H__
#define __PINPOINT_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <clutter/clutter.h>

typedef struct _PinPointPoint    PinPointPoint;
typedef struct _PinPointRenderer PinPointRenderer;

typedef enum
{
  PP_TEXT_LEFT = PANGO_ALIGN_LEFT,
  PP_TEXT_CENTER = PANGO_ALIGN_CENTER,
  PP_TEXT_RIGHT = PANGO_ALIGN_RIGHT
} PPTextAlign;

typedef enum
{
  PP_BG_NONE,
  PP_BG_COLOR,
  PP_BG_IMAGE,
  PP_BG_VIDEO,
  PP_BG_SVG
} PPBackgroundType;

typedef enum
{
  PP_BG_FIT,  /* default value */
  PP_BG_FILL
} PPBackgroundScale;

#define PINPOINT_RENDERER(renderer) ((PinPointRenderer *) renderer)

struct _PinPointRenderer
{
  void      (*init)          (PinPointRenderer  *renderer,
                              char              *pinpoint_file);
  void      (*run)           (PinPointRenderer *renderer);
  void      (*finalize)      (PinPointRenderer *renderer);
  gboolean  (*make_point)    (PinPointRenderer *renderer,
                              PinPointPoint    *point);
  void *    (*allocate_data) (PinPointRenderer *renderer);
  void      (*free_data)     (PinPointRenderer *renderer,
                              void             *datap);
  char *      source;
};

struct _PinPointPoint
{
  const char        *stage_color;

  const gchar       *bg;
  PPBackgroundType   bg_type;
  PPBackgroundScale  bg_scale;

  const char        *text;            /*  the text of the slide */
  ClutterGravity     position;
  const char        *font;
  PPTextAlign        text_align;
  const char        *text_color;
  gboolean           use_markup;

  const char        *shading_color;
  float              shading_opacity;
  const char        *transition;      /* transition template to use, if any */

  const char        *command;

  void              *data;            /* the renderer can attach data here */
};

extern char      *pp_output_filename;
extern gboolean   pp_fullscreen;
extern gboolean   pp_maximized;

extern GList *pp_slides;
extern GList *pp_slidep;

void     pp_parse_slides  (PinPointRenderer *renderer,
                           const char       *slide_src);

void
pp_get_padding (float  stage_width,
                float  stage_height,
                float *padding);

void
pp_get_background_position_scale (PinPointPoint *point,
                                  float          stage_width,
                                  float          stage_height,
                                  float          bg_width,
                                  float          bg_height,
                                  float         *bg_x,
                                  float         *bg_y,
                                  float         *bg_scale);
void
pp_get_text_position_scale (PinPointPoint *point,
                            float          stage_width,
                            float          stage_height,
                            float          text_width,
                            float          text_height,
                            float         *text_x,
                            float         *text_y,
                            float         *text_scale);

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
                              float *shading_height);

#endif
