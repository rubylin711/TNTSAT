///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  CAVRendererGst.cpp
//
//
///////////////////////////////////////////////////////////////////////////////


//=============================================================================
// I N C L U D E     F I L E S
//=============================================================================

#include <pkPAL.h>
#include <memory.h>
#include <string.h>

#include <pkExecutive.h>

#include <palPrint.h>
#include <platPrivate.h>

#include "CAVRendererGst.h"

// -------------------------------------------------------------------
// Renderer errors that are used by the PAL.
// -------------------------------------------------------------------

#define pkE_RENDERER_BASE_ERROR_CODE                ((pkRESULT) 0xC0000000)
#define pkE_RENDERER_STREAM_STARVED                 ( pkE_RENDERER_BASE_ERROR_CODE + 1 )
#define pkE_RENDERER_STREAM_ERROR_RECOVERABLE       ( pkE_RENDERER_BASE_ERROR_CODE + 2 )
#define pkE_RENDERER_PROPERTY_NOT_FOUND             ( pkE_RENDERER_BASE_ERROR_CODE + 3 )
#define pkE_RENDERER_STREAM_TYPE_NOT_SUPPORTED      ( pkE_RENDERER_BASE_ERROR_CODE + 4 )
#define pkE_RENDERER_STREAM_ERROR_FLUSH_REQUIRED    ( pkE_RENDERER_BASE_ERROR_CODE + 5 )
#define pkE_RENDERER_STREAM_ERROR_TIMEOUT           ( pkE_RENDERER_BASE_ERROR_CODE + 6 )
#define pkE_RENDERER_STREAM_ERROR_SEEK              ( pkE_RENDERER_BASE_ERROR_CODE + 7 )
#define pkE_RENDERER_STREAM_ERROR_POSQUERY          ( pkE_RENDERER_BASE_ERROR_CODE + 8 )

//
// The GStreamer API version 0.10.11 and earlier use the word "interface" for
// the name of the parameter in one of the library's function's prototypes. In
// modern C and C++ compilers, that word is a reserved keyword and can cause
// a compilation error. This version of GStreamer is the latest available version
// on Fedora Core 6, which is what is installed on our build machine, so we
// need to work around this. The work around is to #define it in the preprocessor
// so that the compiler itself never sees it.
//
// Note that on newer installations of Fedora, like Fedora 8, GStreamer 0.10.14
// is available, and the issue is fixed. This #define won't have any negative
// effect in those cases.
//

#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>
#include <gst/base/gstbasesrc.h>
#include <gst/audio/multichannel.h>

#include <X11/Xlib.h>

//=============================================================================
// Muting notification
//=============================================================================

static pkRESULT _SetMuting_priv(int isMuted)
{
    return pkS_OK;
}

// ================================================================
// Local helpers
// ================================================================

#ifdef PRINTMSG_ENABLED

#define CRC32_INITIALIZE    0xffffffff

//
// Calculate a CRC 32 value of a byte buffer.
//
// Arguments:
// [dwCRC]      Starting value of the CRC 32 value (start with CRC32_INITIALIZE)
// [pbBuffer]   The byte buffer used to calculate the CRC 32.
// [dwLength]   Size of the buffer mentioned above.
//
// Returns:
// The CRC 32 value of the buffer.
//
static uint32_t s_ComputeCRC32( uint32_t dwCRC, const uint8_t* pbBuffer,  size_t dwLength )
{
    const uint8_t* pbEnd = pbBuffer + dwLength;

    while ( pbBuffer != pbEnd )
    {
        uint8_t value = *pbBuffer++;
        uint32_t masking = 1 << 8;
        while ( ( masking >>= 1 ) )
        {
            uint32_t carry = dwCRC & 0x80000000;
            dwCRC <<= 1;
            if( ! carry ^ ! ( value & masking ) ) 
            {
                dwCRC ^= 0x04c11db7; // CRC32_POLYNOMIAL
            }
        }
    }
    return dwCRC;
}

#endif // PRINTMSG_ENABLED

//===========================================================================
// command line options
//===========================================================================

// buffer size configurable parameters:

static unsigned int s_MAX_VIDEO_BUFFER_TIME_SECS = 8; // to match video 8 second SSPK default
static unsigned int s_MAX_AUDIO_BUFFER_TIME_SECS = 1; // to match audio 1 second SSPK default

static unsigned int s_MAX_VIDEO_BYTERATE =  (1024 * 1024); // 8 mbps
static unsigned int s_MAX_AUDIO_BYTERATE = (24 * 1024); // 192 kbps

// Maximum encoded size of a single frame
static unsigned int s_MAX_VIDEO_FRAME_SIZE = (1024*1024);

// buffer size override command line options:

#define CMDOPT_MAX_VIDEO_BUFFER_TIME_SECS "-gstVBT"
#define CMDOPT_MAX_AUDIO_BUFFER_TIME_SECS "-gstABT"
#define CMDOPT_MAX_VIDEO_BYTERATE         "-gstVBR"
#define CMDOPT_MAX_AUDIO_BYTERATE         "-gstABR"
#define CMDOPT_MAX_VIDEO_FRAME_SIZE       "-gstVFS"

// codec override command line options:

#define CMDOPT_WMA                   "-wma"
#define CMDOPT_AUDIOSINK             "-audiosink"
#define CMDOPT_MP3                   "-mp3"
#define CMDOPT_WMV                   "-wmv"
#define CMDOPT_MPEG2                 "-mpeg2"
#define CMDOPT_H264                  "-h264"
#define CMDOPT_AAC                   "-aac"

//===========================================================================
// Static variables
//===========================================================================
static int s_argc = 0;
static char **s_argv = NULL;

//===========================================================================
// Basic functions
//===========================================================================
//
// Function: Renderer_SetCmdLineArgs_priv
//
// Purpose: Store/init the command-line args so that subsequent calls to
//          parse the command-line args may succeed.
//
// Arguments:
//   argc [in] - Count of command-line arguments, including the name of
//               the executable. Identical to C-language main(argc).
//   argv [in] - Array of char strings, each representing a cmd-line
//               argument. Identical to C-language main(argv). This includes
//               the assumption that the char strings will be available during
//               the entire execution session.
//
// Returns:
//   Nothing.
//
void Renderer_SetCmdLineArgs_priv(int argc, char **argv)
{
    // Confirm that the argc, argv passed in matches C-language main defn
    pkASSERT(argc >= 1);
    pkASSERT(argv != NULL);
    pkASSERT(argv[argc] == NULL);

    s_argc = argc;
    s_argv = argv;
}


//
// Function: Renderer_ClearCmdLineArgs_priv
//
// Purpose: Null out state info so that subsequent calls to parse cmd-line args
//          will fail. This helps app developers to prohibit initialization
//          after a certain point in the program flow.
//
// Arguments: None.
//
// Returns:
//   Nothing.
//
void Renderer_ClearCmdLineArgs_priv(void)
{
    s_argc = 0;
    s_argv = NULL;
}

// ===========================================================
namespace CAVRendererGst
{


//
// Function: _GetCmdLineArg
//
// Purpose: Returns the command-line argument for the given switch.
//
// Arguments:
//   pszSwitch [in] - Case-sensitive string, including dash prefix, which describes
//                    the switch we are looking for (eg. "-input"). The full string
//                    must be matched, abbreviations are not considered matches.
//   iPos [in] - relative position of the argument you would like returned. For
//               example, given cmd line "-input one two three", iPos=1 returns
//               "one", while iPos=3 returns "three". iPos=0 returns *ppArg = NULL
//               and is used to check existence of a switch, with no args.
//   ppArg [out, optional] - Pointer to string containing the requested cmd-line argument.
//
// Returns:
//   pkS_OK means the command-line argument argument was found and returned.
//   pkS_FALSE means the command-line argument was not found.
//   Otherwise, error code indicating fatal error.
//
static pkRESULT _GetCmdLineArg(const char *pszSwitch, int iPos, const char **ppArg)
{
    pkRESULT pkRes = pkS_OK;
    bool_t fFoundSwitch = FALSE;
    int iSwitch;
    int i;

    if (ppArg)
        *ppArg = NULL;

    if (0 == s_argc || NULL == s_argv)
    {
        pkRes = pkE_UNEXPECTED;
        goto exit;
    }

    if (NULL == pszSwitch)
    {
        pkRes = pkE_INVALIDARG;
        goto exit;
    }

    // First, see if the requested switch exists
    for (iSwitch = 0; iSwitch < s_argc; iSwitch++)
    {
        if (0 == strcmp(s_argv[iSwitch], pszSwitch))
        {
            fFoundSwitch = TRUE;
            break;
        }
    }

    if (FALSE == fFoundSwitch)
    {
        pkRes = pkS_FALSE;
        goto exit;
    }

    // Check if this is a no-argument switch
    if (0 == iPos)
    {
        pkRes = pkS_OK;
        goto exit;
    }

    // Skip the requested number of arguments
    // Does the skip request go past end of command line?
    if (iSwitch + iPos >= s_argc)
    {
        pkRes = pkS_FALSE;
        goto exit;
    }

    // Nope, not past end of command line. Check if another switch starts before we
    // reach the requested position
    for (i = 1; i <= iPos; i++)
    {
        if (s_argv[iSwitch + i][0] == '-')
        {
            // We encountered a command-line switch before reaching the requested pos
            pkRes = pkS_FALSE;
            goto exit;
        }
    }

    // If we reached this point, we have found the requested switch (eg. "-input")
    // and the requested cmd-line argument position (eg. iPos = 3) without finding
    // any new switches in between. Return the argument.
    if (ppArg)
        *ppArg = s_argv[iSwitch + iPos];

    pkRes = pkS_OK;

exit:
    return pkRes;
}


//
// Function: _GetCmdLineArg_UInt
//
// Purpose: Stores the command-line unsigned int argument for the given switch.
//
// Arguments:
//   pszSwitch [in] - Case-sensitive string, including dash prefix, which describes
//                    the switch we are looking for (eg. "-input"). The full string
//                    must be matched, abbreviations are not considered matches.
//   pUIResult [out] - Pointer to unsigned int containing the requested cmd-line argument value.
//
// Returns:
//   pkS_OK means the command-line argument argument was found and value returned.
//   pkS_FALSE means the command-line argument was not found.
//   Otherwise, error code indicating fatal error.
//
static pkRESULT _GetCmdLineArg_UInt(const char *pszSwitch, unsigned int *pUIResult)
{
    const char *pszOpt;
    long longValue;

    pkRESULT pkResult = _GetCmdLineArg(pszSwitch, 1, &pszOpt);

    if(pkS_OK == pkResult)
    {
        longValue = atol(pszOpt);
        if (longValue > 0)
        {
            *pUIResult = (unsigned int) longValue;
        }
        else
        {
            pkResult = pkS_FALSE;
        }
    }
    return pkResult;
}


//=============================================================================
// D A T A     S T R U C T U R E    D E F I N I T I O N S
//=============================================================================

typedef enum EAVBufferStatus_tag
{
    eAVBufferStatus_Available,
    eAVBufferStatus_CheckedOut,
    eAVBufferStatus_Submitted,
    eAVBufferStatus_Transferred,
    eAVBufferStatus_NeedsCleanup,
}EAVBufferStatus;

typedef enum EPipelineStatus_tag
{
    ePipelineStatus_Stopped,
    ePipelineStatus_Running,
    ePipelineStatus_Paused,
}EPipelineStatus;

typedef enum TYPESEARCHRESULT
{
    eTypeSearch_NULL =0,
    eTypeSearch_DONE,
    eTypeSearch_TYPE_MISMATCH,
    eTypeSearch_TYPE_NO_ELEMENT
}TYPESEARCHRESULT;

typedef struct av_buffer
{
    EAVBufferStatus eStatus;
    GstBuffer *pBuffer;
}AVBuffer;

typedef struct av_buffer_pool
{
    pkHANDLE hLock;
    AVBuffer *pPool;
    uint8_t *pbBuffer;
    uint32_t cbSingleBufferSize;
    uint32_t cbBuffer;
    uint32_t cBufferCount;
    uint32_t cBufferCtxCount;
    // pHead and pTail help to simulate buffer pool as a FIFO.
    uint8_t *pbBufferHead;     // Which can only be modified by SubmitDecoderBuffer.
    uint8_t *pbBufferTail;     // Which can only be modified by FindAvailableBuffer.
    // idxHead and idxTail help to simulate buffer pool ctx as a FIFO.
    uint32_t idxHead;          // Which can only be modified by SubmitDecoderBuffer.
    uint32_t idxTail;          // Which can only be modified by FindAvailableBuffer.

    uint32_t idxClnHead;
    uint32_t idxClnTail;
    uint32_t cCtxUsed;
    uint32_t cTransferred;
    uint32_t cSubmitted;
    uint64_t tsMinSubmittedNs; // ns
    uint64_t tsMaxSubmittedNs; // ns

    uint32_t idxFragBufHead;
    GstBuffer *pFragBuffer;
    bool_t isBufferAvailable;
}AVBufferPool;

typedef struct wma_private_ctx{
    GstElement* pCaps;
    GstElement* pDecoder;
    GstElement* pConvert;
    GstElement* pAudioResample;
}WMAPrivate_ctx;

typedef struct mp3_private_ctx{
    GstElement* pCaps;
    GstElement* pDecoder;
    GstElement* pConvert;
    GstElement* pAudioResample;
}MP3Private_ctx;

typedef struct pcm_private_ctx{
    GstElement* pCaps;
    GstElement* pDecoder;
    GstElement* pConvert;
    GstElement* pAudioResample;
}PCMPrivate_ctx;

typedef struct mpv_private_ctx{
    GstElement* pCaps;
    GstElement* pDecoder;
    GstElement* pColorspace;
}MPVPrivate_ctx;

typedef struct wmv_private_ctx{
    GstElement* pCaps;
    GstElement* pDecoder;
    GstElement* pColorspace;
}WMVPrivate_ctx;

typedef struct h264_private_ctx{
    GstElement* pDecoder;
    GstElement* pColorspace;
    GstElement* pCaps;
}H264Private_ctx;

typedef struct ac3_private_ctx{
    GstElement* pDecoder;
    GstElement* pConvert;
    GstElement* pAudioResample;
}AC3Private_ctx;

typedef struct aac_private_ctx{
    GstElement* pCaps;
    GstElement* pDecoder;
    GstElement* pConvert;
    GstElement* pAudioResample;
}AACPrivate_ctx;

typedef struct decoder_ctx{
    AVBufferPool* pBufferPool;
    uint32_t currentBuf;
    uint32_t avgBitRate;
    bool_t fNeedMerge;
    uint32_t lastNumBufferTransferred;
    uint64_t stallBeginTime90kHz;
    bool fIsEndOfStream;

    GstElement* pBin;
    GstElement* pSink;
    GstElement* pSource;
    union
    {
        WMAPrivate_ctx wma_ctx;
        MP3Private_ctx mp3_ctx;
        PCMPrivate_ctx pcm_ctx;
        MPVPrivate_ctx mpv_ctx;
        WMVPrivate_ctx wmv_ctx;
        H264Private_ctx h264_ctx;
        AC3Private_ctx ac3_ctx;
        AACPrivate_ctx aac_ctx;
    }PrivateCtx;
}AVDecoder_ctx;

typedef struct renderer_ctx{
    bool_t fInUse;
    bool_t fAvailable;
    EPipelineStatus eStatus;

    uint32_t selectionFlags;
    AVDecoder_ctx audioDecoder;
    AVDecoder_ctx videoDecoder;
    GstElement* pPipeline;
    GstElement* pVolume;
    bool_t fIsMute;
    uint32_t currentVolume;
    int32_t  iPlayRate;
    uint64_t uiPCRMS;           // in ms
    uint32_t dwPreRunningFlags;
    bool_t fNoHandoff;          // flag indicating whether buffers can be transferred in OnFakesrcHandoff().
                                // It will be set before pipeline state changes from NULL to PAUSED and
                                // reset after seek is done. So we won't lose key frame with a flushing seek.
}AVRenderer_ctx;


//=============================================================================
// M A C R O    D E F I N I T I O N S
//=============================================================================

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)                 (sizeof(x)/sizeof(x[0]))
#endif

#define PRERUNNING_FLAG_FLUSH        (0x1)
#define PRERUNNING_FLAG_NEWPCR       (0x2)
#define PRERUNNING_FLAG_NEWPLAYRATE  (0x4)

#define SINGLE_VIDEO_BUFFER_MAX_SIZE (0x10000) //64KB
#define SINGLE_AUDIO_BUFFER_MAX_SIZE (0x4000)  //16KB (to handle large WM audio blocks)

#define MAX_BUFFER_OF_ONE_FRAME (s_MAX_VIDEO_FRAME_SIZE/SINGLE_VIDEO_BUFFER_MAX_SIZE)

// Note: in the following some headroom is added to the buffer time to allow for some over-filling
#define VIDEO_BUFFER_TIME_HEADROOM_SECS 4
#define AUDIO_BUFFER_TIME_HEADROOM_SECS 2

#define VIDEO_BUFFER_AMOUNT (((s_MAX_VIDEO_BUFFER_TIME_SECS + VIDEO_BUFFER_TIME_HEADROOM_SECS) * s_MAX_VIDEO_BYTERATE + SINGLE_VIDEO_BUFFER_MAX_SIZE - 1) / SINGLE_VIDEO_BUFFER_MAX_SIZE)
#define AUDIO_BUFFER_AMOUNT (((s_MAX_AUDIO_BUFFER_TIME_SECS + AUDIO_BUFFER_TIME_HEADROOM_SECS) * s_MAX_AUDIO_BYTERATE + SINGLE_AUDIO_BUFFER_MAX_SIZE - 1) / SINGLE_AUDIO_BUFFER_MAX_SIZE)

// A buffer context object is needed for each frame. To compute the number of objects needed
// to span the maximum buffer time, a minimum frame time (max frame rate) must be assumed.
#define VIDEO_FRAME_TIME_MIN_MS 16 // minimum time span for a video frame (62.5 fps)
#define AUDIO_FRAME_TIME_MIN_MS 20 // minimum time span for an audio frame (50 fps)

// There can be 2 buffers used per video frame
#define VIDEO_BUFFER_CTX_AMOUNT (2 * ((s_MAX_VIDEO_BUFFER_TIME_SECS + VIDEO_BUFFER_TIME_HEADROOM_SECS) * 1000 / VIDEO_FRAME_TIME_MIN_MS))
#define AUDIO_BUFFER_CTX_AMOUNT ((s_MAX_AUDIO_BUFFER_TIME_SECS + AUDIO_BUFFER_TIME_HEADROOM_SECS) * 1000 / AUDIO_FRAME_TIME_MIN_MS)

