///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CAVEngineConfiguration.h"
#include "Trace.h"
#include <string>
#include <vector>
using namespace std;

// Configuration value sanity limits:

static const int kSanityLimit_HttpRedirect = 20;
static const int kSanityLimit_HttpTimeoutMax = 300;
static const int kSanityLimit_HttpTimeoutMin = 1;
static const int kSanityLimit_HttpReconnect = 10;

// ===============================================================================================================
// Global configuration parameters common to all components and paths
// ===============================================================================================================

CAVEngineConfiguration::CAVEngineConfiguration()
{
    //Our client ID
    ClientID = "";
    //Client version string
    ClientVersion = "";
    //Current build flavor
    ClientFlavor = "";

    //User agent string to be used for all HTTP/Web communications
    UserAgentString = "Microsoft IIS SSPK 1";

    //Maximum bitrate for HD streams
    MaxBitRateSD = 6000000;
    //Masimum bit rate for HD streams
    MaxBitRateHD = 12000000;

    //Multisocket (VOD/Timeshift) - block size in KB.
    VodBlockSizeKB = 16;
    //Multisocket (VOD/Timeshift) - maximum read size in KB.
    VodMaxReadSizeKB = 16;
    //Multisocket (VOD/Timeshift) - initial minimum data rate in Kb/s.
    VodInitialMinDataRateKbs = 20480;
    //Multisocket (VOD/Timeshift) - maximum number of sessions to use for an asset.
    VodMaxSessionCount = 4;

    //HTTP Proxy server host name (including optional port number after colon)
    HttpProxyHost = "";
    //HTTP maximum number of redirects
    HttpMaxRedirectCount = 10;
    //HTTP response timeout in seconds
    HttpResponseTimeout = 20;
    //HTTP initial receive timeout in seconds
    HttpInitialReceiveTimeout = 30;
    //HTTP subsequent receive timeout in seconds
    HttpSubsequentReceiveTimeout = 6;
    //HTTP maximum number of request reconnect attempts
    HttpMaxReconnectAttemptCount = 10;//modify for network retry function
}

