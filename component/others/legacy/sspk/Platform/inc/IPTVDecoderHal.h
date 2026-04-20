///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////


/// <summary>
///     IPTVDecoderHal.h - Spec version 1.25
///        This file contains definitions of A/V decoder APIs that    need to be implemented by
///        the mediaprocessor silicon vendor in order to interface with Microsoft MediaRoom.
/// </summary>

#pragma once

#include "IPTVError.h"
#include "IPTVPhysMemMgr.h"

typedef UINT64 IPTV_HAL_DECODER_TIME;

//Parametric values for WMV decoder
typedef struct
{
    uint32_t nSequenceHeader;
    uint16_t nWidth;
    uint16_t nHeight;
    uint32_t nFourCC;
    uint16_t nAspW;
    uint16_t nAspH;
} IPTV_HAL_DECODER_WMV_HEADER;

//Parametric values for Audio decoder
typedef struct
{
    uint32_t iVersionNumber;
    uint32_t cSubband;
    uint32_t iSamplingRate;
    uint16_t cChannel;
    uint32_t cBytePerSec;
    uint32_t cbPacketLength;
    uint16_t wEncodeOpt;
    uint16_t Reserved1;
    uint16_t cBitsPerSample;
    uint16_t cValidBitsPerSample;
    uint32_t u32ChannelMask;
    uint32_t bDRCDataIsValid;
    uint32_t u32DRCSetting;
    uint32_t u32DRCAverageReference;
    uint32_t u32DRCAverageTarget;
    uint32_t u32DRCPeakReference;
    uint32_t u32DRCPeakTarget;
} IPTV_HAL_DECODER_AUDIO_HEADER;

/// <topic name="DataStructs" displayname="Decoder HAL Data Structures"> </topic>

/// <summary>
/// Audio/Video Codec Types.
/// </summary>
/// <remarks>
/// Each codec type is a 32bit unsigned integer: Video codecs will have bit 16 cleared while audio codecs will have bit 16 set
/// IPTV_HAL_DECODER_CODECTYPE_AUDIO_WMASTD is the standard, non-professional WMA audio codec
/// IPTV_HAL_DECODER_CODECTYPE_AUDIO_WMATS is a special format, non-professional WMA audio codec with a 24 byte header
/// </remarks>
typedef enum _IPTV_HAL_DECODER_CODECTYPE
{
    IPTV_HAL_DECODER_CODECTYPE_VIDEO_MPEG2 = 0,
    IPTV_HAL_DECODER_CODECTYPE_VIDEO_H264,
    IPTV_HAL_DECODER_CODECTYPE_VIDEO_VC1,
    IPTV_HAL_DECODER_CODECTYPE_VIDEO_WMV9,
    IPTV_HAL_DECODER_CODECTYPE_VIDEO_AVS,
    IPTV_HAL_DECODER_CODECTYPE_MAXVIDEO,

    IPTV_HAL_DECODER_CODECTYPE_AUDIO_AC3 = 0x10000,
    IPTV_HAL_DECODER_CODECTYPE_AUDIO_PCM,
    IPTV_HAL_DECODER_CODECTYPE_AUDIO_MPEG,
    IPTV_HAL_DECODER_CODECTYPE_AUDIO_WMASTD,
    IPTV_HAL_DECODER_CODECTYPE_AUDIO_WMATS,
    IPTV_HAL_DECODER_CODECTYPE_AUDIO_AAC,
    IPTV_HAL_DECODER_CODECTYPE_AUDIO_AVS,
    IPTV_HAL_DECODER_CODECTYPE_AUDIO_EAC3,
    IPTV_HAL_DECODER_CODECTYPE_AUDIO_WMAPRO,
    IPTV_HAL_DECODER_CODECTYPE_MAXAUDIO
} IPTV_HAL_DECODER_CODECTYPE;

/// <summary>
/// Video Codec Resolutions
/// </summary>
typedef enum _IPTV_HAL_DECODER_RESOLUTION
{
    IPTV_HAL_DECODER_RESOLUTION_SD = 0,
    IPTV_HAL_DECODER_RESOLUTION_HD,
    IPTV_HAL_DECODER_RESOLUTION_PIP

} IPTV_HAL_DECODER_RESOLUTION;

/// <summary>
/// Audio types
/// Normal indicates the decoder is acquired for the primary audio
/// Description indicates a decoder acquisition for an audio description stream
/// </summary>
typedef enum _IPTV_HAL_DECODER_AUDIOTYPE
{
    IPTV_HAL_DECODER_AUDIOTYPE_NORMAL = 0,
    IPTV_HAL_DECODER_AUDIOTYPE_DESCRIPTION
} IPTV_HAL_DECODER_AUDIOTYPE;

