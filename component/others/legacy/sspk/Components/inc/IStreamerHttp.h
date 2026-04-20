///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

/*
 * IStreamerHttp.h
 */
#pragma once

#include "IStreamer.h"
#include <string>
#include <map>

// ===============================================================================================================
// Streamer to read from Http server
// ===============================================================================================================

class IStreamerHttp : public IStreamer
{
protected:
    IStreamerHttp() {} // abstract interface clase

public:
    virtual ~IStreamerHttp() {}

public:
    //Factory for the implementation
    static IStreamerHttp* CreateStreamerHttp();

    enum ERedirect
    {
        eRedirectDisable,
        eRedirectEnable
    };

    //The connect function that supports http-redirect.
    //Upon return the url param contains the final URL, including redirections
    virtual bool    Connect( _Inout_ std::string* pUrl, _In_ const std::string& extraHeader, _Out_ std::string* pResponse, ERedirect eRedirect ) = 0;

    //Send a new request on a connected socket
    virtual bool    SendHttpRequest( _In_ const CTuneRequest& tuneRequest, _In_ const std::string& extraHeader) = 0;

    //Receive response for the request sent earlier
    virtual bool    RecvHttpResponse( _Out_opt_ std::string* pResponse ) = 0;

    //Send request on connected socket and receive response
    virtual bool    HttpRequestResponse( _In_ const CTuneRequest& tuneRequest, _In_ const std::string& extraHeader, _Out_ std::string* pResponse ) = 0;

    //Return a given response header
    virtual bool    GetResponseHeader( _In_ const char* pszHeader, _Out_ std::string* pValue ) = 0;

    //Return the session id response header
    virtual bool    GetResponseSessionIdHeader( _Out_ std::string* pValue) = 0;

    //Set window timeout (amount of time streamer will wait before sending CR+LF w/ window update to ping server)
    virtual void    SetWindowTimeout(uint32 timeout) = 0;

    //Set TCP receive buffer size
    virtual void    SetTcpRecvBuffSize(uint32 size) = 0;

    //Get TCP receive buffer size
    virtual uint32  GetTcpWindowSize(void) = 0;
};

// ===============================================================================================================
// ===============================================================================================================