//Process commands
bool CAVEngineConfiguration::Command(const string& command, const vector<string>& args)
{
    //Number of arguments
    uint32 numargs = (uint32)args.size();

    //Length of pause buffer in seconds
    if (command == "clientid")
    {
        if (numargs == 1)
        {
            ClientID = args[0];

            TRACE(("ClientId=%s", ClientID.c_str()));
        }
        return true;
    }

    //Client version string
    if (command == "clientversion")
    {
        if (numargs == 1)
        {
            ClientVersion = args[0];

            TRACE(("ClientVersion=%s", ClientVersion.c_str()));
        }
        return true;
    }

    //Current build falvor
    if (command == "clientflavor")
    {
        if (numargs == 1)
        {
            ClientFlavor = args[0];

            TRACE(("ClientFlavor=%s", ClientFlavor.c_str()));
        }
        return true;
    }

    //User agant string to be used for all HTTP/Web communications
    if (command == "useragentstring")
    {
        if (numargs > 0)
        {
            //Rebuild the user agent string since  it may have been split up
            //due to argument parsing
            UserAgentString = "";
            for (uint32 i = 0; i < numargs; i++)
            {
                UserAgentString += args[i];
                if (i < numargs-1)
                {
                    UserAgentString += " ";
                }
            }
            TRACE(("UserAgentString=%s", UserAgentString.c_str()));
        }
        return true;
    }

    //Maximum bitrate for SD streams
    if (command == "maxbitratesd")
    {
        if (numargs == 1)
        {
            MaxBitRateSD = atoi(args[0].c_str());

            TRACE(("MaxBitRateSD=%u", MaxBitRateSD));
        }
        return true;
    }

    //Maximum bit rate for HD streams
    if (command == "maxbitratehd")
    {
        if (numargs == 1)
        {
            MaxBitRateHD = atoi(args[0].c_str());

            TRACE(("MaxBitRateHD=%u", MaxBitRateHD));
        }
        return true;
    }

    //Multisocket (VOD/Timeshift) - block size in KB.
    if (command == "vodblocksizekb")
    {
        if (numargs == 1)
        {
            VodBlockSizeKB = (byte)atoi(args[0].c_str());

            TRACE(("VodBlockSizeKB=%u", VodBlockSizeKB));
        }
        return true;
    }

    //Multisocket (VOD/Timeshift) - maximum read size in KB.
    if (command == "vodmaxreadsizekb")
    {
        if (numargs == 1)
        {
            VodMaxReadSizeKB = (byte)atoi(args[0].c_str());

            TRACE(("VodMaxReadSizeKB=%u", VodMaxReadSizeKB));
        }
        return true;
    }

    //Multisocket (VOD/Timeshift) - initial minimum data rate in Kb/s.
    if (command == "vodinitialmindataratekbs")
    {
        if (numargs == 1)
        {
            VodInitialMinDataRateKbs = atoi(args[0].c_str());

            TRACE(("VodInitialMinDataRateKbs=%u", VodInitialMinDataRateKbs));
        }
        return true;
    }

    //Multisocket (VOD/Timeshift) - maximum number of sessions to use for an asset.
    if (command == "vodmaxsessioncount")
    {
        if (numargs == 1)
        {
            VodMaxSessionCount = (byte)atoi(args[0].c_str());

            TRACE(("VodMaxSessionCount=%u", VodMaxSessionCount));
        }
        return true;
    }

    //HTTP Proxy server host name (including optional port number after colon)
    if (command == "httpproxyhost")
    {
        if (numargs == 1)
        {
            HttpProxyHost = args[0];

            TRACE(("HttpProxyHost=%s", HttpProxyHost.c_str()));
        }
        return true;
    }

    //HTTP maximum number of redirects
    if (command == "httpmaxredirectcount")
    {
        if (numargs == 1)
        {
            int iArg = atoi(args[0].c_str());
            if (iArg < 0 || iArg > kSanityLimit_HttpRedirect)
            {
                TRACE(("IGNORED: HttpMaxRedirectCount=%d invalid", iArg));
            }
            else
            {
                HttpMaxRedirectCount = iArg;
                TRACE(("HttpMaxRedirectCount=%d", HttpMaxRedirectCount));
            }
        }
        return true;
    }

    //HTTP response timeout in seconds
    if (command == "httpresponsetimeout")
    {
        if (numargs == 1)
        {
            int iArg = atoi(args[0].c_str());
            if (iArg < kSanityLimit_HttpTimeoutMin || iArg > kSanityLimit_HttpTimeoutMax)
            {
                TRACE(("IGNORED: HttpResponseTimeout=%d invalid", iArg));
            }
            else
            {
                HttpResponseTimeout = iArg;
                TRACE(("HttpResponseTimeout=%d", HttpResponseTimeout));
            }
        }
        return true;
    }

    //HTTP initial receive timeout in seconds
    if (command == "httpinitialreceivetimeout")
    {
        if (numargs == 1)
        {
            int iArg = atoi(args[0].c_str());
            if (iArg < kSanityLimit_HttpTimeoutMin || iArg > kSanityLimit_HttpTimeoutMax)
            {
                TRACE(("IGNORED: HttpInitialReceiveTimeout=%d invalid", iArg));
            }
            else
            {
                HttpInitialReceiveTimeout = iArg;
                TRACE(("HttpInitialReceiveTimeout=%d", HttpInitialReceiveTimeout));
            }
        }
        return true;
    }

    //HTTP subsequent receive timeout in seconds
    if (command == "httpsubsequentreceivetimeout")
    {
        if (numargs == 1)
        {
            int iArg = atoi(args[0].c_str());
            if (iArg < kSanityLimit_HttpTimeoutMin || iArg > kSanityLimit_HttpTimeoutMax)
            {
                TRACE(("IGNORED: HttpSubsequentReceiveTimeout=%d invalid", iArg));
            }
            else
            {
                HttpSubsequentReceiveTimeout = iArg;
                TRACE(("HttpSubsequentReceiveTimeout=%d", HttpSubsequentReceiveTimeout));
            }
        }
        return true;
    }

    //HTTP maximum number of request reconnect attempts
    if (command == "httpmaxreconnectattemptcount")
    {
        if (numargs == 1)
        {
            int iArg = atoi(args[0].c_str());
            if (iArg < 0 || iArg > kSanityLimit_HttpReconnect)
            {
                TRACE(("IGNORED: HttpMaxReconnectAttemptCount=%d invalid", iArg));
            }
            else
            {
                HttpMaxReconnectAttemptCount = iArg;
                TRACE(("HttpMaxReconnectAttemptCount=%d", HttpMaxReconnectAttemptCount));
            }
        }
        return true;
    }

    return false;
}

// ===============================================================================================================
// ===============================================================================================================

CAVEngineConfiguration gAVEngineConfiguration;

// ===============================================================================================================
// ===============================================================================================================
