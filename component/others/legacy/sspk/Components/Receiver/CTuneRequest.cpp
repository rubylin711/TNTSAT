///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CTuneRequest.h"
#include "CAVEngineConfiguration.h"
#include "CRecordingConfiguration.h"
#include "StringUtils.h"
#include "Trace.h"
#include <string>
#include <map>
using namespace std;

// ===============================================================================================================
// ===============================================================================================================

void CTuneRequest::Init(void)
{
    //Various arguments
    Args.clear();
    //Tune url
    TunerUrl = "";
    //Canonical form of orginal tuned url of service we are watching
    CanonicalUrl = "";
    //Protocol
    Protocol = "";
    //Path
    Path = "";
    //Query
    Query = "";
    //Host
    Host = "";
    //Port
    Port = "";
    //ServiceId for this service
    memset(&ServiceId, 0, sizeof(ServiceId));
    //Unique Ids assigned for this tune
    MediaTransportId = 0;
    UniqueId = 0;
    memset(&ContentId, 0, sizeof(ContentId));
    //Current channel number
    Channel = -1;
    //Overall bitrate expected for the stream
    BitRate = 0;
    //Position in the steam at which to start a tune
    Rap = MIN_TIME64;
    //Whether to take the floor or ceiling when requested to playback from a point in stream
    FloorIt = false;
    //Playback speed at which to start the tune and stream at
    Speed = 0;
    //Tune to live?
    TuneToLive = false;
}

bool CTuneRequest::ParseUrl(const std::wstring& tunerurl)
{
    return ParseUrl(WStr2Str(tunerurl));
}

bool CTuneRequest::ParseUrl(const string& tunerurl)
{
    //Initialize all tune variables
    Init();

    //Check if it is an empty tune url
    if (tunerurl.empty())
    {
        TRACE_ERROR(("Empty tune url"));
        return false;
    }

    //Save the tune url
    TunerUrl = tunerurl;

    //Protocol
    size_t protocol = tunerurl.find_first_of(":");
    if (protocol == string::npos)
    {
        TRACE_ERROR(("No protocol specified"));
        return false;
    }
    Protocol = tunerurl.substr(0, protocol);
    protocol++;

    //CanonicalUrl, Path, Query and Args
    size_t query = tunerurl.find_first_of("?");
    if (query == string::npos)
    {
        //Canonical form of orginal tuned url of service we are watching
        CanonicalUrl = tunerurl;
        //Get rest of the url
        Path = tunerurl.substr(protocol);
    }
    else
    {
        //Canonical form of orginal tuned url of service we are watching
        CanonicalUrl = tunerurl.substr(0, query);
        //Get rest of the url
        Path = tunerurl.substr(protocol, query-protocol);
        //Save query string
        Query = tunerurl.substr(query + 1);
        //Prepare arguments
        nameValue(Query, Args);
    }

    //Path, Host, Port and ServiceId
    if ((Path.find("//") == 0))
    {
        if (Protocol == URL_PROTOCOL_FILE)
        {
            Path = Path.substr(2);
        }
        else
        if (Protocol != URL_PROTOCOL_DOWNLOAD)
        {
            size_t path = Path.find_first_of("/", 2);
            if (path != string::npos)
            {
                Host = Path.substr(2, path-2);
                Path = Path.substr(path+1);
            }
            else
            {
                Host = Path.substr(2);
                Path = "";
            }

            size_t port0 = Host.find_first_of(":");
            if (port0 != string::npos)
            {
                Port = Host.substr(port0 + 1);
                Host = Host.substr(0, port0);

                size_t port1 = Port.find_first_of(":");
                if (port1 != string::npos)
                {
                    //Convert the ServiceId to GUID
                    string serviceId = Port.substr(port1 + 1);
                    Port = Port.substr(0, port1);

                    if (Protocol == URL_PROTOCOL_UDP)
                    {
                        ServiceId = GuidFromString(serviceId.c_str());
                    }
                }
            }
        }
    }

    //Unique Ids assigned for this tune
    MediaTransportId = GetIntFromHex(TUNE_REQUEST_MEDIATRANSPORTID);
    UniqueId = GetIntFromHex(TUNE_REQUEST_UNIQUEID);
    ContentId = GetGUID(TUNE_REQUEST_CONTENTID);

    //Read the current channel number
    Channel = GetInt(TUNE_REQUEST_CHANNElNO, -1);

    //Parse overall bitrate of the stream
    BitRate = GetInt(TUNE_REQUEST_BITRATE);
    if (!BitRate)
    {
        BitRate = GetInt(TUNE_REQUEST_BITRATE_ALT);
    }
    //If no bitrate argument provided, use max bit rate
    if (!BitRate)
    {
        BitRate = IsWms() ? gAVEngineConfiguration.MaxBitRateSD : gAVEngineConfiguration.MaxBitRateHD;
    }

    //Position in the steam at which to start a tune
    Rap = GetUInt64(TUNE_REQUEST_RAP, MIN_TIME64);
    //Override for RAP point - used for splicing
    Rap = GetUInt64(TUNE_REQUEST_OVERRIDE_RAP, Rap);

    //Whether to look for floor or ceiling index
    FloorIt = GetBool(TUNE_REQUEST_FLOORIT);

    //Trick mode
    string& speed_string = Args[TUNE_REQUEST_SPEED];
    //Backward compatibility with 1.6.4 and older clients
    if (speed_string.empty())
    {
        speed_string = Args[TUNE_REQUEST_TRICKSPEED];
    }
    Speed = speed_string.empty() ? 0 : toInt(speed_string);

    //Tune to live?
    TuneToLive = GetBool(TUNE_REQUEST_TUNETOLIVE);

    return true;
}

