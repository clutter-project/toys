AC_INIT([clutter-ply], [0.1])

AC_CONFIG_SRCDIR([clutter-ply/clutter-ply.h])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.9])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_LIBTOOL

PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.16 gobject-2.0 >= 2.16])
PKG_CHECK_MODULES(CLUTTER, [clutter-1.0 >= 1.2])

# prefixes for fixing gtk-doc references
CLUTTER_PREFIX="`$PKG_CONFIG --variable=prefix clutter-1.0`"
AC_SUBST(CLUTTER_PREFIX)

GTK_DOC_CHECK([1.14],[--flavour no-tmpl])

GOBJECT_INTROSPECTION_CHECK([0.6.7])

AC_PATH_PROG([GLIB_MKENUMS], [glib-mkenums])

AC_CONFIG_FILES([
        Makefile
        clutter-ply/Makefile
        clutter-ply/rply/Makefile
        example/Makefile
        docs/Makefile
        docs/reference/Makefile
        docs/reference/clutter-ply-docs.sgml
        clutter-ply.pc
])

AC_OUTPUT
