LIBS=`pkg-config --libs clutter-0.7`
INCS=`pkg-config --cflags clutter-0.7`

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

all: odo odo-pageturn


odo: odo.o clutter-texture-odo.o
	$(CC) -g -Wall $(CFLAGS) -o $@ odo.o clutter-texture-odo.o $(LIBS)

odo-pageturn: odo-pageturn.o clutter-texture-odo.o
	$(CC) -g -Wall $(CFLAGS) -o $@ odo-pageturn.o clutter-texture-odo.o $(LIBS)

clean:
	rm -fr *.o odo odo-pageturn
