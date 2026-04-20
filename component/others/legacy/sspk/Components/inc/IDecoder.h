///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////


#pragma once

#include "IXDrm.h"
#include <string>

// ===============================================================================================================
// Decoder IO Control
// ===============================================================================================================

enum DECODER_CONTROL
{
    //Video decoder related controls
    DECODER_SETWMVHEADER,         //in=DECODER_WMV_HEADER
    DECODER_HASRENDEREDFIRSTFRAME,//in=bool*
    DECODER_CCREGISTER_TRIGGER,   //in=uint, register 1, unregister 0
    DECODER_GETPICTUREINFO,       //out=DECODER_VIDEO_PICTUREINFO
    DECODER_GETVIDEODIAGS,        //out=DECODER_VIDEO_DIAGS

    //Audio decoder related controls
    DECODER_SETAUDIOHEADER,       //in=DECODER_AUDIO_HEADER
    DECODER_GETAUDIODIAGS,        //out=DECODER_AUDIO_DIAGS

    //Subtitle decoder related control
    DECODER_GETDVBS_INFO,         //out=struct DECODERDVBS_INFO, DVB subtitle page, language & type
    DECODER_SETDVBS_INFO,         //in=struct DECODERDVBS_INFO

    //Closed captioning decoder related controls
    DECODER_CCTPYE,               //in=DECODERCC_CHANNELS

    //teletext decoder related controls
    DECODER_KEYEVENT,             //in=uint
    DECODER_RESOLUTION_CHANGES,   //notify when screen resolution changes (i=uint 1 when 16to9)
    DECODER_GETTELETEXTPAGE,      //in=size of buffer out=teletext page copied

    //Currently supported controls end here
    DECODER_NUMCONTROLS
};

//Parametric values for HAL video picture info
//
//IMPORTANT: Do not touch the order or add additional fields here
typedef struct
{
    uint16 u16Width;
    uint16 u16Height;
    uint16 u16Aspx;
    uint16 u16Aspy;
} DECODER_VIDEO_PICTUREINFO;

//Parametric values for HAL video diagnostics
//
//IMPORTANT: Do not touch the order or add additional fields here
typedef struct
{
    uint32 u32FramesDecoded;
    uint32 u32FramesDropped;
    uint32 u32FramesErrors;
    uint32 u32FramesUnderruns;
    uint32 u32FramesOverflows;
    uint32 u32FifoSize;
    uint32 u32FifoReadPtr;
    uint32 u32FifoWritePtr;
    char   szDecoderResets[64];
} DECODER_VIDEO_DIAGS;

//Parametric values for HAl audio diagnostics
//
//IMPORTANT: Do not touch the order or add additional fields here
typedef struct
{
    uint32 u32FramesDecoded;
    uint32 u32FramesDropped;
    uint32 u32FramesErrors;
    uint32 u32SamplesUnderruns;
    uint32 u32FramesOverflows;
    uint32 u32FifoSize;
    uint32 u32FifoReadPtr;
    uint32 u32FifoWritePtr;
    char   szDecoderResets[64];
} DECODER_AUDIO_DIAGS;

typedef struct
{
    LPVOID pFrame;
    bool   bBitmapChanged;
    bool   bBlockChanged;
    bool   bBlocked;
} GETNEXTPIPFRAME_INPUT;

typedef struct
{
    int    n608Channel;
    int    n708Channel;
} DECODERCC_CHANNELS;

typedef struct
{
    int    page;
    int    language;
    int    type;
} DECODERDVBS_INFO;

enum DECODER_ERR
{
    DECODER_ERR_SUCCESS = 0,
    DECODER_ERR_NOT_IMPLEMENTED,
    DECODER_ERR_FAILED,
    DECODER_ERR_BAD_PARAMETER,
    DECODER_ERR_NOT_FOUND,
};

// ===============================================================================================================
// Mechanism to listen to decoder events
// Used for forwarding ECM, VPS etc
// ===============================================================================================================

enum DecoderEvent
{
    DecoderEvent_Unknown,
    DecoderEvent_ECM,
    DecoderEvent_VPS,
};

class IDecoderEventSink
{
public:
    virtual ~IDecoderEventSink() {}
    virtual void OnDecoderEvent(DecoderEvent eventType, uint32 ref, const std::string& eventData) = 0;
};

// ===============================================================================================================
// Base Decoder Class
// ===============================================================================================================

class Buffer;

class IDecoder
{
public:
    //Destructor
    virtual ~IDecoder() {}

    //Acquire a decoder
    virtual bool           Acquire(void) = 0;
    //Release the decoder
    virtual void           Release(void) = 0;
    //Whether a decoder has been acquired
    virtual long           IsAcquired(void) = 0;

    //Decoder control
    //inBuf/inSize contain optional parameters to the control
    //outBuf/outSize points to memory for optional returned data - if outSize is too small for the control, the call fails.
    virtual DECODER_ERR    IoControl(DECODER_CONTROL control, void* inBuf=NULL, int inSize=0, void* outBuf=NULL, int outSize=0) = 0;