#define INVALID_BUFFER_CTX_AMOUNT \
    ((VIDEO_BUFFER_CTX_AMOUNT>AUDIO_BUFFER_CTX_AMOUNT)?\
    (VIDEO_BUFFER_CTX_AMOUNT+1):\
    (AUDIO_BUFFER_CTX_AMOUNT+1))

#define VOLUME_DEFAULT_LEVEL         5 // scale 0-10
#define START_WAIT_CYCLES            50
#define TARGET_BUFFER_DURATION       2000 /* ms */

#define DEFAULT_FRAME_RATE 30 /* frames/scale */
#define DEFAULT_FRAME_RATE_SCALE 1

#ifndef SCALE_PTS_TO_1KHZ
#define SCALEPTSTO1KHZ_ONLY(x)       x
#else
#define SCALEPTSTO1KHZ_ONLY(x)
#endif

#define BYTE_TO_BIT                  (8LL) // byte to bit
#define SECOND_TO_MS                 (1000LL) // second to millisecond
#define MS_TO_NS                     (1000000LL) // millisecond to nanosecond
#define STATE_CHANGE_WAIT_TIMEOUT_NS (200000000LL) // ns
#define NORMAL_PLAY_RATE             1

#define DEFAULT_WMA_BITRATE          256000
#define DEFAULT_MP3_BITRATE          256000
#define DEFAULT_PCM_BITRATE          2000000
#define DEFAULT_WMV_BITRATE          5000000
#define DEFAULT_MPV_BITRATE          5000000
#define DEFAULT_H264_BITRATE         10000000
#define DEFAULT_M4P2_BITRATE         5000000
#define DEFAULT_AC3_BITRATE          640000
#define DEFAULT_AAC_BITRATE          640000
#define DEFAULT_MPEG1_BITRATE        2048000
#define MAX_AUDIO_CHANNELS           2

#define DEFAULT_AAC_SAMPLERATE       48000
#define DEFAULT_AAC_CHANNELS         2

#define GETGSTELEMENTFROMCMDLINE(pszSwitch, pszDefault) GetGstElementFromCmdLine(pszSwitch, pszDefault, __FUNCTION__)

#define AVRP_TS_INT64_MAX_VALUE      (0x7FFFFFFFFFFFFFFFLL)

#define TypeDemux_mpegpsdemux        "flupsdemux"
#define TypeDemux_dvddemux           "dvddemux"
#define TypeDemux_avidemux           "avidemux"
#define TypeDemux_mpegtsdemux        "flutsdemux"
#define TypeDemux_quicktime          "qtdemux"
#define TypeDemux_x_msvideo          "avidemux"
#define TypeDemux_Pcmdemux           "wavparse"
#define TypeVideoDecoder_mpeg2dec    "mpeg2dec"
#define TypeVideoDecoder_ffdec_mpeg4 "ffdec_mpeg4"
#define TypeVideoDecoder_ffdec_h264  "ffdec_h264"
#define TypeAudioDecoder_mad         "mad"
#define TypeAudioDecoder_faad        "faad"
#define TypeAudioDecoder_a52dec      "a52dec"
#define TypeAudioDecoder_wavparse    "wavparse"

#if !defined (WAVE_FORMAT_WMAUDIO_LOSSLESS)
#define WAVE_FORMAT_WMAUDIO_LOSSLESS 0x0163
#endif

#if !defined (WAVE_FORMAT_WMAUDIO3)
#define WAVE_FORMAT_WMAUDIO3         0x0162
#endif

#define CHECKHR_GOTOBAIL(ret) \
    do{ \
        if(pkFAILED(ret)) \
        { \
            PALPRINTMSG(PALPRINT_ERROR, ("%s():line[%d]: *** TRACE *** "  \
                "ret = 0x%x!\n", __FUNCTION__, __LINE__, (ret))); \
            goto bail;  \
        } \
    }while(0) \

#define CHECKHR_TRACEERROR(expn) \
    do{ \
        pkRESULT hrTemp = (expn); \
        if(pkFAILED(hrTemp)) \
        { \
            PALPRINTMSG(PALPRINT_ERROR, ("%s():line[%d]: *** TRACE *** "  \
                "ret = 0x%x!\n", __FUNCTION__, __LINE__, (hrTemp))); \
        } \
    }while(0) \

#define CHECKPTR_GOTOBAIL(ptr) \
    do{ \
        if(NULL == (ptr)) \
        { \
            PALPRINTMSG(PALPRINT_ERROR, ("%s():line[%d]: *** TRACE *** "  \
                "ptr NULL!\n", __FUNCTION__, __LINE__));\
            retVal = pkE_POINTER; \
            goto bail;  \
        } \
    }while(0) \

#define CHECKBOOLEAN_GOTOBAIL(fRet) \
    do{ \
        if(FALSE == (fRet)) \
        { \
            PALPRINTMSG(PALPRINT_ERROR, ("%s():line[%d]: *** TRACE *** "  \
                "fRet = %d!\n", __FUNCTION__, __LINE__, (fRet))); \
            retVal = pkE_FAIL; \
            goto bail;  \
        } \
    }while(0) \

// Only when ptr is a floating gst object, we can unref it,
// otherwise the owner is responsible to release the resource.
#define SAFE_RELEASE_GST_OBJECT(ptr) \
    do{ \
        if(NULL != (ptr)) \
        { \
            if(GST_OBJECT_IS_FLOATING(ptr)) \
            {\
                gst_object_unref(GST_OBJECT(ptr)); \
            }\
            ptr = NULL; \
        } \
    }while(0) \

#define SAFE_G_FREE(ptr) \
    do{ \
        if(NULL != (ptr)) \
        { \
            g_free((ptr)); \
            ptr = NULL; \
        } \
    }while(0) \

#define SAFE_GST_CAPS_UNREF(ptr) \
    do{ \
        if(NULL != (ptr)) \
        { \
            gst_caps_unref((ptr)); \
            ptr = NULL; \
        } \
    }while(0) \

#define SAFE_G_ERROR_FREE(ptr) \
    do{ \
        if(NULL != (ptr)) \
        { \
            g_error_free((ptr)); \
            ptr = NULL; \
        } \
    }while(0) \

#define SAFE_GST_TAG_LIST_FREE(ptr) \
    do{ \
        if(NULL != (ptr)) \
        { \
            gst_tag_list_free((ptr)); \
            ptr = NULL; \
        } \
    }while(0) \

static const uint64_t c_stallTimeLimit90kHz = 90000 * 2; // 2 seconds

//=============================================================================
// G L O B A L    V A R I A B L E S
//=============================================================================

static pkHANDLE s_hAVRendererLock       = NULL;
static AVRenderer_ctx *s_pRendererCtx   = NULL;
static AVBufferPool *s_pVideoBufferPool = NULL, *s_pAudioBufferPool = NULL;
static guint s_uiMajorVersion = 0, s_uiMinorVersion = 0, s_uiMicroVersion = 0, s_uiNanoVersion = 0;


//=============================================================================
// P R I V A T E    F U N C T I O N    D E F I N I T I O N S
//=============================================================================

//
// Function: GetGstElementFromCmdLine
//
// Purpose: Returns the command-line override for a GStreamer element, otherwise
//   returns the default value.
//
// Arguments:
//   pszSwitch [in] - Case-sensitive string, including dash prefix, which describes
//                    the switch we are looking for (eg. "-input"). The full string
//                    must be matched, abbreviations are not considered matches.
//   pszDefault [in] - Pointer to string containing the default GStreamer element we
//                     should use if no command-line override was found.
//
// Returns:
//   Pointer to the name of the GStreamer element to instantiate. This will either be
//   pszDefault, or pointer to one of the argv[] entries.
//
static const char *GetGstElementFromCmdLine(const char *pszSwitch, const char *pszDefault, const char *pszFunction)
{
    const char *pszResult;
    if(pkS_OK != _GetCmdLineArg(pszSwitch, 1, &pszResult))
    {
        pszResult = pszDefault;
    }
    else
    {
        PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: override detected: using %s instead of %s\n",
            __FUNCTION__, __LINE__, pszFunction, pszSwitch, pszResult, pszDefault));
    }
    return pszResult;
}

static void PrintBufferPool(AVBufferPool *pBufferPool)
{
    uint32_t idx;
    if(pBufferPool)
    {
        PALPRINTMSG(PALPRINT_AVRENDERER_BUFFERPOOL,
            ("============== %s Buffer Pool Record ==============\n",
            (pBufferPool == s_pVideoBufferPool)?"VIDEO":"AUDIO"));
        PALPRINTMSG(PALPRINT_AVRENDERER_BUFFERPOOL, ("pBuffer: %x, size: %d.\n",
            pBufferPool->pbBuffer, pBufferPool->cbSingleBufferSize*pBufferPool->cBufferCount));
        PALPRINTMSG(PALPRINT_AVRENDERER_BUFFERPOOL, ("cBufferCount: %d, cBufferCtxCount : %d.\n",
            pBufferPool->cBufferCount,
            pBufferPool->cBufferCtxCount));
        PALPRINTMSG(PALPRINT_AVRENDERER_BUFFERPOOL, ("idxHead : %d, pbBufferHead: %d.\n",
            pBufferPool->idxHead,
            pBufferPool->pbBufferHead - pBufferPool->pbBuffer));
        PALPRINTMSG(PALPRINT_AVRENDERER_BUFFERPOOL, ("idxTail: %d, pbBufferTail: %d.\n",
            pBufferPool->idxTail,
            pBufferPool->pbBufferTail - pBufferPool->pbBuffer));
        PALPRINTMSG(PALPRINT_AVRENDERER_BUFFERPOOL, ("Used : %d.\n", pBufferPool->cCtxUsed));

#if PALPRINT_AVRENDERER_BUFFERPOOL_VERBOSE
        for(idx=0; idx<pBufferPool->cBufferCtxCount; idx++)
        {
            PALPRINTMSG(PALPRINT_AVRENDERER_BUFFERPOOL,
                ("    Buffer[%d]: {%d}\n",
                idx,
                pBufferPool->pPool[idx].eStatus));
        }
        PALPRINTMSG(PALPRINT_AVRENDERER_BUFFERPOOL, ("\n\n"));
#endif
    }
    return;
}

static bool_t FindAvailableBuffer(AVBufferPool *pBufferPool, uint32_t *number)
{
    bool_t fFound = FALSE;
    uint32_t idx, idxTail, idxHead, cbBuffer, cbSingleBufferSize, cBufferCount, cBufferCtxCount;
    uint8_t *pbBuffer, *pbBufferHead, *pbBufferTail;
    EAVBufferStatus eOriginalStatus;

    if(pBufferPool && number)
    {
        Executive_EnterLock(pBufferPool->hLock);

        idxTail            = pBufferPool->idxTail;
        idxHead            = pBufferPool->idxHead;
        cbBuffer           = pBufferPool->cbBuffer;
        cbSingleBufferSize = pBufferPool->cbSingleBufferSize;
        cBufferCount       = pBufferPool->cBufferCount;
        cBufferCtxCount    = pBufferPool->cBufferCtxCount;
        pbBuffer           = pBufferPool->pbBuffer;
        pbBufferHead       = pBufferPool->pbBufferHead;
        pbBufferTail       = pBufferPool->pbBufferTail;

        // Make NeedsCleanup buffers Available. Figure out where is the new tail.
        for(idx=idxTail; idx!=idxHead; idx=(idx+1)%cBufferCtxCount)
        {
            eOriginalStatus = (EAVBufferStatus)Executive_InterlockedCompareExchange(
                (int32_t*)&(pBufferPool->pPool[idx].eStatus),
                eAVBufferStatus_Available, eAVBufferStatus_NeedsCleanup);
            if(!(eOriginalStatus == eAVBufferStatus_NeedsCleanup ||
                eOriginalStatus == eAVBufferStatus_Available))
            {
                break;
            }
        }
        if(idx != idxTail)
        {
            if(idx != idxHead)
            {
                pbBufferTail = GST_BUFFER_DATA(pBufferPool->pPool[idx].pBuffer);
            }
            else
            {
                pbBufferTail = pbBufferHead;
            }
            pBufferPool->pbBufferTail = pbBufferTail;
            Executive_InterlockedExchange((int32_t*)&(pBufferPool->idxTail), idx);
            idxTail = idx;
        }

        // Never allow idxHead overtake the idxTail, even though a buffer will be wasted.
        for(idx=idxHead; (idx+1)%cBufferCtxCount != idxTail; idx=(idx+1)%cBufferCtxCount)
        {
            eOriginalStatus = (EAVBufferStatus)Executive_InterlockedCompareExchange(
                (int32_t*)&(pBufferPool->pPool[idx].eStatus),
                eAVBufferStatus_CheckedOut, eAVBufferStatus_Available);
            if(eOriginalStatus == eAVBufferStatus_Available)
            {
                *number = idx;
                // There are 2 cases here:
                // 1. If head is lesser than tail, it means we should figure out
                //    if there's enough space between them.
                // 2. If head is larger or equal to tail, we need to compare the
                //    head and tail with borders seperately.
                if((pbBufferHead+cbSingleBufferSize < pbBufferTail &&
                    pbBuffer+cbSingleBufferSize < pbBufferTail) ||
                    (pbBufferHead >= pbBufferTail &&
                    (pbBufferHead+cbSingleBufferSize < pbBuffer+cbBuffer ||
                    pbBuffer+cbSingleBufferSize < pbBufferTail)))
                {
                    // Have enough space as avaliable buffer.
                    fFound = TRUE;
                    GST_BUFFER_SIZE(pBufferPool->pPool[idx].pBuffer) = cbSingleBufferSize;
                    GST_BUFFER_DATA(pBufferPool->pPool[idx].pBuffer) =
                        (pbBufferHead+cbSingleBufferSize > pbBuffer+cbBuffer)?pbBuffer:pbBufferHead;
                }
                else
                {
                    // No more space, return checked out buffer.
                    Executive_InterlockedCompareExchange((int32_t*)&(pBufferPool->pPool[idx].eStatus),
                        eAVBufferStatus_Available, eAVBufferStatus_CheckedOut);
                    PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: No more space. \n",
                        __FUNCTION__, __LINE__));
                    PrintBufferPool(pBufferPool);
                }
                break;
            }
        }
        // Assert head has not overtaken tail
        pkASSERT((idx+1)%cBufferCtxCount != idxTail);
        Executive_ExitLock(pBufferPool->hLock);
    }
    return fFound;
}

static void ResetBufferPool(AVBufferPool *p)
{
    AVBufferPool *pBufferPool = p;
    if(p)
    {
        Executive_EnterLock(pBufferPool->hLock);
        pBufferPool->pbBufferHead     = pBufferPool->pbBuffer;
        pBufferPool->pbBufferTail     = pBufferPool->pbBuffer;
        pBufferPool->idxTail          = 0;
        pBufferPool->idxHead          = 0;
        pBufferPool->idxClnHead       = 0;
        pBufferPool->idxClnTail       = 0;
        pBufferPool->cCtxUsed         = 0;
        pBufferPool->cTransferred     = 0;
        pBufferPool->cSubmitted       = 0;
        pBufferPool->tsMinSubmittedNs = AVRP_TS_INT64_MAX_VALUE;
        pBufferPool->tsMaxSubmittedNs = 0;
        pBufferPool->idxFragBufHead   = INVALID_BUFFER_CTX_AMOUNT;
        pBufferPool->isBufferAvailable = TRUE;

        for(uint32_t idx=0; idx<pBufferPool->cBufferCtxCount; idx++)
        {
            pBufferPool->pPool[idx].eStatus = eAVBufferStatus_Available;
        }
        Executive_ExitLock(pBufferPool->hLock);
    }
    return;
}

static pkRESULT CreateBufferPool(
    AVBufferPool **ppBufferPool,
    uint32_t cBufferCount,
    uint32_t cbSingleBufferSize,
    uint32_t cBufferCtxCount)
{
    pkRESULT retVal = pkE_POINTER;
    uint32_t idx;
    AVBufferPool *pBufferPool;

    *ppBufferPool = (AVBufferPool *)Executive_Alloc(sizeof(AVBufferPool), TRUE);
    pBufferPool   = *ppBufferPool;
    if(pBufferPool && cbSingleBufferSize && cBufferCtxCount)
    {
        retVal = Executive_CreateLock(&(pBufferPool->hLock));
        CHECKHR_GOTOBAIL(retVal);

        // Create buffer pool
        pBufferPool->pbBuffer = (uint8_t *)Executive_Alloc(cbSingleBufferSize*cBufferCount, TRUE);
        if(!pBufferPool->pbBuffer)
        {
            retVal = pkE_FAIL;
            PALPRINTMSG(PALPRINT_ERROR, ("%s():line[%d]: failed to create buffer. \n", __FUNCTION__, __LINE__));
            CHECKHR_GOTOBAIL(retVal);
        }
        pBufferPool->pPool = (AVBuffer *)Executive_Alloc(cBufferCtxCount*sizeof(AVBuffer), TRUE);
        if(!pBufferPool->pPool)
        {
            retVal = pkE_FAIL;
            PALPRINTMSG(PALPRINT_ERROR, ("%s():line[%d]: failed to create pool. \n", __FUNCTION__, __LINE__));
            CHECKHR_GOTOBAIL(retVal);
        }
        for(idx=0; idx<cBufferCtxCount; idx++)
        {
            pBufferPool->pPool[idx].eStatus = eAVBufferStatus_Available;
            pBufferPool->pPool[idx].pBuffer = gst_buffer_new();
            GST_BUFFER_MALLOCDATA(pBufferPool->pPool[idx].pBuffer) = 0;
        }
        pBufferPool->cbSingleBufferSize = cbSingleBufferSize;
        pBufferPool->cbBuffer = cbSingleBufferSize*cBufferCount;
        pBufferPool->cBufferCount = cBufferCount;
        pBufferPool->cBufferCtxCount = cBufferCtxCount;
        pBufferPool->pFragBuffer = gst_buffer_new();
        GST_BUFFER_MALLOCDATA(pBufferPool->pFragBuffer) = 0;
        ResetBufferPool(pBufferPool);
        retVal = pkS_OK;
    }
bail:
    return retVal;
}

