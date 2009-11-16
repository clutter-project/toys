# A generic buildfiles to build single executable directory projects depending
# only on pkg-config ability to build. It automatically names the project on
# the toplevel directory you're in.
#
# Setting additional CFLAGS like $ export CFLAGS=-Wall -Werror # can help you
# track issues down better after compilation.
#
# 20071008
# Øyvind Kolås (c) 2007 <pippin@gimp.org>  placed in the Public Domain.
##

PKGMODULES = tidy-1.0 clutter-0.8 webkit-clutter-1.0

# you only need to change the following if you want to change where the
# generated tarball gets scp'd to:

SCP_DESTINATION=pug.vpn:tmp

BINARY=$(shell basename `pwd`)#
PACKAGE=../$(BINARY).tar.bz2 # you can use both .gz and .bz2 as extension here


##
# end of template configuration.
#

# This makefile uses the current directory as the only target binary, and
# expects a single of the .c files to contain a main function.



all: $(BINARY)

# The help available also contains brief information about the different
# build rules supported.
help: 
	@echo ''
	@echo 'Available targets in this make system'
	@echo ''
	@echo '  (none)   builds $(BINARY)'
	@echo '  dist     create $(PACKAGE)'
	@echo '  clean    rm *.o *~ and foo and bar'
	@echo '  run      ./$(BINARY)'
	@echo '  gdb      gdb ./$(BINARY)'
	@echo '  gdb2     gdb ./$(BINARY) --g-fatal-warnings'
	@echo '  scp      scp $(PACKAGE) $(SCP_DESTINATION)'
	@echo '  help     this help'
	@echo ''


LIBS= $(shell pkg-config --libs $(PKGMODULES) | sed -e 's/-Wl,\-\-export\-dynamic//')
INCS= $(shell pkg-config --cflags $(PKGMODULES))


CFILES  = $(wildcard *.c)
OBJECTS = $(subst ./,,$(CFILES:.c=.o))
HFILES  = $(wildcard *.h)
%.o: %.c $(HFILES)
	$(CC) -g $(CFLAGS) $(INCS) -c $< -o$@
$(BINARY): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LIBS)

#$(BINARY): $(CFILES)
#	$(LD) $(CFLAGS) $(INCS) $(CFILES) $(LIBS) -o $@
test: run
run: $(BINARY)
	./$(BINARY)

../$(BINARY).tar.gz: clean $(CFILES) $(HFILES)
	cd ..;tar czvhf $(BINARY).tar.gz $(BINARY)/*
	@ls -slah ../$(BINARY).tar.gz
../$(BINARY).tar.bz2: clean $(CFILES) $(HFILES)
	cd ..;tar cjvhf $(BINARY).tar.bz2 $(BINARY)/*
	@ls -slah ../$(BINARY).tar.bz2

dist: $(PACKAGE) 
	echo $(PACKAGE) 
scp: dist
	scp $(PACKAGE) $(SCP_DESTINATION)

gdb: all
	gdb --args ./$(BINARY)
gdb2: all
	gdb --args ./$(BINARY) -demo --g-fatal-warnings
clean:
	rm -fvr *.o $(BINARY) *~ *.patch
