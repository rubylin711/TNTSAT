///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AutoLock.h"
#include "CTuneRequest.h"
#include "CSocketDefinitions.h"
#include "SmoothErrorDefinitions.h"
#include <vector>

// ===============================================================================================================
// IStreamer interface
// ===============================================================================================================

class IStreamer
{
public:
    // Constructor
    IStreamer()
        : _socketConnected( true )
        , _socketError( eSocketErrorNone )
        , _socketPKResult( pkS_OK )
        , _socketHttpResponse( 0 )
    {
    }

    // Destructor
    virtual ~IStreamer()
    {
        ASSERT(_socketConnected == false);
    }

    // Connect streamer to the given URL.
    // Upon return the url parameter contains the final URL after any redirections
    virtual bool Connect( _Inout_ std::string* pUrl )
    {
        if (!_socketConnected)
        {
            _socketError = eSocketErrorAlreadyClosed;
            return false;
        }
        return true;
    }

    // Stop current tune and start cleaning up streamer
    virtual bool Close(void)
    {
        _socketConnected = false;
        return true;
    }

    // Receive data from streamer
    // Note: Depending on the derived class, where data is segmented (such as http), 
    // "pLastContentData" is set true when the data being returned is the last in the segment.
    virtual int Recv(
                    _Out_cap_(dstCap) byte* dst,
                    _In_              int dstCap,
                    _Out_             bool* pLastContentData,
                    _In_              int timeout = 0 )
    {
        ASSERT(false);
        return eRecvErrorUnknown;
    }

    // Is streamer currently connected
    virtual bool IsConnected(void) const
    {
        return _socketConnected;
    }

    // Get streamer specific error
    virtual eSocketError GetSocketError(void) const
    {
        return _socketError;
    }

    // Get response code
    virtual int GetHttpResponse(void) const
    {
        return _socketHttpResponse;
    }

    // Get PKResult code
    virtual int GetPKResult(void) const
    {
        return _socketPKResult;
    }

    // Generic interface to send custom commands
    virtual bool Command(
                    _In_ const std::string& command,
                    _In_ const std::vector<std::string>& args )
    {
        return false;
    }

protected:

    // A generic lock used by various streamer
    mutable Lockable    _socketLock;

    // Whether this streamer is currently connected
    bool                _socketConnected;
    // streamer specific error
    eSocketError        _socketError;
    // Get pkResult if any
    pkRESULT            _socketPKResult;
    // Get http response if any
    int                 _socketHttpResponse;
};

// ===============================================================================================================
// ===============================================================================================================