static pkRESULT DestroyBufferPool(AVBufferPool *pBufferPool)
{
    pkRESULT retVal = pkE_POINTER;
    uint32_t idx;
    if(pBufferPool)
    {
        if(pBufferPool->pPool)
        {
            for(idx=0; idx<pBufferPool->cBufferCtxCount; idx++)
            {
                GST_BUFFER_DATA(pBufferPool->pPool[idx].pBuffer) = NULL;
                gst_buffer_unref(pBufferPool->pPool[idx].pBuffer);
            }
        }
        gst_buffer_unref(pBufferPool->pFragBuffer);
        Executive_Free(pBufferPool->pPool);
        pBufferPool->pPool              = NULL;
        Executive_Free(pBufferPool->pbBuffer);
        pBufferPool->pbBuffer           = NULL;
        Executive_DeleteLock(pBufferPool->hLock);
        pBufferPool->cbSingleBufferSize = 0;
        pBufferPool->cbBuffer           = 0;
        pBufferPool->cBufferCount       = 0;
        pBufferPool->cBufferCtxCount    = 0;
        Executive_Free(pBufferPool);
        pBufferPool                     = NULL;

        retVal = pkS_OK;
    }
    return retVal;
}

static void GstBufferCopy(GstBuffer *pDestBuf, GstBuffer *pSrcBuf)
{
    GST_BUFFER_MALLOCDATA(pDestBuf) = GST_BUFFER_MALLOCDATA(pSrcBuf);
    GST_BUFFER_DATA(pDestBuf)       = GST_BUFFER_DATA(pSrcBuf);
    GST_BUFFER_SIZE(pDestBuf)       = GST_BUFFER_SIZE(pSrcBuf);

    gst_buffer_copy_metadata(pDestBuf, pSrcBuf, (GstBufferCopyFlags)(GST_BUFFER_COPY_FLAGS | GST_BUFFER_COPY_TIMESTAMPS));
    return;
}

// Sometimes we should save the buffer and handoff function will send it with other buffer(s) together.
// This function can only be called once in a handoff callback function.
static uint32_t PreHandoff(AVBufferPool *pBufferPool, GstBuffer *pHandoffBuf, GstBuffer *pBufFound, uint32_t idx)
{
    GstBuffer *pFragBuffer = NULL, *pNewBuffer = NULL, *pTempBuffer = NULL;
    uint32_t idxRet = 0;
    static uint32_t fragNumber = 0;

    pkASSERT(fragNumber < MAX_BUFFER_OF_ONE_FRAME);
    pFragBuffer = pBufferPool->pFragBuffer;

    // If this buffer is a part of a big buffer, we need to record it and leave now, pick it later.
    if(GST_BUFFER_SIZE(pBufFound) >= pBufferPool->cbSingleBufferSize)
    {
        fragNumber++;

        // It's a "middle" one.
        if(pBufferPool->idxFragBufHead < INVALID_BUFFER_CTX_AMOUNT)
        {
            pTempBuffer = gst_buffer_new();
            pNewBuffer = gst_buffer_merge(pFragBuffer, pBufFound);

            GST_BUFFER_MALLOCDATA(pTempBuffer) = GST_BUFFER_MALLOCDATA(pFragBuffer);
            gst_buffer_unref(pTempBuffer);
            pTempBuffer = NULL;
            GstBufferCopy(pFragBuffer, pNewBuffer);

            GST_BUFFER_MALLOCDATA(pNewBuffer) = 0;
            gst_buffer_unref(pNewBuffer);
            pNewBuffer = NULL;
        }
        else // It's the "head".
        {
            pTempBuffer = gst_buffer_new();
            GST_BUFFER_MALLOCDATA(pTempBuffer) = GST_BUFFER_MALLOCDATA(pFragBuffer);
            GST_BUFFER_MALLOCDATA(pFragBuffer) = 0;
            GstBufferCopy(pFragBuffer, pBufFound);
            gst_buffer_unref(pTempBuffer);
            pTempBuffer = NULL;

            pBufferPool->idxFragBufHead = idx;
        }

        PALPRINTMSG(PALPRINT_AVRENDERER_BUFFERPOOL, ("\n    This buffer is not an independent one. \n"));
        PALPRINTMSG(PALPRINT_AVRENDERER_BUFFERPOOL,
            ("    Pre handoff %s buffer %d!!!! size:%d, address:0x%08x, time:%lld\n",
            (pBufferPool == s_pVideoBufferPool)?"VIDEO":"AUDIO",
            idx,
            GST_BUFFER_SIZE(pFragBuffer),
            GST_BUFFER_DATA(pFragBuffer),
            GST_BUFFER_TIMESTAMP(pFragBuffer)/MS_TO_NS)
            );

        // Leave now, we will come back next hand off.
        // TODO: need to clean up the following casting of pointers to int32_t; also, 2nd param was (int32_t*) which seems incorrect
        Executive_InterlockedExchange((int32_t*)&(pBufferPool->pFragBuffer), (int32_t)(pFragBuffer));
        return pBufferPool->idxFragBufHead;
    }

    // Here the GST_BUFFER_SIZE(pBufFound) will be smaller than SINGLE_VIDEO_BUFFER_MAX_SIZE.
    // It must be an "end" fragment, or a regular buffer.
    if(pBufferPool->idxFragBufHead < INVALID_BUFFER_CTX_AMOUNT) // It's the "end".
    {
        pTempBuffer = gst_buffer_new();
        pNewBuffer = gst_buffer_merge(pFragBuffer, pBufFound);

        GST_BUFFER_MALLOCDATA(pTempBuffer) = GST_BUFFER_MALLOCDATA(pFragBuffer);
        gst_buffer_unref(pTempBuffer);
        pTempBuffer = NULL;
        GstBufferCopy(pFragBuffer, pNewBuffer);

        GST_BUFFER_MALLOCDATA(pNewBuffer) = 0;
        GstBufferCopy(pHandoffBuf, pNewBuffer);
        gst_buffer_unref(pNewBuffer);
        pNewBuffer = NULL;

        idxRet = pBufferPool->idxFragBufHead;
        pBufferPool->idxFragBufHead = INVALID_BUFFER_CTX_AMOUNT;
        fragNumber = 0;
    }
    else // regular buffer
    {
        GstBufferCopy(pHandoffBuf, pBufFound);
        idxRet = idx;
    }

    PALPRINTMSG(PALPRINT_AVRENDERER_BUFFERPOOL,
    ("    Prehandoff %s buffer %d size:%d, addr:0x%08x, ts:%lld, tfr:%d, sbmt:%d, tsMax:%d\n",
        (pBufferPool == s_pVideoBufferPool)?"V":"A",
        idx,
        GST_BUFFER_SIZE(pHandoffBuf),
        GST_BUFFER_DATA(pHandoffBuf)-pBufferPool->pbBuffer,
        GST_BUFFER_TIMESTAMP(pHandoffBuf)/MS_TO_NS,
        pBufferPool->cTransferred,
        pBufferPool->cSubmitted,
        pBufferPool->tsMaxSubmittedNs/MS_TO_NS));

    return idxRet;
}

static void OnFakesrcHandoff(
    GstElement *pFakesrc,
    GstBuffer *pBuffer,
    GstPad *pPad,
    gpointer pUserData)
{
    long int nsWaitTime = 10000000; // 10 ms
    struct timespec sTimeReq;

    uint32_t idx;
    uint32_t idxTail;
    uint32_t idxHead;
    uint32_t cBufferCount;
    uint32_t cBufferCtxCount;
    uint32_t idxClnHead;
    uint32_t idxClnTail;
    uint32_t cTransferred;
    uint8_t *pbBuffer;
    
    EAVBufferStatus eOriginalStatus;
    bool_t fFound = FALSE;
    
    AVDecoder_ctx *pDecoder = (AVDecoder_ctx*)pUserData;
    AVBufferPool *pBufferPool = pDecoder->pBufferPool;

    if(s_pRendererCtx->fNoHandoff)
    {
        PALPRINTMSG(PALPRINT_AVRENDERER_BUFFERPOOL, ("OnFakesrcHandoff %s suppressed\n",
            (pBufferPool == s_pVideoBufferPool)?"V":"A"));
        {
            sTimeReq.tv_sec = 0;
            sTimeReq.tv_nsec = nsWaitTime;
            nanosleep(&sTimeReq, NULL);
        }
        return;
    }

    Executive_EnterLock(pBufferPool->hLock);
    
    idxTail         = pBufferPool->idxTail;
    idxHead         = pBufferPool->idxHead;
    cBufferCtxCount = pBufferPool->cBufferCtxCount;
    idxClnHead      = pBufferPool->idxClnHead;
    idxClnTail      = pBufferPool->idxClnTail;
    cTransferred    = pBufferPool->cTransferred;
    cBufferCount    = pBufferPool->cBufferCount;
    pbBuffer        = pBufferPool->pbBuffer;

    // Looking for submitted buffer and transfer it to pipeline.
    {
        for(idx=idxTail; idx!=idxHead; idx=(idx+1)%cBufferCtxCount)
        {
            eOriginalStatus = (EAVBufferStatus)Executive_InterlockedCompareExchange(
                (int32_t*)&(pBufferPool->pPool[idx].eStatus),
                eAVBufferStatus_Transferred,
                eAVBufferStatus_Submitted);
            if(eOriginalStatus == eAVBufferStatus_Submitted)
            {
                Executive_InterlockedCompareExchange((int32_t*)&(pBufferPool->cTransferred),
                    (cTransferred+1), cTransferred);
                fFound = TRUE;
                if(pDecoder->fNeedMerge)
                {
                    idxClnTail = PreHandoff(pBufferPool, pBuffer, pBufferPool->pPool[idx].pBuffer, idx);
                }
                else
                {
                    GST_BUFFER_MALLOCDATA(pBufferPool->pPool[idx].pBuffer) = 0;
                    GstBufferCopy(pBuffer, pBufferPool->pPool[idx].pBuffer);
                    idxClnTail = idx;

                    PALPRINTMSG(PALPRINT_AVRENDERER_BUFFERPOOL,
                        ("    Handoff %s buffer %d queued: %d size:%d, crc:0x%08x, ts:%lld, tfr:%d, sbmt:%d, tsMax:%d\n",
                        (pBufferPool == s_pVideoBufferPool)?"V":"A",
                        idx,
                        (int)(pBufferPool->cSubmitted - pBufferPool->cTransferred),
                        GST_BUFFER_SIZE(pBuffer),
                        s_ComputeCRC32(CRC32_INITIALIZE, (uint8_t*)(GST_BUFFER_DATA(pBuffer)), GST_BUFFER_SIZE(pBuffer)),
                        GST_BUFFER_TIMESTAMP(pBuffer)/MS_TO_NS,
                        pBufferPool->cTransferred,
                        pBufferPool->cSubmitted,
                        pBufferPool->tsMaxSubmittedNs/MS_TO_NS));

                    pBufferPool->isBufferAvailable = TRUE;
                }
                goto found;
            }
        }

        // isBufferAvailable is used to limit the log output to just log the transition into starvation to reduce log spam
        if (pBufferPool->isBufferAvailable)
        {
            pBufferPool->isBufferAvailable = FALSE;

            PALPRINTMSG(PALPRINT_AVRENDERER,
                ("%s():line[%d]: No %s buffer available Head:%u, Tail:%u Span:%u\n",
                __FUNCTION__, __LINE__,
                (pBufferPool == s_pVideoBufferPool)?"VIDEO":"AUDIO",
                idxHead,
                idxTail,
                (idxHead + cBufferCtxCount - idxTail) % cBufferCtxCount));
        }

        // Exit the buffer pool lock before doing the sleep to allow new samples to be submitted
        Executive_ExitLock(pBufferPool->hLock);

        // Sleep a short time because gstreamer will retry immediately
        sTimeReq.tv_sec = 0;
        sTimeReq.tv_nsec = nsWaitTime;
        nanosleep(&sTimeReq, NULL);

        return;
    }
found:

    // Clean up buffer which is transferred already.
    for(idx=idxClnHead; idx!=idxClnTail; idx=(idx+1)%cBufferCtxCount)
    {
        eOriginalStatus = (EAVBufferStatus)Executive_InterlockedCompareExchange(
            (int32_t*)&(pBufferPool->pPool[idx].eStatus),
            eAVBufferStatus_NeedsCleanup,
            eAVBufferStatus_Transferred);
        if(eOriginalStatus == eAVBufferStatus_Transferred)
        {
            Executive_InterlockedDecrement((int32_t*)&(pBufferPool->cCtxUsed));
        }
    }
    pBufferPool->idxClnHead = idx;

    Executive_ExitLock(pBufferPool->hLock);
    return;
}

static gboolean GstMsgCenter(GstBus *pBus, GstMessage *msg, gpointer data)
{
    AVRenderer_ctx *pRenderer = (AVRenderer_ctx *)data;

    if((GST_MESSAGE_TYPE(msg) != GST_MESSAGE_STATE_CHANGED) &&
        (GST_MESSAGE_TYPE(msg) != GST_MESSAGE_ELEMENT))
    {
        PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]:received message type: %s. \n",
            __FUNCTION__, __LINE__, GST_MESSAGE_TYPE_NAME(msg)));
    }

    switch(GST_MESSAGE_TYPE(msg))
    {
        case GST_MESSAGE_TAG:
        {
            GstTagList *tags_received = NULL;
            gchar *szCodecVideo = NULL;
            gchar *szCodecAudio = NULL;

            gst_message_parse_tag (msg, &tags_received);

            PALPRINTMSG(PALPRINT_AVRENDERER_VERBOSE, ("%s():line[%d]: Received tags: %" GST_PTR_FORMAT"\n",
                __FUNCTION__, __LINE__, tags_received));

            if(gst_tag_list_get_string(tags_received, GST_TAG_VIDEO_CODEC, &szCodecVideo))
            {
                PALPRINTMSG(PALPRINT_AVRENDERER_VERBOSE, ("%s():line[%d]: %s: %s\n",
                    __FUNCTION__, __LINE__, GST_TAG_VIDEO_CODEC, szCodecVideo));
            }

            if(gst_tag_list_get_string (tags_received, GST_TAG_AUDIO_CODEC, &szCodecAudio))
            {
                PALPRINTMSG(PALPRINT_AVRENDERER_VERBOSE, ("%s():line[%d]: %s: %s\n",
                    __FUNCTION__, __LINE__, GST_TAG_AUDIO_CODEC, szCodecAudio));
            }

            SAFE_G_FREE (szCodecAudio);
            SAFE_G_FREE (szCodecVideo);
            SAFE_GST_TAG_LIST_FREE (tags_received);

            break;
        }
        case GST_MESSAGE_ASYNC_DONE:

            break;

        case GST_MESSAGE_EOS:

            PALPRINTMSG(PALPRINT_AVRENDERER_VERBOSE,
                ("%s():line[%d]: pkMEDIA_EVENT_TYPE_END_OF_MEDIA \n", __FUNCTION__, __LINE__));

            break;

         case GST_MESSAGE_ERROR:
         {
            gchar *debug = NULL;
            GError *err = NULL;

            gst_message_parse_error(msg, &err, &debug);
            SAFE_G_FREE(debug);
            PALPRINTMSG(PALPRINT_ERROR, ("%s():line[%d]: error message: %s. \n",
                __FUNCTION__, __LINE__, err->message));

            SAFE_G_ERROR_FREE(err);
            break;
        }

        case GST_MESSAGE_BUFFERING:
        {
            gint percent;
            gst_message_parse_buffering (msg, &percent);
            PALPRINTMSG(PALPRINT_AVRENDERER_VERBOSE, ("%s():line[%d]: buffering... %d  \n",
                __FUNCTION__, __LINE__, percent));

            break;
        }

        default:
            break;
    }
bail:
    return TRUE;
}

static pkRESULT SetSinkWindow(GstElement *pVideosink)
{
    // Just let gstreamer video sink create it's own window
    return pkS_OK;
}

static gboolean FakesrcDoSeek(GstBaseSrc *pSrc, GstSegment *pSegment)
{
    PALPRINTMSG(PALPRINT_AVRENDERER, ("%s is called with src[%p] seg[%p]\n", 
        __FUNCTION__, pSrc, pSegment));
    return TRUE;
}

static pkRESULT EnableTimeBasedSeek(GstElement *pFakeSrc)
{
    pkRESULT retVal = pkS_OK;
    CHECKPTR_GOTOBAIL(pFakeSrc);

    gst_base_src_set_format(GST_BASE_SRC(pFakeSrc), GST_FORMAT_TIME);
    GST_BASE_SRC_GET_CLASS(pFakeSrc)->do_seek = GST_DEBUG_FUNCPTR(FakesrcDoSeek);
bail:
    return retVal;
}

static void InitializeFakesrc(GstElement *pFakesrc, gpointer pData)
{
    PALPRINTMSG(PALPRINT_AVRENDERER, ("%s is called with src[%p] handoffCtx[%p]\n", 
        __FUNCTION__, pFakesrc, pData));
    // Setup fake source - NOTE that all g_object_set calls return void
    // Set separately because we don't know how g_object_set responds to failure
    g_object_set(G_OBJECT(pFakesrc), "signal-handoffs", TRUE, NULL);
    g_object_set(G_OBJECT(pFakesrc), "sizetype", 1, NULL);
    if(s_uiMinorVersion > 10 || (s_uiMinorVersion == 10 && s_uiMicroVersion >= 20))
    {
        g_object_set(G_OBJECT(pFakesrc), "format", GST_FORMAT_TIME, NULL);
    }
    g_signal_connect(pFakesrc, "handoff", G_CALLBACK(OnFakesrcHandoff), pData);
    return;
}

