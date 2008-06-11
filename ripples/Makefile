LIBS=`pkg-config --libs clutter-0.7`
INCS=`pkg-config --cflags clutter-0.7`

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

all: ripples

ripples: ripples.o
	$(CC) -g -Wall $(CFLAGS) -o $@ ripples.o $(LIBS)

clean:
	rm -fr *.o ripples
