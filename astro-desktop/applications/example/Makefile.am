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
	astro-example.c

APP_LDADD = \
	$(DEPS_LIBS) \
	$(top_builddir)/libastro-desktop/libastro-desktop.la	        
        

examplelibdir = $(libdir)/astro-desktop/apps
examplelib_LTLIBRARIES = example.la
example_la_SOURCES = $(APP_SOURCES)
example_la_LIBADD = $(APP_LDADD)
example_la_LDFLAGS = -module -avoid-version
example_la_CFLAGS =

