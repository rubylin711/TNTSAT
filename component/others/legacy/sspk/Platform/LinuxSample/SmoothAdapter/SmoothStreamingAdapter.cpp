///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

// SmoothSample.cpp : Defines the entry point for the console application.
//

#include "pkPAL.h"
#include "pkSockets.h"
#include "pkDnsCache.h"
#include "pkExecutive.h"
#include "pkTestFramework.h"
#include "TLCommon.h"
#include "ISmoothTransport.h"
#include "IAVManager.h"
#include "IDiagsManager.h"
#include "DRMAcquireLicense.h"
#include "IXDrm.h"
#include "AutoLock.h"
#include "strsafe.h"
#include "IDrmManager.h"


#include "IPTVDecoderHal.h"

#ifdef MSPK_USE_TELNET_CONSOLE
#include "TelnetConsole.h"
#else
#include <iostream>
#endif //MSPK_USE_TELNET_CONSOLE
#include <string>
#include <deque>
#include <map>

#define SS_ADAPTER_USED
#ifdef SS_ADAPTER_USED
//#define SS_ADAPTER_USED_TEST
#define SS_ADAPTER_USED_LIB
#include <platPrivate.h>

#include "SmoothStreamingAdapter.h"

#ifdef SS_ADAPTER_USED_TEST
extern "C" {
static SSAdapter_Callback_t callback;
static SS_ADAPTER_ERROR_STATUS SSAdapter_Test_Status_Callback(SS_ADAPTER_EVENT_STATUS_Type type, int32_t param);
static SS_ADAPTER_ERROR_STATUS SSAdapter_Test_Error_Callback(SS_ADAPTER_EVENT_ERROR_Type type, int32_t param);
SS_ADAPTER_ERROR_STATUS SSAdapter_Test(std::string url);
}
#endif //SS_ADAPTER_USED_TEST
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))


extern "C" {
int FourCCTo2Codec(uint32_t fourCC);
}

#endif//SS_ADAPTER_USED

using namespace std;
using namespace SSPK;

static const int c_MAX_INT_CHAR_STRING_SIZE = 16;

static bool g_IsAbandoned = false;

static const char* g_szHTTPProxyHost = NULL;
static const char* g_szAudioHalBufferingThreshold = NULL;
static const char* g_szLivePlaybackOffsetSecs = NULL;
static const char* g_szLiveBackOffSecs = NULL;
static const char* g_szLiveMinTimeBufferSecs = NULL;
static const char* g_szUserAgentString = NULL;
static const char* g_szHttpResponseTimeout = NULL;
static const char* g_szHttpInitialReceiveTimeout = NULL;
static const char* g_szHttpSubsequentReceiveTimeout = NULL;

static const char* g_szDrmServerUrl = NULL;
static const char* g_szLicenseKeyId = NULL;
static const char* g_szDrmCustomData = NULL;
static const char* g_szDrmLogFilePath = NULL;
static const char* g_szChunkListMaxSize = NULL;

// Note: c_FIFO_LIMIT_NOT_SET must be zero because that is the value
//       set by TLC_GetCmdLineArgInt when no switch argument is found
static const int c_FIFO_LIMIT_NOT_SET = 0;
static const int c_MIN_FIFO_LIMIT_MS = 500;
static const int c_MAX_FIFO_LIMIT_MS = 100000;
static int g_iEsFifoLimitNormalPlay = c_FIFO_LIMIT_NOT_SET;
static int g_iEsFifoLimitTrickPlay = c_FIFO_LIMIT_NOT_SET;

static const int c_MIN_BITRATE_KBPS = 100; // 100 kilobits/sec
static const int c_MAX_BITRATE_KBPS = 100000; // 100 megabits/sec

// Note: c_VALUE_NOT_SET must be zero because that is the value
//       set by TLC_GetCmdLineArgInt when no switch argument is found
static const int c_VALUE_NOT_SET = 0;
static int g_iMaxBitrateKbps = c_VALUE_NOT_SET;
static int g_iMinBitrateKbps = c_VALUE_NOT_SET;
static int g_iMinRangeDeltaSec = c_VALUE_NOT_SET;
static bool g_bRestrictTracks = true;
static bool g_bPrintText = false;
static bool g_bPlayAd = false;

static const char* g_szForcedQualityLevel = NULL;

static const char* s_CmdList = "Unknown command: %s; Available:\n"
    " play, pause, rst [optional: url], ff, rew, sf [optional: sec], sb [optional: sec],\n"
    " live, begin, ct, nextaudio, close, exit, quit, seltracks [minKbps] [maxKbps]\n"
    " togstream [streamname], seek [ @sec], open [optional: url], adjminrange [+/-sec],\n"
    ;

static const int32_t kSkipForwardSecs = 30;
static const int32_t kSkipBackSecs = 7;
static const int32_t kSkipToLiveSecs = 86400; // 24 hours
static const int32_t kSkipToBeginSecs = 86400; // 24 hours


static bool  load_media_succuss = false;//add for during loadmeida stop by user to exit

#define SPEEDS_COUNT 7
#define SPEED_ZERO 3
static float speeds[SPEEDS_COUNT] = {-15.0, -60.0, -300.0, 0.0, 15.0, 60.0, 300.0};
static int speedidx = SPEED_ZERO;
static bool shouldAutoPlay = false;
static int32_t isLiveUrl = 0;
#ifdef MSPK_USE_TELNET_CONSOLE
#define TELNET_CONSOLE_PORT        2323
#endif

#ifdef SS_ADAPTER_USED
ss_adapter_film_info_t g_ss_adapter_film_info;
ss_adapter_track_lang_t g_ss_adapter_audio_info[SS_ADAPTER_MAX_A_STREAMS] = {0};
ss_adapter_subt_t       g_ss_adapter_subt_info;
int32_t                 g_ss_adapter_aud_lang_cnt = 0;
#endif
//Conversion between 10MHz and 32.32FP units
inline uint64_t
NTP_UINT64TO10MHZ(uint64_t x) { return   ((((x) >> 32) * 10000000) + ((((x) & 0xFFFFFFFF) * 10000000) >> 32)); }

inline TimeSpan_NTP NTP_FROM_SECONDS( int64_t v )
{
    return( TimeSpan_NTP::ConvertFrom( TimeSpan_s::FromTicks( v ) ) );
}

// ========================================================================
// CEventHelper Declaration
// ========================================================================
class CEventHelper
{
public:
    enum EResetMode { eResetModeAuto, eResetModeManual };
    enum EWaitResult { eWaitAbandoned, eWaitSignaled, eWaitTimeout, eWaitFailed };

    CEventHelper( EResetMode eResetMode, bool bInitiallySignaled = false );
    ~CEventHelper();

    bool Set();
    bool Reset();
    EWaitResult Wait( uint32_t dwMsTimeout = INFINITE );

private:
    pkHANDLE    m_Event;
};


#define SMOOTH_TRACE_ENABLE

#if defined(SMOOTH_TRACE_ENABLE)
#define SMOOTH_MSG(format, args...) TF_Printf(format, ##args)
#else
#define SMOOTH_MSG(format, args... )   TF_Printf_NONE(format, ##args)
#endif

#define SMOOTH_ERR_MSG(format, args...) TF_Printf(format, ##args)

//#define SMOOTH_ADAPTER_TRACE_ENABLE

#if defined(SMOOTH_ADAPTER_TRACE_ENABLE)
#define SMOOTH_ADAPTER_MSG(format, args...) TF_Printf(format, ##args)
#else
#define SMOOTH_ADAPTER_MSG(format, args... )   TF_Printf_NONE(format, ##args)
#endif

#define SMOOTH_ADAPTER_ERR_MSG(format, args...) TF_Printf(format, ##args)

// ========================================================================
// SmoothTransportInfo Declaration
// ========================================================================
class SmoothTransportWorker;
class SmoothTransportInfo : public IManifestReadyCallback
                     , public IStreamsSelectedCallback
                     , public ISmoothTransportErrorSink
                     , public ISmoothTransportStatusSink
                     , public ISetPlaybackRangeCallback
                     , public IFragmentCallback
{
public:
    SmoothTransportInfo();
    SmoothTransportInfo(ISmoothTransport* pISmoothTransport);
    ~SmoothTransportInfo();

    void Initialize(SmoothTransportWorker* pSmoothTransportWorker);

    // 
    // Interface implementations
    // 
    __override void StreamSelectedCallback(_In_ StreamSelectedEventArgs* pEventArgs);
    __override void ManifestReadyCallback(IManifest* pManifest, pkRESULT manifestResult);
    __override void ErrorCallback(SmoothTransportError& errorInfo);
    __override void StatusCallback(SmoothTransportStatus& status);
    __override void SetPlaybackRangeCallback(IManifest* pManifest, pkRESULT result);
    __override pkRESULT OnFragmentData(
                        _In_ pkRESULT hrResult,
                        _In_ CHUNK_INFO* pChunkInfo,
                        _In_opt_ IRefBuffer* pBuffer,
                        _In_ bool fFinalBuffer,
                        _In_ size_t cbTotalLength );
    // 
    // Implementation
    // 
    bool IsLive();
    TimeSpan_NTP GetStartTime();
    void SetPlayAtPos(TimeSpan_NTP time);
    void Reset();
    IManifest* GetManifest();
    float GetSpeed();
    int64_t GetTimeScale() const;

private:
    static const char* GetErrorName( ESmoothTransportError error);
    static const char* GetTunerStateName( ESmoothTransportState SmoothTransportState);
    static const char* GetStatusUpdateName( ESmoothTransportStatusUpdate SmoothTransportStatusUpdate);

    void SetManifest(IManifest* pManifest);
    void UpdateStartEndTime(TimeSpan_NTP startTime, TimeSpan_NTP endTime);
    void SetLiveEnded();
    TimeSpan_NTP GetPlayAtPos();
    void UpdateSpeed(float newSpeed);

private:
    static const int32_t c_iMaxPrintOutByteCount = 80;
    CEventHelper m_streamSelectedEvent;
    ISmoothTransport* m_pISmoothTransport;
    SmoothTransportWorker* m_pSmoothTransportWorker;
        
    Lockable m_Lock;
    bool m_liveEnded;
    TimeSpan_NTP m_startTime;
    TimeSpan_NTP m_endTime;
    TimeSpan_NTP m_playAtTime;
    float m_currentSpeed;
    AutoRefPtr<IManifest> m_apManifest;
};
// ========================================================================
// SmoothTransportWorker Declaration
// ========================================================================
class SmoothTransportWorker
{
public:
    SmoothTransportWorker(ISmoothTransport* pISmoothTransport, 
                          SmoothTransportInfo *pSTInfo);

    ~SmoothTransportWorker();

    enum ETransportCommand
    {
        eTransportCommand_Play        = 0,
        eTransportCommand_Pause,
        eTransportCommand_Close,
        eTransportCommand_Open,
        eTransportCommand_Skip,
        eTransportCommand_Seek,
        eTransportCommand_SetPlaybackRange,
        eTransportCommand_FragmentDataDone,
        eTransportCommand_FragmentNoMoreItems,
        eTransportCommand_AddAd,
        eTransportCommand_UpdateState,
    };

    enum ETransportState
    {
        eTransportState_Rendering   = 0,
        eTransportState_MediaEnded,
        eTransportState_MediaError,
        eTransportState_Opened,
    };

    pkRESULT Initialize();
    void ShutDown();
    void Execute();
    void RequestSmoothTransportCommand(ETransportCommand cmd, float speed = 1.0, int32_t sec = 0, string str ="", int64_t val64 = 0 );
    void SetTextStream(IManifestStream* pTextStream);
    bool IsPlayingAd();

private:
    static const int32_t c_iMaxDownloadByteCount = 10 * 1024;
    static const int64_t c_iSparseFragmentLookAheadSec = 5;
    static const uint32_t c_iWorkerThreadPollPeriodMSec = 1000;

    struct SmoothTransportCommand
    {
        ETransportCommand cmd;
        float speed;
        int32_t sec;
        string str;
        int64_t val64;
    };

    struct AdInfo
    {
        AdInfo()
            : played ( false )
        {
        }

        AdInfo(string strUrl, int32_t iDurationSec)
            : played ( false )
        {
            url = strUrl;
            duration = TimeSpan_NTP::ConvertFrom(TimeSpan_s::FromTicks(iDurationSec));
        }

        string url;
        bool played;
        TimeSpan_NTP duration;
    };

    bool GetFrontCmd(SmoothTransportCommand* pCmd);
    TimeSpan_NTP GetCurrentPlaybackTime();
    void Reset();

    void AddAd(string url, int64_t startTime, int32_t duration);
    void CheckToPlayAd();
    void ResumeMainPlayback();
    void CancelNextAd();
    void UpdateState(ETransportState newState);
    void SetIsPlayingAd(bool value);
    bool ShouldGetTextFragment();
    void DownloadTextFragment();
    ChunkIterator GetCurrentTextChunkIterator();

    static uint32_t pkAPI _WorkThreadEntryPoint( void* pParam )
    {
        reinterpret_cast<SmoothTransportWorker*>( pParam )->Execute();
        return( 0 );
    }

    ISmoothTransport* m_pISmoothTransport;
    SmoothTransportInfo* m_pSTInfo;
    Lockable m_Lock;
    pkHANDLE m_hWorkThread;
    CEventHelper m_eventSignal;
    deque<SmoothTransportCommand> m_cmdList;

    map<TimeSpan_NTP, AdInfo> m_adList;
    map<TimeSpan_NTP, AdInfo>::iterator m_nextAd;

    AutoRefPtr<IManifestStream> m_apTextStream;
    ChunkIterator m_itChunk;

    string m_mainURL;
    bool m_isPlayingAd;
    bool m_downloadFragmentInProgress;
    bool m_iteratorInitialized;
    TimeSpan_NTP m_resumeMainURLTime;
};

// ========================================================================
// CXDrmDiagDelegate Declaration
// ========================================================================
class CXDrmDiagDelegate : public IXDrmDiagDelegate
{
public:
    CXDrmDiagDelegate();
    ~CXDrmDiagDelegate();

    pkRESULT OpenLogFile( const char* szLogFileUri );

protected:
    void OnDecrypt( const IXDrmDiagDelegate::SDecryptInfo& sDecryptInfo );
    void OnDecryptBufferChain( const IXDrmDiagDelegate::SDecryptInfo& sDecryptInfo );

private:
    pkHANDLE m_hLog;
};

#ifdef SS_ADAPTER_USED
// ========================================================================
// SmoothTransportInfoAdapter Declaration
// ========================================================================
class SmoothTransportInfoAdapter : public SmoothTransportInfo
{
public:
    SmoothTransportInfoAdapter();
    SmoothTransportInfoAdapter(ISmoothTransport* pISmoothTransport);
//    SmoothTransportInfoAdapter(ISmoothTransport* pISmoothTransport, SSAdapter_Callback_t* pCallback);
    ~SmoothTransportInfoAdapter();
    // 
    // Interface implementations
    // 
    __override void ManifestReadyCallback(IManifest* pManifest, pkRESULT manifestResult);
    __override void ErrorCallback(SmoothTransportError& errorInfo);
    __override void StatusCallback(SmoothTransportStatus& status);
    __override void StreamSelectedCallback(_In_ StreamSelectedEventArgs* pEventArgs);
    __override pkRESULT OnFragmentData(
                        _In_ pkRESULT hrResult,
                        _In_ CHUNK_INFO* pChunkInfo,
                        _In_opt_ IRefBuffer* pBuffer,
                        _In_ bool fFinalBuffer,
                        _In_ size_t cbTotalLength );
    // 
    // Implementation
    // 
    void SetSSAdapterCallback(SSAdapter_Callback_t *pCallback);

private:
    //SSAdapter_Callback_t *m_pCallback;
    SSAdapter_Callback_t m_Callback;
};

// ========================================================================
// SmoothStreamingAdapter Declaration
// ========================================================================
typedef struct SmoothStreamingAdapter
{
    ISmoothTransport* pISmoothTransport;
//    SmoothTransportInfo* pSmoothTransportInfo;
    SmoothTransportInfoAdapter* pSmoothTransportInfoAdapter;
    SmoothTransportWorker* pSmoothTransportWorker;
    CXDrmDiagDelegate drmDiagDelegate;
    IXDrm* poXDrm;
    CLicenseAcquirer licenseAcquirer;
    SmoothStreamingAdapter()
    {
        pISmoothTransport = NULL;
 //       pSmoothTransportInfo = NULL;
        pSmoothTransportInfoAdapter = NULL;
        pSmoothTransportWorker = NULL;
        poXDrm = NULL;
    }
    string strUrl;
} SmoothStreamingAdapter_t;

typedef struct {
/*!
    size of subtitle packet
    */
int size;
/*!
    pts of subtitle packet
    */
int pts;
/*!
    data of subtitle packet
    */
unsigned char * data;

} SUBT_DATA;
#endif

// ========================================================================
// codec Helper Functions
// ========================================================================
#define PRINTFOURCC(fourCC)  if(fourCC <= 0xffff) {SMOOTH_MSG(("\n%s() fourCC=0x%x\n", __FUNCTION__, fourCC)); } \
                             else {SMOOTH_MSG(("\n%s() fourCC=0x%x,%c%c%c%c\n", __FUNCTION__, fourCC, \
                             ((char*)&fourCC)[0], ((char*)&fourCC)[1], ((char*)&fourCC)[2], ((char*)&fourCC)[3]));}
int FourCCTo2Codec(uint32_t fourCC)
{
    switch (fourCC)
    {
    //Video types
    //
    //VC1
    case MAKEFOURCC('W','M','V','A'):
    case MAKEFOURCC('W','V','C','1'):
        return SS_ADAPTER_DECODER_CODECTYPE_VIDEO_VC1;

    //AVC
    case MAKEFOURCC('H','2','6','4'):
    case MAKEFOURCC('A','V','C','1'):
    case MAKEFOURCC('X','2','6','4'):
    case MAKEFOURCC('D','A','V','C'):
    case MAKEFOURCC('A','V','C','A'):
    case MAKEFOURCC('A','V','C','B'):
        return SS_ADAPTER_DECODER_CODECTYPE_VIDEO_H264;

    //WMV9
    case MAKEFOURCC('W','M','V','3'):
        return SS_ADAPTER_DECODER_CODECTYPE_VIDEO_WMV9;

    //Audio types
    //
    //MP3
    case 0x00000055:
        return SS_ADAPTER_DECODER_CODECTYPE_AUDIO_MPEG;

    //AAC
    case 0x00001601:
    case 0x000000FF:
    case MAKEFOURCC('A','A','C', 0 ):
    case MAKEFOURCC('A','A','C',' '):
    case MAKEFOURCC('A','A','C','L'):
    case MAKEFOURCC('A','A','C','H'):
    case MAKEFOURCC('A','A','C','P'):
        return SS_ADAPTER_DECODER_CODECTYPE_AUDIO_AAC;

    //AC3
    case 0x00000092:
    case MAKEFOURCC('A','C','3',' '):
        return SS_ADAPTER_DECODER_CODECTYPE_AUDIO_AC3;

    case MAKEFOURCC('E','A','C','3'):
    case MAKEFOURCC('E','C','-','3'):
        return SS_ADAPTER_DECODER_CODECTYPE_AUDIO_EAC3;

    //WMA
    case 0x00000160:
    case 0x00000161:
    case 0x00005052:
    case MAKEFOURCC('W','M','A', 0 ):
    case MAKEFOURCC('W','M','A',' '):
    case MAKEFOURCC('W','M','A','2'):
        return SS_ADAPTER_DECODER_CODECTYPE_AUDIO_WMASTD;

    //WMA Pro
    case 0x00000162:
    case MAKEFOURCC('W','M','A','P'):
        return SS_ADAPTER_DECODER_CODECTYPE_AUDIO_WMAPRO;
    }
    return SS_ADAPTER_DECODER_CODECTYPE_INVALID;
}

string FourCCTo2String(uint32_t fourCC)
{
    char cFourCC[5] = {0};
    memcpy(cFourCC, ((char*)&fourCC), 4);
    string retStr = cFourCC;
    return retStr;
}

// ========================================================================
// String Helper Functions
// ========================================================================

//escape spaces in the string
string stringEscSpaces(const string& s)
{
    string d;
    d.reserve(s.length() << 1);
    int len = (int)s.length();
    for (int i = 0; i < len; i++)
    {
        if (' ' == s[i])
        {
            d += "%20";
        }
        else
        {
            d += s[i];
        }
    }
    return d;
}

//trims characters from beginning and ending of string
string stringTrim(const string& s, const char* trimArray)
{
    if (s.length() == 0)
        return s;
    size_t b = s.find_first_not_of(trimArray);
    size_t e = s.find_last_not_of(trimArray);
    if (b == -1)
        return "";
    return string(s, b, e - b + 1);
}

