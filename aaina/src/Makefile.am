bin_PROGRAMS=aaina

PKGDATADIR = $(datadir)/aaina
AM_CFLAGS = \
	$(DEPS_CFLAGS) 				\
	$(GCC_FLAGS) 				\
	-I$(top_builddir)			\
	-I$(top_srcdir)				\
	-D_GNU_SOURCE 				\
	-DPKGDATADIR=\"$(PKGDATADIR)\"

aaina_LDADD  = \
	$(DEPS_LIBS)				\
	$(top_builddir)/libaaina/libaaina.la	\
	$(top_builddir)/sources/libsources.la

aaina_SOURCES = \
	aaina-slide-show.c			\
	aaina-slide-show.h			\
	main.c
