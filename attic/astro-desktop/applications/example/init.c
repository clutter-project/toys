
#include <glib.h>
#include <libastro-desktop/astro.h>
#include <libastro-desktop/astro-application.h>

#include "astro-example.h"


AstroApplication *
astro_application_factory_init ()
{
  AstroApplication *app;
  GdkPixbuf *pixbuf;
 
  pixbuf = gdk_pixbuf_new_from_file_at_scale (PKGDATADIR "/icons/exec.png", 
                                     ASTRO_APPICON_SIZE(), ASTRO_APPICON_SIZE(),
                                     TRUE,
                                     NULL);

  app = astro_example2_new ("Example Application", pixbuf);

  g_debug ("Example application loaded\n");

  return app;
}
