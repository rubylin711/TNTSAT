///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "CRendererRaw.h"
#include "CStreamInfo.h"
#include "Trace.h"
#include <string>
#include <map>
using namespace std;

//#define RENDERER_SPEW
#if defined(RENDERER_SPEW)
#define RENDERER_MSG(x) TRACE(x)
#else
#define RENDERER_MSG(x)
#endif

//#define RENDERER_SPEW_EXTRA
#if defined(RENDERER_SPEW_EXTRA)
#define RENDERER_MSG_EXTRA(x) TRACE(x)
#else
#define RENDERER_MSG_EXTRA(x)
#endif

// ===============================================================================================================
// ===============================================================================================================

CRendererRaw::CRendererRaw(IReceiverControl* receiverControl)
    : CRendererBase(receiverControl)
{
}

CRendererRaw::~CRendererRaw()
{
    FlushPendingBuffers();

    for (MediaRendererMapIter it = _mediaRenderers.begin(); it != _mediaRenderers.end(); ++it)
    {
        delete it->second;
    }
    _mediaRenderers.clear();
}

void CRendererRaw::OnSync(bool sync, bool cleanStall)
{
    RENDERER_MSG(("CRendererChunk::OnSync(%d,%d)", sync, cleanStall));

    MediaRendererMapIter it;
    for (it=_mediaRenderers.begin(); it!=_mediaRenderers.end(); ++it)
    {
        it->second->OnSync(sync, cleanStall);
    }
}

bool CRendererRaw::ProcessPacket(IPacket& packet)
{
    ASSERT(packet.IsType(IPacket::kPacketType_Raw));
    CRawPacket& rawPacket = (CRawPacket&) packet;

    //Find the chunk streamer based on the stream id
    MediaRendererMapIter it = _mediaRenderers.find(rawPacket.StreamId);
    CMediaRenderer* mediaRenderer = NULL;

    //Create a new one if nothing is found for the given stream id
    if (it == _mediaRenderers.end())
    {
        mediaRenderer = CMediaRenderer::CreateMediaRenderer(mReceiverControl, rawPacket);

        //Return if we still do not have a streamer.
        if (mediaRenderer == NULL)
            return true;

        //Add the new stream to our list of streamers
        _mediaRenderers[rawPacket.StreamId] = mediaRenderer;
    }
    else
    {
        mediaRenderer = it->second;
    }

    return mediaRenderer->ProcessPacket(rawPacket);
}

void CRendererRaw::FlushPendingBuffers()
{
    int numRetries = 1;
    bool bHasMoreBuffers;
    CMediaRenderer* pMediaRenderer;

    while (true)
    {
        bHasMoreBuffers = false;
        for (MediaRendererMapIter it = _mediaRenderers.begin(); it != _mediaRenderers.end(); ++it)
        {
            pMediaRenderer = it->second;

            if (pMediaRenderer->ShouldFlushBuffer() && pMediaRenderer->ProcessPendingBuffers())
                bHasMoreBuffers = true;
        }

        if (bHasMoreBuffers == false)
            break;

        numRetries++;
        if (numRetries >= 5)
            break;

        //Wait for 100 milliseconds
        Executive_Sleep(100);
    }

    RENDERER_MSG(("CRendererRaw[%08x], flush buffers ...done, retried:%d times.", mPipeIdN, numRetries));
}

CMediaRenderer* CMediaRenderer::CreateMediaRenderer(IReceiverControl* receiverControl, CRawPacket& packet)
{
    if (packet.IsVideo())
        return new CMediaRendererVideo(receiverControl);

    if (packet.IsAudio())
        return new CMediaRendererAudio(receiverControl);

    if (packet.IsSubtitle())
        return new CMediaRendererSubtitle(receiverControl);

    return NULL;
}

CMediaRenderer::CMediaRenderer(IReceiverControl* receiverControl)
    : mReceiverControl(receiverControl)
    , mHalDecoderFactory(mReceiverControl->GetAVManager()->GetHalDecoderFactory())
    , mDecoderFactory(mReceiverControl->GetAVManager()->GetDecoderFactory())
    , mBufferPoolFactory(mReceiverControl->GetAVManager()->GetBufferPoolFactory())
    , mRendererState(mReceiverControl->GetRendererState())
    , mDiagnostics(mReceiverControl->GetDiagnostics())
    , mClock(mReceiverControl->GetClock())
    , mPipeIdN(mRendererState.mPipeIdN)
    , _decoder(NULL)
    , _rawFrame(NULL)
    , _synced(false)
{
}

CMediaRenderer::~CMediaRenderer()
{
}

