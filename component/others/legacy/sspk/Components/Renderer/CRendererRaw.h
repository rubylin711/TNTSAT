///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

/// <summary>
///     CRendererRaw.h
///     Used for WMS playback - handles audio and video decoders directly
//        and renderes raw audio and video elementary stream data fed
//      directly from the sockets
/// </summary>

#pragma once

#include "CRendererBase.h"
#include "CRawPacket.h"

// ===============================================================================================================
// ===============================================================================================================

/// <summary>
/// Handles state conditions while converting CRawPacket objects into buffers to pass on to the decoder
/// </summary>

class CRawFrame
{
public:
    CRawFrame();
    virtual ~CRawFrame();

    void Init(IBufferPool* bufferPool, StreamType streamType);
    virtual void Reset();

    bool WriteBuffers(CRawPacket& rawPacket);
    void ReleaseBuffers() { _chain.Release(); }
    Buffer* GetBuffers() { return( _chain.HeadBuffer() ); }
    void SetBuffers( _In_ Buffer* pFirstBuffer ) { _chain.SetBuffers( pFirstBuffer ); }

    bool HasBuffers() { return _chain.GetChainFirst() != NULL; }

protected:
    void ResetFrameCounters();
    bool Write(const byte* data, int32 len, bool encrypted = false);

    virtual void StoreFrameState();
    virtual void RestoreFrameState();

    virtual int32 GetNextDataLength(CRawPacket& rawPacket, int32& offset, bool& encrypted);
    virtual bool InsertHeaders(CRawPacket& rawPacket, int32& offset, bool bFrameStart) { return true; }

    struct SubSampleState
    {
        int32        Entry;
        int32        Len;
        bool        Encrypted;
        bool        NewBlock;
    };

    IBufferPool*    _bufferPool;
    StreamType        _streamType;

    SubSampleState    _subSampleState;
    SubSampleState    _lastSubSampleState;

    BufferChain     _chain;
};

class CRawFrameVideo : public CRawFrame
{
public:
    CRawFrameVideo();

    __override void Reset();

protected:
    __override void StoreFrameState();
    __override void RestoreFrameState();

    __override int32 GetNextDataLength(CRawPacket& rawPacket, int32& offset, bool& encrypted);
    __override bool InsertHeaders(CRawPacket& rawPacket, int32& offset, bool bFrameStart);

private:
    //Clear Nal carryover fields
    void ResetNal();
    //Parse Nal length and handle possible carryover from one packet to the next
    bool ParseNalLen(CRawPacket& rawPacket, int32& offset, int32& len);

    struct NalState
    {
        int32    BaseLen;
        uint32    Len;
        bool    Start;
        bool    FoundAUD;
        int32    CarryoverCount;
        byte    Carryover[4];
    };

    NalState    _nalState;
    NalState    _lastNalState;
#ifdef TV2INTERNAL
    bool _traceNalLengthShown;
#endif
};

class CRawFrameAudio : public CRawFrame
{
protected:
    __override bool InsertHeaders(CRawPacket& rawPacket, int32& offset, bool bFrameStart);
};

// ===============================================================================================================
// ===============================================================================================================
class CMediaRenderer;
typedef std::map<int, CMediaRenderer*> MediaRendererMap;
typedef MediaRendererMap::iterator MediaRendererMapIter;

/// <summary>
/// Implements the glue needed for connecting a socket to downstream components for rendering RAW streams.
/// </summary>
class CRendererRaw : public CRendererBase
{
public:
    /// <summary>
    /// Constructor
    /// </summary>
    CRendererRaw(IReceiverControl* receiverControl);
    /// <summary>
    /// Destructor
    /// </summary>
    virtual ~CRendererRaw();
    /// <summary>
    /// Implements <see cref="IRenderer.OnSync">IRenderer::OnSync</see>
    /// Resynchronise downstream components. Also forces a microcode reload if so requested.
    /// </summary>
    /// <param name="sync"></param>
    /// <param name="cleanStall"></param>
    /// <seealso cref="IRenderer">IRenderer</seealso>
    __override void        OnSync(bool sync, bool cleanStall);
    /// <summary>
    /// Implements <see cref="IRenderer.ProcessPacket">IRenderer::ProcessPacket</see>
    /// Process the packet downstream
    /// </summary>
    /// <param name="packet">[IN] </param>
    /// <return>
    /// <para>Returns false when packet needs to be retried</para>
    /// </return>
    /// <seealso cref="IRenderer">IRenderer</seealso>
    __override bool        ProcessPacket(IPacket& packet);

private:
    void                   FlushPendingBuffers();

