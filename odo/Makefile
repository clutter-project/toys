LIBS=`pkg-config --libs clutter-0.8`
INCS=`pkg-config --cflags clutter-0.8`

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

all: odo odo-pageturn odo-cloth odo-bowtie odo-slideshow


odo: odo.o clutter-texture-odo.o
	$(CC) -g -Wall $(CFLAGS) -o $@ odo.o clutter-texture-odo.o $(LIBS)

odo-pageturn: odo-pageturn.o clutter-texture-odo.o
	$(CC) -g -Wall $(CFLAGS) -o $@ odo-pageturn.o clutter-texture-odo.o $(LIBS)

odo-cloth: odo-cloth.o clutter-texture-odo.o
	$(CC) -g -Wall $(CFLAGS) -o $@ odo-cloth.o clutter-texture-odo.o $(LIBS)

odo-bowtie: odo-bowtie.o clutter-texture-odo.o
	$(CC) -g -Wall $(CFLAGS) -o $@ odo-bowtie.o clutter-texture-odo.o $(LIBS)

odo-slideshow: odo-slideshow.o clutter-texture-odo.o
	$(CC) -g -Wall $(CFLAGS) -o $@ odo-slideshow.o clutter-texture-odo.o $(LIBS)

clean:
	rm -fr *.o odo odo-pageturn odo-cloth odo-bowtie odo-slideshow
