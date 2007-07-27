LIBS=`pkg-config --libs clutter-0.3 clutter-cairo-0.3`
INCS=`pkg-config --cflags clutter-0.3 clutter-cairo-0.3`

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

all: pong2


pong2: pong2.o
	$(CC) -g -Wall $(CFLAGS) -o $@ pong2.o $(LIBS)

clean:
	rm -fr *.o pong2
