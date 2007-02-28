LIBS=`pkg-config --libs clutter-0.2`
INCS=`pkg-config --cflags clutter-0.2`

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

OBJS=opt.o opt-show.o opt-slide.o opt-config.o opt-transition.o opt-menu.o

opt: $(OBJS)
	$(CC) -g -Wall $(CFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -fr *.o opt