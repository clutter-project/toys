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
	astro-music.c \
	astro-music-window.c

APP_LDADD = \
	$(DEPS_LIBS) \
	$(top_builddir)/libastro-desktop/libastro-desktop.la	        
        

musiclibdir = $(libdir)/astro-desktop/apps
musiclib_LTLIBRARIES = music.la
music_la_SOURCES = $(APP_SOURCES)
music_la_LIBADD = $(APP_LDADD)
music_la_LDFLAGS = -module -avoid-version
music_la_CFLAGS =

