SUBDIRS = rply

lib_LTLIBRARIES = libclutter-ply.la

INCLUDES = \
	-I$(top_srcdir)

AM_CPPFLAGS = \
	-DCLUTTER_PLY_COMPILATION=1 \
	@CLUTTER_CFLAGS@

source_h = \
	$(srcdir)/clutter-ply.h \
	$(srcdir)/clutter-ply-data.h \
	$(srcdir)/clutter-ply-model.h

libclutter_ply_la_SOURCES = \
	$(source_h) \
	$(srcdir)/clutter-ply-data.c \
	$(srcdir)/clutter-ply-model.c

clutter_plydir = $(includedir)/clutter-ply

clutter_ply_HEADERS = \
	$(source_h)

libclutter_ply_la_LIBADD = \
	@CLUTTER_LIBS@ \
	rply/librply.la
