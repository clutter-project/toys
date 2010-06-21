PKGS   = clutter-1.0 
CFLAGS = -O2 -g -Wall

# uncomment the following to enable gstreamer playback
#CFLAGS = -O2 -g -Wall -DUSE_GST
  #PKGS   = clutter-1.0 clutter-gst-1.0

PROGRAM = pinpoint

LIBS = `pkg-config --libs $(PKGS)`
INCS = `pkg-config --cflags $(PKGS)`

$(PROGRAM): *.c
	gcc $(CFLAGS) $(INCS) $(LIBS) *.c -o $@
clean:
	rm -f $(PROGRAM) *~