static pkRESULT CreateWMABin(pkHANDLE hCtx, pkHANDLE *phDecoder,
    pkAV_WAVEFORMATEX *pWave)
{
    pkRESULT retVal            = pkS_OK;
    AVRenderer_ctx *pRenderer  = (AVRenderer_ctx*)hCtx;
    WMAPrivate_ctx *wmadecoder = &(pRenderer->audioDecoder.PrivateCtx.wma_ctx);
    int wmaversion             = 2;
    GstBuffer *data;
    gboolean ret;

    memset(&(pRenderer->audioDecoder), 0, sizeof(AVDecoder_ctx));
    pRenderer->audioDecoder.pBufferPool = s_pAudioBufferPool;
    ResetBufferPool(pRenderer->audioDecoder.pBufferPool);
    pRenderer->selectionFlags |= pkSTREAM_SELECTION_FLAG_AUDIO;
    pRenderer->audioDecoder.pBin = gst_bin_new("wmabin");
    CHECKPTR_GOTOBAIL(pRenderer->audioDecoder.pBin);
    wmadecoder->pCaps = gst_element_factory_make("capsfilter", "wma-caps");
    CHECKPTR_GOTOBAIL(wmadecoder->pCaps);

    wmadecoder->pDecoder = gst_element_factory_make(
        GETGSTELEMENTFROMCMDLINE(CMDOPT_WMA, "fluwmadec"),
        "wma-decoder");
    CHECKPTR_GOTOBAIL(wmadecoder->pDecoder);

    wmadecoder->pConvert = gst_element_factory_make("audioconvert", "convert");
    CHECKPTR_GOTOBAIL(wmadecoder->pConvert);
    wmadecoder->pAudioResample = gst_element_factory_make("audioresample", "aresample");
    CHECKPTR_GOTOBAIL(wmadecoder->pAudioResample);
    pRenderer->pVolume = gst_element_factory_make("volume", "volume");
    CHECKPTR_GOTOBAIL(pRenderer->pVolume);

    pRenderer->audioDecoder.pSink = gst_element_factory_make(
        GETGSTELEMENTFROMCMDLINE(CMDOPT_AUDIOSINK, "alsasink"),
        "audiosink");
    CHECKPTR_GOTOBAIL(pRenderer->audioDecoder.pSink);

    pRenderer->audioDecoder.pSource = gst_element_factory_make("fakesrc","wma-source");
    CHECKPTR_GOTOBAIL(pRenderer->audioDecoder.pSource);

    InitializeFakesrc(pRenderer->audioDecoder.pSource, (gpointer)&(pRenderer->audioDecoder));
    EnableTimeBasedSeek(pRenderer->audioDecoder.pSource);

    // Setup decoder caps
    
    if(WAVE_FORMAT_WMAUDIO2 == pWave->wFormatTag)
    {
        // for the ordinary WMA8/9 codec
        wmaversion = 2;
    }
    else if(WAVE_FORMAT_WMAUDIO3 == pWave->wFormatTag)
    {
        // for the WMA9 Professional
        wmaversion = 3;
    }
    else
    {
        retVal = pkE_UNEXPECTED;
        CHECKHR_GOTOBAIL(retVal);
    }

    data = gst_buffer_new_and_alloc(pWave->cbSize);
    if(GST_BUFFER_DATA(data))
    {
        memcpy(GST_BUFFER_DATA(data), pWave->reserved, pWave->cbSize);
    }

    g_object_set(G_OBJECT(wmadecoder->pCaps), "caps",
        gst_caps_new_simple("audio/x-wma",
        "rate", G_TYPE_INT, pWave->nSamplesPerSec,
        "channels", G_TYPE_INT, pWave->nChannels,
        "wmaversion", G_TYPE_INT, wmaversion,
        "width", G_TYPE_INT, pWave->wBitsPerSample,
        "depth", G_TYPE_INT, pWave->wBitsPerSample,
        "endianness", G_TYPE_INT, 1234,
        "codec_data", GST_TYPE_BUFFER, data,
        "block_align", G_TYPE_INT, pWave->nBlockAlign,
        "bitrate", G_TYPE_INT, pWave->nAvgBytesPerSec*8,
        NULL), NULL);
    gst_buffer_unref(data);

    // Setup bin, gst_bin_add_many returns void.
    gst_bin_add_many(GST_BIN(pRenderer->audioDecoder.pBin),
        pRenderer->audioDecoder.pSource,
        wmadecoder->pCaps,
        wmadecoder->pDecoder,
        wmadecoder->pConvert,
        wmadecoder->pAudioResample,
        pRenderer->pVolume,
        pRenderer->audioDecoder.pSink,
        NULL);
    ret = gst_element_link_many(pRenderer->audioDecoder.pSource,
        wmadecoder->pCaps,
        wmadecoder->pDecoder,
        wmadecoder->pConvert,
        wmadecoder->pAudioResample,
        pRenderer->pVolume,
        pRenderer->audioDecoder.pSink, NULL);
    CHECKBOOLEAN_GOTOBAIL(ret);

    *phDecoder = &(pRenderer->audioDecoder);

bail:
    if(pkFAILED(retVal))
    {
        SAFE_RELEASE_GST_OBJECT(pRenderer->audioDecoder.pSource);
        SAFE_RELEASE_GST_OBJECT(pRenderer->audioDecoder.pSink);
        SAFE_RELEASE_GST_OBJECT(pRenderer->pVolume);
        SAFE_RELEASE_GST_OBJECT(wmadecoder->pAudioResample);
        SAFE_RELEASE_GST_OBJECT(wmadecoder->pConvert);
        SAFE_RELEASE_GST_OBJECT(wmadecoder->pDecoder);
        SAFE_RELEASE_GST_OBJECT(wmadecoder->pCaps);
        SAFE_RELEASE_GST_OBJECT(pRenderer->audioDecoder.pBin);
    }
    return retVal;
}

static pkRESULT CreateMP3Bin(pkHANDLE hCtx, pkHANDLE *phDecoder,
    pkAV_WAVEFORMATEX *pWave)
{
    pkRESULT retVal = pkS_OK;
    AVRenderer_ctx *pRenderer  = (AVRenderer_ctx*)hCtx;
    MP3Private_ctx *mp3decoder = &(pRenderer->audioDecoder.PrivateCtx.mp3_ctx);
    gboolean ret;

    memset(&(pRenderer->audioDecoder), 0, sizeof(AVDecoder_ctx));
    pRenderer->audioDecoder.pBufferPool = s_pAudioBufferPool;
    ResetBufferPool(pRenderer->audioDecoder.pBufferPool);
    pRenderer->selectionFlags |= pkSTREAM_SELECTION_FLAG_AUDIO;
    pRenderer->audioDecoder.pBin = gst_bin_new("mp3bin");
    CHECKPTR_GOTOBAIL(pRenderer->audioDecoder.pBin);

    mp3decoder->pDecoder = gst_element_factory_make(
        GETGSTELEMENTFROMCMDLINE(CMDOPT_MP3, "mad"),
        "mp3-decoder");
    CHECKPTR_GOTOBAIL(mp3decoder->pDecoder);

    mp3decoder->pCaps = gst_element_factory_make("capsfilter", "mp3-caps");
    CHECKPTR_GOTOBAIL(mp3decoder->pCaps);
    mp3decoder->pConvert = gst_element_factory_make("audioconvert", "convert");
    CHECKPTR_GOTOBAIL(mp3decoder->pConvert);
    mp3decoder->pAudioResample = gst_element_factory_make("audioresample", "aresample");
    CHECKPTR_GOTOBAIL(mp3decoder->pAudioResample);
    pRenderer->pVolume = gst_element_factory_make("volume", "volume");
    CHECKPTR_GOTOBAIL(pRenderer->pVolume);

    pRenderer->audioDecoder.pSink = gst_element_factory_make(
        GETGSTELEMENTFROMCMDLINE(CMDOPT_AUDIOSINK, "alsasink"),
        "audiosink");
    CHECKPTR_GOTOBAIL(pRenderer->audioDecoder.pSink);

    pRenderer->audioDecoder.pSource = gst_element_factory_make("fakesrc","mp3-source");
    CHECKPTR_GOTOBAIL(pRenderer->audioDecoder.pSource);

    g_object_set(G_OBJECT(mp3decoder->pCaps), "caps",
        gst_caps_new_simple("audio/mpeg",
        "rate", G_TYPE_INT, pWave->nSamplesPerSec,
        "mpegversion", G_TYPE_INT, 1,
        "layer", G_TYPE_INT, 3,
        "channels", G_TYPE_INT, pWave->nChannels,
        NULL), NULL);

    InitializeFakesrc(pRenderer->audioDecoder.pSource, (gpointer)&(pRenderer->audioDecoder));
    EnableTimeBasedSeek(pRenderer->audioDecoder.pSource);

    // Setup bin, gst_bin_add_many returns void.
    gst_bin_add_many(GST_BIN(pRenderer->audioDecoder.pBin),
        pRenderer->audioDecoder.pSource,
        mp3decoder->pCaps,
        mp3decoder->pDecoder,
        mp3decoder->pConvert,
        mp3decoder->pAudioResample,
        pRenderer->pVolume,
        pRenderer->audioDecoder.pSink,
        NULL);
    ret = gst_element_link_many(pRenderer->audioDecoder.pSource,
        mp3decoder->pCaps,
        mp3decoder->pDecoder,
        mp3decoder->pConvert,
        mp3decoder->pAudioResample,
        pRenderer->pVolume,
        pRenderer->audioDecoder.pSink, NULL);
    CHECKBOOLEAN_GOTOBAIL(ret);

    *phDecoder = &(pRenderer->audioDecoder);

bail:
    if(pkFAILED(retVal))
    {
        SAFE_RELEASE_GST_OBJECT(pRenderer->audioDecoder.pSource);
        SAFE_RELEASE_GST_OBJECT(pRenderer->audioDecoder.pSink);
        SAFE_RELEASE_GST_OBJECT(pRenderer->pVolume);
        SAFE_RELEASE_GST_OBJECT(mp3decoder->pAudioResample);
        SAFE_RELEASE_GST_OBJECT(mp3decoder->pConvert);
        SAFE_RELEASE_GST_OBJECT(mp3decoder->pCaps);
        SAFE_RELEASE_GST_OBJECT(mp3decoder->pDecoder);
        SAFE_RELEASE_GST_OBJECT(pRenderer->audioDecoder.pBin);
    }
    return retVal;
}

static pkRESULT CreatePCMBin(pkHANDLE hCtx, pkHANDLE *phDecoder, pkAV_WAVEFORMATEX *pWFX)
{
    pkRESULT retVal              = pkS_OK;
    GstCaps *pCaps               = NULL;
    GstStructure *pStructure     = NULL;
    AVRenderer_ctx *pRenderer    = (AVRenderer_ctx*)hCtx;
    PCMPrivate_ctx *pPcmdecoder  = &(pRenderer->audioDecoder.PrivateCtx.pcm_ctx);
    GstAudioChannelPosition *pos = NULL;
    gboolean ret;

    // currently we don't support PCM with more than 2 channels
    if(pWFX->nChannels > MAX_AUDIO_CHANNELS)
    {
        PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: We don't support PCM with more than 2 channels. \n",
            __FUNCTION__, __LINE__));
        retVal = pkE_RENDERER_STREAM_TYPE_NOT_SUPPORTED;
        goto bail;
    }

    memset(&(pRenderer->audioDecoder), 0, sizeof(pRenderer->audioDecoder));
    pRenderer->audioDecoder.pBufferPool = s_pAudioBufferPool;
    ResetBufferPool(pRenderer->audioDecoder.pBufferPool);
    pRenderer->selectionFlags |= pkSTREAM_SELECTION_FLAG_AUDIO;
    pRenderer->audioDecoder.pBin = gst_bin_new("pcmbin");
    CHECKPTR_GOTOBAIL(pRenderer->audioDecoder.pBin);
    pPcmdecoder->pCaps = gst_element_factory_make("capsfilter", "pcm-caps");
    CHECKPTR_GOTOBAIL(pPcmdecoder->pCaps);

    pRenderer->audioDecoder.pSink = gst_element_factory_make(
        GETGSTELEMENTFROMCMDLINE(CMDOPT_AUDIOSINK, "alsasink"),
        "audiosink");
    CHECKPTR_GOTOBAIL(pRenderer->audioDecoder.pSink);

    pRenderer->audioDecoder.pSource = gst_element_factory_make("fakesrc","pcm-source");
    CHECKPTR_GOTOBAIL(pRenderer->audioDecoder.pSource);
    pRenderer->pVolume = gst_element_factory_make("volume", "volume");
    CHECKPTR_GOTOBAIL(pRenderer->pVolume);

    pCaps = gst_caps_new_simple("audio/x-raw-int",
        "rate", G_TYPE_INT, pWFX->nSamplesPerSec,
        "channels", G_TYPE_INT, pWFX->nChannels,
        "width", G_TYPE_INT, pWFX->wBitsPerSample,
        "depth", G_TYPE_INT, pWFX->wBitsPerSample,
        "signed", G_TYPE_BOOLEAN, 1, // NOTE: there is no such information conveyed by parameter
        "endianness", G_TYPE_INT, G_BYTE_ORDER, // NOTE: there is no such information conveyed by parameter
        NULL);

    g_object_set(G_OBJECT(pPcmdecoder->pCaps), "caps", pCaps, NULL);

    if(2 == pWFX->nChannels)
    {
        pos = g_new(GstAudioChannelPosition, 2);
        pos[0] = GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT;
        pos[1] = GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT;

        pStructure = gst_caps_get_structure(pCaps, 0);
        gst_audio_set_channel_positions(pStructure, pos);
    }

    InitializeFakesrc(pRenderer->audioDecoder.pSource, (gpointer)&(pRenderer->audioDecoder));
    EnableTimeBasedSeek(pRenderer->audioDecoder.pSource);

    // Setup bin, gst_bin_add_many returns void.
    gst_bin_add_many(GST_BIN(pRenderer->audioDecoder.pBin),
        pRenderer->audioDecoder.pSource,
        pPcmdecoder->pCaps,
        pRenderer->pVolume,
        pRenderer->audioDecoder.pSink,
        NULL);
    ret = gst_element_link_many(pRenderer->audioDecoder.pSource,
        pPcmdecoder->pCaps,
        pRenderer->pVolume,
        pRenderer->audioDecoder.pSink, NULL);
    CHECKBOOLEAN_GOTOBAIL(ret);

    *phDecoder = &(pRenderer->audioDecoder);

bail:
    if(pkFAILED(retVal))
    {
        SAFE_RELEASE_GST_OBJECT(pRenderer->pVolume);
        SAFE_RELEASE_GST_OBJECT(pRenderer->audioDecoder.pSource);
        SAFE_RELEASE_GST_OBJECT(pRenderer->audioDecoder.pSink);
        SAFE_RELEASE_GST_OBJECT(pPcmdecoder->pCaps);
        SAFE_RELEASE_GST_OBJECT(pRenderer->audioDecoder.pBin);
    }
    return retVal;
}

static pkRESULT CreateWMVBin(pkHANDLE hCtx, pkHANDLE *phDecoder, pkAV_BITMAPINFOHEADER *pBitmap)
{
    pkRESULT retVal = pkS_OK;
    AVRenderer_ctx *pRenderer  = (AVRenderer_ctx*)hCtx;
    WMVPrivate_ctx *wmvdecoder = &(pRenderer->videoDecoder.PrivateCtx.wmv_ctx);
    GstBuffer *data;
    gboolean ret;

    memset(&(pRenderer->videoDecoder), 0, sizeof(AVDecoder_ctx));
    pRenderer->videoDecoder.pBufferPool = s_pVideoBufferPool;
    ResetBufferPool(pRenderer->videoDecoder.pBufferPool);
    pRenderer->selectionFlags |= pkSTREAM_SELECTION_FLAG_VIDEO;

    pRenderer->videoDecoder.pBin = gst_bin_new("wmvbin");
    CHECKPTR_GOTOBAIL(pRenderer->videoDecoder.pBin);

    wmvdecoder->pCaps = gst_element_factory_make("capsfilter", "wmv-caps");
    CHECKPTR_GOTOBAIL(wmvdecoder->pCaps);

    if (GST_MAKE_FOURCC('W', 'V', 'C', '1') == pBitmap->biCompression)
    {
        wmvdecoder->pDecoder = gst_element_factory_make(
            GETGSTELEMENTFROMCMDLINE(CMDOPT_WMV, "fluwmvdec"), // ffdec_vc1; fluwmvdec
            "wmv-decoder");
    }
    else // not VC1
    {
        wmvdecoder->pDecoder = gst_element_factory_make(
            GETGSTELEMENTFROMCMDLINE(CMDOPT_WMV, "fluwmvdec"), // ffdec__wmv1,2,3 for 7,8,9; fluwmvdec
            "wmv-decoder");
    }

    CHECKPTR_GOTOBAIL(wmvdecoder->pDecoder);

    wmvdecoder->pColorspace = gst_element_factory_make("ffmpegcolorspace", "wmvcolorspace");
    CHECKPTR_GOTOBAIL(wmvdecoder->pColorspace);
    pRenderer->videoDecoder.pSink = gst_element_factory_make("autovideosink", "videosink");
    CHECKPTR_GOTOBAIL(pRenderer->videoDecoder.pSink);
    pRenderer->videoDecoder.pSource = gst_element_factory_make("fakesrc","wmv-source");
    CHECKPTR_GOTOBAIL(pRenderer->videoDecoder.pSource);

    InitializeFakesrc(pRenderer->videoDecoder.pSource, (gpointer)&(pRenderer->videoDecoder));
    EnableTimeBasedSeek(pRenderer->videoDecoder.pSource);

    // Setup decoder caps
    if (pBitmap->biSize <= sizeof(*pBitmap))
    {
        PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: No extra data. \n", __FUNCTION__, __LINE__));
        g_object_set(G_OBJECT(wmvdecoder->pCaps), "caps",
            gst_caps_new_simple("video/x-wmv",
            "width", G_TYPE_INT, pBitmap->biWidth,
            "height", G_TYPE_INT, pBitmap->biHeight,
            "framerate", GST_TYPE_FRACTION, DEFAULT_FRAME_RATE, 1,
            "wmvversion", G_TYPE_INT, 3, //for ffdec_wmv3, wmv9
            "fourcc", GST_TYPE_FOURCC, pBitmap->biCompression,
            NULL), NULL);
    }
    else
    {
        uint32_t size = pBitmap->biSize - sizeof(*pBitmap);
        PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: %u extra data. \n", __FUNCTION__, __LINE__, size));
        data = gst_buffer_new_and_alloc(size);
        if(GST_BUFFER_DATA(data))
        {
            memcpy(GST_BUFFER_DATA(data), (char*)pBitmap+sizeof(*pBitmap), size);
        }

        if (GST_MAKE_FOURCC('W', 'V', 'C', '1') == pBitmap->biCompression)
        {
            g_object_set(G_OBJECT(wmvdecoder->pCaps), "caps",
                gst_caps_new_simple("video/x-wmv",
                "width", G_TYPE_INT, pBitmap->biWidth,
                "height", G_TYPE_INT, pBitmap->biHeight,
                "framerate", GST_TYPE_FRACTION, DEFAULT_FRAME_RATE, 1,
                "wmvversion", G_TYPE_INT, 3, //for ffdec_wmv3, wmv9
                "codec_data", GST_TYPE_BUFFER, data,
                "fourcc", GST_TYPE_FOURCC, pBitmap->biCompression,
                NULL), NULL);
        }
        else
        {
            g_object_set(G_OBJECT(wmvdecoder->pCaps), "caps",
                gst_caps_new_simple("video/x-wmv",
                "width", G_TYPE_INT, pBitmap->biWidth,
                "height", G_TYPE_INT, pBitmap->biHeight,
                "framerate", GST_TYPE_FRACTION, DEFAULT_FRAME_RATE, 1,
                "wmvversion", G_TYPE_INT, 3, //for ffdec_wmv3, wmv9
                "codec_data", GST_TYPE_BUFFER, data,
                NULL), NULL);
        }

        gst_buffer_unref(data);
    }


    // Setup xoverlay window
    retVal = SetSinkWindow(pRenderer->videoDecoder.pSink);
    CHECKHR_GOTOBAIL(retVal);

    // Setup bin, gst_bin_add_many returns void.
    gst_bin_add_many(GST_BIN(pRenderer->videoDecoder.pBin),
        pRenderer->videoDecoder.pSource,
        wmvdecoder->pCaps,
        wmvdecoder->pDecoder,
        wmvdecoder->pColorspace,
        pRenderer->videoDecoder.pSink,
        NULL);

    ret = gst_element_link_many(pRenderer->videoDecoder.pSource,
        wmvdecoder->pCaps,
        wmvdecoder->pDecoder,
        wmvdecoder->pColorspace,
        pRenderer->videoDecoder.pSink,
        NULL);

    CHECKBOOLEAN_GOTOBAIL(ret);

    pRenderer->videoDecoder.fNeedMerge = TRUE;
    *phDecoder = &(pRenderer->videoDecoder);

bail:
    if(pkFAILED(retVal))
    {
        SAFE_RELEASE_GST_OBJECT(pRenderer->videoDecoder.pSource);
        SAFE_RELEASE_GST_OBJECT(pRenderer->videoDecoder.pSink);
        SAFE_RELEASE_GST_OBJECT(wmvdecoder->pColorspace);
        SAFE_RELEASE_GST_OBJECT(wmvdecoder->pDecoder);
        SAFE_RELEASE_GST_OBJECT(wmvdecoder->pCaps);
        SAFE_RELEASE_GST_OBJECT(pRenderer->videoDecoder.pBin);
    }
    return retVal;
}

