CXX           = g++
CXXFLAGS      = -pipe -O2 -std=gnu++11 -Wall -Wextra
LINK          = g++
LFLAGS        = -Wl,-O1
LIBS          = -lpthread -L/opt/SVS/SVCamKit/SDK/Linux64_x64/ -lSVGenSDK64
INC_CV        = -I/usr/local/include/opencv2
LIBS_CV       =-lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_stitching -lopencv_imgcodecs

BUILD_FOLDER  = build
SRC_FOLDER    = src
OBJECTS       = imageProc.o
TARGET        = imageProc

all: imageProc

imageProc:  $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(BUILD_FOLDER)/$(OBJECTS) $(LIBS) $(LIBS_CV)

clean:
	-rm -f $(OBJECTS) $(TARGET)

imageProc.o: $(SRC_FOLDER)/main.cpp $(SRC_FOLDER)/sv_gen_sdk.h
	$(CXX) -c $(CXXFLAGS) -o $(BUILD_FOLDER)/$@ $<
