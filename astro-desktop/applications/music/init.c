
#include <glib.h>
#include <libastro-desktop/astro.h>
#include <libastro-desktop/astro-application.h>

#include "astro-music.h"


AstroApplication *
astro_application_factory_init ()
{
  AstroApplication *app;
  GdkPixbuf *pixbuf;
 
  pixbuf = gdk_pixbuf_new_from_file (PKGDATADIR "/icons/music.png", NULL);

  app = astro_music_new ("Example Application", pixbuf);

  g_debug ("Example applet loaded\n");

  return app;
}
