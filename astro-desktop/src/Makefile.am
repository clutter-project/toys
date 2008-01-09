INCLUDES = \
	$(DEPS_CFLAGS) \
	$(WARN_CFLAGS)

bin_PROGRAMS = astro-desktop

AM_CFLAGS = \
	-DDATADIR=\""$(datadir)"\" \
	-DLIBDIR=\""$(libdir)"\" \
	-DPKGDATADIR=\""$(pkgdatadir)"\" \
	-I$(top_builddir)/libastro-desktop

astro_desktop_LDADD  = \
	$(DEPS_LIBS) \
	$(top_builddir)/libastro-desktop/libastro-desktop.la 

astro_desktop_SOURCES = \
	astro-appicon.c \
	astro-appicon.h \
	astro-applet.c \
	astro-applet.h \
	astro-applet-manager.c \
	astro-applet-manager.h \
	astro-appview.c \
	astro-appview.h \
	astro-desktop.c \
	astro-desktop.h \
	astro-example.c \
	astro-example.h \
	astro-panel.c \
	astro-panel.h \
	main.c

	