//Returns false when packet needs to be retried
bool CMediaRenderer::ProcessPacket(CRawPacket& rawPacket)
{
    //Consume any lingering buffers first
    //We will let the receiver retry if we still have lingering buffers
    ASSERT(_rawFrame);
    bool buffersPending = ProcessPendingBuffers();

    if (ShouldResetDecoder(rawPacket))
    {
        RENDERER_MSG(("[%08x] media[%s] packet requires reset.", mPipeIdN, GetMediaRendererType().c_str()));
    }

    if (ShouldDropPacket(rawPacket))
    {
        RENDERER_MSG(("[%08x] media[%s] packet is to be dropped.", mPipeIdN, GetMediaRendererType().c_str()));
        return true;
    }

    if (buffersPending)
    {
        RENDERER_MSG(("[%08x] media[%s] Buffers are still pending", mPipeIdN, GetMediaRendererType().c_str()));
        return false;
    }

    //Acquire decoder
    if (_decoder == NULL)
    {
        if (AcquireDecoder(rawPacket) == false)
            return true;
    }

	bool write_ok = WritePacket(rawPacket);
	if(write_ok != true){
        TRACE(("WritePacket failed\n"));
	}
    return write_ok;
}

void CMediaRenderer::OnSync(bool sync, bool cleanStall)
{
    //Resync the decoders
    if (_decoder)
    {
        _decoder->OnSync(sync, false, cleanStall);
    }
    _synced = false;
    _rawFrame->Reset();
}

bool CMediaRenderer::ShouldDropPacket(const CRawPacket& rawPacket)
{
    if (rawPacket.IsMetaDataPacket())
    {
        RENDERER_MSG(("[%08x] raw meta data, media type[%s].", mPipeIdN, GetMediaRendererType().c_str()));
        return true;
    }

    return false;
}

void CMediaRenderer::ResetDecoder()
{
    if (_decoder)
    {
        CHECK_ALLOC(mDecoderFactory);
        mDecoderFactory->DisposeDecoder(_decoder);
        _decoder = NULL;
    }

    _rawFrame->ReleaseBuffers();
    _rawFrame->Reset();
    _synced = false;
}

//Return false if packet needs to be retried
bool CMediaRenderer::WritePacket(CRawPacket& rawPacket)
{
    //Allocate buffer and write data to buffer
    if (!_rawFrame->WriteBuffers(rawPacket))
        return false;

    SetDecoderParameters(rawPacket);

    // Send buffer to decoder
    // If the decoder consumes the buffer it'll return NULL.
    // If the decoder refuses the buffer it'll return the buffer to
    // the raw frame for a retry later on

    Buffer* pFirstBuffer = _rawFrame->GetBuffers();

    pFirstBuffer = _decoder->Write( pFirstBuffer );

    _rawFrame->SetBuffers( pFirstBuffer );

    return true;
}

void CMediaRenderer::SetDecoderParameters(CRawPacket& rawPacket)
{
    //the clock treats a zero PTS as invalid so make it very slightly larger
    if (rawPacket.Pts == 0)
    {
        rawPacket.Pts = 1;
    }

    if (rawPacket.Rap)
    {
        _decoder->OnRap();
    }

    if (rawPacket.IsSync)
    {
        _decoder->OnSyncPoint();
    }

    _decoder->SetDuration(rawPacket.Duration);

    _decoder->SetQualityLevel(rawPacket.QualityLevel);

    //If encrypted, pass on encryption mode to decoder
    if (rawPacket.IsEncrypted())
    {
        _decoder->SetDRMHandle(rawPacket.DrmHandle);
    }
    if (rawPacket.KeyID)
    {
        _decoder->SetKeyId(rawPacket.KeyID, rawPacket.KeyIDLength);
    }
    if (IS_VALID_SAMPLEID(rawPacket.SampleID))
    {
        _decoder->SetSampleId(rawPacket.SampleID);
    }
}

bool CMediaRenderer::ProcessPendingBuffers()
{
    if (!_rawFrame->HasBuffers())
        return false;

    if (!_decoder)
    {
        _rawFrame->ReleaseBuffers();
        return false;
    }

    // Send buffer to decoder
    // If the decoder consumes the buffer it'll return NULL.
    // If the decoder refuses the buffer it'll return the buffer to
    // the raw frame for a retry later on

    Buffer* pFirstBuffer = _rawFrame->GetBuffers();

    pFirstBuffer = _decoder->Write(pFirstBuffer);

    _rawFrame->SetBuffers( pFirstBuffer );

    return( pFirstBuffer != NULL );
}

CMediaRendererVideo::CMediaRendererVideo(IReceiverControl* receiverControl)
    : CMediaRenderer(receiverControl)
{
    _rawFrame = &_videoFrame;
}


CMediaRendererVideo::~CMediaRendererVideo()
{
    ResetDecoder();
}

bool CMediaRendererVideo::ShouldFlushBuffer()
{
    return (!mRendererState.FlushVideoDecoderOnRelease);
}

