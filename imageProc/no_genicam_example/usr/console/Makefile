CC=gcc
CPP=g++

CFLAGS=-g -O2 -Wall -Wunused  
BINDIR=.
LDFLAGS=-lsvgige `pkg-config --libs gtk+-2.0` -lgthread-2.0 -lpthread
INCFLAGS=`pkg-config --cflags gtk+-2.0` 


INCPATH=-I. -I../include 

EXECUTABLE=$(BINDIR)/gec

all: $(BINDIR) $(EXECUTABLE)

$(BINDIR):
	mkdir $(BINDIR)

$(EXECUTABLE): sv_gec.o sv_helper.o sv_window.o sv_debayer.o 
	$(CPP) $(CFLAGS) -o $(EXECUTABLE) sv_gec.o sv_helper.o sv_window.o  sv_debayer.o $(LDFLAGS)

sv_gec.o: sv_gec.cpp sv_gec.h
	$(CPP) $(CFLAGS) $(INCPATH) -c sv_gec.cpp -o $@ $(INCFLAGS)
	
sv_helper.o: sv_helper.cpp sv_helper.h
	$(CPP) $(CFLAGS) $(INCPATH) -c sv_helper.cpp -o $@ $(INCFLAGS)
	
sv_window.o: sv_window.cpp sv_window.h
	$(CPP) $(CFLAGS)  $(INCPATH) -c sv_window.cpp -o $@ $(INCFLAGS)
	
sv_debayer.o: sv_debayer.cpp sv_debayer.h
	$(CPP) $(CFLAGS) $(INCPATH) -c sv_debayer.cpp -o $@ $(INCFLAGS)

clean:
	rm -f *.o *~ $(EXECUTABLE) gec
