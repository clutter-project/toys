INCLUDES =\
	-I$(srcdir) \
	$(DEPS_CFLAGS) \
	$(GCC_CFLAGS) \
	-DPREFIX=\"$(prefix)\" \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DPKGDATADIR=\"$(pkgdatadir)\" \
	-DLIBDIR=\"$(libdir)\" \
	-I$(top_builddir)/libastro-desktop

APP_SOURCES = \
	init.c \
	astro-contacts.c \
	astro-contacts-window.c \
	astro-reflection.c \
	clutter-reflect-texture.c

APP_LDADD = \
	$(DEPS_LIBS) \
	$(top_builddir)/libastro-desktop/libastro-desktop.la	        
        

contactslibdir = $(libdir)/astro-desktop/apps
contactslib_LTLIBRARIES = contacts.la
contacts_la_SOURCES = $(APP_SOURCES)
contacts_la_LIBADD = $(APP_LDADD)
contacts_la_LDFLAGS = -module -avoid-version
contacts_la_CFLAGS =

