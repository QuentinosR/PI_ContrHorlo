
/*******************************************************************************
 * SVS GenSDK API Declaration of SVS camera access functions
 *******************************************************************************
 *
 * Version:     2.5.8
 *
 * Copyright:   SVS VISTEK GmbH
 */
#ifndef SV_GEN_SDK_H_
#define SV_GEN_SDK_H_

#if defined (_WIN32) && !defined(WIN32)
#define WIN32
#endif

#  ifdef __cplusplus
typedef bool bool8_t;
#  else
typedef uint8_t bool8_t;
#  endif

#if defined(_WIN32)
#include <windows.h>
#ifndef _STDINT_H
#ifdef _MSC_VER // Microsoft compiler
#if _MSC_VER < 1600
typedef __int8            int8_t;
typedef __int16           int16_t;
typedef __int32           int32_t;
typedef __int64           int64_t;
typedef unsigned __int8   uint8_t;
typedef unsigned __int16  uint16_t;
typedef unsigned __int32  uint32_t;
typedef unsigned __int64  uint64_t;
#else
// In Visual Studio 2010 is stdint.h already included
#include <stdint.h>
#endif
#else
// Not a Microsoft compiler
#include <stdint.h>
#endif
#endif
#else
#include <stddef.h>
#include <stdint.h>
#endif


#ifdef _MSC_VER
#   ifdef SVGENSDK_EXPORTS
#       define SV_GEN_SDK_API __declspec(dllexport)
#   elif SVGENSDK_DYNAMIC_LOAD
#       define SV_GEN_SDK_API
#   else
#       define SV_GEN_SDK_API __declspec(dllimport)
#   endif
#elif defined (__GNUC__)
#   ifdef SVGENSDK_EXPORTS
#       define SV_GEN_SDK_API __attribute__((visibility("default")))
#   else
#      define SV_GEN_SDK_API
#   endif
#else
#   error Unsupported compiler or platform
#endif

#ifdef __cplusplus
#   ifndef EXTERN_C
#   define EXTERN_C extern "C"
#   endif
#else
#   define EXTERN_C
#endif

#define SV_STRING_SIZE 512

typedef int SV_RETURN;

typedef void* SV_SYSTEM_HANDLE;
typedef void* SV_INTERFACE_HANDLE;
typedef void* SV_DEVICE_HANDLE;
typedef void* SV_REMOTE_DEVICE_HANDLE;
typedef void* SV_STREAM_HANDLE;
typedef void* SV_BUFFER_HANDLE;
typedef void* SV_FEATURE_HANDLE;
typedef void* SV_EVENT_HANDLE;

#ifndef SV_GEN_SDK_VERSION_DEFINED
#define SV_GEN_SDK_VERSION_DEFINED
typedef struct
{
    uint32_t   MajorVersion;
    uint32_t   MinorVersion;
    uint32_t   Revision;
    uint32_t   BuildVersion;
} SV_LIB_VERSION;

#define SV_GEN_SDK_VERSION_MAJOR        2
#define SV_GEN_SDK_VERSION_MINOR        5
#define SV_GEN_SDK_VERSION_REVISION     7
#define SV_GEN_SDK_VERSION_BUILD        2193

#endif

#define GENTL_INFINITE        0xFFFFFFFFFFFFFFFFULL
typedef enum
{
    SV_ERROR_SUCCESS = 0,      ///< OK
    SV_ERROR_UNKNOWN = -1001,  ///< Generic errorcode
    SV_ERROR_NOT_INITIALIZED = -1002,
    SV_ERROR_NOT_IMPLEMENTED = -1003,
    SV_ERROR_RESOURCE_IN_USE = -1004,
    SV_ERROR_ACCESS_DENIED = -1005,
    SV_ERROR_INVALID_HANDLE = -1006,
    SV_ERROR_INVALID_ID = -1007,
    SV_ERROR_NO_MORE_DATA = -1008,
    SV_ERROR_INVALID_PARAMETER = -1009,
    SV_ERROR_FILE_IO = -1010,
    SV_ERROR_TIMEOUT = -1011,
    SV_ERROR_ABORT = -1012,

    SV_ERROR_NOT_OPENED = -2001,
    SV_ERROR_NOT_AVAILABLE = -2002,
    SV_ERROR_NOT_FOUND = -2003,
    SV_ERROR_BUFFER_TOO_SMALL = -2004,
    SV_ERROR_INVALID_FEATURE_TYPE = -2005,
    SV_ERROR_GENICAM_EXCEPTION = -2006,
    SV_ERROR_OUT_OF_MEMORY = -2007,
    SV_ERROR_GENICAM_DLL_NOT_FOUND = -2008,
    SV_ERROR_INVALID_GENICAM_CACHE_DIR = -2009,
    SV_ERROR_GENICAM_DLL_LOAD_FAILED = -2010,
    SV_ERROR_INVALID_CONFIG_FILE = -2011,
    SV_ERROR_LOG_DLL_NOT_LOADED = -2012,

    //SDK 2.5.0
    SV_ERROR_PIXEL_FORMAT_NOT_SUPPORTED = -2013,
    SV_ERROR_LIBPNG_DLL_NOT_LOADED = -2014,
    SV_ERROR_DLL_VERSION_MISMATCH = -2015,

    //SDK 2.5.1
    SV_ERROR_EVENT_NOT_SUPPORTED = -2016
} SV_STATUS;

typedef enum
{
    SV_Beginner = 0,               //!< Always visible
    SV_Expert = 1,                 //!< Visible for experts or Gurus
    SV_Guru = 2,                   //!< Visible for Gurus
    SV_Invisible = 3,              //!< Not Visible
    SV_UndefinedVisibility = 99  //!< Object is not yet initialized
} SV_FEATURE_VISIBILITY;

//TBD only temporary
typedef enum
{
    SVGigETL_SIGNAL_NONE = 0,
    SVGigETL_SIGNAL_FRAME_COMPLETED = 1,		// new image available, transfer was successful
    SVGigETL_SIGNAL_FRAME_ABANDONED = 2,		// an image could not be completed in time and was therefore abandoned
    SVGigETL_SIGNAL_START_OF_TRANSFER = 3,		// transfer of a new image has started
    SVGigETL_SIGNAL_BANDWIDTH_EXCEEDED = 4,     // available network bandwidth has been exceeded
    SVGigETL_SIGNAL_TEST_PACKET = 5,            // driver problem due to old-style driver behavior (prior to 2003, not WDM driver)
    SVGigETL_SIGNAL_FRAME_INCOMPLETE = 7        // a frame could not be properly completed
} SVGigETL_SIGNAL_TYPE;

#pragma pack (push, 1)
typedef struct SV_DS_EVENT_DATA
{
    uint32_t imageID;
    uint64_t timeStamp;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t reserved3;
    uint32_t reserved4;
} SV_DS_EVENT_DATA;
#pragma pack (pop)

#pragma pack (push, 1)
typedef struct SV_EVENT_ERROR_DATA
{
    int32_t errorCode;
    char description[1024];
} SV_EVENT_ERROR_DATA;
#pragma pack (pop)

#pragma pack (push, 1)
typedef struct SV_EVENT_MODULE_DATA
{
    uint64_t eventID;
    uint8_t  data[1024];
} SV_EVENT_MODULE_DATA;
#pragma pack (pop)

#pragma pack (push, 1)
typedef struct SV_EVENT_REMOTE_DEVICE_DATA
{
    uint32_t eventID;
    uint16_t dataSize;
    uint8_t  data[540];
} SV_EVENT_REMOTE_DEVICE_DATA;
#pragma pack (pop)

#pragma pack (push, 1)
typedef struct SV_EVENT_DEVICE_CONNECTION_DATA
{
    int32_t status;
    char deviceID[256];
} SV_EVENT_DEVICE_CONNECTION_DATA;
#pragma pack (pop)

#pragma pack (push, 1)
typedef struct SV_EVENT_DS_ACQUISITION_DATA
{
    int32_t  signalType;
    SV_DS_EVENT_DATA data;
} SV_EVENT_DS_ACQUISITION_DATA;
#pragma pack (pop)

