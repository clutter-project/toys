LIBS=`pkg-config --libs clutter-0.5 gnome-vfs-2.0 clutter-gst-0.5`
INCS=`pkg-config --cflags clutter-0.5 gnome-vfs-2.0 clutter-gst-0.5`

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

all: test


test: test.o clutter-reflect-texture.o
	$(CC) -g -Wall $(CFLAGS) -o $@ test.o clutter-reflect-texture.o $(LIBS)

clean:
	rm -fr *.o test
