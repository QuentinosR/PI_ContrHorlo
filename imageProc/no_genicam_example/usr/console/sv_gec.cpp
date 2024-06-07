#include "../include/svgige.h"

#include "sv_gec.h"
#include "sv_helper.h"
#include "sv_window.h"
#include "sv_debayer.h"

//###global variable###
std::map<unsigned int, CameraData> mCamData;
std::map<unsigned int, unsigned int> mCamIP;
std::map<unsigned int, unsigned int> mCamLIP;
std::map<unsigned long long, unsigned int> mCamMAC;

//number of cameras known to the program after device discovery
unsigned int iCamCount;

//number of selected camera
unsigned int iSelectedCam;

// State of image acquisition
bool acquisitionRunning;

// State of image display
bool isDisplayImage;

// Show statistics flag
bool ShowStatistics;

// program synchronization
static bool in_use;

//stores internal program state
STATE program_state;

//handle to camera
Camera_handle camera;

//handle to stream
Stream_handle stream;

//camera IP and port
unsigned int cameraControlIP;

//lokales interface and port to be used
unsigned int localIP;
unsigned short localPort;

//image data
int ImageID;
unsigned int imageWidth;
unsigned int imageHeight;
unsigned int imageSize;
unsigned char *imageBuffer8bit;
SVGIGE_PIXEL_DEPTH svgigePixelDepth;

//data for stream-connection
unsigned int bufferSize;
unsigned int bufferCount;
unsigned int packetSize;
unsigned int packetResendTimeout;

//thread containing the window for stream-display
pthread_t windowThread;


SVGigE_RETURN camCallback(SVGigE_SIGNAL *Signal, void* Context);


void* startStreamOutput(void *name);
void init();
static void get_next_command(void);
static void getinput();
void show_startup();
int cmdEndAcquisition();
int cmdCloseStream();
int cmdCloseCamera();
int getCommand(char *str);
int command_help();
int command_quit();
int command_device_discovery();
bool command_connect(char *arg);
int command_defaults();
int command_stream_open();
int command_start_acquisition();
int command_stream_close();
int command_close_connection();
int command_list_nic();
int command_list();
int command_trigger(char *arg);
int command_software_trigger();
int command_end_acquisition();
int command_exposure(char *arg);
int command_framerate(char *arg);
int command_gain(char *arg);
int command_autogain(char *arg);
int command_aoi_sizex(char *arg);
int command_aoi_sizey(char *arg);
int command_aoi_offsetx(char *arg);
int command_aoi_offsety(char *arg);
int command_inter_packet_delay(char *arg);
int command_force_ip(char* sn_mac, char* ip, char* mask);
int command_set_ip(char* ip, char* mask);

/// main program
/**
 *
 */
int main(int argc, char **argv) {

    init();
    init_helper();

    show_startup();

    in_use = true;
    getinput();

    return 0;
}

void init()
{
    mCamData.clear();
    mCamIP.clear();
    mCamLIP.clear();
    mCamMAC.clear();

    iCamCount = 0;
    iSelectedCam = 0;
    acquisitionRunning = false;
    isDisplayImage = false;
    ShowStatistics = false;
    in_use = false;
    program_state = STATE_NONE;
    camera = SVGigE_NO_CAMERA;
    stream = SVGigE_NO_STREAMING_CHANNEL;
    cameraControlIP = 0;
    localIP = 0;
    localPort = 0;
    ImageID =0;
    imageWidth = 0;
    imageHeight = 0;
    imageSize = 0;
    imageBuffer8bit = NULL;
    svgigePixelDepth = SVGIGE_PIXEL_DEPTH_UNKNOWN;


    bufferSize = 0;
    bufferCount = 0;
    packetSize = 0;
    packetResendTimeout = 0;
}

/// process command
/**
 * reads the command (and its arguents, if any) from command line and parses
 * it. Depending on the current state of the application the command is
 * processed.
 */