enum SV_EVENT_TYPE_LIST
{
    SV_EVENT_ERROR                      = 1,
    SV_EVENT_REMOTE_DEVICE              = 4,
    SV_EVENT_MODULE                     = 5,
    SV_EVENT_DEVICE_CONNECTION          = 4000,
    SV_EVENT_DS_ACQUISITION             = 5000
};
typedef int32_t  SV_EVENT_TYPE;

typedef enum
{
    SV_intfIValue,       //!> IValue interface
    SV_intfIBase,        //!> IBase interface
    SV_intfIInteger,     //!> IInteger interface
    SV_intfIBoolean,     //!> IBoolean interface
    SV_intfICommand,     //!> ICommand interface
    SV_intfIFloat,       //!> IFloat interface
    SV_intfIString,      //!> IString interface
    SV_intfIRegister,    //!> IRegister interface
    SV_intfICategory,    //!> ICategory interface
    SV_intfIEnumeration, //!> IEnumeration interface
    SV_intfIEnumEntry,   //!> IEnumEntry interface
    SV_intfIPort         //!> IPort interface
} SV_FEATURE_TYPE;

typedef enum
{
    SV_NoCache,              //!< Do not use cache
    SV_WriteThrough,         //!< Write to cache and register
    SV_WriteAround,          //!< Write to register, write to cache on read
    SV_UndefinedCachingMode //!< Not yet initialized
} SV_FEATURE_CACHINGMODE;

typedef enum
{
    SV_Linear,              //!< Slider with linear behavior
    SV_Logarithmic,         //!< Slider with logarithmic behavior
    SV_Boolean,             //!< Check box
    SV_PureNumber,          //!< Decimal number in an edit control
    SV_HexNumber,           //!< Hex number in an edit control
    SV_IPV4Address,         //!< IP-Address
    SV_MACAddress,          //!< MAC-Address
    SV_UndefinedRepresentation,
} SV_FEATURE_REPRESENTATION;

typedef enum
{
    SV_fnAutomatic,                //!> the notation if either scientific or fixed depending on what is shorter
    SV_fnFixed,                    //!> the notation is fixed, e.g. 123.4
    SV_fnScientific,               //!> the notation is scientific, e.g. 1.234e2
    SV_UndefinedEDisplayNotation  //!< Object is not yetinitialized
} SV_FEATURE_DISPLAY_NOTATION;

typedef enum
{
    SV_IMAGE_FILE_PNG,
    SV_IMAGE_FILE_BMP,
    SV_IMAGE_FILE_RAW
} SV_IMAGE_FILE_TYPE;

typedef struct _SV_DEVICE_INFO
{
    SV_INTERFACE_HANDLE hParentIF;
    char uid[SV_STRING_SIZE];
    char vendor[SV_STRING_SIZE];
    char model[SV_STRING_SIZE];
    char displayName[SV_STRING_SIZE];
    char tlType[SV_STRING_SIZE];
    int accessStatus;
    char userDefinedName[SV_STRING_SIZE];
    char serialNumber[SV_STRING_SIZE];
    char version[SV_STRING_SIZE];
    uint64_t timeStampFreq;

    //extra info gev specific
    char ipAddress[SV_STRING_SIZE];
    char subnetMask[SV_STRING_SIZE];
    char macAddress[SV_STRING_SIZE];
}SV_DEVICE_INFO, *PSV_DEVICE_INFO;

typedef struct _SV_INTERFACE_INFO
{
    SV_SYSTEM_HANDLE hParentTL;
    char uid[SV_STRING_SIZE];
    char displayName[SV_STRING_SIZE];
    char tlType[SV_STRING_SIZE];

    //extra info gev specific
    char ipAddress[SV_STRING_SIZE];
    char subnetMask[SV_STRING_SIZE];
    char macAddress[SV_STRING_SIZE];
}SV_INTERFACE_INFO, *PSV_INTERFACE_INFO;

typedef struct _SV_TL_INFO
{
    char id[SV_STRING_SIZE];
    char vendor[SV_STRING_SIZE];
    char model[SV_STRING_SIZE];
    char version[SV_STRING_SIZE];
    char tlType[SV_STRING_SIZE];
    char name[SV_STRING_SIZE];
    char pathName[SV_STRING_SIZE];
    char displayName[SV_STRING_SIZE];
    uint32_t gentlVersionMajor;
    uint32_t gentlVersionMinor;
    int32_t encoding;
}SV_TL_INFO, *PSV_TL_INFO;

typedef struct _SV_DS_INFO
{
    char id[SV_STRING_SIZE];
    uint64_t numDelivered;
    uint64_t numUnderrun;
    size_t numAnnounced;
    size_t numQueued;
    size_t numAwaitDelivery;
    size_t payloadSize;
    bool8_t isGrabbing;
    bool8_t isPayloadSizeDefined;
    char tlType[SV_STRING_SIZE];
    size_t numChunksMax;
    size_t bufAnnounceMin;
    size_t bufAlignment;
} SV_DS_INFO, *PSV_DS_INFO;

typedef struct _SV_FEATURE_INFO
{
    uint8_t type;
    char name[SV_STRING_SIZE];
    char node[SV_STRING_SIZE];
    char displayName[SV_STRING_SIZE];
    char toolTip[SV_STRING_SIZE];
    uint8_t level;
    uint8_t visibility;
    uint8_t isImplemented;
    uint8_t isAvailable;
    uint8_t isLocked;

    //feature specific
    int64_t intMin;
    int64_t intMax;
    int64_t intInc;

    double floatMin;
    double floatMax;
    double floatInc;

    uint8_t representation;
    uint8_t displayNotation;
    int64_t displayPrecision;

    int64_t strMaxLength;

    int32_t enumSelectedIndex;
    int64_t enumCount;

    int64_t pollingTime;

    char unit[SV_STRING_SIZE];
}SV_FEATURE_INFO, *PSV_FEATURE_INFO;

typedef struct _SV_EVENT_INFO
{
    int32_t type;
    size_t numInQueue;
    uint64_t numFired;
    size_t sizeMax;
    size_t infoDataSizeMax;
}SV_EVENT_INFO, *PSV_EVENT_INFO;

typedef enum
{
    SV_LOG_LVL_CRITICAL = 0x00000001,
    SV_LOG_LVL_ERROR = 0x00000002,
    SV_LOG_LVL_WARNING = 0x00000004,
    SV_LOG_LVL_INFO = 0x00000008,
    SV_LOG_LVL_CAMERA_IO = 0x00000010,
    SV_LOG_LVL_DEBUG = 0x00000020,
    SV_LOG_LVL_RESERVED = 0x10000000
}SV_LOG_LEVEL;

// Indicate that pixel is monochrome
#define SV_GVSP_PIX_MONO                       0x01000000
#define SV_GVSP_PIX_RGB                        0x02000000

// Indicate effective number of bits occupied by the pixel (including padding).
// This can be used to compute amount of memory required to store an image.
#define SV_GVSP_PIX_OCCUPY8BIT                 0x00080000
#define SV_GVSP_PIX_OCCUPY12BIT                0x000C0000
#define SV_GVSP_PIX_OCCUPY16BIT                0x00100000
#define SV_GVSP_PIX_OCCUPY24BIT                0x00180000

// Bit masks
#define SV_GVSP_PIX_COLOR_MASK                 0xFF000000
#define SV_GVSP_PIX_EFFECTIVE_PIXELSIZE_MASK   0x00FF0000
#define SV_GVSP_PIX_ID_MASK                    0x0000FFFF

