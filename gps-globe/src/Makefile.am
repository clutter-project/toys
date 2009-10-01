bin_PROGRAMS = gps-globe

INCLUDES = \
	@CLUTTER_CFLAGS@

source_h = \
	gpsg-sphere.h

gps_globe_SOURCES = \
	gpsg-main.c \
	gpsg-sphere.c \
	gpsg-enum-types.c \
	gpsg-enum-types.h \
	gpsg-sphere-vertex-shader.c \
	gpsg-sphere-vertex-shader.h \
	$(source_h)

.glsl.c :
	echo $< | \
	sed -e 's/-/_/g' -e 's/^\(.\+\)\.glsl$$/const char \1[] =/' > $@ ; \
	sed -e 's/["\\]/\\&/g' -e 's/^/"/' -e 's/$$/\\n"/' $< >> $@ ; \
	echo ";" >> $@

gps_globe_LDADD = \
	@CLUTTER_LIBS@

ENUMFILES = gpsg-enum-types.c gpsg-enum-types.h
STAMPFILES = stamp-gpsg-enum-types.h
BUILT_SOURCES = $(ENUMFILES)

gpsg-enum-types.h: stamp-gpsg-enum-types.h
	@true
stamp-gpsg-enum-types.h: $(source_h) Makefile
	$(QUIET_GEN)( $(GLIB_MKENUMS) \
	  	--template $(srcdir)/gpsg-enum-types.h.in \
	  $(source_h) ) >> xgen-ceth && \
	(cmp -s xgen-ceth gpsg-enum-types.h || cp xgen-ceth gpsg-enum-types.h) && \
	rm -f xgen-ceth && \
	echo timestamp > $(@F)

gpsg-enum-types.c: gpsg-enum-types.h
	$(QUIET_GEN)( $(GLIB_MKENUMS) \
	  	--template $(srcdir)/gpsg-enum-types.c.in \
	  $(source_h) ) >> xgen-cetc && \
	cp xgen-cetc gpsg-enum-types.c && \
	rm -f xgen-cetc

DISTCLEANFILES = \
	$(ENUMFILES)

EXTRA_DIST = \
	gpsg-enum-types.h.in \
	gpsg-enum-types.c.in \
	gpsg-sphere-vertex-shader.glsl