static void get_next_command(void)
{
    char *ptr;
    char *cmdstr = NULL;
    char *arg = NULL;
    char *arg2 = NULL;
    char *arg3 = NULL;
    //char *arg4 = NULL;
    char str[64];

    //read command-line
    fgets(str, 64, stdin);
    ptr = &str[0];
    //strip command-line
    remove_newline(str);

    //split command-line to command and arguments
    char separator[] = " ";
    cmdstr = strtok(str, separator);
    arg = strtok(NULL, separator);
    arg2 = strtok(NULL, separator);
    arg3 = strtok(NULL, separator);
    //arg4 = strtok(NULL, separator);

    // Determine command code
    int cmd = getCommand(cmdstr);

    //process command
    if (ptr == NULL)
    {
        printf(" NULL\n");
        //in_use = FALSE;
    }
    else
    {
        if (cmd == CMD_NONE)
        {
            //do nothing
        }
        else if (cmd == CMD_UNKNOWN)
        {
            printf(" invalid command\n");
        }
        else if (cmd == CMD_HELP)
        {
            command_help();
        }
        else if (cmd == CMD_QUIT)
        {
            command_quit();
            in_use = false;
        }
        else
        {
            if (program_state == STATE_NONE)
            {
                if (cmd == CMD_DEVICE_DISCOVERY)
                {
                    //Device Discovery
                    command_device_discovery();

                    if (iCamCount > 0)
                        program_state = STATE_DEVICE_DISCOVERY;
                }
                else
                    if (cmd == CMD_FORCE_IP)
                {
                        //Force IP
                    command_force_ip(arg, arg2, arg3);
                }
                else
                    if (cmd == CMD_LIST_NIC)
                {
                    //List network interfaces
                    command_list_nic();
                }
                else
                {
                    printf(" command not allowed in this state\n");
                    if (iCamCount == 0)
                        printf(" reason: no cameras known, use \"dd\" command to discover cameras\n");
                }
            }
            else if (program_state == STATE_DEVICE_DISCOVERY)
            {
                switch (cmd)
                {
                    case CMD_DEVICE_DISCOVERY:
                        //Device Discovery
                        command_device_discovery();
                        break;
                    case CMD_LIST_NIC:
                        //List network interfaces
                        command_list_nic();
                        break;
                    case CMD_FORCE_IP:
                        //Force IP
                        command_force_ip(arg, arg2, arg3);
                        break;
                    case CMD_CONNECT:
                        //Connect
                        if (arg != NULL)
                        {
                            if (command_connect(arg) == true)
                            {
                                program_state = STATE_CAMERA_SELECTED;
                            }
                        }
                        else
                            printf(" ERROR: argument is missing\n");
                        break;
                    default:
                        printf(" command not allowed in this state\n");
                        printf(" reason: not connected to a camera\n");
                }

            }
            else if (program_state == STATE_CAMERA_SELECTED)
            {
                switch (cmd)
                {
                    case CMD_CLOSE_CONNECTION:
                        //Close Connection
                        command_close_connection();
                        program_state = STATE_DEVICE_DISCOVERY;
                        break;
                    case CMD_SET_IP:
                        //Set IP
                        command_set_ip(arg, arg2);
                        break;
                    case CMD_LIST:
                        //List
                        command_list();
                        break;
                    case CMD_STREAM_OPEN:
                        //Stream Open
                        if( SVGigE_SUCCESS == command_stream_open() )
                        program_state = STATE_STREAM_OPEN;
                        break;
                    case CMD_AOI_SIZEX:
                        if (arg != NULL)
                            command_aoi_sizex(arg);
                        else
                            printf(" ERROR: argument is missing\n");
                        break;
                    case CMD_AOI_SIZEY:
                        if (arg != NULL)
                            command_aoi_sizey(arg);
                        else
                            printf(" ERROR: argument is missing\n");
                        break;
                    case CMD_AOI_OFFSETX:
                        if (arg != NULL)
                            command_aoi_offsetx(arg);
                        else
                            printf(" ERROR: argument is missing\n");
                        break;
                    case CMD_AOI_OFFSETY:
                        if (arg != NULL)
                            command_aoi_offsety(arg);
                        else
                            printf(" ERROR: argument is missing\n");
                        break;
                    case CMD_INTER_PACKET_DELAY:
                        if (arg != NULL)
                            command_inter_packet_delay(arg);
                        else
                            printf(" ERROR: argument is missing\n");
                        break;
                    case CMD_TRIGGER:
                        if (arg != NULL)
                            command_trigger(arg);
                        else
                            printf(" ERROR: argument is missing\n");
                        break;
                    case CMD_FRAMERATE:
                        //Framerate
                        if (arg != NULL)
                            command_framerate(arg);
                        else
                            printf(" ERROR: argument is missing\n");
                        break;
                    case CMD_EXPOSURE:
                        //Exposure
                        if (arg != NULL)
                            command_exposure(arg);
                        else
                            printf(" ERROR: argument is missing\n");
                        break;
                    case CMD_GAIN:
                        //Gain
                        if (arg != NULL)
                            command_gain(arg);
                        else
                            printf(" ERROR: argument is missing\n");
                        break;
                    case CMD_AUTOGAIN:
                        //Auto gain
                        if (arg != NULL)
                            command_autogain(arg);
                        else
                            printf(" ERROR: argument is missing\n");
                        break;

                    default:
                        printf(" command not allowed in this state\n");
                }

            }
            else if (program_state == STATE_STREAM_OPEN)
            {
                switch (cmd)
                {
                    case CMD_CLOSE_CONNECTION:
                        //Stream Close
                        command_stream_close();
                        //Close Connection
                        command_close_connection();
                        program_state = STATE_DEVICE_DISCOVERY;
                        break;
                    case CMD_STREAM_CLOSE:
                        //Stream Close
                        command_stream_close();
                        program_state = STATE_CAMERA_SELECTED;
                        break;
                    case CMD_LIST:
                        //List
                        command_list();
                        break;
                    case CMD_STATISTICS:
                        // Statistics
                        ShowStatistics = ShowStatistics? false : true;
                        break;
                    case CMD_START_ACQUISITION:
                        //Start Acquisition
                        command_start_acquisition();
                        program_state = STATE_ACQUISITION;
                        break;
                    case CMD_TRIGGER:
                        if (arg != NULL)
                            command_trigger(arg);
                        else
                            printf(" ERROR: argument is missing\n");
                        break;
                    case CMD_FRAMERATE:
                        //Framerate
                        if (arg != NULL)
                            command_framerate(arg);
                        else
                            printf(" ERROR: argument is missing\n");
                        break;
                    case CMD_EXPOSURE:
                        //Exposure
                        if (arg != NULL)
                            command_exposure(arg);
                        else
                            printf(" ERROR: argument is missing\n");
                        break;
                    case CMD_GAIN:
                        //Gain
                        if (arg != NULL)
                            command_gain(arg);
                        else
                            printf(" ERROR: argument is missing\n");
                        break;
                    case CMD_AUTOGAIN:
                        //Auto gain
                        if (arg != NULL)
                            command_autogain(arg);
                        else
                            printf(" ERROR: argument is missing\n");
                        break;
                    default:
                        printf(" command not allowed in this state\n");
                }
            }
            else if (program_state == STATE_ACQUISITION)
            {
                switch (cmd)
                {
                    case CMD_CLOSE_CONNECTION:
                        //End Acqusisition
                        command_end_acquisition();
                        //Stream Close
                        command_stream_close();
                        //Close Connection
                        command_close_connection();
                        program_state = STATE_DEVICE_DISCOVERY;
                        break;
                    case CMD_STREAM_CLOSE:
                        //End Acquisition
                        command_end_acquisition();
                        //Stream Close
                        command_stream_close();
                        program_state = STATE_CAMERA_SELECTED;
                        break;
                    case CMD_END_ACQUISITION:
                        //End Acquisition
                        command_end_acquisition();
                        program_state = STATE_STREAM_OPEN;
                        break;
                    case CMD_LIST:
                        //List
                        command_list();
                        break;
                     case CMD_STATISTICS:
                        // Statistics
                        ShowStatistics = ShowStatistics? false : true;
                        break;
                    case CMD_TRIGGER:
                        if (arg != NULL)
                            command_trigger(arg);
                        else
                            printf(" ERROR: argument is missing\n");
                        break;
                    case CMD_SOFTWARE_TRIGGER:
                        //Softwaretrigger
                        command_software_trigger();
                        break;
                    case CMD_FRAMERATE:
                        //Framerate
                        if (arg != NULL)
                            command_framerate(arg);
                        else
                            printf(" ERROR: argument is missing\n");
                        break;
                    case CMD_EXPOSURE:
                        //Exposure
                        if (arg != NULL)
                            command_exposure(arg);
                        else
                            printf(" ERROR: argument is missing\n");
                        break;
                    case CMD_GAIN:
                        //Gain
                        if (arg != NULL)
                            command_gain(arg);
                        else
                            printf(" ERROR: argument is missing\n");
                        break;
                    case CMD_AUTOGAIN:
                        //Auto gain
                        if (arg != NULL)
                            command_autogain(arg);
                        else
                            printf(" ERROR: argument is missing\n");
                        break;
                    default:
                        printf(" command not allowed in this state\n");
                }

            }
        }
    }

}

/// read new command
/**
 */
static void getinput()
{
    while (in_use)
    {
        get_next_command();
    }
}

/// show startup message
/**
 */
void show_startup()
{
    printf("\nInteractive SVS GigE Linux SDK command line utility\n");
    printf("--\n");

    //Device Discovery
    command_device_discovery();

    if (iCamCount > 0)
        program_state = STATE_DEVICE_DISCOVERY;

    printf("--\n");
    printf("press 'h' for the list of commands\n");
    printf("\n");
}

/// perform device discovery
/**
 *
 * \return
 */
int command_device_discovery()
{
    SVGigE_RETURN result;

    CameraData camData;
    char *szMAC;
    char *szIP;

    //reset old lists
    mCamData.clear();
    mCamIP.clear();
    mCamLIP.clear();
    mCamMAC.clear();

    iCamCount = 0;

    #define MAX_NETWORK_COUNT	  16
    unsigned int networks[MAX_NETWORK_COUNT];
    unsigned int networkCount = 0;
    unsigned int networkLocal = 0;

    //find all local network interfaces
    printf(" Searching network for available cameras...");
    result = findNetworkAdapters((unsigned int*) &networks, MAX_NETWORK_COUNT);

    if (result == SVGigE_SUCCESS)
    {
        for (networkCount = 0; networkCount < MAX_NETWORK_COUNT; networkCount++)
        {
            if (networks[networkCount] == 0x7F000001 )
        networkLocal = 1;
            if (networks[networkCount] == 0)
                break;
        }

        printf("\n\n Number of Network Adapters: %d\n", networkCount - networkLocal);

    }
    else
  {
        // Verify find Adapter Result
        printf("\n\n ERROR: Network access not available (socket problem).\n\n");
        return -1;
  }


    //perform device discovery on each local network interface
  for (unsigned int i = 0; i < networkCount; i++)
  {
    result = discoverCameras(networks[i], 1000, camCallback, NULL);

      if (result == SVGigE_SUCCESS)
      {

          //printf("\n\n camera %d found !!  \n\n", i);
      }
  }


    //print list of cameras found
    if (mCamData.size() > 0)
    {
        printf("\n   #      MAC Address        IP Address\n");
        for (unsigned int i = 1; i <= mCamData.size(); i++)
        {
            camData = mCamData[i];
            szMAC = intToMAC(camData.lMAC);
            szIP = intToIP(camData.iIP);

            printf("  %2.0d   %17.17s   %s\n",i,szMAC,szIP);
            free(szMAC);
            free(szIP);
        }
    }
    else
  {
    if( networkCount > 0 )
    {
          printf(" no cameras found in subnets:\n");

          for(unsigned int i = 0; i < networkCount; i++){
        if( networks[i] != 0x7F000001 )
              printf("   %d.%d.%d.%d\n", (networks[i]>>24)&0xFF, (networks[i]>>16)&0xFF, (networks[i]>>8)&0xFF, networks[i]&0xFF);
          }
      printf(" use 'force IP' (command: fi) to get a camera to respective subnet!\n");
    }
    else
    {
      printf(" no network interfaces available!\n\n");
    }
  }

    return 0;
}