bool CMediaRendererVideo::ShouldDropPacket(const CRawPacket& rawPacket)
{
    if (CMediaRenderer::ShouldDropPacket(rawPacket))
        return true;

    if (!_synced)
    {
        if (!rawPacket.Rap)
        {
            RENDERER_MSG(("[%08x] Sync-ing media[%s]: throwing away non sync point data", mPipeIdN, GetMediaRendererType().c_str()));
            return true;
        }
        RENDERER_MSG(("[%08x] Sync-ing media[%s]", mPipeIdN, GetMediaRendererType().c_str()));
        _synced = true;
    }

    return false;
}

bool CMediaRendererVideo::AcquireDecoder(const CRawPacket& rawPacket)
{
    CStreamInfo si;

    //Extract stream type
    si.Type = StreamInfoType_Video;
    si.Format = FourCCToStreamType(rawPacket.FourCC, StreamType_Video_WMV9);
    si.StreamId = rawPacket.StreamId;

    //Now create the audio decoder
    CHECK_ALLOC(mDecoderFactory);
    _decoder = mDecoderFactory->AcquireDecoder(mReceiverControl, si);
    if (!_decoder)
    {
        return false;
    }

    if (si.Format == StreamType_Video_WMV9)
    {
        IPTV_HAL_DECODER_WMV_HEADER value;

        value.nSequenceHeader = rawPacket.Video.PrivateData ? *((const uint32*)rawPacket.Video.PrivateData) : (uint32)0;
        value.nWidth = (uint16)rawPacket.Video.FrameWidth;
        value.nHeight = (uint16)rawPacket.Video.FrameHeight;
        value.nFourCC = (uint32)rawPacket.FourCC;
        value.nAspW = (uint16)rawPacket.Video.PixelAspectX;
        value.nAspH = (uint16)rawPacket.Video.PixelAspectY;

        _decoder->IoControl(DECODER_SETWMVHEADER, &value, sizeof(value), NULL, 0);

        //Post the WMV3 parameters
        mDiagnostics.OnVideoDecoderParametersEvent(rawPacket.FourCC, rawPacket.Video.FrameWidth, rawPacket.Video.FrameHeight, rawPacket.Video.PixelAspectX, rawPacket.Video.PixelAspectY);
    }
    else
    {
        //For other codecs, post FourCC only
        mDiagnostics.OnVideoDecoderParametersEvent(rawPacket.FourCC);
    }

    //Get the decoder ready for work
    if (!_decoder->Acquire())
    {
        ResetDecoder();
        return false;
    }

    CHECK_ALLOC(mBufferPoolFactory);
    _videoFrame.Init(mBufferPoolFactory->Get(BUFFERPOOL_VIDEO), (StreamType)si.Format);

    //And video is primary stream
    mClock.VideoIsPrimaryStream(true);

    return true;
}

//Return false if packet needs to be retried
void CMediaRendererVideo::SetDecoderParameters(CRawPacket& rawPacket)
{
    CMediaRenderer::SetDecoderParameters(rawPacket);

    //Process correspondences in the clock
    //Note that the correspondences are faked in the socket
    //so that there is consistency in the downstream component in terms of
    //handling different timelines and the code paths for trick mode handling etc.
    //Clock considers corresondences w/NTP of 0 to be invalid, so change NTP to 1.
    if (rawPacket.Rap && IS_VALID_TIME(rawPacket.NTP))
    {
        NTP_PCR_PAIR correspondence;
        correspondence.ntp = rawPacket.NTP ? rawPacket.NTP : 1;
        correspondence.pcr = rawPacket.PCR;
        mClock.PushCorrespondence(&correspondence);
    }

    if (IS_VALID_TIME(rawPacket.Pts))
    {
        _decoder->SetPTS(rawPacket.Pts);
    }

    if (IS_VALID_TIME(rawPacket.Dts))
    {
        _decoder->SetDTS(rawPacket.Dts);
    }

    _decoder->SetFrameSize(rawPacket.Video.FrameWidth, rawPacket.Video.FrameHeight);
}

CMediaRendererAudio::CMediaRendererAudio(IReceiverControl* receiverControl)
    : CMediaRenderer(receiverControl)
{
    _rawFrame = &_audioFrame;
}

CMediaRendererAudio::~CMediaRendererAudio()
{
    ResetDecoder();
}

bool CMediaRendererAudio::ShouldFlushBuffer()
{
    return (!mRendererState.FlushAudioDecoderOnRelease);
}

bool CMediaRendererAudio::ShouldResetDecoder(CRawPacket& rawPacket)
{
    //Check if audio needs to be reset
    if (rawPacket.Flags & RAWPACKET_IS_RESETAUDIO)
    {
        ResetDecoder();
        RENDERER_MSG(("CMediaRendererAudio::ShouldResetDecoder(), reset decoder"));
        rawPacket.Flags &= ~RAWPACKET_IS_RESETAUDIO;

        return true;
    }

    return false;
}

