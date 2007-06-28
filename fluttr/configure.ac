AC_PREREQ(2.53)
AC_INIT(fluttr, 0.1.1, [])
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/main.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

dnl Check for clutter-0.3 and print a warning message if not found, as people
dnl are still svn co'ing Fluttr from the blog post.
PKG_CHECK_EXISTS(clutter-0.3, , AC_MSG_ERROR([*** Fluttr is now tracking clutter svn. Please see the README file for more information***]))

PKG_CHECK_MODULES(DEPS, clutter-0.3 gdk-2.0 gtk+-2.0 neon >= 0.26.0 libxml-2.0)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

if test "x$GCC" = "xyes"; then
        GCC_FLAGS="-g -Wall"
fi

AC_SUBST(GCC_FLAGS)

AC_OUTPUT([
Makefile
data/Makefile
libnflick/Makefile
src/Makefile
])