static pkRESULT CreateMPVBin(pkHANDLE hCtx, pkHANDLE *phDecoder)
{
    pkRESULT retVal = pkS_OK;
    AVRenderer_ctx *pRenderer  = (AVRenderer_ctx*)hCtx;
    MPVPrivate_ctx *mpvdecoder = &(pRenderer->videoDecoder.PrivateCtx.mpv_ctx);
    gboolean ret;

    memset(&(pRenderer->videoDecoder), 0, sizeof(AVDecoder_ctx));
    pRenderer->videoDecoder.pBufferPool = s_pVideoBufferPool;
    ResetBufferPool(pRenderer->videoDecoder.pBufferPool);
    pRenderer->selectionFlags |= pkSTREAM_SELECTION_FLAG_VIDEO;
    pRenderer->videoDecoder.pBin = gst_bin_new("mpvbin");
    CHECKPTR_GOTOBAIL(pRenderer->videoDecoder.pBin);
    mpvdecoder->pCaps = gst_element_factory_make("capsfilter", "mpv-caps");
    CHECKPTR_GOTOBAIL(mpvdecoder->pCaps);

    mpvdecoder->pDecoder = gst_element_factory_make(
        GETGSTELEMENTFROMCMDLINE(CMDOPT_MPEG2, "ffdec_mpeg2video"), //  flumpeg2vdec
        "mpv-decoder");
    CHECKPTR_GOTOBAIL(mpvdecoder->pDecoder);

    mpvdecoder->pColorspace = gst_element_factory_make("ffmpegcolorspace", "colorspace");
    CHECKPTR_GOTOBAIL(mpvdecoder->pColorspace);
    pRenderer->videoDecoder.pSink = gst_element_factory_make("autovideosink", "videosink");
    CHECKPTR_GOTOBAIL(pRenderer->videoDecoder.pSink);
    pRenderer->videoDecoder.pSource = gst_element_factory_make("fakesrc","mpv-source");
    CHECKPTR_GOTOBAIL(pRenderer->videoDecoder.pSource);

    g_object_set(G_OBJECT(mpvdecoder->pCaps), "caps",
        gst_caps_new_simple("video/mpeg",
        "mpegversion", G_TYPE_INT, 2,
        "systemstream", G_TYPE_BOOLEAN, FALSE,
        NULL), NULL);

    InitializeFakesrc(pRenderer->videoDecoder.pSource, (gpointer)&(pRenderer->videoDecoder));
    EnableTimeBasedSeek(pRenderer->videoDecoder.pSource);

    // Setup xoverlay window
    retVal = SetSinkWindow(pRenderer->videoDecoder.pSink);
    CHECKHR_GOTOBAIL(retVal);

    // Setup bin, gst_bin_add_many returns void.
    gst_bin_add_many(GST_BIN(pRenderer->videoDecoder.pBin),
        pRenderer->videoDecoder.pSource,
        mpvdecoder->pCaps,
        mpvdecoder->pDecoder,
        mpvdecoder->pColorspace,
        pRenderer->videoDecoder.pSink,
        NULL);
    ret = gst_element_link_many(pRenderer->videoDecoder.pSource,
        mpvdecoder->pCaps,
        mpvdecoder->pDecoder,
        mpvdecoder->pColorspace,
        pRenderer->videoDecoder.pSink, NULL);
    CHECKBOOLEAN_GOTOBAIL(ret);

    *phDecoder = &(pRenderer->videoDecoder);

bail:
    if(pkFAILED(retVal))
    {
        SAFE_RELEASE_GST_OBJECT(pRenderer->videoDecoder.pSource);
        SAFE_RELEASE_GST_OBJECT(pRenderer->videoDecoder.pSink);
        SAFE_RELEASE_GST_OBJECT(mpvdecoder->pColorspace);
        SAFE_RELEASE_GST_OBJECT(mpvdecoder->pDecoder);
        SAFE_RELEASE_GST_OBJECT(mpvdecoder->pCaps);
        SAFE_RELEASE_GST_OBJECT(pRenderer->videoDecoder.pBin);
    }
    return retVal;
}

static pkRESULT CreateH264Bin(pkHANDLE hCtx, pkHANDLE *phDecoder, pkAV_BITMAPINFOHEADER *pBitmapHeader)
{
    pkRESULT retVal = pkS_OK;
    AVRenderer_ctx *pRenderer     = (AVRenderer_ctx*)hCtx;
    H264Private_ctx *pH264decoder = &(pRenderer->videoDecoder.PrivateCtx.h264_ctx);
    gboolean ret;

    memset(&(pRenderer->videoDecoder), 0, sizeof(pRenderer->videoDecoder));
    pRenderer->videoDecoder.pBufferPool = s_pVideoBufferPool;
    ResetBufferPool(pRenderer->videoDecoder.pBufferPool);
    memset(pH264decoder, 0, sizeof(*pH264decoder));
    pRenderer->selectionFlags |= pkSTREAM_SELECTION_FLAG_VIDEO;
    pRenderer->videoDecoder.pBin = gst_bin_new("h264bin");
    CHECKPTR_GOTOBAIL(pRenderer->videoDecoder.pBin);
    pH264decoder->pCaps = gst_element_factory_make("capsfilter", "h264-caps_filter");
    CHECKPTR_GOTOBAIL(pH264decoder->pCaps);

    pH264decoder->pDecoder = gst_element_factory_make(
        GETGSTELEMENTFROMCMDLINE(CMDOPT_H264, "ffdec_h264"), // ffdec_h264, fluh264dec
        "h264-decoder");
    CHECKPTR_GOTOBAIL(pH264decoder->pDecoder);

    pH264decoder->pColorspace = gst_element_factory_make("ffmpegcolorspace", "colorspace");
    CHECKPTR_GOTOBAIL(pH264decoder->pColorspace);
    pRenderer->videoDecoder.pSink = gst_element_factory_make("autovideosink", "videosink");
    CHECKPTR_GOTOBAIL(pRenderer->videoDecoder.pSink);
    pRenderer->videoDecoder.pSource = gst_element_factory_make("fakesrc","h264-source");
    CHECKPTR_GOTOBAIL(pRenderer->videoDecoder.pSource);

    g_object_set(G_OBJECT(pH264decoder->pCaps), "caps",
        gst_caps_new_simple("video/x-h264",
        "variant", G_TYPE_STRING, "itu",
        "framerate", GST_TYPE_FRACTION, DEFAULT_FRAME_RATE, DEFAULT_FRAME_RATE_SCALE,
        "width", G_TYPE_INT, pBitmapHeader->biWidth,
        "height", G_TYPE_INT, pBitmapHeader->biHeight,
        NULL), NULL);

    InitializeFakesrc(pRenderer->videoDecoder.pSource, (gpointer)&(pRenderer->videoDecoder));
    EnableTimeBasedSeek(pRenderer->videoDecoder.pSource);

    // Setup xoverlay window.
    retVal = SetSinkWindow(pRenderer->videoDecoder.pSink);
    CHECKHR_GOTOBAIL(retVal);

    // Setup bin, gst_bin_add_many returns void.
    gst_bin_add_many(GST_BIN(pRenderer->videoDecoder.pBin),
        pRenderer->videoDecoder.pSource,
        pH264decoder->pCaps,
        pH264decoder->pDecoder,
        pH264decoder->pColorspace,
        pRenderer->videoDecoder.pSink,
        NULL);
    ret = gst_element_link_many(pRenderer->videoDecoder.pSource,
        pH264decoder->pCaps,
        pH264decoder->pDecoder,
        pH264decoder->pColorspace,
        pRenderer->videoDecoder.pSink, NULL);
    CHECKBOOLEAN_GOTOBAIL(ret);

    *phDecoder = &(pRenderer->videoDecoder);

bail:
    if(pkFAILED(retVal))
    {
        SAFE_RELEASE_GST_OBJECT(pRenderer->videoDecoder.pSource);
        SAFE_RELEASE_GST_OBJECT(pRenderer->videoDecoder.pSink);
        SAFE_RELEASE_GST_OBJECT(pH264decoder->pColorspace);
        SAFE_RELEASE_GST_OBJECT(pH264decoder->pDecoder);
        SAFE_RELEASE_GST_OBJECT(pH264decoder->pCaps);
        SAFE_RELEASE_GST_OBJECT(pRenderer->videoDecoder.pBin);
    }
    return retVal;
}

static pkRESULT CreateAC3Bin(pkHANDLE hCtx, pkHANDLE *phDecoder)
{
    pkRESULT retVal = pkS_OK;
    AVRenderer_ctx *pRenderer   = (AVRenderer_ctx*)hCtx;
    AC3Private_ctx *pAc3decoder = &(pRenderer->audioDecoder.PrivateCtx.ac3_ctx);
    gboolean ret;

    memset(&(pRenderer->audioDecoder), 0, sizeof(pRenderer->audioDecoder));
    pRenderer->audioDecoder.pBufferPool = s_pAudioBufferPool;
    ResetBufferPool(pRenderer->audioDecoder.pBufferPool);
    pRenderer->selectionFlags |= pkSTREAM_SELECTION_FLAG_AUDIO;
    pRenderer->audioDecoder.pBin = gst_bin_new("ac3bin");
    CHECKPTR_GOTOBAIL(pRenderer->audioDecoder.pBin);
    pAc3decoder->pDecoder = gst_element_factory_make("a52dec", "ac3-decoder");
    CHECKPTR_GOTOBAIL(pAc3decoder->pDecoder);
    pAc3decoder->pConvert = gst_element_factory_make("audioconvert", "convert");
    CHECKPTR_GOTOBAIL(pAc3decoder->pConvert);
    pAc3decoder->pAudioResample = gst_element_factory_make("audioresample", "aresample");
    CHECKPTR_GOTOBAIL(pAc3decoder->pAudioResample);
    pRenderer->pVolume = gst_element_factory_make("volume", "volume");
    CHECKPTR_GOTOBAIL(pRenderer->pVolume);

    pRenderer->audioDecoder.pSink = gst_element_factory_make(
        GETGSTELEMENTFROMCMDLINE(CMDOPT_AUDIOSINK, "alsasink"),
        "audiosink");
    CHECKPTR_GOTOBAIL(pRenderer->audioDecoder.pSink);

    pRenderer->audioDecoder.pSource = gst_element_factory_make("fakesrc","ac3-source");
    CHECKPTR_GOTOBAIL(pRenderer->audioDecoder.pSource);

    InitializeFakesrc(pRenderer->audioDecoder.pSource, (gpointer)&(pRenderer->audioDecoder));
    EnableTimeBasedSeek(pRenderer->audioDecoder.pSource);

    // Setup bin, gst_bin_add_many returns void.
    gst_bin_add_many(GST_BIN(pRenderer->audioDecoder.pBin),
        pRenderer->audioDecoder.pSource,
        pAc3decoder->pDecoder,
        pAc3decoder->pConvert,
        pAc3decoder->pAudioResample,
        pRenderer->pVolume,
        pRenderer->audioDecoder.pSink,
        NULL);
    ret = gst_element_link_many(pRenderer->audioDecoder.pSource,
        pAc3decoder->pDecoder,
        pAc3decoder->pConvert,
        pAc3decoder->pAudioResample,
        pRenderer->pVolume,
        pRenderer->audioDecoder.pSink, NULL);
    CHECKBOOLEAN_GOTOBAIL(ret);

    *phDecoder = &(pRenderer->audioDecoder);

bail:
    if(pkFAILED(retVal))
    {
        SAFE_RELEASE_GST_OBJECT(pRenderer->audioDecoder.pSource);
        SAFE_RELEASE_GST_OBJECT(pRenderer->audioDecoder.pSink);
        SAFE_RELEASE_GST_OBJECT(pRenderer->pVolume);
        SAFE_RELEASE_GST_OBJECT(pAc3decoder->pAudioResample);
        SAFE_RELEASE_GST_OBJECT(pAc3decoder->pConvert);
        SAFE_RELEASE_GST_OBJECT(pAc3decoder->pDecoder);
        SAFE_RELEASE_GST_OBJECT(pRenderer->audioDecoder.pBin);
    }
    return retVal;
}

static pkRESULT CreateAACBin(pkHANDLE hCtx, pkHANDLE *phDecoder,
        pkAV_WAVEFORMATEX *pWave)
{
    pkRESULT retVal = pkS_OK;
    AVRenderer_ctx *pRenderer  = (AVRenderer_ctx*)hCtx;
    AACPrivate_ctx *aacdecoder = &(pRenderer->audioDecoder.PrivateCtx.aac_ctx);
    uint32_t samplesPerSec = (pWave->nSamplesPerSec != 0) ? pWave->nSamplesPerSec : DEFAULT_AAC_SAMPLERATE;
    uint16_t channels = (pWave->nChannels != 0) ? pWave->nChannels : DEFAULT_AAC_CHANNELS;
    gboolean ret;

    memset(&(pRenderer->audioDecoder), 0, sizeof(AVDecoder_ctx));
    pRenderer->audioDecoder.pBufferPool = s_pAudioBufferPool;
    ResetBufferPool(pRenderer->audioDecoder.pBufferPool);
    pRenderer->selectionFlags |= pkSTREAM_SELECTION_FLAG_AUDIO;
    pRenderer->audioDecoder.pBin = gst_bin_new("aacbin");
    CHECKPTR_GOTOBAIL(pRenderer->audioDecoder.pBin);

    aacdecoder->pDecoder = gst_element_factory_make(
        GETGSTELEMENTFROMCMDLINE(CMDOPT_AAC, "ffdec_aac"), // ffdec_aac, faad
        "aac-decoder");
    CHECKPTR_GOTOBAIL(aacdecoder->pDecoder);

    aacdecoder->pCaps = gst_element_factory_make("capsfilter", "aac-caps");
    CHECKPTR_GOTOBAIL(aacdecoder->pCaps);
    aacdecoder->pConvert = gst_element_factory_make("audioconvert", "convert");
    CHECKPTR_GOTOBAIL(aacdecoder->pConvert);
    aacdecoder->pAudioResample = gst_element_factory_make("audioresample", "aresample");
    CHECKPTR_GOTOBAIL(aacdecoder->pAudioResample);
    pRenderer->pVolume = gst_element_factory_make("volume", "volume");
    CHECKPTR_GOTOBAIL(pRenderer->pVolume);

    pRenderer->audioDecoder.pSink = gst_element_factory_make(
        GETGSTELEMENTFROMCMDLINE(CMDOPT_AUDIOSINK, "alsasink"),
        "audiosink");
    CHECKPTR_GOTOBAIL(pRenderer->audioDecoder.pSink);

    pRenderer->audioDecoder.pSource = gst_element_factory_make("fakesrc","aac-source");
    CHECKPTR_GOTOBAIL(pRenderer->audioDecoder.pSource);

    PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: AAC decoder SampleRate: %d Channels: %d. \n", 
        __FUNCTION__, __LINE__, samplesPerSec, channels));

    g_object_set(G_OBJECT(aacdecoder->pCaps), "caps",
        gst_caps_new_simple("audio/mpeg",
            "rate", G_TYPE_INT, samplesPerSec,
            "channels", G_TYPE_INT, channels,
            "mpegversion", G_TYPE_INT, 4,
            "stream-format", G_TYPE_STRING, "adts",
            NULL), 
        NULL);

    InitializeFakesrc(pRenderer->audioDecoder.pSource, (gpointer)&(pRenderer->audioDecoder));
    EnableTimeBasedSeek(pRenderer->audioDecoder.pSource);

    // Setup bin, gst_bin_add_many returns void.
    gst_bin_add_many(GST_BIN(pRenderer->audioDecoder.pBin),
        pRenderer->audioDecoder.pSource,
        aacdecoder->pCaps,
        aacdecoder->pDecoder,
        aacdecoder->pConvert,
        aacdecoder->pAudioResample,
        pRenderer->pVolume,
        pRenderer->audioDecoder.pSink,
        NULL);
    ret = gst_element_link_many(pRenderer->audioDecoder.pSource,
        aacdecoder->pCaps,
        aacdecoder->pDecoder,
        aacdecoder->pConvert,
        aacdecoder->pAudioResample,
        pRenderer->pVolume,
        pRenderer->audioDecoder.pSink, NULL);
    CHECKBOOLEAN_GOTOBAIL(ret);

    *phDecoder = &(pRenderer->audioDecoder);

