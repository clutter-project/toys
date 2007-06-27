AC_PREREQ(2.53)
AC_INIT(aaina, 0.1, [])
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/main.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

PKG_CHECK_MODULES(DEPS, clutter-0.3 gdk-2.0 gtk+-2.0 neon >= 0.26 libxml-2.0)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

if test "x$GCC" = "xyes"; then
        GCC_FLAGS="-g -Wall -Werror"
fi

AC_SUBST(GCC_FLAGS)

AC_OUTPUT([
Makefile
libaaina/Makefile
sources/Makefile
src/Makefile
])
