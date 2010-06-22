bin_PROGRAMS=pinpoint

AM_CFLAGS = $(DEPS_CFLAGS) $(GCC_FLAGS) -D_GNU_SOURCE

pinpoint_LDADD  = $(DEPS_LIBS)
pinpoint_SOURCES = pinpoint.c

EXTRA_DIST=sample.txt wth.jpg bg.jpg

MAINTAINERCLEANFILES = aclocal.m4 compile config.guess config.sub configure depcomp install-sh ltmain.sh Makefile.in missing

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-snap`date +"%Y%m%d"`

