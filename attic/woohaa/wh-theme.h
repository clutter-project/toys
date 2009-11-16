#ifndef _WH_THEME
#define _WH_THEME

#include <glib.h>
#include <clutter/clutter.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

void
wh_theme_init();

const char*
wh_theme_get_font (const char *id);

const ClutterColor*
wh_theme_get_color(const char *id);

const ClutterMedia*
wh_theme_get_sound(const char *id);

GdkPixbuf*
wh_theme_get_pixbuf(const char *id);

G_END_DECLS

#endif