/// <summary>
/// Supported header formats appearing before the AAC access units
/// ADTS transport syntax as defined in ISO 13818-7
/// LATM transport syntax as defined in ISO 14496-3
/// </summary>
typedef enum _IPTV_HAL_DECODER_AACHEADER
{
    IPTV_HAL_DECODER_AACHEADER_ADTS = 0,
    IPTV_HAL_DECODER_AACHEADER_LATM
} IPTV_HAL_DECODER_AACHEADER;

/// <summary>
/// Audio/Video Codec Acquisition Parameters
/// </summary>
/// <remarks>
/// Note that "eResolution" is only applicable to video codecs
/// and "eAudioType" is only applicable to audio codecs
/// </remarks>
typedef struct _IPTV_HAL_DECODER_ACQUIRE
{
    IPTV_HAL_DECODER_CODECTYPE eCodecType;
    IPTV_HAL_DECODER_RESOLUTION eResolution; //only applicable for video codecs
    IPTV_HAL_DECODER_AUDIOTYPE eAudioType; // only applicable for audio codecs
} IPTV_HAL_DECODER_ACQUIRE, *PIPTV_HAL_DECODER_ACQUIRE;

/// <summary>
/// Audio/Video Codec Initialization Parameters
/// </summary>
typedef struct _IPTV_HAL_DECODER_INIT
{
    LPVOID          pClockContext;

    union
    {
          // Audio Decoder initialization parameters
        struct
        {
            IPTV_HAL_DECODER_AACHEADER eAACHeader;
        } audio;
          // Video Decoder initialization Parameters
        //struct
        //{
        //} video;
    };
} IPTV_HAL_DECODER_INIT, *PIPTV_HAL_DECODER_INIT;

/// <summary>
/// Video ES UserData FIFO pointers
/// </summary>
typedef struct _IPTV_HAL_DECODER_VIDEO_USERDATA
{
    PUCHAR           pData[2];
    UINT32           u32Len[2];
} IPTV_HAL_DECODER_VIDEO_USERDATA, *PIPTV_HAL_DECODER_VIDEO_USERDATA;

