#include <opencv2/opencv.hpp> 

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include "sv_gen_sdk.h"

#if defined (WIN32)
#include "sv_gen_sdk_dynamic.h"
#endif

using namespace std;

// global variable
SV_RETURN ret;
SV_SYSTEM_HANDLE g_hSystem;
SV_INTERFACE_HANDLE g_hInterface;
SV_DEVICE_HANDLE g_hDevice;
SV_REMOTE_DEVICE_HANDLE g_hRemoteDevice;
SV_STREAM_HANDLE g_hStream;
vector<SV_DEVICE_INFO> g_devInfoList;

#define HARDCODE_BUFFER_COUNT 4

// forward declaration
bool InitSDK();
void PrepareAcquisitionBuffer(SV_STREAM_HANDLE hDS, SV_REMOTE_DEVICE_HANDLE hRemoteDev);

void * AcquisitionThread(SV_STREAM_HANDLE context)
{
    SV_STREAM_HANDLE hDS = (SV_STREAM_HANDLE)context;
    if (NULL == hDS)
        return nullptr;

    printf("%s AcquisitionThread Running...\n", __FUNCTION__);

    while (true)
    {
        SV_BUFFER_HANDLE hBuffer = NULL;
        SV_RETURN ret = SVStreamWaitForNewBuffer(hDS, NULL, &hBuffer, 1000);
        if (SV_ERROR_SUCCESS == ret)
        {
            SV_BUFFER_INFO bufferInfo = { 0 };
            ret = SVStreamBufferGetInfo(hDS, hBuffer, &bufferInfo);
            if (SV_ERROR_SUCCESS != ret)
            {
                printf("%s SVStreamBufferGetInfo Failed!:%d\n", __FUNCTION__, ret);
                continue;
            }

            printf("Image Received FrameId:%lld Info Ptr:0x%p Width:%zd Height:%zd\n", bufferInfo.iImageId, bufferInfo.pImagePtr, bufferInfo.iSizeX, bufferInfo.iSizeY);
            printf("----------------------------------------------\n");
            
            char filename[1024];
            sprintf(filename, "img/image_%lld.raw", bufferInfo.iImageId);

            FILE *fp = fopen(filename, "wb");
            cv::Mat img(bufferInfo.iSizeY, bufferInfo.iSizeX, CV_8UC1, bufferInfo.pImagePtr);

            char filename_png[1024];
            sprintf(filename_png, "img/image_%lld.png", bufferInfo.iImageId);

            cv::imwrite(filename_png, img);
            
            printf("test to save\n");

            if(fp)
            {
            fwrite(bufferInfo.pImagePtr, bufferInfo.iSizeX * bufferInfo.iSizeY, 1, fp);
            fclose(fp);
            }

            SVStreamQueueBuffer(hDS, hBuffer);
        }
        else if (SV_ERROR_TIMEOUT == ret)
        {
            printf("%s SVStreamWaitForNewBuffer TIMEOUT\n", __FUNCTION__);
            continue;
        }
        else
        {
            printf("%s SVStreamWaitForNewBuffer Failed!:%d\n", __FUNCTION__, ret);
            break;
        }
    }

    printf("%s AcquisitionThread End\n", __FUNCTION__);
    return 0;
}


