///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <map>

// ===============================================================================================================
// CTuneRequest
//
// Encapsulate parameters needed for different types of tunes.
// Responsible for parsing the incoming URL and provide helper methods and fields for tuning
// ===============================================================================================================

#define TUNE_REQUEST_MEDIATRANSPORTID       "mtid"                // uint32
#define TUNE_REQUEST_UNIQUEID               "uid"                 // uint32
#define TUNE_REQUEST_CONTENTID              "cid"                 // GUID

#define TUNE_REQUEST_SOURCEURL              "src"                 // string

#define TUNE_REQUEST_QUALITYLEVEL           "q"                   // int

#define TUNE_REQUEST_CHANNElNO              "ch"                  // int

#define TUNE_REQUEST_BITRATE                "r"                   // int
#define TUNE_REQUEST_BITRATE_ALT            "rate"                // int

#define TUNE_REQUEST_RAP                    "rap"                 // uint64
#define TUNE_REQUEST_OVERRIDE_RAP           "orap"                // uint64
#define TUNE_REQUEST_FLOORIT                "floorit"             // bool
#define TUNE_REQUEST_SPEED                  "speed"               // int
#define TUNE_REQUEST_TRICKSPEED             "trk"                 // int

#define TUNE_REQUEST_LOCALHOST              "n"                   // string

#define TUNE_REQUEST_PROGRAMID              "p"                   // int

#define TUNE_REQUEST_MUXRATE                "b"                   // int

#define TUNE_REQUEST_JOINMULTICAST          "jm"                  // bool
#define TUNE_REQUEST_SBRECVBUFSIZE          "sbrbs"               // int
#define TUNE_REQUEST_DISABLEBURST           "disableBurst"        // bool
#define TUNE_REQUEST_DISABLEBURSTSIDATA     "disable_burst"       // bool

#define TUNE_REQUEST_RECORDINGID            "rid"                 // GUID

#define TUNE_REQUEST_STARTTIME              "st"                  // uint64
#define TUNE_REQUEST_ENDTIME                "et"                  // uint64

#define TUNE_REQUEST_SCAN                   "scan"                // int
#define TUNE_REQUEST_SCANTOTIME             "end"                 // uint64

#define TUNE_REQUEST_FILEPOSITION           "filepos"             // uint64
#define TUNE_REQUEST_PAUSELIVE              "pauselive"           // bool

#define TUNE_REQUEST_IFRAMEONLY             "iframeonly"          // bool
#define TUNE_REQUEST_ISIMMEDIATEMODE        "isimmediate"         // bool

#define TUNE_REQUEST_VODDOWNLOADMODE        "dl"                  // bool

#define TUNE_REQUEST_ISTS                   "ists"                // bool
#define TUNE_REQUEST_ISFRAMED               "isframed"            // bool

#define TUNE_REQUEST_BLACKOUTTABLEVERSION   "bov"                 // int
#define TUNE_REQUEST_BLACKOUTSERVICEIDS     "bos"                 // string
#define TUNE_REQUEST_BLACKOUTONLY           "bons"                // bool

#define TUNE_REQUEST_ISSAP                  "sap"                 // bool

#define TUNE_REQUEST_ISSTREAMAVAILS         "isstreamavails"      // bool
#define TUNE_REQUEST_ISADSTREAM             "isad"                // bool
#define TUNE_REQUEST_MAPTIMEVALID           "maptimevalid"        // bool
#define TUNE_REQUEST_MAPFROMPCR             "mapfrompcr"          // uint64
#define TUNE_REQUEST_MAPFROMNTP             "mapfromntp"          // uint64
#define TUNE_REQUEST_MAPTOPCR               "maptopcr"            // uint64
#define TUNE_REQUEST_MAPTONTP               "maptontp"            // uint64

#define TUNE_REQUEST_TUNEPREPAREHANDLE      "tune-prepare-handle" // int

#define TUNE_REQUEST_SESSIONID              "sessionid"           // string

#define TUNE_REQUEST_CLOCKTIMINGMODE        "ctm"                 // uint32 (enum)

#define TUNE_REQUEST_TUNETOLIVE             "tunetolive"          // bool

// ===============================================================================================================
// ===============================================================================================================

