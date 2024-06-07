CXX           = g++
CXXFLAGS      = -pipe -O2 -std=gnu++11 -Wall -Wextra
INCPATH       = -I..
LINK          = g++
LFLAGS        = -Wl,-O1
LIBS          = -lpthread -L/opt/SVS/SVCamKit/SDK/Linux64_x64/ -lSVGenSDK64

OBJECTS       = main.o
TARGET        = SVCamMiniExample

all: SVCamMiniExample

SVCamMiniExample:  $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

clean:
	-rm -f $(OBJECTS) $(TARGET)

main.o: ../main.cpp ../sv_gen_sdk.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o main.o ../main.cpp