bail:
    if(pkFAILED(retVal))
    {
        SAFE_RELEASE_GST_OBJECT(pRenderer->audioDecoder.pSource);
        SAFE_RELEASE_GST_OBJECT(pRenderer->audioDecoder.pSink);
        SAFE_RELEASE_GST_OBJECT(pRenderer->pVolume);
        SAFE_RELEASE_GST_OBJECT(aacdecoder->pAudioResample);
        SAFE_RELEASE_GST_OBJECT(aacdecoder->pConvert);
        SAFE_RELEASE_GST_OBJECT(aacdecoder->pDecoder);
        SAFE_RELEASE_GST_OBJECT(pRenderer->audioDecoder.pBin);
    }
    return retVal;
}

static pkRESULT ChangePipelineState(AVRenderer_ctx *pRenderer, GstState state, int64_t timeout)//ns
{
    pkRESULT retVal = pkS_OK;
    GstStateChangeReturn ret;

    CHECKPTR_GOTOBAIL(pRenderer);
    CHECKPTR_GOTOBAIL(pRenderer->pPipeline);

    PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: Try to set pipeline state to %d\n", __FUNCTION__, __LINE__, state));

    gst_element_set_state(pRenderer->pPipeline, state);
    ret = gst_element_get_state(pRenderer->pPipeline, NULL, NULL, timeout);
    if(GST_STATE_CHANGE_FAILURE == ret)
    {
        retVal = pkE_FAIL;
        PALPRINTMSG(PALPRINT_ERROR, ("Failed to change pipeline state to %d\n", state));
        goto bail;
    }
    else if(GST_STATE_CHANGE_ASYNC == ret)
    {
        retVal = pkS_FALSE;
        PALPRINTMSG(PALPRINT_WARNING, ("%s():line[%d]: Time out (%lld ns) to change pipeline state to %d\n",
            __FUNCTION__, __LINE__, timeout, state));
        goto bail;
    }

    PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: Succeeded setting pipeline state to %d \n",
        __FUNCTION__, __LINE__, state));

bail:
    return retVal;
}

static void PrePipelineRunning(AVRenderer_ctx *pRenderer, bool_t bStarted)
{
    pkRESULT retVal = pkS_OK;

    CHECKPTR_GOTOBAIL(pRenderer);
    CHECKPTR_GOTOBAIL(pRenderer->pPipeline);

    if(pRenderer->dwPreRunningFlags != 0)
    {
        // Note: GstClockTime is a guint64 which is type compatible with uint64_t
        GstClockTime tsStartNS = AVRP_TS_INT64_MAX_VALUE;
        GstSeekFlags seekFlag = GST_SEEK_FLAG_NONE;
        if(pRenderer->dwPreRunningFlags & PRERUNNING_FLAG_NEWPCR)
        {
            SCALEPTSTO1KHZ_ONLY(pkASSERT(FALSE)); // We had assumed all incoming timestamps are in milliseconds
            tsStartNS = 0; // all timestamps are relative to pRenderer->uiPCRMS so we start from zero here
        }
        else if(bStarted) // if called in Renderer_Resume(), we need get current position in Time format
        {
            if(pRenderer->audioDecoder.pBufferPool != NULL)
            {
                if (tsStartNS > pRenderer->audioDecoder.pBufferPool->tsMinSubmittedNs)
                {
                    tsStartNS = pRenderer->audioDecoder.pBufferPool->tsMinSubmittedNs;
                }
            }

            if(pRenderer->videoDecoder.pBufferPool != NULL)
            {
                if (tsStartNS > pRenderer->videoDecoder.pBufferPool->tsMinSubmittedNs)
                {
                    tsStartNS = pRenderer->videoDecoder.pBufferPool->tsMinSubmittedNs;
                }
            }

            if(AVRP_TS_INT64_MAX_VALUE == tsStartNS)
            {
                GstFormat format = GST_FORMAT_TIME;
                gint64 position;
                gboolean bRet = gst_element_query_position(pRenderer->pPipeline, &format, &position);
                if(!bRet || format != GST_FORMAT_TIME)
                {
                    PALPRINTMSG(PALPRINT_ERROR,
                        ("%s():line[%d]: cannot query current position or format isn't GST_FORMAT_TIME. set position to 0\n",
                        __FUNCTION__, __LINE__));
                    tsStartNS = 0;
                }
                else
                {
                    tsStartNS = (GstClockTime)position;
                }
            }
        }

        if((pRenderer->dwPreRunningFlags & PRERUNNING_FLAG_NEWPLAYRATE) && (pRenderer->audioDecoder.pBin != NULL))
        {
            if(NORMAL_PLAY_RATE == pRenderer->iPlayRate)
            {
                gst_bin_add(GST_BIN(pRenderer->pPipeline), pRenderer->audioDecoder.pBin);
                pRenderer->selectionFlags |= pkSTREAM_SELECTION_FLAG_AUDIO;
            }
            else
            {
                if(pRenderer->selectionFlags & pkSTREAM_SELECTION_FLAG_AUDIO)
                {
                    gst_bin_remove(GST_BIN(pRenderer->pPipeline), pRenderer->audioDecoder.pBin);
                    pRenderer->selectionFlags &= ~pkSTREAM_SELECTION_FLAG_AUDIO;
                }
            }
        }

        if((pRenderer->dwPreRunningFlags & PRERUNNING_FLAG_NEWPCR) ||
            (pRenderer->dwPreRunningFlags & PRERUNNING_FLAG_FLUSH))
        {
            seekFlag = GST_SEEK_FLAG_FLUSH;
        }

        PALPRINTMSG(PALPRINT_AVRENDERER,("%s():line[%d]: do seek flag:%d play rate:%d, start time:%"GST_TIME_FORMAT"\n",
                    __FUNCTION__, __LINE__, seekFlag, pRenderer->iPlayRate, GST_TIME_ARGS(tsStartNS)));

        if(pRenderer->selectionFlags & pkSTREAM_SELECTION_FLAG_VIDEO)
        {
            // Note: Normal play rate is used because in trick play mode sample 
            //       I-frame timestamps are synthesized for normal rate use.
            
            if(!gst_element_seek(
                pRenderer->videoDecoder.pSource, 
                gdouble(NORMAL_PLAY_RATE), 
                GST_FORMAT_TIME,
                seekFlag, 
                GST_SEEK_TYPE_SET, 
                (gint64)tsStartNS,
                GST_SEEK_TYPE_NONE, 
                GST_CLOCK_TIME_NONE))
            {
                PALPRINTMSG(PALPRINT_ERROR,
                    ("%s():line[%d]: Failed to send video seek event. play rate:%d, start time:%"GST_TIME_FORMAT"\n",
                    __FUNCTION__, __LINE__,
                    pRenderer->iPlayRate,
                    GST_TIME_ARGS(tsStartNS)));
            }
        }

        if((pRenderer->selectionFlags & pkSTREAM_SELECTION_FLAG_AUDIO) && (NORMAL_PLAY_RATE == pRenderer->iPlayRate))
        {
            if(!gst_element_seek(
                pRenderer->audioDecoder.pSource, 
                gdouble(NORMAL_PLAY_RATE), 
                GST_FORMAT_TIME,
                seekFlag, 
                GST_SEEK_TYPE_SET, 
                (gint64)tsStartNS,
                GST_SEEK_TYPE_NONE, 
                GST_CLOCK_TIME_NONE))
            {
                PALPRINTMSG(PALPRINT_ERROR,
                    ("%s():line[%d]: Failed to send audio seek event. play rate:%d, start time:%"GST_TIME_FORMAT"\n",
                    __FUNCTION__, __LINE__,
                    pRenderer->iPlayRate,
                    GST_TIME_ARGS(tsStartNS)));
            }
        }
        pRenderer->dwPreRunningFlags = 0;
    }

bail:
    return;
}

//=============================================================================
// P U B L I C    F U N C T I O N    D E F I N I T I O N S
//=============================================================================

pkRESULT Open(pkHANDLE *hCtx)
{
    pkRESULT retVal = pkE_POINTER;
    Executive_EnterLock(s_hAVRendererLock);
    PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]\n", __FUNCTION__, __LINE__));

    if(hCtx)
    {
        if(NULL == s_pRendererCtx)
        {
            PALPRINTMSG(PALPRINT_ERROR, ("%s():line[%d]: AVRenderer not started. \n", __FUNCTION__, __LINE__));
            retVal = pkE_UNEXPECTED;
            CHECKHR_GOTOBAIL(retVal);
        }

        if(s_pRendererCtx->fInUse)
        {
            PALPRINTMSG(PALPRINT_ERROR, ("%s():line[%d]: AVRenderer already in use. \n", __FUNCTION__, __LINE__));
            retVal = pkE_UNEXPECTED;
            CHECKHR_GOTOBAIL(retVal);
        }

        // Initialize s_pRendererCtx
        s_pRendererCtx->fInUse = TRUE;
        *hCtx = (pkHANDLE)s_pRendererCtx;
        retVal = pkS_OK;
    }
bail:
    Executive_ExitLock(s_hAVRendererLock);
    return retVal;
}

pkRESULT Close(pkHANDLE hCtx)
{
    Executive_EnterLock(s_hAVRendererLock);
    PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: close renderer. \n", __FUNCTION__, __LINE__));

    if(hCtx)
    {
        AVRenderer_ctx *pRenderer = (AVRenderer_ctx *)hCtx;
        pRenderer->fInUse         = FALSE;
    }
    Executive_ExitLock(s_hAVRendererLock);
    return pkS_OK;
}

pkRESULT OpenDecoder
(
    pkHANDLE hCtx,
    pkAV_STREAM_DESCRIPTOR *pStreamDescriptor,
    pkBUFFER_POOL_DESCRIPTOR *pBufferPoolDesc,
    pkHANDLE *phDecoder
)
{
    pkRESULT retVal = pkE_POINTER;
    GstBus *pBus = NULL;
    AVRenderer_ctx *pRenderer = NULL;
    uint32_t prevSelectionFlags = 0;

    PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: types Aud:%d, Vid:%d, Txt:%d, Sys:%d\n", __FUNCTION__, __LINE__,
        (int)pStreamDescriptor->formatType.eAudioType,
        (int)pStreamDescriptor->formatType.eVideoType,
        (int)pStreamDescriptor->formatType.eVbiType,
        (int)pStreamDescriptor->formatType.eSystemType));

    Executive_EnterLock(s_hAVRendererLock);
    if(hCtx && pStreamDescriptor && pBufferPoolDesc && phDecoder)
    {
        pRenderer = (AVRenderer_ctx *)hCtx;
        prevSelectionFlags = pRenderer->selectionFlags;
        AVDecoder_ctx *pDecoder   = NULL;
        uint32_t dwDefaultAvgBitrate = 0;

        retVal = pkS_OK;

        if( 0 == prevSelectionFlags )
        {
            if( NULL == pRenderer->pPipeline )
            {
                PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d] creating Pipeline CAVRendererGst \n",
                    __FUNCTION__, __LINE__));

                pRenderer->pPipeline = gst_pipeline_new("AV_Renderer");
                CHECKPTR_GOTOBAIL(pRenderer->pPipeline);

                // Connect bus callback.
                pBus = gst_pipeline_get_bus(GST_PIPELINE(pRenderer->pPipeline));
                CHECKPTR_GOTOBAIL(pBus);

                gst_bus_add_watch(pBus, GstMsgCenter, (gpointer)(pRenderer));
                gst_object_unref(pBus);
            }
            else
            {
                PALPRINTMSG(PALPRINT_ERROR, ("%s():line[%d] Inconsistent selectionFlags and pipeline state \n",
                    __FUNCTION__, __LINE__));
                retVal = pkE_UNEXPECTED;
                goto bail;
            }
        }

        // Start pipeline manually according to content type/format
        if(pStreamDescriptor->formatType.eVideoType != pkVIDEO_FORMAT_TYPE_NONE)
        {
            PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: initialize VIDEO decoder. \n", __FUNCTION__, __LINE__));
            switch(pStreamDescriptor->formatType.eVideoType)
            {
            case pkVIDEO_FORMAT_TYPE_MPEG2:
                retVal = CreateMPVBin(hCtx, phDecoder);
                dwDefaultAvgBitrate = DEFAULT_MPV_BITRATE;
                CHECKHR_GOTOBAIL(retVal);
                break;
            case pkVIDEO_FORMAT_TYPE_WMV9:
            case pkVIDEO_FORMAT_TYPE_WVC1:
                retVal = CreateWMVBin(hCtx, phDecoder, &pStreamDescriptor->bitmapInfo);
                dwDefaultAvgBitrate = DEFAULT_WMV_BITRATE;
                CHECKHR_GOTOBAIL(retVal);
                break;
            case pkVIDEO_FORMAT_TYPE_H264: // MPEG-4 part 10
                retVal = CreateH264Bin(hCtx, phDecoder, &pStreamDescriptor->bitmapInfo);
                dwDefaultAvgBitrate = DEFAULT_H264_BITRATE;
                CHECKHR_GOTOBAIL(retVal);
                break;
            case pkVIDEO_FORMAT_TYPE_M4P2: // MPEG-4 part 2 (aka XVID)
            case pkVIDEO_FORMAT_TYPE_WMV7:
            case pkVIDEO_FORMAT_TYPE_WMV8:
            case pkVIDEO_FORMAT_TYPE_WMVP:
            case pkVIDEO_FORMAT_TYPE_WVP2:
            case pkVIDEO_FORMAT_TYPE_WMVA:
                PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: This video type %d is not supported. \n",
                    __FUNCTION__, __LINE__, pStreamDescriptor->formatType.eVideoType));
                retVal = pkE_RENDERER_STREAM_TYPE_NOT_SUPPORTED;
                CHECKHR_GOTOBAIL(retVal);
                break;
            default:
                {
                    PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: Unknown video type. \n", __FUNCTION__, __LINE__));
                    retVal = pkE_RENDERER_STREAM_TYPE_NOT_SUPPORTED;
                    CHECKHR_GOTOBAIL(retVal);
                }
                break;
            }
        }
        else if(pStreamDescriptor->formatType.eAudioType != pkAUDIO_FORMAT_TYPE_NONE)
        {
            PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: initialize AUDIO decoder. \n", __FUNCTION__, __LINE__));
            switch(pStreamDescriptor->formatType.eAudioType)
            {
            case pkAUDIO_FORMAT_TYPE_AC3:
                retVal = CreateAC3Bin(hCtx, phDecoder);
                dwDefaultAvgBitrate = DEFAULT_AC3_BITRATE;
                CHECKHR_GOTOBAIL(retVal);
                break;
            case pkAUDIO_FORMAT_TYPE_PCM:
                retVal = CreatePCMBin(hCtx, phDecoder, &pStreamDescriptor->waveFormat);
                dwDefaultAvgBitrate = DEFAULT_PCM_BITRATE;
                CHECKHR_GOTOBAIL(retVal);
                break;
            case pkAUDIO_FORMAT_TYPE_MPEG:
            case pkAUDIO_FORMAT_TYPE_MP3:
                retVal = CreateMP3Bin(hCtx, phDecoder, &pStreamDescriptor->waveFormat);
                dwDefaultAvgBitrate = DEFAULT_MP3_BITRATE;
                CHECKHR_GOTOBAIL(retVal);
                break;
            case pkAUDIO_FORMAT_TYPE_WMA8:
            case pkAUDIO_FORMAT_TYPE_WMA9:
            case pkAUDIO_FORMAT_TYPE_WMA9Pro:
                retVal = CreateWMABin(hCtx, phDecoder, &pStreamDescriptor->waveFormat);
                dwDefaultAvgBitrate = DEFAULT_WMA_BITRATE;
                CHECKHR_GOTOBAIL(retVal);
                break;
            case pkAUDIO_FORMAT_TYPE_AAC:
                retVal = CreateAACBin(hCtx, phDecoder, &pStreamDescriptor->waveFormat);
                dwDefaultAvgBitrate = DEFAULT_AAC_BITRATE;
                CHECKHR_GOTOBAIL(retVal);
                break;
            case pkAUDIO_FORMAT_TYPE_WMALossless:
                PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: Audio type WMA Lossless is not supported. \n",
                    __FUNCTION__, __LINE__));
                retVal = pkE_RENDERER_STREAM_TYPE_NOT_SUPPORTED;
                CHECKHR_GOTOBAIL(retVal);
                break;
            case pkAUDIO_FORMAT_TYPE_PCM_DVD:
                PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: Audio type PCM_DVD is not supported. \n",
                    __FUNCTION__, __LINE__));
                retVal = pkE_RENDERER_STREAM_TYPE_NOT_SUPPORTED;
                CHECKHR_GOTOBAIL(retVal);
                break;
            default:
                {
                    PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: Unknown audio type. \n", __FUNCTION__, __LINE__));
                    retVal = pkE_RENDERER_STREAM_TYPE_NOT_SUPPORTED;
                    CHECKHR_GOTOBAIL(retVal);
                }
                break;
            }

            SetVolumeLevel(hCtx, VOLUME_DEFAULT_LEVEL);
            SetMuteState(hCtx, FALSE);
        }

        CHECKPTR_GOTOBAIL(*phDecoder);
        pDecoder = (AVDecoder_ctx *)(*phDecoder);
        pDecoder->avgBitRate = pStreamDescriptor->avgBitRate ? pStreamDescriptor->avgBitRate : dwDefaultAvgBitrate;

        // Buffer initial
        pBufferPoolDesc->cBuffers = pDecoder->pBufferPool->cBufferCount;
        pBufferPoolDesc->cbBuffer = pDecoder->pBufferPool->cbSingleBufferSize;
        pBufferPoolDesc->cbAlign  = 1; // info is not available
        pBufferPoolDesc->cbPrefix = 0;

        gst_bin_add(GST_BIN(pRenderer->pPipeline), ((AVDecoder_ctx*)(*phDecoder))->pBin);
        gst_object_ref(((AVDecoder_ctx*)(*phDecoder))->pBin);

        pRenderer->fAvailable = FALSE;
        pDecoder->lastNumBufferTransferred = 0;
        pDecoder->stallBeginTime90kHz = 0;

        if (0 != prevSelectionFlags // an additional decoder was opened 
            && ePipelineStatus_Running == pRenderer->eStatus) // and the pipeline is already running
        {
            static const uint32_t msMaxDecoderStartTime = 1000;
            static const uint32_t msDecoderPollTime = 20; // about a frame time
            int tryCount = 0;
            do
            {
                bool isAudio = 0 != ((pRenderer->selectionFlags ^ prevSelectionFlags) & pkSTREAM_SELECTION_FLAG_AUDIO);
                
                GstFormat format = GST_FORMAT_TIME;
                gint64 position = 0;
                
                gboolean bRet = gst_element_query_position(pRenderer->pPipeline, &format, &position);
                
                if(!bRet || format != GST_FORMAT_TIME)
                {
                    PALPRINTMSG(PALPRINT_ERROR,
                        ("%s():line[%d]: cannot query current %s position or format [%d] isn't GST_FORMAT_TIME. \n",
                        __FUNCTION__, __LINE__, isAudio ? "Audio" : "Video", format));
                    retVal = pkE_RENDERER_STREAM_ERROR_POSQUERY;
                    break;
                }

                if (0 != position)
                {
                    if(!gst_element_seek(
                        isAudio ? pRenderer->audioDecoder.pSource : pRenderer->videoDecoder.pSource, 
                        (gdouble)pRenderer->iPlayRate, 
                        GST_FORMAT_TIME,
                        GST_SEEK_FLAG_NONE, 
                        GST_SEEK_TYPE_SET, 
                        position,
                        GST_SEEK_TYPE_NONE, 
                        GST_CLOCK_TIME_NONE))
                    {
                        PALPRINTMSG(PALPRINT_ERROR,
                            ("%s():line[%d]: failed to seek added %s decoder to position %lld\n",
                            __FUNCTION__, __LINE__, isAudio ? "Audio" : "Video", position));
                        retVal = pkE_RENDERER_STREAM_ERROR_SEEK;
                    }
                    else
                    {
                        PALPRINTMSG(PALPRINT_AVRENDERER,
                            ("%s():line[%d]: added %s decoder seeked to position %lld\n",
                            __FUNCTION__, __LINE__, isAudio ? "Audio" : "Video", position));
                    }
                    break;
                }

                if (++tryCount > msMaxDecoderStartTime/msDecoderPollTime) 
                {
                    PALPRINTMSG(PALPRINT_ERROR,
                        ("%s():line[%d]: timed out waiting for non-zero current %s decoder position\n",
                        __FUNCTION__, __LINE__, isAudio ? "Audio" : "Video"));
                    retVal = pkE_RENDERER_STREAM_ERROR_TIMEOUT;
                    break;                    
                } 
                
                PALPRINTMSG(PALPRINT_AVRENDERER,
                    ("%s():line[%d]: waiting for non-zero current %s decoder position\n",
                    __FUNCTION__, __LINE__, isAudio ? "Audio" : "Video"));
                
                Executive_Sleep(msDecoderPollTime); 
            } while (true); // loop until the first decoder gets going
        }
    }
