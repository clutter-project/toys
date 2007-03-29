AC_PREREQ(2.53)
AC_INIT(woohaa, 0.0, [http://bugzilla.o-hand.com/enter_bug.cgi?product=woohaa])
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(woohaa.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS

PKG_CHECK_MODULES(DEPS, clutter-0.2 gnome-vfs-2.0 clutter-gst-0.1 sqlite3 gdk-2.0 gconf-2.0)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

if test "x$GCC" = "xyes"; then
        GCC_FLAGS="-g -Wall"
fi

AC_SUBST(GCC_FLAGS)

AC_OUTPUT([
Makefile
data/Makefile
])