//split string by delimited characters
int stringSplit(const string& text, vector<string>& words, const string& separators)
{
    size_t textLen = text.length();
    size_t start = text.find_first_not_of(separators, 0);
    while ((start != string::npos) && (start < textLen))
    {
        size_t stop = text.find_first_of(separators,start);

        if ((stop == string::npos) || (stop > textLen))
            stop = textLen;

        words.push_back(text.substr(start, stop-start));
        start = text.find_first_not_of(separators, stop+1);
    }
    return (int)words.size();
}

// convert string into integer
int stringToInt(const string& s)
{
    char* stopPoint;
    return strtol(s.c_str(), &stopPoint, 0);
}

// convert string to wString
wstring stringToWString(const string& str )
{
    uint32_t len = 0;
    wstring wStr;
    wStr.clear();

    if( pkSUCCEEDED(Executive_MultiByteToWideChar( EXECUTIVE_CODEPAGE_ANSII, 0, (int8_t *)str.c_str(), str.size(), NULL, 0 , &len)) )
    {
        uint32_t written = 0;
        wchar_t* wszTemp = NEW_NO_THROW wchar_t[len];

        Executive_MultiByteToWideChar( EXECUTIVE_CODEPAGE_ANSII, 0, (int8_t *)str.c_str(), str.size(), wszTemp, len, &written );

        // handle the case where the null terminator was processed
        if (written > 0 && 0 == wszTemp[written-1])
        {
            written--;
        }

        wStr.assign(wszTemp, written);

        delete [] wszTemp;
    }

    return wStr;
}

//convert wString to string
string wstring2string(wstring wstr)  
{  
    string result;  
    //  
    int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);  
    char* buffer = new char[len + 1];  
    //  
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);  
    buffer[len] = '\0';  
    //  
    result.append(buffer);  
    delete[] buffer;  
    return result;  
}

uint32_t wstringToUInt32(const wstring& wstr)
{
    char szMultibyte[32];
    szMultibyte[0] = 0;
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, szMultibyte, sizeof(szMultibyte), NULL, NULL);
    char* stopPoint;
    return (uint32_t)strtoul(szMultibyte, &stopPoint, 0);
}


// ========================================================================
// Other Helper functions
// ========================================================================
void PrintStreamList(const std::vector< AutoRefPtr<IManifestStream> > &streamList)
{
    SMOOTH_MSG("Contents of stream list, size: %d\n", streamList.size());
    SMOOTH_MSG("Stream names:");
    for(size_t i=0; i<streamList.size(); i++)
    {
        SMOOTH_MSG(" \"%ls\"",streamList[i]->Name().c_str());
    }
    SMOOTH_MSG("\n");
}

void PrintCmdList(const string& s)
{
#ifdef MSPK_USE_TELNET_CONSOLE
    ConsoleWrite(s_CmdList, s.c_str());
#else
    SMOOTH_MSG(s_CmdList, s.c_str());
#endif
}

int GetNextSpeedIndex(bool isff)
{
    speedidx = (speedidx + 1) % SPEEDS_COUNT;
    if (isff && speedidx <= SPEED_ZERO)
    {
        speedidx = SPEED_ZERO+1;
    }
    if (!isff && speedidx >= SPEED_ZERO)
    {
        speedidx = 0;
    }
    SMOOTH_MSG("play at speed:%.2f\n", speeds[speedidx]);

    return speedidx;
}

void SendExtendedCommand(ISmoothTransport* pISmoothTransport, _In_ const char* command, _In_ int argc, _In_ const char* argv[])
{
    if (pkFAILED(pISmoothTransport->SendExtendedCommand(command, argc, argv)))
    {
        SMOOTH_MSG("SendExtendedCommand %s failed\n", command);
    }
}

void LimitVideoTracks(IManifest* pManifest, 
                      int minKbps,
                      int maxKbps,
                      bool restrictTracks)
{
    AutoRefPtr<IManifest> apManifest(pManifest);

    // verify the kbps range
    if (c_VALUE_NOT_SET != minKbps
        && (minKbps < c_MIN_BITRATE_KBPS || minKbps > c_MAX_BITRATE_KBPS))
    {
        SMOOTH_MSG("INVALID setting for minbr (%d)\n", minKbps);
        return;
    }
            
    if (c_VALUE_NOT_SET != g_iMaxBitrateKbps
        && (maxKbps > c_MAX_BITRATE_KBPS || maxKbps < c_MIN_BITRATE_KBPS))
    {
        SMOOTH_MSG("INVALID setting for maxbr (%d)\n", maxKbps);
        return;
    }

    if(c_VALUE_NOT_SET != minKbps
        && c_VALUE_NOT_SET != maxKbps
        && minKbps > maxKbps)
    {
        SMOOTH_MSG("INVALID setting for minbr (%d) greater than maxbr (%d)\n",
            minKbps, g_iMaxBitrateKbps);
        return;
    }

    if (NULL != apManifest)
    {
        std::vector< AutoRefPtr<IManifestStream> > selectedStreams;
        std::vector< AutoRefPtr<IManifestTrack> > proposedTracks;

        apManifest->GetSelectedStreams(&selectedStreams);
        for (size_t ixStream = 0; ixStream < selectedStreams.size(); ++ixStream)
        {
            if (MediaStreamTypeVideo != selectedStreams[ixStream]->Type())
            {
                continue;
            }
            
            int iMinBitrateKbps = (c_VALUE_NOT_SET != minKbps) ? minKbps : c_MIN_BITRATE_KBPS;
            int iMaxBitrateKbps = (c_VALUE_NOT_SET != maxKbps) ? maxKbps : c_MAX_BITRATE_KBPS;

            std::vector< AutoRefPtr<IManifestTrack> > availableTracks;

            pkRESULT pkResult = selectedStreams[ixStream]->GetAvailableTracks( &availableTracks );
            if (pkFAILED(pkResult))
            {
                SMOOTH_MSG("FAILED to GetAvailableTracks: 0x%X\n", pkResult);
                return;
            }

            for (size_t ixTrack = 0; ixTrack < availableTracks.size(); ++ixTrack)
            {
                int trackBitrateKbps = availableTracks[ixTrack]->Bitrate() / 1000;

                if (iMinBitrateKbps <= trackBitrateKbps && trackBitrateKbps <= iMaxBitrateKbps)
                {
                    SMOOTH_MSG("Accepting track at %d kbps\n", trackBitrateKbps);
                    proposedTracks.push_back( availableTracks[ixTrack] );
                }
                else
                {
                    SMOOTH_MSG("Rejecting track at %d kbps\n", trackBitrateKbps);
                }
            }
                        
            if (0 == proposedTracks.size())
            {
                SMOOTH_MSG("FAILED to find any tracks from %d to %d kbps\n", iMinBitrateKbps, iMaxBitrateKbps);
                return;
            }

            if (restrictTracks)
            {
                pkResult = selectedStreams[ixStream]->RestrictTracks(proposedTracks);
            }
            else
            {
                pkResult = selectedStreams[ixStream]->SelectTracks(proposedTracks);
            }

            if (pkFAILED(pkResult))
            {
                SMOOTH_MSG("FAILED to %sTracks: 0x%X\n", g_bRestrictTracks ? "Restrict" : "Select", pkResult);
            }
            break;
        }
    }
}

// #define DIAG_ENABLE
#ifdef DIAG_ENABLE
uint32_t pkAPI DiagEventRetrieveFun(void* param)
{
    if (param)
    {
        IDiagsManager* pDiagsManager = (IDiagsManager*)param;

        int isError;
        static const size_t EVENT_LENGTH = 256;
        static const size_t EVENT_DATA_LENGTH = 4096;

        WCHAR wcsEventMsg[EVENT_LENGTH];
        WCHAR wcsEventData[EVENT_DATA_LENGTH];

        while (!g_IsAbandoned)
        {
            memset(wcsEventMsg, 0, sizeof(wcsEventMsg));
            memset(wcsEventData, 0, sizeof(wcsEventData));

            while (pDiagsManager->GetDiagnosticEvent(&isError, wcsEventMsg, EVENT_LENGTH, wcsEventData, EVENT_DATA_LENGTH))
            {
                SMOOTH_MSG("[AVDiagEvent]: <%ls>      <%ls>\n", wcsEventMsg, wcsEventData);
            }

            //If no diag event, sleep for a while and try again
            Executive_Sleep(100);
        }
    }

    return 0;
}
#endif // DIAG_ENABLE

//Update Media info
pkRESULT UpdateMediaInfo(_In_ StreamSelectedEventArgs* pEventArgs)
{

    ss_adapter_film_info_t *pSs_adapter_film_info = &g_ss_adapter_film_info;

    for (size_t ixStream = 0; ixStream < pEventArgs->StreamChanges.size(); ++ixStream)
    {
        IManifestStream* pStream = pEventArgs->StreamChanges[ixStream].pStream;// The stream that is selected or de-selected
        if (pEventArgs->StreamChanges[ixStream].Action == StreamChangedEventArgs::StreamSelected)
        {
            if (MediaStreamTypeVideo == pStream->Type())
            {
                std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;
                pStream->GetSelectedTracks(&selectedTracks);
                pSs_adapter_film_info->video_type = FourCCTo2Codec(selectedTracks[0]->FourCC());

                pSs_adapter_film_info->video_disp_w = pStream->DisplayWidth();
                pSs_adapter_film_info->video_disp_h = pStream->DisplayHeight();
                pSs_adapter_film_info->video_bps = selectedTracks[0]->Bitrate();//availableTracks[0]->Bitrate();
            }
            else if (MediaStreamTypeAudio == pStream->Type())
            {
                std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;

                pStream->GetSelectedTracks(&selectedTracks);
                pSs_adapter_film_info->audio_type = FourCCTo2Codec(selectedTracks[0]->FourCC());
                pSs_adapter_film_info->audio_bps = selectedTracks[0]->Bitrate();//availableTracks[0].Bitrate();
                wstring samplerate;
                selectedTracks[0]->GetAttribute(stringToWString("SamplingRate"), &samplerate);
                pSs_adapter_film_info->audio_samplerate = wstringToUInt32(samplerate);
                string lang = wstring2string(pStream->Language());
                if (pSs_adapter_film_info->audio_language)
                {
                    Executive_Free(pSs_adapter_film_info->audio_language);
                }
                
                pSs_adapter_film_info->audio_language = (int8_t*)Executive_Alloc(lang.length()+1, true);
                if (pSs_adapter_film_info->audio_language)
                {
                    memcpy( pSs_adapter_film_info->audio_language, lang.c_str(), lang.length());
                    // pSs_adapter_film_info->audio_language[lang.length()] = '\0';
                }
                
                //title
                string title = wstring2string(pStream->Name());
                size_t langLen = 0;
                size_t titleLen = title.length();
                for (size_t iCurrentIndex = 0; iCurrentIndex < g_ss_adapter_aud_lang_cnt; ++iCurrentIndex)
                {

                    langLen = strlen(g_ss_adapter_audio_info[iCurrentIndex].lang);
                    if (strlen((char *)pSs_adapter_film_info->audio_language) == langLen
                        && strlen((char *)g_ss_adapter_audio_info[iCurrentIndex].title) == titleLen)
                    {
                        if (!memcmp(pSs_adapter_film_info->audio_language, g_ss_adapter_audio_info[iCurrentIndex].lang, langLen)
                            && !memcmp(title.c_str(), g_ss_adapter_audio_info[iCurrentIndex].title, titleLen))
                        {
                            pSs_adapter_film_info->audio_track_id = g_ss_adapter_audio_info[iCurrentIndex].track_id;
                            SMOOTH_MSG("UpdateMediaInfo() selected audio_track_id=%d\n", g_ss_adapter_audio_info[iCurrentIndex].track_id);
                        }
                    }
                }
            }
        }
       
    }
}

// Switch to the next audio stream
void SwitchAudio(SmoothTransportInfo& stInfo)
{
    AutoRefPtr<IManifest> apManifest(stInfo.GetManifest());
    if (NULL != apManifest)
    {
        std::vector< AutoRefPtr<IManifestStream> > proposedSelection;
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestStream> > selectedStreams;
        AutoRefPtr<IManifestStream> apSelectedAudio;
        AutoRefPtr<IManifestStream> apNextAudio;

        apManifest->GetAvailableStreams(&availableStreams);
        apManifest->GetSelectedStreams(&selectedStreams);

        // Count how many audio streams we currenlty have
        int iNumAudio = 0;
        for (size_t i = 0; i < availableStreams.size(); ++i)
        {
            if (MediaStreamTypeAudio == availableStreams[i]->Type())
            {
                ++iNumAudio;
            }
        }

        if (iNumAudio < 2)
        {
            // There is nothing to switch
            SMOOTH_MSG("There is only %d audio streams and there is nothing to switch", iNumAudio);
        }
        else
        {
            // Find the current selected audio stream
            for (size_t i = 0; i < selectedStreams.size(); ++i)
            {
                if (MediaStreamTypeAudio == selectedStreams[i]->Type())
                {
                    apSelectedAudio.Set(selectedStreams[i]);
                }
                else
                {
                    proposedSelection.push_back(selectedStreams[i]);
                }
            }

            // Find the index of the current selected audio in the available stream list
            size_t iCurrentIndex = 0;
            for (; iCurrentIndex < availableStreams.size(); ++iCurrentIndex)
            {
                if (apSelectedAudio == availableStreams[iCurrentIndex])
                {
                    break;
                }
            }

            // Find the next audio stream in the available stream list
            apNextAudio = apSelectedAudio;
            for (size_t i = iCurrentIndex + 1; i < availableStreams.size(); ++i)
            {
                if (MediaStreamTypeAudio == availableStreams[i]->Type())
                {
                    apNextAudio.Set(availableStreams[i]);
                    break;
                }
            }

            if (apNextAudio == apSelectedAudio)
            {
                for (size_t i = 0; i < iCurrentIndex; ++i)
                {
                    if (MediaStreamTypeAudio == availableStreams[i]->Type())
                    {
                        apNextAudio.Set(availableStreams[i]);
                        break;
                    }
                }
            }

            ASSERT(apNextAudio != apSelectedAudio);
            proposedSelection.push_back(apNextAudio);

            pkRESULT pkResult = apManifest->SelectStreamsAsync(&stInfo, proposedSelection);
            if( pkFAILED( pkResult ) )
            {
                SMOOTH_MSG("FAILED to SelectStreamsAsync: 0x%X\n", pkResult);
                PrintStreamList( proposedSelection );
            }
        }
    }
}

void change_audio_track(SmoothTransportInfo& stInfo, int trackId)
{
    AutoRefPtr<IManifest> apManifest(stInfo.GetManifest());
    if (NULL != apManifest)
    {
        std::vector< AutoRefPtr<IManifestStream> > proposedSelection;
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestStream> > selectedStreams;
        AutoRefPtr<IManifestStream> apSelectedAudio;
        AutoRefPtr<IManifestStream> apNextAudio;

        apManifest->GetAvailableStreams(&availableStreams);
        apManifest->GetSelectedStreams(&selectedStreams);

        // Count how many audio streams we currenlty have
        int iNumAudio = 0;
        for (size_t i = 0; i < availableStreams.size(); ++i)
        {
            if (MediaStreamTypeAudio == availableStreams[i]->Type())
            {
                ++iNumAudio;
            }
        }

        if (iNumAudio < 2 || iNumAudio <= trackId || trackId < 0)
        {
            // There is nothing to switch
            SMOOTH_ADAPTER_MSG("There is only %d audio streams and can't switch to %d", iNumAudio, trackId);
        }
        else
        {
            SMOOTH_ADAPTER_MSG("Switch to audio %d\n", trackId);
            // Find the current selected audio stream
            for (size_t i = 0; i < selectedStreams.size(); ++i)
            {
                if (MediaStreamTypeAudio == selectedStreams[i]->Type())
                {
                    apSelectedAudio.Set(selectedStreams[i]);
                }
                else
                {
                    proposedSelection.push_back(selectedStreams[i]);
                }
            }

            // Find the index of the current selected audio in the available stream list
            size_t iCurrentTrack = 0;
            for (size_t iCurrentIndex = 0; iCurrentIndex < availableStreams.size(); ++iCurrentIndex)
            {
                if (MediaStreamTypeAudio == availableStreams[iCurrentIndex]->Type())
                {
                    if (apSelectedAudio == availableStreams[iCurrentIndex])
                    {
                        break;
                    }
                    iCurrentTrack++;
                }
            }

            if (trackId != iCurrentTrack)
            {
                // Find the next audio stream in the available stream list
                apNextAudio = apSelectedAudio;
                size_t iNextTrack = 0;
                for (size_t i = 0; i < availableStreams.size(); ++i)
                {
                    //SMOOTH_ADAPTER_MSG("change_audio_track i %d, trackId=%d\n", i, trackId);
                    if (MediaStreamTypeAudio == availableStreams[i]->Type())
                    {
                        if (iNextTrack == trackId)
                        {
                            apNextAudio.Set(availableStreams[i]);
                            break;
                        }
                        iNextTrack++;
                    }
                }

                if (apNextAudio != apSelectedAudio)
                {
                    proposedSelection.push_back(apNextAudio);
                    SMOOTH_ADAPTER_MSG("change_audio_track - SelectStreamsAsync trackId=%d\n", trackId);

                    pkRESULT pkResult = apManifest->SelectStreamsAsync(&stInfo, proposedSelection);
                    if( pkFAILED( pkResult ) )
                    {
                        SMOOTH_ADAPTER_MSG("FAILED to SelectStreamsAsync: 0x%X\n", pkResult);
                        PrintStreamList( proposedSelection );
                    }
                }
            }
        }
    }
}

void change_subt_id(SmoothTransportInfo& stInfo, int subtId)
{
    AutoRefPtr<IManifest> apManifest(stInfo.GetManifest());
    if (NULL != apManifest)
    {
        std::vector< AutoRefPtr<IManifestStream> > proposedSelection;
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestStream> > selectedStreams;
        AutoRefPtr<IManifestStream> apSelectedSubt;
        AutoRefPtr<IManifestStream> apNextSubt;

        apManifest->GetAvailableStreams(&availableStreams);
        apManifest->GetSelectedStreams(&selectedStreams);

        // Count how many audio streams we currenlty have
        int iNumSubt = 0;
        for (size_t i = 0; i < availableStreams.size(); ++i)
        {
            if (MediaStreamTypeText == availableStreams[i]->Type())
            {
                ++iNumSubt;
            }
        }

        if (iNumSubt < 2 || iNumSubt <= subtId || subtId < 0)
        {
            // There is nothing to switch
            SMOOTH_ADAPTER_MSG("There is only %d subtitle streams and can't switch to %d", iNumSubt, subtId);
        }
        else
        {
            SMOOTH_ADAPTER_MSG("Switch to subtitle %d\n", subtId);
            // Find the current selected audio stream
            for (size_t i = 0; i < selectedStreams.size(); ++i)
            {
                if (MediaStreamTypeText == selectedStreams[i]->Type())
                {
                    apSelectedSubt.Set(selectedStreams[i]);
                }
                else
                {
                    proposedSelection.push_back(selectedStreams[i]);
                }
            }

            // Find the index of the current selected audio in the available stream list
            size_t iCurrentTrack = 0;
            for (size_t iCurrentIndex = 0; iCurrentIndex < availableStreams.size(); ++iCurrentIndex)
            {
                if (MediaStreamTypeText == availableStreams[iCurrentIndex]->Type())
                {
                    if (apSelectedSubt == availableStreams[iCurrentIndex])
                    {
                        break;
                    }
                    iCurrentTrack++;
                }
            }

            if (subtId != iCurrentTrack)
            {
                // Find the next audio stream in the available stream list
                apNextSubt = apSelectedSubt;
                size_t iNextTrack = 0;
                for (size_t i = 0; i < availableStreams.size(); ++i)
                {
                    //SMOOTH_ADAPTER_MSG("%s i %d, subtId=%d\n", __FUNCTION__, i, subtId);
                    if (MediaStreamTypeText == availableStreams[i]->Type())
                    {
                        if (iNextTrack == subtId)
                        {
                            apNextSubt.Set(availableStreams[i]);
                            break;
                        }
                        iNextTrack++;
                    }
                }

                if (apNextSubt != apSelectedSubt)
                {
                    proposedSelection.push_back(apNextSubt);
                    SMOOTH_ADAPTER_MSG("%s - SelectStreamsAsync subtId=%d\n", __FUNCTION__, subtId);

                    pkRESULT pkResult = apManifest->SelectStreamsAsync(&stInfo, proposedSelection);
                    if( pkFAILED( pkResult ) )
                    {
                        SMOOTH_ADAPTER_MSG("FAILED to SelectStreamsAsync: 0x%X\n", pkResult);
                        PrintStreamList( proposedSelection );
                    }
                }
            }
        }
    }
}

