
PROGRAMS = \
  object-store-example \
  object-store-test \
  $(NULL)

example_SOURCES = \
  foo-object-store.c \
  foo-object-store.h \
  foo-test-object.c \
  foo-test-object.h \
  object-store-example.c \
  $(NULL)

test_SOURCES = \
  foo-object-store.c \
  foo-object-store.h \
  foo-test-object.c \
  foo-test-object.h \
  object-store-test.c \
  $(NULL)

PKGFLAGS = `pkg-config --cflags --libs clutter-1.0 mx-1.0`

all: $(PROGRAMS)

clean:
	rm -f $(PROGRAMS)

object-store-example: $(example_SOURCES)
	$(CC) $(CPPLAGS) $(CFLAGS) $(PKGFLAGS) -o $@ $^

object-store-test: $(test_SOURCES)
	$(CC) $(CPPLAGS) $(CFLAGS) $(PKGFLAGS) -o $@ $^

.PHONY: clean
