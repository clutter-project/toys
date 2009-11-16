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
	astro-images.c \
	astro-images-window.c \
	clutter-reflect-texture.c 

APP_LDADD = \
	$(DEPS_LIBS) \
	$(top_builddir)/libastro-desktop/libastro-desktop.la	        
        

imageslibdir = $(libdir)/astro-desktop/apps
imageslib_LTLIBRARIES = images.la
images_la_SOURCES = $(APP_SOURCES)
images_la_LIBADD = $(APP_LDADD)
images_la_LDFLAGS = -module -avoid-version
images_la_CFLAGS =

