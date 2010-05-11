noinst_PROGRAMS = viewer

INCLUDES = \
	-I $(top_srcdir)

AM_CPPFLAGS = \
	@CLUTTER_CFLAGS@

viewer_SOURCES = \
	viewer.c

viewer_LDADD = \
	@CLUTTER_LIBS@ \
	$(top_builddir)/clutter-ply/libclutter-ply.la

EXTRA_DIST = TheMonkey.ply