/// <summary>
/// Audio/Video Decoder Parameters
/// </summary>
/// <remarks>
/// IPTV_HAL_DECODER_VALUETYPE_VIDEO_WMV9, IPTV_HAL_DECODER_VALUETYPE_MACROVISION, IPTV_HAL_DECODER_VALUETYPE_CGMSA and IPTV_HAL_DECODER_VALUETYPE_CCPASSTHROUGH are "Set" only
/// IPTV_HAL_DECODER_VALUETYPE_AUDIO_WMA and IPTV_HAL_DECODER_VALUETYPE_AUDIO_CURRENTPTS are "Set" only
/// IPTV_HAL_DECODER_VALUETYPE_VIDEO_PICTUREINFO, IPTV_HAL_DECODER_VALUETYPE_VIDEO_DIAGS and IPTV_HAL_DECODER_VALUETYPE_VIDEO_CURRENTPTS are "Get" only
/// IPTV_HAL_DECODER_VALUETYPE_AUDIO_DIAGS is "Get" only
/// </remarks>
typedef enum _IPTV_HAL_DECODER_VALUETYPE
{
    // decoder values for video will have bit 16 clear
    IPTV_HAL_DECODER_VALUETYPE_VIDEO_WMV9 = 0,
    IPTV_HAL_DECODER_VALUETYPE_VIDEO_MACROVISION,
    IPTV_HAL_DECODER_VALUETYPE_VIDEO_CGMSA,
    IPTV_HAL_DECODER_VALUETYPE_VIDEO_CCPASSTHROUGH,
    IPTV_HAL_DECODER_VALUETYPE_VIDEO_PICTUREINFO,
    IPTV_HAL_DECODER_VALUETYPE_VIDEO_DIAGS,
    IPTV_HAL_DECODER_VALUETYPE_VIDEO_CURRENTPTS,
    IPTV_HAL_DECODER_VALUETYPE_VIDEO_PICTUREINFO_EX,
    IPTV_HAL_DECODER_VALUETYPE_VIDEO_ORIGPTS,
    IPTV_HAL_DECODER_VALUETYPE_VIDEO_MAXVAL,

    // decoder values for audio will have bit 16 set
    IPTV_HAL_DECODER_VALUETYPE_AUDIO_WMA = 0x10000,
    IPTV_HAL_DECODER_VALUETYPE_AUDIO_DIAGS,
    IPTV_HAL_DECODER_VALUETYPE_AUDIO_CURRENTPTS,
    IPTV_HAL_DECODER_VALUETYPE_AUDIO_KARAOKE,
    // per-decoder volume used for Audio Description
    IPTV_HAL_DECODER_VALUETYPE_AUDIO_VOLUME,
    // panning for Audio Description track
    IPTV_HAL_DECODER_VALUETYPE_AUDIO_PANNING,
    // Audio Output Format Type
    IPTV_HAL_DECODER_VALUETYPE_AUDIO_OUTPUTFORMAT,
    IPTV_HAL_DECODER_VALUETYPE_AUDIO_SPDIFCHANNELSTATUS,
    IPTV_HAL_DECODER_VALUETYPE_AUDIO_LINESTATE,
    IPTV_HAL_DECODER_VALUETYPE_AUDIO_DOLBYTB11,
    IPTV_HAL_DECODER_VALUETYPE_AUDIO_MAXVAL,

    // decoder values for DRM decrypter
    IPTV_HAL_DECODER_VALUETYPE_DRM_SETHANDLE = 0x20000, // 32-bit handle
    IPTV_HAL_DECODER_VALUETYPE_DRM_SETKEYID,
    IPTV_HAL_DECODER_VALUETYPE_DRM_SETSAMPLEID, // 64-bit IV
    IPTV_HAL_DECODER_VALUETYPE_DRM_SETOPL,
    IPTV_HAL_DECODER_VALUETYPE_DRM_MAXVAL,

    // values for streams
    IPTV_HAL_DECODER_VALUETYPE_STREAM_SET_EOS = 0x30000,
    IPTV_HAL_DECODER_VALUETYPE_STREAM_GET_RENDERDONE,
    IPTV_HAL_DECODER_VALUETYPE_STREAM_GET_DECODERSTALL,
    IPTV_HAL_DECODER_VALUETYPE_STREAM_SET_PLAYRATE,
    IPTV_HAL_DECODER_VALUETYPE_STREAM_MAXVAL,

    IPTV_HAL_DECODER_VALUETYPE_LASTVAL
} IPTV_HAL_DECODER_VALUETYPE;

/// <summary>
/// Parametric values for IPTV_HAL_DECODER_VALUETYPE_VIDEO_WMV9
/// </summary>
typedef struct _IPTV_HAL_DECODER_VALUE_VIDEO_WMV9
{
    UINT32 nSequenceHeader;
    UINT16 nWidth;
    UINT16 nHeight;
    UINT32 nFourCC;
    UINT16 nAspW;
    UINT16 nAspH;
} IPTV_HAL_DECODER_VALUE_VIDEO_WMV9, *PIPTV_HAL_DECODER_VALUE_VIDEO_WMV9;

/// <summary>
/// Parametric values for IPTV_HAL_DECODER_VALUETYPE_VIDEO_CCPASSTHROUGH
/// </summary>
typedef struct _IPTV_HAL_DECODER_VALUE_VIDEO_CCPASSTHROUGH
{
    bool   bEnableCCPassthrough;
} IPTV_HAL_DECODER_VALUE_VIDEO_CCPASSTHROUGH, *PIPTV_HAL_DECODER_VALUETYPE_VIDEO_CCPASSTHROUGH;

/// <summary>
/// Parametric values for IPTV_HAL_DECODER_VALUETYPE_VIDEO_PICTUREINFO
/// </summary>
typedef struct _IPTV_HAL_DECODER_VALUE_VIDEO_PICTUREINFO
{
    UINT16           u16Width;
    UINT16           u16Height;
    UINT16           u16Aspx;
    UINT16           u16Aspy;
} IPTV_HAL_DECODER_VALUE_VIDEO_PICTUREINFO, *PIPTV_HAL_DECODER_VALUE_VIDEO_PICTUREINFO;

/// <summary>
/// Parametric values for IPTV_HAL_DECODER_VALUETYPE_VIDEO_PICTUREINFO_EX
/// </summary>
typedef struct _IPTV_HAL_DECODER_VALUE_VIDEO_PICTUREINFO_EX
{
    UINT16           u16Width;
    UINT16           u16Height;
    UINT16           u16Aspx;
    UINT16           u16Aspy;
    IPTV_HAL_DECODER_TIME hdtDTS;
    IPTV_HAL_DECODER_TIME hdtDuration;
    UINT16           u16QualityLevel;
    UINT32           u32Flags;
} IPTV_HAL_DECODER_VALUE_VIDEO_PICTUREINFO_EX, *PIPTV_HAL_DECODER_VALUE_VIDEO_PICTUREINFO_EX;

