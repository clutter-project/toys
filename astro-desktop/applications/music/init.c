
#include <glib.h>
#include <libastro-desktop/astro.h>
#include <libastro-desktop/astro-application.h>

#include "astro-music.h"


AstroApplication *
astro_application_factory_init ()
{
  AstroApplication *app;
  GdkPixbuf *pixbuf;
 
  pixbuf = gdk_pixbuf_new_from_file_at_scale (PKGDATADIR "/icons/music.png",
                                     ASTRO_APPICON_SIZE(), ASTRO_APPICON_SIZE(),
                                     TRUE,
                                     NULL);

  app = astro_music_new ("Music Player", pixbuf);

  g_debug ("Example applet loaded\n");

  return app;
}
