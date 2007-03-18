SUBDIRS=data

bin_PROGRAMS=woohaa

PKGDATADIR = $(datadir)/woohaa

AM_CFLAGS = $(DEPS_CFLAGS) $(GCC_FLAGS) -D_GNU_SOURCE -DPKGDATADIR=\"$(PKGDATADIR)\"

woohaa_LDADD  = $(DEPS_LIBS)
woohaa_SOURCES = woohaa.c               \
		clutter-simple-layout.c \
		clutter-simple-layout.h \
		wh-busy.c               \
		wh-busy.h               \
		wh-slider-menu.c        \
		wh-slider-menu.h        \
		wh-screen-video.c       \
		wh-screen-video.h       \
		wh-video-model.c        \
		wh-video-model.h        \
		wh-video-view.c         \
		wh-video-view.h         \
		wh-video-model-row.c    \
		wh-video-model-row.h    \
		wh-video-row-renderer.c \
		wh-video-row-renderer.h \
		wh-db.c                 \
		wh-db.h                 \
		eggsequence.c	        \
		eggsequence.h           \
		util.c                  \
		util.h                  

MAINTAINERCLEANFILES = aclocal.m4 compile config.guess config.sub configure depcomp install-sh ltmain.sh Makefile.in missing

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-snap`date +"%Y%m%d"`