#define IPTV_HAL_DECODER_VALUE_VIDEO_PICTUREINFO_EX_FLAGS_RAP   0x1
#define IPTV_HAL_DECODER_VALUE_VIDEO_PICTUREINFO_EX_FLAGS_SYNC  0x2

/// <summary>
/// Parametric values for IPTV_HAL_DECODER_VALUETYPE_VIDEO_DIAGS
/// </summary>
typedef struct _IPTV_HAL_DECODER_VALUE_VIDEO_DIAGS
{
    UINT32           u32FramesDecoded;
    UINT32           u32FramesDropped;
    UINT32           u32FramesErrors;
    UINT32           u32FramesUnderruns;
    UINT32           u32FramesOverflows;
    UINT32           u32FifoSize;
    UINT32           u32FifoReadPtr;
    UINT32           u32FifoWritePtr;
    CHAR             szDecoderResets[64];
} IPTV_HAL_DECODER_VALUE_VIDEO_DIAGS, *PIPTV_HAL_DECODER_VALUE_VIDEO_DIAGS;

/// <summary>
/// Parametric values for IPTV_HAL_DECODER_VALUETYPE_VIDEO_CURRENTPTS
/// </summary>
typedef struct _IPTV_HAL_DECODER_VALUE_VIDEO_CURRENTPTS
{
    IPTV_HAL_DECODER_TIME CurrentPTS;
} IPTV_HAL_DECODER_VALUE_VIDEO_CURRENTPTS, *PIPTV_HAL_DECODER_VALUE_VIDEO_CURRENTPTS;

/// <summary>
/// Parametric values for IPTV_HAL_DECODER_VALUETYPE_AUDIO_WMA
/// </summary>
typedef struct _IPTV_HAL_DECODER_VALUE_AUDIO_WMA9
{
    UINT32 iVersionNumber;
    UINT32 cSubband;
    UINT32 iSamplingRate;
    UINT16 cChannel;
    UINT32 cBytePerSec;
    UINT32 cbPacketLength;
    UINT16 wEncodeOpt;
    UINT16 Reserved1;
    UINT16 cBitsPerSample;
    UINT16 cValidBitsPerSample;
    UINT32 u32ChannelMask;
    UINT32 bDRCDataIsValid;
    UINT32 u32DRCSetting;
    UINT32 u32DRCAverageReference;
    UINT32 u32DRCAverageTarget;
    UINT32 u32DRCPeakReference;
    UINT32 u32DRCPeakTarget;
} IPTV_HAL_DECODER_VALUE_AUDIO_WMA9, *PIPTV_HAL_DECODER_VALUE_AUDIO_WMA9;

/// <summary>
/// Parametric values for IPTV_HAL_DECODER_VALUETYPE_AUDIO_DIAGS
/// </summary>
typedef struct _IPTV_HAL_DECODER_VALUE_AUDIO_DIAGS
{
    UINT32           u32FramesDecoded;
    UINT32           u32FramesDropped;
    UINT32           u32FramesErrors;
    UINT32           u32SamplesUnderruns;
    UINT32           u32FramesOverflows;
    UINT32           u32FifoSize;
    UINT32           u32FifoReadPtr;
    UINT32           u32FifoWritePtr;
    CHAR             szDecoderResets[64];
} IPTV_HAL_DECODER_VALUE_AUDIO_DIAGS, *PIPTV_HAL_DECODER_VALUE_AUDIO_DIAGS;

/// <summary>
/// Parametric values for IPTV_HAL_DECODER_VALUETYPE_AUDIO_KARAOKE
/// </summary>
typedef struct _IPTV_HAL_DECODER_VALUE_AUDIO_KARAOKE
{
    BOOL bEnable;
} IPTV_HAL_DECODER_VALUE_AUDIO_KARAOKE, *PIPTV_HAL_DECODER_VALUE_AUDIO_KARAOKE;

/// <summary>
/// Parametric values for IPTV_HAL_DECODER_VALUETYPE_AUDIO_CURRENTPTS
/// </summary>
typedef struct _IPTV_HAL_DECODER_VALUE_AUDIO_CURRENTPTS
{
    IPTV_HAL_DECODER_TIME CurrentPTS;
} IPTV_HAL_DECODER_VALUE_AUDIO_CURRENTPTS, *PIPTV_HAL_DECODER_VALUE_AUDIO_CURRENTPTS;

