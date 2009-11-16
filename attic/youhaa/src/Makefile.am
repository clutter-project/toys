bin_PROGRAMS=youhaa

PKGDATADIR = $(datadir)/youhaa

AM_CFLAGS = $(DEPS_CFLAGS) $(GCC_FLAGS) -D_GNU_SOURCE -DPKGDATADIR=\"$(PKGDATADIR)\"

youhaa_LDADD  = $(DEPS_LIBS)
youhaa_SOURCES = \
		yh-main.c		\
		yh-theme.c		\
		yh-theme.h		\
		yh-youtube-browser.c	\
		yh-youtube-browser.h	\
		yh-youtube.c		\
		yh-youtube.h		\
		glibcurl.c		\
		glibcurl.h		\
		pause_png.h		\
		play_png.h

