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
	fluttr-auth.c				\
	fluttr-auth.h				\
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
	nflick-gft-response.c			\
	nflick-gft-response.h			\
	nflick-gft-response-private.h		\
	nflick-types.h				\
	nflick-worker.c				\
	nflick-worker.h				\
	nflick-worker.private.h						