typedef enum
{
    // Unknown pixel format
    SV_GVSP_PIX_UNKNOWN = 0x0000,

    // Mono buffer format defines
    SV_GVSP_PIX_MONO8 = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY8BIT | 0x0001),
    SV_GVSP_PIX_MONO10 = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY16BIT | 0x0003),
    SV_GVSP_PIX_MONO10_PACKED = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY12BIT | 0x0004),
    SV_GVSP_PIX_MONO12 = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY16BIT | 0x0005),
    SV_GVSP_PIX_MONO12_PACKED = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY12BIT | 0x0006),
    SV_GVSP_PIX_MONO16 = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY16BIT | 0x0007),

    // Bayer buffer format defines
    SV_GVSP_PIX_BAYGR8 = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY8BIT | 0x0008),
    SV_GVSP_PIX_BAYRG8 = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY8BIT | 0x0009),
    SV_GVSP_PIX_BAYGB8 = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY8BIT | 0x000A),
    SV_GVSP_PIX_BAYBG8 = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY8BIT | 0x000B),
    SV_GVSP_PIX_BAYGR10 = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY16BIT | 0x000C),
    SV_GVSP_PIX_BAYRG10 = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY16BIT | 0x000D),
    SV_GVSP_PIX_BAYGB10 = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY16BIT | 0x000E),
    SV_GVSP_PIX_BAYBG10 = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY16BIT | 0x000F),
    SV_GVSP_PIX_BAYGR12 = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY16BIT | 0x0010),
    SV_GVSP_PIX_BAYRG12 = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY16BIT | 0x0011),
    SV_GVSP_PIX_BAYGB12 = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY16BIT | 0x0012),
    SV_GVSP_PIX_BAYBG12 = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY16BIT | 0x0013),

    SV_GVSP_PIX_BAYGR10_PACKED = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY12BIT | 0x0026),
    SV_GVSP_PIX_BAYRG10_PACKED = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY12BIT | 0x0027),
    SV_GVSP_PIX_BAYGB10_PACKED = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY12BIT | 0x0028),
    SV_GVSP_PIX_BAYBG10_PACKED = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY12BIT | 0x0029),
    SV_GVSP_PIX_BAYGR12_PACKED = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY12BIT | 0x002A),
    SV_GVSP_PIX_BAYRG12_PACKED = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY12BIT | 0x002B),
    SV_GVSP_PIX_BAYGB12_PACKED = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY12BIT | 0x002C),
    SV_GVSP_PIX_BAYBG12_PACKED = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY12BIT | 0x002D),

    SV_GVSP_PIX_BAYGR16 = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY16BIT | 0x002E),  //  0x0110002E PFNC Bayer Green-Red 16-bit
    SV_GVSP_PIX_BAYRG16 = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY16BIT | 0x002F),  //  0x0110002F PFNC  Bayer Red-Green 16-bit
    SV_GVSP_PIX_BAYGB16 = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY16BIT | 0x0030), // 0x01100030   PFNC Bayer Green-Blue 16-bit
    SV_GVSP_PIX_BAYBG16 = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY16BIT | 0x0031),  // 0x01100031   PFNC  Bayer Blue-Green 16-bit

    // Color buffer format defines
    SV_GVSP_PIX_RGB24 = (SV_GVSP_PIX_RGB | SV_GVSP_PIX_OCCUPY24BIT),

    // Define for a gray image that was converted from a bayer coded image
    SV_GVSP_PIX_GRAY8 = (SV_GVSP_PIX_MONO | SV_GVSP_PIX_OCCUPY8BIT),

} SV_GVSP_PIXEL_TYPE;

enum SV_DEVICE_ACCESS_STATUS_LIST
{
    SV_DEVICE_ACCESS_STATUS_UNKNOWN = 0,   /* The device accessibility is not known. */
    SV_DEVICE_ACCESS_STATUS_READWRITE = 1,   /* The device is available for read/write access. */
    SV_DEVICE_ACCESS_STATUS_READONLY = 2,   /* The device is available for read only access. */
    SV_DEVICE_ACCESS_STATUS_NOACCESS = 3,   /* The device is not accessible. */
    SV_DEVICE_ACCESS_STATUS_BUSY = 4,   /* The device has already been opened by another process/host. GenTL v1.5 */
    SV_DEVICE_ACCESS_STATUS_OPEN_READWRITE = 5,   /* The device has already been opened by this process. GenTL v1.5 */
    SV_DEVICE_ACCESS_STATUS_OPEN_READ = 6,   /* The device has already been opened by this process. GenTL v1.5 */

    SV_DEVICE_ACCESS_STATUS_CUSTOM_ID = 1000 /* Starting value for custom IDs. */
};

enum SV_DEVICE_ACCESS_FLAGS_LIST
{
    SV_DEVICE_ACCESS_UNKNOWN = 0,         /* Not used in a command. Can be used to initialize a variable to query that information. */
    SV_DEVICE_ACCESS_NONE = 1,         /* This either means that the device is not open because it was not opened before or the access to it was denied. */
    SV_DEVICE_ACCESS_READONLY = 2,         /* Open the device read only. All Port functions can only read from the device. */
    SV_DEVICE_ACCESS_CONTROL = 3,         /* Open the device in a way that other hosts/processes can have read only access to the device. Device access level is read/write for this process. */
    SV_DEVICE_ACCESS_EXCLUSIVE = 4,         /* Open the device in a way that only this host/process can have access to the device. Device access level is read/write for this process. */

    SV_DEVICE_ACCESS_CUSTOM_ID = 1000       /*  Starting value for GenTL Producer custom IDs. */
};
typedef int32_t SV_DEVICE_ACCESS_FLAGS;

enum SV_ACQ_START_FLAGS_LIST
{
    SV_ACQ_START_FLAGS_DEFAULT = 0,      /* Default behavior. */
    SV_ACQ_START_FLAGS_CUSTOM_ID = 1000    /* Starting value for GenTL Producer custom IDs. */
};
typedef int32_t SV_ACQ_START_FLAGS;

enum SV_ACQ_STOP_FLAGS_LIST
{
    SV_ACQ_STOP_FLAGS_DEFAULT = 0,         /* Stop the acquisition engine when the currently running tasks like filling a buffer are completed (default behavior). */
    SV_ACQ_STOP_FLAGS_KILL = 1,         /* Stop the acquisition engine immediately and leave buffers currently being filled in the Input Buffer Pool. */

    SV_ACQ_STOP_FLAGS_CUSTOM_ID = 1000       /* Starting value for GenTL Producer custom IDs. */
};
typedef int32_t SV_ACQ_STOP_FLAGS;

enum SV_ACQ_QUEUE_TYPE_LIST
{
    SV_ACQ_QUEUE_INPUT_TO_OUTPUT = 0,    /* Flushes the input pool to the output queue and if necessary adds entries in the New Buffer event data queue. */
    SV_ACQ_QUEUE_OUTPUT_DISCARD = 1,    /* Discards all buffers in the output queue and if necessary remove the entries from the event data queue. */
    SV_ACQ_QUEUE_ALL_TO_INPUT = 2,    /* Puts all buffers in the input pool. Even those in the output queue and discard entries in the event data queue. */
    SV_ACQ_QUEUE_UNQUEUED_TO_INPUT = 3,    /* Puts all buffers that are not in the input pool or the output queue in the input pool. */
    SV_ACQ_QUEUE_ALL_DISCARD = 4,    /* Discards all buffers in the input pool and output queue. */

    SV_ACQ_QUEUE_CUSTOM_ID = 1000  /* Starting value for GenTL Producer custom IDs. */
};
typedef int32_t SV_ACQ_QUEUE_TYPE;

typedef struct _SV_BUFFER_FLAG
{
    union
    {
        uint32_t value;
        struct
        {
            uint8_t newData;
            uint8_t acquiring;
            uint8_t queued;
            uint8_t incomplete;
        };
    };
}SV_BUFFER_FLAG, *PSV_BUFFER_FLAG;

typedef struct _SV_BUFFER_INFO
{
    uint8_t *pImagePtr;
    uint8_t *pUserPtr;
    size_t iSizeX;
    size_t iSizeY;
    size_t iImageSize;
    SV_BUFFER_FLAG flags;
    uint64_t iPixelType;
    uint64_t iImageId;
    uint64_t iTimeStamp;
    uint32_t iReserved2;
    uint32_t iReserved3;
    uint32_t iReserved4;
    uint32_t iReserved5;
    uint32_t iReserved6;
} SV_BUFFER_INFO, *PSV_BUFFER_INFO;

enum SV_TL_TYPE
{
    TL_GEV = 0,
    TL_U3V = 1,
    TL_CL = 2,
};