/// callback from camera
/**
 * \return return code
 * \param Signal signal structure
 * \param Context context
 */
SVGigE_RETURN camCallback(SVGigE_SIGNAL *Signal, void* Context)
{
    if (Signal->SignalType == SVGigE_SIGNAL_FRAME_COMPLETED)
    {
        SVGigE_IMAGE *vData = (SVGigE_IMAGE *)Signal->Data;
        ImageID = vData->ImageID;

        // Run image conversion if needed
        GVSP_PIXEL_TYPE pixelType = vData->PixelType;
        int imgWidth = vData->ImageWidth;
        int imgHeight = vData->ImageHeight;
        int imgSize = vData->ImageSize;

        unsigned char *imageBuffer = NULL;
        unsigned char *imageBuffer01 = NULL;

        if(pixelType == GVSP_PIX_MONO8)
        {
            imageBuffer = new unsigned char [imgSize];
            memcpy(imageBuffer, vData->ImageData, imgSize);
        }
        else if (pixelType == GVSP_PIX_MONO12_PACKED)
        {
            imageBuffer = new unsigned char [imgSize];
            Image_getImage12bitAs8bit(vData->ImageData,imgWidth,imgHeight,pixelType,imageBuffer,imgSize);

        }
        else if(pixelType == GVSP_PIX_BAYGR8 || pixelType == GVSP_PIX_BAYRG8 || pixelType == GVSP_PIX_BAYGB8 || pixelType == GVSP_PIX_BAYBG8)
        {
            imageBuffer = new unsigned char [imgSize*3];
            simple(vData->ImageData, imageBuffer, imgWidth, imgHeight, pixelType);
        }
        else if ( pixelType == GVSP_PIX_BAYGR12_PACKED || pixelType == GVSP_PIX_BAYRG12_PACKED ||
                  pixelType == GVSP_PIX_BAYGB12_PACKED || pixelType == GVSP_PIX_BAYBG12_PACKED )
            {
                imageBuffer01 = new unsigned char [imgSize];
                Image_getImage12bitAs8bit(vData->ImageData,imgWidth,imgHeight,pixelType,imageBuffer01,imgSize);

                imageBuffer = new unsigned char [imgSize*3];
                simple(imageBuffer01, imageBuffer, imgWidth, imgHeight, pixelType);
            }

        //let the stream-window know about the buffer
        setBuffer(imageBuffer, imageWidth, imageHeight);

        //if the stream-window is open...
        if( displayWindowOpen() == true && isDisplayImage == false )
        {
            // draw content of current buffer (if previous drawing has finished
            isDisplayImage = true;

            drawImage();

            if ( pixelType == GVSP_PIX_MONO8 || pixelType == GVSP_PIX_MONO12_PACKED||
                pixelType == GVSP_PIX_BAYGR8 || pixelType == GVSP_PIX_BAYRG8 ||
                pixelType == GVSP_PIX_BAYGB8 || pixelType == GVSP_PIX_BAYBG8 )
             {
                 delete [] imageBuffer;
             }

             if ( pixelType == GVSP_PIX_BAYGR12_PACKED || pixelType == GVSP_PIX_BAYRG12_PACKED ||
                  pixelType == GVSP_PIX_BAYGB12_PACKED || pixelType == GVSP_PIX_BAYBG12_PACKED )
             {
                delete [] imageBuffer01;
                delete [] imageBuffer;
             }

            isDisplayImage = false;
        }
        else
        {
            //... else stop image acquisition
            program_state = STATE_CAMERA_SELECTED;
            acquisitionRunning = false;
            setImageAcquisitionRunning(acquisitionRunning);
            return enableStream(stream, false);
        }

        // Show statistics
        if( ShowStatistics )
        {
            int FrameLoss;
            float FrameRate;
            float DataRate;
            double Timestamp;
            int totalPacketResend;

            // Get statistics
            StreamingChannel_getFrameLoss(stream,&FrameLoss);
            StreamingChannel_getActualFrameRate(stream,&FrameRate);
            StreamingChannel_getActualDataRate(stream,&DataRate);
            StreamingChannel_getTotalPacketResend(stream, &totalPacketResend);

            Timestamp = vData->Timestamp;
            printf("\r t=%7.2f  ID=%d  frame loss: %d  frame rate: %3.1f fps  data rate: %3.1f MB/s  ",Timestamp,ImageID,FrameLoss,FrameRate,DataRate);
        }

    }
    else
    if (Signal->SignalType == SVGigE_SIGNAL_FRAME_ABANDONED)
    {
        // bandwidth problems

        // Show statistics
        if( ShowStatistics )
        {
            int FrameLoss;
            float FrameRate;
            float DataRate;
            double Timestamp;
            int totalPacketResend;

          //get image buffer
          SVGigE_IMAGE *vData = (SVGigE_IMAGE *)Signal->Data;

            // Get statistics
            StreamingChannel_getFrameLoss(stream,&FrameLoss);
            StreamingChannel_getActualFrameRate(stream,&FrameRate);
            StreamingChannel_getActualDataRate(stream,&DataRate);
            StreamingChannel_getTotalPacketResend(stream, &totalPacketResend);
            Timestamp = vData->Timestamp;
            printf("\r t=%7.2f  ID=%d  frame loss: %d  frame rate: %3.1f fps  data rate: %3.1f MB/s  ",Timestamp,ImageID,FrameLoss,FrameRate,DataRate);
        }

    }
    else
    if (Signal->SignalType == SVGigE_SIGNAL_CAMERA_FOUND)
    {
    SVGigE_CAMERA* camInfo = (SVGigE_CAMERA*) Signal->Data;

    CameraData camData;

        iCamCount++;
        camData.lMAC = ((unsigned long long) camInfo->macHigh << 32) + ((unsigned long long) camInfo->macLow);
        camData.iIP = camInfo->controlIP;
        camData.iIPLocal = camInfo->localIP;

        //store connect-informations about camera
        mCamLIP[camData.iIP] = camInfo->localIP;
        mCamData[iCamCount] = camData;
        mCamIP[camData.iIP] = iCamCount;
        mCamMAC[camData.lMAC] = iCamCount;
    }
    else
    {
    // Unknown signal
    }

    fflush(stdout);

    return SVGigE_SUCCESS;
}
/// translate command-string into an internal command-value
/**
 *
 * \return command
 * \param str command value
 */