/// <summary>
/// Parametric values for IPTV_HAL_DECODER_VALUETYPE_AUDIO_OUTPUTFORMAT
/// </summary>
typedef enum _IPTV_HAL_DECODER_AUDIOOUTPUTTYPE
{
    IPTV_HAL_DECODER_AUDIOOUTPUTTYPE_STEREO,
    IPTV_HAL_DECODER_AUDIOOUTPUTTYPE_MULTICHANNELPCM,
    IPTV_HAL_DECODER_AUDIOOUTPUTTYPE_COMPRESSED
} IPTV_HAL_DECODER_AUDIOOUTPUTTYPE;

typedef struct _IPTV_HAL_DECODER_VALUE_AUDIO_OUTPUTFORMAT
{
    IPTV_HAL_DECODER_AUDIOOUTPUTTYPE OutputType;
} IPTV_HAL_DECODER_VALUE_AUDIO_OUTPUTFORMAT, *PIPTV_HAL_DECODER_VALUE_AUDIO_OUTPUTFORMAT;

/// <summary>
/// Parametric values for IPTV_HAL_DECODER_AUDIOOUTPUT_LINE
/// This enumeration (bit field) represents the audio output lines from the SoC
/// </summary>
typedef enum _IPTV_HAL_DECODER_AUDIOOUTPUT_LINE
{
    IPTV_HAL_DECODER_AUDIOOUTPUT_LINE_ANALOG = 0x1,
    IPTV_HAL_DECODER_AUDIOOUTPUT_LINE_SPDIF = 0x2,
    IPTV_HAL_DECODER_AUDIOOUTPUT_LINE_HDMI = 0x4,
    IPTV_HAL_DECODER_AUDIOOUTPUT_LINE_ALL =
    (IPTV_HAL_DECODER_AUDIOOUTPUT_LINE_ANALOG |
     IPTV_HAL_DECODER_AUDIOOUTPUT_LINE_SPDIF |
     IPTV_HAL_DECODER_AUDIOOUTPUT_LINE_HDMI)
} IPTV_HAL_DECODER_AUDIOOUTPUT_LINE;

/// <summary>
/// Parametric values for IPTV_HAL_DECODER_VALUE_AUDIO_LINESTATE
///
/// u32UnMuteMask is a bitmask representing the mute state for the audio output connectors
/// defined by IPTV_HAL_DECODER_AUDIOOUTPUT_LINE
///
/// Bit set to "1" represents that the audio connector should be UnMuted
/// Bit set to "0" represents that the audio connector should be Muted
/// </summary>
typedef struct _IPTV_HAL_DECODER_VALUE_AUDIO_LINESTATE
{
    UINT32 u32UnMuteMask;
} IPTV_HAL_DECODER_VALUE_AUDIO_LINESTATE;

/// <summary>
/// Parametric values for IPTV_HAL_DECODER_VALUETYPE_AUDIO_DOLBYTB11
/// If bDisable is true, Dolby Technical bulletin 11 should be disabled
/// Default: Dolby TB11 should be enabled.
/// </summary>
typedef struct _IPTV_HAL_DECODER_VALUE_AUDIO_DOLBYTB11
{
    bool bDisable;
} IPTV_HAL_DECODER_VALUE_AUDIO_DOLBYTB11, *PIPTV_HAL_DECODER_VALUE_AUDIO_DOLBYTB11;

/// <summary>
/// Parametric values for IPTV_HAL_DECODER_VALUETYPE_DRM_SETOPL
/// </summary>
typedef enum IPTV_HAL_DECODER_DRM_SETOPL_TYPE
{
    IPTV_HAL_DECODER_DRM_SETOPL_TYPE_HDCP_DISABLE,
    IPTV_HAL_DECODER_DRM_SETOPL_TYPE_HDCP_ENABLE_ALWAYS,
    IPTV_HAL_DECODER_DRM_SETOPL_TYPE_HDCP_ENABLE_DOWN_RES,
    IPTV_HAL_DECODER_DRM_SETOPL_TYPE_COMPONENT_DISABLE,
    IPTV_HAL_DECODER_DRM_SETOPL_TYPE_COMPONENT_ENABLE
} IPTV_HAL_DECODER_DRM_SETOPL_TYPE;

typedef struct _IPTV_HAL_DECODER_DRM_SETOPL
{
    UINT32   pipeIdN; // decoder pipe id
    IPTV_HAL_DECODER_DRM_SETOPL_TYPE  oplType;
} IPTV_HAL_DECODER_DRM_SETOPL, *PIPTV_HAL_DECODER_DRM_SETOPL;


