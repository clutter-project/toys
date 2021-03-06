
#include <glib.h>
#include <libastro-desktop/astro.h>
#include <libastro-desktop/astro-defines.h>
#include <libastro-desktop/astro-application.h>

#include "astro-images.h"


AstroApplication *
astro_application_factory_init ()
{
  AstroApplication *app;
  GdkPixbuf *pixbuf;
 
  pixbuf = gdk_pixbuf_new_from_file_at_scale (PKGDATADIR "/icons/images.png", 
                                     ASTRO_APPICON_SIZE(), ASTRO_APPICON_SIZE(),
                                     TRUE,
                                     NULL);

  app = astro_images_new ("Images", pixbuf);

  g_debug ("Images application loaded\n");

  return app;
}
