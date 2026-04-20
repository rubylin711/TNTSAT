///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>

// ===============================================================================================================
// Global configuration parameters common to all components and paths
// ===============================================================================================================

class CAVEngineConfiguration
{
public:
    //Constructor
    CAVEngineConfiguration();

    //Process commands
    bool        Command(const std::string& command, const std::vector<std::string>& args);

public:
    //Our client ID
    std::string ClientID;
    //Client version string
    std::string ClientVersion;
    //Current build falvor
    std::string ClientFlavor;
    //User agant string to be used for all HTTP/Web communications
    std::string UserAgentString;

    //Maximum bitrate for HD streams
    uint32      MaxBitRateSD;
    //Masimum bit rate for HD streams
    uint32      MaxBitRateHD;

    //Multisocket (VOD/Timeshift) - block size in KB.
    byte        VodBlockSizeKB;
    //Multisocket (VOD/Timeshift) - maximum read size in KB.
    byte        VodMaxReadSizeKB;
    //Multisocket (VOD/Timeshift) - initial minimum data rate in Kb/s.
    uint32      VodInitialMinDataRateKbs;
    //Multisocket (VOD/Timeshift) - maximum number of sessions to use for an asset.
    byte        VodMaxSessionCount;

    //HTTP Proxy server host name (including optional port number after colon)
    std::string HttpProxyHost;

    //HTTP maximum number of redirects
    int         HttpMaxRedirectCount;
    //HTTP response timeout in seconds
    int         HttpResponseTimeout;
    //HTTP initial receive timeout in seconds
    int         HttpInitialReceiveTimeout;
    //HTTP subsequent receive timeout in seconds
    int         HttpSubsequentReceiveTimeout;
    //HTTP maximum number of request reconnect attempts
    int         HttpMaxReconnectAttemptCount;
};

// ===============================================================================================================
// ===============================================================================================================

extern CAVEngineConfiguration gAVEngineConfiguration;

// ===============================================================================================================
// ===============================================================================================================
