bin_PROGRAMS=fluttr

PKGDATADIR = $(datadir)/fluttr
AM_CFLAGS = \
	$(DEPS_CFLAGS) 				\
	$(GCC_FLAGS) 				\
	-D_GNU_SOURCE 				\
	-DPKGDATADIR=\"$(PKGDATADIR)\"

fluttr_LDADD  = $(DEPS_LIBS)
fluttr_SOURCES = \
	main.c				
			
	 
