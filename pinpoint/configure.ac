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

PINPOINT_DEPS="clutter-1.0 >= 1.3 gio-2.0 cairo-pdf pangocairo gdk-pixbuf-2.0"

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
       AC_DEFINE([HAVE_PDF], [1], [Whether pinpoint will generate PDFs])])
AM_CONDITIONAL([HAVE_PDF], [test "x$have_pdf" = "xyes"])

# ClutterGst support
AC_ARG_ENABLE([cluttergst],
	      [AS_HELP_STRING([--enable-cluttergst=@<:no/auto/yes:>@],
			      [Show media files using ClutterGst])],,
			      [enable_cluttergst=auto])
AC_MSG_CHECKING([for the stuff needed to show media files])
AS_CASE([$enable_cluttergst],
	[no], [have_cluttergst="no (disabled)"],
	[yes], [PKG_CHECK_EXISTS([clutter-gst-1.0 >= 1.3],
                                 have_cluttergst="yes",
                                 AC_MSG_ERROR([Oh no!]))],
	[auto], [PKG_CHECK_EXISTS([clutter-gst-1.0 >= 1.3],
				  have_cluttergst="yes",
				  have_cluttergst="no")],
	AC_MSG_ERROR([invalid argumented passed to --enable-cluttergst]))
AC_MSG_RESULT([$have_cluttergst])
AS_IF([test "x$have_cluttergst" = "xyes"], [
       PINPOINT_DEPS="$PINPOINT_DEPS clutter-gst-1.0 >= 1.3"
       AC_DEFINE([USE_CLUTTER_GST], [1], [Whether pinpoint will display media files using ClutterGst])])
AM_CONDITIONAL([USE_CLUTTER_GST], [test "x$have_cluttergst" = "xyes"])

# SVG output support in the PDFs
AC_ARG_ENABLE([rsvg],
	      [AS_HELP_STRING([--enable-rsvg=@<:no/auto/yes:>@],
			      [Use SVGs in PDFs])],,
			      [enable_rsvg=auto])
AC_MSG_CHECKING([for the stuff needed to put SVGs in PDFs])
AS_CASE([$enable_rsvg],
	[no], [have_rsvg="no (disabled)"],
	[yes], [PKG_CHECK_EXISTS([librsvg-2.0],
                                 have_rsvg="yes",
                                 AC_MSG_ERROR([Oh no!]))],
	[auto], [PKG_CHECK_EXISTS([librsvg-2.0],
				  have_rsvg="yes",
				  have_rsvg="no")],
	AC_MSG_ERROR([invalid argumented passed to --enable-rsvg]))
AC_MSG_RESULT([$have_rsvg])
AS_IF([test "x$have_rsvg" = "xyes"], [
       PINPOINT_DEPS="$PINPOINT_DEPS librsvg-2.0"
       AC_DEFINE([HAVE_RSVG], [1], [Whether pinpoint will generate RSVGs])])
AM_CONDITIONAL([HAVE_RSVG], [test "x$have_rsvg" = "xyes"])

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
       AC_DEFINE([USE_DAX], [1], [Whether pinpoint will use Dax])])
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
echo "       Images:      yes (built-in)"
echo "       SVG:         ${use_dax}"
echo "       SVG in PDF:  ${have_rsvg}"
echo "       ClutterGst:  ${have_cluttergst}"

echo ""
echo " • Renderers"
echo "       Clutter: yes (built-in)"
echo "       PDF    : $have_pdf"

echo ""
