bin_PROGRAMS=opt

AM_CFLAGS = $(DEPS_CFLAGS) $(GCC_FLAGS) -D_GNU_SOURCE

opt_LDADD  = $(DEPS_LIBS)
opt_SOURCES = opt.c            \
	      opt.h            \
              opt-show.c       \
	      opt-show.h       \
              opt-slide.c      \
              opt-slide.h      \
	      opt-transition.h \
	      opt-transition.c \
	      opt-menu.c       \
	      opt-menu.h       \
	      opt-config.c 

EXTRA_DIST=powers.png opt.dtd demo.xml bg.png

MAINTAINERCLEANFILES = aclocal.m4 compile config.guess config.sub configure depcomp install-sh ltmain.sh Makefile.in missing

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-snap`date +"%Y%m%d"`