#define URL_PROTOCOL_VOD                    "vod"
#define URL_PROTOCOL_HTTP                   "http"
#define URL_PROTOCOL_DVRFS                  "dvrfs"
#define URL_PROTOCOL_FILE                   "file"
#define URL_PROTOCOL_UDP                    "udp"
#define URL_PROTOCOL_SBP                    "bp"
#define URL_PROTOCOL_DVB                    "dvb"
#define URL_PROTOCOL_DOWNLOAD               "dl"
#define URL_PROTOCOL_MPEGTS                 "mpegts"
#define URL_PROTOCOL_TS                     "ts"
#define URL_PROTOCOL_WMS                    "wms"
#define URL_PROTOCOL_MP3                    "mp3"
#define URL_PROTOCOL_TIMESHIFT              "timeshift"
#define URL_PROTOCOL_MP4                    "mp4"
#define URL_PROTOCOL_MULTICAST_DOWNLOAD     "mcdl"
#define URL_PROTOCOL_MBR                    "mbr"

// ===============================================================================================================
// ===============================================================================================================

class CTuneRequest
{
public:
    CTuneRequest() { Init(); }
    ~CTuneRequest() {}

    //Initialize the tune request
    void                Init(void);

    //Parse given url and populate all necessary fields
    bool                ParseUrl(const std::wstring& tunerurl);
    bool                ParseUrl(const std::string& tunerurl);

    //Access arguments
    std::string         GetArg(const std::string& p);
    void                SetArg(const std::string& p, const std::string& v);

    int                 GetIntFromHex(const std::string& p, int iDefault = 0);
    int                 GetInt(const std::string& p, int iDefault = 0);
    void                SetInt(const std::string& p, int i);

    uint64              GetUInt64(const std::string& p, uint64 u64Default = 0);
    void                SetUInt64(const std::string& p, uint64 i, bool hex = false);

    bool                GetBool(const std::string& p, bool bDefault = false);
    void                SetBool(const std::string& p, bool b);

    GUID                GetGUID(const std::string& p);
    float               GetFloat(const std::string& p, float iDefault = 0.0f);

    //Network bandwidth usage
    uint32              NetworkBandwidthUsage(void) const;

    //Returns a URL to be used for tune status- called by GetStatus for Receiver.
    static std::string  TunerStatusUrl(std::string& url);

    //Get SSRC info
    int                 GetSSRCs(uint32 ssrcsToExpect[], uint32 ssrcBitRate[]);

    //Queries on type
    bool                IsLive(void)              const { return Protocol == URL_PROTOCOL_UDP; }
    bool                IsDvr(void)               const { return Protocol == URL_PROTOCOL_DVRFS; }
    bool                IsVod(void)               const { return Protocol == URL_PROTOCOL_VOD || Protocol == URL_PROTOCOL_DOWNLOAD; }
    bool                IsHttp(void)              const { return Protocol == URL_PROTOCOL_HTTP; }
    bool                IsFile(void)              const { return Protocol == URL_PROTOCOL_FILE; }
    bool                IsSbp(void)               const { return Protocol == URL_PROTOCOL_SBP; }
    bool                IsDvb(void)               const { return Protocol == URL_PROTOCOL_DVB; }
    bool                IsWms(void)               const { return Protocol == URL_PROTOCOL_WMS; }
    bool                IsMp3(void)               const { return Protocol == URL_PROTOCOL_MP3; }
    bool                IsMpegTs(void)            const { return Protocol == URL_PROTOCOL_MPEGTS; }
    bool                IsTs(void)                const { return Protocol == URL_PROTOCOL_TS; }
    bool                IsTimeShift(void)         const { return Protocol == URL_PROTOCOL_TIMESHIFT;}
    bool                IsMp4(void)               const { return Protocol == URL_PROTOCOL_MP4; }
    bool                IsMbr(void)               const { return Protocol == URL_PROTOCOL_MBR; }
    bool                IsMulticastDownload(void) const { return Protocol == URL_PROTOCOL_MULTICAST_DOWNLOAD; }

private:
    //Various arguments
    std::map<std::string,std::string> Args;

public:
    //Tune url
    std::string         TunerUrl;
    //Canonical url of service being watched
    std::string         CanonicalUrl;
    //Protocol
    std::string         Protocol;
    //Path
    std::string         Path;
    //Query
    std::string         Query;
    //Host
    std::string         Host;
    //Port
    std::string         Port;
    //ServiceId for this service
    GUID                ServiceId;
    //Unique Ids associated with this tune
    uint32              MediaTransportId;
    uint32              UniqueId;
    GUID                ContentId;
    //Current channel number
    int                 Channel;
    //Overall bitrate expected for the stream
    uint32              BitRate;
    //Position in the steam at which to start a tune
    uint64              Rap;
    //Whether to take the floor or ceiling when requested to playback from a point in stream
    bool                FloorIt;
    //Playback speed at which to start the tune and stream at
    int                 Speed;
    //Tune to live?
    bool                TuneToLive;
};

// ===============================================================================================================
// ===============================================================================================================
