LIBS=`pkg-config --libs clutter-0.6`
INCS=`pkg-config --cflags clutter-0.6`

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

all: odo


odo: odo.o clutter-texture-odo.o
	$(CC) -g -Wall $(CFLAGS) -o $@ odo.o clutter-texture-odo.o $(LIBS)

clean:
	rm -fr *.o odo
