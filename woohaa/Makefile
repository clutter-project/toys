LIBS=`pkg-config --libs clutter-0.2 gnome-vfs-2.0 clutter-gst-0.1`
INCS=`pkg-config --cflags clutter-0.2 gnome-vfs-2.0 clutter-gst-0.1`
OBJS=woohaa.o clutter-slider-menu.o clutter-video-model.o  clutter-video-view.o clutter-simple-layout.o util.o

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

woohaa: $(OBJS)
	$(CC) -g -Wall $(CFLAGS) -o $@ $(OBJS) $(LIBS)

all: woohaa

clean:
	rm -fr *.o woohaa