    MediaRendererMap       _mediaRenderers;
};

class CMediaRenderer
{
public:
    static CMediaRenderer* CreateMediaRenderer(IReceiverControl* receiverControl, CRawPacket& packet);

    CMediaRenderer(IReceiverControl* receiverControl);
    virtual ~CMediaRenderer();

    virtual bool           ProcessPacket(CRawPacket& rawPacket); //Returns false when packet needs to be retried
    virtual void           OnSync(bool sync, bool cleanStall);
    virtual bool           ShouldFlushBuffer() { return false; }
    virtual bool           ProcessPendingBuffers();

protected:
    virtual bool           ShouldResetDecoder(CRawPacket& rawPacket) { return false; }
    virtual bool           ShouldDropPacket(const CRawPacket& rawPacket);
    virtual bool           AcquireDecoder(const CRawPacket& rawPacket) = 0;
    virtual void           ResetDecoder(void);
    virtual bool           WritePacket(CRawPacket& rawPacket); //Returns false when packet needs to be retried.
    virtual void           SetDecoderParameters(CRawPacket& rawPacket);
    virtual std::string    GetMediaRendererType() = 0;

protected:
    //Receiver control APIs
    IReceiverControl*      mReceiverControl;

    //Hal decoder factory to use
    IHalDecoderFactory*    mHalDecoderFactory;
    //Decoder factory to use
    IDecoderFactory*       mDecoderFactory;
    //Buffer pool factory to use
    IBufferPoolFactory*    mBufferPoolFactory;
    //Current rendering state
    CRendererState&        mRendererState;
    //Diagnostics to use
    CReceiverDiagnostics&  mDiagnostics;
    //Clock to use
    Clock&                   mClock;
    //Pipe id
    uint32                   mPipeIdN;

    IDecoder*              _decoder;
    CRawFrame*             _rawFrame;
    bool                   _synced;
};

class CMediaRendererVideo : public CMediaRenderer
{
public:
    CMediaRendererVideo(IReceiverControl* receiverControl);
    ~CMediaRendererVideo();

protected:
    __override bool        ShouldFlushBuffer();
    __override bool        ShouldDropPacket(const CRawPacket& rawPacket);
    __override bool        AcquireDecoder(const CRawPacket& rawPacket);
    __override void        SetDecoderParameters(CRawPacket& rawPacket);
    __override std::string GetMediaRendererType() { return "video"; }

private:
    CRawFrameVideo         _videoFrame;
};

class CMediaRendererAudio : public CMediaRenderer
{
public:
    CMediaRendererAudio(IReceiverControl* receiverControl);
    ~CMediaRendererAudio();

protected:
    __override bool        ShouldFlushBuffer();
    __override bool        ShouldResetDecoder(CRawPacket& rawPacket);
    __override bool        ShouldDropPacket(const CRawPacket& rawPacket);
    __override bool        AcquireDecoder(const CRawPacket& rawPacket);
    __override void        ResetDecoder(void);
    __override void        SetDecoderParameters(CRawPacket& rawPacket);
    __override std::string GetMediaRendererType() { return "audio"; }

private:
    CRawFrameAudio         _audioFrame;
};

class CMediaRendererSubtitle : public CMediaRenderer
{
public:
    CMediaRendererSubtitle(IReceiverControl* receiverControl);
    ~CMediaRendererSubtitle();

protected:
    __override bool        ShouldDropPacket(const CRawPacket& rawPacket);
    __override bool        AcquireDecoder(const CRawPacket& rawPacket);
    __override void        SetDecoderParameters(CRawPacket& rawPacket);
    __override std::string GetMediaRendererType() { return "subtitle"; }

private:
    CRawFrame              _subtitleFrame;
};

// ===============================================================================================================
// ===============================================================================================================