void ToggleStream(SmoothTransportInfo& stInfo, const wstring& streamName)
{
    AutoRefPtr<IManifest> apManifest(stInfo.GetManifest());
    if (NULL != apManifest)
    {
        AutoRefPtr<IManifestStream> apStream;
        bool toSelect = true;
        std::vector< AutoRefPtr<IManifestStream> > proposedSelection;
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestStream> > selectedStreams;

        apManifest->GetAvailableStreams(&availableStreams);
        apManifest->GetSelectedStreams(&selectedStreams);

        // check if the stream exists
        for(size_t i=0; i<availableStreams.size(); i++)
        {
            if( streamName == availableStreams[i]->Name() )
            {
                apStream = availableStreams[i];
                break;
            }
        }

        if ( apStream )
        {
            for(size_t i=0; i<selectedStreams.size(); i++)
            {
                if( streamName != selectedStreams[i]->Name() )
                {
                    proposedSelection.push_back(selectedStreams[i]);
                }
                else
                {
                    toSelect = false;
                }
            }

            if ( toSelect )
            {
                proposedSelection.push_back(apStream);
            }

            pkRESULT pkResult = apManifest->SelectStreamsAsync(&stInfo, proposedSelection);
            if( pkFAILED( pkResult ) )
            {
                SMOOTH_ADAPTER_MSG("FAILED to SelectStreamsAsync: 0x%X\n", pkResult);
                PrintStreamList( proposedSelection );
            }
        }
        else
        {
            SMOOTH_ADAPTER_MSG("FAILED to toggle stream: %ls stream name not found\n", streamName.c_str());
        }
    }
}

// ========================================================================
// CEventHelper Implementation
// ========================================================================
CEventHelper::CEventHelper( EResetMode eResetMode, bool bInitiallySignaled /*= false*/ )
{
    pkRESULT pkResult =
        ::Executive_CreateEvent(
            NULL, // ignored
            (eResetMode == eResetModeManual),
            bInitiallySignaled,
            &m_Event );

    if (pkFAILED(pkResult))
    {
        m_Event = NULL;
        pkASSERT(FALSE);
    }
}

CEventHelper::~CEventHelper()
{
    if(m_Event != NULL)
    {
        ::Executive_CloseEvent(m_Event);
        m_Event = NULL;
    }
}

bool CEventHelper::Set() 
{
    return( pkSUCCEEDED(::Executive_SetEvent(m_Event)) ? true : false );
}

bool CEventHelper::Reset() 
{ 
    return( pkSUCCEEDED(::Executive_ResetEvent(m_Event)) ? true : false );
}

CEventHelper::EWaitResult CEventHelper::Wait( uint32_t dwMsTimeout /*= INFINITE*/ )
{
    pkRESULT pkResult = ::Executive_WaitForEvent(m_Event, dwMsTimeout);

    EWaitResult eWaitResult;

    switch (pkResult)
    {
        case pkE_ABORT:
            eWaitResult = eWaitAbandoned;
            break;
        case pkS_OK:
            eWaitResult = eWaitSignaled;
            break;
        case pkS_FALSE:
            eWaitResult = eWaitTimeout;
            break;
        default:
            eWaitResult = eWaitFailed;
            break;
    }
    return eWaitResult;
}

// ========================================================================
// SmoothTransportWorker Implementation
// ========================================================================
SmoothTransportWorker::SmoothTransportWorker(ISmoothTransport* pISmoothTransport, 
                                             SmoothTransportInfo *pSTInfo)
    : m_eventSignal( CEventHelper::eResetModeAuto )
    , m_pISmoothTransport ( pISmoothTransport )
    , m_pSTInfo ( pSTInfo )
    , m_isPlayingAd ( false )
    , m_downloadFragmentInProgress ( false )
    , m_iteratorInitialized ( false )

{
}

SmoothTransportWorker::~SmoothTransportWorker()
{
    ShutDown();
}


pkRESULT SmoothTransportWorker::Initialize()
{
    Reset();
    return( Executive_CreateThread( _WorkThreadEntryPoint, this, 0, &m_hWorkThread ) );
}

void SmoothTransportWorker::ShutDown()
{
    pkHANDLE hWorkThread = m_hWorkThread;
    m_hWorkThread = NULL;

    if( hWorkThread != NULL )
    {
        // Wake up the thread to detect a shutdown
        m_eventSignal.Set();
        Executive_WaitForThread( hWorkThread, EXEC_WAIT_INFINITE );
        Executive_CloseThread( hWorkThread );
    }
}

void SmoothTransportWorker::Execute()
{
    SMOOTH_MSG("SmoothTranportWorker::Execute Entry\n");
    while( true )
    {
        uint32_t waitTimeMS = INFINITE;

        // for text and ads, the thread polls to check for starting/stopping ad 
        // and check to download more sparse data
        if(g_bPrintText || g_bPlayAd)
        {
            waitTimeMS = c_iWorkerThreadPollPeriodMSec;
        }
        m_eventSignal.Wait(waitTimeMS);

        // check for shutdown
        if( m_hWorkThread == NULL )
        {
            break;
        }

        SmoothTransportCommand cmdInfo;
        while(GetFrontCmd(&cmdInfo))
        {
            // ignore the following commands while playing an ad
            if(IsPlayingAd())
            {
                if( eTransportCommand_Play == cmdInfo.cmd 
                    || eTransportCommand_Skip == cmdInfo.cmd
                    || eTransportCommand_Seek == cmdInfo.cmd
                    || eTransportCommand_SetPlaybackRange == cmdInfo.cmd
                    || eTransportCommand_FragmentDataDone == cmdInfo.cmd
                    || eTransportCommand_FragmentNoMoreItems == cmdInfo.cmd
                    || eTransportCommand_AddAd == cmdInfo.cmd ) 
                {
                    SMOOTH_MSG("SmoothTranportWorker::Execute command %d ignored, playing ad\n", cmdInfo.cmd);
                    continue;
                }
            }

            pkRESULT pkResult = pkS_OK;
            SMOOTH_MSG("SmoothTranportWorker::Execute cmd=%d\n", cmdInfo.cmd);

            switch(cmdInfo.cmd)
            {
                case eTransportCommand_Play:
                    SMOOTH_MSG("SmoothTranportWorker::Execute eTransportCommand_Play\n");
                    pkResult = m_pISmoothTransport->Play(cmdInfo.speed);
                    break;
                case eTransportCommand_Pause:
                    pkResult = m_pISmoothTransport->Pause();
                    break;
                case eTransportCommand_Close:
                    pkResult = m_pISmoothTransport->Close();
                    break;
                case eTransportCommand_Open:
                    m_mainURL = cmdInfo.str;
                    SMOOTH_MSG("SmoothTranportWorker::Execute eTransportCommand_Open=%s\n", m_mainURL.c_str());
                    m_pSTInfo->Reset();
                    Reset();
                    pkResult = m_pISmoothTransport->Open(cmdInfo.str, SmoothTransportProtocol_Mbr, true);
                    break;
                case eTransportCommand_Skip:
                    CancelNextAd();
                    pkResult = m_pISmoothTransport->Skip( NTP_FROM_SECONDS( cmdInfo.sec ) );
                    break;
                case eTransportCommand_Seek:
                    CancelNextAd();
                    pkResult = m_pISmoothTransport->Seek( NTP_FROM_SECONDS( cmdInfo.sec ) );
                    break;
                case eTransportCommand_SetPlaybackRange:
                    pkResult = m_pISmoothTransport->SetPlaybackRangeAsync( m_pSTInfo, TimeSpan_NTP::FromTicks(cmdInfo.val64 ) );
                    break;
                case eTransportCommand_FragmentDataDone:
                    m_downloadFragmentInProgress = false;
                    m_itChunk.MoveNext();
                    DownloadTextFragment();
                    break;
                case eTransportCommand_FragmentNoMoreItems:
                    m_iteratorInitialized = false;
                    break;
                case eTransportCommand_AddAd:
                    AddAd(cmdInfo.str, cmdInfo.val64, cmdInfo.sec);
                    break;
                case eTransportCommand_UpdateState:
                    UpdateState((ETransportState)cmdInfo.sec);
                    break;
                default:
                    pkResult = pkE_UNEXPECTED;
                    SMOOTH_MSG("SmoothTranportWorker::Execute unsupported command %d\n", cmdInfo.cmd);
                    break;
            }

            if (pkFAILED(pkResult))
            {
                SMOOTH_MSG("SmoothTranportWorker::Execute command %d failed 0x%X\n", cmdInfo.cmd, pkResult);
            }
        }

        if(m_nextAd != m_adList.end())
        {
            //check if the duration of the ad has been played
            if(IsPlayingAd())
            {
                TimeSpan_NTP stopTime = m_pSTInfo->GetStartTime() + TimeSpan_NTP::ConvertFrom(m_nextAd->second.duration);
                if(m_nextAd->second.duration > TimeSpan_NTP::FromTicks(0) && GetCurrentPlaybackTime() >= stopTime)
                {
                    ResumeMainPlayback();
                }
            }
            else
            {
                CheckToPlayAd();
            }
        }
        
        if(!m_downloadFragmentInProgress)
        {
            DownloadTextFragment();
        }
    }
    SMOOTH_MSG("SmoothTranportWorker::Execute Exit\n");
}

void SmoothTransportWorker::RequestSmoothTransportCommand(ETransportCommand cmd, float speed /*= 1.0*/, int32_t sec /*= 0*/, string str /*=""*/, int64_t val64 /*= 0*/ )
{
    SmoothTransportCommand stCmd;
    stCmd.cmd = cmd;
    stCmd.speed = speed;
    stCmd.sec = sec;
    stCmd.str = str;
    stCmd.val64 = val64;

    AutoLock al( &m_Lock );
    m_cmdList.push_back(stCmd);
    m_eventSignal.Set();
}

void SmoothTransportWorker::SetTextStream(IManifestStream* pTextStream)
{
    AutoLock al( &m_Lock );
    m_apTextStream.Set(pTextStream);
}

bool SmoothTransportWorker::GetFrontCmd(SmoothTransportCommand* pCmd)
{
    AutoLock al( &m_Lock );
    if(m_cmdList.empty())
    {
        return false;
    }
        
    pCmd->cmd = m_cmdList.front().cmd;
    pCmd->speed = m_cmdList.front().speed;
    pCmd->sec = m_cmdList.front().sec;
    pCmd->str = m_cmdList.front().str;
    pCmd->val64 = m_cmdList.front().val64;

    m_cmdList.pop_front();
    return true;
}

void SmoothTransportWorker::DownloadTextFragment()
{
    if( ShouldGetTextFragment() )
    {
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;
        pkRESULT pkResult = pkS_OK;

        m_apTextStream->GetSelectedTracks(&selectedTracks);
        pkResult = m_apTextStream->DownloadFragmentAsync(m_itChunk, selectedTracks[0], c_iMaxDownloadByteCount, m_pSTInfo);
        if(pkFAILED(pkResult))
        {
            SMOOTH_MSG("DownloadFragmentAsync failed 0x%X\n", pkResult);
        }
        else
        {
            m_downloadFragmentInProgress = true;
        }
    }
}

bool SmoothTransportWorker::ShouldGetTextFragment()
{
    if(!m_apTextStream || !m_iteratorInitialized || 1.0 != m_pSTInfo->GetSpeed())
    {
        return false;
    }

    CHUNK_INFO chunkInfo;

    // find the next chunk that needs to be downloaded
    while(pkSUCCEEDED(m_apTextStream->TryGetChunkInfo(m_itChunk, &chunkInfo)))
    {
        TimeSpan_NTP chunkTime = TimeSpan_NTP::ConvertFrom(TimeSpan_hns::FromTicks(chunkInfo.chunkTime));
        
        // check if chunktime is beyond limit
        if(chunkTime - GetCurrentPlaybackTime() > TimeSpan_NTP::ConvertFrom(TimeSpan_s::FromTicks(c_iSparseFragmentLookAheadSec)))
        {
            return false;
        }

        // check if the chunk has already been downloaded
        if(m_adList.end() == m_adList.find(chunkTime))
        {
            return true;
        }
        m_itChunk.MoveNext();
    }

    return true;
}

TimeSpan_NTP SmoothTransportWorker::GetCurrentPlaybackTime()
{
    TimeSpan_NTP currentTime;
    m_pISmoothTransport->GetCurrentPlaybackTime(&currentTime);
    return currentTime;
}

ChunkIterator SmoothTransportWorker::GetCurrentTextChunkIterator()
{
    TimeSpan_NTP currentTime = GetCurrentPlaybackTime();
    TimeSpanGeneric currentTimeTS;
    TimeSpanGeneric startTimeTS;

    currentTimeTS.Set(currentTime.Ticks(), currentTime.TicksPerSecond(), m_pSTInfo->GetTimeScale());
    ChunkIterator chunkIt = m_apTextStream->GetFirstInCurrentChunkList();

    CHUNK_INFO chunkInfo;

    // walk the iterator to find the next upcoming chunk
    while(pkSUCCEEDED(m_apTextStream->TryGetChunkInfo(chunkIt, &chunkInfo)))
    {
        TimeSpan_NTP chunkTime = TimeSpan_NTP::ConvertFrom(TimeSpan_hns::FromTicks(chunkInfo.chunkTime));
        
        if(chunkInfo.chunkTime >= currentTimeTS.Ticks())
        {
            break;
        }

        if(pkFAILED(chunkIt.MoveNext()))
        {
            break;
        }
    }

    return chunkIt;
}

void SmoothTransportWorker::AddAd(string url, int64_t startTime, int32_t duration)
{
    if( IsPlayingAd() )
    {
        return;
    }

    TimeSpan_NTP requestedTime = TimeSpan_NTP::ConvertFrom(TimeSpan_hns::FromTicks(startTime));
    bool timeExist = (1 == m_adList.count(requestedTime));

    if(!timeExist)
    {
        m_adList[requestedTime] = AdInfo(url, duration);
        if(1.0 == m_pSTInfo->GetSpeed())
        {
            // set if the next ad time is the next one coming up
            if(GetCurrentPlaybackTime() < requestedTime && (m_nextAd == m_adList.end() || requestedTime < m_nextAd->first ) )
            {
                m_nextAd = m_adList.find(requestedTime);
            }   
        }
    }
    else
    {
        SMOOTH_MSG("AddAd: Ignoring duplicate ad time %.3f", requestedTime.ToSeconds());
    }

    // check to see if there is any ad times that are outside of the dvr window
    map<TimeSpan_NTP, AdInfo>::iterator it = m_adList.begin(); 
    while(it != m_adList.end())
    {
        map<TimeSpan_NTP, AdInfo>::iterator deleteIt = it;
        it++;
        if(deleteIt->first < m_pSTInfo->GetStartTime())
        {
            m_adList.erase(deleteIt);
        }
        else
        {
            break;
        }
    }
}

void SmoothTransportWorker::CheckToPlayAd()
{
    if(m_nextAd != m_adList.end())
    {
        TimeSpan_NTP currentTime = GetCurrentPlaybackTime();

        // figure out whether an ad should be played right now
        if(currentTime > m_nextAd->first && 1.0 == m_pSTInfo->GetSpeed())
        {
            SMOOTH_MSG("\n");
            SMOOTH_MSG("<---------[%.3f]Playing Ad@%.3f for %.1f sec %s--------->\n", 
                currentTime.ToSeconds(), 
                m_nextAd->first.ToSeconds(),
                m_nextAd->second.duration.ToSeconds(),
                m_nextAd->second.url.c_str());
            SMOOTH_MSG("\n");

            // calculate where to resume the main URL
            m_resumeMainURLTime = (m_pSTInfo->IsLive() ? m_nextAd->first + m_nextAd->second.duration : m_nextAd->first);

            m_nextAd->second.played = true;
            
            pkRESULT pkR = m_pISmoothTransport->Open(m_nextAd->second.url, SmoothTransportProtocol_Mbr, true);
            if(pkFAILED(pkR))
            {
                SMOOTH_MSG("SmoothTranportWorker Open Ad %s failed 0x%X\n", m_nextAd->second.url.c_str(), pkR);
            }
            else
            {
                SetIsPlayingAd(true);
            }
        }
    }
}

void SmoothTransportWorker::ResumeMainPlayback()
{
    pkRESULT pkR = pkS_OK;           
    if( IsPlayingAd() )
    {
        SMOOTH_MSG("\n");
        SMOOTH_MSG("<---------[%.3f]Resuming main URL@%.3f--------->\n", GetCurrentPlaybackTime().ToSeconds(), m_resumeMainURLTime.ToSeconds());
        SMOOTH_MSG("\n");

        // store the resume position for manifest ready know where to seek to
        m_pSTInfo->SetPlayAtPos(m_resumeMainURLTime); 

        pkR = m_pISmoothTransport->Open(m_mainURL, SmoothTransportProtocol_Mbr, false);      
        
        if(pkFAILED(pkR))
        {
            SMOOTH_MSG("SmoothTranportWorker Switching to mainURL %s failed 0x%X\n", m_mainURL.c_str(), pkR);
        }
        else
        {
            SetIsPlayingAd(false);
        }
    }
}

void SmoothTransportWorker::UpdateState(ETransportState newState)
{
    pkRESULT pkResult = pkS_OK;
    std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;
    if(eTransportState_MediaEnded == newState || eTransportState_MediaError == newState)
    {
        ResumeMainPlayback();
    }
    else if(eTransportState_Rendering == newState)
    {
        if( (g_bPrintText || g_bPlayAd) && !IsPlayingAd() && m_apTextStream)
        {  
            m_itChunk = GetCurrentTextChunkIterator();
            m_iteratorInitialized = true;

            // start a download now if needed
            if(!m_downloadFragmentInProgress)
            {
                DownloadTextFragment();
            }
        }
    }
    else if(eTransportState_Opened == newState)
    {
        m_downloadFragmentInProgress = false;
        m_iteratorInitialized = false;
    }
}

void SmoothTransportWorker::CancelNextAd()
{
    m_nextAd = m_adList.end();
}

void SmoothTransportWorker::Reset()
{
    AutoLock al( &m_Lock );
    m_adList.clear();
    m_nextAd = m_adList.end();
    m_isPlayingAd = false;
    m_resumeMainURLTime.ResetTicks();
    m_apTextStream.Set(NULL);
    m_downloadFragmentInProgress = false;
    m_iteratorInitialized = false;
}

bool SmoothTransportWorker::IsPlayingAd()
{
    AutoLock al( &m_Lock );
    return m_isPlayingAd;
}

void SmoothTransportWorker::SetIsPlayingAd(bool value)
{
    AutoLock al( &m_Lock );
    m_isPlayingAd = value;
}

// ========================================================================
// SmoothTransportInfo Implementation
// ========================================================================
SmoothTransportInfo::SmoothTransportInfo()
    : m_streamSelectedEvent(CEventHelper::eResetModeAuto)
    , m_pISmoothTransport(NULL)
    , m_pSmoothTransportWorker(NULL)
    , m_liveEnded(false)
{
}

SmoothTransportInfo::SmoothTransportInfo(ISmoothTransport* pISmoothTransport)
    : m_streamSelectedEvent(CEventHelper::eResetModeAuto)
    , m_pISmoothTransport(pISmoothTransport)
    , m_pSmoothTransportWorker(NULL)
    , m_liveEnded(false)
{
}

SmoothTransportInfo::~SmoothTransportInfo()
{
    m_apManifest.Release();
}

void SmoothTransportInfo::Initialize(SmoothTransportWorker* pSmoothTransportWorker)
{
    m_pSmoothTransportWorker = pSmoothTransportWorker;
}

//
// StreamSelectedCallback
// 
void SmoothTransportInfo::StreamSelectedCallback(_In_ StreamSelectedEventArgs* pEventArgs)
{
    if (pkSUCCEEDED(pEventArgs->Result))
    {
        for (size_t i = 0; i < pEventArgs->StreamChanges.size(); ++i)
        {
            SMOOTH_MSG("SmoothTransportInfo::StreamSelectedCallback Successfully %s stream %ls\n",
                pEventArgs->StreamChanges[i].Action == StreamChangedEventArgs::StreamSelected ? "selected" : "deselected",
                pEventArgs->StreamChanges[i].pStream->Name().c_str());
        }
    }

    m_streamSelectedEvent.Set();
}

