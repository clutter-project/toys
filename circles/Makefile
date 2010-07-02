LIBS=`pkg-config --libs clutter-1.0`
INCS=`pkg-config --cflags clutter-1.0`

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

all: circles

circles: circles.o
	$(CC) -g -Wall $(CFLAGS) -o $@ circles.o -lm $(LIBS)

clean:
	rm -fr *.o circles
