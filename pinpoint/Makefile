# Hytte a generic buildfiles to build single executable directory projects
# depending only on pkg-config ability to build. 
#
# Setting additional CFLAGS like $ export CFLAGS=-Wall -Werror # can help you
# track issues down better after compilation.
#
# Authored by Øyvind Kolås 2007-2008 <pippin@gimp.org> 
# placed in the Public Domain.
#

PROJECT_NAME=$(shell basename `pwd`)

# Dependencies of the project.
PKGMODULES = clutter-1.0

#extra CFLAGS
CFLAGS=-Wall 
#extra LIBS
LIBS=

TEST_PARAMS= # parameters passed to binary when testing

# the locations used for web, git-sync and snapshot targets:

DOCS_REMOTE=pippin@gimp.org:public_html/$(PROJECT_NAME)
GIT_RSYNC_REMOTE=$(DOCS_REMOTE)/$(PROJECT_NAME).git
SNAPSHOT_REMOTE=$(DOCS_REMOTE)/snapshots

#
# end of project/user high level configuration.
#

# This makefile uses the current directory as the only target binary, and
# expects a single of the .c files to contain a main function.

BINARY=$(PROJECT_NAME)
all: DOCUMENTATION $(BINARY)

DOCUMENTATION:
	@test -d docs && make -C docs || true
web: DOCUMENTATION
	@test -d docs && scp docs/*.html $(DOCS_REMOTE) || true


TIMESTAMP=$(shell date +%Y%m%d-%H%M)
PACKAGE=../$(BINARY)-$(TIMESTAMP).tar.bz2

# The help available also contains brief information about the different
#
# build rules supported.
help: 
	@echo '$(PROJECT_NAME) make targets.'
	@echo ''
	@echo '  (none)   builds $(PROJECT_NAME)'
	@echo '  dist     create $(PACKAGE)'
	@echo '  clean    rm *.o *~ and foo and bar'
	@echo '  test     ./$(BINARY)'
	@echo '  gdb      gdb ./$(BINARY)'
	@echo '  gdb2     gdb ./$(BINARY) --g-fatal-warnings'
	@echo '  valgrind valgrind ./$(BINARY)'
	@echo '  git-sync rsync git repo at $(GIT_RSYNC_REMOTE)'
	@echo '  snapshot put tarball at $(SNAPSHOT_REMOVE)'
	@echo '  help     this help'
	@test -d docs && echo '  web      publish website'||true
	@echo ''


LIBS+= $(shell pkg-config --libs $(PKGMODULES) | sed -e 's/-Wl,\-\-export\-dynamic//')
INCS= $(shell pkg-config --cflags $(PKGMODULES)) -I.. -I.


CFILES  = $(wildcard *.c) $(wildcard */*.c)
HFILES  = $(wildcard *.h) $(wildcard */*.h)
# we rebuild on every .h file change this could be improved by making
# deps files.
OBJECTS = $(subst ./,,$(CFILES:.c=.o))

%.o: %.c $(HFILES) 
	@$(CC) -g $(CFLAGS) $(INCS) -c $< -o $@\
	    && echo "$@ compiled" \
	    || ( echo "$@ **** compile failed ****"; false)
$(BINARY): $(OBJECTS)
	@$(CC) -o $@ $(OBJECTS) $(LIBS) \
	    && echo "$@ linked" \
	    || (echo "$@ **** linking failed ****"; false)

test: run
run: $(BINARY)
	@echo -n "$$ "
	./$(BINARY) $(TEST_PARAMS)

clean:
	@rm -fvr *.o */*.o $(BINARY) *~ */*~ *.patch $(CLEAN_LOCAL)
	@test -d docs && make -C docs clean || true

install:
	install $(BINARY) /usr/local/bin

gdb: all
	gdb --args ./$(BINARY)
gdb2: all
	gdb --args ./$(BINARY) -demo --g-fatal-warnings
valgrind: all
	valgrind --leak-check=full ./$(BINARY)
callgrind: all
	valgrind --tool=callgrind ./$(BINARY)



../$(BINARY)-$(TIMESTAMP).tar.bz2: clean $(CFILES) $(HFILES)
	cd ..;tar cjvf $(PROJECT_NAME)-$(TIMESTAMP).tar.bz2 $(PROJECT_NAME)/*
	@ls -slah ../$(PROJECT_NAME)-$(TIMESTAMP).tar.bz2

dist: $(PACKAGE) 
snapshot: dist
	scp $(PACKAGE) $(SNAPSHOT_REMOTE)
git-sync:
	git checkout master
	git-update-server-info
	rsync --delete -avprl .git/* $(GIT_RSYNC_REMOTE)


#### custom rules for project, could override a specific .o file for instance
#
#all_local: foo
#all: all_local
#
#foo: Makefile
#	touch foo
#
#CLEAN_LOCAL=foo
