LIBS=`pkg-config --libs clutter-0.8 sqlite3`
INCS=`pkg-config --cflags clutter-0.8 sqlite3`

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

all: test-sqlite-model

test-sqlite-model: test-sqlite-model.o clutter-sqlite-model.o
	$(CC) -g -Wall $(CFLAGS) -o $@ test-sqlite-model.o clutter-sqlite-model.o $(LIBS)

clean:
	rm -fr *.o test-sqlite-model
