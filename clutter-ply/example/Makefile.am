noinst_PROGRAMS = monkey

INCLUDES = \
	-I $(top_srcdir)

AM_CPPFLAGS = \
	@CLUTTER_CFLAGS@

monkey_SOURCES = \
	monkey.c

monkey_LDADD = \
	@CLUTTER_LIBS@ \
	$(top_builddir)/clutter-ply/libclutter-ply.la

EXTRA_DIST = TheMonkey.ply
