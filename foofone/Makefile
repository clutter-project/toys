LIBS=`pkg-config --libs clutter-1.0`
INCS=`pkg-config --cflags clutter-1.0`
CFLAGS="-lm"

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

all: foofone

foofone: foofone.o
	$(CC) -g -Wall $(CFLAGS) -o $@ foofone.o $(LIBS)

clean:
	rm -fr *.o foofone
