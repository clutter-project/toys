
PROGRAM = object-store-test

SOURCES = \
  foo-object-store.c \
  foo-object-store.h \
  foo-test-object.c \
  foo-test-object.h \
  object-store-test.c \
  $(NULL)

PKGFLAGS = `pkg-config --cflags --libs clutter-1.0 mx-1.0`

all: $(PROGRAM)

clean:
	rm -f $(PROGRAM)

$(PROGRAM): $(SOURCES)
	$(CC) $(CPPLAGS) $(CFLAGS) $(PKGFLAGS) -o $@ $^

.PHONY: clean