int getCommand(char *str)
{
    if (str == NULL)
    {
        return CMD_NONE;
    }
    else if (strcmp(str, "h") == 0)
    {
        return CMD_HELP;
    }
    else if (strcmp(str, "dd") == 0)
    {
        return CMD_DEVICE_DISCOVERY;
    }
    else if (strcmp(str, "ni") == 0)
    {
        return CMD_LIST_NIC;
    }
    else if (strcmp(str, "fi") == 0)
    {
        return CMD_FORCE_IP;
    }
    else if (strcmp(str, "si") == 0)
    {
        return CMD_SET_IP;
    }
    else if (strcmp(str, "co") == 0)
    {
        return CMD_CONNECT;
    }
    else if (strcmp(str, "cc") == 0)
    {
        return CMD_CLOSE_CONNECTION;
    }
    else if (strcmp(str, "ls") == 0)
    {
        return CMD_LIST;
    }
    else if (strcmp(str, "sx") == 0)
    {
        return CMD_AOI_SIZEX;
    }
    else if (strcmp(str, "sy") == 0)
    {
        return CMD_AOI_SIZEY;
    }
    else if (strcmp(str, "ox") == 0)
    {
        return CMD_AOI_OFFSETX;
    }
    else if (strcmp(str, "oy") == 0)
    {
        return CMD_AOI_OFFSETY;
    }
    else if (strcmp(str, "dl") == 0)
    {
        return CMD_INTER_PACKET_DELAY;
    }
    else if (strcmp(str, "so") == 0)
    {
        return CMD_STREAM_OPEN;
    }
    else if (strcmp(str, "sc") == 0)
    {
        return CMD_STREAM_CLOSE;
    }
    else if (strcmp(str, "sa") == 0)
    {
        return CMD_START_ACQUISITION;
    }
    else if (strcmp(str, "ea") == 0)
    {
        return CMD_END_ACQUISITION;
    }
    else if (strcmp(str, "tr") == 0)
    {
        return CMD_TRIGGER;
    }
    else if (strcmp(str, "st") == 0)
    {
        return CMD_SOFTWARE_TRIGGER;
    }
    else if (strcmp(str, "stat") == 0)
    {
        return CMD_STATISTICS;
    }
    else if (strcmp(str, "fr") == 0)
    {
        return CMD_FRAMERATE;
    }
    else if (strcmp(str, "ex") == 0)
    {
        return CMD_EXPOSURE;
    }
    else if (strcmp(str, "gn") == 0)
    {
        return CMD_GAIN;
    }
    else if (strcmp(str, "q") == 0)
    {
        return CMD_QUIT;
    }
    else
    {
        return CMD_UNKNOWN;
    }
}

/// print command list
/**
 * print help (list of commands)
 *
 * \return
 */
int command_help()
{
    printf("\n");

    printHelpLine("ag 100         		","auto gain");
    printHelpLine("cc             		","close connection");
    printHelpLine("co <id>        		","connect (by ID)");
    printHelpLine("dd             		","device discovery");
    printHelpLine("dl             		","inter-packet delay");
    printHelpLine("ea             		","end acquisition");
    printHelpLine("ex 40.0        		","exposure [ms]");
    printHelpLine("fi <mac><ip><sm>","force IP (MAC address, IP address, subnet mask)");
    printHelpLine("fr 25.0        		","framerate (for fixed frequency) [fps]");
    printHelpLine("ga 1.0 1.0 0   		","gamma, digital gain, offset (no arg = gamma off");
    printHelpLine("gn 0.0         		","gain [dB] (auto gain = off)");
    printHelpLine("h              	        ","help");
    printHelpLine("ls             		","list");
    printHelpLine("ox             		","AOI offset X");
    printHelpLine("oy             		","AOI offset Y");
    printHelpLine("q              		","quit");
    printHelpLine("ni             		","network interfaces");
    printHelpLine("sa             		","start acquisition");
    printHelpLine("sc             		","stream close");
    printHelpLine("si <ip><sm>    		","set IP (IP address or 'off',subnet mask)");
    printHelpLine("so             		","stream open");
    printHelpLine("st             		","start acquisition (for software trigger)");
    printHelpLine("stat           		","toggle statistics on/off");
    printHelpLine("sx             		","AOI width");
    printHelpLine("sy             		","AOI height");
    printHelpLine("tp 0 | 1           		","trigger polarity [ 0 | 1]");
    printHelpLine("tr 0           		","fixed frequency (free running)");
    printHelpLine("tr 1           		","internal trigger (software)");
    printHelpLine("tr 2           		","external trigger, internal exposure");
    printHelpLine("tr 3           		","external trigger, external exposure");

    printf("\n");
    return 0;
}

/// quit program
/**
 * close connections, destroy objects, ...
 *
 * \return
 */
int command_quit()
{
    cmdEndAcquisition();
    cmdCloseStream();
    cmdCloseCamera();

    // Close camera info container
    shutdownCameras();

    // Delete image buffer
    if( imageBuffer8bit)
        free(imageBuffer8bit);

  printf(" Quit...\n");

    return 0;
}


/// close camera handle
/**
 * \return
 */
int cmdCloseCamera()
{
    SVGigE_RETURN status = SVGigE_SUCCESS;

    if (camera != SVGigE_NO_CAMERA)
    {
      printf(" Closing camera... ");
      fflush(stdout);

        status = closeCamera(camera);

        if (status == SVGigE_SUCCESS)
        printf("success\n");
        else
        printf("failed\n");

      fflush(stdout);
    }

    return errorHandling(status);
}

/// close stream handle
/**
 * \return
 */
int cmdCloseStream()
{
    SVGigE_RETURN status = SVGigE_SUCCESS;

    if (stream != SVGigE_NO_STREAMING_CHANNEL)
    {
      printf(" Closing stream... ");
        status = closeStream(stream);
        stream = SVGigE_NO_STREAMING_CHANNEL;

        if (status == SVGigE_SUCCESS)
        printf("success\n");
        else
        printf("failed\n");

      fflush(stdout);
    }

    return errorHandling(status);
}

/// end image acquisition
/**
 * \return
 */
int cmdEndAcquisition()
{
    SVGigE_RETURN status = SVGigE_SUCCESS;

    if ((stream != SVGigE_NO_STREAMING_CHANNEL) && (acquisitionRunning == true))
    {
      printf(" Disabling acquisition... ");
      status = Camera_setAcquisitionControl(camera,ACQUISITION_CONTROL_STOP);

        if (status == SVGigE_SUCCESS)
            printf("success\n");
        else
        printf("failed\n");

        printf(" Disabling stream... ");
        acquisitionRunning = false;
        setImageAcquisitionRunning(acquisitionRunning);

        if (status == SVGigE_SUCCESS)
        {
        printf("success\n");
        }
        else
        {
        printf("failed\n");
      }
      fflush(stdout);
    }

    return errorHandling(status);
}

/// connect to a camera
/**
 * \return
 * \param arg number/MAC/IP of camera
 */