//
// ManifestReadyCallback
// 
void SmoothTransportInfo::ManifestReadyCallback(IManifest* pManifest, pkRESULT manifestResult)
{
    if (pkFAILED(manifestResult))
    {
        SMOOTH_MSG("FAILED ManifestReadyCallback: 0x%X - no manifest processed!\n", manifestResult);
    }
    else // succeeded
    {
        load_media_succuss = true;
        SMOOTH_MSG("SmoothTransportInfo::ManifestReadyCallback:  - succeeded!\n");
        m_pSmoothTransportWorker->RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_UpdateState, 0, (int32_t) SmoothTransportWorker::eTransportState_Opened);
        SetManifest(pManifest);
        UpdateStartEndTime(TimeSpan_NTP::FromTicks(pManifest->StartTime()), 
            TimeSpan_NTP::FromTicks(pManifest->StartTime()) +  TimeSpan_NTP::ConvertFrom(TimeSpan_hns::FromTicks(pManifest->Duration())));

        shouldAutoPlay = pManifest->IsLive();
		isLiveUrl = (int32_t)pManifest->IsLive();

        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestStream> > selectedStreams;
        AutoRefPtr<IManifestStream> apTextStream;

        pManifest->GetAvailableStreams(&availableStreams);
        pManifest->GetSelectedStreams(&selectedStreams);
        uint64_t startTime = pManifest->StartTime();
        int64_t duration = pManifest->Duration();

        SMOOTH_MSG("[%.3f] Successfully get the %s manifest [%.3f, %.3f] with %d available streams and %d selected streams\n",
            (double)Executive_GetTickCount() / 1000.0,
            (ManifestType_Segmented == pManifest->Type()) ? "seg" : "stndrd",
            (double)NTP_UINT64TO10MHZ(startTime) / 1.0e7,
            (double)(NTP_UINT64TO10MHZ(startTime) + duration) / 1.0e7,
            availableStreams.size(),
            selectedStreams.size());

        // Select the first video, first audio and first text stream
        bool fFoundVideo = false;
        bool fFoundAudio = false;
        bool fFoundText = false;

        std::vector< AutoRefPtr<IManifestStream> > proposedSelection;
        for (size_t ixStream = 0; ixStream < availableStreams.size(); ++ixStream)
        {
            SMOOTH_MSG("available stream %d[%s] type=%s, streamType=%d, subType=%s, Max[%d,%d], Display[%d,%d] lang=%s, timescale=%ld\n", ixStream, 
                wstring2string(availableStreams[ixStream]->Name()).c_str(),
                (ManifestType_Segmented == pManifest->Type()) ? "seg" : "stndrd",
                availableStreams[ixStream]->Type(),
                wstring2string(availableStreams[ixStream]->SubType()).c_str(), 
                availableStreams[ixStream]->MaxWidth(), 
                availableStreams[ixStream]->MaxHeight(), 
                availableStreams[ixStream]->DisplayWidth(), 
                availableStreams[ixStream]->DisplayHeight(), 
                wstring2string(availableStreams[ixStream]->Language()).c_str(),
                availableStreams[ixStream]->TimeScale());

            if (MediaStreamTypeVideo == availableStreams[ixStream]->Type())
            {
                if (!fFoundVideo)
                {
                    proposedSelection.push_back( availableStreams[ixStream] );
                    fFoundVideo = true;
                }
            }
            else if (MediaStreamTypeAudio == availableStreams[ixStream]->Type())// && ixStream == 4)
            {
                if (!fFoundAudio)
                {
                    proposedSelection.push_back( availableStreams[ixStream] );
                    fFoundAudio = true;
                }
            }
            else
            {
                if (!fFoundText && !g_bPlayAd)
                {
                    string subtype = wstring2string(availableStreams[ixStream]->SubType());
                    if (subtype == "SUBT")
                    {
                        proposedSelection.push_back( availableStreams[ixStream] );
                        apTextStream = availableStreams[ixStream];
                        fFoundText = true;
                        SMOOTH_MSG("fFoundText = true, ixStream=%d, subType=%s\n", ixStream, subtype.c_str());
                    }
                }
            }
        }

        if (fFoundVideo || fFoundAudio)
        {
            if (g_bPlayAd)
            {
                // if playing ad try to find a sparse stream in the selected streams
                for (size_t ixStream = 0; ixStream < proposedSelection.size(); ++ixStream)
                {
                    std::vector< AutoRefPtr<IManifestStream> > childStreams;
                    proposedSelection[ixStream]->GetChildStreams(&childStreams);
                    if(childStreams.size() > 0)
                    {
                        proposedSelection.push_back( childStreams.front() );
                        apTextStream = childStreams.front();
                        fFoundText = true;
                        break;
                    }
                }

                if (!fFoundText && !m_pSmoothTransportWorker->IsPlayingAd())
                {
                    SMOOTH_MSG("FAILED to find sparse stream\n");
                }
            }

            pkRESULT pkResult = pManifest->SelectStreamsAsync(this, proposedSelection);
            if (pkSUCCEEDED(pkResult))
            {
                m_streamSelectedEvent.Wait();
            }
            else
            {
                SMOOTH_MSG("FAILED to SelectStreamsAsync: 0x%X\n", pkResult);
                PrintStreamList( proposedSelection );
            }
        }

        if (c_VALUE_NOT_SET != g_iMinBitrateKbps
            || c_VALUE_NOT_SET != g_iMaxBitrateKbps)
        {
            // select/restrict requested video tracks
            LimitVideoTracks(pManifest, g_iMinBitrateKbps, g_iMaxBitrateKbps, g_bRestrictTracks);
        }
        
        // store text stream
        if(fFoundText && (g_bPrintText || g_bPlayAd))
        {
            m_pSmoothTransportWorker->SetTextStream(apTextStream);
        }
        
        if (c_VALUE_NOT_SET != g_iMinRangeDeltaSec)
        {
            TimeSpan_NTP minTime = TimeSpan_NTP::FromTicks(startTime);
            minTime = minTime + TimeSpan_NTP::ConvertFrom(TimeSpan_s::FromTicks(g_iMinRangeDeltaSec));
            SMOOTH_MSG("In ManifestReady setting playback range to %.3f sec\n", minTime.ToSeconds());
            pkRESULT pkResult = m_pISmoothTransport->SetPlaybackRangeAsync( this, minTime );
            if (pkFAILED(pkResult))
            {
                SMOOTH_MSG("FAILED to SetPlaybackRangeAsync in ManifestReady: 0x%X\n", pkResult);
            }
        }

        TimeSpan_NTP playAtTime = GetPlayAtPos();
        if(playAtTime.Ticks() > 0)
        {
            m_pSmoothTransportWorker->RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Seek, 1.0, (int32_t)playAtTime.ToSeconds());
        }
    }
}

//
// ErrorCallback
// 
void SmoothTransportInfo::ErrorCallback(SmoothTransportError& errorInfo)
{
    m_pSmoothTransportWorker->RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_UpdateState, 0, (int32_t) SmoothTransportWorker::eTransportState_MediaError);
    SMOOTH_MSG("[%.3f] TransportError: %s, pkResult: 0x%08x, httpResponse: %d\n",
        (double)Executive_GetTickCount() / 1000.0,
        GetErrorName(errorInfo._errorCode),
        errorInfo._pkResult,
        errorInfo._httpResponse);
}

const char* SmoothTransportInfo::GetErrorName( ESmoothTransportError error)
{
    switch (error)
    {
        case SmoothTransportError_None                              : return "None";
        case SmoothTransportError_Unknown                           : return "Unknown";

        // tuner errors
        case SmoothTransportError_TunerAllocationFailure            : return "TunerAllocationFailure";
        case SmoothTransportError_TunerSharedReceivers              : return "TunerSharedReceivers";

        //Manifest errors
        case SmoothTransportError_ManifestParseFailed               : return "ManifestParseFailed";
        case SmoothTransportError_ManifestVersionUnsupported        : return "ManifestVersionUnsupported";
        case SmoothTransportError_ManifestInvalid                   : return "ManifestInvalid";
        case SmoothTransportError_ManifestHttpInvalidResult         : return "ManifestHttpInvalidResult";

        // Socket errors
        case SmoothTransportError_SocketAlreadyClosed               : return "SocketAlreadyClosed";
        case SmoothTransportError_SocketReadError                   : return "SocketReadError";
        case SmoothTransportError_SocketOpenFailed                  : return "SocketOpenFailed";
        case SmoothTransportError_SocketConnectFailed               : return "SocketConnectFailed";
        case SmoothTransportError_SocketSendFailed                  : return "SocketSendFailed";
        case SmoothTransportError_SocketRecvFailed                  : return "SocketRecvFailed";

        //HTTP errors
        case SmoothTransportError_HttpParseResponseFailed           : return "HttpParseResponseFailed";
        case SmoothTransportError_HttpInvalidResult                 : return "HttpInvalidResult";
        case SmoothTransportError_HttpTooManyRedirect               : return "HttpTooManyRedirect";
        case SmoothTransportError_HttpRedirectFailed                : return "HttpRedirectFailed";
        case SmoothTransportError_HttpRedirectNotAllowed            : return "HttpRedirectNotAllowed";
        case SmoothTransportError_HttpCreateFailed                  : return "HttpCreateFailed";

        //Chunk socket
        case SmoothTransportError_ChunkConnectHttpInvalidResult     : return "ChunkConnectHttpInvalidResult";
        case SmoothTransportError_ChunkNextHttpInvalidResult        : return "ChunkNextHttpInvalidResult";
        case SmoothTransportError_ChunkHdrParseFailed               : return "ChunkHdrParseFailed";
        case SmoothTransportError_ChunkInvalidData                  : return "ChunkInvalidData";

        //Drm
        case SmoothTransportError_DrmInitFailed                     : return "DrmInitFailed";
        }
        return "*invalid*";
}

//
// StatusCallback
//
void SmoothTransportInfo::StatusCallback(SmoothTransportStatus& status)
{
    static int latestBitrateKbps = 0;

    // if quality level is to be forced, send the command once the sockets are up and running
    if (SmoothTransportStatus_Rendering == status._update)
    {
        m_pSmoothTransportWorker->RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_UpdateState, 0, (int32_t) SmoothTransportWorker::eTransportState_Rendering);
        if (g_szForcedQualityLevel)
        {
            SendExtendedCommand(m_pISmoothTransport, "forcequalitylevel", 1, &g_szForcedQualityLevel);
        }
    }
    // if at window edge default to playing
    else if (SmoothTransportStatus_AtWindowEdge == status._update)
    {
        m_pSmoothTransportWorker->RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Play);
    }
    else if (SmoothTransportStatus_TunerStateChanged == status._update)
    {
        if (SmoothTransportTunerState_MediaEnded == status._currentState)
        {
            m_pSmoothTransportWorker->RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_UpdateState, 0, (int32_t) SmoothTransportWorker::eTransportState_MediaEnded);
            // auto play if media ended is reached on live presentations
            if(shouldAutoPlay)
            {
                m_pSmoothTransportWorker->RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Play);
            }
        }
    }
    else if (SmoothTransportStatus_EndOfLive == status._update)
    {
        shouldAutoPlay = false;
        SetLiveEnded();
    }
    else if (SmoothTransportStatus_BitrateChanged == status._update)
    {
        latestBitrateKbps = (atoi(status._additionalInfo.c_str()) + 500)/ 1000;
    }
    else if (SmoothTransportStatus_StartEndTime == status._update)
    {
        UpdateStartEndTime(status._startTime, status._endTime);
    }

    int startTimeSecs = (int)status._startTime.ToSeconds() % 86400;
    int durationSecs = (int)( status._endTime - status._startTime ).ToSeconds() % 86400;

    UpdateSpeed(status._speed);
    
    if (SmoothTransportStatus_Heartbeat != status._update)
    {
        SMOOTH_MSG("[%.3f] Status: %s (%s) [%.3f, %.3f, %.3f] (%.3f) [%02d:%02d:%02d (%02d:%02d)] kbps: %d\n",
            (double)Executive_GetTickCount() / 1000.0,
            GetTunerStateName(status._currentState),
            GetStatusUpdateName(status._update),
            status._startTime.ToSeconds(),
            status._currentTime.ToSeconds(),
            status._endTime.ToSeconds(),
            ( status._currentTime > status._startTime ) ? ( status._currentTime - status._startTime ).ToSeconds() : 0,
            (startTimeSecs / 3600),      // hours
            (startTimeSecs % 3600) / 60, // minutes
            (startTimeSecs % 60),        // seconds
            (durationSecs / 60),         // duration minutes
            (durationSecs % 60),         // duration seconds
            latestBitrateKbps);
    }

    if (status._pkResult != pkS_OK || status._httpResponse != 0)
    {
        SMOOTH_MSG("Status pkResult: 0x%08x httpResponse: %d\n",
            status._pkResult,
            status._httpResponse);
    }

    if (!status._additionalInfo.empty())
    {
        SMOOTH_MSG("Status Info: %s\n", status._additionalInfo.c_str());
    }
}

const char* SmoothTransportInfo::GetTunerStateName( ESmoothTransportState SmoothTransportState)
{
    switch (SmoothTransportState)
    {
        case SmoothTransportTunerState_Unknown:     return "Unknown  ";
        case SmoothTransportTunerState_Tuning:      return "Tuning   ";
        case SmoothTransportTunerState_Playing:     return "Playing  ";
        case SmoothTransportTunerState_Paused:      return "Paused   ";
        case SmoothTransportTunerState_MediaEnded:  return "MediaEnd ";
        case SmoothTransportTunerState_Detuned:     return "Detuned  ";
        case SmoothTransportTunerState_Closed:      return "Closed   ";
        case SmoothTransportTunerState_Max:         return "Max";
    }
    return "*invalid*";
}

const char* SmoothTransportInfo::GetStatusUpdateName( ESmoothTransportStatusUpdate SmoothTransportStatusUpdate)
{
    switch (SmoothTransportStatusUpdate)
    {
        case SmoothTransportStatus_Unknown:                 return "Unknown      ";
        case SmoothTransportStatus_Heartbeat:               return "Heartbeat    ";
        case SmoothTransportStatus_TunerStateChanged:       return "TunerState   ";
        case SmoothTransportStatus_Streaming:               return "Streaming    ";
        case SmoothTransportStatus_Rendering:               return "Rendering    ";
        case SmoothTransportStatus_Underrun:                return "Underrun     ";
        case SmoothTransportStatus_Rebuffer:                return "Rebuffer     ";
        case SmoothTransportStatus_StartEndTime:            return "StartEndTime ";
        case SmoothTransportStatus_DrmStateChanged:         return "DrmState     ";
        case SmoothTransportStatus_BitrateChanged:          return "BPS change   ";
        case SmoothTransportStatus_DecoderError:            return "Decoder err  ";
        case SmoothTransportStatus_ChunkConnectHttpInvalid: return "ChunkConnHttp";
        case SmoothTransportStatus_NextChunkHttpInvalid:    return "NextChunkHttp";
        case SmoothTransportStatus_ChunkHdrHttpInvalid:     return "ChunkHdrHttp ";
        case SmoothTransportStatus_ChunkHdrError:           return "ChunkHdrErr  ";
        case SmoothTransportStatus_AtWindowEdge:            return "AtWndwEdge   ";
        case SmoothTransportStatus_EndOfLive:               return "EndOfLive    ";
        case SmoothTransportStatus_OutsideWindowEdge:       return "OutsideWndw  ";
        case SmoothTransportStatus_SegmentManifestError:    return "SegManfstErr ";
        case SmoothTransportStatus_DrmInitError:            return "DrmInitErr   ";
        case SmoothTransportStatus_Max:                     return "Max          ";
    }
    return "*invalid*";
}

// ========================================================================
// SamplePlaybackRangeCallback Implementation
// ========================================================================

void SmoothTransportInfo::SetPlaybackRangeCallback(IManifest* pManifest, pkRESULT result)
{
    m_apManifest.Set(pManifest);
                    
    uint64_t startTime = pManifest->StartTime();
    int64_t duration = pManifest->Duration();

    UpdateStartEndTime(TimeSpan_NTP::FromTicks(startTime), 
            TimeSpan_NTP::FromTicks(startTime) +  TimeSpan_NTP::ConvertFrom(TimeSpan_hns::FromTicks(duration)));

    if (pkFAILED(result))
    {   
        SMOOTH_MSG("[%.3f] Failed 0x%X SamplePlaybackRangeCallback [%.3f, %.3f]\n",
                (double)Executive_GetTickCount() / 1000.0,
                result,
                (double)NTP_UINT64TO10MHZ(startTime) / 1.0e7,
                (double)(NTP_UINT64TO10MHZ(startTime) + duration) / 1.0e7);
    }
    else
    {
        SMOOTH_MSG("[%.3f] Successful SamplePlaybackRangeCallback [%.3f, %.3f]\n",
                (double)Executive_GetTickCount() / 1000.0,
                (double)NTP_UINT64TO10MHZ(startTime) / 1.0e7,
                (double)(NTP_UINT64TO10MHZ(startTime) + duration) / 1.0e7);
    }
}

//
// SampleFragmentCallback 
//
pkRESULT SmoothTransportInfo::OnFragmentData(
                        _In_ pkRESULT hrResult,
                        _In_ CHUNK_INFO* pChunkInfo,
                        _In_opt_ IRefBuffer* pBuffer,
                        _In_ bool fFinalBuffer,
                        _In_ size_t cbTotalLength )
{
    if(pkSUCCEEDED(hrResult) && pBuffer)
    {
        if(g_bPrintText)
        {
            string strData( (const char *)pBuffer->Data(), 
                pBuffer->Length() > c_iMaxPrintOutByteCount ? c_iMaxPrintOutByteCount : pBuffer->Length());
            //string strData( (const char *)pBuffer->Data(), pBuffer->Length());

            SMOOTH_ADAPTER_MSG("SmoothTransportInfo::OnFragmentData [%.3f, ts: %.3f, len: %d] %s", 
                (double)Executive_GetTickCount() / 1000.0, 
                pChunkInfo->chunkTime/ 1.0e7,
                pBuffer->Length(),
                strData.c_str());

            // indicate that print out was truncated
            if(pBuffer->Length() > c_iMaxPrintOutByteCount || !fFinalBuffer)
            {
                SMOOTH_ADAPTER_MSG("[...]\n");
            }
            else
            {
                SMOOTH_ADAPTER_MSG("\n");
            }
        }

        ASSERT(fFinalBuffer);
        if(g_bPlayAd)
        {
            //expected format is [durationSec];[adUrl]
            string strData( (const char *)pBuffer->Data(), pBuffer->Length());
            std::vector<string> splitData;
            stringSplit(strData, splitData, ";");
            if(splitData.size() > 1 && splitData[1].find("http://") != string::npos)
            {
                m_pSmoothTransportWorker->RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_AddAd,     // command
                                                                        1.0,                                                // not used
                                                                        stringToInt(splitData[0].c_str()),                  // ad duration
                                                                        splitData[1],                                       // ad url
                                                                        pChunkInfo->chunkTime);                             // time to play ad
            }
            else
            {
                SMOOTH_ERR_MSG("Not adding ad since data does not match expected format: [durationSec];[adUrl]\n");
            }
        }
    }
    else
    {
        if( hrResult == pkE_NO_MORE_ITEMS )
        {
            m_pSmoothTransportWorker->RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_FragmentNoMoreItems);
        }
        else
        {
            SMOOTH_ERR_MSG("SmoothTransportInfo::OnFragmentData FAILED OnFragmentData\n");
            //SMOOTH_ERR_MSG("[%.3f, %.3f] FAILED OnFragmentData 0x%X\n", 
            //    (double)Executive_GetTickCount() / 1000.0, 
            //    pChunkInfo->chunkTime/ 1.0e7, 
            //    hrResult);
        }
    }

    m_pSmoothTransportWorker->RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_FragmentDataDone);
    return pkS_OK;
}

// ========================================================================
// SmoothTransportInfo Implementation
// ========================================================================
bool SmoothTransportInfo::IsLive()
{
    AutoLock al( &m_Lock );
    return (!m_apManifest->IsLive() ? false : !m_liveEnded);
}
 
void SmoothTransportInfo::SetManifest(IManifest* pManifest)
{
     AutoLock al( &m_Lock );
     m_apManifest.Set(pManifest);
}

IManifest* SmoothTransportInfo::GetManifest()
{
     AutoLock al( &m_Lock );
     return m_apManifest;
}
   
void SmoothTransportInfo::UpdateStartEndTime(TimeSpan_NTP startTime, TimeSpan_NTP endTime)
 {
     AutoLock al( &m_Lock );
     m_startTime = startTime;
     m_endTime = endTime;
 }
    
TimeSpan_NTP SmoothTransportInfo::GetStartTime()
{
    AutoLock al( &m_Lock );
    return m_startTime;
}

void SmoothTransportInfo::UpdateSpeed(float newSpeed)
{
    AutoLock al( &m_Lock );
    m_currentSpeed = newSpeed;
}

float SmoothTransportInfo::GetSpeed()
{
    AutoLock al( &m_Lock );
    return m_currentSpeed;
}

int64_t SmoothTransportInfo::GetTimeScale() const
{
    return m_apManifest ? m_apManifest->TimeScale() : 0;
}

void SmoothTransportInfo::SetLiveEnded()
{
    AutoLock al( &m_Lock );
    m_liveEnded = true;
}

void SmoothTransportInfo::SetPlayAtPos(TimeSpan_NTP time)
{
     AutoLock al( &m_Lock );
     m_playAtTime = time;
}
 
TimeSpan_NTP SmoothTransportInfo::GetPlayAtPos()
{
     AutoLock al( &m_Lock );
     TimeSpan_NTP playAtTime = m_playAtTime;
     m_playAtTime.ResetTicks();

     return playAtTime;
}

void SmoothTransportInfo::Reset()
{
    AutoLock al( &m_Lock );
    m_liveEnded = false;
    m_playAtTime.ResetTicks();
    m_startTime.ResetTicks();
    m_endTime.ResetTicks();
    m_currentSpeed = 1.0;
}

// ========================================================================
// CXDrmDiagDelegate Implementation
// ========================================================================
CXDrmDiagDelegate::CXDrmDiagDelegate()
    : m_hLog(NULL)
{
}

