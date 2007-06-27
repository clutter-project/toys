noinst_LTLIBRARIES = libaaina.la

INCLUDES = \
	$(DEPS_CFLAGS)
	-I$(top_srcdir) 				\
	-I$(top_builddir) 				\
	$(DEPS_CFLAGS)					\
	-DDATADIR=\""$(datadir)"\" 			\
	-DSYSCONFDIR=\""$(sysconfdir)"\" 		\
	-Wall						\
	$(NULL)	

libaaina_la_SOURCES =	\
	aaina-behave.c					\
	aaina-behave.h					\
	aaina-library.c					\
	aaina-library.h					\
	aaina-photo.c					\
	aaina-photo.h					\
	eggsequence.c					\
	eggsequence.h

libaaina_la_LIBADD = $(DEPS_LIBS)
libaaina_la_LDFLAGS = -version-info 0:1:0