bool CMediaRendererAudio::ShouldDropPacket(const CRawPacket& rawPacket)
{
    if (CMediaRenderer::ShouldDropPacket(rawPacket))
        return true;

    //Check audio PID
    if (!mRendererState.AudioLanguage.IsUsedPid(rawPacket.StreamId) && mRendererState.AudioLanguage.IsUsedSet())
    {
        //Discard packet from unused audio stream
        return true;
    }

    return false;
}

bool CMediaRendererAudio::AcquireDecoder(const CRawPacket& rawPacket)
{
    //Acquire audio decoder
    CStreamInfo si;

    //Extract stream type
    si.Type = StreamInfoType_Audio;
    si.Format = FourCCToStreamType(rawPacket.FourCC, StreamType_Audio_WMA);
    if (si.Format == StreamType_Audio_11172)
    {
        si.IsMp3 = true;
    }
    si.StreamId = rawPacket.StreamId;

    //If no language has been selected, just use PID from first audio packet
    if (!mRendererState.AudioLanguage.IsUsedSet())
    {
        mRendererState.AudioLanguage.SetUsed(0, 0, si.StreamId, 1);
    }

    //Now create the audio decoder
    CHECK_ALLOC(mDecoderFactory);
    _decoder = mDecoderFactory->AcquireDecoder(mReceiverControl, si);
    if (!_decoder)
    {
        return false;
    }

    // send the audio parameters to the decoder
    IPTV_HAL_DECODER_AUDIO_HEADER audioHeader;
    memset(&audioHeader, 0, sizeof(audioHeader));

    audioHeader.iVersionNumber = rawPacket.Audio.VersionNumber;
    audioHeader.cSubband       = 0x8800;
    audioHeader.iSamplingRate  = rawPacket.Audio.SamplingRate;
    audioHeader.cChannel       = rawPacket.Audio.ChannelNumber;
    audioHeader.cBytePerSec    = rawPacket.Audio.BytesPerSec;
    audioHeader.cbPacketLength = rawPacket.Audio.BlockSize;
    audioHeader.wEncodeOpt     = rawPacket.Audio.EncoderOption;
    audioHeader.cBitsPerSample = rawPacket.Audio.BitsPerSample;

    if (si.Format == StreamType_Audio_WMAPRO)
    {
        audioHeader.cValidBitsPerSample = rawPacket.Audio.ValidBitsPerSample;
        audioHeader.u32ChannelMask = rawPacket.Audio.ChannelMask;

        //TODO: Set these once DRC values become available
        audioHeader.bDRCDataIsValid = 0;
        audioHeader.u32DRCSetting = 0;
        audioHeader.u32DRCAverageReference = 0;
        audioHeader.u32DRCAverageTarget = 0;
        audioHeader.u32DRCPeakReference = 0;
        audioHeader.u32DRCPeakTarget = 0;
    }
    else
    {
        audioHeader.cValidBitsPerSample = audioHeader.cBitsPerSample;
    }

    RENDERER_MSG(("# WMA ver %d rate %d ch %d BytesPerSec %d size %d EncOpt 0x%x ChannelMask 0x%x vbps %d",
        rawPacket.Audio.VersionNumber,
        rawPacket.Audio.SamplingRate,
        rawPacket.Audio.ChannelNumber,
        rawPacket.Audio.BytesPerSec,
        rawPacket.Audio.BlockSize,
        rawPacket.Audio.EncoderOption,
        rawPacket.Audio.ChannelMask,
        audioHeader.cValidBitsPerSample));

    _decoder->IoControl(DECODER_SETAUDIOHEADER, &audioHeader, sizeof(audioHeader), NULL, 0);

    //Post audio stream parameters
    mDiagnostics.OnAudioDecoderParametersEvent
        (
        rawPacket.FourCC,
        rawPacket.Audio.VersionNumber,
        0x8800,
        rawPacket.Audio.SamplingRate,
        rawPacket.Audio.ChannelNumber,
        rawPacket.Audio.BytesPerSec,
        rawPacket.Audio.BlockSize,
        rawPacket.Audio.EncoderOption,
        0
        );

    //Get the decoder ready for work
    if (!_decoder->Acquire())
    {
        ResetDecoder();
        return false;
    }

    CHECK_ALLOC(mBufferPoolFactory);
    _audioFrame.Init(mBufferPoolFactory->Get(BUFFERPOOL_AUDIO), (StreamType)si.Format);

    if (rawPacket.AudioOnly)
    {
        //We force teardown the video picture buffer if there is no video stream present
        CHECK_ALLOC(mHalDecoderFactory);
        mHalDecoderFactory->TeardownPicture(mPipeIdN);
    }

    return true;
}

void CMediaRendererAudio::SetDecoderParameters(CRawPacket& rawPacket)
{
    CMediaRenderer::SetDecoderParameters(rawPacket);

    //VideoIsPrimaryStream flag can be reset by clock without disposing
    //of the decoder so we have to set this for every packet
    if (rawPacket.AudioOnly)
    {
        //Audio is primary stream
        mClock.VideoIsPrimaryStream(false);
    }

    if (IS_VALID_TIME(rawPacket.Pts))
    {
        _decoder->SetPTS(rawPacket.Pts);
    }
}

