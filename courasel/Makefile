LIBS=`pkg-config --libs clutter-1.0`
INCS=`pkg-config --cflags clutter-1.0`
CFLAGS="-lm"

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

all: courasel

courasel: courasel.o
	$(CC) -g -Wall $(CFLAGS) -o $@ courasel.o $(LIBS)

clean:
	rm -fr *.o courasel
