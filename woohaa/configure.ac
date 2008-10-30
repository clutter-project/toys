AC_PREREQ(2.53)
AC_INIT(woohaa, 0.0, [http://bugzilla.o-hand.com/enter_bug.cgi?product=woohaa])
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(woohaa.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS

PKG_CHECK_MODULES(DEPS, clutter-0.8 gnome-vfs-2.0 clutter-gst-0.8 sqlite3 gdk-2.0 gdk-pixbuf-2.0 gconf-2.0)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

AC_ARG_ENABLE([helix],
	      AS_HELP_STRING([--enable-helix],[enable helix]),
	      [enable_helix=$enableval],
	      [enable_helix=no])

if test "x$GCC" = "xyes"; then
        GCC_FLAGS="-g -Wall"
fi

if test "x$enable_helix" = "xyes"; then
	PKG_CHECK_MODULES(CLUTTER_HELIX, [clutter-helix-0.8],
				         [has_helix=yes],
				         [has_helix=no])
	if test "x$has_helix" = "xno"; then
            AC_MSG_ERROR([Clutter-Helix libraries are not available.])
	fi

	GCC_FLAGS+=" -DUSE_HELIX"
        AC_SUBST(CLUTTER_HELIX_CFLAGS)
        AC_SUBST(CLUTTER_HELIX_LDFAGS)
fi

AC_SUBST(GCC_FLAGS)

AC_OUTPUT([
Makefile
data/Makefile
])
