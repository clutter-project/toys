noinst_LTLIBRARIES = libaaina.la

INCLUDES = \
	$(DEPS_CFLAGS)
	-I$(top_srcdir) 				\
	-I$(top_builddir) 				\
	$(GCC_CFLAGS)					\
	-DDATADIR=\""$(datadir)"\" 			\
	-DSYSCONFDIR=\""$(sysconfdir)"\" 		\
	-Werror						\
	$(NULL)	

libaaina_la_SOURCES =	\
	aaina-behave.c					\
	aaina-behave.h					\
	aaina-library.c					\
	aaina-library.h					\
	aaina-photo.c					\
	aaina-photo.h					\
	aaina-source.c					\
	aaina-source.h					\
	eggsequence.c					\
	eggsequence.h

libaaina_la_LIBADD = $(DEPS_LIBS)
libaaina_la_LDFLAGS = 