bool command_connect(char *arg)
{
    SVGigE_RETURN result;

    if (matchMAC(arg) == true)
    {
        //probably MAC address (aa:bb:cc:dd:ee:ff)
        unsigned long long lMAC = strMACToLongLong(arg);
        if (mCamMAC.find(lMAC) == mCamMAC.end())
        {
            printf(" ERROR: MAC does not belong to a camera\n");
            return false;
        }
        iSelectedCam = mCamMAC[lMAC];
        cameraControlIP = mCamData[mCamMAC[lMAC]].iIP;
    }
    else
    if (matchIP(arg) == true)
    {
        //probably IP address (aaa.bbb.ccc.ddd)
        unsigned int iIP = strIPToInt(arg);
        if (mCamIP.find(iIP) == mCamIP.end())
        {
            printf(" ERROR: IP does not belong to a camera\n");
            return false;
        }
        iSelectedCam = mCamIP[iIP];
        cameraControlIP = mCamData[mCamIP[iIP]].iIP;;
    }
    else
    if (matchInt(arg) == true)
    {
        //probably list number
        iSelectedCam = atoi(arg);
        cameraControlIP = mCamData[atoi(arg)].iIP;
    }

    if (cameraControlIP != 0)
    {
        //get local IP
        localIP = mCamLIP[cameraControlIP];

        // Open a connection to camera,
        // a heartbeat [ms] keeps the connection open
    #define CAMERA_HEARTBEAT  3000
        printf(" Connecting to camera... ");
        result = openCamera(&camera, cameraControlIP, localIP, CAMERA_HEARTBEAT, MULTICAST_MODE_NONE);

    // Retry opening in case of failure
    int trials = 2;
        while( result!=SVGigE_SUCCESS )
    {
      printf("\n %s",getErrorMessage(result));

      if( trials-- <= 0 )
        break;

      printf("\n Retry connecting to camera... ");
      result = openCamera(&camera, cameraControlIP, localIP, CAMERA_HEARTBEAT, MULTICAST_MODE_NONE);
      usleep(500000);
    }

        if( result==SVGigE_SUCCESS )
        {
            // Adjust camera to default values
            command_defaults();
            printf("success\n");
            return true;
        }

        printf("failed\n");
        return false;
    }
    else
    {
        printf(" camera index #%d not found\n",iSelectedCam);
        return false;
    }
}


/// set some default settings
/**
 *
 * \return
 */
int command_defaults()
{
    SVGigE_RETURN Ret;

    // Stop image acquisition
    if( SVGigE_SUCCESS != (Ret = Camera_setAcquisitionMode(camera, ACQUISITION_MODE_NO_ACQUISITION,false)) )
        return errorHandling(Ret);

    // Determine image size and pixel depth
    if( SVGigE_SUCCESS != (Ret = Camera_getSizeX(camera,&imageWidth)) )
        return Ret;
    if( SVGigE_SUCCESS != (Ret = Camera_getSizeY(camera,&imageHeight)) )
        return Ret;
    if( SVGigE_SUCCESS != (Ret = Camera_getPixelDepth(camera,&svgigePixelDepth)) )
        return Ret;

    // Adjust a default frame rate
    if( SVGigE_SUCCESS != (Ret = Camera_setFrameRate(camera,5.0)) )
        return errorHandling(Ret);

    // All settings have been performed successfully
    return SVGigE_SUCCESS;
}

/// open stream and start acquisition
/**
 * \return
 */
int command_stream_open()
{
    SVGigE_RETURN result;

    packetResendTimeout = PACKET_RESEND_TIMEOUT;
    bufferCount = BUFFER_COUNT;
    packetSize = PACKET_SIZE;

    GVSP_PIXEL_TYPE pixelType;

    Camera_getPixelType(camera,&pixelType);
    Camera_getBufferSize(camera,&bufferSize);

    printf(" open stream... ");
    // Create a streaming port using a pre-selected limit for port number
#define LOCAL_STREAMING_PORT_MAX 1500
    result = addStreamExt(camera, &stream,
                &localIP, &localPort, LOCAL_STREAMING_PORT_MAX,
                bufferSize, bufferCount, packetSize, packetResendTimeout,
                camCallback, NULL);

    if (result == SVGigE_SUCCESS)
    {
        printf(" success\n");
    }
    else
    {
        printf(" failed\n");
        return errorHandling(result);
    }


    // Adjust image format
    if (pixelType == GVSP_PIX_MONO8)
    {
        setImageFormat(false);
    }
    else if (pixelType == GVSP_PIX_MONO12_PACKED)
    {
        setImageFormat(false);
    }
    else if (
        pixelType == GVSP_PIX_BAYGR8 ||
        pixelType == GVSP_PIX_BAYRG8 ||
        pixelType == GVSP_PIX_BAYGB8 ||
        pixelType == GVSP_PIX_BAYBG8)
    {
        setImageFormat(true);
    }
    else if (
             pixelType == GVSP_PIX_BAYGR12_PACKED ||
             pixelType == GVSP_PIX_BAYRG12_PACKED ||
             pixelType == GVSP_PIX_BAYGB12_PACKED ||
             pixelType == GVSP_PIX_BAYBG12_PACKED )
    {
        setImageFormat(true);
    }


    printf(" open window\n");
    if (displayWindowOpen() == false)
    {
        if(pthread_create(&windowThread, NULL, &startStreamOutput, NULL) != 0)
        printf(" error while creating thread\n");
        else
        {
        // thread created
        }
    }
    else
        printf(" window already open\n");

    //wait some time for opening the window
    usleep(200000);

    printf(" enabling stream... ");
    if (result == SVGigE_SUCCESS)
    {
        acquisitionRunning = true;
        setImageAcquisitionRunning(acquisitionRunning);
        result = enableStream(stream, true);
        if (result == SVGigE_SUCCESS)
            printf("success\n");
        else
        {
          acquisitionRunning = true;
          setImageAcquisitionRunning(acquisitionRunning);
            printf("failed\n");
        }
    }

    return errorHandling(result);
}

/// open window for camera output
/**
 * \return
 */
void* startStreamOutput(void *name)
{
    // Open a graphical window with actual image size
    openWindow(0,NULL, imageWidth, imageHeight);
    printf(" window was closed\n");
    pthread_exit((void *) 0);

    return NULL;
}


/// start frame acquisition
/**
 *
 * \return
 */
int command_start_acquisition()
{
    SVGigE_RETURN result;

    // Start image acquisition
    printf(" Enabling acquisition... ");
    result = Camera_setAcquisitionControl(camera, ACQUISITION_CONTROL_START);

    if (result != SVGigE_SUCCESS)
    {
        printf(" failed\n");
        return errorHandling(result);
    }
    else
    {
        printf(" success\n");
        acquisitionRunning = true;
        setImageAcquisitionRunning(acquisitionRunning);
    }

    return 0;
}


/// close stream
/**
 * \return
 */
int command_stream_close()
{
    cmdEndAcquisition();
    cmdCloseStream();

    return 0;
}

/// disconnect from camera
/**
 * \return
 */
int command_close_connection()
{
    cmdEndAcquisition();
    cmdCloseStream();
    cmdCloseCamera();

    return 0;
}

/// List network interfaces
/**
 * \return
 */
int command_list_nic()
{
    unsigned int networks[MAX_NETWORK_COUNT];

    // List available network interfaces
    printf(" Network interfaces:\n");
    if( SVGigE_SUCCESS == findNetworkAdapters((unsigned int*) &networks, MAX_NETWORK_COUNT) )
        for(int i = 0; i < MAX_NETWORK_COUNT; i++)
        {
            if( networks[i]==0 )
            {
                if( i==0 )
                    printf(" -> no network interfaces found\n");
                break;
            }

            printf("   %d.%d.%d.%d\n", (networks[i]>>24)&0xFF, (networks[i]>>16)&0xFF, (networks[i]>>8)&0xFF, networks[i]&0xFF);
        }

    return	SVGigE_SUCCESS;
}

