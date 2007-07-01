noinst_LTLIBRARIES = libnflick.la

INCLUDES = \
	$(DEPS_CFLAGS)
	-I$(top_srcdir) 				\
	-I$(top_builddir) 				\
	$(DEPS_CFLAGS)					\
	-DDATADIR=\""$(datadir)"\" 			\
	-DSYSCONFDIR=\""$(sysconfdir)"\" 		\
	-Wall													\
	$(NULL)	

libnflick_la_SOURCES =	\
	nflick.h				\
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
	nflick-photo-list-response-private.h	\
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
	nflick-worker-private.h

libnflick_la_LIBADD = $(DEPS_LIBS)
libnflick_la_LDFLAGS = 