int main(int argc, char**argv)
{

    printf("Initialisation...\n");
    /*
    cv::Mat image = cv::imread("test.jpg"); 
    cv::imshow("Image",image);
    cv::waitKey(0);
    */

    bool isInited = InitSDK();
    if (!isInited)
    {
        printf("%s InitSDK Failed!\n", __FUNCTION__);
        system("pause");
        return -1;
    }

    //Get System
    uint32_t tlCount = 0;
    ret = SVLibSystemGetCount(&tlCount);

    // e.g. only Gev
    bool bOpenGev = true, bOpenU3V = false, bOpenCL = false;
    for (uint32_t i = 0; i < tlCount; i++)
    {
        SV_TL_INFO tlInfo = { {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}};
        ret = SVLibSystemGetInfo(i, &tlInfo);
        if (SV_ERROR_SUCCESS != ret)
        {
            printf("SVLibSystemGetInfo Failed! :%d\n", ret);
            continue;
        }

        bool bOpenTL = false;
        if (bOpenGev &&  string("GEV") == string(tlInfo.tlType))
            bOpenTL = true;
        else if (bOpenU3V && string("U3V") == string(tlInfo.tlType))
            bOpenTL = true;
        else if (bOpenCL && string("CL") == string(tlInfo.tlType))
            bOpenTL = true;

        if (false == bOpenTL)
            continue;

        g_hSystem = NULL;
        ret = SVLibSystemOpen(i, &g_hSystem);
        if (SV_ERROR_SUCCESS != ret)
            printf("SVLibSystemOpen Failed! :%d\n", ret);
    }

    //Get Interface
    bool bChanged = false;

    SV_RETURN ret = SVSystemUpdateInterfaceList(g_hSystem, &bChanged, 1000);
    if (SV_ERROR_SUCCESS != ret)
    {
        printf("%s SVSystemUpdateInterfaceList failed!:%d\n", __FUNCTION__, ret);
        return -1;
    }

    uint32_t numInterface = 0;
    ret = SVSystemGetNumInterfaces(g_hSystem, &numInterface);
    if (SV_ERROR_SUCCESS != ret)
    {
        printf("%s SVSystemGetNumInterfaces failed!:%d\n", __FUNCTION__, ret);
        return -1;
    }

    if (0 == numInterface)
    {
        printf("%s No Interface Available!\n", __FUNCTION__);
        return -1;
    }

    printf("*************Enumerate Interface****************\n");
    for (uint32_t i = 0; i < numInterface; i++)
    {
        char interfaceId[SV_STRING_SIZE] = { 0 };
        size_t interfaceIdSize = sizeof(interfaceId);

        ret = SVSystemGetInterfaceId(g_hSystem, i, interfaceId, &interfaceIdSize);
        if (SV_ERROR_SUCCESS != ret)
        {
            printf("%s SVSystemGetInterfaceId failed!:%d\n", __FUNCTION__, ret);
            continue;
        }

        SV_INTERFACE_INFO interfaceInfo = { {}, {}, {}, {}, {}, {}, {}};
        ret = SVSystemInterfaceGetInfo(g_hSystem, interfaceId, &interfaceInfo);
        if (SV_ERROR_SUCCESS != ret)
        {
            printf("%s SVSystemInterfaceGetInfo failed!:%d\n", __FUNCTION__, ret);
            continue;
        }

        printf("[%d] Interface Id:%s\n", i, interfaceId);


        g_hInterface = NULL;
        ret = SVSystemInterfaceOpen(g_hSystem, interfaceId, &g_hInterface);
        if (SV_ERROR_SUCCESS != ret)
        {
            printf("%s SVSystemInterfaceOpen failed!:%d\n", __FUNCTION__, ret);
            continue;
        }

        // Get Device
        ret = SVInterfaceUpdateDeviceList(g_hInterface, &bChanged, 5000);
        if (SV_ERROR_SUCCESS != ret)
        {
            printf("%s SVInterfaceUpdateDeviceList failed!:%d\n", __FUNCTION__, ret);
            continue;
        }

        uint32_t numDevices = 0;
        ret = SVInterfaceGetNumDevices(g_hInterface, &numDevices);
        if (SV_ERROR_SUCCESS != ret)
        {
            printf("%s SVInterfaceGetNumDevices failed!:%d\n", __FUNCTION__, ret);
            SVInterfaceClose(g_hInterface);
            continue;
        }

        if (0 == numDevices)
        {
            printf("%s No Device Connected!\n", __FUNCTION__);
            SVInterfaceClose(g_hInterface);
            continue;
        }


        for (uint32_t j = 0; j < numDevices; j++)
        {
            char deviceId[SV_STRING_SIZE] = { 0 };
            size_t deviceIdSize = sizeof(deviceId);
            ret = SVInterfaceGetDeviceId(g_hInterface, j, deviceId, &deviceIdSize);
            if (SV_ERROR_SUCCESS != ret)
            {
                printf("%s SVInterfaceGetDeviceId Failed!:%d\n", __FUNCTION__, ret);
                continue;
            }

            printf("\t[%d] device Id:%s\n", j, deviceId);

            SV_DEVICE_INFO devInfo = { {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}};
            ret = SVInterfaceDeviceGetInfo(g_hInterface, deviceId, &devInfo);
            if (SV_ERROR_SUCCESS != ret)
            {
                printf("%s SVInterfaceDeviceGetInfo Failed!:%d\n", __FUNCTION__, ret);
                continue;
            }

            g_devInfoList.push_back(devInfo);
        }
    }

    if (g_devInfoList.empty())
    {
        printf("no camera \n");
        return -1;
    }

    SV_DEVICE_INFO devInfo = g_devInfoList.front();

    g_hDevice = NULL;
    g_hRemoteDevice = NULL;
    SVInterfaceDeviceOpen(devInfo.hParentIF, devInfo.uid, SV_DEVICE_ACCESS_EXCLUSIVE, &g_hDevice, &g_hRemoteDevice);

    char streamId0[SV_STRING_SIZE] = { 0 };
    size_t streamId0Size = SV_STRING_SIZE;

    ret = SVDeviceGetStreamId(g_hDevice, 0, streamId0, &streamId0Size);
    if (SV_ERROR_SUCCESS != ret)
    {
        printf("%s SVDeviceGetStreamId Failed!:%d\n", __FUNCTION__, ret);
        return -1;
    }


    // Acq START
    g_hStream = NULL;
    ret = SVDeviceStreamOpen(g_hDevice, streamId0, &g_hStream);
    if (SV_ERROR_SUCCESS != ret)
    {
        printf("%s SVDeviceStreamOpen Failed!:%d\n", __FUNCTION__, ret);
        return -1;
    }

    SV_FEATURE_HANDLE hFeature = NULL;

    SVFeatureGetByName(g_hRemoteDevice, "TriggerMode", &hFeature);
    SVFeatureSetValueInt64Enum(g_hRemoteDevice, hFeature, 1);

    //Added by me, to test
    hFeature = NULL;
    SVFeatureGetByName(g_hRemoteDevice, "TriggerSource", &hFeature);
    SVFeatureSetValueInt64Enum(g_hRemoteDevice, hFeature, 1);


    hFeature = NULL;
    SVFeatureGetByName(g_hRemoteDevice, "ExposureMode", &hFeature);
    SVFeatureSetValueInt64Enum(g_hRemoteDevice, hFeature, 1);


    hFeature = NULL;
    SVFeatureGetByName(g_hRemoteDevice, "AcquisitionFrameRate", &hFeature);
    SVFeatureSetValueFloat(g_hRemoteDevice, hFeature, 5.0f);

    hFeature = NULL;
    SVFeatureGetByName(g_hRemoteDevice, "TLParamsLocked", &hFeature);
    SVFeatureSetValueInt64(g_hRemoteDevice,hFeature, 1);

    std::thread  hThreadAcquisition(AcquisitionThread, g_hStream);
    hThreadAcquisition.detach();

    PrepareAcquisitionBuffer(g_hStream, g_hRemoteDevice);

#ifndef INFINITE
#define INFINITE 0xFFFFFFFF
#endif
    ret = SVStreamAcquisitionStart(g_hStream, SV_ACQ_START_FLAGS_DEFAULT, INFINITE);
    if (SV_ERROR_SUCCESS != ret)
    {
        printf("%s SVStreamAcquisitionStart Failed!:%d\n", __FUNCTION__, ret);
        return -1;
    }

    hFeature = NULL;
    SVFeatureGetByName(g_hRemoteDevice, "AcquisitionStart", &hFeature);
    SVFeatureCommandExecute(g_hRemoteDevice, hFeature, 1000);

    while(1){}
    hFeature = NULL;
    SVFeatureGetByName(g_hRemoteDevice, "TriggerSoftware", &hFeature);

    int triggerCounter = 0;
    for (int i = 0; i < 10; i++)
    {
        printf("SoftwareTrigger:%d\n", triggerCounter++);
        SVFeatureCommandExecute(g_hRemoteDevice, hFeature, 1000, true);
        this_thread::sleep_for(std::chrono::milliseconds(500)); // e.g. 1/fps
    }


    // Acq STOP
    hFeature = NULL;
    uint32_t ExecuteTimeout = 1000;
    SVFeatureGetByName(g_hRemoteDevice, "AcquisitionStop", &hFeature);
    SVFeatureCommandExecute(g_hRemoteDevice, hFeature, ExecuteTimeout);

    hFeature = NULL;
    SVFeatureGetByName(g_hRemoteDevice, "TLParamsLocked", &hFeature);
    SVFeatureSetValueInt64(g_hRemoteDevice, hFeature, 0);


    SVStreamAcquisitionStop(g_hStream, SV_ACQ_STOP_FLAGS_DEFAULT);
    SVStreamFlushQueue(g_hStream, SV_ACQ_QUEUE_INPUT_TO_OUTPUT);
    SVStreamFlushQueue(g_hStream, SV_ACQ_QUEUE_OUTPUT_DISCARD);

    for (uint32_t i = 0; i < HARDCODE_BUFFER_COUNT; i++)
    {
        SV_BUFFER_HANDLE hBuffer = NULL;
        uint8_t *pBuffer = NULL;
        SVStreamGetBufferId(g_hStream, 0, &hBuffer);

        if (hBuffer)
        {
            SVStreamRevokeBuffer(g_hStream, hBuffer, (void**)&pBuffer, NULL);
            if (pBuffer)
                delete [] pBuffer;
        }
    }

    SVStreamClose(g_hStream);
    SVDeviceClose(g_hDevice);
    SVInterfaceClose(g_hInterface);
    SVSystemClose(g_hSystem);
    SVLibClose();
    return 0;
}


