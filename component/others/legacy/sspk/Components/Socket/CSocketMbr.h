///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

/*
 * CSockethMbr.h
 *
 * This file describes the functionality of the CSocketMbr
 */
#pragma once

#include "CSocketMbrManifest.h"
#include "CSocketMbrRetry.h"
#include "CSocketRaw.h"
#include "IStreamerHttp.h"
#include "ADTSHeader.h"
#include "Heuristics.h"
#include "ManifestParser.h"
#include "MP4Parser.h"
#include "RateControl.h"

#include <string>

class CSocketMbr;
class CSocketMbrChunk;

// ===============================================================================================================
// ===============================================================================================================

class MbrTuneInfo
{
public:
    MbrTuneInfo();

    void    Init( _In_ CTuneRequest& tuneRequest, _In_ int64 hnsMarkIn );

    bool    IsInTrickMode() { return (_speed != 0) && (_speed != 1); }

    bool    MoveChunkIndex(
                _Inout_ MBR::CMediaPosition* pPos,
                _In_ MBR::CMediaStreamDescription* pMSD );

    //0->pause, 1->normal play, 10->10x ff, -10->10x rew
    int32   _speed;
    //Whether to take the floor or ceiling in seeking
    bool    _floorIt;
    //Frames per second to be played back during tricks
    int32   _fps;
    //Start position for the tune
    int64   _hnsStartPos;
    //For throttling the input bitrate over the network
    uint32  _maxBandwidth;
    //Fixed quality level (ordered index)
    byte    _ql;
};

// ===============================================================================================================
// MBR chunk reader - used for each MBR stream
//
// This is being derived from CSocketRaw since we think the underlying socket may need to
// support different socket types: Http/HttpRateControlled/File/etc
// ===============================================================================================================

class CSocketMbrChunk: public CSocketRaw, public MP4Streamer
{
public:
    CSocketMbrChunk(IAVManager* avManager, uint32 pipeIdN, const CTuneRequest& tuneRequest, eSocketType socketType = eSocketType_MbrChunk);
    virtual ~CSocketMbrChunk();

public:
    //ISocket interfaces
    __override bool         Connect(void);
    __override bool         Close(void);
    __override bool         IsConnected(void) const;
    __override bool         Command(const std::string& command, const std::vector<std::string>& args);

    //CSocketRaw interfaces
    __override int          Recv(CRawPacket& rawPacket);

    //MP4Streamer interface
    __override int          RecvCount(__out_ecount(dstlen) byte* dst, int dstlen, int dataLength, int timeout = 0);

    //MBR extensions
    void                    Init(CSocketMbr* pSocketMbr, MBR::CMediaStreamDescription* pMSD, uint64 hnsPos, bool isTuning);
    bool                    Connect(CTuneRequest& tuneRequest);

    uint64                  NextTimestamp() const { return _hnsNextDts; }
    uint32                  GetStreamId() const { return _streamId; }
    MediaStreamType         GetMediaType() const { return _mediaType; }
    MediaStreamSubType      GetMediaSubType() const { return _mediaSubType; }
    uint32                  GetCurrentBitrate();
    uint32                  GetCurrentQualityLevel();

    //Specifies KeyID to be passed down with raw packet
    void                    SetSignedKeyID(const byte *pKeyID, uint32 len);

private:
    bool                    ConnectSocketAtPos(uint64 hnsTime);

    int                     SkipRecv(uint64 pos);
    int                     SkipCount(int len);
    int                     SkipToEnd();

    //MBR extensions

    struct FRAME_INFO
    {
        bool isSync;
        bool isRAP;
        uint64 pts;
        uint64 dts;
        uint64 duration;
        uint64 sampleID;
        uint16 cDrmSubSamples;
        uint16* prgDrmSubSampleClearBytes;
        uint32* prgDrmSubSampleEncryptedBytes;
        uint32 cbFrameSize;

        FRAME_INFO()
        {
            memset( this, 0, sizeof(FRAME_INFO) );
            sampleID = INVALID_SAMPLEID;
            pts = INVALID_TIME;
            dts = INVALID_TIME;
        }
    };

    // keeps track of retries
    struct CSocketMbrChunkRetry
    {
        bool triedHigherBitRate;
        int triedSameIndexDifferentBitRateCount;
        CSocketMbrRetry retryState;

        CSocketMbrChunkRetry()
        {
            triedHigherBitRate = false;
            triedSameIndexDifferentBitRateCount = 0;
        }
    };

    bool                    GetNextFrameInfo( _Out_ FRAME_INFO* pFI );