void CMediaRendererAudio::ResetDecoder()
{
    if (_decoder)
    {
        //Check with the audio decoder if it needs to stick around for rendering completion
        //of all its buffered data
        if (!_decoder->SetExpirationTime(true, mClock.LastDecoderVideoPts()))
        {
            CHECK_ALLOC(mDecoderFactory);
            mDecoderFactory->DisposeDecoder(_decoder);
        }
        _decoder = NULL;
    }

    CMediaRenderer::ResetDecoder();
}

CMediaRendererSubtitle::CMediaRendererSubtitle(IReceiverControl* receiverControl)
    : CMediaRenderer(receiverControl)
{
    _rawFrame = &_subtitleFrame;
}

CMediaRendererSubtitle::~CMediaRendererSubtitle()
{
    ResetDecoder();
}

bool CMediaRendererSubtitle::ShouldDropPacket(const CRawPacket& rawPacket)
{
    if (CMediaRenderer::ShouldDropPacket(rawPacket))
        return true;

    if (rawPacket.DataLength > (int32)mBufferPoolFactory->Size(BUFFERPOOL_SUBTITLES))
    {
        RENDERER_MSG(("DFXP document size %d exceeds buffer size of %d.", rawPacket.DataLength, mBufferPoolFactory->Size(BUFFERPOOL_SUBTITLES)));
        ASSERT(false);

        //Drop the document, we have no buffer space for it and partial docs are no use to us anyway
        return true;
    }

    return false;
}

bool CMediaRendererSubtitle::AcquireDecoder(const CRawPacket& rawPacket)
{
    CStreamInfo si;

    //Extract stream type
    si.Type = StreamInfoType_Subtitle;
    si.Format = StreamType_DFXP;
    si.StreamId = rawPacket.StreamId;

    //Now create the subtitles decoder
    try
    {
        CHECK_ALLOC(mDecoderFactory);
        _decoder = mDecoderFactory->AcquireDecoder(mReceiverControl, si);
    }
    catch (const DecoderException&)
    {
        return false;
    }

    if (!_decoder)
    {
        return false;
    }

    //Get the decoder ready for work
    if (!_decoder->Acquire())
    {
        ResetDecoder();
        return false;
    }

    CHECK_ALLOC(mBufferPoolFactory);
    _subtitleFrame.Init(mBufferPoolFactory->Get(BUFFERPOOL_SUBTITLES), (StreamType)si.Format);

    return true;
}

void CMediaRendererSubtitle::SetDecoderParameters(CRawPacket& rawPacket)
{
    CMediaRenderer::SetDecoderParameters(rawPacket);

    _decoder->SetPTS(rawPacket.Subtitles.startTime);
}

// ===============================================================================================================
// ===============================================================================================================

CRawFrame::CRawFrame()
    : _bufferPool(NULL)
    , _streamType(StreamType_None)
{
    ResetFrameCounters();
}

CRawFrame::~CRawFrame()
{
    _chain.Release();
}

void CRawFrame::Init(IBufferPool* bufferPool, StreamType streamType)
{
    _bufferPool = bufferPool;
    _streamType = streamType;
}

void CRawFrame::Reset()
{
    ResetFrameCounters();
}

//Number of bytes left in a buffer before we decide to start a new one instead
#define NEW_BUFFER_THRESHOLD    (0x10)

//Write data to buffer chain, allocating new buffer if necessary
//Returns false if buffer fails to allocate
bool CRawFrame::Write(const byte* data, int32 len, bool encrypted)
{
    Buffer* buffer = NULL;
    const byte* readPtr = data;
    int bytesToWrite = len;

    //Determine if we can append to the end of the previous buffer
    Buffer* tail = _chain.GetChainLast();
    if (tail)
    {
        bool tailEncrypted = tail->GetFlag(BUFFER_ENCRYPTED);
        if ((encrypted == tailEncrypted) && ((bytesToWrite <= tail->Space()) || (tail->Space() < NEW_BUFFER_THRESHOLD)))
        {
            buffer = tail;
        }
    }

    //Loop until all data is copied to buffers
    while (bytesToWrite > 0)
    {
        if (!buffer)
        {
            buffer = _bufferPool->Get();
            if (!buffer)
            {
                //Failed to allocate buffers - release buffers and signal retry
                RENDERER_MSG(("CRawFrame(%x)::Write - Failed to allocate buffers for data:0x%x, dataLength:%d, _streamType", _streamType, data, len));
                _chain.Release();
                return false;
            }

            _chain.Add(buffer);
            if (encrypted)
            {
                buffer->SetFlag(BUFFER_ENCRYPTED);
            }

            if (_subSampleState.NewBlock)
            {
                //Current state of PIFF spec is to not increment block counter
                //between subsamples so leave flag unset for now
                //buffer->SetFlag(BUFFER_NEWBLOCK);
                _subSampleState.NewBlock = false;
            }
        }

        //Copy data
        int32 bytesWritten = buffer->Write(readPtr, bytesToWrite);
        RENDERER_MSG_EXTRA(("CRawFrame(%x)::Write(%d,%d) - Mark %d toWrite %d", _streamType, len, encrypted, buffer->Mark, bytesWritten));

        //Adjust pointers
        readPtr += bytesWritten;
        bytesToWrite -= bytesWritten;

        //Clear buffer pointer so we fetch a new one if we have more data to write
        buffer = NULL;
    }
    return true;
}

