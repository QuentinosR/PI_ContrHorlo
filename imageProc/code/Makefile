CXX           = g++
CXXFLAGS      = -pipe -O2 -std=gnu++11 -Wall -Wextra
INCPATH       = -I..
LINK          = g++
LFLAGS        = -Wl,-O1
LIBS          = -lpthread -L/opt/SVS/SVCamKit/SDK/Linux64_x64/ -lSVGenSDK64

BUILD_FOLDER  = build
SRC_FOLDER    = src
OBJECTS       = imageProc.o
TARGET        = imageProc

all: imageProc

imageProc:  $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(BUILD_FOLDER)/$(OBJECTS) $(LIBS)

clean:
	-rm -f $(OBJECTS) $(TARGET)

imageProc.o: $(SRC_FOLDER)/main.cpp $(SRC_FOLDER)/sv_gen_sdk.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(BUILD_FOLDER)/$@ $<
