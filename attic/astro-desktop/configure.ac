AC_PREREQ(2.53)
AC_INIT(astro-desktop, 0.1, [])
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/main.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

PKG_CHECK_MODULES(DEPS, 
                  clutter-0.6 
                  gdk-2.0)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

if test "x$GCC" = "xyes"; then
        GCC_CFLAGS="-g -Wall"
fi
AC_SUBST(GCC_CFLAGS)

AC_OUTPUT([
Makefile
data/Makefile
data/albums/Makefile
data/icons/Makefile
libastro-desktop/Makefile
applets/Makefile
applications/Makefile
applications/contacts/Makefile
applications/example/Makefile
applications/images/Makefile
applications/music/Makefile
src/Makefile
])
