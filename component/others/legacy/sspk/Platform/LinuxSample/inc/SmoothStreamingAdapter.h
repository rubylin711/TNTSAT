#ifndef SMOOTHSTREAMING_ADAPTER_H
#define SMOOTHSTREAMING_ADAPTER_H

/////MARO//////////
#ifndef bool
#ifndef __cplusplus
// bool is not defined in the "C" case. bool is 1 byte in the C++ case.
typedef char bool;
#endif
#endif

#ifndef int8_t
typedef signed char int8_t;
#endif

#ifndef uint8_t
/*!
  Unsigned integer, weight is 8 bits. In most case it equal to unsigned char.
  */
typedef unsigned char uint8_t;
#endif

#ifndef int16_t
/*!
  Signed integer, weight is 16 bits. In most case it equal to short.
  */
typedef signed short int16_t;
#endif

#ifndef uint16_t
/*!
  Unsigned integer, weight is 16 bits. In most case it equal to unsigned short.
  */
typedef unsigned short uint16_t;
#endif

#ifndef int32_t
/*!
  Signed integer, weight is 32 bits. In most case it equal to long.
  */
typedef signed int int32_t;
#endif

#ifndef uint32_t

/*!
  Unsigned integer, weight is 32 bits. In most case it equal to unsigned long.
  */
typedef unsigned int uint32_t;
#endif

#ifndef int64_t
/*!
  Signed integer, weight is 64 bits
  */
typedef signed long long int64_t;
#endif

#ifndef uint64_t
/*!
  Usigned integer, weight is 64 bits
  */
typedef unsigned long long uint64_t;
#endif



#ifndef SS_ADAPTER_SUBTITLE_LEN
#define SS_ADAPTER_SUBTITLE_LEN 20
#endif

#ifndef SS_ADAPTER_SUBTITLE_CNT
#define SS_ADAPTER_SUBTITLE_CNT 20
#endif

