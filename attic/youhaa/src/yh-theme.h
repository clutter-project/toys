
#ifndef _YH_THEME_H
#define _YH_THEME_H

#include <clutter/clutter-color.h>

#define BORDER  24
#define UBORDER CLUTTER_UNITS_FROM_INT(BORDER)
#define FRAME   6
#define UFRAME  CLUTTER_UNITS_FROM_INT(FRAME)

extern const ClutterColor stage_color;
extern const ClutterColor frame_color;
extern const ClutterColor bg_color;
extern const ClutterColor entry_color;
extern const ClutterColor text_color;
extern const ClutterColor black;
extern const ClutterColor white;
extern const ClutterColor red;
extern const gchar *font;
extern const gchar *small_font;

#endif

