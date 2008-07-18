LIBS=`pkg-config --libs clutter-0.8`
INCS=`pkg-config --cflags clutter-0.8`

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

all: circles

circles: circles.o
	$(CC) -g -Wall $(CFLAGS) -o $@ circles.o $(LIBS)

clean:
	rm -fr *.o circles