    //Check decoder stall
    virtual bool           CheckStall(void) = 0;

    //Sets the current DRM decryption mode
    virtual void           SetDRMHandle(uint32 drmHandle) = 0;
    //Sets the current DRM key id into the security core
    virtual void           SetKeyId(__in_bcount(signedKeyIdLength) const byte* signedKeyId, uint32 signedKeyIdLength) = 0;
    //Sets the current DRM sample id into the security core
    virtual void           SetSampleId(uint64 sampleId) = 0;

    //Check for parental control on this decoder
    virtual int            CheckAccessControl(uint64 pts = 0) = 0;

    //Return current pts being rendererd on screen
    virtual bool           GetCurrentPTS(uint64* pPTS) = 0;

    //Set the expiry time in decoder based on it's sample queue
    virtual bool           SetExpirationTime(bool ondestruct, uint64 pts) = 0;

    //Return the fade parameter parsed out of the AD descriptor for this stream
    virtual uint8          GetAudioDescriptionFade(void) = 0;

    //Initialization API
    virtual void           OnSync(bool sync, bool teardownPicture, bool cleanStall) = 0;

    //Handle a RAP/SyncPoint
    virtual void           OnRap(void) = 0;
    virtual void           OnSyncPoint(void) = 0;
    //PTS DTS
    virtual void           SetPTS(uint64 pts) = 0;
    virtual uint64         GetPTS(void) const = 0;
    virtual void           SetDTS(uint64 dts) = 0;
    virtual uint64         GetDTS(void) const = 0;
    //Set duration of upcoming sample
    virtual void           SetDuration(uint64 duration) = 0;
    //Set video frame size
    virtual void           SetFrameSize(uint32 width, uint32 height) = 0;
    //Set the fade and pan for audio descriptor
    virtual void           SetFadeAndPan(uint8 fade, int pan) = 0;
    //Set quality level index
    virtual void           SetQualityLevel(uint16 qualityLevel) = 0;
    //Write the given buffer to the decoder
    virtual Buffer*        Write(Buffer* buffer) = 0;

    //Checks whether the decoder has finished rendering its stream
    virtual bool           IsRenderingDone(uint64 stc) = 0;

    //Send a event from a decoder to the outside world
    //A bit of a layer violation but better than the organically emerging add hoc solutions
    virtual void           SetEventSink(IDecoderEventSink* eventSink) = 0;

    //Tell the decoders that no more frames will be sent
    virtual void           SetEndOfStream() = 0;

    //Returns the last sample's timestamp sent to the decoder
    virtual uint64         GetLastPtsSent() const = 0;
};

// ===============================================================================================================
// Exception that can be thown by decoders
// ===============================================================================================================

class DecoderException: public std::exception
{
public:
    DecoderException(const char *const& what)
        : szWhat(what) {}

    virtual const char* what ( ) const throw () { return szWhat; }

protected:
    const char* szWhat;

protected:
    DecoderException(); // inhibit default construction
};

// ===============================================================================================================
// IDecoderFactory - manages instantiation/destruction and other interactions compositor
// ===============================================================================================================

class IReceiverControl;
class CStreamInfo;

class IDecoderFactory
{
public:
    /// <summary>
    /// Destructor
    /// </summary>
    virtual ~IDecoderFactory() {}
    /// <summary>
    /// Allocate a decoder based on the given elementary stream information
    /// </summary>
    virtual IDecoder*      AcquireDecoder(IReceiverControl* receiverControl, const CStreamInfo& si) = 0;
    /// <summary>
    /// Dispose off the given decoder
    /// </summary>
    virtual void           DisposeDecoder(IDecoder* decoder) = 0;
    /// <summary>
    /// Save given decoder into decoder map
    /// </summary>
    virtual void           RegisterDecoder(uint32 pipeIdN, IDecoder* decoder, LPVOID pDecoderContext = NULL) = 0;
    /// <summary>
    /// Remove given decoder from decoder map
    /// </summary>
    virtual void           UnRegisterDecoder(uint32 pipeIdN, const IDecoder* decoder, LPVOID pDecoderContext = NULL) = 0;
    /// <summary>
    /// Direct communication with decoders
    /// </summary>
    virtual DECODER_ERR    IoControl(uint32 pipeIdN, DECODER_CONTROL control, void* inBuf=NULL, int inSize=0, void** outBuf=NULL, int outSize=0) = 0;
    /// <summary>
    /// Notifies the compositor of fullscreen block state at video decoder instantiation time
    /// </summary>
    virtual void           NotifyFullscreenBlockStatus(bool bBlock) = 0;
    /// <summary>
    /// Enable HDCP (this is required by playready DRM)
    /// </summary>
    virtual void           HandleHDCPOutput(uint32 pipeIdN, XDRM_OPL_ACTION action) = 0;
    /// <summary>
    /// Disable component video (this is required by playready DRM)
    /// </summary>
    virtual void           HandleComponentVideoOutput(uint32 pipeIdN, bool disable) = 0;
};

// ===============================================================================================================
// ===============================================================================================================
