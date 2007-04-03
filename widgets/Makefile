LIBS=`pkg-config --libs clutter-0.2 gnome-vfs-2.0 clutter-gst-0.1`
INCS=`pkg-config --cflags clutter-0.2 gnome-vfs-2.0 clutter-gst-0.1`

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

all: test


test: test.o clutter-reflect-texture.o
	$(CC) -g -Wall $(CFLAGS) -o $@ test.o clutter-reflect-texture.o $(LIBS)

clean:
	rm -fr *.o test 