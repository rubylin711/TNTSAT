///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ISmoothTransport.h"
#include "SSPKTimeSpan.h"

using namespace SSPK;

typedef struct
{
    /// <summary>
    /// Is this live smooth streaming?
    /// </summary>
    bool IsLive;
    /// <summary>
    /// Max bitrate available for this stream
    /// This includes maximum bitrate from different quality levels of all stream types
    /// plus a delta and is used for stream management perspective only.
    /// </summary>
    uint32 Bitrate;
    /// <summary>
    /// Bitrate for video stream with lowest quality
    /// </summary>
    uint32 MinVideoBitrate;
    /// <summary>
    /// Bitrate for video stream with highest quality
    /// </summary>
    uint32 MaxVideoBitrate;
    /// <summary>
    /// Type of encoding used for thumbnails
    /// </summary>
    uint32 FourCC;
    /// <summary>
    /// Session Id as was received from the last call to manifest retrieval
    /// </summary>
    GUID   SessionId;
    /// <summary>
    /// Service/resource id for this asset
    /// </summary>
    GUID   ResourceId;
    /// <summary>
    /// Start time of the stream
    /// </summary>
    TimeSpan_NTP StartTime;
    /// <summary>
    /// End time of the stream
    /// </summary>
    TimeSpan_NTP EndTime;
} TunePrepareResult;

// ===============================================================================================================
// ===============================================================================================================

class ITunePrepare
{
public:
    ITunePrepare() 
        : _pManifestCallback(NULL)
        , _refCount(1) 
        , _token(0) 
        {}
        
    virtual ~ITunePrepare() {}

    //Do whatever prepare task needed.
    virtual bool Prepare(void) = 0;
    //Get result of 'Prepare'
    virtual void GetPrepareResult(TunePrepareResult* pResult) = 0;
    //Cancel the ongoing 'Prepare'
    virtual void CancelPrepare(void) = 0;
    //Set playback range
    virtual pkRESULT RequestPlaybackRange( _In_ ISetPlaybackRangeCallback* pCallback, 
                                           _In_ TimeSpan_NTP minTime, 
                                           _In_ TimeSpan_NTP maxTime ) = 0;

    //Ref counting api
    LONG AddRef()
    {
        return( InterlockedIncrement(&_refCount) );
    }

    LONG Release()
    {
        LONG c = InterlockedDecrement(&_refCount);
        if( c == 0 )
        {
            delete this;
        }
        return( c );
    }

    //Token
    uint32 GetToken() { return _token; }
    void SetToken(uint32 token) { _token = token; }
    void SetManifestCallback(_In_opt_ IManifestReadyCallback* pCallback = NULL) { _pManifestCallback = pCallback; }

protected:
    // The callback for mainfest ready
    IManifestReadyCallback* _pManifestCallback;

private:
    //Ref count
    LONG _refCount;
    //A token that's attached to the object.
    uint32 _token;
};

// ===============================================================================================================
// ===============================================================================================================

class CTuneRequest;

class ITunePrepareFactory
{
public:
    //Destructor
    virtual ~ITunePrepareFactory() {}
    virtual uint32        CreateTunePrepareObject(uint32 pipeIdN, const char* tunerurl) = 0;
    virtual uint32        CreateTunePrepareObject(uint32 pipeIdN, CTuneRequest& tuneRequest) = 0;
    virtual ITunePrepare* GetTunePrepareInterface(uint32 token, bool addRef = true) = 0;
    virtual void          ReleaseTunePrepare(uint32 token) = 0;
};

// ===============================================================================================================
// ===============================================================================================================
