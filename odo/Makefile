LIBS=`pkg-config --libs clutter-1.0`
INCS=`pkg-config --cflags clutter-1.0`

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

all: odo

odo: odo.o odo-texture.o odo-distort-funcs.o
	$(CC) -g -Wall $(CFLAGS) -o $@ odo.o odo-texture.o odo-distort-funcs.o $(LIBS)

clean:
	rm -f *.o odo