/// list camera informations
/**
 *
 * \return
 */

int command_list()
{
    unsigned int imager_sizex;
    unsigned int imager_sizey;

    unsigned int aoi_sizex;
    unsigned int aoi_sizey;
    unsigned int aoi_offsetx;
    unsigned int aoi_offsety;

    float inter_packet_delay;
    float maxframerate;
    float framerate;
    float maxexposure;
    float maxgain;
    SVGIGE_PIXEL_DEPTH svgige_pixel_depth;
    int pixel_depth;
    unsigned int aoir_sizex_min;
    unsigned int aoir_sizey_min;
    unsigned int aoir_sizex_max;
    unsigned int aoir_sizey_max;
    unsigned int aoi_sizex_increment;
    unsigned int aoi_sizey_increment;
    unsigned int aoi_offsetx_increment;
    unsigned int aoi_offsety_increment;

    printf(" Model name/version  : %s\n", Camera_getModelName(camera));
    printf(" Manufacturer name   : %s\n", Camera_getManufacturerName(camera));
    printf(" Specific information: %s\n", Camera_getManufacturerSpecificInformation(camera));
    printf(" MAC address         : %s\n", Camera_getMacAddress(camera));
    printf(" IP  address         : %s\n", Camera_getIPAddress(camera));

    if( SVGigE_SUCCESS == Camera_getImagerWidth(camera,&imager_sizex) )
        if( SVGigE_SUCCESS == Camera_getImagerHeight(camera,&imager_sizey) )
            printf(" Imager size         : %dx%d\n", imager_sizex,imager_sizey);

    if( SVGigE_SUCCESS == Camera_getAreaOfInterest(camera,&aoi_sizex,&aoi_sizey,&aoi_offsetx,&aoi_offsety) )
    {
        printf(" AOI size            : %dx%d\n", aoi_sizex,aoi_sizey);
        printf(" AOI offset          : %dx%d\n", aoi_offsetx,aoi_offsety);
    }
    if( SVGigE_SUCCESS == Camera_getAreaOfInterestRange(camera,&aoir_sizex_min,&aoir_sizey_min,&aoir_sizex_max,&aoir_sizey_max) )
    {
        printf(" AOIR min size       : %dx%d\n", aoir_sizex_min,aoir_sizey_min);
        printf(" AOIR max size       : %dx%d\n", aoir_sizex_max,aoir_sizey_max);
    }

    if( SVGigE_SUCCESS == Camera_getAreaOfInterestIncrement(camera, &aoi_sizex_increment, &aoi_sizey_increment, &aoi_offsetx_increment, &aoi_offsety_increment) )
    {
        printf(" AOI increment size  : %dx%d\n", aoi_sizex_increment,aoi_sizey_increment);
        printf(" AOI increment offset: %dx%d\n", aoi_offsetx_increment,aoi_offsety_increment);
    }

    if( SVGigE_SUCCESS == Camera_getPixelDepth(camera,&svgige_pixel_depth) )
    {
        switch(svgige_pixel_depth)
        {
            case SVGIGE_PIXEL_DEPTH_8: pixel_depth = 8; break;
            case SVGIGE_PIXEL_DEPTH_12: pixel_depth = 12; break;
            case SVGIGE_PIXEL_DEPTH_16: pixel_depth = 16; break;
            default: pixel_depth = 8; break;
        }
        printf(" Pixel depth         : %d\n", pixel_depth);
    }

    int pixelclock =0;
    if( SVGigE_SUCCESS == Camera_getPixelClock(camera,&pixelclock) )
        printf(" Camera pixelclock   : %d Hz\n", pixelclock);


    if( SVGigE_SUCCESS == Camera_getInterPacketDelay(camera,&inter_packet_delay) )
        printf(" Inter-packet delay  : %1.0f \n", inter_packet_delay);

    if( SVGigE_SUCCESS == Camera_getMaxFrameRate(camera,&maxframerate) )
        printf(" Maximal framerate   : %3.1f fps\n", maxframerate);

    if( SVGigE_SUCCESS == Camera_getFrameRate(camera,&framerate) )
        printf(" Current framerate   : %3.1f fps\n", framerate);

    if( SVGigE_SUCCESS == Camera_getMaxExposureTime(camera,&maxexposure))
        printf(" Maximal exposure    : %3.1f ms (%3.1f fps)\n", maxexposure/1000,framerate);

    if( SVGigE_SUCCESS == Camera_getMaxGain(camera,&maxgain) )
        printf(" Maximal gain        : %3.1f dB\n", maxgain);


    return 0;
}



/// set trigger
/**
 * \return
 * \param arg trigger-type
 */
int command_trigger(char *arg)
{
    SVGigE_RETURN result;
    int triggerSetting = -1;
    bool isAcquisition =  (program_state == STATE_ACQUISITION);

    result = SVGigE_SUCCESS;

    //parse value
    if (matchInt(arg) == true)
    {
        triggerSetting = atoi(arg);
    }

    if ((triggerSetting >= 0) && (triggerSetting <= 3))
    {
        switch (triggerSetting)
        {
            case 0:
                printf(" switching to fixed frequency... ");
                result = Camera_setAcquisitionMode(camera, ACQUISITION_MODE_FIXED_FREQUENCY, isAcquisition);
                break;
            case 1:
                printf(" switching to software trigger... ");
                result = Camera_setAcquisitionMode(camera, ACQUISITION_MODE_SOFTWARE_TRIGGER, isAcquisition);
                break;
            case 2:
                printf(" switching to external trigger, internal exposure ... ");
                result = Camera_setAcquisitionMode(camera, ACQUISITION_MODE_EXT_TRIGGER_INT_EXPOSURE, isAcquisition);
                break;
            case 3:
                printf(" switching to external trigger, external exposure ... ");
                result = Camera_setAcquisitionMode(camera, ACQUISITION_MODE_EXT_TRIGGER_EXT_EXPOSURE, isAcquisition);
                break;
        }

        if (result == SVGigE_SUCCESS)
            printf("success\n");
        else
        {
            printf(" Error while setting trigger: ");
        }
    }
    else
    {
        printf(" Illegal value for trigger: %d", triggerSetting);
    }

    return errorHandling(result);
}

/// software-trigger
/**
 * \return
 */
int command_software_trigger()
{
    SVGigE_RETURN result;

    // Trigger acquisition for a single image
    result = Camera_startAcquisitionCycle(camera);

    if (result != SVGigE_SUCCESS)
    {
        printf(" Error while using software trigger: ");
    }

    return errorHandling(result);
}

/// end frame acquisition
/**
 * \return
 */
int command_end_acquisition()
{
    cmdEndAcquisition();

    return 0;
}


/// change exposure
/**
 *
 * \return
 * \param arg exposure value
 */
int command_exposure(char *arg)
{
    float exposure;
    SVGigE_RETURN result;

    result = SVGigE_SUCCESS;

    //parse value
    if (matchInt(arg) == true)
    {
        exposure = parseFPNumber(arg)*1000;  // [ms] to [us]

        if (exposure <= 60000000)
        {
            result = Camera_setExposureTime(camera, exposure);

            if( result == SVGigE_SUCCESS )
                printf(" exposure %3.1f ms\n", exposure/1000);
            else
            {
                printf(" Error while setting exposure: ");
            }
        }
        else
            printf(" exposure value not supported!\n");
    }

    return errorHandling(result);
}

/// change framerate
/**
 * \return
 * \param arg framerate value
 */