bool InitSDK()
{
    string ctiPath;
    string genicamPath;
    string genicamCachePath;
    string clProtocolPath;

#if defined (WIN32)

    char buffer[1024] = { 0 };
    int res = GetEnvironmentVariableA("GENICAM_GENTL64_PATH", buffer, sizeof(buffer));
    if (0 == res)
        return false;

    ctiPath = string(buffer);

    memset(buffer, 0, sizeof(buffer));
    res = GetEnvironmentVariableA("SVS_GENICAM_ROOT", buffer, sizeof(buffer));
    if (0 == res)
        return false;

    genicamPath = string(buffer);

    memset(buffer, 0, sizeof(buffer));
    res = GetEnvironmentVariableA("SVS_GENICAM_CACHE", buffer, sizeof(buffer));
    if (0 == res)
        return false;

    genicamCachePath = string(buffer);

    memset(buffer, 0, sizeof(buffer));
    res = GetEnvironmentVariableA("SVS_GENICAM_CLPROTOCOL", buffer, sizeof(buffer));
    if (0 == res)
        return false;

    clProtocolPath = string(buffer);

    SV_RETURN ret = SVLibInit(ctiPath.c_str(), genicamPath.c_str(), genicamCachePath.c_str(), clProtocolPath.c_str());
#elif defined(__linux__)
    ctiPath ="/opt/SVS/SVCamKit/SDK/Linux64_x64/cti";
    SV_RETURN ret = SVLibInit(ctiPath.c_str(), NULL, NULL, NULL);

#elif defined(__APPLE__) && defined(__MACH__)
    ctiPath = "/Library/Frameworks/SVGenSDK.framework/Versions/2.5.8/Libraries/cti";
    SV_RETURN ret = SVLibInit(ctiPath.c_str(), NULL, NULL, NULL);
#endif
    if (SV_ERROR_SUCCESS != ret)
    {
        printf("SVLibInit Failed! :%d", ret);
        return false;
    }

    return true;
}

void PrepareAcquisitionBuffer(SV_STREAM_HANDLE hDS, SV_REMOTE_DEVICE_HANDLE hRemoteDev)
{
    SV_FEATURE_HANDLE hFeature = NULL;
    int64_t payloadSize = 0;

    SVFeatureGetByName(hRemoteDev, "PayloadSize", &hFeature);
    SVFeatureGetValueInt64(hRemoteDev, hFeature, &payloadSize);

    for (uint32_t i = 0; i<HARDCODE_BUFFER_COUNT; i++)
    {
        uint8_t *buffer = new uint8_t[(size_t)payloadSize];
        memset(buffer, 0, (size_t)payloadSize);

        SV_BUFFER_HANDLE hBuffer = NULL;
        SV_RETURN ret = SVStreamAnnounceBuffer(hDS, buffer, (uint32_t)payloadSize, NULL, &hBuffer);
        if (SV_ERROR_SUCCESS != ret)
        {
            printf(":%s SVStreamAnnounceBuffer[%d] Failed!:%d\n", __FUNCTION__, i, ret);
            continue;
        }

        SVStreamQueueBuffer(hDS, hBuffer);
    }

    SVStreamFlushQueue(hDS, SV_ACQ_QUEUE_ALL_TO_INPUT);
}
