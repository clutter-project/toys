LIBS=`pkg-config --libs clutter-0.3`
INCS=`pkg-config --cflags clutter-0.3`

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

all: odo


odo: odo.o clutter-texture-odo.o
	$(CC) -g -Wall $(CFLAGS) -o $@ odo.o clutter-texture-odo.o $(LIBS)

clean:
	rm -fr *.o odo