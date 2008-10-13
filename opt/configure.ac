AC_PREREQ(2.53)
AC_INIT(opt, 0.1, [http://bugzilla.o-hand.com/enter_bug.cgi?product=woohaa])
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(opt.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS

PKG_CHECK_MODULES(DEPS, gdk-pixbuf-2.0 clutter-0.8)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

if test "x$GCC" = "xyes"; then
        GCC_FLAGS="-g -Wall"
fi

AC_SUBST(GCC_FLAGS)

AC_OUTPUT([
Makefile
])
