LIBS=`pkg-config --libs clutter-0.4 gnome-vfs-2.0 clutter-gst-0.4`
INCS=`pkg-config --cflags clutter-0.4 gnome-vfs-2.0 clutter-gst-0.4`

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

all: table


table: table.o clutter-dominatrix.o clutter-video-player.o
	$(CC) -g -Wall $(CFLAGS) -o $@ table.o clutter-dominatrix.o clutter-video-player.o $(LIBS)

clean:
	rm -fr *.o table
