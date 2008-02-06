LIBS=`pkg-config --libs clutter-0.5`
INCS=`pkg-config --cflags clutter-0.5`

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

all: courasel

courasel: courasel.o
	$(CC) -g -Wall $(CFLAGS) -o $@ courasel.o $(LIBS)

clean:
	rm -fr *.o courasel
