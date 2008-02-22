AC_PREREQ(2.53)
AC_INIT(youhaa, 0.0, [])
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/yh-main.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS

m4_include(libcurl.m4)

PKG_CHECK_MODULES(DEPS, clutter-0.6 clutter-gst-0.6 json-glib-1.0)

LIBCURL_CHECK_CONFIG([yes], [7.14.0],
	[DEPS_CFLAGS="$DEPS_CFLAGS $LIBCURL_CPPFLAGS"
	DEPS_LIBS="$DEPS_LIBS $LIBCURL"
	],
	AC_MSG_ERROR([libcurl >= 7.14.0 not found]))

AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

if test "x$GCC" = "xyes"; then
        GCC_FLAGS="-g -Wall"
fi

AC_SUBST(GCC_FLAGS)

AC_OUTPUT([
Makefile
data/Makefile
src/Makefile
])