bail:

    // cleanup if there is a failure
    if(pkFAILED(retVal) && pRenderer)
    {
        // close the decoder
        if(*phDecoder)
        {
            CloseDecoder(hCtx, *phDecoder);
        }

        // reset the selection flags
        pRenderer->selectionFlags = prevSelectionFlags;

        // reset pipeline if no other decoders are open
        if((0 == prevSelectionFlags) && (pRenderer->pPipeline))
        {
            PALPRINTMSG(PALPRINT_ERROR,
                ("%s():line[%d] reset Pipeline CAVRendererGst \n",
                __FUNCTION__, __LINE__));            
            pRenderer->fAvailable = TRUE;
            gst_object_unref(GST_OBJECT(pRenderer->pPipeline));
            pRenderer->pPipeline = NULL;
        }
    }
    Executive_ExitLock(s_hAVRendererLock);
    return retVal;
}

pkRESULT CloseDecoder(pkHANDLE hCtx, pkHANDLE hDecoder)
{
    Executive_EnterLock(s_hAVRendererLock);
    PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: close decoder called. \n", __FUNCTION__, __LINE__));
    if(hCtx && hDecoder)
    {
        AVRenderer_ctx *pRenderer = (AVRenderer_ctx *)hCtx;
        AVDecoder_ctx *pDecoder   = (AVDecoder_ctx *)hDecoder;
        uint32_t closeFlag;

        closeFlag = (pDecoder == &pRenderer->audioDecoder) ?
            pkSTREAM_SELECTION_FLAG_AUDIO :
            pkSTREAM_SELECTION_FLAG_VIDEO;

        // If pipeline is running, stop it.
        if(ePipelineStatus_Stopped != pRenderer->eStatus)
        {
            Stop(hCtx, pRenderer->selectionFlags);
        }

        if((closeFlag == pkSTREAM_SELECTION_FLAG_VIDEO) ||
            (pRenderer->selectionFlags & pkSTREAM_SELECTION_FLAG_AUDIO))
        {
            gst_bin_remove(GST_BIN(pRenderer->pPipeline), pDecoder->pBin);
        }
        else
        {
            gst_element_set_state(pDecoder->pBin, GST_STATE_NULL);
        }
        gst_object_unref(pDecoder->pBin);
        pDecoder->pBin = NULL;

        // Reset all buffers.
        ResetBufferPool(pDecoder->pBufferPool);
        pRenderer->selectionFlags &= ~closeFlag;
        if(0 == pRenderer->selectionFlags)
        {
            pRenderer->fAvailable = TRUE;
            if(pRenderer->pPipeline)
            {
                PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d] reset Pipeline CAVRendererGst \n",
                __FUNCTION__, __LINE__));
                gst_object_unref(GST_OBJECT(pRenderer->pPipeline));
                pRenderer->pPipeline = NULL;
            }
        }
    }
    Executive_ExitLock(s_hAVRendererLock);
    return pkS_OK;
}

pkRESULT QueryDecoderBufferPool
(
    pkHANDLE hCtx,
    pkHANDLE hDecoder,
    uint32_t *pNumFree,
    uint32_t *pLastBufferTransferred
)
{
    pkRESULT retVal = pkE_POINTER;
    if(hCtx && hDecoder && pNumFree && pLastBufferTransferred)
    {
        AVDecoder_ctx *pDecoder   = (AVDecoder_ctx*)hDecoder;
        AVBufferPool *pBufferPool = pDecoder->pBufferPool;
        uint32_t cbBuffer, cbSingleBufferSize;
        uint8_t * pbBufferHead, *pbBufferTail, *pbBuffer;

        Executive_EnterLock(pBufferPool->hLock);
        pbBuffer           = pBufferPool->pbBuffer;
        cbBuffer           = pBufferPool->cbBuffer;
        cbSingleBufferSize = pBufferPool->cbSingleBufferSize;
        pbBufferHead       = pBufferPool->pbBufferHead;
        pbBufferTail       = pBufferPool->pbBufferTail;

        if(pbBufferHead >= pbBufferTail)
        {
            *pNumFree = (pbBuffer+cbBuffer-pbBufferHead)/cbSingleBufferSize
                +(pbBufferTail-pbBuffer)/cbSingleBufferSize;
        }
        else
        {
            *pNumFree = (pbBufferTail - pbBufferHead)/cbSingleBufferSize;
        }

        *pLastBufferTransferred = pBufferPool->cTransferred;
        PALPRINTMSG(PALPRINT_AVRENDERER_VERBOSE,
            ("%s():line[%d]: Free: %d, Last: %d. \n",
            __FUNCTION__, __LINE__,
            *pNumFree,
            *pLastBufferTransferred));
        retVal = pkS_OK;
        Executive_ExitLock(pBufferPool->hLock);
    }
    return retVal;
}

pkRESULT GetDecoderBuffer
(
    pkHANDLE hCtx,
    pkHANDLE hDecoder,
    uint8_t **ppBuffer,
    uint32_t *pBufSize,
    uint32_t timeoutMillisecs
)
{
    pkRESULT retVal = pkE_POINTER;
    if(hCtx && hDecoder && ppBuffer && pBufSize)
    {
        AVDecoder_ctx *pDecoder   = (AVDecoder_ctx *)hDecoder;
        AVBufferPool *pBufferPool = pDecoder->pBufferPool;
        uint32_t time = 0;
        uint32_t number = 0;

        while(1)
        {
            retVal = pkS_FALSE;
            if(time > timeoutMillisecs)
            {
                PALPRINTMSG(PALPRINT_AVRENDERER, \
                    ("%s():line[%d]: get %s buffer time out. \n",
                    __FUNCTION__, __LINE__,
                    (pBufferPool == s_pVideoBufferPool)?"VIDEO":"AUDIO"));
                PrintBufferPool(pBufferPool);
                break;
            }

            if(FindAvailableBuffer(pBufferPool, &number))
            {
                PALPRINTMSG(PALPRINT_AVRENDERER_VERBOSE, \
                    ("%s():line[%d]: get %s buffer:%d.\n",
                    __FUNCTION__, __LINE__,
                    (pBufferPool == s_pVideoBufferPool)?"VIDEO":"AUDIO", number));
                *ppBuffer = GST_BUFFER_DATA(pBufferPool->pPool[number].pBuffer);
                *pBufSize = pBufferPool->cbSingleBufferSize;
                pDecoder->currentBuf = number;
                retVal = pkS_OK;
                break;
            }
            time += 20;
            if(timeoutMillisecs)
            {
                Executive_Sleep(20);
            }
        }
    }
    return retVal;
}

pkRESULT SubmitDecoderBuffer
(
    pkHANDLE hCtx,
    pkHANDLE hDecoder,
    uint8_t *pBuffer,
    uint32_t bufLength,
    uint32_t sampleFlags,
    pkSAMPLE_FRAGMENT_TYPE fragmentType,
    int64_t timeStamp
)
{
    uint32_t idx;
    pkRESULT retVal = pkE_POINTER;

    if(hCtx && hDecoder && pBuffer)
    {
        AVRenderer_ctx * pRenderer = (AVRenderer_ctx*)hCtx;
        AVDecoder_ctx * pDecoder   = (AVDecoder_ctx *)hDecoder;
        AVBufferPool * pBufferPool = pDecoder->pBufferPool;
        EAVBufferStatus eStatus;
        uint32_t cBufferCount, cBufferCtxCount;

        PALPRINTMSG(PALPRINT_AVRENDERER_VERBOSE,
            ("%s():line[%d]: submit %s buffer %d, size: %d, timestamp: %d.\n",
            __FUNCTION__, __LINE__,
            (pBufferPool == s_pVideoBufferPool)?"V":"A",
            pDecoder->currentBuf,
            bufLength,
            timeStamp));

        retVal = pkS_OK;
        idx = pDecoder->currentBuf;
        cBufferCtxCount = pBufferPool->cBufferCtxCount; // no lock needed, it's static.

        if(bufLength)
        {
            if(idx < cBufferCtxCount)
            {
                Executive_EnterLock(pBufferPool->hLock);

                cBufferCount = pBufferPool->cBufferCount;
                if(timeStamp < 0)
                {
                    timeStamp = GST_CLOCK_TIME_NONE;
                }
                else
                {
                    // We assume incoming timestamps are in milliseconds
                    SCALEPTSTO1KHZ_ONLY(pkASSERT(FALSE));
                    if (timeStamp > pRenderer->uiPCRMS)
                    {
                        timeStamp -= pRenderer->uiPCRMS;
                        timeStamp *= MS_TO_NS;
                    }
                    else
                    {
                        timeStamp = 0;
                    }

                    if(GST_CLOCK_TIME_NONE == pBufferPool->tsMinSubmittedNs)
                    {
                        pBufferPool->tsMinSubmittedNs = timeStamp;
                    }
                    else if(timeStamp < pBufferPool->tsMinSubmittedNs)
                    {
                        pBufferPool->tsMinSubmittedNs = timeStamp;
                    }
                }


                pBufferPool->cSubmitted++;
                GST_BUFFER_SIZE(pBufferPool->pPool[idx].pBuffer) = bufLength;
                GST_BUFFER_TIMESTAMP(pBufferPool->pPool[idx].pBuffer) = timeStamp;
                GST_BUFFER_DURATION(pBufferPool->pPool[idx].pBuffer) = 1000000000/DEFAULT_FRAME_RATE;

                eStatus = (EAVBufferStatus)Executive_InterlockedExchange(
                    (int32_t*)&(pBufferPool->pPool[idx].eStatus),
                    eAVBufferStatus_Submitted);
                pkASSERT(eStatus == eAVBufferStatus_CheckedOut);
                Executive_InterlockedIncrement((int32_t*)&(pBufferPool->cCtxUsed));
                pBufferPool->pbBufferHead = GST_BUFFER_DATA(pBufferPool->pPool[idx].pBuffer)+bufLength;
                Executive_InterlockedExchange((int32_t*)&(pBufferPool->idxHead), (idx+1)%cBufferCtxCount);

                PALPRINTMSG(PALPRINT_AVRENDERER_VERBOSE,
                    ("Submit %s idxHead: %d\n",
                    (pBufferPool == s_pVideoBufferPool)?"VIDEO":"AUDIO",
                    pBufferPool->idxHead));

                // we need transfer this flag to decoder.
                if (sampleFlags & pkSAMPLEFLAG_DISCONTINUITY)
                {
                    GST_BUFFER_FLAG_SET(pBufferPool->pPool[idx].pBuffer, GST_BUFFER_FLAG_DISCONT);
                }
                else
                {
                    // unset the buffer flag which may be set before.
                    GST_BUFFER_FLAG_UNSET(pBufferPool->pPool[idx].pBuffer, GST_BUFFER_FLAG_DISCONT);
                }

                if (sampleFlags & pkSAMPLEFLAG_KEY_FRAME)
                {
                    GST_BUFFER_FLAG_UNSET(pBufferPool->pPool[idx].pBuffer, GST_BUFFER_FLAG_DELTA_UNIT);
                }
                else
                {
                    GST_BUFFER_FLAG_SET(pBufferPool->pPool[idx].pBuffer, GST_BUFFER_FLAG_DELTA_UNIT);
                }
                Executive_ExitLock(pBufferPool->hLock);
            }
        }
        else // Buffer returned
        {
            Executive_EnterLock(pBufferPool->hLock);
            eStatus = (EAVBufferStatus)Executive_InterlockedExchange(
                (int32_t*)&(pBufferPool->pPool[idx].eStatus),
                eAVBufferStatus_Available);
            pkASSERT(eStatus == eAVBufferStatus_CheckedOut);
            Executive_ExitLock(pBufferPool->hLock);
        }
    }
    return retVal;
}

pkRESULT Start(pkHANDLE hCtx, uint32_t startFlags, uint64_t iStartTime)
{
    pkRESULT retVal = pkS_OK;
    AVRenderer_ctx * pRenderer = (AVRenderer_ctx *)hCtx;
    bool_t fMuted = FALSE;

    CHECKPTR_GOTOBAIL(pRenderer);
    CHECKPTR_GOTOBAIL(pRenderer->pPipeline);

    if(startFlags != pRenderer->selectionFlags && startFlags != pkSTREAM_SELECTION_FLAG_AUDIO_AND_VIDEO) 
    {
        PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: failed - startFlags:%u mismatch selFlags:%u\n",
            __FUNCTION__, __LINE__, startFlags, pRenderer->selectionFlags));
        retVal = pkE_INVALIDARG;
        goto bail;
    }

    PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: iStartTime:%llu, startFlags:%u\n",
        __FUNCTION__, __LINE__, iStartTime, startFlags));
    
    // Start pipeline.

    if((pRenderer->selectionFlags & pkSTREAM_SELECTION_FLAG_AUDIO) && pRenderer->audioDecoder.pBin)
    {
        retVal = _SetMuting_priv(TRUE);
        CHECKHR_GOTOBAIL(retVal);
        fMuted = TRUE;
    }

    if(GST_CLOCK_TIME_NONE != iStartTime)
    {
        SetPCR(hCtx, iStartTime);
    }

    pRenderer->fNoHandoff = TRUE;

    retVal = ChangePipelineState(pRenderer, GST_STATE_PAUSED, 0);
    CHECKHR_GOTOBAIL(retVal);

    PrePipelineRunning(pRenderer, FALSE);
    
    pRenderer->fNoHandoff = FALSE;

    retVal = ChangePipelineState(pRenderer, GST_STATE_PLAYING, STATE_CHANGE_WAIT_TIMEOUT_NS);
    CHECKHR_GOTOBAIL(retVal);
    
    pRenderer->eStatus = ePipelineStatus_Running;

bail:
    if(pkFAILED(retVal) && fMuted)
    {
        CHECKHR_TRACEERROR(_SetMuting_priv(FALSE));
    }
    return retVal;
}

pkRESULT Stop(pkHANDLE hCtx, uint32_t stopFlags)
{
    pkRESULT retVal = pkS_OK;
    AVRenderer_ctx *pRenderer = (AVRenderer_ctx *)hCtx;

    CHECKPTR_GOTOBAIL(pRenderer);
    CHECKPTR_GOTOBAIL(pRenderer->pPipeline);

    if(0 == pRenderer->selectionFlags)
    {
        PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: stopFlags(%d) IGNORED (no selection flags)\n", 
            __FUNCTION__, __LINE__, stopFlags));
        goto bail;
    }
    
    PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: stopFlags(%d) \n", 
        __FUNCTION__, __LINE__, stopFlags));
    
    // Stop pipeline and flush the selected decoders.

    retVal = ChangePipelineState(pRenderer, GST_STATE_NULL, STATE_CHANGE_WAIT_TIMEOUT_NS);
    CHECKHR_GOTOBAIL(retVal);
    
    if((stopFlags & pkSTREAM_SELECTION_FLAG_AUDIO) && pRenderer->audioDecoder.pBin)
    {
        CHECKHR_TRACEERROR(_SetMuting_priv(FALSE));
        FlushDecoder(hCtx, (pkHANDLE)&pRenderer->audioDecoder);
    }
    if((stopFlags & pkSTREAM_SELECTION_FLAG_VIDEO) && pRenderer->videoDecoder.pBin)
    {
        FlushDecoder(hCtx, (pkHANDLE)&pRenderer->videoDecoder);
    }
    
    pRenderer->fNoHandoff = FALSE;
    pRenderer->eStatus = ePipelineStatus_Stopped;
    pRenderer->iPlayRate = NORMAL_PLAY_RATE;
    pRenderer->dwPreRunningFlags = 0;

bail:
    return retVal;
}

