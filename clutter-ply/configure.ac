AC_INIT([clutter-ply], [0.1])

AC_CONFIG_SRCDIR([clutter-ply/clutter-ply.h])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.9])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_LIBTOOL

PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.16 gobject-2.0 >= 2.16])
PKG_CHECK_MODULES(CLUTTER, [clutter-1.0])

AC_CONFIG_FILES([
        Makefile
        clutter-ply/Makefile
        clutter-ply/rply/Makefile
        example/Makefile
        clutter-ply.pc
])

AC_OUTPUT