CXDrmDiagDelegate::~CXDrmDiagDelegate()
{
    if (m_hLog)
    {
        TF_Logging_Close(m_hLog);
        m_hLog = NULL;
    }
}

pkRESULT CXDrmDiagDelegate::OpenLogFile( const char* szLogFileUri )
{
    return TF_Logging_Open(&m_hLog, szLogFileUri);
}

void CXDrmDiagDelegate::OnDecrypt( const IXDrmDiagDelegate::SDecryptInfo& sDecryptInfo )
{
    if (NULL == m_hLog)
    {
        SMOOTH_MSG("ERROR OnDecrypt called with no log file opened\n");
        return;
    }

    if (sDecryptInfo.cbKeyID != 16)
    {
        TF_Logging_Printf(m_hLog, "ERROR OnDecrypt key ID length %u != 16\n", sDecryptInfo.cbKeyID);
        return;
    }

    TF_Logging_Printf(m_hLog, "Decrypt(NOCHAIN)Info [0x%016llx %d %08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x 0x%08x] %llu\n",
        sDecryptInfo.qwSampleID,
        (int)(sDecryptInfo.fIsAES ? 1 : 0),

        (unsigned int) ((uint32_t*)&sDecryptInfo.pbKeyID[0]),
        (unsigned int) ((uint16_t*)&sDecryptInfo.pbKeyID[4]),
        (unsigned int) ((uint16_t*)&sDecryptInfo.pbKeyID[6]),
        (unsigned int) sDecryptInfo.pbKeyID[8],
        (unsigned int) sDecryptInfo.pbKeyID[9],
        (unsigned int) sDecryptInfo.pbKeyID[10],
        (unsigned int) sDecryptInfo.pbKeyID[11],
        (unsigned int) sDecryptInfo.pbKeyID[12],
        (unsigned int) sDecryptInfo.pbKeyID[13],
        (unsigned int) sDecryptInfo.pbKeyID[14],
        (unsigned int) sDecryptInfo.pbKeyID[15],

        sDecryptInfo.crc32PSSH,
        sDecryptInfo.qwOffset );
}

void CXDrmDiagDelegate::OnDecryptBufferChain( const IXDrmDiagDelegate::SDecryptInfo& sDecryptInfo )
{
    if (NULL == m_hLog)
    {
        SMOOTH_MSG("ERROR OnDecryptBufferChain called with no log file opened\n");
        return;
    }

    if (sDecryptInfo.cbKeyID != 16)
    {
        TF_Logging_Printf(m_hLog, "ERROR OnDecryptBufferChain key ID length %u != 16\n", sDecryptInfo.cbKeyID);
        return;
    }

    TF_Logging_Printf(m_hLog, "DecryptInfo [0x%016llx %d %08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x 0x%08x] %llu\n",
        sDecryptInfo.qwSampleID,
        (int)(sDecryptInfo.fIsAES ? 1 : 0),

        (unsigned int) ((uint32_t*)&sDecryptInfo.pbKeyID[0]),
        (unsigned int) ((uint16_t*)&sDecryptInfo.pbKeyID[4]),
        (unsigned int) ((uint16_t*)&sDecryptInfo.pbKeyID[6]),
        (unsigned int) sDecryptInfo.pbKeyID[8],
        (unsigned int) sDecryptInfo.pbKeyID[9],
        (unsigned int) sDecryptInfo.pbKeyID[10],
        (unsigned int) sDecryptInfo.pbKeyID[11],
        (unsigned int) sDecryptInfo.pbKeyID[12],
        (unsigned int) sDecryptInfo.pbKeyID[13],
        (unsigned int) sDecryptInfo.pbKeyID[14],
        (unsigned int) sDecryptInfo.pbKeyID[15],

        sDecryptInfo.crc32PSSH,
        sDecryptInfo.qwOffset );
}

//#ifndef SS_ADAPTER_USED_LIB
#if 1
// ========================================================================
// LaunchSmoothTransport Implementation
// ========================================================================
#ifdef SS_ADAPTER_USED_TEST
static void LaunchSmoothTransport(string url)
{
    SSAdapter_Test(url);
    return;
}
#else
static void LaunchSmoothTransport(string url)
{
    pkRESULT pkResult = pkS_OK;
    ISmoothTransport* pISmoothTransport = ISmoothTransport::CreateSmoothTransport();
    SmoothTransportInfo smoothTransportInfo(pISmoothTransport);
    SmoothTransportWorker smoothTransportWorker(pISmoothTransport, &smoothTransportInfo);
    smoothTransportInfo.Initialize(&smoothTransportWorker);

    CXDrmDiagDelegate drmDiagDelegate;
    IXDrm* poXDrm = NULL;

    CLicenseAcquirer licenseAcquirer;
    pkResult = licenseAcquirer.Initialize();
    if (pkFAILED(pkResult))
    {
        SMOOTH_MSG("FAILED to initialize LicenseAcquirer: 0x%X\n", pkResult);
    }

    //Register status/error callback.
    pkResult = pISmoothTransport->RegisterStatusCallback(&smoothTransportInfo);
    if (pkFAILED(pkResult))
    {
        SMOOTH_MSG("FAILED to register status callback: 0x%X\n", pkResult);
    }

    pkResult = pISmoothTransport->RegisterErrorCallback(&smoothTransportInfo);
    if (pkFAILED(pkResult))
    {
        SMOOTH_MSG("FAILED to register error callback: 0x%X\n", pkResult);
    }

    pkResult = pISmoothTransport->SetManifestCallback(&smoothTransportInfo);
    if (pkFAILED(pkResult))
    {
        SMOOTH_MSG("FAILED to set manifest callback: 0x%X\n", pkResult);
    }

    if (g_szHTTPProxyHost)
    {
        SendExtendedCommand(pISmoothTransport, "httpproxyhost", 1, &g_szHTTPProxyHost);
    }
    if (g_szAudioHalBufferingThreshold)
    {
        SendExtendedCommand(pISmoothTransport, "audiohalbufferingthreshold", 1, &g_szAudioHalBufferingThreshold);
    }
    if (c_FIFO_LIMIT_NOT_SET != g_iEsFifoLimitNormalPlay)
    {
        char szInt[c_MAX_INT_CHAR_STRING_SIZE];
        const char* szIntArg = szInt;

        if (g_iEsFifoLimitNormalPlay < c_MIN_FIFO_LIMIT_MS
            || g_iEsFifoLimitNormalPlay > c_MAX_FIFO_LIMIT_MS)
        {
            SMOOTH_MSG("FAILED -fifo option value (%d) outside of range (%d..%d ms)\n",
                g_iEsFifoLimitNormalPlay,
                c_MIN_FIFO_LIMIT_MS,
                c_MAX_FIFO_LIMIT_MS);
        }
        else
        {
            // convert int milliseconds to 90kHz timescale as decimal string
            pkResult = StringCbPrintfA( szInt, sizeof(szInt), "%d", 90 * g_iEsFifoLimitNormalPlay );
            if (pkFAILED(pkResult))
            {
                SMOOTH_MSG("FAILED to convert g_iEsFifoLimitNormalPlay (%d) [0x%X]\n", g_iEsFifoLimitNormalPlay, pkResult);
            }
            else
            {
                SendExtendedCommand(pISmoothTransport, "maxesfifolimitinnormalplayback90kHz", 1, &szIntArg);
            }
        }
    }
    if (c_FIFO_LIMIT_NOT_SET != g_iEsFifoLimitTrickPlay)
    {
        char szInt[c_MAX_INT_CHAR_STRING_SIZE];
        const char* szIntArg = szInt;

        if (g_iEsFifoLimitTrickPlay < c_MIN_FIFO_LIMIT_MS
            || g_iEsFifoLimitTrickPlay > c_MAX_FIFO_LIMIT_MS)
        {
            SMOOTH_MSG("FAILED -fifotrick option value (%d) outside of range (%d..%d ms)\n",
                g_iEsFifoLimitTrickPlay,
                c_MIN_FIFO_LIMIT_MS,
                c_MAX_FIFO_LIMIT_MS);
        }
        else
        {
            // convert int milliseconds to 90kHz timescale as decimal string
            pkResult = StringCbPrintfA( szInt, sizeof(szInt), "%d", 90 * g_iEsFifoLimitTrickPlay );
            if (pkFAILED(pkResult))
            {
                SMOOTH_MSG("FAILED to convert g_iEsFifoLimitTrickPlay (%d) [0x%X]\n", g_iEsFifoLimitTrickPlay, pkResult);
            }
            else
            {
                SendExtendedCommand(pISmoothTransport, "maxesfifolimitintrickplayback90kHz", 1, &szIntArg);
            }
        }
    }
    if (g_szLivePlaybackOffsetSecs)
    {
        SendExtendedCommand(pISmoothTransport, "ssliveplaybackoffsetsec", 1, &g_szLivePlaybackOffsetSecs);
    }
    if (g_szLiveBackOffSecs)
    {
        SendExtendedCommand(pISmoothTransport, "sslivebackoffsec", 1, &g_szLiveBackOffSecs);
    }
    if (g_szLiveMinTimeBufferSecs)
    {
        SendExtendedCommand(pISmoothTransport, "sslivemintimebuffersec", 1, &g_szLiveMinTimeBufferSecs);
    }
    if (g_szHttpResponseTimeout)
    {
        SendExtendedCommand(pISmoothTransport, "httpresponsetimeout", 1, &g_szHttpResponseTimeout);
    }
    if (g_szHttpInitialReceiveTimeout)
    {
        SendExtendedCommand(pISmoothTransport, "httpinitialreceivetimeout", 1, &g_szHttpInitialReceiveTimeout);
    }
    if (g_szHttpSubsequentReceiveTimeout)
    {
        SendExtendedCommand(pISmoothTransport, "httpsubsequentreceivetimeout", 1, &g_szHttpSubsequentReceiveTimeout);
    }
    if(g_szChunkListMaxSize)
    {
        SendExtendedCommand(pISmoothTransport, "chunklistmaxsize", 1, &g_szChunkListMaxSize);
    }
    if (g_szDrmServerUrl)
    {
        std::string ResponseCustomData;

        if (g_szDrmCustomData)
        {
            licenseAcquirer.ChallengeCustomData = g_szDrmCustomData;
        }

        pkResult = licenseAcquirer.AcquireLicense(g_szDrmServerUrl, g_szLicenseKeyId, &ResponseCustomData);
        if (pkFAILED(pkResult))
        {
            SMOOTH_MSG("FAILED to AcquireLicense from %s: 0x%X\n", g_szDrmServerUrl, pkResult);
        }
        else if (0 != ResponseCustomData.length())
        {
            SMOOTH_MSG("AcquireLicense response custom data:\n%s\n", ResponseCustomData.c_str());
        }
    }
    if (g_szDrmLogFilePath)
    {
        pkResult = drmDiagDelegate.OpenLogFile(g_szDrmLogFilePath);
        if (pkFAILED(pkResult))
        {
            SMOOTH_MSG("FAILED to open log file %s: 0x%X\n", g_szDrmLogFilePath, pkResult);
        }

        pkResult = XDRM_CreateInstance(&poXDrm);
        if (pkFAILED(pkResult))
        {
            SMOOTH_MSG("FAILED to XDRM_CreateInstance 0x%X\n", pkResult);
        }
        else if (!poXDrm->SetDiagDelegate(&drmDiagDelegate))
        {
            SMOOTH_MSG("FAILED to SetDiagDelegate\n");
        }
    }

    SendExtendedCommand(pISmoothTransport, "useragentstring", 1, &g_szUserAgentString);

#ifdef DIAG_ENABLE
    IDiagsManager* diagManager = IAVManager::Instance()->GetDiagsManager();
    pkHANDLE hDiagThread = 0;

    if (diagManager)
    {
        DiagSetChannelPriority( kDiagChannel_Heuristics, kDiagsPriority_Medium );
        DiagSetChannelPriority( kDiagChannel_Manifest, kDiagsPriority_Medium );
        DiagSetChannelPriority( kDiagChannel_ChunkList, kDiagsPriority_Medium );
        DiagSetChannelPriority( kDiagChannel_FragInfo, kDiagsPriority_Medium );

        Executive_CreateThread(&DiagEventRetrieveFun, diagManager, 0, &hDiagThread);

        // only turn on diagnostics if the thread is created
        if ( hDiagThread )
        {
            diagManager->RegisterFilters(true);
        }
    }
#endif // DIAG_ENABLE
    smoothTransportWorker.Initialize();
    url = stringEscSpaces(stringTrim(url, " "));

    smoothTransportWorker.RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Open, 0, 0, url);

#ifdef MSPK_USE_TELNET_CONSOLE
    if (!ConsoleOpen(TELNET_CONSOLE_PORT))
    {
        goto _error_exit;
    }
#endif
    while (!g_IsAbandoned)
    {
        string s = "";
#ifdef MSPK_USE_TELNET_CONSOLE
        {
            char buf[4096];
            ConsoleRead(buf, sizeof(buf));
            s = buf;
        }
#else
        SMOOTH_MSG("> ");
        getline(cin, s);
#endif
        std::vector<string> arg;
        stringSplit(s, arg, " ");

        if (arg.size() == 0)
        {
            PrintCmdList(s);
        }
        else if (arg[0] == "close")
        {
            smoothTransportWorker.RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Close);
        }
        else if ((arg[0] == "exit") || (arg[0] == ("quit")))
        {
            g_IsAbandoned = true;
            break;
        }
        else if (arg[0] == "play")
        {
            speedidx = SPEED_ZERO;
            smoothTransportWorker.RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Play);
        }
        else if (arg[0] == "pause")
        {
            speedidx = SPEED_ZERO;
            smoothTransportWorker.RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Pause);
        }
        else if (arg[0] == "open")
        {
            speedidx = SPEED_ZERO;

            if (arg.size() > 1)
            {
                // no checking of "find" index values needed since arg[1] was found before
                url = stringEscSpaces(stringTrim(s.substr(s.find_first_of(" ", s.find_first_not_of(" ", 0))), " "));
            }
            smoothTransportWorker.RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Open, 0, 0, url);
        }
        else if (arg[0] == "rst")
        {
            smoothTransportWorker.RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Close);

            speedidx = SPEED_ZERO;

            if (arg.size() > 1)
            {
                // no checking of "find" index values needed since arg[1] was found before
                url = stringEscSpaces(stringTrim(s.substr(s.find_first_of(" ", s.find_first_not_of(" ", 0))), " "));
            }
            smoothTransportWorker.RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Open, 0, 0, url);
        }
        else if (arg[0] == "ff")
        {
            //check for optional play rate parameter
            if (arg.size() == 2)
            {
                int32_t playRate = stringToInt(arg[1]);
                if (playRate > 0)
                {
                    smoothTransportWorker.RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Play, (float)playRate);
                }
                else
                {
                    PrintCmdList(s);
                }
            }
            else
            {
                smoothTransportWorker.RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Play, speeds[GetNextSpeedIndex(true)]);
            }
        }
        else if (arg[0] == "rew")
        {
            //check for optional play rate parameter
            if (arg.size() == 2)
            {
                int32_t playRate = stringToInt(arg[1]);
                if (playRate > 0)
                {
                    smoothTransportWorker.RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Play, (float)(-playRate));
                }
                else
                {
                    PrintCmdList(s);
                }
            }
            else
            {
                smoothTransportWorker.RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Play, speeds[GetNextSpeedIndex(false)]);
            }
        }
        else if (arg[0] == "sf")
        {
            speedidx = SPEED_ZERO;

            //check for optional parameter
            if (arg.size() == 2)
            {
                int32_t sec = stringToInt(arg[1]);
                if (sec > 0)
                {
                    smoothTransportWorker.RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Skip, 0, sec);
                }
                else
                {
                    PrintCmdList(s);
                }
            }
            else
            {
                // skip forward kSkipForwardSecs second
                smoothTransportWorker.RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Skip, 0, kSkipForwardSecs);
            }
        }
        else if (arg[0] == "sb")
        {
            speedidx = SPEED_ZERO;

            //check for optional parameter
            if (arg.size() == 2)
            {
                int32_t sec = stringToInt(arg[1]);
                if (sec > 0)
                {
                    smoothTransportWorker.RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Skip, 0, -sec);
                }
                else
                {
                    PrintCmdList(s);
                }
            }
            else
            {
                // skip backward kSkipBackSecs second
                smoothTransportWorker.RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Skip, 0, -kSkipBackSecs);
            }
        }
        else if (arg[0] == "seek")
        {
            speedidx = SPEED_ZERO;

            if (arg.size() == 2)
            {
                int32_t timeStampSec = stringToInt(arg[1]);
                if(timeStampSec > 0)
                {
                    smoothTransportWorker.RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Seek, 0, timeStampSec);
                }
                else
                {
                    PrintCmdList(s);
                }
            }
            else
            {
                PrintCmdList(s);
            }
        }
        else if (arg[0] == "live")
        {
            speedidx = SPEED_ZERO;
            // skip forward kSkipToLiveSecs; big value
            smoothTransportWorker.RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Skip, 0, kSkipToLiveSecs);
        }
        else if (arg[0] == "begin")
        {
            speedidx = SPEED_ZERO;
            // skip backward kSkipToBeginSecs; big value
            smoothTransportWorker.RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Skip, 0, -kSkipToBeginSecs);
        }
        else if (arg[0] == "ct")
        {
            SSPK::TimeSpan_NTP timeNTP;

            pISmoothTransport->GetCurrentPlaybackTime(&timeNTP);

            double dblTimeSecs = timeNTP.ToSeconds();

#ifdef MSPK_USE_TELNET_CONSOLE
            ConsoleWrite("current time: %.3f secs\n", dblTimeSecs);
#else
            SMOOTH_MSG("current time: %.3f secs\n", dblTimeSecs);
#endif
        }
        else if (arg[0] == "nextaudio")
        {
            SwitchAudio(smoothTransportInfo);
        }
        else if (arg[0] == "togstream")
        {
            if (2 == arg.size())
            {
                ToggleStream(smoothTransportInfo, stringToWString(arg[1].c_str()));
            }
            else
            {
                SMOOTH_MSG("Invalid number of parameters.  Usage: togstream [stream name]\n");
            }
        }
        else if (arg[0] == "seltracks")
        {
            if(3 == arg.size())
            {
                LimitVideoTracks(smoothTransportInfo.GetManifest(), stringToInt(arg[1]), stringToInt(arg[2]), false);
            }
            else
            {
                SMOOTH_MSG("Invalid number of parameters.  Usage: seltracks [minKbps] [maxKbps]\n");
            }
        }
        else if (arg[0] == "adjminrange")
        {
            if (2 == arg.size())
            {      
                TimeSpan_NTP minTime = smoothTransportInfo.GetStartTime() + TimeSpan_NTP::ConvertFrom(TimeSpan_s::FromTicks(stringToInt(arg[1])));

                SMOOTH_MSG("Setting playback range to %.3f sec\n", minTime.ToSeconds());
                smoothTransportWorker.RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_SetPlaybackRange,
                                                                      0,
                                                                      0,
                                                                      "",
                                                                      minTime.Ticks());
            }
            else
            {
                SMOOTH_MSG("Invalid number of parameters.  Usage: adjminrange [+/- delta sec]\n");
            }
        }
        else
        {
            PrintCmdList(s);
        }
    }
#ifdef MSPK_USE_TELNET_CONSOLE
_error_exit:
    ConsoleClose();
#endif

    if (poXDrm)
    {
        if (!poXDrm->SetDiagDelegate(NULL))
        {
            SMOOTH_MSG("FAILED to SetDiagDelegate(NULL)\n");
        }

        pkResult = XDRM_DestroyInstance(poXDrm);
        if (pkFAILED(pkResult))
        {
            SMOOTH_MSG("FAILED to XDRM_DestroyInstance 0x%X\n", pkResult);
        }
    }

#ifdef DIAG_ENABLE
    if ( hDiagThread )
    {
        Executive_WaitForThread(hDiagThread, INFINITE);
        Executive_CloseThread(hDiagThread);
    }
#endif // DIAG_ENABLE

    pISmoothTransport->Close();

    smoothTransportWorker.ShutDown();
    
    ISmoothTransport::DestroySmoothTransport(pISmoothTransport);
}
#endif

// ========================================================================
// TestFramework entry points
// ========================================================================

static string g_URLstring;