pkRESULT Pause(pkHANDLE hCtx)
{
    pkRESULT retVal = pkS_OK;
    AVRenderer_ctx *pRenderer = (AVRenderer_ctx *)hCtx;

    CHECKPTR_GOTOBAIL(pRenderer);
    CHECKPTR_GOTOBAIL(pRenderer->pPipeline);

    // Pause pipeline.
    if(ePipelineStatus_Running != pRenderer->eStatus)
    {
        PALPRINTMSG(PALPRINT_AVRENDERER,("%s():line[%d]: ignored because not running. \n", __FUNCTION__, __LINE__));
        
        retVal = pkS_FALSE;
        goto bail;
    }

    PALPRINTMSG(PALPRINT_AVRENDERER,("%s():line[%d]: pausing. \n", __FUNCTION__, __LINE__));
    
    retVal = ChangePipelineState(pRenderer, GST_STATE_PAUSED, STATE_CHANGE_WAIT_TIMEOUT_NS);
    CHECKHR_GOTOBAIL(retVal);
    
    pRenderer->eStatus = ePipelineStatus_Paused;

    if((pRenderer->selectionFlags & pkSTREAM_SELECTION_FLAG_AUDIO) && pRenderer->audioDecoder.pBin)
    {
        CHECKHR_TRACEERROR(_SetMuting_priv(FALSE));
    }

bail:
    return retVal;
}

pkRESULT Resume(pkHANDLE hCtx)
{
    pkRESULT retVal = pkS_OK;
    AVRenderer_ctx *pRenderer = (AVRenderer_ctx *)hCtx;

    CHECKPTR_GOTOBAIL(pRenderer);
    CHECKPTR_GOTOBAIL(pRenderer->pPipeline);

    PALPRINTMSG(PALPRINT_AVRENDERER,("%s():line[%d]: resuming. \n", __FUNCTION__, __LINE__));

    // Resume pipeline.

    if((pRenderer->selectionFlags & pkSTREAM_SELECTION_FLAG_AUDIO) && pRenderer->audioDecoder.pBin)
    {
        CHECKHR_TRACEERROR(_SetMuting_priv(FALSE));
    }

    PrePipelineRunning(pRenderer, TRUE);

    retVal = ChangePipelineState(pRenderer, GST_STATE_PLAYING, STATE_CHANGE_WAIT_TIMEOUT_NS);
    CHECKHR_GOTOBAIL(retVal);

    pRenderer->eStatus = ePipelineStatus_Running;

bail:
    return retVal;
}

pkRESULT NotifyPrerollStart(pkHANDLE hCtx)
{
    pkRESULT retVal = pkE_POINTER;
    if(hCtx)
    {
        retVal = pkS_OK;
    }
    return retVal;
}

pkRESULT NotifyPrerollComplete(pkHANDLE hCtx)
{
    pkRESULT retVal = pkE_POINTER;
    if(hCtx)
    {
        retVal = pkS_OK;
    }
    return retVal;
}

pkRESULT SetPCR(pkHANDLE hCtx, uint64_t uiPCR)
{
    pkRESULT retVal = pkS_OK;
    AVRenderer_ctx *pRenderer = (AVRenderer_ctx *)hCtx;

    PALPRINTMSG(PALPRINT_AVRENDERER,("%s():line[%d]: PCR %lf. \n", __FUNCTION__, __LINE__, (double)uiPCR));

    CHECKPTR_GOTOBAIL(pRenderer);
    pRenderer->uiPCRMS = uiPCR;
    pRenderer->dwPreRunningFlags |= PRERUNNING_FLAG_NEWPCR;

bail:
    return retVal;
}

pkRESULT SetClockRate(pkHANDLE hCtx, int32_t uiPPM)
{
    pkRESULT retVal = pkE_POINTER;
    if(hCtx)
    {
        retVal = pkS_OK;
    }
    return retVal;
}

pkRESULT SetVolumeLevel(pkHANDLE hCtx, uint32_t uiVolLevel)
{
    pkRESULT retVal = pkE_POINTER;
    if(hCtx)
    {
        AVRenderer_ctx *pRenderer = (AVRenderer_ctx *)hCtx;
        gdouble v;

        // change range from 0-10 to 0-4
        uiVolLevel = (uiVolLevel > 10)?10:uiVolLevel;
        v = (gdouble)uiVolLevel * 0.4;
        if(pRenderer->pVolume)
        {
            PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: set volume level %lf. \n",
                __FUNCTION__, __LINE__, (double)v));
            g_object_set(G_OBJECT(pRenderer->pVolume),
                "volume", v, NULL);
        }
        pRenderer->currentVolume = uiVolLevel;
        retVal = pkS_OK;
    }
    return retVal;
}

pkRESULT GetVolumeLevel(pkHANDLE hCtx, uint32_t *puiVolLevel)
{
    pkRESULT retVal = pkE_POINTER;
    if(hCtx && puiVolLevel)
    {
        AVRenderer_ctx * pRenderer = (AVRenderer_ctx *)hCtx;
        PALPRINTMSG(PALPRINT_AVRENDERER_VERBOSE, ("%s():line[%d]: get volume level. \n",
            __FUNCTION__, __LINE__));
        *puiVolLevel = pRenderer->currentVolume;
        retVal = pkS_OK;
    }
    return retVal;
}

pkRESULT SetMuteState(pkHANDLE hCtx, bool_t isMuted)
{
    pkRESULT retVal = pkE_POINTER;
    if(hCtx)
    {
        AVRenderer_ctx *pRenderer = (AVRenderer_ctx *)hCtx;
        if(pRenderer->pVolume)
        {
            PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: set mute state %d. \n",
                __FUNCTION__, __LINE__, (int)isMuted));
            g_object_set(G_OBJECT(pRenderer->pVolume),
                "mute", (gboolean)isMuted, NULL);
        }
        pRenderer->fIsMute = isMuted;
        retVal = pkS_OK;
    }
    return retVal;
}

pkRESULT GetMuteState(pkHANDLE hCtx, bool_t *pIsMuted)
{
    pkRESULT retVal = pkE_POINTER;
    if(hCtx && pIsMuted)
    {
        AVRenderer_ctx *pRenderer = (AVRenderer_ctx *)hCtx;
        PALPRINTMSG(PALPRINT_AVRENDERER_VERBOSE, ("%s():line[%d]: get mute state. \n",
            __FUNCTION__, __LINE__));
        *pIsMuted = pRenderer->fIsMute;
        retVal = pkS_OK;
    }
    return retVal;
}

pkRESULT SetPlayRate(pkHANDLE hCtx, int32_t iPlayRate)
{
    pkRESULT retVal = pkS_OK;
    AVRenderer_ctx *pRenderer = (AVRenderer_ctx *)hCtx;

    CHECKPTR_GOTOBAIL(pRenderer);

    if(iPlayRate != pRenderer->iPlayRate)
    {
        pRenderer->dwPreRunningFlags |= PRERUNNING_FLAG_NEWPLAYRATE;
    }
    pRenderer->iPlayRate = iPlayRate;
bail:
    return retVal;
}


pkRESULT GetCurrentPTS(pkHANDLE hCtx, uint64_t *pCurrentPTShns)
{
    pkRESULT retVal           = pkE_FAIL;
    gboolean fQueryRet        = FALSE;
    GstFormat fmt             = GST_FORMAT_TIME;
    GstFormat fmtRequested    = fmt;
    gint64 tsCurrPosNS        = 0;
    AVRenderer_ctx *pRenderer = (AVRenderer_ctx *)hCtx;

    CHECKPTR_GOTOBAIL(pRenderer);
    CHECKPTR_GOTOBAIL(pRenderer->pPipeline);
    CHECKPTR_GOTOBAIL(pCurrentPTShns);

    fQueryRet = gst_element_query_position(pRenderer->pPipeline, &fmt, &tsCurrPosNS);

    if(fQueryRet && (fmt == fmtRequested) && (tsCurrPosNS != GST_CLOCK_TIME_NONE))
    {
        tsCurrPosNS += pRenderer->uiPCRMS * MS_TO_NS;
        
        *pCurrentPTShns = (uint64_t)(tsCurrPosNS / 100); //convert to 10 MHz

        PALPRINTMSG(PALPRINT_AVRENDERER_VERBOSE,
            ("%s():line[%d]:stream type:GST_FORMAT_TIME, Current position: %lld hns \n",
            __FUNCTION__, __LINE__, *pCurrentPTShns));

        retVal = pkS_OK;
    }
    else
    {
        PALPRINTMSG(PALPRINT_AVRENDERER,
            ("%s():line[%d]: Failed to get current position\n",
            __FUNCTION__, __LINE__));
    }

bail:
    return retVal;
}

pkRESULT SetEndOfStream(pkHANDLE hRenderer, pkHANDLE hDecoder, bool isEoS)
{
    pkRESULT retVal = pkE_POINTER;
    if(hRenderer && hDecoder)
    {
        AVRenderer_ctx *pRenderer = (AVRenderer_ctx *)hRenderer;
        AVDecoder_ctx *pDecoder   = (AVDecoder_ctx *)hDecoder;
        
        PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: %s EoS %s. \n",
            __FUNCTION__, __LINE__, 
            pDecoder == &(pRenderer->audioDecoder) ? "A" : "V", 
            isEoS ? "TRUE" : "FALSE"));

        pDecoder->fIsEndOfStream = isEoS;
        retVal = pkS_OK;
    }
    return retVal;
}

pkRESULT GetRenderingDone(pkHANDLE hRenderer, pkHANDLE hDecoder, bool *isDone)
{
    pkRESULT retVal = pkE_POINTER;
    if(hRenderer && hDecoder)
    {
        AVRenderer_ctx *pRenderer = (AVRenderer_ctx *)hRenderer;
        AVDecoder_ctx *pDecoder   = (AVDecoder_ctx *)hDecoder;
        
        // TODO: for now consider rendering is done when the bufferpool is empty, which does not consider the last frames in the gst buffers,
        // a better way would be to use a gst notification (Bug 32252)
        *isDone = (pDecoder->fIsEndOfStream && (pDecoder->pBufferPool->cSubmitted == pDecoder->pBufferPool->cTransferred));
        
        PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: %s (%d, %d) %s. \n",
            __FUNCTION__, __LINE__,
            pDecoder == &(pRenderer->audioDecoder) ? "A" : "V",
            pDecoder->pBufferPool->cSubmitted, 
            pDecoder->pBufferPool->cTransferred, 
            *isDone ? "TRUE" : "FALSE"));

        retVal = pkS_OK;
    }
    return retVal;
}

pkRESULT GetDecoderStalled(pkHANDLE hRenderer, pkHANDLE hDecoder, uint64_t systemTime90kHz, bool *isStalled)
{
    pkRESULT retVal = pkE_POINTER;
    if(hRenderer && hDecoder)
    {
        AVRenderer_ctx *pRenderer = (AVRenderer_ctx *)hRenderer;
        AVDecoder_ctx *pDecoder   = (AVDecoder_ctx *)hDecoder;
        
        uint32_t uiLastTransferred = pDecoder->pBufferPool->cTransferred;

        PALPRINTMSG(PALPRINT_AVRENDERER_VERBOSE, ("%s():line[%d]: %s (buffer=[0x%x,0x%x] time=[%llu,%llu]. \n",
                __FUNCTION__, __LINE__,
                pDecoder == &(pRenderer->audioDecoder) ? "A" : "V",
                pDecoder->lastNumBufferTransferred,
                uiLastTransferred,
                pDecoder->stallBeginTime90kHz,
                systemTime90kHz));

        *isStalled = FALSE;
        if (pDecoder->lastNumBufferTransferred == uiLastTransferred &&
            (systemTime90kHz - pDecoder->stallBeginTime90kHz > c_stallTimeLimit90kHz))
        {
            *isStalled = TRUE;
            PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d]: %s %s. \n",
                __FUNCTION__, __LINE__, 
                pDecoder == &(pRenderer->audioDecoder) ? "A" : "V",
                "TRUE"));
        }
        else if (pDecoder->lastNumBufferTransferred != uiLastTransferred)
        {
            pDecoder->lastNumBufferTransferred = uiLastTransferred;
            pDecoder->stallBeginTime90kHz = systemTime90kHz;

            PALPRINTMSG(PALPRINT_AVRENDERER_VERBOSE, ("%s():line[%d]: %s %s. \n",
                __FUNCTION__, __LINE__, 
                pDecoder == &(pRenderer->audioDecoder) ? "A" : "V",
                "FALSE"));
        }


        retVal = pkS_OK;
    }
bail:
    return retVal;
}

pkRESULT pkAPI FlushDecoder(pkHANDLE hCtx, pkHANDLE hDecoder)
{
    pkRESULT retVal = pkS_OK;
    AVDecoder_ctx *pDecoder   = (AVDecoder_ctx*)hDecoder;
    AVRenderer_ctx *pRenderer = (AVRenderer_ctx*)hCtx;
    CHECKPTR_GOTOBAIL(hCtx);
    CHECKPTR_GOTOBAIL(pDecoder);

    PALPRINTMSG(PALPRINT_AVRENDERER,("%s():line[%d]: flush. \n", __FUNCTION__, __LINE__));

    ResetBufferPool(pDecoder->pBufferPool);

    // Set flush flag. For any seek operations like skipping forward/back
    // or resuming, FlushDecoder() will be called to discard all old buffers
    pRenderer->dwPreRunningFlags |= PRERUNNING_FLAG_FLUSH;

bail:
    return retVal;
}

pkRESULT Startup()
{
    pkRESULT retVal = pkS_OK;

    if(s_pRendererCtx || s_hAVRendererLock)
    {
        retVal = pkE_FAIL;
        PALPRINTMSG(PALPRINT_ERROR, ("%s():line[%d]: Failed because renderer has already been started. \n",
            __FUNCTION__, __LINE__));
        goto just_exit;
    }

    PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d] Starting up CAVRendererGst \n",
        __FUNCTION__, __LINE__));
    
    // Create global lock.
    retVal = Executive_CreateLock(&s_hAVRendererLock);
    CHECKHR_GOTOBAIL(retVal);

    // Create renderer context.
    s_pRendererCtx = (AVRenderer_ctx*)Executive_Alloc(sizeof(AVRenderer_ctx), TRUE);
    if(!s_pRendererCtx)
    {
        retVal = pkE_OUTOFMEMORY;
        PALPRINTMSG(PALPRINT_ERROR, ("%s():line[%d]: Failed by out of memory. \n", __FUNCTION__, __LINE__));
        CHECKHR_GOTOBAIL(retVal);
    }

    // Initialize gstreamer.

    _GetCmdLineArg_UInt(CMDOPT_MAX_VIDEO_BUFFER_TIME_SECS, &s_MAX_VIDEO_BUFFER_TIME_SECS);
    _GetCmdLineArg_UInt(CMDOPT_MAX_AUDIO_BUFFER_TIME_SECS, &s_MAX_AUDIO_BUFFER_TIME_SECS);
    _GetCmdLineArg_UInt(CMDOPT_MAX_VIDEO_BYTERATE, &s_MAX_VIDEO_BYTERATE);
    _GetCmdLineArg_UInt(CMDOPT_MAX_AUDIO_BYTERATE, &s_MAX_AUDIO_BYTERATE);
    _GetCmdLineArg_UInt(CMDOPT_MAX_VIDEO_FRAME_SIZE, &s_MAX_VIDEO_FRAME_SIZE);

    // NOTE: gst_init and gst_version return void.
    gst_init(NULL, NULL);
    gst_version (&s_uiMajorVersion, &s_uiMinorVersion, &s_uiMicroVersion, &s_uiNanoVersion);

    s_pRendererCtx->fAvailable        = TRUE;
    s_pRendererCtx->eStatus           = ePipelineStatus_Stopped;
    s_pRendererCtx->iPlayRate         = NORMAL_PLAY_RATE;
    s_pRendererCtx->dwPreRunningFlags = 0;
    s_pRendererCtx->uiPCRMS           = 0;
    s_pRendererCtx->selectionFlags    = 0;
    s_pRendererCtx->fNoHandoff        = FALSE;

    retVal = CreateBufferPool(&s_pVideoBufferPool,
        VIDEO_BUFFER_AMOUNT,
        SINGLE_VIDEO_BUFFER_MAX_SIZE,
        VIDEO_BUFFER_CTX_AMOUNT);
    CHECKHR_GOTOBAIL(retVal);

    retVal = CreateBufferPool(&s_pAudioBufferPool,
        AUDIO_BUFFER_AMOUNT,
        SINGLE_AUDIO_BUFFER_MAX_SIZE,
        AUDIO_BUFFER_CTX_AMOUNT);
    CHECKHR_GOTOBAIL(retVal);

bail:
    if(pkFAILED(retVal))
    {
        if(s_pAudioBufferPool)
        {
            DestroyBufferPool(s_pAudioBufferPool);
            s_pAudioBufferPool = NULL;
        }

        if(s_pVideoBufferPool)
        {
            DestroyBufferPool(s_pVideoBufferPool);
            s_pVideoBufferPool = NULL;
        }

        if(s_pRendererCtx)
        {
            Executive_Free(s_pRendererCtx);
            s_pRendererCtx = NULL;
        }
        if(s_hAVRendererLock)
        {
            Executive_DeleteLock(s_hAVRendererLock);
            s_hAVRendererLock = NULL;
        }

    }
just_exit:
    return retVal;
}

pkRESULT Shutdown()
{
    pkRESULT retVal = pkS_OK;

    if(!s_pRendererCtx || !s_hAVRendererLock)
    {
        retVal = pkE_FAIL;
        PALPRINTMSG(PALPRINT_ERROR, ("%s():line[%d]: failed because Renderer has already been shut down. \n",
            __FUNCTION__, __LINE__));
        goto bail;
    }

    PALPRINTMSG(PALPRINT_AVRENDERER, ("%s():line[%d] Shutting down CAVRendererGst \n",
        __FUNCTION__, __LINE__));

    gst_deinit();

    // TODO: Need to disable GetDecoderBuffer first.
    DestroyBufferPool(s_pVideoBufferPool);
    DestroyBufferPool(s_pAudioBufferPool);
    s_pVideoBufferPool = NULL;
    s_pAudioBufferPool = NULL;
    
    Executive_Free(s_pRendererCtx);
    s_pRendererCtx = NULL;
    Executive_DeleteLock(s_hAVRendererLock);
    s_hAVRendererLock = NULL;
bail:
    return retVal;
}

}// end namespace CAVRendererGst
// ===========================================================


