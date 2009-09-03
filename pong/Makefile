LIBS=`pkg-config --libs clutter-1.0`
INCS=`pkg-config --cflags clutter-1.0`

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

all: pong2


pong2: pong2.o
	$(CC) -g -Wall $(CFLAGS) -o $@ pong2.o $(LIBS)

clean:
	rm -fr *.o pong2
