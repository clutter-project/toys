SUBDIRS = rply

lib_LTLIBRARIES = libclutter-ply.la

INCLUDES = \
	-I$(top_srcdir)

AM_CPPFLAGS = \
	-DCLUTTER_PLY_COMPILATION=1 \
	@CLUTTER_CFLAGS@

source_h = \
	$(srcdir)/clutter-ply.h \
	$(srcdir)/clutter-ply-data.h \
	$(srcdir)/clutter-ply-model.h

libclutter_ply_la_SOURCES = \
	$(source_h) \
	$(srcdir)/clutter-ply-data.c \
	$(srcdir)/clutter-ply-model.c

clutter_plydir = $(includedir)/clutter-ply

clutter_ply_HEADERS = \
	$(source_h)

libclutter_ply_la_LIBADD = \
	@CLUTTER_LIBS@ \
	rply/librply.la

CLEANFILES =

BUILT_GIRSOURCES =

if HAVE_INTROSPECTION

ClutterPly.gir: $(INTROSPECTION_SCANNER) libclutter-ply.la
	$(QUIET_GEN)$(INTROSPECTION_SCANNER) -v \
		--namespace ClutterPly --nsversion=0.1 \
		$(INCLUDES) \
		$(AM_CPPFLAGS) \
		--add-include-path=cogl/cogl \
		$(json_gir_include_path) \
		--c-include='clutter-ply/clutter-ply.h' \
		--include=Clutter-1.0 \
		--output $@ \
		--library=libclutter-ply.la \
		$(libclutter_ply_la_SOURCES)

BUILT_GIRSOURCES += ClutterPly.gir

# INTROSPECTION_GIRDIR/INTROSPECTION_TYPELIBDIR aren't the right place to
# install anything - we need to install inside our prefix.
girdir = $(datadir)/gir-1.0
gir_DATA = $(BUILT_GIRSOURCES)

typelibsdir = $(libdir)/girepository-1.0/

typelibs_DATA = $(BUILT_GIRSOURCES:.gir=.typelib)

%.typelib: %.gir $(INTROSPECTION_COMPILER)
	$(QUIET_GEN) \
	LD_LIBRARY_PATH=.libs$${LD_LIBRARY_PATH:+:$$LD_LIBRARY_PATH} \
	$(INTROSPECTION_COMPILER) \
		--includedir=$(srcdir) \
		--includedir=. \
		$(INTROSPECTION_COMPILER_OPTS) $< -o $(@F)

CLEANFILES += $(BUILT_GIRSOURCES) $(typelibs_DATA)

endif # HAVE_INTROSPECTION