int command_framerate(char *arg)
{
    float framerate;
    SVGigE_RETURN result;

    result = SVGigE_SUCCESS;

    //parse value
    if (matchInt(arg) == true)
    {
        framerate = parseFPNumber(arg);

        if ((framerate >= 1) && (framerate <= 1000))
        {
            result = Camera_setFrameRate(camera, framerate);

            if( result == SVGigE_SUCCESS )
                printf(" framerate %3.1f fps\n", framerate);
            else
            {
                printf(" Error while setting framerate: ");
            }
        }
        else
            printf(" framerate value not supported!\n");
    }

    return errorHandling(result);
}

/// change gain
/**
 * \return
 * \param arg gain value
 */
int command_gain(char *arg)
{
    #define MAX_GAIN  36

    float gain;
    float max_gain = MAX_GAIN;
    SVGigE_RETURN result;

    result = SVGigE_SUCCESS;

    //parse value
    if (matchInt(arg) == true)
    {
        gain = parseFPNumber(arg);

        if (gain <= MAX_GAIN)
        {
            result = Camera_setGain(camera,gain);

            if( result == SVGigE_SUCCESS ) {
            Camera_getMaxGain(camera,&max_gain);
          if( gain > max_gain )
                  printf(" gain %3.1f dB (max %3.1f dB exceeded)\n", gain, max_gain);
        else
                  printf(" gain %3.1f dB\n", gain);
            }
            else
            {
                printf(" Error while setting gain: ");
            }
            // Disable auto gain
            Camera_setAutoGainEnabled(camera,false);
        }
        else
            printf(" gain value not supported!\n");
    }

    return errorHandling(result);
}


/// change auto gain
/**
 * \return
 * \param arg target brightness value
 */
int command_autogain(char *arg)
{
    float brightness;
    SVGigE_RETURN result;

    result = SVGigE_SUCCESS;

    //parse value
    if (matchInt(arg) == true)
    {
        brightness = parseFPNumber(arg);

        if (brightness>=0 && brightness<=255)
        {
            result = Camera_setAutoGainBrightness(camera,brightness);

            if( result == SVGigE_SUCCESS )
                printf(" auto gain brightness %3.1f\n", brightness);
            else
            {
                printf(" Error while setting auto gain: ");
            }
            result = Camera_setAutoGainEnabled(camera,true);

            if( result == SVGigE_SUCCESS )
                printf(" auto gain enabled\n");
            else
            {
                printf(" Error while enabling auto gain: ");
            }
        }
        else
            printf(" auto gain value not supported! (range: 0 - 255)\n");
    }

    return errorHandling(result);
}

/// set aoi size x
/**
 * \return
 * \param arg aoi size x
 */
int command_aoi_sizex(char *arg)
{
    SVGigE_RETURN result = SVGigE_SUCCESS;
    unsigned int new_aoi_sizex = -1;
    unsigned int aoi_sizex = -1;
    unsigned int aoi_sizey = -1;
    unsigned int aoi_offsetx = -1;
    unsigned int aoi_offsety = -1;

    if (displayWindowOpen() == true)
    {
        printf(" Close camera view (application) before adjusting a new AOI\n");
        return SVGigE_TL_PENDING_CHANNEL_DETECTED;
    }

    //parse value
    if (matchInt(arg) == true)
        new_aoi_sizex = atoi(arg);
    else
    {
        printf(" Invalid argument\n");
        return SVGigE_INVALID_PARAMETERS;
    }

    // Get current AOI settings
    printf(" Setting new image width %d... ", new_aoi_sizex);
    result = Camera_getAreaOfInterest(camera,&aoi_sizex,&aoi_sizey,&aoi_offsetx,&aoi_offsety);
    if (result == SVGigE_SUCCESS)
    {
        // Set new image width
        result = Camera_setAreaOfInterest(camera,new_aoi_sizex,aoi_sizey,aoi_offsetx,aoi_offsety);
        if (result == SVGigE_SUCCESS)
        {
            result = Camera_getSizeX(camera, &imageWidth);
            if (result == SVGigE_SUCCESS)
            {
                result = Camera_getSizeY(camera, &imageHeight);

                // Check for size reduction
                if( imageWidth < new_aoi_sizex )
                    printf("reduced to %d... ", imageWidth);
            }
        }
    }

    if (result != SVGigE_SUCCESS)
    {
        printf(" failed\n");
        return errorHandling(result);
    }
    else
        printf(" success\n");

    return 0;
}

/// set aoi size y
/**
 * \return
 * \param arg aoi size y
 */
int command_aoi_sizey(char *arg)
{
    SVGigE_RETURN result = SVGigE_SUCCESS;
    unsigned int new_aoi_sizey = -1;
    unsigned int aoi_sizex = -1;
    unsigned int aoi_sizey = -1;
    unsigned int aoi_offsetx = -1;
    unsigned int aoi_offsety = -1;

    if (displayWindowOpen() == true)
    {
        printf(" Close camera view (application) before adjusting a new AOI\n");
        return SVGigE_TL_PENDING_CHANNEL_DETECTED;
    }

    //parse value
    if (matchInt(arg) == true)
        new_aoi_sizey = atoi(arg);
    else
    {
        printf(" Invalid argument\n");
        return SVGigE_INVALID_PARAMETERS;
    }

    // Get current AOI settings
    printf(" Setting new image height %d... ", new_aoi_sizey);
    result = Camera_getAreaOfInterest(camera,&aoi_sizex,&aoi_sizey,&aoi_offsetx,&aoi_offsety);
    if (result == SVGigE_SUCCESS)
    {
        // Set new image height
        result = Camera_setAreaOfInterest(camera,aoi_sizex,new_aoi_sizey,aoi_offsetx,aoi_offsety);
        if (result == SVGigE_SUCCESS)
        {
            result = Camera_getSizeX(camera, &imageWidth);
            if (result == SVGigE_SUCCESS)
                result = Camera_getSizeY(camera, &imageHeight);

            // Check for size reduction
            if( imageHeight < new_aoi_sizey )
                printf("reduced to %d... ", imageHeight);
        }
    }

    if (result != SVGigE_SUCCESS)
    {
        printf(" failed\n");
        return errorHandling(result);
    }
    else
        printf(" success\n");

    return 0;
}

/// set aoi offset x
/**
 * \return
 * \param arg aoi offset x
 */
int command_aoi_offsetx(char *arg)
{
    SVGigE_RETURN result = SVGigE_SUCCESS;
    unsigned int new_aoi_offsetx = -1;
    unsigned int aoi_sizex = -1;
    unsigned int aoi_sizey = -1;
    unsigned int aoi_offsetx = -1;
    unsigned int aoi_offsety = -1;

    if (displayWindowOpen() == true)
    {
        printf(" Close camera view (application) before adjusting a new AOI\n");
        return SVGigE_TL_PENDING_CHANNEL_DETECTED;
    }

    //parse value
    if (matchInt(arg) == true)
        new_aoi_offsetx = atoi(arg);
    else
    {
        printf(" Invalid argument\n");
        return SVGigE_INVALID_PARAMETERS;
    }

    // Get current AOI settings
    printf(" Setting new image offset X %d... ", new_aoi_offsetx);
    result = Camera_getAreaOfInterest(camera,&aoi_sizex,&aoi_sizey,&aoi_offsetx,&aoi_offsety);
    if (result == SVGigE_SUCCESS)
    {
        // Set new image offset X
        result = Camera_setAreaOfInterest(camera,aoi_sizex,aoi_sizey,new_aoi_offsetx,aoi_offsety);
        if (result == SVGigE_SUCCESS)
        {
            result = Camera_getSizeX(camera, &imageWidth);
            if (result == SVGigE_SUCCESS)
                result = Camera_getSizeY(camera, &imageHeight);
        }
    }

    if (result != SVGigE_SUCCESS)
    {
        printf(" failed\n");
        return errorHandling(result);
    }
    else
        printf(" success\n");

    return 0;
}

