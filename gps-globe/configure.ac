AC_INIT([gps-globe], 0.1)

AM_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([1.9])

AC_PROG_CC
AC_PROG_RANLIB

PKG_CHECK_MODULES(CLUTTER, [clutter-1.0])

AC_PATH_PROG([GLIB_GENMARSHAL], [glib-genmarshal])
AC_PATH_PROG([GLIB_MKENUMS], [glib-mkenums])

AC_CONFIG_FILES([
        Makefile
        src/Makefile
])

AC_OUTPUT