    int                     RecvNextFrame(CRawPacket& rawPacket);
    int                     RecvNextFrame_ChunkHeader(CRawPacket& rawPacket);
    int                     RecvNextFrame_video(CRawPacket& rawPacket);
    int                     RecvNextFrame_audio(CRawPacket& rawPacket);
    int                     RecvNextFrame_text(CRawPacket& rawPacket);

    bool                    RecvChunkHeader(uint64 hnsStartPos);
    void                    RecvChildChunkHeader();
    bool                    RetryChunk(_Inout_ CSocketMbrChunkRetry* retryInfo);
    pkRESULT                RecvNextChunk(void);
    pkRESULT                SendNextChunkRequest();

    void                    UpdateHeuristics();
    void                    AddBwInfoHeader(_Inout_ std::string *pHeaderString);

    void                    SetSubSample(int entries, const uint16* clear, const uint32* encrypted);
    void                    FreeSubSampleArrays();

    //Utilities
    void                    SetQualityLevel();
    bool                    PrepareChunkUrl();

private:

    //Default TCP receive buffer size
    static const uint32     TCP_BUFFER_SIZE_HTTP = 0;

    //Streamer to be used
    IStreamerHttp*          _pChunkSocket;

    CSocketMbr*             _pMbrSocket;
    CMbrManifest*           _pMbrManifest;
    AutoRefPtr<MBR::CChunkManifest>    _apChunkManifest;
    MBR::CHeuristicsMBR*    _pHeuristics;

    MBR::CMediaStreamDescription* _pMSD;
    MediaStreamType         _mediaType;
    MediaStreamSubType      _mediaSubType;

    //StreamId used to request chunk URL
    uint32                  _streamId;
    //Number of fragments ahead, used to indicate end of live.
    int32                   _fragmentsAhead;

    uint32                  _drmHandle;
    bool                    _isPlayReadyEncrypted;

    //Trick related info.
    MbrTuneInfo*            _pTuneInfo;

    //URL to GET chunk
    wstring                 _wstrChunkURL;

    // Indicates the request for the next chunk must be sent
    bool                    _mustRequestNextChunk;

    //Handle to chunk header for the container parser
    bool                    _chunkHeader;
    //Current quality index within this stream
    uint32                  _mbrIndex;
    //Next quality index within this stream
    uint32                  _nextMbrIndex;

    //Current requested chunk index of this stream
    CMediaPosition          _mediaPos;

    //Current "in-response" chunk index of this stream
    int32                   _chunkIndexInResponse;
    //Timestamp of the end of the current frame
    uint64                  _hnsNextDts;
    //Start position for this stream
    uint64                  _hnsStartPos;
    //Chunk buffer pointer
    byte*                   _chunkBuffer;
    //Block size of the chunk buffer pointer
    uint32                  _chunkBlockSize;
    //Number of bytes of the chunk header for the current chunk.
    uint32                  _chunkHeaderLength;

    //Block reading and fragments
    //
    //Bytes remaining for the frame that's not sent to the decoder yet
    uint32                  _remainingFrameSize;
    //Bytes remaining in _chunkBuffer
    uint32                  _remainingBufferSize;
    //Start position of the remaining data in _chunkBuffer
    uint32                  _bufferOffset;
    //Bytes remaining in the mdat
    uint32                  _remainingMdatByteCount;
    //Read size on the socket
    uint32                  _recvBlockSize;

    //RawPacket data
    uint32                  _fourcc;
    WAVEFORMATEX*           _pWaveFormatEx;
    ADTSHeader              _adtsHeader;
    uint32                  _signedKeyLen;
    byte*                   _signedKeyID;
    bool                    _signedKeyChanged;
    uint64                  _sampleID;
    uint16                  _subSampleEntries;
    int                     _subSampleAllocSize;
    uint16*                 _subSampleClearBytes;
    uint32*                 _subSampleEncryptedBytes;

    //Bitrate calculation
    RateControl             _rateControl;
    bool                    _measuringRate;
    uint32                  _mdatSize;

    //TCP receiving window in bytes
    uint32                  _tcpWindowSize;
    //Fixed quality level (ordered index)
    byte                    _ql;

    //MP4 parser
    FMP4Parser              _fmp4Parser;
    FMP4Info*               _fmp4Info;

    CRawPacket::eDataType   _rawDataType;

    bool                    _started;
    bool                    _resetAudio;
    bool                    _signalDiscontinuity;

    uint32                  _maxStreamBandwidth;    // max stream bandwidth

