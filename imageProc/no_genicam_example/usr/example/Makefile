CC=gcc
CPP=g++

CFLAGS=-g -O0 -Wall -Wunused
BINDIR=.
LDFLAGS=-lsvgige -lpthread
INCFLAGS=

EXECUTABLE=$(BINDIR)/example

all: $(BINDIR) $(EXECUTABLE)

$(BINDIR):
	mkdir $(BINDIR)

$(EXECUTABLE): example.o 
	$(CPP) $(CFLAGS) -o $(EXECUTABLE) example.o  $(LDFLAGS)

clean:
	rm -f *.o *~ $(EXECUTABLE) example