typedef iptv_hal_error (*LPFN_DECRYPTCB)(PIPTV_HAL_BUFFER pInBufList, PIPTV_HAL_BUFFER pOutBufList,
                                         PVOID pDecryptContext);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <topic name="Decoder" displayname="Audio/Video Decoder Control APIs"> </topic>

/// <summary>
/// This function is called once when the IPTV application starts up on the box.
/// </summary>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Hal Initialize failed</para>
/// </returns>
/// <remarks>
/// The implementation is expected to initialize the decoder subsystem and get the system into a ready state such that subsequent DecoderHAL APIs can be called.
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_HALInit();

/// <summary>
/// This function is called once when the IPTV application is about to exit
/// </summary>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Hal Exit failed</para>
/// </returns>
/// <remarks>
/// The implementation is expected to turn off the decoders and release all software/hardware resources associated with the decoder subsystem.
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_HALExit();

/// <summary>
/// This function acquires an available decoder and returns a decoder context for a given codec type if the codec type is supported and a decoder is available
/// </summary>
/// <param name="pAcqParams">[IN] Contains codec type and resoution (if a video codec is requested) </param>
/// <param name="ppDecoderContext">[OUT] Pointer to receive decoder context </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: No decoders available for eCodecType</para>
/// </returns>
/// <remarks>
/// The implementation is expected to arbitrate among the available decoders considering the memory and decoding capability of the decoding engine (DSP).
/// For example if the implementation can support 1 HD and 1 PIP or 1SD and 6 PIPs, this decision logic has to be in the acquire implementation.
/// </remarks>
/// <see cref="PIPTV_HAL_DECODER_ACQUIRE"/>
iptv_hal_error IPTV_HAL_Decoder_Acquire(PIPTV_HAL_DECODER_ACQUIRE pAcqParams, LPVOID *ppDecoderContext);

/// <summary>
/// This function initializes the decoder, synchronized with the clock context passed in, and sets up the fifo and reference buffers.
/// </summary>
/// <param name="pDecoderContext">[IN] Pointer to the decoder context </param>
/// <param name="pInitParams">[IN] Pointer to a IPTV_HAL_DECODER_INIT structure that contains an STC context to synchronize with  </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: The decoder was not successfully initialized</para>
/// </returns>
iptv_hal_error IPTV_HAL_Decoder_Init(LPVOID pDecoderContext, PIPTV_HAL_DECODER_INIT pInitParams);

/// <summary>
/// This function flushes the decoder.
/// </summary>
/// <param name="pDecoderContext">[IN] Pointer to the decoder context </param>
/// <param name="bClearPicture">[IN] If true, picture buffers are also cleared for the decoder and a black screen is expected on the display. Not applicable for an audio decodercontext </param>
/// <param name="bClearDecoderStall">[IN] If true, this is a notification that an attempt to write to the decoder has failed and decoder recovery should be attempted</param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: The decoder was not successfully flushed</para>
/// </returns>
/// <remarks>
/// The Decoder implementation should clear the reference frames and the input FIFO.
/// The FlushDecoder API is used during a channel change that does not involve changing codecs.
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_Flush(LPVOID pDecoderContext, bool bClearPicture, bool bClearDecoderStall);

/// <summary>
/// This function is used by the application to release all resources associated with this decoder and put the decoder back to the available state.
/// </summary>
/// <param name="pDecoderContext">[IN] Pointer to the decoder context </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: The decoder was not successfully released.</para>
/// </returns>
/// <remarks>
/// The decoder release method is used during a channel change that also involves a codec change.
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_ReleaseDecoder(LPVOID pDecoderContext);