pkRESULT pkAPI Test_GetConfig(int argc, char *argv[], TF_Config *pConfig)
{
    pkRESULT result = pkS_OK;

    TLC_SetCmdLineArgs(argc, argv);

    bool_t isVerboseHelpRequested = false;

#ifdef MSPK_USE_TELNET_CONSOLE
    if (argc == 1 || TLC_IsHelpRequested(&isVerboseHelpRequested))
#else
    if (argc == 1)
    {
        SMOOTH_MSG("SmoothSample\n");
        SMOOTH_MSG("---------\n");
        SMOOTH_MSG("Enter URL: ");

        getline(cin, g_URLstring);
    }
    else if (TLC_IsHelpRequested(&isVerboseHelpRequested))
#endif
    {
        SMOOTH_MSG("Usage: SmoothSample [tuneUrl] [options]\n");
        SMOOTH_MSG("Option: -proxy <proxy host name[:port]>\n");
        SMOOTH_MSG("Option: -audbuf <threshold milliseconds>\n");
        SMOOTH_MSG("Option: -fifo <ES FIFO limit milliseconds>\n");
        SMOOTH_MSG("Option: -fifotrick <ES FIFO limit milliseconds in trick play>\n");
        SMOOTH_MSG("Option: -liveoffset <live playback offset (from livebackoff) to enforce in secs>\n");
        SMOOTH_MSG("Option: -livebackoff <live backoff to enforce in secs>\n");
        SMOOTH_MSG("Option: -minbr <min bitrate kbps> (must be > 100)\n");
        SMOOTH_MSG("Option: -maxbr <max bitrate kbps> (must be < 100000)\n");
        SMOOTH_MSG("Option: -respto <max response timeout secs>\n");
        SMOOTH_MSG("Option: -recvito <max initial recv timeout secs>\n");
        SMOOTH_MSG("Option: -recvsto <max subsequent recv timeout secs>\n");
        SMOOTH_MSG("Option: -drmsvr <DRM license server URL>\n");
        SMOOTH_MSG("Option: -drmkid <DRM license key id (base64)>\n");
        SMOOTH_MSG("Option: -drmdata <DRM custom data string>\n");
        SMOOTH_MSG("Option: -drmlog <DRM diag log file>\n");
        SMOOTH_MSG("Option: -forceql <forced quality level> (255 for sweep)\n");
        SMOOTH_MSG("Option: -maxchunklist <max chunklist size>\n");
        SMOOTH_MSG("Option: -adjminrangeearlier <delta secs to adjust the min time of the playback range to an earlier time during manifest ready>\n");
        SMOOTH_MSG("Option: -adjminrangelater <delta secs to adjust the min time of the playback range to a later time during manifest ready>\n");
        SMOOTH_MSG("Option: -useragent <request header User-Agent value>\n");
        SMOOTH_MSG("Option: -norestrict <use select tracks instead of restrict tracks in manifest ready, to be used with options -minbr/-maxbr>\n");
        SMOOTH_MSG("Option: -printtext <use to turn on print out of text streams\n");
        SMOOTH_MSG("Option: -playad <use plays ads in a sparse stream>\n"); 

        if (isVerboseHelpRequested)
        {
            SMOOTH_MSG("Interactive commands:\n");
            SMOOTH_MSG("  play, pause, close, quit (or exit)\n");
            SMOOTH_MSG("  ff [optional: int rate], rew [optional: int rate]\n");
            SMOOTH_MSG("  sf [optional: sec] (default 30s), sb [optional: sec] (default 7s) seek [@sec]\n");
            SMOOTH_MSG("  ct (get current time), rst [optional: url] (restart play), open [optional: url]\n");
            SMOOTH_MSG("  live (skip to live), begin (skip to begin)\n");
            SMOOTH_MSG("  nextaudio (switch to next audio stream if available)\n");
            SMOOTH_MSG("  togstream [stream name] (select/deselect specified stream name)\n");
            SMOOTH_MSG("  adjminrange [sec] (delta secs to adjust the playback range, positive forward, negative for backwards)\n");
            SMOOTH_MSG("  seltracks [minKbps] [maxKbps] (select video tracks between minKbps and maxKbps)\n");
        }
        result = pkS_FALSE;
    }
    else
    {
        SMOOTH_MSG("SmoothSample\n");
        SMOOTH_MSG("---------\n");
        SMOOTH_MSG("URL: %s\n", argv[1] );

        g_URLstring = argv[1];
    }

    // get the option values

    TLC_GetCmdLineArg("-proxy", 1, (char**)&g_szHTTPProxyHost);
    TLC_GetCmdLineArg("-audbuf", 1, (char**)&g_szAudioHalBufferingThreshold);
    TLC_GetCmdLineArgInt("-fifo", 1, &g_iEsFifoLimitNormalPlay);
    TLC_GetCmdLineArgInt("-fifotrick", 1, &g_iEsFifoLimitTrickPlay);
    TLC_GetCmdLineArg("-liveoffset", 1, (char**)&g_szLivePlaybackOffsetSecs);
    TLC_GetCmdLineArg("-livebackoff", 1, (char**)&g_szLiveBackOffSecs);
    TLC_GetCmdLineArg("-livemintimebuf", 1, (char**)&g_szLiveMinTimeBufferSecs);
    TLC_GetCmdLineArg("-respto", 1, (char**)&g_szHttpResponseTimeout);
    TLC_GetCmdLineArg("-recvito", 1, (char**)&g_szHttpInitialReceiveTimeout);
    TLC_GetCmdLineArg("-recvsto", 1, (char**)&g_szHttpSubsequentReceiveTimeout);
    TLC_GetCmdLineArg("-drmsvr", 1, (char**)&g_szDrmServerUrl);
    TLC_GetCmdLineArg("-drmkid", 1, (char**)&g_szLicenseKeyId);
    TLC_GetCmdLineArg("-drmdata", 1, (char**)&g_szDrmCustomData);
    TLC_GetCmdLineArg("-drmlog", 1, (char**)&g_szDrmLogFilePath);
    TLC_GetCmdLineArg("-forceql", 1, (char**)&g_szForcedQualityLevel);
    TLC_GetCmdLineArgInt("-minbr", 1, &g_iMinBitrateKbps);
    TLC_GetCmdLineArgInt("-maxbr", 1, &g_iMaxBitrateKbps);
    TLC_GetCmdLineArg("-maxchunklist", 1, (char**)&g_szChunkListMaxSize);
    TLC_GetCmdLineArgInt("-adjminrangeearlier", 1, &g_iMinRangeDeltaSec);
    if(c_VALUE_NOT_SET != g_iMinRangeDeltaSec)
    {
        g_iMinRangeDeltaSec = -g_iMinRangeDeltaSec;
    }
    else
    {
        TLC_GetCmdLineArgInt("-adjminrangelater", 1, &g_iMinRangeDeltaSec);
    }

    TLC_GetCmdLineArg("-useragent", 1, (char**)&g_szUserAgentString);
    if (NULL == g_szUserAgentString)
    {
        g_szUserAgentString = "SSPKSample/1.0";
    }

    if ( pkS_OK == TLC_GetCmdLineArg("-norestrict", 0, NULL))
    {
        g_bRestrictTracks = false;
    }
        
    if ( pkS_OK == TLC_GetCmdLineArg("-printtext", 0, NULL))
    {
        g_bPrintText = true;
    }

    if ( pkS_OK == TLC_GetCmdLineArg("-playad", 0, NULL))
    {
        g_bPlayAd = true;
    }
    return result;
}

pkRESULT pkAPI Test_Run(void)
{
    pkRESULT result = (SOCKET_SUCCESS == Socket_Startup()) ? pkS_OK : pkE_FAIL;
    if (pkSUCCEEDED(result))
    {
        IAVManager::Create();
        LaunchSmoothTransport(g_URLstring);
        IAVManager::Destroy();
        Socket_Cleanup();
    }
    return result;
}

pkRESULT pkAPI Test_Abandon(void)
{
    SMOOTH_MSG("Abandonment requested. Please hit Enter key.\n");
    g_IsAbandoned = true;
    return pkS_OK;
}
#endif

#ifdef SS_ADAPTER_USED
//////////////////////////////////////////////////////
//SmoothStreamingAdapter below

// ========================================================================
// SmoothTransportInfoAdapter Implementation
// ========================================================================
SmoothTransportInfoAdapter::SmoothTransportInfoAdapter()
    : m_Callback()
{
}

SmoothTransportInfoAdapter::SmoothTransportInfoAdapter(ISmoothTransport* pISmoothTransport)
    : m_Callback()
{
}

#if 0
SmoothTransportInfoAdapter(ISmoothTransport* pISmoothTransport, SSAdapter_Callback_t* pCallback)
    : m_pCallback(pCallback)
{
    m_pISmoothTransport = pISmoothTransport;
}
#endif

SmoothTransportInfoAdapter::~SmoothTransportInfoAdapter()
{
}

//
// ManifestReadyCallback
//
void SmoothTransportInfoAdapter::ManifestReadyCallback(IManifest* pManifest, pkRESULT manifestResult)
{
    SmoothTransportInfo::ManifestReadyCallback(pManifest, manifestResult);

    //to copy info for streams
    if (pkFAILED(manifestResult))
    {
        SMOOTH_ADAPTER_MSG("FAILED ManifestReadyCallback: 0x%X - no manifest processed!\n", manifestResult);
    }
    else // succeeded
    {
        //shouldAutoPlay = pManifest->IsLive();

        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestStream> > selectedStreams;
        AutoRefPtr<IManifestStream> apTextStream;

        pManifest->GetAvailableStreams(&availableStreams);
        pManifest->GetSelectedStreams(&selectedStreams);
        uint64_t startTime = pManifest->StartTime();
        int64_t duration = pManifest->Duration();

       // ss_adapter_film_info_t ss_adapter_film_info;
        ss_adapter_film_info_t *pSs_adapter_film_info = &g_ss_adapter_film_info;
        ss_adapter_event_info_t ss_event_info;

        pSs_adapter_film_info->film_duration = (uint32_t)(1000*((double)(NTP_UINT64TO10MHZ(startTime) + duration) / 1.0e7));
        pSs_adapter_film_info->audio_type = SS_ADAPTER_DECODER_CODECTYPE_AUDIO_NONE;
        pSs_adapter_film_info->video_type = SS_ADAPTER_DECODER_CODECTYPE_VIDEO_NONE;
        pSs_adapter_film_info->audio_track_num = 0;
        pSs_adapter_film_info->video_track_num = 0;
        pSs_adapter_film_info->canTrickPlay = true;
        pSs_adapter_film_info->video_disp_w = 0;
        pSs_adapter_film_info->video_disp_h = 0;
        pSs_adapter_film_info->audio_track_id = 0;
        pSs_adapter_film_info->audio_language = NULL;
        pSs_adapter_film_info->audio_samplerate = 0;
        pSs_adapter_film_info->video_bps = 0;
        pSs_adapter_film_info->video_fps = 0;
        pSs_adapter_film_info->audio_bps = 0;
        pSs_adapter_film_info->file_name = NULL;

        g_ss_adapter_aud_lang_cnt = 0;
        g_ss_adapter_subt_info.cnt = 0;

        // Select the first video, first audio and first text stream
        std::vector< AutoRefPtr<IManifestStream> > proposedSelection;
        size_t iAudioIdx = 0;
        size_t iSubtIdx = 0;
        for (size_t ixStream = 0; ixStream < availableStreams.size(); ++ixStream)
        {
            if (MediaStreamTypeVideo == availableStreams[ixStream]->Type())
            {
                pSs_adapter_film_info->video_track_num++;
            }
            else if (MediaStreamTypeAudio == availableStreams[ixStream]->Type())
            {
                pSs_adapter_film_info->audio_track_num++;
                
                if (iAudioIdx < SS_ADAPTER_MAX_A_STREAMS)
                {
                    //trackid
                    g_ss_adapter_audio_info[iAudioIdx].track_id = iAudioIdx;
                    
                    //lang
                    string lang = wstring2string(availableStreams[ixStream]->Language());
                    if (g_ss_adapter_audio_info[iAudioIdx].lang)
                    {
                        Executive_Free(g_ss_adapter_audio_info[iAudioIdx].lang);
                    }
                    g_ss_adapter_audio_info[iAudioIdx].lang = (char*)Executive_Alloc(lang.length()+1, true);
                    if (g_ss_adapter_audio_info[iAudioIdx].lang)
                    {
                        memcpy( g_ss_adapter_audio_info[iAudioIdx].lang, lang.c_str(), lang.length());
                    }
                    //title
                    string title = wstring2string(availableStreams[ixStream]->Name());
                    if (g_ss_adapter_audio_info[iAudioIdx].title)
                    {
                        Executive_Free(g_ss_adapter_audio_info[iAudioIdx].title);
                    }
                    g_ss_adapter_audio_info[iAudioIdx].title = (char*)Executive_Alloc(title.length()+1, true);
                    if (g_ss_adapter_audio_info[iAudioIdx].title)
                    {
                        memcpy( g_ss_adapter_audio_info[iAudioIdx].title, title.c_str(), title.length());
                    }
                    iAudioIdx++;
                }
                else
                {
                    SMOOTH_ADAPTER_MSG("SmoothTransportInfoAdapter::ManifestReadyCallback, trackid=%d,max audio is %d\n", pSs_adapter_film_info->audio_track_num, SS_ADAPTER_MAX_A_STREAMS);
                }
            }
            else if (MediaStreamTypeText == availableStreams[ixStream]->Type())
            {
                if (iSubtIdx < SS_ADAPTER_MAX_S_STREAMS)
                {
                    g_ss_adapter_subt_info.subtitle[iSubtIdx].id = iSubtIdx;
                    int len = 0;
                    string subtype = wstring2string(availableStreams[ixStream]->SubType());
                    SMOOTH_ADAPTER_MSG("SmoothTransportInfoAdapter::ManifestReadyCallback, iSubtIdx=%d, subtype %s\n", iSubtIdx, subtype.c_str());
                    //lang
                    string lang = wstring2string(availableStreams[ixStream]->Language());
                    len = lang.length() > SS_ADAPTER_SUBTITLE_LEN-1 ? SS_ADAPTER_SUBTITLE_LEN-1 : lang.length();
                    memcpy( g_ss_adapter_subt_info.subtitle[iSubtIdx].lang, lang.c_str(), len);

                    //title
                    string title = wstring2string(availableStreams[ixStream]->Name());
                    len = title.length() > SS_ADAPTER_SUBTITLE_LEN-1 ? SS_ADAPTER_SUBTITLE_LEN-1 : title.length();
                    memcpy( g_ss_adapter_subt_info.subtitle[iSubtIdx].title, title.c_str(), len);

                    //code
                    std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
                    availableStreams[ixStream]->GetAvailableTracks(&availableTracks);
                    string code = FourCCTo2String(availableTracks[0]->FourCC());
                    memcpy( g_ss_adapter_subt_info.subtitle[iSubtIdx].code, code.c_str(), code.length());

                    g_ss_adapter_subt_info.cnt++;
                    iSubtIdx++;
                }
                else
                {
                    SMOOTH_ADAPTER_MSG("SmoothTransportInfoAdapter::ManifestReadyCallback, subtid=%d,max subt is %d\n", g_ss_adapter_subt_info.cnt, SS_ADAPTER_MAX_S_STREAMS);
                }
            }
        }
        g_ss_adapter_aud_lang_cnt = pSs_adapter_film_info->audio_track_num;

        for (size_t ixStream = 0; ixStream < selectedStreams.size(); ++ixStream)
        {

            if (MediaStreamTypeVideo == selectedStreams[ixStream]->Type())
            {
                //std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
                std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;
                //selectedStreams[ixStream]->GetAvailableTracks(&availableTracks);
                selectedStreams[ixStream]->GetSelectedTracks(&selectedTracks);
                pSs_adapter_film_info->video_type = FourCCTo2Codec(selectedTracks[0]->FourCC());

                pSs_adapter_film_info->video_disp_w = selectedStreams[ixStream]->DisplayWidth();
                pSs_adapter_film_info->video_disp_h = selectedStreams[ixStream]->DisplayHeight();
                pSs_adapter_film_info->video_bps = selectedTracks[0]->Bitrate();//availableTracks[0]->Bitrate();
            }
            else if (MediaStreamTypeAudio == selectedStreams[ixStream]->Type())
            {
                std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;

                selectedStreams[ixStream]->GetSelectedTracks(&selectedTracks);
                pSs_adapter_film_info->audio_type = FourCCTo2Codec(selectedTracks[0]->FourCC());
                pSs_adapter_film_info->audio_bps = selectedTracks[0]->Bitrate();//availableTracks[0].Bitrate();
                wstring samplerate;
                selectedTracks[0]->GetAttribute(stringToWString("SamplingRate"), &samplerate);
                pSs_adapter_film_info->audio_samplerate = wstringToUInt32(samplerate);
                string lang = wstring2string(selectedStreams[ixStream]->Language());
                if (pSs_adapter_film_info->audio_language)
                {
                    Executive_Free(pSs_adapter_film_info->audio_language);
                }
                
                pSs_adapter_film_info->audio_language = (int8_t*)Executive_Alloc(lang.length()+1, true);
                if (pSs_adapter_film_info->audio_language)
                {
                    memcpy( pSs_adapter_film_info->audio_language, lang.c_str(), lang.length());
                   // pSs_adapter_film_info->audio_language[lang.length()] = '\0';
                }
                
                size_t iCurrentIndex = 0;
                size_t track_id = 0;
                for (; iCurrentIndex < availableStreams.size(); ++iCurrentIndex)
                {
                    if (MediaStreamTypeAudio == availableStreams[iCurrentIndex]->Type())
                    {
                        if (selectedStreams[ixStream] == availableStreams[iCurrentIndex])
                        {
                            break;
                        }
                        track_id++;
                    }
                }
                if (iCurrentIndex != availableStreams.size())
                {
                    pSs_adapter_film_info->audio_track_id = track_id;
                }
            }
        }
        //send manifest ready event
        ss_event_info.m_EventType = SS_ADAPTER_EVENT_TYPE_MANIFEST_READY;
        ss_event_info.m_Data = NULL;
        ss_event_info.m_Len = 0;
        if (m_Callback.event_callback)
        {
            m_Callback.event_callback(ss_event_info.m_EventType, (int32_t)&ss_event_info);
        }
    }
}

void SmoothTransportInfoAdapter::StreamSelectedCallback(_In_ StreamSelectedEventArgs* pEventArgs)
{
    SmoothTransportInfo::StreamSelectedCallback(pEventArgs);
    UpdateMediaInfo(pEventArgs);

    ss_adapter_event_info_t ss_event_info;
    //send stream changed event
    ss_event_info.m_EventType = SS_ADAPTER_EVENT_TYPE_SELECTED_STREAM_CHANGED;
    ss_event_info.m_Data = NULL;
    ss_event_info.m_Len = 0;
    if (m_Callback.event_callback)
    {
        m_Callback.event_callback(ss_event_info.m_EventType, (int32_t)&ss_event_info);
    }
}

//
// StatusCallback
//
void SmoothTransportInfoAdapter::StatusCallback(SmoothTransportStatus& status)
{
    SmoothTransportInfo::StatusCallback(status);

    ss_adapter_status_info_t ss_status_info;
    ss_adapter_status_info_t *pSs_status_info = &ss_status_info;
    ss_adapter_event_info_t ss_event_info;
 
    ss_event_info.m_EventType = SS_ADAPTER_EVENT_TYPE_STATUS;
    ss_event_info.m_Data = (void *)pSs_status_info;
    ss_event_info.m_Len = sizeof(ss_adapter_status_info_t);

    SS_ADAPTER_EVENT_STATUS_Type type = (SS_ADAPTER_EVENT_STATUS_Type)status._update;
    ss_status_info.m_currentState = (SS_ADAPTER_EVENT_Tuner_State_Type)status._currentState;
    ss_status_info.m_update = (SS_ADAPTER_EVENT_STATUS_Type)status._update;
    SMOOTH_ADAPTER_MSG("%s current: %d, update: %d\n",
        __PRETTY_FUNCTION__,
        ss_status_info.m_currentState,
        ss_status_info.m_update);
    ss_status_info.m_currentTime = (int64_t)(status._currentTime.ToSeconds() * 1000);
    ss_status_info.m_startTime = (int64_t)(status._startTime.ToSeconds() * 1000);
    ss_status_info.m_endTime = (int64_t)(status._endTime.ToSeconds() * 1000);
    SMOOTH_ADAPTER_MSG("%s start: %lld ms, cur: %lld ms, end: %lld ms,\n",
        __PRETTY_FUNCTION__,
        ss_status_info.m_startTime,
        ss_status_info.m_currentTime,
        ss_status_info.m_endTime);
    ss_status_info.m_pkResult =  (int32_t)status._pkResult;
    ss_status_info.m_speed = status._speed;
    ss_status_info.m_clockStarted = status._clockStarted;
    ss_status_info.m_httpResponse = (int32_t)status._httpResponse;
    if (!status._additionalInfo.empty())
    {
        ss_status_info.m_additionalInfo = (char *)status._additionalInfo.c_str();
        ss_status_info.m_additionalInfoLen = status._additionalInfo.length();
    }
    else
    {
        ss_status_info.m_additionalInfo = NULL;
        ss_status_info.m_additionalInfoLen = 0;       
    }

    if (m_Callback.event_callback)
    {
        m_Callback.event_callback(ss_event_info.m_EventType, (int32_t)&ss_event_info);
    }
}

