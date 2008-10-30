#include "wh-theme.h"

#define FONT_DEFAULT "Sans"

#define PIXBUF_BG        PKGDATADIR "/bg.png"
#define PIXBUF_BUSY      PKGDATADIR "/busy.png"
#define PIXBUF_THUMBNAIL PKGDATADIR "/thumbnail-default.png"

#define COLOR_SLIDER
#define COLOR_SELECTOR
#define COLOR_TITLE_ACTIVE
#define COLOR_TITLE_INACTIVE
#define COLOR_DETAIS


typedef struct WHTheme
{
  GHashTable *sounds, *fonts, *colors, *pixbufs;
}
WHTheme;

static WHTheme *_theme = NULL;

void
wh_theme_init()
{
  GdkPixbuf    *pixbuf;

  _theme = g_new(WHTheme, 1);

  _theme->fonts   = g_hash_table_new (g_str_hash, g_str_equal);
  _theme->sounds  = g_hash_table_new (NULL, NULL);
  _theme->colors  = g_hash_table_new (NULL, NULL);
  _theme->pixbufs = g_hash_table_new (NULL, NULL);

  g_hash_table_insert (_theme->fonts, "default", FONT_DEFAULT);

  pixbuf = gdk_pixbuf_new_from_file (PKGDATADIR "/busy.png", NULL);
  if (pixbuf == NULL)
    g_error ("Failed to load" PKGDATADIR "/busy.png");
  g_hash_table_insert (_theme->pixbufs, "busy", pixbuf);

  pixbuf = gdk_pixbuf_new_from_file (PKGDATADIR "/bg.png", NULL);
  if (pixbuf == NULL)
    g_error ("Failed to load" PKGDATADIR "/bg.png");
  g_hash_table_insert (_theme->pixbufs, "bg", pixbuf);

  pixbuf = gdk_pixbuf_new_from_file (PKGDATADIR "/default-thumb.png", NULL);
  if (pixbuf == NULL)
    g_error ("Failed to load " PKGDATADIR "/default-thumb.png");
  g_hash_table_insert (_theme->pixbufs, "default-thumbnail", pixbuf);
}

const char*
wh_theme_get_font (const char *id)
{
  return (const char*)g_hash_table_lookup (_theme->fonts, id);
}

const ClutterColor*
wh_theme_get_color(const char *id)
{
  /* FIXME */
  return NULL;
}

const ClutterMedia*
wh_theme_get_sound(const char *id)
{
  return NULL;
}

GdkPixbuf*
wh_theme_get_pixbuf(const char *id)
{
  /* FIXME */
    return (GdkPixbuf*)g_hash_table_lookup (_theme->pixbufs, id);
}
