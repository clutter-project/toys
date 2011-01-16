SUBDIRS=transitions
bin_PROGRAMS=pinpoint

AM_CFLAGS = $(DEPS_CFLAGS) $(MAINTAINER_FLAGS) -D_GNU_SOURCE -DPKGDATADIR=\"$(pkgdatadir)/\"
ACLOCAL_AMFLAGS = -I m4

if USE_DAX
DAX_SOURCES = pp-super-aa.c pp-super-aa.h
endif

pinpoint_LDADD  = $(DEPS_LIBS)
pinpoint_SOURCES = \
  pinpoint.c \
  pinpoint.h \
  pp-cairo.c \
  pp-clutter.c \
  gst-video-thumbnailer.c \
  $(DAX_SOURCES)

EXTRA_DIST=sample.txt wth.jpg bg.jpg pp-super-aa.c pp-super-aa.h

MAINTAINERCLEANFILES = aclocal.m4 compile config.guess config.sub configure depcomp install-sh ltmain.sh Makefile.in missing

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-snap`date +"%Y%m%d"`

