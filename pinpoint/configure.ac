AC_PREREQ(2.53)
AC_INIT(pinpoint, [0.0], [])
AC_CONFIG_SRCDIR(pinpoint.c)
AC_CONFIG_AUX_DIR([build])
AM_INIT_AUTOMAKE([foreign -Wno-portability no-define])
AM_CONFIG_HEADER([config.h])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_PROG_CC
PKG_PROG_PKG_CONFIG
AC_HEADER_STDC

PINPOINT_DEPS="clutter-1.0 gio-2.0 cairo-pdf pangocairo gdk-pixbuf-2.0"

AS_COMPILER_FLAGS([MAINTAINER_CFLAGS], [-Wall])

# PDF output support
AC_ARG_ENABLE([pdf],
	      [AS_HELP_STRING([--enable-pdf=@<:no/auto/yes:>@],
			      [Produce PDFs])],,
			      [enable_pdf=auto])
AC_MSG_CHECKING([for the stuff needed to generate PDFs])
AS_CASE([$enable_pdf],
	[no], [have_pdf="no (disabled)"],
	[yes], [PKG_CHECK_EXISTS([cairo-pdf >= 1.9.12 pangocairo gdk-pixbuf-2.0],
                                 have_pdf="yes",
                                 AC_MSG_ERROR([Oh no!]))],
	[auto], [PKG_CHECK_EXISTS([cairo-pdf >= 1.9.12 pangocairo gdk-pixbuf-2.0],
				  have_pdf="yes",
				  have_pdf="no")],
	AC_MSG_ERROR([invalid argumented passed to --enable-pdf]))
AC_MSG_RESULT([$have_pdf])
AS_IF([test "x$have_pdf" = "xyes"], [
       PINPOINT_DEPS="$PINPOINT_DEPS cairo-pdf >= 1.9.12 pangocairo gdk-pixbuf-2.0"
       AC_DEFINE([HAVE_PDF], [1], [Wether pipoint will generate PDFs])])
AM_CONDITIONAL([HAVE_PDF], [test "x$have_pdf" = "xyes"])

# SVG support with Dax
AC_ARG_ENABLE([dax],
	      [AS_HELP_STRING([--enabled-dax=@<:no/auto/yes:>@],
			      [SVG loading with dax])],,
			      [enable_dax=auto])
AC_MSG_CHECKING([for dax])
AS_CASE([$enable_dax],
	[no], [use_dax="no (disabled)"],
	[yes], [PKG_CHECK_EXISTS([dax-0.2 mx-1.0],
				use_dax="yes",
				AC_MSG_ERROR([Dax not installed]))],
	[auto], [PKG_CHECK_EXISTS([dax-0.2 mx-1.0],
				  use_dax="yes",
				  use_dax="no")],
	AC_MSG_ERROR([invalid argumented passed to --enable-dax]))
AC_MSG_RESULT([$use_dax])
AS_IF([test "x$use_dax" = "xyes"], [
       PINPOINT_DEPS="$PINPOINT_DEPS dax-0.2 mx-1.0"
       AC_DEFINE([USE_DAX], [1], [Wether pipoint will use Dax])])
AM_CONDITIONAL([USE_DAX], [test "x$use_dax" = "xyes"])

PKG_CHECK_MODULES([DEPS], [$PINPOINT_DEPS])

AC_OUTPUT([
	Makefile
        transitions/Makefile
])

echo ""
echo "Pinpoint"

echo""
echo " • General"
echo "       prefix: ${prefix}"
echo "       cflags: ${MAINTAINER_CFLAGS}"

echo ""
echo " • Slides' background"
echo "       Images: yes (built-in)"
echo "       SVG   : ${use_dax}"

echo ""
echo " • Renderers"
echo "       Clutter: yes (built-in)"
echo "       PDF    : $have_pdf"

echo ""