#if defined (_WIN32)
    typedef void(__stdcall *SV_CB_FEATURE_INVALIDATED_PFN2)(void *pContext, const char* nodeName);
    #if defined (_MSC_VER) || defined (__MINGW64__)
        class SVDummyClass{};
        typedef SVDummyClass *SV_CB_OBJECT;
        typedef void (SVDummyClass::*SV_CB_FEATURE_INVALIDATED_PFN)(const char *featureName);
    #endif
#elif __linux__ || __APPLE__
    typedef void *SV_CB_OBJECT;
    typedef void(*SV_CB_FEATURE_INVALIDATED_PFN2)(void *pContext, const char* nodeName);
#endif

 /** SVLibInit. 
	*  This function must be called prior to any other function call to allow global initialization of SVGenSDK.
	*  Multiple calls to SVLibInit without accompanied calls to SVLibClose will return the error
	*  SV_ERROR_RESOURCE_IN_USE.
	*  If the parameter is NULL SVGenSDK will use the environment variable.  
    *  @param [in] TLIPath (optional) Path containing the SVS CTI's. 
	*  @param [in] genicamRootDir (optional)Path containing the Genicam Dll's needed by SVGenSDK
	*  @param [in] genicamCacheDir (optional) Path containing the Genicam cache dir. If this
	*  NOTE:Application need to have a read/write access on specified folder.
	*  @param [in] clProtocolDriverDir (optional)   Path containing the SVCLProtocolDriver dll's.
	*  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVLibInit(const char *TLIPath = NULL, 
                                            const char *genicamRootDir = NULL,
                                            const char *genicamCacheDir = NULL,
                                            const char *clProtocolDriverDir = NULL);


/** SVLibSystemGetCount
  *  Queries the number of available SVS GenTL Producers on TLIPath directory specified on SVLibInit.
  *  @param [out] tlCount Number of available System module/SVS GenTL Producers.
  *  @return success or error code
*/
EXTERN_C SV_GEN_SDK_API SV_RETURN SVLibSystemGetCount(uint32_t *tlCount);


/** SVLibSystemGetInfo
 *  Queries the information of SVS GenTL Producers.
 *  @param [in] uiIndex Zero-based index of loaded GenTLProducers.
 *  @param[out] pInfoOut Information about the current GenTL Producers
 *  @return success or error code
 */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVLibSystemGetInfo(uint32_t uiIndex, SV_TL_INFO *pInfoOut);


