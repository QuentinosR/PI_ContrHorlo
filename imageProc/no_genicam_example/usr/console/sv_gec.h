#ifndef SV_GEC_H
#define SV_GEC_H

#include <iostream>
#include <cstdio>
#include <map>
#include <string>
#include <cstring>
#include <unistd.h>

#define BUFFER_COUNT             10
#define PACKET_SIZE            9000
#define PACKET_RESEND_TIMEOUT  1000

//commands
typedef enum {
    CMD_UNKNOWN = -1,
    CMD_NONE = 0,
    CMD_HELP = 1,
    CMD_QUIT = 2,
    CMD_DEVICE_DISCOVERY = 3,
    CMD_LIST_NIC = 4,
    CMD_FORCE_IP = 5,
    CMD_SET_IP = 6,
    CMD_CONNECT = 7,
    CMD_CLOSE_CONNECTION = 8,
    CMD_LIST = 9,
    CMD_AOI_SIZEX = 10,
    CMD_AOI_SIZEY = 11,
    CMD_AOI_OFFSETX = 12,
    CMD_AOI_OFFSETY = 13,
    CMD_PIXEL_DEPTH = 14,
    CMD_INTER_PACKET_DELAY = 15,
    CMD_STREAM_OPEN = 16,
    CMD_STREAM_CLOSE = 17,
    CMD_START_ACQUISITION = 18,
    CMD_END_ACQUISITION = 19,
    CMD_SOFTWARE_TRIGGER = 20,
    CMD_TRIGGER = 21,
    CMD_FRAMERATE = 22,
    CMD_EXPOSURE = 23,
    CMD_GAIN = 24,
    CMD_AUTOGAIN = 25,
    CMD_STATISTICS = 26,
} COMMAND;

//internal program state
typedef enum {
  STATE_NONE = 0,
  STATE_DEVICE_DISCOVERY = 1,
  STATE_CAMERA_SELECTED = 2,
  STATE_STREAM_OPEN = 3,
  STATE_ACQUISITION = 4,
} STATE;

typedef struct {
    unsigned int iIP;
    unsigned int iIPLocal;
    unsigned long long lMAC;
} CameraData;

#endif // SV_GEC_H
