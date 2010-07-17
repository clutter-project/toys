AC_PREREQ(2.53)
AC_INIT(pinpoint, [0.0], [])
AC_CONFIG_SRCDIR(pinpoint.c)
AC_CONFIG_AUX_DIR([build])
AM_INIT_AUTOMAKE([foreign -Wno-portability no-define])
AM_CONFIG_HEADER([config.h])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_PROG_CC
AC_HEADER_STDC

PKG_CHECK_MODULES(DEPS, clutter-1.0 gio-2.0)

AS_COMPILER_FLAGS([MAINTAINER_CFLAGS], [-Wall])

AC_OUTPUT([
	Makefile
])