/** SVLibSystemOpen
   *  Opens the System module and puts the instance in the phSystem handle. This allocates all system wide
   *  resources. A System module can only be opened once.
   *  @param [in] uiIndex  Zero-based index of loaded GenTLProducers.
   *  @param [out] phSystemOut System module handle.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVLibSystemOpen(uint32_t uiIndex, SV_SYSTEM_HANDLE *phSystemOut);

/** SVLibClose
   *  Close the library and frees all the allocated resources.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVLibClose();


/** SVSystemGetInfo
   *  Queries the information on System Module.
   *  @param [in] hSystem System module handle from SVLibSystemOpen.
   *  @param [out] pInfoOut Information about the System Module.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVSystemGetInfo(SV_SYSTEM_HANDLE hSystem, SV_TL_INFO *pInfoOut);


/** SVSystemUpdateInterfaceList
   *  Updates the internal list of available interfaces.
   *  @param [in] hSystem System module handle from SVLibSystemOpen.
   *  @param [out] pbChanged (optional) returns true if the internal list was changed and false
   *  otherwise. If set to NULL nothing is written to this parameter.
   *  @param [in] timeOut timeout in ms.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVSystemUpdateInterfaceList(SV_SYSTEM_HANDLE hSystem, bool8_t *pbChanged, uint32_t timeOut);


/** SVSystemGetNumInterfaces
   *  Queries the number of available interfaces on this System module.
   *  @param [in] hSystem System module handle from SVLibSystemOpen.
   *  @param [out] piNumIfaces Number of interfaces on this System module.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVSystemGetNumInterfaces(SV_SYSTEM_HANDLE hSystem, uint32_t *piNumIfaces);


/** SVSystemGetInterfaceId
   *  Queries the ID of the interface at iIndex in the internal interface list .
   *  @param [in] hSystem System module handle from SVLibSystemOpen.
   *  @param [in] Index Number of interfaces on this System module.
   *  @param [in]  pInterfaceId (optional) Pointer to a user allocated C string buffer to receive the Interface
   *  module ID at the given iIndex. If this parameter is NULL, pSize will contain the needed size of sIfaceID in bytes. The size. includes the *  terminating 0.
   *  @param [in,out] pSize size of the Interface id.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVSystemGetInterfaceId(SV_SYSTEM_HANDLE hSystem, uint32_t Index, char* pInterfaceId, size_t *pSize);


/** SVSystemInterfaceGetInfo
   *  Queries the information about the interface on this System module without opening the interface.
   *  @param [in] hSystem System module handle from SVLibSystemOpen.
   *  @param [in] pInterfaceId Id of interface acquired on SVSystemGetInterfaceId.
   *  @param [out] pInfoOut Information about the current Interface.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVSystemInterfaceGetInfo(SV_SYSTEM_HANDLE hSystem,const char *pInterfaceId, SV_INTERFACE_INFO *pInfoOut);


/** SVSystemInterfaceOpen
   *  Opens the given pInterfaceId on the given hSystem.
   *  @param [in] hSystem System module handle from SVLibSystemOpen.
   *  @param [in] pInterfaceId Id of interface acquired on SVSystemGetInterfaceId.
   *  @param [out] phInterfaceOut Interface module handle.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVSystemInterfaceOpen(SV_SYSTEM_HANDLE hSystem, const char *pInterfaceId, SV_INTERFACE_HANDLE *phInterfaceOut);


/** SVSystemClose
   *  Close system module and frees all the resources.
   *  @param [in] hSystem System module handle from SVLibSystemOpen.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVSystemClose(SV_SYSTEM_HANDLE hSystem);


/** SVInterfaceGetInfo
   *  Queries the information about the interface after opening the interface.
   *  @param [in] hInterface Interface module handle from SVSystemInterfaceOpen.
   *  @param [out] pInfoOut Information about the current Interface.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVInterfaceGetInfo(SV_INTERFACE_HANDLE hInterface, SV_INTERFACE_INFO *pInfoOut);


/** SVInterfaceUpdateDeviceList
   *  Updates the internal list of available devices on this interface.
   *  @param [in] hInterface Interface module handle from SVSystemInterfaceOpen.
   *  @param [out] pbChanged (optional) returns true if the internal list was changed and false
   *  otherwise. If set to NULL nothing is written to this parameter.
   *  @param [in] timeOut timeout in ms.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVInterfaceUpdateDeviceList(SV_INTERFACE_HANDLE hInterface, bool8_t *pbChanged, uint32_t timeOut);


/** SVInterfaceGetNumDevices
   *  Queries the number of available devices on this interface.
   *  @param [in] hInterface Interface module handle from SVSystemInterfaceOpen.
   *  @param [out] piDevices Number of Devices on this interface.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVInterfaceGetNumDevices(SV_INTERFACE_HANDLE hInterface, uint32_t *piDevices);


/** SVInterfaceGetDeviceId
   *  Queries the ID of the Device at Index in the internal device list.
   *  @param [in] hInterface System module handle from SVLibSystemOpen.
   *  @param [in] Index Number of interfaces on this System module.
   *  @param [in,out] pDeviceId Pointer to a user allocated C string buffer to receive the Device module ID at the given iIndex.
   *  If this parameter is NULL piSize will contain the needed size of pDeviceId in bytes. The size includes the terminating 0.
   *  @param [in,out] pSize size of the Device id.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVInterfaceGetDeviceId(SV_INTERFACE_HANDLE hInterface, uint32_t Index, char* pDeviceId, size_t *pSize);


/** SVInterfaceDeviceGetInfo
   *  Queries the information about the Device on this Interface module without opening the Device.
   *  @param [in] hInterface Interface module handle from SVSystemInterfaceOpen.
   *  @param [in] pDeviceId Id of Device acquired on SVInterfaceGetDeviceId.
   *  @param [out] pInfoOut Information about the current Device.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVInterfaceDeviceGetInfo(SV_INTERFACE_HANDLE hInterface,const char *pDeviceId, SV_DEVICE_INFO *pInfoOut);


/** SVInterfaceClose
   *  Close the interface and frees all the resources allocated by this module.
   *  @param  [in] hInterface Interface module handle from SVSystemInterfaceOpen.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVInterfaceClose(SV_INTERFACE_HANDLE hInterface);


/** SVInterfaceDeviceOpen
   *  Opens the device with pDeviceId connected on this interface.
   *  @param [in] hInterface Interface module handle from SVSystemInterfaceOpen.
   *  @param [in] pDeviceId Id of Device acquired on SVInterfaceGetDeviceId.
   *  @param [in] accessFlags Configures the open process as defined in the SV_DEVICE_ACCESS_FLAGS.
   *  @param [out] phDeviceOut handle for local Device Module.
   *  @param [out] phRemoteDeviceOut handle for Remote Device.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVInterfaceDeviceOpen(SV_INTERFACE_HANDLE hInterface, 
                                                        const char *pDeviceId,
                                                        SV_DEVICE_ACCESS_FLAGS accessFlags,
                                                        SV_DEVICE_HANDLE *phDeviceOut,
                                                        SV_REMOTE_DEVICE_HANDLE *phRemoteDeviceOut);


/** SVDeviceGetInfo
   *  Queries the information for the Device after opening.
   *  @param [in] hDevice Device module handle from SVInterfaceDeviceOpen.
   *  @param [out] pInfoOut Information about the Device module.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVDeviceGetInfo(SV_DEVICE_HANDLE hDevice, SV_DEVICE_INFO *pInfoOut);


/** SVDeviceGetNumStreams
   *  Queries the number of streams supported by device module.
   *  @param [in] hDevice Device module handle from SVInterfaceDeviceOpen.
   *  @param [out] piStreams Number of streams supported on this device module.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVDeviceGetNumStreams(SV_DEVICE_HANDLE hDevice, uint32_t *piStreams);


/** SVDeviceGetStreamId
   *  Queries the stream id at index.
   *  @param [in] hDevice Device module handle from SVInterfaceDeviceOpen.
   *  @param [in] Index Zero-based index of the Stream on this device.
   *  @param [in,out] pStreamId Pointer to a user allocated C string buffer to receive the Stream module ID at the given iIndex.
   *  If this parameter is NULL, piSize will contain the needed size of pDeviceId in bytes. The size includes the terminating 0.
   *  @param [in,out] pSize size of the Stream id.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVDeviceGetStreamId(SV_DEVICE_HANDLE hDevice, uint32_t Index, char* pStreamId, size_t *pSize);

/** SVDeviceSaveSettings
   * The current streamable settings will be stored in the given file.
   *  @param [in] hDevice Device module handle from SVInterfaceDeviceOpen.
   *  @param [in] fileName a complete path and filename where to save the streamable settings.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVDeviceSaveSettings(SV_DEVICE_HANDLE hDevice, const char *fileName);

/** SVDeviceLoadSettings
   *  load the streamable camera settings.
   *  @param [in] hDevice Device module handle from SVInterfaceDeviceOpen.
   *  @param [in] fileName  a complete path and filename where to load the settings from.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVDeviceLoadSettings(SV_DEVICE_HANDLE hDevice, const char *fileName);


/** SVDeviceRead
   *  Reads a number of bytes from a given iAddress from Remote Device.
   *  @param [in] hDevice Device module handle from SVInterfaceDeviceOpen.
   *  @param [in] nAddress Byte address to read from.
   *  @param [out] pData Pointer to a user allocated byte buffer to receive data; this must not be NULL.
   *  @param [in,out] pSize size of the read data in bytes.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVDeviceRead(SV_DEVICE_HANDLE hDevice, uint64_t nAddress, void *pData, size_t *pSize);


/** SVDeviceWrite
   *  Write a number of bytes from a given iAddress from Remote Device.
   *  @param [in] hDevice Device module handle from SVInterfaceDeviceOpen.
   *  @param [in] nAddress Byte address to write to.
   *  @param [in] pData Pointer to a user allocated byte buffer containing the data to write; this must not be NULL.
   *  @param [in,out] pSize size of the written data in bytes.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVDeviceWrite(SV_DEVICE_HANDLE hDevice, uint64_t nAddress, const void *pData, size_t *pSize);


/** SVDeviceStreamOpen
   *
   *  Opens the given sDataStreamID on the given hDevice.
   *  @param [in] hDevice Device module handle from SVInterfaceDeviceOpen.
   *  @param [in] sDataStreamID Id of Stream module acquired on SVDeviceGetStreamId.
   *  @param [out] phStream handle for the opened stream module.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVDeviceStreamOpen(SV_DEVICE_HANDLE hDevice, const char *sDataStreamID, SV_STREAM_HANDLE *phStream);


/** SVDeviceClose
   *  Close the device module and frees all the resources allocated by this module.
   *  @param [in] hDevice Device module handle from SVInterfaceDeviceOpen.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVDeviceClose(SV_DEVICE_HANDLE hDevice);


/** SVStreamAcquisitionStart
   * Starts the acquisition engine on the host. Each call to SVStreamAcquisitionStart must be accompanied by a call to SVStreamAcquisitionStop.
   *  @param [in] hStream Data Stream module handle from SVDeviceStreamOpen.
   *  @param [in] flags As defined in SV_ACQ_START_FLAGS .
   *  @param [in] iNumToAcquire Number of buffer to be delivered.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVStreamAcquisitionStart(SV_STREAM_HANDLE hStream, SV_ACQ_START_FLAGS flags, uint64_t iNumToAcquire);


/** SVStreamAcquisitionStop
   *  Stops the acquisition engine on the host.
   *  @param [in] hStream Data Stream module handle from SVDeviceStreamOpen.
   *  @param [in] flags As defined in SV_ACQ_STOP_FLAGS.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVStreamAcquisitionStop(SV_STREAM_HANDLE hStream, SV_ACQ_STOP_FLAGS flags);


/** SVStreamAnnounceBuffer
   *  This announces a user allocated memory to the Data Stream associated with the hStream handle and
   *  returns a hBuffer handle which references that single buffer until the buffer is revoked. This will allocate
   *  internal resources which will be freed upon a call to SVStreamRevokeBuffer.
   *  @param [in] hStream Data Stream module handle from SVDeviceStreamOpen.
   *  @param [in] pBuffer Pointer to buffer memory to announce.
   *  @param [in] uiSize Size of the pBuffer in bytes.
   *  @param [in] pPrivate Pointer to private data which user can pass and retrieved on SVStreamWaitForNewBuffer. This parameter may be NULL.
   *  @param [out] phBuffer Buffer module handle for the announce Buffer.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVStreamAnnounceBuffer(SV_STREAM_HANDLE hStream,void *pBuffer, uint32_t uiSize, void *pPrivate, SV_BUFFER_HANDLE *phBuffer);


/** SVStreamAllocAndAnnounceBuffer
   * Allocates the memory and announce to the Data Stream module associated with the hStream handle.
   * This will allocate internal resources which will be freed upon a call to SVStreamRevokeBuffer.
   *  @param [in] hStream Data Stream module handle from SVDeviceStreamOpen.
   *  @param [in] uiSize Size of the buffer in bytes.
   *  @param [in] pPrivate Pointer to private data which user can pass and retrieved on SVStreamWaitForNewBuffer. This parameter may be NULL.
   *  @param [out] phBuffer Buffer module handle for the announce Buffer.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVStreamAllocAndAnnounceBuffer(SV_STREAM_HANDLE hStream, uint32_t uiSize, void *pPrivate, SV_BUFFER_HANDLE *phBuffer);


/** SVStreamRevokeBuffer
   *  Removes an announced buffer from the acquisition engine. This function will free all internally allocated resources associated with this buffer.
   *  @param [in] hStream Data Stream module handle from SVDeviceStreamOpen.
   *  @param [in] hBuffer Buffer handle to revoke.
   *  @param [out] pBuffer Pointer to the buffer memory if buffer is announce usingSVStreamAnnounceBuffer.
   *  This value is NULL if buffer was announced using SVStreamAllocAndAnnounceBuffer.
   *  @param [out] pPrivate Pointer to private data which is pass upon SVStreamAnnounceBuffer or SVStreamAllocAndAnnounceBuffer.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVStreamRevokeBuffer(SV_STREAM_HANDLE hStream, SV_BUFFER_HANDLE hBuffer, void **pBuffer, void **pPrivate);


/** SVStreamQueueBuffer
   * Queues a particular buffer for acquisition. Unqueued buffer will not be used by the SDK and thus effectively locking it.
   *  @param [in] hStream Data Stream module handle from SVDeviceStreamOpen.
   *  @param [in] hBuffer Buffer handle to queue.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVStreamQueueBuffer(SV_STREAM_HANDLE hStream, SV_BUFFER_HANDLE hBuffer);


/** SVStreamGetBufferId
   *  Queries the buffer handle for the give iIndex.
   *  @param [in] hStream Data Stream module handle from SVDeviceStreamOpen
   *  @param [in] iIndex Zero-based index of the buffer on this data stream.
   *  @param [out] phBuffer Buffer handle of the given index.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVStreamGetBufferId(SV_STREAM_HANDLE hStream, uint32_t iIndex, SV_BUFFER_HANDLE *phBuffer);


/** SVStreamFlushQueue
   *  Flushes the Buffer queue by iOperation.
   *  @param [in] hStream Data Stream module handle from SVDeviceStreamOpen.
   *  @param [in] iOperation Flush operation to perform as defined in SV_ACQ_QUEUE_TYPE.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVStreamFlushQueue(SV_STREAM_HANDLE hStream, SV_ACQ_QUEUE_TYPE iOperation);


/** SVStreamWaitForNewBuffer
   *  Waits for the new buffer event.
   *  @param [in] hStream Data Stream module handle from SVDeviceStreamOpen.
   *  @param [out] ppUserData Pointer of the pPrivateData parameter passed on SVStreamAnnounceBuffer or SVStreamAllocAndAnnounceBuffer.
   *  @param [out] phBufferOut Buffer handle.
   *  @param [in] timeOut Timeout in ms.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVStreamWaitForNewBuffer(SV_STREAM_HANDLE hStream, void **ppUserData, SV_BUFFER_HANDLE *phBufferOut,uint32_t timeOut);


/** SVStreamBufferGetInfo
   *  Queries the information of hBuffer on hStream.
   *  @param [in] hStream Data Stream module handle from SVDeviceStreamOpen.
   *  @param [in] hBuffer Buffer handle.
   *  @param [out] pInfoOut Information about the current Buffer.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVStreamBufferGetInfo(SV_STREAM_HANDLE hStream, SV_BUFFER_HANDLE hBuffer,SV_BUFFER_INFO *pInfoOut);


/** SVStreamClose
   *  Close the stream module and frees all the resources it allocated including the Buffers.
   *  @param [in] hStream Data Stream module handle from SVDeviceStreamOpen.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVStreamClose(SV_STREAM_HANDLE hStream);


/** SVFeatureGetByName
   *  Get the handle for the features/node on hModule by name.
   *  @param [in] hModule Module handle.
   *  @param [in] featureName Feature/Node name to get.
   *  @param [out] phFeature feature handle.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureGetByName(void *hModule, const char *featureName, SV_FEATURE_HANDLE *phFeature);


/** SVFeatureGetByIndex
   *  Get the handle for the features/node on hModule by Index.
   *  @param [in] hModule Module handle.
   *  @param [in] iIndex Zero base index of features on current module.
   *  @param [out] phFeature feature handle.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureGetByIndex(void *hModule, uint32_t iIndex, SV_FEATURE_HANDLE *phFeature);


/** SVFeatureGetInfo
   *  Get the Feature Info on  hModule for a given Feature handle.
   *  @param [in] hModule Module handle.
   *  @param [in] hFeature Feature handle from SVFeatureGetByIndex or SVFeatureGetByName.
   *  @param [out] pInfoOut Information about the current features/node.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureGetInfo(void *hModule, SV_FEATURE_HANDLE hFeature, SV_FEATURE_INFO *pInfoOut);


/** SVFeatureGetValueBool
   *  Get the value of Feature with type SV_intfIBoolean.
   *  @param [in] hModule Module handle.
   *  @param [in] hFeature Feature handle from SVFeatureGetByIndex or SVFeatureGetByName.
   *  @param [out] pBoolValue Boolean Value of hFeature.
   *  @param [in] verify (optional) Enable range verification.
   *  @param [in] ignoreCache (optional) Ignore cache and read directly from the module when set to true.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureGetValueBool(void *hModule, SV_FEATURE_HANDLE hFeature, bool8_t *pBoolValue, bool verify = false, bool ignoreCache = false);


/** SVFeatureGetValueInt64
   *  Get the value of Feature with type SV_intfIInteger.
   *  @param [in] hModule Module handle.
   *  @param [in] hFeature Feature handle from SVFeatureGetByIndex or SVFeatureGetByName.
   *  @param [out] pInt64Value int64_t Value of hFeature.
   *  @param [in] verify (optional) Enable range verification.
   *  @param [in] ignoreCache (optional) Ignore cache and read directly from the module when set to true.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureGetValueInt64(void *hModule, SV_FEATURE_HANDLE hFeature, int64_t *pInt64Value, bool verify = false, bool ignoreCache = false);


/** SVFeatureGetValueFloat
   *  Get the value of Feature with type SV_intfIFloat.
   *  @param [in] hModule Module handle.
   *  @param [in] hFeature Feature handle from SVFeatureGetByIndex or SVFeatureGetByName.
   *  @param [out] pFloatValue double Value of hFeature.
   *  @param [in] verify (optional) Enable range verification.
   *  @param [in] ignoreCache (optional) Ignore cache and read directly from the module when set to true.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureGetValueFloat(void *hModule, SV_FEATURE_HANDLE hFeature, double *pFloatValue, bool verify = false, bool ignoreCache = false);


/** SVFeatureGetValueString
   *  Get the value of Feature with type SV_intfIString.
   *  @param [in] hModule Module handle.
   *  @param [in] hFeature Feature handle from SVFeatureGetByIndex or SVFeatureGetByName.
   *  @param [out] strValue String Value of hFeature.
   *  @param [in] bufferSize Size in bytes of strValue.
   *  @param [in] verify (optional) Enable range verification.
   *  @param [in] ignoreCache (optional) Ignore cache and read directly from the module when set to true.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureGetValueString(void *hModule, SV_FEATURE_HANDLE hFeature, char *strValue, uint32_t bufferSize, bool verify = false, bool ignoreCache = false);


/** SVFeatureGetValueInt64Enum
   *  Get the value of Feature with type SV_intfIEnumeration.
   *  @param [in] hModule Module handle.
   *  @param [in] hFeature Feature handle from SVFeatureGetByIndex or SVFeatureGetByName.
   *  @param [out] pInt64Value int64_t Value of hFeature.
   *  @param [in] verify (optional) Enable range verification.
   *  @param [in] ignoreCache (optional) Ignore cache and read directly from the module when set to true.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureGetValueInt64Enum(void *hModule, SV_FEATURE_HANDLE hFeature, int64_t *pInt64Value, bool verify = false, bool ignoreCache = false);


/** SVFeatureSetValueBool
   *  Set the value of Feature with type SV_intfIBoolean.
   *  @param [in] hModule Module handle.
   *  @param [in] hFeature Feature handle from SVFeatureGetByIndex or SVFeatureGetByName.
   *  @param [in] boolValue bool8_t Value to set.
   *  @param [in] verify (optional) Enable access mode and range verification.
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureSetValueBool(void *hModule, SV_FEATURE_HANDLE hFeature, const bool8_t boolValue, bool verify = false);


/** SVFeatureSetValueInt64
   *  Set the value of Feature with type SV_intfIInteger.
   *  @param [in] hModule Module handle.
   *  @param [in] hFeature Feature handle from SVFeatureGetByIndex or SVFeatureGetByName.
   *  @param [in] int64Value int64_t Value to set.
   *  @param [in] verify (optional) Enable access mode and range verification.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureSetValueInt64(void *hModule, SV_FEATURE_HANDLE hFeature, const int64_t int64Value, bool verify = false);


/** SVFeatureSetValueFloat
   *  Set the value of Feature with type SV_intfIFloat.
   *  @param [in] hModule Module handle.
   *  @param [in] hFeature Feature handle from SVFeatureGetByIndex or SVFeatureGetByName.
   *  @param [in] floatValue double Value to set.
   *  @param [in] verify (optional) Enable access mode and range verification.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureSetValueFloat(void *hModule, SV_FEATURE_HANDLE hFeature, const double floatValue, bool verify = false);


/** SVFeatureSetValueString
   *  Set the value of Feature with type SV_intfIString.
   *  @param [in] hModule Module handle.
   *  @param [in] hFeature Feature handle from SVFeatureGetByIndex or SVFeatureGetByName.
   *  @param [in] strValue String Value to set.
   *  @param [in] verify (optional) Enable access mode and range verification.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureSetValueString(void *hModule, SV_FEATURE_HANDLE hFeature, const char *strValue, bool verify = false);


/** SVFeatureSetValueInt64Enum
   *  Set the value of Feature with type SV_intfIEnumeration.
   *  @param [in] hModule Module handle.
   *  @param [in] hFeature Feature handle from SVFeatureGetByIndex or SVFeatureGetByName.
   *  @param [in] int64Value int64_t Value to set.
   *  @param [in] verify (optional) Enable access mode and range verification.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureSetValueInt64Enum(void *hModule, SV_FEATURE_HANDLE hFeature, const int64_t int64Value, bool verify = false);


/** SVFeatureCommandExecute
   *  Execute the Feature with type SV_intfICommand.
   *  @param [in] hModule Module handle.
   *  @param [in] hFeature Feature handle from SVFeatureGetByIndex or SVFeatureGetByName.
   *  @param [in] Timeout Timeout in MS.
   *  @param [in] bWait Ignores timeout and returns immediately when set to false. Wait until command is executed or Timeout expires when set to true
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureCommandExecute(void *hModule, SV_FEATURE_HANDLE hFeature, uint32_t Timeout, bool bWait = false);


/** SVFeatureEnumSubFeatures
   *  Enumerate all the Subfeatures of Feature with type SV_intfIEnumeration.
   *  @param [in]  hModule Module handle.
   *  @param [in]  hFeature Feature handle from SVFeatureGetByIndex or SVFeatureGetByName.
   *  @param [in]  iIndex Zero based index of the child/sub features of hFeature.
   *  @param [out] subFeatureName Name of the subfeature on iIndex.
   *  @param [in] bufferSize Buffer size in bytes of subFeatureName.
   *  @param [out] pValue (optional) Value of the subfeature.
   *  This will be the value to set on SVFeatureSetValueInt64Enum when updating the hFeature.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureEnumSubFeatures(void *hModule, SV_FEATURE_HANDLE hFeature, int32_t iIndex, char *subFeatureName, unsigned int bufferSize, int64_t *pValue=NULL);


/** SVFeatureRegisterInvalidateCB
   *  Register callback to be called when hFeature is invalidated.
   *  @param [in] hModule Module handle.
   *  @param [in] hFeature Feature handle from SVFeatureGetByIndex or SVFeatureGetByName.
   *  @param [in] objCb Callback object.This can't be NULL.
   *  @param [in] pfnFeatureInvalidateCb Callback function.This can't be NULL.
   *  @return success or error code
  */

