noinst_LTLIBRARIES = librply.la
noinst_PROGRAMS = convert

AM_CPPFLAGS = \
	@GLIB_CFLAGS@

librply_la_SOURCES = \
	rply.c \
	rply.h

convert_SOURCES = \
	convert.c

convert_LDADD = librply.la @GLIB_LIBS@
