bin_PROGRAMS=fluttr

PKGDATADIR = $(datadir)/fluttr
AM_CFLAGS = \
	$(DEPS_CFLAGS) 				\
	$(GCC_FLAGS) 				\
	-I$(top_builddir)/libaffinity	 	\	
	-D_GNU_SOURCE 				\
	-DPKGDATADIR=\"$(PKGDATADIR)\"

fluttr_LDADD  = $(DEPS_LIBS)			\
	$(top_builddir)/libnflick/libnflick.la  

fluttr_SOURCES = \
	main.c					\
	eggsequence.c				\
	eggsequence.h				\
	fluttr-auth.c				\
	fluttr-auth.h				\
	fluttr-behave.c				\
	fluttr-behave.h				\
	fluttr-library.c			\
	fluttr-library.h			\
	fluttr-library-row.c			\
	fluttr-library-row.h			\
	fluttr-list.c				\
	fluttr-list.h				\
	fluttr-list-view.c			\
	fluttr-list-view.h			\
	fluttr-photo.c				\
	fluttr-photo.h				\
	fluttr-set.c				\
	fluttr-set.h				\
	fluttr-set-view.c			\
	fluttr-set-view.h			\
	fluttr-settings.c			\
	fluttr-settings.h			\
	fluttr-spinner.c			\
	fluttr-spinner.h			\
	fluttr-viewer.c				\
	fluttr-viewer.h				