#if defined (_WIN32) && (_MSC_VER) //only for windows and vs compiler otherwise use SVFeatureRegisterInvalidateCB2
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureRegisterInvalidateCB(void *hModule, SV_FEATURE_HANDLE hFeature, SV_CB_OBJECT objCb, SV_CB_FEATURE_INVALIDATED_PFN pfnFeatureInvalidateCb);
#endif

/** SVFeatureUnRegisterInvalidateCB
   *  Unregister callback function registered on SVFeatureRegisterInvalidateCB
   *  @param [in] hModule Module handle.
   *  @param [in] hFeature Feature handle from SVFeatureGetByIndex or SVFeatureGetByName.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureUnRegisterInvalidateCB(void *hModule, SV_FEATURE_HANDLE hFeature);


/** SVUtilBuffer12BitTo8Bit
   *  Utility function to convert 12 bit Image format to 8 Bit Mono
   *  @param [in] srcInfo Information of the source Image to convert.
   *  @param [out] pDest Pointer to user allocated buffer to receive the converted buffer.
   *  @param [in] pDestLength Size in bytes of the buffer passed on pDest.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVUtilBuffer12BitTo8Bit(SV_BUFFER_INFO srcInfo, unsigned char *pDest, int pDestLength);


/** SVUtilBuffer12BitTo16Bit
   *  Utility function to convert 12 bit Image format to 16 Bit Mono.
   *  @param [in] srcInfo Information of the source Image to convert.
   *  @param [out] pDest Pointer to user allocated buffer to receive the converted buffer.
   *  @param [in] pDestLength Size in bytes of the buffer passed on pDest.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVUtilBuffer12BitTo16Bit(SV_BUFFER_INFO srcInfo, unsigned char *pDest, int pDestLength);


/** SVUtilBuffer16BitTo8Bit
   *  Utility function to convert 16 bit Image format to 8 Bit Mono.
   *  @param [in] srcInfo Information of the source Image to convert.
   *  @param [out] pDest Pointer to user allocated buffer to receive the converted buffer.
   *  @param [in] pDestLength Size in bytes of the buffer passed on pDest.
   *  @return success or error code
 */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVUtilBuffer16BitTo8Bit(SV_BUFFER_INFO srcInfo, unsigned char *pDest, int pDestLength);


