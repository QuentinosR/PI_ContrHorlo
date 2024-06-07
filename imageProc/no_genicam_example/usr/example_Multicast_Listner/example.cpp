#include <stdio.h>
#include <stdlib.h>
#include "../include/svgige.h"

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include <signal.h>

Camera_handle camera;
Stream_handle stream;

int pipefd[2];
long long t1;
bool saveImg = false;


void writeimage(unsigned short block_id, unsigned char *buf, int len)
{
    char filename[1024];
    sprintf(filename, "image_%d.raw", block_id);

    FILE *fp = fopen(filename, "wb");

    if(fp)
    {
        fwrite(buf, len, 1, fp);
        fclose(fp);
    }
}


SVGigE_RETURN imageCallback(SVGigE_SIGNAL *signal, void *context)
{
    if(signal->SignalType == SVGigE_SIGNAL_FRAME_COMPLETED)
    {
        SVGigE_IMAGE *signalImage = (SVGigE_IMAGE *)signal->Data;

        printf("first pixel: %u\n", signalImage->ImageData[0]);

        write(pipefd[1], &signalImage, sizeof(signalImage));
    }
    else
        if(signal->SignalType == SVGigE_SIGNAL_FRAME_ABANDONED)
        {
            printf("console: frame lost\n");
        }
        else
            if(signal->SignalType == SVGigE_SIGNAL_START_OF_TRANSFER)
            {
                //printf("console: SVGigE_SIGNAL_START_OF_TRANSFER\n");
            }
            else
            {
                printf("console: invalid callback %d\n", signal->SignalType);
            }

    fflush(stdout);

    return SVGigE_SUCCESS;
}


void sig_handler(int signum){
    printf("\n Received signal %d\n", signum);

    SVGigE_RETURN retVal;
    retVal = closeStream(stream);
    if ( retVal == SVGigE_SUCCESS )
        printf("closeStream successful\n");
    else
        printf("closeStream failed\n");

    retVal = closeCamera(camera);
    if ( retVal == SVGigE_SUCCESS )
        printf("closeCamera successful\n");
    else
        printf("closeCamera failed\n");

    return;
}
int main(int argc, char *argv[])
{
    printf("console demo -- start --\n");

		signal(SIGINT, sig_handler);
	
    pipe(pipefd);

    int result;
    struct in_addr addr;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <dotted-ip-address>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("console: addr %s\n", argv[1] );
    if (inet_aton(argv[1], &addr) == 0)
    {
        perror("inet_aton");
        exit(EXIT_FAILURE);
    }

    // set the ip, of which is used to communicate with the camera
    int sourcIP = ntohl(inet_addr("169.254.11.11"));
    result = openCamera(&camera, ntohl(addr.s_addr), sourcIP, 3000, MULTICAST_MODE_LISTENER);
    if(result)
    {
        printf("console: openCamera failed\n");
        return -1;
    }


    // set the camera's multicast group(ip) and port for multicast participant
    int mcIP = ntohl(inet_addr("232.227.99.99"));
    unsigned short mcPort= 9999;
    Camera_setMulticastGroup(camera, mcIP, mcPort);


    unsigned int local_ip;
    unsigned short local_port;
    unsigned int bufsize;
    int bufcount = 10;
    int packetsize = 9000;//1500;


    unsigned int x;
    Camera_getSizeX(camera, &x);
    printf("console: x = %i\n", x);

    unsigned int y;
    Camera_getSizeY(camera, &y);
    printf("console: y = %i\n", y);
		
		
					
    result = Camera_getBufferSize(camera, &bufsize);
    if(result)
    {
        printf("console: can't read buffersize\n");
        return -1;
    }

    result = addStream(camera, &stream, &local_ip, &local_port, bufsize, bufcount, packetsize, 1000, imageCallback, NULL);
    if(result)
    {
        printf("console: addStream failed\n");
        return -1;
    }

    printf("console: enable stream\n");
    enableStream(stream, true);

    printf("console: acquisition mode fixed freq\n");
    Camera_setAcquisitionMode(camera,ACQUISITION_MODE_FIXED_FREQUENCY,false);

    printf("console: acq start\n");
    Camera_setAcquisitionControl(camera, ACQUISITION_CONTROL_START);

    fflush(stdout);
    int imgcount = 0;

    while(-1)
    {
        SVGigE_IMAGE *signalImage;

        read(pipefd[0], &signalImage, sizeof(signalImage));

        unsigned short block_id = signalImage->ImageID;
        unsigned char *img = signalImage->ImageData;
        unsigned int s = signalImage->ImageSize;
        unsigned int packetresend = signalImage->PacketResend;

        if(packetresend)
        {
            printf("console: block_id: %d packet_resend %d\n", block_id, packetresend);
            fflush(stdout);
        }

        if(saveImg)
        {
            printf("writeimage: %d\n",block_id);
            writeimage(block_id, img, s);
        }

        float framerate;
        StreamingChannel_getActualFrameRate(stream, &framerate);
        printf("\rframerate %.2f    ", framerate);

        fflush(stdout);

        imgcount++;
    }

    closeStream(stream);

    closeCamera(camera);

    return 0;
}
