LIBS=`pkg-config --libs clutter-0.3 gnome-vfs-2.0 clutter-gst-0.1`
INCS=`pkg-config --cflags clutter-0.3 gnome-vfs-2.0 clutter-gst-0.1`

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

all: test table


test: test.o clutter-reflect-texture.o
	$(CC) -g -Wall $(CFLAGS) -o $@ test.o clutter-reflect-texture.o $(LIBS)

table: table.o clutter-dominatrix.o
	$(CC) -g -Wall $(CFLAGS) -o $@ table.o clutter-dominatrix.o $(LIBS)

clean:
	rm -fr *.o test table