/** SVUtilBufferBayerToRGB
   * Utility function to convert BAYXX8 or BAYXX12 Image format to 24 bit RGB
   *  @param [in] srcInfo Information of the source Image to convert.
   *  @param [out] pDest Pointer to user allocated buffer to receive the converted buffer.
   *  @param [in] pDestLength Size in bytes of the buffer passed on pDest.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVUtilBufferBayerToRGB(SV_BUFFER_INFO srcInfo, unsigned char *pDest, int pDestLength); //TBD add parameter for debayer

/** SVLogRegister
   *  Log messages can be requested for various log levels:
   *   SV_LOG_LVL_CRITICAL
   *   SV_LOG_LVL_ERROR
   *   SV_LOG_LVL_WARNING
   *   SV_LOG_LVL_INFO
   *   SV_LOG_LVL_CAMERA_IO
   *   SV_LOG_LVL_DEBUG
   *
   *  @param [in] moduleName module name that will show on left most side of log traces.
   *  @param [in] debugLevel debug level to set on specific module.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVLogRegister(const char *moduleName, const unsigned int debugLevel);


/** SVLogEnableWindbg
   *  allows enabling or disabling system logging.
   *  @param [in] bEnable flag to enable or disable system logging.
   *  @return success or error code
   */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVLogEnableWindbg(bool bEnable);


/** SVLogEnableFileLogging
   *  Resulting log messages can be written into a log file if enabled.
   *  @param [in] bEnable flag to enable or disable file writing
   *  @param [in] logFileName Log file name to be generated. Must have read/write access.
   *  it can be set to NULL when disabling File Logging.
   *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVLogEnableFileLogging(bool bEnable,const char *logFileName);


/** SVLogSetGlobalDebugLevel
   *  Set Global debug Level. This will overwrite module specific debug level set from SVLogRegister.
   *  @param [in] debugLevel debug level to be set.
   *  @return success or error code
   */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVLogSetGlobalDebugLevel(const unsigned int debugLevel);


/** SVLogTraceA
  *  Printf like function for logging debug traces.
  *  @param [in] moduleName module name registered on SVLogRegister.
  *  @param [in] debugLevel debug level of the debug trace.
  *  @param [in] format printf like string formatting args
  *  @return success or error code
 */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVLogTraceA(const char *moduleName, const unsigned int debugLevel, const char *format, ...);