//
// ErrorCallback
// 
void SmoothTransportInfoAdapter::ErrorCallback(SmoothTransportError& errorInfo)
{
    SmoothTransportInfo::ErrorCallback(errorInfo);
    SMOOTH_ADAPTER_MSG("%s TransportError: %d, pkResult: 0x%08x, httpResponse: %d\n",
        __PRETTY_FUNCTION__,
        errorInfo._errorCode,
        errorInfo._pkResult,
        errorInfo._httpResponse);
    ss_adapter_error_info_t ss_error_info;
    ss_adapter_error_info_t *pSs_error_info = &ss_error_info;
    ss_adapter_event_info_t ss_event_info;

    ss_event_info.m_EventType = SS_ADAPTER_EVENT_TYPE_ERROR;
    ss_event_info.m_Data = (void *)pSs_error_info;
    ss_event_info.m_Len = sizeof(ss_adapter_error_info_t);

	ss_error_info.m_isLive = isLiveUrl;
    ss_error_info.m_errorCode     = (SS_ADAPTER_EVENT_ERROR_Type)errorInfo._errorCode;
    ss_error_info.m_pkResult       =  (int32_t)errorInfo._pkResult;
    ss_error_info.m_httpResponse   = (int32_t)errorInfo._httpResponse;
    if (!errorInfo._message.empty())
    {
        ss_error_info.m_message = (char *)errorInfo._message.c_str();
        ss_error_info.m_msgLen = errorInfo._message.length();
    }
    else
    {
        ss_error_info.m_message = NULL;
        ss_error_info.m_msgLen = 0;       
    }

    if (m_Callback.event_callback)
    {
        m_Callback.event_callback(ss_event_info.m_EventType, (int32_t)&ss_event_info);
    }
}
pkRESULT SmoothTransportInfoAdapter::OnFragmentData(
                        _In_ pkRESULT hrResult,
                        _In_ CHUNK_INFO* pChunkInfo,
                        _In_opt_ IRefBuffer* pBuffer,
                        _In_ bool fFinalBuffer,
                        _In_ size_t cbTotalLength )
{
    SmoothTransportInfo::OnFragmentData(hrResult, pChunkInfo, pBuffer, fFinalBuffer, cbTotalLength);

    if(pkFAILED(hrResult) || !pBuffer || !pChunkInfo || pBuffer->Length() <= 0)
    {
        SMOOTH_ADAPTER_ERR_MSG("[%s] buffer is null!\n", __FUNCTION__);
        return pkS_FALSE;
    }

    ss_adapter_event_info_t ss_event_info;
    uint32_t pts = (uint32_t)(pChunkInfo->chunkTime/ 1.0e4);
    SMOOTH_ADAPTER_MSG("[%s] pts=%d ms, cbTotalLength=%d len=%d\n", __FUNCTION__, pts, cbTotalLength, pBuffer->Length());


    SUBT_DATA * subt_data = (SUBT_DATA *) Executive_Alloc(sizeof(SUBT_DATA), true);//mtos_malloc(sizeof(SUBT_DATA));
    if (!subt_data)
    {
        SMOOTH_ADAPTER_ERR_MSG("[%s] return, subt_data=0x%x\n", __FUNCTION__, subt_data);
        return pkS_FALSE;
    }
    subt_data->data = (uint8_t *)Executive_Alloc(pBuffer->Length() + 8 + 200, true);
    if (!subt_data->data)
    {
        SMOOTH_ADAPTER_ERR_MSG("[%s] return, subt_data->data=0x%x\n", __FUNCTION__, subt_data->data);
        return pkS_FALSE;
    }

    //len + pts
    subt_data->pts =  pts;
    subt_data->size = pBuffer->Length();
    subt_data->data[0] = ((subt_data->size) & 0xff000000) >> 24;
    subt_data->data[1] = ((subt_data->size) & 0xff0000) >> 16;
    subt_data->data[2] = ((subt_data->size) & 0xff00) >> 8;
    subt_data->data[3] = ((subt_data->size)) & 0xff;
    subt_data->data[4] = ((subt_data->pts) & 0xff000000) >> 24;
    subt_data->data[5] = ((subt_data->pts) & 0xff0000) >> 16;
    subt_data->data[6] = ((subt_data->pts) & 0xff00) >> 8;
    subt_data->data[7] = ((subt_data->pts)) & 0xff;
    //subt data
    memcpy(subt_data->data + 8, pBuffer->Data(), pBuffer->Length());

    SMOOTH_ADAPTER_MSG("[%s] subt data:%s\n", __FUNCTION__, subt_data->data + 8);

    //send fragment received event
    ss_event_info.m_EventType = SS_ADAPTER_EVENT_TYPE_FRAGEMENT_DATA_RECEIVED;
    ss_event_info.m_Data = subt_data->data;
    ss_event_info.m_Len = subt_data->size + 8;
    if (m_Callback.event_callback)
    {
        //disable subtitle as not done
        m_Callback.event_callback(ss_event_info.m_EventType, (int32_t)&ss_event_info);
    }

    Executive_Free(subt_data->data);
    Executive_Free(subt_data);
}

//
// SetSSAdapterCallback
// 
void SmoothTransportInfoAdapter::SetSSAdapterCallback(SSAdapter_Callback_t *pCallback)
{
    SMOOTH_ADAPTER_MSG("%s event_callback = 0x%x\n", __PRETTY_FUNCTION__, pCallback->event_callback);
    //m_pCallback = pCallback;
   // m_Callback.status_callback = pCallback->status_callback;
    //m_Callback.error_callback = pCallback->error_callback;
    m_Callback.event_callback = pCallback->event_callback;
}

