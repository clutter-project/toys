bin_PROGRAMS=fluttr

PKGDATADIR = $(datadir)/fluttr
AM_CFLAGS = \
	$(DEPS_CFLAGS) 				\
	$(GCC_FLAGS) 				\
	-D_GNU_SOURCE 				\
	-DPKGDATADIR=\"$(PKGDATADIR)\"

fluttr_LDADD  = $(DEPS_LIBS)
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
	fluttr-photo.c				\
	fluttr-photo.h				\
	fluttr-settings.c			\
	fluttr-settings.h			\
	fluttr-spinner.c			\
	fluttr-spinner.h			\
	nflick-api-request.c			\
	nflick-api-request.h			\
	nflick-api-request-private.h		\
	nflick-api-response.c			\
	nflick-api-response.h			\
	nflick-api-response-private.h		\
	nflick-auth-worker.c			\
	nflick-auth-worker.h			\
	nflick-auth-worker-private.h		\
	nflick-flickr.h				\
	nflick-get-sizes-response.c		\
	nflick-get-sizes-response.h		\
	nflick-get-sizes-response-private.h	\
	nflick-gft-response.c			\
	nflick-gft-response.h			\
	nflick-gft-response-private.h		\
	nflick-no-set-response.c		\
	nflick-no-set-response.h		\
	nflick-no-set-response-private.h	\
	nflick-photo-data.c			\
	nflick-photo-data.h			\
	nflick-photo-list-response.c		\
	nflick-photo-list-response.h		\
	nflick-photo-list-private.h		\
	nflick-photo-list-worker.c		\
	nflick-photo-list-worker.h		\
	nflick-photo-list-worker-private.h	\
	nflick-photo-set.c			\
	nflick-photo-set.h			\
	nflick-photo-set-private.h		\
	nflick-pixbuf-fetch.c			\
	nflick-pixbuf-fetch.h			\
	nflick-pixbuf-fetch-private.h		\
	nflick-set-list-response.c		\
	nflick-set-list-response.h		\
	nflick-set-list-response-private.h	\
	nflick-set-list-worker.c		\
	nflick-set-list-worker.h		\
	nflick-set-list-worker-private.h	\
	nflick-show-worker.c			\
	nflick-show-worker.h			\
	nflick-show-worker-private.h		\
	nflick-types.h				\
	nflick-worker.c				\
	nflick-worker.h				\
	nflick-worker.private.h						