void CRawFrame::ResetFrameCounters()
{
    _subSampleState.Entry = 0;
    _subSampleState.Len = 0;
    _subSampleState.Encrypted = false;
    _subSampleState.NewBlock = false;
}

void CRawFrame::StoreFrameState()
{
    _lastSubSampleState.Entry = _subSampleState.Entry;
    _lastSubSampleState.Len = _subSampleState.Len;
    _lastSubSampleState.Encrypted = _subSampleState.Encrypted;
    _lastSubSampleState.NewBlock = _subSampleState.NewBlock;
}

void CRawFrame::RestoreFrameState()
{
    _subSampleState.Entry = _lastSubSampleState.Entry;
    _subSampleState.Encrypted = _lastSubSampleState.Encrypted;
    _subSampleState.Len = _lastSubSampleState.Len;
    _subSampleState.NewBlock = _lastSubSampleState.NewBlock;
}

int32 CRawFrame::GetNextDataLength(CRawPacket& rawPacket, int32& offset, bool& encrypted)
{
    int32 bytesRemaining = rawPacket.DataLength - offset;
    encrypted = rawPacket.IsEncrypted();

    //If there are no sub-samples, just return remaining length
    if (!encrypted || !rawPacket.SubSampleEntries)
    {
        return bytesRemaining;
    }

    int32 len = 0;
    if (_subSampleState.Len)
    {
        //Continuing to read from previous subsample
        len = _subSampleState.Len;
        encrypted = !_subSampleState.Encrypted;
    }
    else
    {
        while (!len && (_subSampleState.Entry < rawPacket.SubSampleEntries))
        {
            //Record the length of current entry
            encrypted = _subSampleState.Encrypted;
            len = encrypted ? rawPacket.SubSampleEncryptedBytes[_subSampleState.Entry] : (uint32) rawPacket.SubSampleClearBytes[_subSampleState.Entry];

            _subSampleState.NewBlock = encrypted;

            //TRACE(("# SSE %d/%d len %d enc %d", _subSampleState.Entry, rawPacket.SubSampleEntries, len, encrypted));

            //Advance counters to next entry
            _subSampleState.Encrypted = !_subSampleState.Encrypted;
            if (!_subSampleState.Encrypted)
                _subSampleState.Entry++;
        }

        //If we still have no length, we must have run out sub-sample entries
        if (!len)
        {
            encrypted = true;
            return bytesRemaining;
        }
    }

    //Does sub-sample overlap the end of the packet?
    if (len > bytesRemaining)
    {
        _subSampleState.Len = len - bytesRemaining;
        return bytesRemaining;
    }

    _subSampleState.Len = 0;
    return (int32) len;
}

bool CRawFrame::WriteBuffers(CRawPacket& rawPacket)
{
    bool bFrameStart = IS_VALID_TIME(rawPacket.Pts);

    if (bFrameStart)
    {
        ResetFrameCounters();
    }

    //Need to store previous frame state in case we need to abort midway and retry
    StoreFrameState();

    int32 packetOffset = 0;
    while (packetOffset < rawPacket.DataLength)
    {
        bool isEncrypted = false;
        int32 len = GetNextDataLength(rawPacket, packetOffset, isEncrypted);
        if (!len)
            break;

        if (!InsertHeaders(rawPacket, packetOffset, bFrameStart))
            goto retry;

        //Write out subsample
        if (!Write(&(rawPacket.Data[packetOffset]), len, isEncrypted))
            goto retry;

        packetOffset += len;
        bFrameStart = false;
    }

    if (rawPacket.Flags & RAWPACKET_IS_ENDOFFRAME)
    {
        _chain.SetEndFrame();
    }

    return true;

retry:
    //Revert to frame state at beginning of this packet
    RestoreFrameState();
    return false;
}

// ===============================================================================================================
// ===============================================================================================================

CRawFrameVideo::CRawFrameVideo()
{
#ifdef TV2INTERNAL
    _traceNalLengthShown = false;
#endif
    ResetNal();
}

void CRawFrameVideo::Reset()
{
    CRawFrame::Reset();
    ResetNal();
}

