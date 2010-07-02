LIBS=`pkg-config --libs clutter-1.0`
INCS=`pkg-config --cflags clutter-1.0`

QUIET_CC=@echo '  CC     '$@;
QUIET_LD=@echo '  CCLD   '$@;
QUIET_RM=@echo '  RM     '$@;

.c.o:
	$(QUIET_CC)$(CC) -g -Wall --std=c99 $(CFLAGS) $(INCS) -c $*.c

all: arc-clock

arc-clock: arc-clock.o
	$(QUIET_LD)$(CC) -g -Wall --std=c99 $(CFLAGS) -o $@ $< -lm $(LIBS)

clean:
	@rm -fr *.o arc-clock