#define SS_ADAPTER_MAX_A_STREAMS 64
#define SS_ADAPTER_MAX_V_STREAMS 64
#define SS_ADAPTER_MAX_S_STREAMS 64
#ifdef __cplusplus
extern "C" {
#endif
/////ENUM
typedef enum
{
	SS_ADAPTER_ERROR_STATUS_OK = 0,
	SS_ADAPTER_ERROR_STATUS_INVALID,
	SS_ADAPTER_ERROR_STATUS_FAILED,
  SS_ADAPTER_ERROR_STATUS_FAILED_LICENSE_INIT,
  SS_ADAPTER_ERROR_STATUS_FAILED_REGISTER_CALLBACK,
}SS_ADAPTER_ERROR_STATUS;

typedef enum
{
  eSS_ADAPTER_CONF_INVALID = 0,
  eSS_ADAPTER_CONF_Open,
  eSS_ADAPTER_CONF_Close,
  eSS_ADAPTER_CONF_MAX,
} SS_ADAPTER_CONF_Type;

typedef enum
{
  eSS_ADAPTER_CMD_INVALID = 0,
  eSS_ADAPTER_CMD_Open,
  eSS_ADAPTER_CMD_Close,
  eSS_ADAPTER_CMD_Play,
  eSS_ADAPTER_CMD_Pause,
  eSS_ADAPTER_CMD_Skip,
  eSS_ADAPTER_CMD_Seek,
  eSS_ADAPTER_CMD_Live,
  eSS_ADAPTER_CMD_Begin,
  eSS_ADAPTER_CMD_Nextaudio,
  eSS_ADAPTER_CMD_Togstream,
  eSS_ADAPTER_CMD_Seltracks,
  eSS_ADAPTER_CMD_Adjminrange,
  //eSS_ADAPTER_CMD_Exit,
  eSS_ADAPTER_CMD_MAX,
} SS_ADAPTER_CMD_Type;

typedef enum
{
    eSS_ADAPTER_EVENT_TunerState_Unknown = 0,
    eSS_ADAPTER_EVENT_TunerState_Tuning,             // Tuning
    eSS_ADAPTER_EVENT_TunerState_Playing,            // Playback started
    eSS_ADAPTER_EVENT_TunerState_Paused,             // Playback paused
    eSS_ADAPTER_EVENT_TunerState_MediaEnded,         // Presentation has completed
    eSS_ADAPTER_EVENT_TunerState_Detuned,            // Detuned due to an error
    eSS_ADAPTER_EVENT_TunerState_Closed,             // Transport is closed, open is required
    eSS_ADAPTER_EVENT_TunerState_Max
}SS_ADAPTER_EVENT_Tuner_State_Type;

typedef enum
{
  eSS_ADAPTER_EVENT_STATUS_INVALID = 0,
  eSS_ADAPTER_EVENT_STATUS_Heartbeat,
  eSS_ADAPTER_EVENT_STATUS_TunerStateChanged, // Tuner State changed
  eSS_ADAPTER_EVENT_STATUS_Streaming, // Data started to come in from the socket
  eSS_ADAPTER_EVENT_STATUS_Rendering, // Clock started
  eSS_ADAPTER_EVENT_STATUS_Underrun, // There was a underrun event
  eSS_ADAPTER_EVENT_STATUS_Rebuffer, // There was a rebuffering event
  eSS_ADAPTER_EVENT_STATUS_StartEndTime, // The start/end time changed
  eSS_ADAPTER_EVENT_STATUS_DrmStateChanged, // Drm status changed
  eSS_ADAPTER_EVENT_STATUS_BitrateChanged, // Bit rate changed
  eSS_ADAPTER_EVENT_STATUS_DecoderError, // Decoder error
  eSS_ADAPTER_EVENT_STATUS_ChunkConnectHttpInvalid,// http <200 or >=400 reponse on connect
  eSS_ADAPTER_EVENT_STATUS_NextChunkHttpInvalid, // http <200 or >=400 reponse on next chunk
  eSS_ADAPTER_EVENT_STATUS_ChunkHdrHttpInvalid, // http <200 or >=400 reponse on chunk hdr
  eSS_ADAPTER_EVENT_STATUS_ChunkHdrError, // chunk header parser error
  eSS_ADAPTER_EVENT_STATUS_AtWindowEdge, // Playback position is at or past DVR window edge
  eSS_ADAPTER_EVENT_STATUS_EndOfLive, // Live presentation is no longer live
  eSS_ADAPTER_EVENT_STATUS_OutsideWindowEdge, // Download position is outside of DVR window
  eSS_ADAPTER_EVENT_STATUS_SegmentManifestError, // Error while getting a segment manifest
  eSS_ADAPTER_EVENT_STATUS_DrmInitError, // Error while initializing DRM
  eSS_ADAPTER_EVENT_STATUS_ManifestReady,
  eSS_ADAPTER_EVENT_STATUS_MAX,
} SS_ADAPTER_EVENT_STATUS_Type;

typedef enum
{
  SS_ADAPTER_EVENT_ERROR_None                           = 0,

  // Generic errors
  SS_ADAPTER_EVENT_ERROR_Unknown                        = 100,

  // Tuner errors
  SS_ADAPTER_EVENT_ERROR_TunerAllocationFailure         = 200,
  SS_ADAPTER_EVENT_ERROR_TunerSharedReceivers,

  // Manifest errors
  SS_ADAPTER_EVENT_ERROR_ManifestParseFailed            = 300,
  SS_ADAPTER_EVENT_ERROR_ManifestVersionUnsupported,
  SS_ADAPTER_EVENT_ERROR_ManifestInvalid,
  SS_ADAPTER_EVENT_ERROR_ManifestHttpInvalidResult,

  // Socket errors
  SS_ADAPTER_EVENT_ERROR_SocketAlreadyClosed            = 400,
  SS_ADAPTER_EVENT_ERROR_SocketReadError,
  SS_ADAPTER_EVENT_ERROR_SocketOpenFailed,
  SS_ADAPTER_EVENT_ERROR_SocketConnectFailed,
  SS_ADAPTER_EVENT_ERROR_SocketSendFailed,
  SS_ADAPTER_EVENT_ERROR_SocketRecvFailed,

  //HTTP errors
  SS_ADAPTER_EVENT_ERROR_HttpParseResponseFailed        = 500,
  SS_ADAPTER_EVENT_ERROR_HttpInvalidResult,
  SS_ADAPTER_EVENT_ERROR_HttpTooManyRedirect,
  SS_ADAPTER_EVENT_ERROR_HttpRedirectFailed,
  SS_ADAPTER_EVENT_ERROR_HttpRedirectNotAllowed,
  SS_ADAPTER_EVENT_ERROR_HttpCreateFailed,

  // Chunk errors
  SS_ADAPTER_EVENT_ERROR_ChunkConnectHttpInvalidResult  = 600,
  SS_ADAPTER_EVENT_ERROR_ChunkNextHttpInvalidResult,
  SS_ADAPTER_EVENT_ERROR_ChunkHdrParseFailed,
  SS_ADAPTER_EVENT_ERROR_ChunkInvalidData,

  //Drm error
  SS_ADAPTER_EVENT_ERROR_DrmInitFailed                  = 700,
} SS_ADAPTER_EVENT_ERROR_Type;

typedef enum
{
  SS_ADAPTER_EVENT_TYPE_STATUS = 0,
  SS_ADAPTER_EVENT_TYPE_ERROR,
  SS_ADAPTER_EVENT_TYPE_MANIFEST_READY,
  SS_ADAPTER_EVENT_TYPE_SELECTED_STREAM_CHANGED,
  SS_ADAPTER_EVENT_TYPE_FRAGEMENT_DATA_RECEIVED,
}SS_ADAPTER_EVENT_TYPE;

/*
typedef enum
{
    SS_ADAPTER_AUDIO_TYPE_NONE  = 0,
    SS_ADAPTER_AUDIO_TYPE_AAC   = 1,
    SS_ADAPTER_AUDIO_TYPE_MP3   = 2,
    SS_ADAPTER_AUDIO_TYPE_AC3   = 3,
    SS_ADAPTER_AUDIO_TYPE_DTS   = 4,
    SS_ADAPTER_AUDIO_TYPE_DRA   = 5
} SS_ADAPTER_AUDIO_TYPE;

typedef enum
{
    SS_ADAPTER_VIDEO_TYPE_NONE    = 0,
    SS_ADAPTER_VIDEO_TYPE_MPEG2   = 1,
    SS_ADAPTER_VIDEO_TYPE_MPEG4   = 2,
    SS_ADAPTER_VIDEO_TYPE_H263    = 3,
    SS_ADAPTER_VIDEO_TYPE_H264    = 4,
    SS_ADAPTER_VIDEO_TYPE_AVS     = 5,
    SS_ADAPTER_VIDEO_TYPE_REAL    = 6,
    SS_ADAPTER_VIDEO_TYPE_AV1     = 7
} SS_ADAPTER_VIDEO_TYPE;
*/
typedef enum
{
    SS_ADAPTER_DECODER_CODECTYPE_INVALID = -1,
    SS_ADAPTER_DECODER_CODECTYPE_VIDEO_NONE = 0,
    SS_ADAPTER_DECODER_CODECTYPE_VIDEO_MPEG2,
    SS_ADAPTER_DECODER_CODECTYPE_VIDEO_H264,//AVC
    SS_ADAPTER_DECODER_CODECTYPE_VIDEO_VC1,//VC1
    SS_ADAPTER_DECODER_CODECTYPE_VIDEO_WMV9,//WMV9
    SS_ADAPTER_DECODER_CODECTYPE_VIDEO_AVS,
    SS_ADAPTER_DECODER_CODECTYPE_MAXVIDEO,
}SS_ADAPTER_DECODER_CODECTYPE_VIDEO;

typedef enum 
{
    SS_ADAPTER_DECODER_CODECTYPE_AUDIO_NONE = 0x10000,
    SS_ADAPTER_DECODER_CODECTYPE_AUDIO_AC3,//AC3
    SS_ADAPTER_DECODER_CODECTYPE_AUDIO_PCM,
    SS_ADAPTER_DECODER_CODECTYPE_AUDIO_MPEG,//MP3
    SS_ADAPTER_DECODER_CODECTYPE_AUDIO_WMASTD,//WMA
    SS_ADAPTER_DECODER_CODECTYPE_AUDIO_WMATS,
    SS_ADAPTER_DECODER_CODECTYPE_AUDIO_AAC,//AAC
    SS_ADAPTER_DECODER_CODECTYPE_AUDIO_AVS,
    SS_ADAPTER_DECODER_CODECTYPE_AUDIO_EAC3,
    SS_ADAPTER_DECODER_CODECTYPE_AUDIO_WMAPRO,//WMA Pro
    SS_ADAPTER_DECODER_CODECTYPE_MAXAUDIO
} SS_ADAPTER_DECODER_CODECTYPE_AUDIO;

/////Structure

typedef struct ss_adapter_video_tracks
{
  int32_t minKbps;
  int32_t maxKbps;
  bool restrictTracks;
} ss_adapter_video_tracks_t;

typedef struct ss_adapter_info
{
  SS_ADAPTER_CMD_Type type;
  float   speed;
  int32_t sec;
  char    *url;
  int64_t val64;
  ss_adapter_video_tracks_t video_tracks;
} ss_adapter_info_t;

typedef struct 
{
    /*!
      language about subtitle
      */
    char        lang[SS_ADAPTER_SUBTITLE_LEN];
    /*!
    title string
    */
    char        title[SS_ADAPTER_SUBTITLE_LEN];
    /*!
    codec type
    */
    char        code[SS_ADAPTER_SUBTITLE_LEN];
    /*!
    subtitle index id
    */
    int32_t     id;
} ss_adapter_subtitle_t;

typedef struct 
{
    int32_t                 cnt;
    ss_adapter_subtitle_t   subtitle[SS_ADAPTER_SUBTITLE_CNT];
} ss_adapter_subt_t;

typedef struct
{
    ss_adapter_subt_t   *subt_array;
    uint32_t            pts;
}  ss_adapter_subt_data_t;

typedef struct ss_adapter_film_info
{
  int32_t     film_duration;//ms
  int32_t     audio_type;
  int32_t     video_type;
  int32_t     audio_track_num;
  int32_t     video_track_num;
  bool        canTrickPlay;
  int32_t     video_disp_w;
  int32_t     video_disp_h;
  int32_t     video_fps;
  int32_t     video_bps;//bytes per secondes
  uint64_t    file_size;
  int32_t     audio_track_id;//streamid, start from 1.
  int8_t      *audio_language; // pointers to a modification forbidden memory space
  int32_t     audio_bps;
  int32_t     audio_samplerate;
  uint8_t     *file_name; // pointers to a modification forbidden memory space
} ss_adapter_film_info_t;

/*!
@brief track language info
*/
typedef struct
{
  int32_t   track_id;
  char      *lang;
  char      *title;
  int32_t   format;
} ss_adapter_track_lang_t;

typedef struct
{
  ss_adapter_film_info_t    film; 
  ss_adapter_subt_t         *subt;// pointers to a modification forbidden memory space
  ss_adapter_track_lang_t   *aud_lang;
  int32_t                   aud_lang_cnt;
} ss_adapter_file_info_t;

typedef struct
{
  char                      *url; 
  char                      *DrmCustomData;
  char                      *licenseUrl;
  char                      *DrmCertPath;
} ss_adapter_media_info_t;

typedef struct ss_adapter_status_info
{
    SS_ADAPTER_EVENT_Tuner_State_Type   m_currentState;       // Current status
    SS_ADAPTER_EVENT_STATUS_Type        m_update;             // Reason for this update
    int64_t                             m_currentTime;        // Current displayed media time in playback stream, ms
    int64_t                             m_startTime;          // Start displayable media time of playback stream, ms
    int64_t                             m_endTime;            // End displayable media time of playback stream, ms
    float                               m_speed;              // Current playback rate
    char*                               m_additionalInfo;     // Any additional info that may go with the reason
    int32_t                             m_additionalInfoLen;  // additional info data len
    bool                                m_clockStarted;       // Is clock started?
    int32_t                             m_pkResult;           // pkRESULT code, if any
    int32_t                             m_httpResponse;       // http response code, if any
}ss_adapter_status_info_t;

typedef struct ss_adapter_error_info
{
    SS_ADAPTER_EVENT_ERROR_Type         m_errorCode;       // Overall error code
    int32_t                             m_pkResult;        // pkRESULT code, if any
    int32_t                             m_httpResponse;    // http response code, if any
    char*                               m_message;         // Error message
    int32_t                             m_msgLen;
    int32_t                             m_isLive;
}ss_adapter_error_info_t;

typedef struct ss_adapter_event_info
{
    SS_ADAPTER_EVENT_TYPE               m_EventType;       // event code
    void *                              m_Data;            // event data
    int16_t                             m_Len;
}ss_adapter_event_info_t;

//typedef SS_ADAPTER_ERROR_STATUS (*SSAdapter_ManifestReady_Callback_t)(SS_ADAPTER_EVENT_ERROR_Type type, int32_t param);
//typedef SS_ADAPTER_ERROR_STATUS (*SSAdapter_Error_Callback_t)(SS_ADAPTER_EVENT_ERROR_Type type, int32_t param);
//typedef SS_ADAPTER_ERROR_STATUS (*SSAdapter_Status_Callback_t)(SS_ADAPTER_EVENT_STATUS_Type type, int32_t param);
typedef SS_ADAPTER_ERROR_STATUS (*SSAdapter_Event_Callback_t)(SS_ADAPTER_EVENT_TYPE type, int32_t param);

typedef struct SSAdapter_Callback
{
    //SSAdapter_ManifestReady_Callback_t manifest_callback;
    //SSAdapter_Error_Callback_t    error_callback;
    //SSAdapter_Status_Callback_t   status_callback;
    SSAdapter_Event_Callback_t    event_callback;
}SSAdapter_Callback_t;

/////Interface

SS_ADAPTER_ERROR_STATUS SSAdapter_Init(void **ppSspkHandle);
SS_ADAPTER_ERROR_STATUS SSAdapter_Register_Callback(void *pSspkHandle, SSAdapter_Callback_t *pCallback);
SS_ADAPTER_ERROR_STATUS SSAdapter_Get_Film_Info(void *pSspkHandle, ss_adapter_film_info_t *pResult);
SS_ADAPTER_ERROR_STATUS SSAdapter_Get_Audio_Track_Lang(void *pSspkHandle, ss_adapter_track_lang_t *pResult);
SS_ADAPTER_ERROR_STATUS SSAdapter_Get_Subt_Info(void * pSspkHandle, ss_adapter_subt_t *pResult);

SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Set_Media(void *pSspkHandle, ss_adapter_media_info_t *pMediaInfo);
SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Open(void *pSspkHandle, const char *url);
SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Close(void *pSspkHandle);
SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Play(void *pSspkHandle, const float speed);
SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Pause(void *pSspkHandle);
SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Resume(void *pSspkHandle);
SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Seek(void *pSspkHandle, const int32_t timeStampSec);
SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Skip(void *pSspkHandle, const int32_t sec);
SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_FastSpeed(void *pSspkHandle, const float speed);
SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Live(void *pSspkHandle);
SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Begin(void *pSspkHandle);
SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Nextaudio(void *pSspkHandle);
SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Change_Audio_Track(void *pSspkHandle, int16_t trackid);
SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Set_Subt_Id(void *pSspkHandle, int16_t subtid);
SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Togstream(void *pSspkHandle);
SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Seltracks(void *pSspkHandle);
SS_ADAPTER_ERROR_STATUS SSAdapter_Cmd_Adjminrange(void *pSspkHandle);
SS_ADAPTER_ERROR_STATUS SSAdapter_Exit(void *pSspkHandle);

SS_ADAPTER_ERROR_STATUS SSAdapter_Launch(void **ppSspkHandle, const char *url);
SS_ADAPTER_ERROR_STATUS SSAdapter_RequestCommand(void *pSspkHandle, ss_adapter_info_t sspk_info);


#ifdef __cplusplus
}
#endif


#endif // SMOOTHSTREAMING_ADAPTER_H