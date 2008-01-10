
#include <glib.h>
#include <libastro-desktop/astro.h>
#include <libastro-desktop/astro-application.h>

#include "astro-example.h"


GObject *
astro_application_factory_init ()
{
  void *a;
  GdkPixbuf *pixbuf;
 
  pixbuf = gdk_pixbuf_new_from_file (PKGDATADIR "/icons/exec.png", NULL);

  a = astro_example2_new ("Example Application", pixbuf);

  g_debug ("Example applet loaded\n");

  return G_OBJECT (a);
}
