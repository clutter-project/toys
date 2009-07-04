LIBS=`pkg-config --libs clutter-0.9`
INCS=`pkg-config --cflags clutter-0.9`

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

all: circles

circles: circles.o
	$(CC) -g -Wall $(CFLAGS) -o $@ circles.o $(LIBS)

clean:
	rm -fr *.o circles
