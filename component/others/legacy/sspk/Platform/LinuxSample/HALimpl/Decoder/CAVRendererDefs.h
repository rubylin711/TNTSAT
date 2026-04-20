///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

enum //sample flags
{
    pkSAMPLEFLAG_KEY_FRAME     = 0x01,
    pkSAMPLEFLAG_DISCONTINUITY = 0x02,
    pkSAMPLEFLAG_ENCRYPTED     = 0x04,
};

#define pkINVALID_TIME_STAMP 0xffffffffffffffffLL
#define pkLARGEST_TIME_STAMP 0x7fffffffffffffffLL

typedef struct _pkRECT {
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
} pkRECT;


// wFormatTag values:
#ifndef WAVE_FORMAT_SDP_PCM
#define WAVE_FORMAT_SDP_PCM     0x0001
#endif
#ifndef WAVE_FORMAT_MPEG
#define WAVE_FORMAT_MPEG        0x0050
#endif
#ifndef WAVE_FORMAT_MPEGLAYER3
#define WAVE_FORMAT_MPEGLAYER3  0x0055
#endif
#ifndef WAVE_FORMAT_WMAVOICE9
#define WAVE_FORMAT_WMAVOICE9   0x000A
#endif
#ifndef WAVE_FORMAT_DOLBY_AC3
#define WAVE_FORMAT_DOLBY_AC3   0x2000
#endif
#ifndef WAVE_FORMAT_AAC // TODO: is there an official tag value for AAC?
#define WAVE_FORMAT_AAC         0x3000
#endif
#ifndef WAVE_FORMAT_WMAUDIO2
#define WAVE_FORMAT_WMAUDIO2    0x0161
#endif
#ifndef WAVE_FORMAT_WMAUDIO3
#define WAVE_FORMAT_WMAUDIO3    0x0162
#endif
#ifndef WAVE_FORMAT_WMAUDIO_LOSSLESS
#define WAVE_FORMAT_WMAUDIO_LOSSLESS 0x0163
#endif

typedef struct
{
    int32_t cBuffers;
    int32_t cbBuffer;
    int32_t cbAlign;
    int32_t cbPrefix;
} pkBUFFER_POOL_DESCRIPTOR;

typedef enum
{
    pkSTREAM_SELECTION_FLAG_VIDEO=0x1,
    pkSTREAM_SELECTION_FLAG_AUDIO=0x2,
    pkSTREAM_SELECTION_FLAG_AUDIO_AND_VIDEO=0x3
} pkSTREAM_SELECTION_FLAG;

typedef enum
{
    pkSAMPLE_FRAGMENT_TYPE_NONE = 0x0,
    pkSAMPLE_FRAGMENT_TYPE_FRAGMENT_BEGIN = 0x1,
    pkSAMPLE_FRAGMENT_TYPE_FRAGMENT_CONTINUE = 0x2,
    pkSAMPLE_FRAGMENT_TYPE_FRAGMENT_END = 0x3,
} pkSAMPLE_FRAGMENT_TYPE;

typedef enum
{
    pkVIDEO_FORMAT_TYPE_NONE,
    pkVIDEO_FORMAT_TYPE_MPEG2,
    pkVIDEO_FORMAT_TYPE_WMV7,
    pkVIDEO_FORMAT_TYPE_WMV8,
    pkVIDEO_FORMAT_TYPE_WMV9,
    pkVIDEO_FORMAT_TYPE_WMVP,
    pkVIDEO_FORMAT_TYPE_WVP2,
    pkVIDEO_FORMAT_TYPE_WVC1,
    pkVIDEO_FORMAT_TYPE_WMVA,
    pkVIDEO_FORMAT_TYPE_H264,
    pkVIDEO_FORMAT_TYPE_M4P2
}  pkVIDEO_FORMAT_TYPE;

typedef enum
{
    pkAUDIO_FORMAT_TYPE_NONE,
    pkAUDIO_FORMAT_TYPE_AC3,
    pkAUDIO_FORMAT_TYPE_PCM,
    pkAUDIO_FORMAT_TYPE_PCM_DVD,
    pkAUDIO_FORMAT_TYPE_AAC,
    pkAUDIO_FORMAT_TYPE_WMA8,
    pkAUDIO_FORMAT_TYPE_WMA9,
    pkAUDIO_FORMAT_TYPE_WMA9Pro,
    pkAUDIO_FORMAT_TYPE_MPEG,
    pkAUDIO_FORMAT_TYPE_MP3,
    pkAUDIO_FORMAT_TYPE_WMALossless

}  pkAUDIO_FORMAT_TYPE;

typedef enum _pkVBI_FORMAT_TYPE
{
    pkVBI_FORMAT_TYPE_NONE
} pkVBI_FORMAT_TYPE;

typedef enum
{
    pkSYSTEM_FORMAT_TYPE_NONE,
    pkSYSTEM_FORMAT_TYPE_PES,
    pkSYSTEM_FORMAT_TYPE_ES,
    pkSYSTEM_FORMAT_TYPE_ASF,
    pkSYSTEM_FORMAT_TYPE_MPEG2_DVD,
    pkSYSTEM_FORMAT_TYPE_MPEG2_DVD_AUDIO,
    pkSYSTEM_FORMAT_TYPE_MPEG2_PROGRAM,
    pkSYSTEM_FORMAT_TYPE_MPEG2_TRANSPORT
} pkSYSTEM_FORMAT_TYPE;


typedef struct
{
    pkVIDEO_FORMAT_TYPE eVideoType;
    pkAUDIO_FORMAT_TYPE eAudioType;
    pkVBI_FORMAT_TYPE eVbiType;
    pkSYSTEM_FORMAT_TYPE eSystemType;

} pkFORMAT_TYPE;

typedef struct
{
    uint16_t  wFormatTag;
    uint16_t  nChannels;
    uint32_t nSamplesPerSec;
    uint32_t nAvgBytesPerSec;
    uint16_t  nBlockAlign;
    uint16_t  wBitsPerSample;
    uint16_t  cbSize;
    uint8_t  reserved[30];
    bool_t  isSampleRateConversionSupported;

} pkAV_WAVEFORMATEX;

typedef struct
{
    uint32_t  biSize;
    int32_t   biWidth;
    int32_t   biHeight;
    uint16_t   biPlanes;
    uint16_t   biBitCount;
    uint32_t  biCompression;
    uint32_t  biSizeImage;
    int32_t   biXPelsPerMeter;
    int32_t   biYPelsPerMeter;
    uint32_t  biClrUsed;
    uint32_t  biClrImportant;

}  pkAV_BITMAPINFOHEADER;

typedef struct
{
    uint32_t streamID;
    uint32_t avgBitRate;
    uint32_t jitterBufferDuration;
    pkFORMAT_TYPE formatType;
    pkAV_WAVEFORMATEX waveFormat;
    pkAV_BITMAPINFOHEADER bitmapInfo;

}  pkAV_STREAM_DESCRIPTOR;

#define INVALID_START_TIME  -1