/// <summary>
/// This API allows the application to gain direct access to the decoder's FIFO.
/// </summary>
/// <param name="pDecoderContext">[IN] Pointer to the decoder context </param>
/// <param name="pInBufList">[IN] Pointer to A/V data for which space is needed in the decoder FIFO (the data may be encrypted ES)</param>
/// <param name="bPTSValid">[IN] If true, the PTSTime90khz is valid</param>
/// <param name="PTSTime90khz">[IN] A presentation timestamp value that represents the PTS in 90KHz units corresponding to this ES data</param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: The decoder rejected this sample</para>
/// <para>IPTV_HAL_ERROR_NOT_ENOUGH_MEMORY: There was not enough room in the FIFO. The application should wait for a period of time to elapse for the decoder FIFO to drain and retry the operation</para>
/// </returns>
/// <remarks>
/// The crypto block will decrypt the incoming encrypted ES data from the AV Subsystem into the decoder FIFO directly.
/// Ideally the FIFO for a decoder should be in the secure memory area accessible to the video processor but not to the CPU.
/// IPTV_HAL_Decoder_PrepareForDecode reserves space in the decoder FIFO for the PES header with any padding bytes (in the case where the decoder consumes only a PES stream) and the ES.
/// It also records various pieces of information in the decoder context such that they need not be supplied again in subsequent functions.
/// The PrepareForDecode, Decrypt (if implemented), Decode sequence will never be interleaved with another similar sequence for the same decodercontext.
/// If possible a single buffer will be allocated, but if the current write point in the FIFO is near the end, then two buffers will need to be allocated.
/// This function will also allocate enough space for the insertion of PES header and padding bytes for alignment purposes (if required as in the case of decoders consuming PES streams)
/// ahead of the ES buffer(s) being returned. This parameter is provided solely for the benefit of test applications that don't use the decrypt function that follows and yet need to
/// know where the payload should be placed.
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_PrepareForDecode(LPVOID pDecoderContext, PIPTV_HAL_BUFFER pInBufList, bool bPTSValid, IPTV_HAL_DECODER_TIME PTSTime90khz);

/// <summary>
/// IPTV_HAL_Decoder_Decrypt calls the cryptocore's DecryptAVPayload function (see below) on behalf of the AV system.
/// </summary>
/// <param name="pDecoderContext">[IN] Pointer to the decoder context </param>
/// <param name="pfnDecryptCallback">[IN] The pfnDecryptCallback points to the function that is to be called when the buffers have been prepared. The caller can supply different callbacks to distinguish the case where decryption is required from the passthrough case where it isn't</param>
/// <param name="pDecryptContext">[IN] The pDecryptContext points to the data required by the callback function </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: The decrypt operation failed</para>
/// </returns>
/// <remarks>
/// IPTV_HAL_Decoder_Decrypt prepares suitable input and output buffer structures and then calls the supplied decryption callback, passing the buffers
/// and the supplied decryption context pointer. The callback will do the actual decryption, typically employing the crypto HAL for that purpose.
/// The function IPTV_HAL_Decoder_PrepareToDecode, referring to the same decoder context, must have been called previously with no
/// intervening call of IPTV_HAL_Decoder_Decode. Typically, each input buffer supplied to the decryption callback will either be an input
/// buffer placed in the decoder context by the prior call of IPTV_HAL_Decoder_PrepareToDecode, or an additional buffer
/// containing headers that may be required by the decoder on that platform. Any buffer of the latter type must have its flags
/// field set to IPTV_HAL_CRYPTO_BUFFER_FLAG_PASSTHRU so that decryption is not applied to such headers
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_Decrypt(LPVOID pDecoderContext, LPFN_DECRYPTCB pfnDecryptCallback, PVOID pDecryptContext);

/// <summary>
/// The IPTV_HAL_Decoder_Decode function makes data that was decrypted using a prior call of IPTV_HAL_Decoder_Decrypt available to the decoder.
/// </summary>
/// <param name="pDecoderContext">[IN] Pointer to the decoder context </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: The decoder did not successfully set up the decode operation</para>
/// </returns>
/// <remarks>
/// Failure (a catastrophic case) can only happen if the fifo area is trashed between the PrepareForDecode and Decode operation.
/// The application should not attempt to retry the sample. The expectation here is an internal monitor thread in the implementation will detect this condition and recover the
/// decoder out of this state.
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_Decode(LPVOID pDecoderContext);

/// <summary>
/// This function is used by the application to "get" a specified Value from the decoder.
/// </summary>
/// <param name="pDecoderContext">[IN] Pointer to the decoder context </param>
/// <param name="eValueType">[IN] Indicates the decoder value to "Get"</param>
/// <param name="pValueData">[OUT] Pointer to receive the value of the desired decoder parameter</param>
/// <param name="pValueDataLength">[OUT] Pointer to receive the length of pValueData</param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Invalid parameter</para>
/// </returns>
iptv_hal_error IPTV_HAL_Decoder_GetValue(LPVOID pDecoderContext, IPTV_HAL_DECODER_VALUETYPE eValueType, LPVOID pValueData, UINT32* pValueDataLength);