// ========================================================================
// SmoothStreamingAdapter Implementation
// ========================================================================
SS_ADAPTER_ERROR_STATUS SSAdapter_SetConfig()
{
    g_szUserAgentString = "MT_SSPK/1.0";
    g_szAudioHalBufferingThreshold = "6000";//ms
    //g_bRestrictTracks = false;
    g_bPrintText = true;
    //g_bPlayAd = true;
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Init(void **ppSspkHandle)
{
    SMOOTH_ADAPTER_MSG( "SSAdapter_Init entry\n");
    pkRESULT pkResult = pkS_OK;

#ifdef SS_ADAPTER_USED_LIB
    pkRESULT hr;
    int halError;

    //init Global config
    SSAdapter_SetConfig();

#ifdef MSPK_TF_PAL_NEEDED_EXECUTIVE
    // Initialize_Executive_PAL
    hr = Executive_Startup();
    if(pkFAILED(hr))
    {
        SMOOTH_ADAPTER_ERR_MSG("%s() ERROR %s: 0x%x\n", __FUNCTION__, "Executive_Startup", hr);
        return SS_ADAPTER_ERROR_STATUS_FAILED;
    }
#endif //MSPK_TF_PAL_NEEDED_EXECUTIVE

    initDnsCache(5);//add for dns cache function

    // Initialize Decoder_HAL
    //Renderer_SetCmdLineArgs_priv(argc, argv);

    halError = IPTV_HAL_Decoder_HALInit_priv();
    if (0 != halError)
    {
        hr = pkS_FALSE;
        SMOOTH_ADAPTER_ERR_MSG("%s() ERROR %s: 0x%x\n", __FUNCTION__, "IPTV_HAL_Decoder_HALInit_priv", halError);
        return SS_ADAPTER_ERROR_STATUS_FAILED;
    }

    // startup socket ///////////////////////////////
    pkRESULT result = (SOCKET_SUCCESS == Socket_Startup()) ? pkS_OK : pkE_FAIL;
    if (pkSUCCEEDED(result))
    {
        IAVManager::Create();
    }
    else
    {
        SMOOTH_ADAPTER_ERR_MSG( "FAILED to startup socket!");
        return SS_ADAPTER_ERROR_STATUS_FAILED;
    }
#endif //SS_ADAPTER_USED_LIB

    //start init
    *ppSspkHandle = new SmoothStreamingAdapter_t();
    if (!(*ppSspkHandle))
    {
        SMOOTH_ADAPTER_ERR_MSG( "FAILED to init handle!");
        return SS_ADAPTER_ERROR_STATUS_FAILED;
    }
    SmoothStreamingAdapter_t * pSSAdapter = (SmoothStreamingAdapter_t *)(*ppSspkHandle);
    pSSAdapter->pISmoothTransport = ISmoothTransport::CreateSmoothTransport();
    pSSAdapter->pSmoothTransportInfoAdapter = new SmoothTransportInfoAdapter(pSSAdapter->pISmoothTransport);
    pSSAdapter->pSmoothTransportWorker = new SmoothTransportWorker(pSSAdapter->pISmoothTransport, pSSAdapter->pSmoothTransportInfoAdapter);
    pSSAdapter->pSmoothTransportInfoAdapter->Initialize(pSSAdapter->pSmoothTransportWorker);
    pSSAdapter->poXDrm = NULL;

    /* pkResult = pSSAdapter->licenseAcquirer.Initialize();
    if (pkFAILED(pkResult))
    {
        SMOOTH_ADAPTER_ERR_MSG( "FAILED to initialize LicenseAcquirer: 0x%X\n", pkResult);
        return SS_ADAPTER_ERROR_STATUS_FAILED_LICENSE_INIT;
    }*/

    //Register status/error callback.
    pkResult = pSSAdapter->pISmoothTransport->RegisterStatusCallback(pSSAdapter->pSmoothTransportInfoAdapter);
    if (pkFAILED(pkResult))
    {
        SMOOTH_ADAPTER_ERR_MSG( "FAILED to register status callback: 0x%X\n", pkResult);
        return SS_ADAPTER_ERROR_STATUS_FAILED_REGISTER_CALLBACK;
    }

    pkResult = pSSAdapter->pISmoothTransport->RegisterErrorCallback(pSSAdapter->pSmoothTransportInfoAdapter);
    if (pkFAILED(pkResult))
    {
        SMOOTH_ADAPTER_ERR_MSG( " to register error callback: 0x%X\n", pkResult);
        return SS_ADAPTER_ERROR_STATUS_FAILED_REGISTER_CALLBACK;
    }

    pkResult = pSSAdapter->pISmoothTransport->SetManifestCallback(pSSAdapter->pSmoothTransportInfoAdapter);
    if (pkFAILED(pkResult))
    {
        SMOOTH_ADAPTER_ERR_MSG( " to set manifest callback: 0x%X\n", pkResult);
        return SS_ADAPTER_ERROR_STATUS_FAILED_REGISTER_CALLBACK;
    }

    if (g_szHTTPProxyHost)
    {
        SendExtendedCommand(pSSAdapter->pISmoothTransport, "httpproxyhost", 1, &g_szHTTPProxyHost);
    }
    if (g_szAudioHalBufferingThreshold)
    {
        SendExtendedCommand(pSSAdapter->pISmoothTransport, "audiohalbufferingthreshold", 1, &g_szAudioHalBufferingThreshold);
    }
    if (c_FIFO_LIMIT_NOT_SET != g_iEsFifoLimitNormalPlay)
    {
        char szInt[c_MAX_INT_CHAR_STRING_SIZE];
        const char* szIntArg = szInt;

        if (g_iEsFifoLimitNormalPlay < c_MIN_FIFO_LIMIT_MS
            || g_iEsFifoLimitNormalPlay > c_MAX_FIFO_LIMIT_MS)
        {
            SMOOTH_ADAPTER_MSG( " -fifo option value (%d) outside of range (%d..%d ms)\n",
                g_iEsFifoLimitNormalPlay,
                c_MIN_FIFO_LIMIT_MS,
                c_MAX_FIFO_LIMIT_MS);
        }
        else
        {
            // convert int milliseconds to 90kHz timescale as decimal string
            pkResult = StringCbPrintfA( szInt, sizeof(szInt), "%d", 90 * g_iEsFifoLimitNormalPlay );
            if (pkFAILED(pkResult))
            {
                SMOOTH_ADAPTER_MSG( " to convert g_iEsFifoLimitNormalPlay (%d) [0x%X]\n", g_iEsFifoLimitNormalPlay, pkResult);
            }
            else
            {
                SendExtendedCommand(pSSAdapter->pISmoothTransport, "maxesfifolimitinnormalplayback90kHz", 1, &szIntArg);
            }
        }
    }
    if (c_FIFO_LIMIT_NOT_SET != g_iEsFifoLimitTrickPlay)
    {
        char szInt[c_MAX_INT_CHAR_STRING_SIZE];
        const char* szIntArg = szInt;

        if (g_iEsFifoLimitTrickPlay < c_MIN_FIFO_LIMIT_MS
            || g_iEsFifoLimitTrickPlay > c_MAX_FIFO_LIMIT_MS)
        {
            SMOOTH_ADAPTER_MSG( " -fifotrick option value (%d) outside of range (%d..%d ms)\n",
                g_iEsFifoLimitTrickPlay,
                c_MIN_FIFO_LIMIT_MS,
                c_MAX_FIFO_LIMIT_MS);
        }
        else
        {
            // convert int milliseconds to 90kHz timescale as decimal string
            pkResult = StringCbPrintfA( szInt, sizeof(szInt), "%d", 90 * g_iEsFifoLimitTrickPlay );
            if (pkFAILED(pkResult))
            {
                SMOOTH_ADAPTER_MSG( " to convert g_iEsFifoLimitTrickPlay (%d) [0x%X]\n", g_iEsFifoLimitTrickPlay, pkResult);
            }
            else
            {
                SendExtendedCommand(pSSAdapter->pISmoothTransport, "maxesfifolimitintrickplayback90kHz", 1, &szIntArg);
            }
        }
    }
    if (g_szLivePlaybackOffsetSecs)
    {
        SendExtendedCommand(pSSAdapter->pISmoothTransport, "ssliveplaybackoffsetsec", 1, &g_szLivePlaybackOffsetSecs);
    }
    if (g_szLiveBackOffSecs)
    {
        SendExtendedCommand(pSSAdapter->pISmoothTransport, "sslivebackoffsec", 1, &g_szLiveBackOffSecs);
    }
    if (g_szLiveMinTimeBufferSecs)
    {
        SendExtendedCommand(pSSAdapter->pISmoothTransport, "sslivemintimebuffersec", 1, &g_szLiveMinTimeBufferSecs);
    }
    if (g_szHttpResponseTimeout)
    {
        SendExtendedCommand(pSSAdapter->pISmoothTransport, "httpresponsetimeout", 1, &g_szHttpResponseTimeout);
    }
    if (g_szHttpInitialReceiveTimeout)
    {
        SendExtendedCommand(pSSAdapter->pISmoothTransport, "httpinitialreceivetimeout", 1, &g_szHttpInitialReceiveTimeout);
    }
    if (g_szHttpSubsequentReceiveTimeout)
    {
        SendExtendedCommand(pSSAdapter->pISmoothTransport, "httpsubsequentreceivetimeout", 1, &g_szHttpSubsequentReceiveTimeout);
    }
    if(g_szChunkListMaxSize)
    {
        SendExtendedCommand(pSSAdapter->pISmoothTransport, "chunklistmaxsize", 1, &g_szChunkListMaxSize);
    }
    if (g_szDrmServerUrl)
    {
        std::string ResponseCustomData;

        if (g_szDrmCustomData)
        {
            pSSAdapter->licenseAcquirer.ChallengeCustomData = g_szDrmCustomData;
        }

        if (g_szDrmServerUrl)
        {
            pSSAdapter->licenseAcquirer.LicenseServerUriOverride = g_szDrmServerUrl;
        }

        pkResult = pSSAdapter->licenseAcquirer.AcquireLicense(g_szDrmServerUrl, g_szLicenseKeyId, &ResponseCustomData);
        if (pkFAILED(pkResult))
        {
            SMOOTH_ADAPTER_MSG( " to AcquireLicense from %s: 0x%X\n", g_szDrmServerUrl, pkResult);
        }
        else if (0 != ResponseCustomData.length())
        {
            SMOOTH_ADAPTER_MSG("AcquireLicense response custom data:\n%s\n", ResponseCustomData.c_str());
        }
    }
    if (g_szDrmLogFilePath)
    {
        SMOOTH_ADAPTER_MSG( " g_szDrmLogFilePath=%s\n", g_szDrmLogFilePath);
        pkResult = pSSAdapter->drmDiagDelegate.OpenLogFile(g_szDrmLogFilePath);
        if (pkFAILED(pkResult))
        {
            SMOOTH_ADAPTER_MSG( " to open log file %s: 0x%X\n", g_szDrmLogFilePath, pkResult);
        }

        pkResult = XDRM_CreateInstance(&pSSAdapter->poXDrm);
        if (pkFAILED(pkResult))
        {
            SMOOTH_ADAPTER_MSG( " to XDRM_CreateInstance 0x%X\n", pkResult);
        }
        else if (!pSSAdapter->poXDrm->SetDiagDelegate(&pSSAdapter->drmDiagDelegate))
        {
            SMOOTH_ADAPTER_MSG( " to SetDiagDelegate\n");
        }
    }

    SendExtendedCommand(pSSAdapter->pISmoothTransport, "useragentstring", 1, &g_szUserAgentString);

#ifdef DIAG_ENABLE
    pSspkHandle->diagManager = IAVManager::Instance()->GetDiagsManager();
    pSspkHandle->hDiagThread = 0;

    if (pSspkHandle->diagManager)
    {
        DiagSetChannelPriority( kDiagChannel_Heuristics, kDiagsPriority_Medium );
        DiagSetChannelPriority( kDiagChannel_Manifest, kDiagsPriority_Medium );
        DiagSetChannelPriority( kDiagChannel_ChunkList, kDiagsPriority_Medium );
        DiagSetChannelPriority( kDiagChannel_FragInfo, kDiagsPriority_Medium );

        Executive_CreateThread(&DiagEventRetrieveFun, pSspkHandle->diagManager, 0, &pSspkHandle->hDiagThread);

        // only turn on diagnostics if the thread is created
        if ( pSspkHandle->hDiagThread )
        {
            pSspkHandle->diagManager->RegisterFilters(true);
        }
    }
#endif // DIAG_ENABLE

    pSSAdapter->pSmoothTransportWorker->Initialize();
    SMOOTH_ADAPTER_MSG( "SSAdapter_Init exit\n");
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Register_Callback(void *pSspkHandle, SSAdapter_Callback_t *pCallback)
{
    if (!pSspkHandle || !pCallback)
    {
        return SS_ADAPTER_ERROR_STATUS_INVALID;
    }
    SmoothStreamingAdapter_t * pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
    //pSSAdapter->callback.error_callback = (SSAdapter_Error_Callback_t)pCallback->error_callback;
    //pSSAdapter->callback.status_callback = (SSAdapter_Status_Callback_t)pCallback->status_callback;
    pSSAdapter->pSmoothTransportInfoAdapter->SetSSAdapterCallback(pCallback);
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Get_Film_Info(void *pSspkHandle, ss_adapter_film_info_t *pResult)
{
    if (!pSspkHandle || !pResult)
    {
        return SS_ADAPTER_ERROR_STATUS_INVALID;
    }

    SmoothStreamingAdapter_t * pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
    ss_adapter_film_info_t *pSs_adapter_film_info = &g_ss_adapter_film_info;

    memcpy(pResult, pSs_adapter_film_info, sizeof(ss_adapter_film_info_t));
    
    SMOOTH_ADAPTER_MSG("SSAdapter_Get_Film_Info() result film info :\n \
    duration=%d, audio_type=0%x, video_type=0x%x, \n \
    audio_track_num=%d,video_track_num=%d, canTrickPlay= %d, \n \
    video_disp_w=%d, video_disp_h=%d, audio_track_id=%d,  lang=%s,\n \
    audio_samplerate=%d, video_bps=%d, video_fps=%d, audio_bps=%d\n",
        pResult->film_duration,
        pResult->audio_type,
        pResult->video_type,
        pResult->audio_track_num,
        pResult->video_track_num,
        pResult->canTrickPlay,
        pResult->video_disp_w,
        pResult->video_disp_h,
        pResult->audio_track_id,
        pResult->audio_language,
        pResult->audio_samplerate,
        pResult->video_bps,
        pResult->video_fps,
        pResult->audio_bps);
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Get_Audio_Track_Lang(void *pSspkHandle, ss_adapter_track_lang_t *pResult)
{
    if (!pSspkHandle || !pResult)
    {
        SMOOTH_ADAPTER_ERR_MSG("SSAdapter_Get_Audio_Track_Lang() pSspkHandle=0x%x, pResult=0x%x", pSspkHandle, pResult);
        return SS_ADAPTER_ERROR_STATUS_INVALID;
    }
    SmoothStreamingAdapter_t *pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
    int32_t audioNum = g_ss_adapter_aud_lang_cnt;
    if (audioNum >= SS_ADAPTER_MAX_A_STREAMS || audioNum <= 0)
    {
        SMOOTH_ADAPTER_ERR_MSG("SSAdapter_Get_Audio_Track_Lang() audio track num=%d", audioNum);
        return SS_ADAPTER_ERROR_STATUS_INVALID;
    }

    memcpy(pResult, g_ss_adapter_audio_info, sizeof(ss_adapter_track_lang_t)* audioNum);
    for (int audioIdx = 0; audioIdx < audioNum; ++audioIdx)
    {
        SMOOTH_ADAPTER_MSG("SSAdapter_Get_Audio_Track_Lang() audio track %d, lang=%s, title=%s\n",
                    pResult[audioIdx].track_id,
                    pResult[audioIdx].lang,
                    pResult[audioIdx].title);
    }
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Get_Subt_Info(void * pSspkHandle, ss_adapter_subt_t *pResult)
{
    if (!pSspkHandle || !pResult)
    {
        SMOOTH_ADAPTER_ERR_MSG("SSAdapter_Get_Audio_Track_Lang() pSspkHandle=0x%x, pResult=0x%x", pSspkHandle, pResult);
        return SS_ADAPTER_ERROR_STATUS_INVALID;
    }
    SmoothStreamingAdapter_t *pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
    memcpy(pResult, &g_ss_adapter_subt_info, sizeof(ss_adapter_subt_t));
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Set_Media(void *pSspkHandle, ss_adapter_media_info_t *pMediaInfo)
{
    if (!pSspkHandle || !pMediaInfo)
    {
        SMOOTH_ADAPTER_ERR_MSG("SSAdapter_Cmd_Set_Media() pSspkHandle=0x%x, pMediaInfo=0x%x\n", pSspkHandle, pMediaInfo);
        return SS_ADAPTER_ERROR_STATUS_INVALID;
    }
    SmoothStreamingAdapter_t *pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
    
    if (pMediaInfo->DrmCustomData)
    {
        IDrmManager* pDrmManager = (IDrmManager*)IAVManager::Instance()->GetDrmManager();
        pDrmManager->OverrideCustomData(pMediaInfo->DrmCustomData);
        //SMOOTH_ADAPTER_MSG("SSAdapter_Cmd_Set_Media() customdata=%s\n", pSSAdapter->licenseAcquirer.ChallengeCustomData.c_str());
    }

    if (pMediaInfo->licenseUrl)
    {
        IDrmManager* pDrmManager = (IDrmManager*)IAVManager::Instance()->GetDrmManager();
        pDrmManager->OverrideLicenseServerUri(pMediaInfo->licenseUrl);
        //SMOOTH_ADAPTER_MSG("SSAdapter_Cmd_Set_Media() licenseUrl=%s\n", pSSAdapter->licenseAcquirer.LicenseServerUriOverride.c_str());
    }
	
    if (pMediaInfo->DrmCertPath)
    {    	
        SMOOTH_ADAPTER_ERR_MSG("SSAdapter_Cmd_Set_Media() DrmCertPath=%s\n",pMediaInfo->DrmCertPath);
        IDrmManager* pDrmManager = (IDrmManager*)IAVManager::Instance()->GetDrmManager();
        pDrmManager->OverrideCertPath(pMediaInfo->DrmCertPath);
    }
    if (pMediaInfo->url)
    {
        pSSAdapter->strUrl = pMediaInfo->url;
    }
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Open(void *pSspkHandle, const char *url)
{
    SmoothStreamingAdapter_t * pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
    speedidx = SPEED_ZERO;
    string strUrl = url;
    strUrl = stringEscSpaces(stringTrim(strUrl, " "));
    SMOOTH_ADAPTER_MSG("SSAdapter_Cmd_Open() strUrl=%s\n", strUrl.c_str());
    load_media_succuss = false;
	isLiveUrl = 0;
    pSSAdapter->pSmoothTransportWorker->RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Open, 0, 0, strUrl);
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Close(void *pSspkHandle)
{
    //add for during loadmeida stop by user to exit
    if(load_media_succuss == false){
        Socket_SetExitNetworkFlag(true);
        SMOOTH_ADAPTER_MSG("SSAdapter_Cmd_Close() ExitNetworkFlag\n");
    }
    SmoothStreamingAdapter_t * pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
    pSSAdapter->pSmoothTransportWorker->RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Close);
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Play(void *pSspkHandle, const float speed)
{
    SmoothStreamingAdapter_t * pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
    speedidx = SPEED_ZERO;
    pSSAdapter->pSmoothTransportWorker->RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Play, speed);
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Pause(void *pSspkHandle)
{
    SmoothStreamingAdapter_t * pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
    speedidx = SPEED_ZERO;
    pSSAdapter->pSmoothTransportWorker->RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Pause);
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Resume(void *pSspkHandle)
{
    SmoothStreamingAdapter_t * pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
    speedidx = SPEED_ZERO;
    pSSAdapter->pSmoothTransportWorker->RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Play);
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Seek(void *pSspkHandle, const int32_t timeStampSec)
{
    SmoothStreamingAdapter_t * pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
    speedidx = SPEED_ZERO;

    if(timeStampSec > 0)
    {
        pSSAdapter->pSmoothTransportWorker->RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Seek, 0, timeStampSec);
    }
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Skip(void *pSspkHandle, const int32_t sec)
{
    SmoothStreamingAdapter_t * pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
    speedidx = SPEED_ZERO;
    pSSAdapter->pSmoothTransportWorker->RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Skip, 0, sec);
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_FastSpeed(void *pSspkHandle, const float speed)
{
    SmoothStreamingAdapter_t * pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
    pSSAdapter->pSmoothTransportWorker->RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Play, speed);
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Live(void *pSspkHandle)
{
    SmoothStreamingAdapter_t * pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
    speedidx = SPEED_ZERO;
    // skip forward kSkipToLiveSecs; big value
    pSSAdapter->pSmoothTransportWorker->RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Skip, 0, kSkipToLiveSecs);
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Begin(void *pSspkHandle)
{
    SmoothStreamingAdapter_t * pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
    speedidx = SPEED_ZERO;
    // skip backward kSkipToBeginSecs; big value
    pSSAdapter->pSmoothTransportWorker->RequestSmoothTransportCommand(SmoothTransportWorker::eTransportCommand_Skip, 0, -kSkipToLiveSecs);
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Nextaudio(void *pSspkHandle)
{
    SmoothStreamingAdapter_t * pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
    SwitchAudio(*(pSSAdapter->pSmoothTransportInfoAdapter));
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Change_Audio_Track(void *pSspkHandle, int16_t trackid)
{
    SmoothStreamingAdapter_t * pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
    change_audio_track(*(pSSAdapter->pSmoothTransportInfoAdapter), trackid);
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Set_Subt_Id(void *pSspkHandle, int16_t subtid)
{
    SmoothStreamingAdapter_t * pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
    change_subt_id(*(pSSAdapter->pSmoothTransportInfoAdapter), subtid);
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Togstream(void *pSspkHandle)
{
    SmoothStreamingAdapter_t * pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Seltracks(void *pSspkHandle)
{
    SmoothStreamingAdapter_t * pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Adjminrange(void *pSspkHandle)
{
    SmoothStreamingAdapter_t * pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Launch(void **ppSspkHandle, const char *url)
{
    SS_ADAPTER_ERROR_STATUS err = SSAdapter_Init(ppSspkHandle);
    if (err != SS_ADAPTER_ERROR_STATUS_OK)
    {
        SMOOTH_ADAPTER_MSG("SSAdapter_Launch() SSAdapter_Init failed! err=%d\n", err);
        return err;
    }
    SMOOTH_ADAPTER_MSG("SSAdapter_Launch() SSAdapter_Cmd_Open\n");
    err = SSAdapter_Cmd_Open(*ppSspkHandle, url);
    if (err != SS_ADAPTER_ERROR_STATUS_OK)
    {
        SMOOTH_ADAPTER_MSG("SSAdapter_Launch() SSAdapter_Cmd_Open failed! err=%d\n", err);
    }
    return err;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Exit(void *pSspkHandle)
{
    SS_ADAPTER_ERROR_STATUS err = SS_ADAPTER_ERROR_STATUS_OK;

    SmoothStreamingAdapter_t * pSSAdapter = (SmoothStreamingAdapter_t *)pSspkHandle;
#ifdef MSPK_USE_TELNET_CONSOLE
    ConsoleClose();
#endif
    if (g_szDrmLogFilePath)
    {
        if (pSSAdapter->poXDrm)
        {
            if (!pSSAdapter->poXDrm->SetDiagDelegate(NULL))
            {
                SMOOTH_ADAPTER_MSG( " to SetDiagDelegate(NULL)\n");
            }

            pkRESULT pkResult = XDRM_DestroyInstance(pSSAdapter->poXDrm);
            if (pkFAILED(pkResult))
            {
                SMOOTH_ADAPTER_MSG( " to XDRM_DestroyInstance 0x%X\n", pkResult);
            }
        }
    }
#ifdef DIAG_ENABLE
    if ( pSSAdapter->hDiagThread )
    {
        Executive_WaitForThread(pSSAdapter->hDiagThread, INFINITE);
        Executive_CloseThread(pSSAdapter->hDiagThread);
    }
#endif // DIAG_ENABLE

    uninitDnsCache();//add for dns cache function

    pSSAdapter->pISmoothTransport->Close();

    pSSAdapter->pSmoothTransportWorker->ShutDown();

    ISmoothTransport::DestroySmoothTransport(pSSAdapter->pISmoothTransport);

    delete pSSAdapter->pSmoothTransportWorker;
    delete pSSAdapter->pSmoothTransportInfoAdapter;
    delete pSSAdapter;
#ifdef SS_ADAPTER_USED_LIB
    int halError;
    pkRESULT hr;

    // Exit Decoder_HAL
    halError = IPTV_HAL_Decoder_HALExit_priv();
    if (0 != halError)
    {
        err = SS_ADAPTER_ERROR_STATUS_FAILED;
        SMOOTH_ADAPTER_MSG("%s() ERROR %s: 0x%x\n", __FUNCTION__, "IPTV_HAL_Decoder_HALExit", halError);
    }
    //Renderer_ClearCmdLineArgs_priv();
#ifdef MSPK_TF_PAL_NEEDED_EXECUTIVE
    // Terminate_Executive_PAL
    hr = Executive_Shutdown();
    if(pkFAILED(hr))
    {
        SMOOTH_ADAPTER_MSG("%s() ERROR %s: 0x%x\n", __FUNCTION__, "Executive_Shutdown", hr);
        err = SS_ADAPTER_ERROR_STATUS_FAILED;
    }
#endif //MSPK_TF_PAL_NEEDED_EXECUTIVE

    IAVManager::Destroy();
    Socket_Cleanup();
#endif
    return err;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_RequestCommand(void *pSspkHandle, ss_adapter_info_t sspk_info)
{
    SS_ADAPTER_ERROR_STATUS err = SS_ADAPTER_ERROR_STATUS_OK;

    if (!pSspkHandle)
    {
        return SS_ADAPTER_ERROR_STATUS_INVALID;
    }
    
    switch((int)sspk_info.type)
    {
        case eSS_ADAPTER_CMD_Play:
            err = SSAdapter_Cmd_Play(pSspkHandle, sspk_info.speed);
            break;
        case eSS_ADAPTER_CMD_Pause:
            err = SSAdapter_Cmd_Pause(pSspkHandle);
            break;
        case eSS_ADAPTER_CMD_Close:
            err = SSAdapter_Cmd_Close(pSspkHandle);
            break;
        case eSS_ADAPTER_CMD_Open:
            err = SSAdapter_Cmd_Open(pSspkHandle, sspk_info.url);
            break;
        case eSS_ADAPTER_CMD_Skip:
            err = SSAdapter_Cmd_Skip(pSspkHandle, sspk_info.sec);
            break;
        case eSS_ADAPTER_CMD_Seek:
            err = SSAdapter_Cmd_Seek(pSspkHandle, sspk_info.sec);
            break;
        case eSS_ADAPTER_CMD_Live:
            err = SSAdapter_Cmd_Live(pSspkHandle);
            break;
        case eSS_ADAPTER_CMD_Begin:
            err = SSAdapter_Cmd_Begin(pSspkHandle);
            break;
        case eSS_ADAPTER_CMD_Nextaudio:
            err = SSAdapter_Cmd_Nextaudio(pSspkHandle);
            break;
        case eSS_ADAPTER_CMD_Togstream:
            //err = SSAdapter_Cmd_Togstream(pSspkHandle);
            break;
        case eSS_ADAPTER_CMD_Seltracks:
            //err = SSAdapter_Cmd_Seltracks(pSspkHandle);
            break;
        case eSS_ADAPTER_CMD_Adjminrange:
            //err = SSAdapter_Cmd_Adjminrange(pSspkHandle);
            break;
        default:
          break;
    }
    return err;
}

#ifdef SS_ADAPTER_USED_TEST
//test
static SS_ADAPTER_ERROR_STATUS SSAdapter_Test_Status_Callback(SS_ADAPTER_EVENT_STATUS_Type type, int32_t param)
{
    SMOOTH_ADAPTER_MSG("SSAdapter_Test_Status_Callback() - type = %d\n", (int)type);
    ss_adapter_event_info_t *pEvent_info = (ss_adapter_event_info_t *)param;
    ss_adapter_status_info_t *pStatus_info = (ss_adapter_status_info_t *)pEvent_info->m_Data;
    SMOOTH_ADAPTER_MSG("%s current: %d, update: %d,\n",
        __PRETTY_FUNCTION__,
        pStatus_info->m_currentState,
        pStatus_info->m_update);
    SMOOTH_ADAPTER_MSG("%s current time: %d, starttime: %d, endtime: %d\n",
    __PRETTY_FUNCTION__,
    pStatus_info->m_currentTime,
    pStatus_info->m_startTime,
    pStatus_info->m_endTime);
    SMOOTH_ADAPTER_MSG("%s m_additionalInfo: %s , Len: %d\n",
    __PRETTY_FUNCTION__,
    pStatus_info->m_additionalInfo,
    pStatus_info->m_additionalInfoLen);
    SMOOTH_ADAPTER_MSG("%s m_pkResult: 0x%x , m_httpResponse: 0x%x\n",
    __PRETTY_FUNCTION__,
    pStatus_info->m_pkResult,
    pStatus_info->m_httpResponse);
    return SS_ADAPTER_ERROR_STATUS_OK;
}

static SS_ADAPTER_ERROR_STATUS SSAdapter_Test_Error_Callback(SS_ADAPTER_EVENT_ERROR_Type type, int32_t param)
{
    SMOOTH_ADAPTER_MSG("SSAdapter_Test_Error_Callback() - type = %d\n", (int)type);
    return SS_ADAPTER_ERROR_STATUS_OK;
}

static SS_ADAPTER_ERROR_STATUS SSAdapter_Test_Event_Callback(SS_ADAPTER_EVENT_TYPE type, int32_t param)
{
    SMOOTH_ADAPTER_MSG("%s() - type = %d\n", __PRETTY_FUNCTION__, (int)type);
    ss_adapter_event_info_t *pEvent_info = (ss_adapter_event_info_t *)param;
    if(type == SS_ADAPTER_EVENT_TYPE_STATUS)
    {
        ss_adapter_status_info_t *pStatus_info = (ss_adapter_status_info_t *)pEvent_info->m_Data;
        SMOOTH_ADAPTER_MSG("%s current: %d, update: %d,\n",
            __PRETTY_FUNCTION__,
            pStatus_info->m_currentState,
            pStatus_info->m_update);
        SMOOTH_ADAPTER_MSG("%s current time: %d, starttime: %d, endtime: %d\n",
        __PRETTY_FUNCTION__,
            pStatus_info->m_currentTime,
            pStatus_info->m_startTime,
            pStatus_info->m_endTime);
        SMOOTH_ADAPTER_MSG("%s m_additionalInfo: %s , Len: %d\n",
        __PRETTY_FUNCTION__,
            pStatus_info->m_additionalInfo,
            pStatus_info->m_additionalInfoLen);
        SMOOTH_ADAPTER_MSG("%s m_pkResult: 0x%x , m_httpResponse: 0x%x\n",
            __PRETTY_FUNCTION__,
            pStatus_info->m_pkResult,
            pStatus_info->m_httpResponse);
    }
    else if (type == SS_ADAPTER_EVENT_TYPE_ERROR)
    {
        ss_adapter_error_info_t *pError_info = (ss_adapter_error_info_t *)pEvent_info->m_Data;
        SMOOTH_ADAPTER_MSG("%s error: %d\n",
            __PRETTY_FUNCTION__,
            pError_info->m_errorCode);
        SMOOTH_ADAPTER_MSG("%s m_pkResult: 0x%x , m_httpResponse: 0x%x\n",
            __PRETTY_FUNCTION__,
            pError_info->m_pkResult,
            pError_info->m_httpResponse);
        SMOOTH_ADAPTER_MSG("%s m_message: %s , len: %d\n",
            __PRETTY_FUNCTION__,
            pError_info->m_message,
            pError_info->m_msgLen);
    }

    return SS_ADAPTER_ERROR_STATUS_OK;
}

SS_ADAPTER_ERROR_STATUS SSAdapter_Test(std::string url)
{
    void *pSspkHandle = NULL;
    //SSAdapter_Callback_t callback;
    SMOOTH_ADAPTER_MSG("%s() - SSAdapter_Init \n", __FUNCTION__);
    SS_ADAPTER_ERROR_STATUS err = SSAdapter_Init(&pSspkHandle);
    if (err != SS_ADAPTER_ERROR_STATUS_OK)
    {
        SMOOTH_ADAPTER_ERR_MSG("%s() SSAdapter_Init failed! err=%d\n", __FUNCTION__, err);
        return err;
    }
    //register callback
    SMOOTH_ADAPTER_MSG("%s() SSAdapter_Init SSAdapter_Test_Status_Callback=0x%x\n" , __FUNCTION__, SSAdapter_Test_Status_Callback);
    SMOOTH_ADAPTER_MSG("%s() SSAdapter_Init SSAdapter_Test_Error_Callback=0x%x\n" , __FUNCTION__, SSAdapter_Test_Error_Callback);
    SMOOTH_ADAPTER_MSG("%s() SSAdapter_Init SSAdapter_Test_Event_Callback=0x%x\n" , __FUNCTION__, SSAdapter_Test_Event_Callback);

    //callback.status_callback = (SSAdapter_Status_Callback_t)SSAdapter_Test_Status_Callback;
    //callback.error_callback = (SSAdapter_Error_Callback_t)SSAdapter_Test_Error_Callback;
    callback.event_callback = (SSAdapter_Event_Callback_t)SSAdapter_Test_Event_Callback;
    SMOOTH_ADAPTER_MSG("%s() - SSAdapter_Register_Callback \n", __FUNCTION__);
    err = SSAdapter_Register_Callback(pSspkHandle, &callback);
    if (err != SS_ADAPTER_ERROR_STATUS_OK)
    {
        SMOOTH_ADAPTER_ERR_MSG("%s() SSAdapter_Register_Callback failed! err=%d\n", __FUNCTION__, err);
        return err;
    }
    //open
    SMOOTH_ADAPTER_MSG("%s() SSAdapter_Cmd_Open %s\n", __FUNCTION__, url.c_str());
    err = SSAdapter_Cmd_Open(pSspkHandle, url.c_str());
    if (err != SS_ADAPTER_ERROR_STATUS_OK)
    {
        SMOOTH_ADAPTER_ERR_MSG("%s() SSAdapter_Cmd_Open failed! err=%d\n", __FUNCTION__, err);
        return err;
    }
    //pause
    SMOOTH_ADAPTER_MSG("%s() SSAdapter_Cmd_Pause\n", __FUNCTION__);
    err = SSAdapter_Cmd_Pause(pSspkHandle);
    if (err != SS_ADAPTER_ERROR_STATUS_OK)
    {
        SMOOTH_ADAPTER_ERR_MSG("%s() SSAdapter_Cmd_Pause failed! err=%d\n", __FUNCTION__, err);
        return err;
    }
    //fast play
    SMOOTH_ADAPTER_MSG("%s() SSAdapter_Cmd_Play %f\n", __FUNCTION__, 2.0);
    err = SSAdapter_Cmd_Play(pSspkHandle, 2.0);
    if (err != SS_ADAPTER_ERROR_STATUS_OK)
    {
        SMOOTH_ADAPTER_ERR_MSG("%s() SSAdapter_Cmd_Play failed! err=%d\n", __FUNCTION__, err);
        return err;
    }
    //seek 30s
    SMOOTH_ADAPTER_MSG("%s() SSAdapter_Cmd_Seek %ds\n", __FUNCTION__, 30);
    err = SSAdapter_Cmd_Seek(pSspkHandle, 30);
    if (err != SS_ADAPTER_ERROR_STATUS_OK)
    {
        SMOOTH_ADAPTER_ERR_MSG("%s() SSAdapter_Cmd_Seek failed! err=%d\n", __FUNCTION__, err);
        return err;
    }
    while(!g_IsAbandoned)
    {
        //10min
        SMOOTH_ADAPTER_MSG("%s() - sleep 30s\n", __FUNCTION__);
        Sleep(30000);
        SMOOTH_ADAPTER_MSG("%s() - sleep 30s end %d\n", __FUNCTION__, g_IsAbandoned);
   }
   //exit
    SMOOTH_ADAPTER_MSG("%s() - SSAdapter_Exit\n", __FUNCTION__);
    err = SSAdapter_Exit(pSspkHandle);
    SMOOTH_ADAPTER_MSG("%s() - return err=%d\n", __FUNCTION__, err);
    return err;
}
#endif //SS_ADAPTER_USED_TEST
#endif //SS_ADAPTER_USED