
#include <glib.h>
#include <libastro-desktop/astro.h>
#include <libastro-desktop/astro-application.h>

#include "astro-contacts.h"


AstroApplication *
astro_application_factory_init ()
{
  AstroApplication *app;
  GdkPixbuf *pixbuf;
 
  pixbuf = gdk_pixbuf_new_from_file (PKGDATADIR "/icons/contacts.png", NULL);

  app = astro_contacts_new ("Contacts", pixbuf);

  g_debug ("Example applet loaded\n");

  return app;
}