std::string CTuneRequest::GetArg(const std::string& p)
{
    std::map<std::string,std::string>::iterator it = Args.find(p);
    if (it != Args.end())
    {
        return it->second;
    }
    return "";
}

void CTuneRequest::SetArg(const std::string& p, const std::string& v)
{
    Args[p] = v;
}

int CTuneRequest::GetIntFromHex(const std::string& p, int iDefault/* = 0*/)
{
    std::map<std::string,std::string>::iterator it = Args.find(p);
    if (it != Args.end())
    {
        std::string& s = it->second;
        if (!s.empty())
        {
            return toIntFromHex(s);
        }
    }
    return iDefault;
}

int CTuneRequest::GetInt(const std::string& p, int iDefault/* = 0*/)
{
    std::map<std::string,std::string>::iterator it = Args.find(p);
    if (it != Args.end())
    {
        std::string& s = it->second;
        if (!s.empty())
        {
            return toInt(s);
        }
    }
    return iDefault;
}

void CTuneRequest::SetInt(const std::string& p, int i)
{
    Args[p] = toString(i);
}

float CTuneRequest::GetFloat(const std::string& p, float iDefault/* = 0.0f*/)
{
    std::map<std::string,std::string>::iterator it = Args.find(p);
    if (it != Args.end())
    {
        std::string& s = it->second;
        if (!s.empty())
        {
            return toFloat(s);
        }
    }
    return iDefault;
}

uint64 CTuneRequest::GetUInt64(const std::string& p, uint64 u64Default/* = 0*/)
{
    std::map<std::string,std::string>::iterator it = Args.find(p);
    if (it != Args.end())
    {
        std::string& s = it->second;
        if (!s.empty())
        {
            return toUInt64(s);
        }
    }
    return u64Default;
}

void CTuneRequest::SetUInt64(const std::string& p, uint64 i, bool hex/* = false*/)
{
    Args[p] = toString64(i, hex);
}

bool CTuneRequest::GetBool(const std::string& p, bool bDefault/* = false*/)
{
    std::map<std::string,std::string>::iterator it = Args.find(p);
    if (it != Args.end())
    {
        std::string& s = it->second;
        if (!s.empty())
        {
            return _stricmp(toLower(s).c_str(), "true") == 0;
        }
    }
    return bDefault;
}

void CTuneRequest::SetBool(const std::string& p, bool b)
{
    Args[p] = b ? "true" : "false";
}

GUID CTuneRequest::GetGUID(const std::string& p)
{
    std::map<std::string,std::string>::iterator it = Args.find(p);
    if (it != Args.end())
    {
        std::string& s = it->second;
        if (!s.empty())
        {
            return GuidFromString(s.c_str());
        }
    }
    return GUID_NULL;
}

uint32 CTuneRequest::NetworkBandwidthUsage(void) const
{
    //Returns network bandwidth usage (bitrate over network)
    return (IsLive() || IsVod() || IsHttp() || IsSbp() || IsTimeShift() || IsWms() || IsMbr() || IsMpegTs())? BitRate : 0;
}

string CTuneRequest::TunerStatusUrl(string& url)
{
    string tunerurl;
    if (url.length() > 0)
    {
        if (startsWith(url, URL_PROTOCOL_VOD))
        {
            tunerurl = URL_PROTOCOL_VOD;
        }
        else
        {
            size_t endpoint = url.find('?');
            tunerurl = endpoint == string::npos ? url : url.substr(0, endpoint);
        }
        tunerurl = escapeXml(tunerurl, true);
    }
    return tunerurl;
}


// ===============================================================================================================
// ===============================================================================================================

