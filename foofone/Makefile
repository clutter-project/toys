LIBS=`pkg-config --libs clutter-0.8`
INCS=`pkg-config --cflags clutter-0.8`

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

all: foofone

foofone: foofone.o
	$(CC) -g -Wall $(CFLAGS) -o $@ foofone.o $(LIBS)

clean:
	rm -fr *.o foofone