/// <summary>
/// This function is used by the application to "set" a specified Value from the decoder.
/// </summary>
/// <param name="pDecoderContext">[IN] Pointer to the decoder context </param>
/// <param name="eValueType">[IN] Indicates the decoder value to "Set"</param>
/// <param name="pValueData">[IN] Pointer to the decoder parameter data</param>
/// <param name="nValueDataLength">[IN] Length of pValueData</param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Invalid parameter</para>
/// </returns>
iptv_hal_error IPTV_HAL_Decoder_SetValue(LPVOID pDecoderContext, IPTV_HAL_DECODER_VALUETYPE eValueType, const LPVOID pValueData, UINT32 nValueDataLength);

/// <summary>
/// This function is used by the application to set the decoder to I-Frame-Only mode.
/// </summary>
/// <param name="pDecoderContext">[IN] Pointer to the decoder context </param>
/// <param name="bIFrameOnly">[IN] If true, decoder is set to decode only I-Frames. If false, the decoder is set to decode all frames</param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Unable to set/reset the decoder into/from I-Frame mode</para>
/// </returns>
/// <remarks>
/// This API may not be necessary for decoders that do not require I-Frame-Only as a separate mode.
/// For field encoded streams, only the top field or the bottom field will be provided to the decoder.
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_SetPlayMode (LPVOID pDecoderContext, bool bIFrameOnly);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <topic name="STC" displayname="System Time Clock (STC) Control APIs"> </topic>

/// <summary>
/// This function is used by the application to acquire an available clock.
/// </summary>
/// <param name="bAVSync">[IN] If true, the clock will be used to synchronize audio and video</param>
/// <param name="ppClockContext">[OUT] Pointer to receive the acquired HAL clock context </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: No available clock contexts</para>
/// </returns>
iptv_hal_error IPTV_HAL_Decoder_Clock_Acquire(bool bAVSync, LPVOID *ppClockContext);

/// <summary>
/// This function is used by the application to initialize an STC.
/// </summary>
/// <param name="pClockContext">[IN] Pointer to the system clock context </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Invalid system clock context</para>
/// </returns>
/// <remarks>
/// The decoder at this point should complete all initialization necessary and have the
/// clock driver APIs ready to start.
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_Clock_Init(LPVOID pClockContext);

/// <summary>
/// This function is used by the application to set the clock start time
/// </summary>
/// <param name="pClockContext">[IN] Pointer to the system clock context </param>
/// <param name="u64Time90khz">[IN] contains the time in 90KHz units to which the STC is being set </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Invalid system clock context</para>
/// </returns>
/// <remarks>
/// Note that the clock should not be started at the time of calling this API.
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_Clock_SetTime(LPVOID pClockContext, IPTV_HAL_DECODER_TIME u64Time90khz);

/// <summary>
/// This function is used by the application to get the current time from the clock
/// </summary>
/// <param name="pClockContext">[IN] Pointer to the system clock context </param>
/// <param name="pu64Time90khz">[OUT] Contains the time in 90KHz units obtained from the STC </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Invalid system clock context</para>
/// </returns>
/// <remarks>
/// The STC value returned by the decoder is used by the Microsoft application to validate the
/// clock prior to starting. It is also used to validate PTS of data pushed into the decoder.
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_Clock_GetTime(LPVOID pClockContext, IPTV_HAL_DECODER_TIME *pu64Time90khz);

/// <summary>
/// This function is used by the application to start the clock
/// </summary>
/// <param name="pClockContext">[IN] Pointer to the system clock context </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Invalid system clock context</para>
/// </returns>
/// <remarks>
/// In the Microsoft IPTV application, video and audio should be independently synchronized with the hardware STC.
/// The decoder must start the decoder immediately as soon as this Clock_Start is invoked
/// Instant Channel Change (ICC) is a key feature of the Microsoft IPTV application - hence the clock
/// start must be implemented strictly as per the requirements specified here
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_Clock_Start(LPVOID pClockContext);

/// <summary>
/// This function is used by the application to stop the clock
/// </summary>
/// <param name="pClockContext">[IN] Pointer to the system clock context </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Invalid system clock context</para>
/// </returns>
iptv_hal_error IPTV_HAL_Decoder_Clock_Stop(LPVOID pClockContext);

/// <summary>
/// This function is used by the application to release a previously acquired system clock context
/// </summary>
/// <param name="pClockContext">[IN] Pointer to the system clock context that is to be released</param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Invalid system clock context</para>
/// </returns>
iptv_hal_error IPTV_HAL_Decoder_Clock_Release(LPVOID pClockContext);



iptv_hal_error IPTV_HAL_Decoder_SetSeekFlag(bool flag);
bool IPTV_HAL_Decoder_GetSeekFlag();


///.End

//.End