void CRawFrameVideo::ResetNal()
{
    _nalState.BaseLen = 0;
    _nalState.Len = 0;
    _nalState.Start = false;
    _nalState.FoundAUD = false;
    _nalState.CarryoverCount = 0;
}

void CRawFrameVideo::StoreFrameState()
{
    CRawFrame::StoreFrameState();
    memcpy_s(&_lastNalState, sizeof(NalState), &_nalState, sizeof(NalState));
}

void CRawFrameVideo::RestoreFrameState()
{
    CRawFrame::RestoreFrameState();
    memcpy_s(&_nalState, sizeof(NalState), &_lastNalState, sizeof(NalState));
}

bool CRawFrameVideo::ParseNalLen(CRawPacket& rawPacket, int32& offset, int32& len)
{
    //Valid unit lengths are 1,2,4 with default of 4
    int unitLen = (rawPacket.Video.NalUnitLength > 0) && (rawPacket.Video.NalUnitLength <= 2) ? rawPacket.Video.NalUnitLength : 4;
    const byte* ptrLen = rawPacket.Data + offset;

    //Check if we need to carryover until next packet
    if (len + _nalState.CarryoverCount < unitLen)
    {
        while (len > 0)
        {
            _nalState.Carryover[_nalState.CarryoverCount++] = rawPacket.Data[offset++];
            len--;
        }

        RENDERER_MSG(("CRawFrameVideo::ParseNalLen() NAL len carryover from last len %d count %d [%02x][%02x][%02x][%02x]",
            unitLen, _nalState.CarryoverCount, _nalState.Carryover[0], _nalState.Carryover[1], _nalState.Carryover[2], _nalState.Carryover[3]));

        return false;
    }

    //Check for carryover bytes and complete unit length
    if (_nalState.CarryoverCount > 0)
    {
        //Assumes that carryover bytes plus remaining length is enough for NAL length plus 1 byte NAL type
        ASSERT((len + _nalState.CarryoverCount) >= (unitLen + 1));
        while (_nalState.CarryoverCount < unitLen)
        {
            _nalState.Carryover[_nalState.CarryoverCount++] = rawPacket.Data[offset++];
            len--;
        }

        RENDERER_MSG(("CRawFrameVideo::ParseNalLen() NAL len carryover to next len %d count %d [%02x][%02x][%02x][%02x]",
            unitLen, _nalState.CarryoverCount, _nalState.Carryover[0], _nalState.Carryover[1], _nalState.Carryover[2], _nalState.Carryover[3]));

        ptrLen = _nalState.Carryover;
        _nalState.CarryoverCount = 0;
    }
    else
    {
        //Sanity check for buffer overrun
        if (offset + unitLen > rawPacket.DataLength)
        {
            TRACE_ERROR(("CRawFrameVideo::ParseNalLen(%d, %d) Invalid state! unitLen %d Data %x ptrLen %x", offset, len, unitLen, rawPacket.Data, ptrLen));

            //Just pass on base length
            ASSERT(false);
            _nalState.Len = len;
            return true;
        }

        offset += unitLen;
        len -= unitLen;
    }

    //Extract the NAL length
    switch(unitLen)
    {
    case 1:
        _nalState.Len = (uint32)ptrLen[0];
        break;
    case 2:
        _nalState.Len = ((uint32)ptrLen[0] << 8) | ptrLen[1];
        break;
    default:
        _nalState.Len = ((uint32)ptrLen[0] << 24) | ((uint32)ptrLen[1] << 16) | ((uint32)ptrLen[2] << 8) | ptrLen[3];
        break;
    }

    _nalState.Start = true;
    return true;
}

int32 CRawFrameVideo::GetNextDataLength(CRawPacket& rawPacket, int32& offset, bool& encrypted)
{
    int32 len = 0;

    do
    {
        if (!_nalState.BaseLen)
        {
            len = CRawFrame::GetNextDataLength(rawPacket, offset, encrypted);
        }
        else
        {
            len = _nalState.BaseLen;
            _nalState.BaseLen = 0;
        }

        if (rawPacket.Flags & RAWPACKET_IS_MPEG4_PART15)
        {
            if (!_nalState.Len)
            {
                //Assuming that data should be unencrypted at this point else we can't parse NAL length
                if (!encrypted)
                {
                    if (!ParseNalLen(rawPacket, offset, len) || (len == 0))
                    {
                        //NAL carrying over to next packet
                        continue;
                    }

                    //Sanity check for NAL len (1 MB - twice size of available PES video buffer pool)
                    if (_nalState.Len > 2 * _bufferPool->PoolSize())
                    {
                        TRACE_ERROR(("CRawFrameVideo::GetNextDataLength - Invalid NAL unitLen 0x%x off %d len %d", _nalState.Len, offset, len));

                        //ASSERT(false);
                        _nalState.Len = 0;
                        continue;
                    }
                }
                else
                {
                    //Can't parse encrypted NAL length - just pass on base length in this case
                    //NOTE: This ASSERT normally occurs when we get an encrypted H264 asset without PIFF subsample entries.
                    //Often, the asset will actually be in AnnexB format ("AVCB"), but will be marked in the manifest as
                    //MPEG4 Part 15 ("H264" or "AVC1"), so if the FourCC code in the manifest is corrected, then the stream will work.
                    //ASSERT(false);
#ifdef TV2INTERNAL
                    if (!_traceNalLengthShown)
                    {
                        _traceNalLengthShown = true;
                        TRACE_ERROR(("CRawFrameVideo::GetNextDataLength - Can't parse encrypted NAL length"));
                    }
#endif
                    break;
                }
            }

            //Take subset of NAL length and base length
            if ((int32) _nalState.Len < len)
            {
                _nalState.BaseLen = len - _nalState.Len;
                len = _nalState.Len;
                _nalState.Len = 0;
            }
            else
            {
                _nalState.Len -= len;
            }
        }
    } while ((len == 0) && (offset < rawPacket.DataLength));

    return len;
}