/** SVUtilBufferBayerToRGB Utility function to convert BAYXX8 or BAYXX12 Image format to 32 bit RGB 
  *  @param[in] srcInfo Information of the source Image to convert.
  *  @param[out] pDest Pointer to user allocated buffer to receive the converted buffer.
  *  @param[in] pDestLength Size in bytes of the buffer passed on pDest.
  *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVUtilBufferBayerToRGB32(SV_BUFFER_INFO srcInfo, unsigned char *pDest, int pDestLength);


/** isVersionCompliant. The DLL's version at compile time will be checked against an expected version at runtime. 
  *  @param[in] expectedVersion a pointer to a version structure with the expected DLL version
  *  @param[out] pCurrentVersion a pointer to a version structure for the current DLL version
  *  @return success or error code
 */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVLibIsVersionCompliant(const SV_LIB_VERSION expectedVersion, SV_LIB_VERSION *pCurrentVersion);


/** SVFeatureGetValueEnum Get feature enumeration value as string. 
  *  @param[in] hModule Module handle.
  *  @param[in] hFeature Feature handle from SVFeatureGetByIndex or SVFeatureGetByName.
  *  @param[in,out] buffer Pointer to a user allocated string buffer for the enumeration value.
  *  @param[in] bufferSize size of StringBuffer for the enumeration value.
  *  @param[in] verify (optional) Enable range verification.
  *  @param[in] ignoreCache (optional) Ignore cache and read directly from the module when set to true.
  *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureGetValueEnum(void *hModule, SV_FEATURE_HANDLE hFeature, char *buffer, unsigned int bufferSize, bool verify = false, bool ignoreCache = false);

/** SVFeatureSetValueEnum Set feature enumeration value as string. 
  *  @param[in] hModule Module handle.
  *  @param[in] hFeature Feature handle from SVFeatureGetByIndex or SVFeatureGetByName.
  *  @param[out] buffer enumeration value to set.
  *  @param[in] verify (optional) Enable range verification.
  *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureSetValueEnum(void *hModule, SV_FEATURE_HANDLE hFeature, const char *buffer, bool verify = false);


/** SVDeviceSaveSettingsToString The current streamable settings will be stored in StringBuffer. 
  *  @param[out] hDevice Device module handle from SVInterfaceDeviceOpen.
  *  @param[in,out] buffer pointer to the user allocated StringBuffer where to receive the streamable settings.
  *  @param[in,out] pBufferSize pointer to the size of StringBuffer.  (To get the correct size for buffer allocation set first the buffer to null, after getting the correct size you can allocate the buffer and save settings)
  *  @return success or error code
 */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVDeviceSaveSettingsToString(SV_DEVICE_HANDLE hDevice, char *buffer, uint32_t *pBufferSize);


/** SVDeviceLoadSettingsFromString load the streamable camera settings from the given StringBuffer. 
  *  @param[in] hDevice Device module handle from SVInterfaceDeviceOpen.
  *  @param[in] buffer a StringBuffer where to load the settings from.
  *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVDeviceLoadSettingsFromString(SV_DEVICE_HANDLE hDevice, const char *buffer);

/** SVFeatureListRefresh refresh the feature list. 
  *  @param[in] hModule Module handle
  *  @return success or error code
 */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureListRefresh(void *hModule);


/** SVUtilSaveImageToPNGFile Write image as a PNG file to disk. 
  *  @param[out] info Information about the current Buffer.
  *  @param[in] fileName a path for the PNG file
  *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVUtilSaveImageToPNGFile(const SV_BUFFER_INFO &info, const char* fileName); //Deprecate

/** SVFeatureGetDescription Get the description of Feature with the a specific Feature handle. 
  *  @param[in] hModule Module handle.
  *  @param[in] hFeature Feature handle from SVFeatureGetByIndex or SVFeatureGetByName.
  *  @param[in,out] pBuffer buffer pointer to the user allocated StringBuffer where to receive the feature description.
  *  @param[in] bufferSize size of StringBuffer.
  *  @return success or error code
 */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureGetDescription(void *hModule, SV_FEATURE_HANDLE hFeature, char *pBuffer, uint32_t bufferSize);

/** SVFeatureRegisterInvalidateCB2 Register callback to be called when hFeature is invalidated. 
  *  @param[in] hModule Module handle.
  *  @param[in] hFeature Feature handle from SVFeatureGetByIndex or SVFeatureGetByName.
  *  @param[in] pContext Callback context.This can't be NULL ( a pointer of void is used instead of objCb Callback).
  *  @param[in] pfnFeatureInvalidateCb" Callback function.This can't be NULL.
  *  @return success or error code
 */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVFeatureRegisterInvalidateCB2(void *hModule, SV_FEATURE_HANDLE hFeature, void *pContext, SV_CB_FEATURE_INVALIDATED_PFN2 pfnFeatureInvalidateCb);


/** SVEventRegister Registers an event object to a iEventID
  *  @param[in] hModule Module handle.
  *  @param[in] iEventID Event type to register.
  *  @param[out] phEvent Event handle.
  *  @return success or error code
 */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVEventRegister(void *hModule, SV_EVENT_TYPE iEventID, SV_EVENT_HANDLE *phEvent);

/** SVEventUnRegister unregister the given iEventID from  the given hModule
  *  @param[in] hModule Module handle.
  *  @param[in] iEventID Event type to unregister.
  *  @return success or error code
 */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVEventUnRegister(void *hModule, SV_EVENT_TYPE iEventID);


/** SVEventGetInfo
  *  Retrieves information from the given hEvent
  *  @param[in] hModule Module handle.
  *  @param[in] hEvent Event handle to retrieve info from.
  *   @param[out] pEventInfo Event Info as defined in SV_EVENT_INFO.
  *  @return success or error code
 */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVEventGetInfo(void *hModule, SV_EVENT_HANDLE hEvent, SV_EVENT_INFO *pEventInfo);


/** SVEventWait
  *  wait for Event Data in given timeout.
  *  @param[in] hModule Module handle.
  *  @param[in] hEvent event handle to wait for.
  *  @param[out] pBuffer Event Data as defined in SV_EVENT_DS_ACQUISITION_DATA,SV_EVENT_DEVICE_CONNECTION_DATA,SV_EVENT_REMOTE_DEVICE_DATA.
  *  @param[in,out] pSize Size of the provided  pBuffer in bytes.
  *  @param[in] iTimeout Timeout for the wait in ms.
  *  @return success or error code
 */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVEventWait(void *hModule, SV_EVENT_HANDLE hEvent, void *pBuffer, size_t *pSize, uint64_t iTimeout);

/** SVEventFlush
  *  Flushes all events in the given hEvent object.
  *  @param[in] hModule Module handle.
  *  @param[in] hEvent Event handle to flush.
  *  @return success or error code
 */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVEventFlush(void *hModule, SV_EVENT_HANDLE hEvent);

/** SVEventKill
  *  terminate event waiting operation.
  *  @param[in] hModule Module handle.
  *  @param[in] hEvent Event handle to kill.
  *  @return success or error code
 */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVEventKill(void *hModule, SV_EVENT_HANDLE hEvent);

/** SVUtilSaveImageToFile Write image with the given file type to disk. 
  *  @param[in] info Information about the current Buffer.
  *  @param[in] fileName a path for the file
  *  @param[in] fileType  image file type as described in the structure SV_IMAGE_FILE_TYPE.
  *  @return success or error code
  */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVUtilSaveImageToFile(const SV_BUFFER_INFO &info, const char* fileName, SV_IMAGE_FILE_TYPE fileType);

/** SVStreamGetInfo
   *  Queries the information on Stream Module.
   *  @param [in]  hStream Data Stream module handle from SVDeviceStreamOpen.
   *  @param [out] pInfoOut Information about the Stream Module.
   *  @return success or error code
   */
EXTERN_C SV_GEN_SDK_API SV_RETURN SVStreamGetInfo(SV_STREAM_HANDLE hStream, SV_DS_INFO *pInfoOut);

#endif