/// set aoi offset y
/**
 * \return
 * \param arg aoi offset y
 */
int command_aoi_offsety(char *arg)
{
    SVGigE_RETURN result = SVGigE_SUCCESS;
    unsigned int new_aoi_offsety = -1;
    unsigned int aoi_sizex = -1;
    unsigned int aoi_sizey = -1;
    unsigned int aoi_offsetx = -1;
    unsigned int aoi_offsety = -1;

    if (displayWindowOpen() == true)
    {
        printf(" Close camera view (application) before adjusting a new AOI\n");
        return SVGigE_TL_PENDING_CHANNEL_DETECTED;
    }

    //parse value
    if (matchInt(arg) == true)
        new_aoi_offsety = atoi(arg);
    else
    {
        printf(" Invalid argument\n");
        return SVGigE_INVALID_PARAMETERS;
    }

    // Get current AOI settings
    printf(" Setting new image offset Y %d... ", new_aoi_offsety);
    result = Camera_getAreaOfInterest(camera,&aoi_sizex,&aoi_sizey,&aoi_offsetx,&aoi_offsety);
    if (result == SVGigE_SUCCESS)
    {
        // Set new image offset Y
        result = Camera_setAreaOfInterest(camera,aoi_sizex,aoi_sizey,aoi_offsetx,new_aoi_offsety);
        if (result == SVGigE_SUCCESS)
        {
            result = Camera_getSizeX(camera, &imageWidth);
            if (result == SVGigE_SUCCESS)
                result = Camera_getSizeY(camera, &imageHeight);
        }
    }

    if (result != SVGigE_SUCCESS)
    {
        printf(" failed\n");
        return errorHandling(result);
    }
    else
        printf(" success\n");

    return 0;
}

/// set inter-packet delay
/**
 * \return
 * \param arg inter-packet delay
 */
int command_inter_packet_delay(char *arg)
{
    SVGigE_RETURN result = SVGigE_SUCCESS;
    int inter_packet_delay = 0;

    //parse value
    if (matchInt(arg) == true)
        inter_packet_delay = atoi(arg);

    // Check for valid inter_packet_delay
    if( inter_packet_delay < 0 ){
        inter_packet_delay = 0;
    }
    else
    if( inter_packet_delay > 260000 ){
        inter_packet_delay = 260000;
    }
/*
    else
    {
        printf(" Invalid argument '%d' (range 0..260000)\n",inter_packet_delay);
        return SVGigE_INVALID_PARAMETERS;
    }
*/
    // Set new inter-packet delay
    printf(" Setting new inter-packet delay %d... ", inter_packet_delay);
    result = Camera_setInterPacketDelay(camera,inter_packet_delay);

    if (result != SVGigE_SUCCESS)
    {
        printf(" failed\n");
        return errorHandling(result);
    }
    else
        printf(" success\n");

    return 0;
}



/// force IP of a camera
/**
 * \return
 * \param sn camera's serial number
 * \param ip new IP address for camera
 * \param mask new subnet mask for camera
 */
int command_force_ip(char* sn_mac, char* ip, char* mask)
{
    SVGigE_RETURN result = SVGigE_SUCCESS;

    unsigned int networks[MAX_NETWORK_COUNT];
    unsigned int ipSource = 0;
    unsigned int ipCamera = 0;
    unsigned int ipSubnet = 0;
//    unsigned int snCamera = 0;


    if (matchMAC(sn_mac))
    {
        printf("complete mac address: %s\n", sn_mac);
    }
    else if (matchLast4HexDigits(sn_mac))
    {
        printf("last 4 hex digits: %s\n",  sn_mac);
    }
    else {
        printf("mac address not correctly specified, this commando will not be executed\n");
        return result;
    }


    if (matchIP(ip) == true)
        ipCamera = strIPToInt(ip);
    else {
        printf(" IP not correctly specified, this commando will not be executed\n");
        return result;
    }

    if (matchIP(mask) == true)
      {
        ipSubnet = strIPToInt(mask);
      }
    else {
        printf(" Subnet mask not correctly specified, this commando will not be executed\n");
    }

 // Search for source IP in network adapters
 if( SVGigE_SUCCESS == findNetworkAdapters((unsigned int*) &networks, MAX_NETWORK_COUNT) )
     for(int i = 0; i < MAX_NETWORK_COUNT; i++)
 {
         if( networks[i]==0 )
             break;

         if( (ipCamera & ipSubnet) == (networks[i] & ipSubnet) )
             ipSource = networks[i];
     }

 if( ipCamera==0 )
     printf(" Invalid IP address\n");
 else
     if( ipSubnet==0 )
         printf(" Invalid subnet mask\n");
     else
         if( ipSource==0 )
             printf(" IP does not match any network interface\n");
         else
         {
             // Adjust IP based on MAC
             result = forceIP_MAC(sn_mac, ipCamera, ipSubnet, ipSource);
             if( result == SVGigE_SUCCESS )
             {
                 printf(" camera SN=%s has been forced to IP=%d.%d.%d.%d/%d.%d.%d.%d\n",sn_mac,
                        (ipCamera>>24)&0xFF,(ipCamera>>16)&0xFF,(ipCamera>>8)&0xFF,ipCamera&0xFF,
                        (ipSubnet>>24)&0xFF,(ipSubnet>>16)&0xFF,(ipSubnet>>8)&0xFF,ipSubnet&0xFF);
             }
         }

 return errorHandling(result);

}

/// set IP of a camera
/**
 * \return
 * \param ip new IP address for camera
 * \param mask new subnet mask for camera
 */
int command_set_ip(char* ip, char* mask)
{
    SVGigE_RETURN result = SVGigE_SUCCESS;

    unsigned int ipCamera = 0;
    unsigned int ipSubnet = 0;

    if( NULL==ip )
            printf(" IP not specified\n");
    else
    {
    if( !strcmp(ip,"off") )
    {
      // Switch persistent IP off (DHCP will be on)
      result = setIPConfig(camera,true,false);
            if( result == SVGigE_SUCCESS )
              printf(" camera's persistent IP has been switched off\n");
      else
              printf(" setting camera's IP config failed\n");
    }
    else
    {
          if (matchIP(ip) == true)
              ipCamera = strIPToInt(ip);

          if( NULL==mask )
              ipSubnet = 0xFFFFFF00;
          else
          if (matchIP(mask) == true)
              ipSubnet = strIPToInt(mask);

          if( ipCamera==0 )
                  printf(" Invalid IP address\n");
          else
          if( ipSubnet==0 )
              printf(" Invalid subnet mask\n");
          else
          {
              result = setIPAddress(camera,ipCamera,ipSubnet);
              if( result == SVGigE_SUCCESS )
                  printf(" camera will have fixed IP '%d.%d.%d.%d/%d.%d.%d.%d' after reboot\n",
                                     (ipCamera>>24)&0xFF,(ipCamera>>16)&0xFF,(ipCamera>>8)&0xFF,ipCamera&0xFF,
                                     (ipSubnet>>24)&0xFF,(ipSubnet>>16)&0xFF,(ipSubnet>>8)&0xFF,ipSubnet&0xFF);
          }
    }
    }
    return errorHandling(result);
}