    XDRM_OPL_DATA           _OPLData;



#ifdef TV2INTERNAL
    //Smooth Streaming flag to force switches to Quality Levels
    bool                    _diagsForceQualitySwitch;
    int32                   _diagsNewQualityLevel;
    //Force smooth streaming to report a fake bitrate to heuristics
    uint32                  _diagsFakeSSBitrate;

    uint32                  _tickStartChunkRequest;             // tick count right before sending the HTTP request for chunk [_chunkIndex]
    uint32                  _tickStartChunkResponseHeader;      // tick count right before receiving the HTTP response headers for chunk [_chunkIndex]
    uint32                  _tickStartChunkResponseBody;        // tick count right before receiving the HTTP response body for chunk [_chunkIndex]
    uint32                  _tickEndChunkRequest;               // tick count right after the end of the HTTP responder body for chunk [_chunkIndex-1]
    uint32                  _tickEndChunkRequest2;              // tick count right after the end of the HTTP responder body for chunk [_chunkIndex-2]

    uint32                  _processedBytes;        //Number of bytes received for the current chunk when the request for the next chunk is sent
    uint32                  _remainingBytes;        //Number of bytes remained for the current chunk when the request for the next chunk is sent.
                                                    //The sum of the above two is the 'content-length' of the http response.
#endif
};

typedef std::vector<CSocketMbrChunk*> CSocketMbrChunkVector;

// ===============================================================================================================
// MBR reader - adaptive streamer from a manifest
// ===============================================================================================================

class CSocketMbr : public CSocketRaw, public IManifestUpdate
{
    friend class CSocketMbrChunk;
    friend class MbrTuneInfo;

public:
    CSocketMbr(IAVManager* avManager, uint32 pipeIdN, const CTuneRequest& tuneRequest, uint32 initialNetworkBitsPerSec, eSocketType socketType = eSocketType_Mbr);
    virtual ~CSocketMbr();

public:
    //ISocket interfaces
    __override bool         Connect(void);
    __override bool         Close(void);
    __override uint32       GetMeasuredNetworkBitsPerSec(void);
    __override bool         SetAudioLanguage(int pid);
    __override bool         SetSubtitleLanguage(int pid);

    __override bool         Command(const std::string& command, const std::vector<std::string>& args);

    //IManifestUpdate interfaces
    __override pkRESULT     SelectStream( _In_ IManifestStream* pStream );
    __override pkRESULT     DeselectStream( _In_ IManifestStream* pStream );

    //CSocketRaw interfaces
    __override int          Recv(CRawPacket& rawPacket);

    //IDiagsProvider interfaces
    __override void         DiagsReset(void);
    __override void         DiagsRetrieve(IDiagsEvent* diagsEvent);
    __override IDiagsEvent* DiagsRetrieve(void);

    bool                    GetNonVideoIsDelayed();
    void                    SetNonVideoIsDelayed(bool isDelay);

private:
    __override void         ReceiversOnConnected(void);

    //MBR extensions
    bool                    Prepare(void);
    bool                    ConnectStream(MBR::CMediaStreamDescription* pMSD, uint64 hnsPos, bool isTuning);
    void                    CloseStream(uint32 streamId);
    bool                    IsStreamConnected(uint32 streamId);

    uint64                  GetCurrentPlaybackTime(void);
    void                    QualityChanged(CSocketMbrChunk* socketMbrChunk);

    //ULONG_PTR is compiled into 32 bit uint on 32 bit platform,
    //and 64 bit uint on 64 bit platform.
    //http://msdn.microsoft.com/en-us/library/aa384255%28VS.85%29.aspx
    //
    //The real type of this parameter is indicated by the event type.
    //Depending on event type, we'll cast it differently.
    typedef ULONG_PTR NOTIFICATION_DATA_VARIANT_TYPE;
    void                    FireNotification(ReceiverNotificationType eType, NOTIFICATION_DATA_VARIANT_TYPE data = 0);

    CMbrManifest*           _pMbrManifest;
    AutoRefPtr<MBR::CChunkManifest>    _apChunkManifest;
    MBR::CHeuristicsMBR*    _pHeuristics;

    CSocketMbrChunkVector   _streams;

    //Tune parameter
    MbrTuneInfo             _tuneInfo;
    uint64                  _lastTime;

    bool                    _started;

    //Diags
    CMbrDiagnosticStats     _diags;
    COttDiscontinuityStats  _discontinuityStats;

    bool                    _isClosed;
    bool                    _isNonVideoDelayed;
    uint32                  _initialNetworkBitsPerSec;
};

// ===============================================================================================================
// ===============================================================================================================