#define IS_NAL_TYPE_AUD(x) ((x & 0x1F) == 0x09)
static const byte sInBandStartCode[]    = { 0x00, 0x00, 0x01, 0x0d };
static const byte sNalStartCode[]        = { 0x00, 0x00, 0x00, 0x01 };

// Handles insertion of headers at beginning of frame or subsample
bool CRawFrameVideo::InsertHeaders(CRawPacket& rawPacket, int32& offset, bool bFrameStart)
{
    if ((_streamType == StreamType_Video_VC1) && bFrameStart)
    {
        //Stick in the sequence header if available but only on RAP points
        if (rawPacket.Rap && rawPacket.Video.PrivateData && rawPacket.Video.PrivateDataLen >= 4)
        {
            const byte* privateData = rawPacket.Video.PrivateData;
            uint32 privateDataLen = rawPacket.Video.PrivateDataLen;
            while (privateDataLen >= 4 &&
                (privateData[0] != 0x00 || privateData[1] != 0x00 || privateData[2] != 0x01 || privateData[3] != 0x0f))
            {
                privateData++;
                privateDataLen--;
            }
            if (privateDataLen < 4)
            {
                ASSERT(false);
                RENDERER_MSG(("CRawFrameVideo::InsertHeaders - Failed to parse sequence header for pts:%lld, data:0x%x, dataLength:%d", rawPacket.Pts, rawPacket.Data, rawPacket.DataLength));
            }

            //Write private data to buffer
            if (!Write(privateData, privateDataLen))
                return false;
        }
        //If the raw packet already has a sequence header or picture start in-band, don't attach one
        if (!(rawPacket.Flags & RAWPACKET_IS_STARTCODE_INBAND) &&
            !((rawPacket.DataLength >= 3) && (rawPacket.Data[0] == 0x00) && (rawPacket.Data[1] == 0x00) && (rawPacket.Data[2] == 0x01))
            )
        {
            //Stick in picture start code along
            if (!Write(sInBandStartCode, sizeof(sInBandStartCode)))
                return false;
        }
    }
    else if ((rawPacket.Flags & RAWPACKET_IS_MPEG4_PART15) && _nalState.Start)
    {
        //Insert SPS/PPS after Access Unit Delimiter NAL or if NAL starts at beginning of packet
        //(which usually only happens at the beginning of each track / chunk)
        if (rawPacket.Rap && (_nalState.FoundAUD || (bFrameStart && !IS_NAL_TYPE_AUD(rawPacket.Data[offset]))))
        {
            //Copy the private data only at the beginning of RAP point
            const byte* privateData = rawPacket.Video.PrivateData;
            uint32 privateDataLen = rawPacket.Video.PrivateDataLen;
            if (privateDataLen > 0)
            {
                if (!Write(privateData, privateDataLen))
                    return false;
            }

            RENDERER_MSG(("#SPS/PPS AUD %d FrameStart %d NALType %d", _nalState.FoundAUD, bFrameStart, rawPacket.Data[offset]));
        }

        //Write the NAL start code
        if (!Write(sNalStartCode, sizeof(sNalStartCode)))
            return false;

        //Look for access unit delimiter NAL before inserting the SPS/PPS headers
        _nalState.FoundAUD = IS_NAL_TYPE_AUD(rawPacket.Data[offset]);
        _nalState.Start = false;
    }
    return true;
}

// ===============================================================================================================
// ===============================================================================================================

bool CRawFrameAudio::InsertHeaders(CRawPacket& rawPacket, int32& offset, bool bFrameStart)
{
    if (bFrameStart && rawPacket.Audio.HeaderData && rawPacket.Audio.HeaderDataLen)
    {
        //Insert audio header (i.e. ADTS header)
        if (!Write(rawPacket.Audio.HeaderData, rawPacket.Audio.HeaderDataLen))
            return false;
    }
    return true;
}

// ===============================================================================================================
// ===============================================================================================================
