/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/**
 \file
 \brief Server (SVR) player module. CNcomment:svr player模块CNend
 \author Montage Technologies Co., Ltd.
 \date 2006-2018
 \version 1.0
 \author
 \date 2017-11-10
 */

#ifndef __MTSU_SVR_PLAYER_H__
#define __MTSU_SVR_PLAYER_H__

#include "mt_type.h"
#include "mtsu_svr_format.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*************************** Structure Definition ****************************/
/** \addtogroup      Suplayer */
/** @{ */ /** <!-- [Suplayer]*/

/** Invalid handle */
/** CNcomment:非法句柄 */
#define MT_SVR_PLAYER_INVALID_HDL (0)

/** Normal playing speed */
/** CNcomment:正常播放速度*/
#define MT_SVR_PLAYER_PLAY_SPEED_NORMAL (1024)
#define MT_SRV_SUPLAYER_INSTANCE_MAX (2)

typedef enum mtUNF_SUPLAYER_TYPE
{
	MT_SUPLAYER_UNKNOWN = 0,
	MT_SUPLAYER_MPLAYER,
	MT_SUPLAYER_GSTREAMER,
	MT_SUPLAYER_MSS,
	MT_SUPLAYER_BUTT
}mtUNF_SUPLAYER_TYPE_E;

/** Player attribute ID */
/** CNcomment:播放器属性ID */
typedef enum mtSVR_PLAYER_ATTR_E {
    MT_SVR_PLAYER_ATTR_STREAMID = 0x0,/* @deprecate */
    /**< Set or obtain the stream ID. This attribute is applicable to multi-audio multi-video streams. The parameter is ::MT_SVR_PLAYER_STREAMID_S. */ /**< CNcomment:设置/获取流编号，该属性针对多音频、多视频流的情况，参数为::MT_SVR_PLAYER_STREAMID_S */
    MT_SVR_PLAYER_ATTR_WINDOW_HDL,
    /**< Obtain the window ID. The parameter is the MT_U32 variable. */ /**< CNcomment:获取window id，参数为MT_U32变量 */
    MT_SVR_PLAYER_ATTR_AVPLAYER_HDL,
    /**< Obtain the audio/video play (AVPlay) handle. The parameter is the MT_HANDLE variable. */ /**< CNcomment:获取AVPlayer句柄，参数为MT_HANDLE变量 */
    MT_SVR_PLAYER_ATTR_SUBTITLE_HDL,/* @deprecate */
    /**< Obtain the subtitle handle. The parameter is the MT_HANDLE variable. */ /**< CNcomment:获取Subtitle句柄，参数为MT_HANDLE变量 */
    MT_SVR_PLAYER_ATTR_SO_HDL,/* @deprecate */
    /**< Obtain the subtitle output (SO) handle. The parameter is the MT_HANDLE variable. */ /**< CNcomment:获取so句柄，参数为MT_HANDLE变量 */
    MT_SVR_PLAYER_ATTR_AUDTRACK_HDL,
    /**< Obtain the audio track handle. The parameter is the MT_HANDLE variable. */ /**< CNcomment:获取音频track句柄，参数为MT_HANDLE变量 */
    MT_SVR_PLAYER_ATTR_SYNC,/* @deprecate */
    /**< Set the sync attribute. The parameter is ::MT_SVR_PLAYER_SYNC_ATTR_S. */ /**< CNcomment:设置音视频、字幕时间戳偏移，参数为::MT_SVR_PLAYER_SYNC_ATTR_S */
    MT_SVR_PLAYER_ATTR_VSINK_HDL, /* @deprecate */                                                /**< set the vsink handle. The parameter is MT_SVR_VSINK_S* */
    MT_SVR_PLAYER_ATTR_AUD_PID,
    MT_SVR_PLAYER_ATTR_VID_PID,
    MT_SVR_PLAYER_ATTR_NET_BITRATE, /* Network download bitrates, unit:bps, type:signed long long */
    MT_SVR_PLAYER_ATTR_BUTT
} MT_SVR_PLAYER_ATTR_E;

typedef struct mtSVR_PLAYER_PARA_LIST {
    /** item numbers in the list */
    int num;
    int *list;
} MT_SVR_PLAYER_PARA_LIST_S;

typedef struct mtSVR_PLAYER_PID_LIST {
    /** pid numbers want to get */
    int pid_num;
    /** list to store pids, Sorted in ascendant order of stream id */
    int *pids;
} MT_SVR_PLAYER_PID_LIST_S;
/* User stream control info */
typedef struct mtSVR_PLAYER_IN_STREAM_PARA {
    /** Type of the stream */
    MT_FORMAT_DATA_TYPE_E stream_type;
    /** list of codec in blacklist set by user,
     * see HA_CODEC_ID_E for audio, MT_UNF_VCODEC_TYPE_E for video,
     * Null if no codec blacklist
     */
    MT_SVR_PLAYER_PARA_LIST_S *codec_blacklist;
} MT_SVR_PLAYER_IN_STREAM_PARA_S;

/** Callback for checking whether to abort blocking functions.
  * During blocking operations, callback is called with void *callback_ctx as parameter.
  * If the callback returns non-zero value, the blocking operation will be aborted.
  */
typedef struct MT_SVR_PLAYER_IN_PARA_INTERRUPT_CB {
    int (*callback)(void*);
    void *callback_ctx;
} MT_SVR_PLAYER_IN_PARA_INTERRUPT_CB_S;

typedef struct mtSVR_PLAYER_IN_PARA {
    struct {
        char *headers;    /* set custom HTTP headers, can override built in default headers */
        char *user_agent; /* override User-Agent header */
        char *cookies; /* holds newline (\n) delimited Set-Cookie header field values (without the "Set-Cookie: " field name), eg."CloudFront-Policy=xxx;\nCloudFront-Signature=xxxx;\nCloudFront-Key-Pair-Id=K90W0I58USR0U;" */
    } net;
    int max_stream_probe_size;       /* bytes */
    int max_stream_analyze_duration; /* unit s(second) */
    MT_SVR_PLAYER_IN_STREAM_PARA_S stream_para[MT_FORMAT_DATA_BUTT];
    MT_SVR_PLAYER_IN_PARA_INTERRUPT_CB_S int_cb;
} MT_SVR_PLAYER_IN_PARA_S;

/************************************Player Event Start***********************************/

/** Error information of the player */
/** CNcomment:播放器错误信息 */
typedef enum mtSVR_PLAYER_ERROR_E {
    MT_SVR_PLAYER_ERROR_NON = 0x0,
    MT_SVR_PLAYER_ERROR_VID_PLAY_FAIL,/* @deprecate */
    /**< The video fails to be played. */ /**< CNcomment: 视频播放启动失败 */
    MT_SVR_PLAYER_ERROR_AUD_PLAY_FAIL,/* @deprecate */
    /**< The audio fails to be played. */ /**< CNcomment: 音频播放启动失败 */
    MT_SVR_PLAYER_ERROR_SUB_PLAY_FAIL,/* @deprecate */
    /**< The subtitle fails to be played. */ /**< CNcomment: 字幕播放启动失败 */
    MT_SVR_PLAYER_ERROR_PLAY_FAIL,
    /**< The file fails to be played. */ /**< CNcomment: 音视频播放失败 */
    MT_SVR_PLAYER_ERROR_TIMEOUT,
    /**< Operation timeout. For example, reading data timeout. */ /**< CNcomment: 操作超时， 如读取数据超时 */
    MT_SVR_PLAYER_ERROR_NOT_SUPPORT,
    /**< The file format is not supportted. */ /**< CNcomment: 文件格式不支持 */
    MT_SVR_PLAYER_ERROR_UNKNOW,
    /**< Unknown error. */ /**< CNcomment: 未知错误 */
    MT_SVR_PALYER_ERROR_UNSUPPORT_VIDEO,
    /*playback unsupport this type video*/
    MT_SVR_PALYER_ERROR_BUTT,
    MT_SVR_PALYER_ERROR_NOT_ENOUGH_MEMORY,
    MT_SVR_PALYER_NETWORK_START,
    MT_SVR_PALYER_NETWORK_END,
    MT_SVR_PALYE_DRV_DECRYPT_FAIL,
    /*playback not have enough memory*/
} MT_SVR_PLAYER_ERROR_E;

/** Player status */
/** CNcomment:PLAYER状态 */
typedef enum mtSVR_PLAYER_STATE_E {
    MT_SVR_PLAYER_STATE_IDLE,
    MT_SVR_PLAYER_STATE_INIT = 1,
    /**< The player is in the initial state. It changes to the initial state after being created. */ /**< CNcomment:播放器当前处于初始状态，create后播放器处于ini状态 */
    MT_SVR_PLAYER_STATE_DEINIT,
    /**< The player is deinitialized. */ /**< CNcomment:播放器已经去初始状态 */
    MT_SVR_PLAYER_STATE_PLAY,
    /**< The player is in the playing state. */ /**< CNcomment:播放器当前处于播放状态 */
    MT_SVR_PLAYER_STATE_FORWARD,
    /**< The player is in the fast forward state. */ /**< CNcomment:播放器当前处于快进状态 */
    MT_SVR_PLAYER_STATE_BACKWARD,
    /**< The player is in the rewind state. */ /**< CNcomment:播放器当前处于快退状态 */
    MT_SVR_PLAYER_STATE_PAUSE,
    /**< The player is in the stop state. */ /**< CNcomment:播放器当前处于暂停状态 */
    MT_SVR_PLAYER_STATE_STOP,
    /**< The player is in the file exit state*/
    MT_SVR_PLAYER_STATE_ES_TASK_EXIT,
    /**< The player is in the stop state. */ /**< CNcomment:播放器当前处于停止状态 */
    MT_SVR_PLAYER_STATE_PREPARING,
    /**< The player is in the preparing state. */ /**< CNcomment:播放器当前处于准备状态 */
    MT_SVR_PLAYER_STATE_CREATE,
    /**<
     * dash and hls contains more playlists sometimes,
     * which will occupy more initial time and postpone play,
     * so we parse one playlist for every a/v/s stream before play,
     * after that, we send async open message and parse other playlists continually.
     * we send async done after all playlists parsed.
     * so, if you get async open message, you should get the file inforamtion after async done.
     */
    MT_SVR_PLAYER_STATE_ASYNC_OPEN,
    MT_SVR_PLAYER_STATE_ASYNC_DONE,
    MT_SVR_PLAYER_STATE_REGEVENT,
    MT_SVR_PLAYER_STATE_SETPATH,
    MT_SVR_PLAYER_STATE_LOADED,
    MT_SVR_PLAYER_STATE_PRESTOP,
    MT_SVR_PLAYER_STATE_PREDESTROY,
    MT_SVR_PLAYER_STATE_BUTT
} MT_SVR_PLAYER_STATE_E;

/** Event type */
/** CNcomment:事件类型 */
typedef enum mtSVR_PLAYER_EVENT_E {
    MT_SVR_PLAYER_EVENT_STATE_CHANGED = 0x0,
    /**< Player status change event. The parameter type is ::MT_SVR_PLAYER_STATE_E. */ /**< CNcomment:播放器状态转换事件，参数类型为::MT_SVR_PLAYER_STATE_E */
    MT_SVR_PLAYER_EVENT_SOF,                                                           /**< Event indicating that file playing starts or a file is rewound to the file header. The parameter type is MT_U32. The value ::MT_SVR_PLAYER_STATE_PLAY indicates that file playing starts and the value ::MT_SVR_PLAYER_STATE_BACKWARD indicates that a file is rewound to the file header. */
                                                                                       /**< CNcomment:文件开始播放或快退到文件头事件，参数类型为MT_U32，值为::MT_SVR_PLAYER_STATE_PLAY表示开始播放，参数值为::MT_SVR_PLAYER_STATE_BACKWARD表示快退到文件头 */
    MT_SVR_PLAYER_EVENT_EOF,
    /**< Event indicating that a file is played till the end of the file. There is no parameter. */ /**< CNcomment:文件播放到尾事件，无参数 */
    MT_SVR_PLAYER_EVENT_PROGRESS,
    /**< Event indicating the current progress of the player. This event is reported once every 300 ms. The parameter is ::MT_SVR_PLAYER_PROGRESS_S. */ /**< CNcomment:播放器当前播放进度事件，每隔300ms上报一次该事件，参数值为::MT_SVR_PLAYER_PROGRESS_S */
    MT_SVR_PLAYER_EVENT_STREAMID_CHANGED,
    /**< Stream ID change event. The parameter is ::MT_SVR_PLAYER_STREAMID_S. */ /**< CNcomment:stream id 发生变化事件，参数为::MT_SVR_PLAYER_STREAMID_S */
    MT_SVR_PLAYER_EVENT_SEEK_FINISHED,                                           /**< Seek operation is complete. The parameter is MT_U32. When the value of the parameter is ::MT_FAILURE, the seek operation fails. When the value of the parameter is ::MT_SUCCESS, the seek operation succeeds. When the value of the parameter is ::MT_FORMAT_ERRO_ENDOFFILE, the file is read to the end. */
                                                                                 /**< CNcomment:Seek操作完成，参数为MT_U32，值为::MT_FAILURE Seek失败，::MT_SUCCESS seek成功，::MT_FORMAT_ERRO_ENDOFFILE 文件读取到文件尾 */
    MT_SVR_PLAYER_EVENT_CODETYPE_CHANGED,
    /**< Event of indicating the byte encoding configuration is complete. The parameter is ::MT_SVR_PLAYER_SUB_CODETYPE_S. */ /**< CNcomment:设置字符编码完成事件，事件参数::MT_SVR_PLAYER_SUB_CODETYPE_S */
    MT_SVR_PLAYER_EVENT_DOWNLOAD_PROGRESS,
    /**< Current download progress of the player. The event is reported every 300 ms. The parameter is ::MT_SVR_PLAYER_PROGRESS_S. */ /**< CNcomment:播放器当前下载进度，每隔300ms上报一次该事件，参数值为::MT_SVR_PLAYER_PROGRESS_S */
    MT_SVR_PLAYER_EVENT_BUFFER_STATE,
    /**< Reporting buffer status. The parameter type is ::MT_SVR_PLAYER_BUFFER_S. */ /**< CNcomment:缓冲状态上报,参数类型为::MT_SVR_PLAYER_BUFFER_S */
    MT_SVR_PLAYER_EVENT_FIRST_FRAME_TIME,
    /**< The display time of the first frame from setting the setMedia. The parameter is ::MT_U32, in the unit of ms. */ /**< CNcomment:从设置媒体setMedia开始第一帧显示时间,参数为::MT_U32,单位为ms .*/
    MT_SVR_PLAYER_EVENT_ERROR,
    /**< Event of indicating that an error occurs in the player. The parameter is ::MT_SVR_PLAYER_ERROR_E.*/ /**< CNcomment:播放器错误信息事件，参数为::MT_SVR_PLAYER_ERROR_E */
    MT_SVR_PLAYER_EVENT_NETWORK_INFO,
    /**< Report the network status. The parameter is ::MT_FORMAT_NET_STATUS_S. */ /**< CNcomment:网络状态上报, 参数为::MT_FORMAT_NET_STATUS_S */
    MT_SVR_PLAYER_EVENT_DOWNLOAD_FINISH,
    /**< File download finish, no parameter. */ /**< CNcomment:文件下载完毕, 无参数 */
    MT_SVR_PLAYER_EVENT_ASYNC_SETMEDIA_FINISH,
    /**< Async set media finish event*/ /**< CNcomment:异步打开媒体完成事件*/
    MT_SVR_PLAYER_EVENT_UPDATE_FILE_INFO,
    /**< The file information has been updated. */ /**< CNcomment:文件信息已更新 */
    MT_SVR_PLAYER_EVENT_STREAM_NOT_AVAIABLE,
    /**< stream not avaiable ,need to stop play*/ /**< CNcomment:码流不可用，需要停止播放*/
    MT_SVR_PLAYER_EVENT_NETWORKBRANDWIDTH,
    /**< Report the current network brand width. The parameter is ::MT_U32, in the unit of kbps. */ /**< CNcomment:上报当前的网络带宽,参数为::MT_U32,单位为kbps */

    /*get the info for the file info*/
    MT_SVR_PLAYER_EVENT_FILE_INFO,
     /* * notify Supper layer that  switch audio spdif out mode **/
    MT_SVR_PLAYER_EVENT_SWITCH_AUDIO_SPDIF_MODE,
    /***/
    MT_SVR_PLAYER_EVENT_NEW_SUBTITLE_RECEIVED,
    /*Report playback don't support this type video**/
    MT_SVR_PLAYBACK_UNSUPPORT_VIDEO,

    /**Enter to trick mode*/
    MT_SVR_PLAYER_EVENT_TRICKMODE_ENTER,

    /**Leave from trick mode*/
    MT_SVR_PLAYER_EVENT_TRICKMODE_LEAVE,

    /**new video frame*/
    MT_SVR_PLAYER_EVENT_NEW_VID_FRAME,

    /** network loading info**/
    MT_SVR_PLAYER_EVENT_NETWORK_LOAD,

    /** network loading **/
    MT_SVR_PLAYER_EVENT_NETWORK_BUFFERING,

    /**< Private event of user define, MT_SVR_PLAYER_EVENT_S:pu8Data is addr of user parameter */ /**< CNcomment:用户定义的私有事件，MT_SVR_PLAYER_EVENT_S:pu8Data参数为事件参数地址 */
    MT_SVR_PLAYER_EVENT_USER_PRIVATE = 100,

	MT_SVR_PLAYER_EVENT_DRM = 200,

    /** server error event **/
    MT_SVR_PLAYER_EVENT_SERVER_ERROR = 300,

    /** Max Enum*/
    MT_SVR_PLAYER_EVENT_BUTT,
} MT_SVR_PLAYER_EVENT_E;


/** Information of playing progress */
/** CNcomment:进度事件信息内容 */
typedef struct mtSVR_PLAYER_PROGRESS_S
{
    MT_U32 u32Progress; /**< Progress of current playing. The value ranges from 0 to 100. */                                                                                                                                             /**< CNcomment:进度值, 值0-100 */
    mt_s64 s64Duration; /**< The duration (in the unit of ms) of  current playing or downloading ,when used for downloading event(e.g. MT_SVR_PLAYER_EVENT_DOWNLOAD_PROGRESS),the duration of data in the decoder's buffer is included*/ /**< CNcomment:当前播放进度或下载时长,单位为ms ,当用于下载事件时(如MT_SVR_PLAYER_EVENT_DOWNLOAD_PROGRESS等)包含了decoder 缓冲中的数据时长*/
    mt_s64 s64BufferSize; /**< The total size(in the unit of byte) of data in the hiplayer's buffer and decoder's buffer*/                                                                                                               /**< CNcomment:hiplayer内部缓冲数据和decoder缓冲数据总大小,单位为bytes */
} MT_SVR_PLAYER_PROGRESS_S;

/** Information of network loading on playing **/
/** CNcomment:网络缓冲提示信息 */
typedef struct mtSVR_PLAYER_NETWORKLOAD_S
{
    mt_u32 u32Bps;
    mt_u32 u32Percentage;
}MT_SVR_PLAYER_NETWORKLOAD_S;

/** Player event callback parameters */
/** CNcomment:播放器事件回调参数 */
typedef struct mtSVR_PLAYER_EVENT_S
{
    MT_SVR_PLAYER_EVENT_E eEvent; /**< Event type */                    /**< CNcomment:事件类型 */
    MT_U32 u32Len; /**< Event parameter length, in the unit of byte. */ /**< CNcomment:事件参数长度，字节为单位 */
    MT_U8 *pu8Data; /**< Start address of event parameter data */       /**< CNcomment:事件参数数据起始地址 */
} MT_SVR_PLAYER_EVENT_S;

/************************************Player Event End************************************/

/** Playing speed flag */
/** CNcomment:播放速度标识 */
typedef enum mtSVR_PLAYER_PLAY_SPEED_E {
    MT_SVR_PLAYER_PLAY_SPEED_1X2_SLOW_FORWARD = MT_SVR_PLAYER_PLAY_SPEED_NORMAL / 2,
    /**< 1/2 speed slow forward */ /**< CNcomment:1/2倍速慢放 */
    MT_SVR_PLAYER_PLAY_SPEED_2X_FAST_FORWARD = 2 * MT_SVR_PLAYER_PLAY_SPEED_NORMAL,
    /**< 2 x speed fast forward */ /**< CNcomment:2倍速快放 */
    MT_SVR_PLAYER_PLAY_SPEED_4X_FAST_FORWARD = 4 * MT_SVR_PLAYER_PLAY_SPEED_NORMAL,
    /**< 4 x speed fast forward */ /**< CNcomment:4倍速快放 */
    MT_SVR_PLAYER_PLAY_SPEED_8X_FAST_FORWARD = 8 * MT_SVR_PLAYER_PLAY_SPEED_NORMAL,
    /**< 8 x speed fast forward */ /**< CNcomment:8倍速快放 */
    MT_SVR_PLAYER_PLAY_SPEED_16X_FAST_FORWARD = 16 * MT_SVR_PLAYER_PLAY_SPEED_NORMAL,
    /**< 16 x speed fast forward */ /**< CNcomment:16倍速快放 */
    MT_SVR_PLAYER_PLAY_SPEED_32X_FAST_FORWARD = 32 * MT_SVR_PLAYER_PLAY_SPEED_NORMAL,
    /**< 32 x speed fast forward */ /**< CNcomment:32倍速快放 */
    MT_SVR_PLAYER_PLAY_SPEED_64X_FAST_FORWARD = 64 * MT_SVR_PLAYER_PLAY_SPEED_NORMAL,
    /**< 64 x speed fast forward */ /**< CNcomment:64倍速快放 */
    MT_SVR_PLAYER_PLAY_SPEED_1X_BACKWARD = -1 * MT_SVR_PLAYER_PLAY_SPEED_NORMAL,
    /**< 1 x speed rewind */ /**< CNcomment:1倍速倒退 */
    MT_SVR_PLAYER_PLAY_SPEED_2X_FAST_BACKWARD = -2 * MT_SVR_PLAYER_PLAY_SPEED_NORMAL,
    /**< 2 x speed rewind */ /**< CNcomment:2倍速快退 */
    MT_SVR_PLAYER_PLAY_SPEED_4X_FAST_BACKWARD = -4 * MT_SVR_PLAYER_PLAY_SPEED_NORMAL,
    /**< 4 x speed rewind */ /**< CNcomment:4倍速快退 */
    MT_SVR_PLAYER_PLAY_SPEED_8X_FAST_BACKWARD = -8 * MT_SVR_PLAYER_PLAY_SPEED_NORMAL,
    /**< 8 x speed rewind */ /**< CNcomment:8倍速快退 */
    MT_SVR_PLAYER_PLAY_SPEED_16X_FAST_BACKWARD = -16 * MT_SVR_PLAYER_PLAY_SPEED_NORMAL,
    /**< 16 x speed rewind */ /**< CNcomment:16倍速快退 */
    MT_SVR_PLAYER_PLAY_SPEED_32X_FAST_BACKWARD = -32 * MT_SVR_PLAYER_PLAY_SPEED_NORMAL,
    /**< 32 x speed rewind */ /**< CNcomment:32倍速快退 */
    MT_SVR_PLAYER_PLAY_SPEED_64X_FAST_BACKWARD = -64 * MT_SVR_PLAYER_PLAY_SPEED_NORMAL,
    /**< 64 x speed rewind */                                 /**< CNcomment:64倍速快退 */
    MT_SVR_PLAYER_PLAY_SPEED_BUTT /**< Invalid speed value */ /**< CNcomment:无效的速度值 */
} MT_SVR_PLAYER_PLAY_SPEED_E;

/** Media source type */
typedef enum mtMT_SVR_PLAYER_MEDIA_SOURCE_TYPE {
    /**
     * General media source, player get multimedia data from file or streaming protocol
     */
    MT_SVR_PLAYER_MEDIA_SOURCE_NORMAL   = 0,
    /**
     * User defined media source, player get multimedia data from user callback i/o functions.
     */
    MT_SVR_PLAYER_MEDIA_SOURCE_EXTIO    = 1,
} MT_SVR_PLAYER_MEDIA_SOURCE_TYPE_E;

/** The parameters need to be specified when the player is created. */
/** CNcomment:播放器创建时需指定的参数 */
typedef struct moSVR_PLAYER_PARAM_S
{
    MT_U32 u32DmxId; /*@deprecate*//**< Dmx ID. Not used now. */                                                                                                                       /**< CNcomment:dmx id，目前没有使用*/
    MT_U32 u32PortId; /*@deprecate*//**< Port ID. Not used now. */                                                                                                                     /**< CNcomment:port id，目前没有使用*/
    MT_U32 x; /*@deprecate*//**< Coordinate of the video output window. This parameter is invalid in the case of hAVPlayer!=MT_SVR_PLAYER_INVALID_HDL. */                              /**< CNcomment:视频输出窗口坐标，hAVPlayer!=MT_SVR_PLAYER_INVALID_HDL，该参数无效 */
    MT_U32 y; /*@deprecate*//**< Coordinate of the video output window. This parameter is invalid in the case of hAVPlayer!=MT_SVR_PLAYER_INVALID_HDL. */                              /**< CNcomment:视频输出窗口坐标，hAVPlayer!=MT_SVR_PLAYER_INVALID_HDL，该参数无效 */
    MT_U32 w; /*@deprecate*//**< Width of the video output window. This parameter is invalid in the case of hAVPlayer!=MT_SVR_PLAYER_INVALID_HDL. */                                   /**< CNcomment:视频输出窗口宽，hAVPlayer!=MT_SVR_PLAYER_INVALID_HDL，该参数无效 */
    MT_U32 h; /*@deprecate*//**< Height of the video output window. This parameter is invalid in the case of hAVPlayer!=MT_SVR_PLAYER_INVALID_HDL. */                                  /**< CNcomment:视频输出窗口高，hAVPlayer!=MT_SVR_PLAYER_INVALID_HDL，该参数无效 */
    MT_U32 u32MixHeight; /*@deprecate*//**< Audio output mix weight. The value ranges from 0 to 100. This parameter is invalid in the case of hAVPlayer!=MT_SVR_PLAYER_INVALID_HDL. */ /**< CNcomment:音频输出混音权重0-100，hAVPlayer!=MT_SVR_PLAYER_INVALID_HDL，该参数无效 */
    MT_HANDLE hAVPlayer;/*@deprecate*/                                                                                                                                                 /**< AVPlay created externally. A window device and a sound device are bound to the AVPlay and the window size and the sound volume are set.
                                                                    The AVPlay handle can be transferred to the SuPlayer, which will use the AVPlay for playing. If this parameter is set to
                                                                    MT_SVR_PLAYER_INVALID_HDL, the SuPlayer internally creates the AVPlay and window automatically. */
                                                                                                                                                                         /**< CNcomment:外部已经创建了avplay并绑定了window,sound，并设置好window的大小位置，sound的音量，
                                                                    可以将avplay句柄传给SuPlayer，SuPlayer继续使用该avplay播放。如果该参数设置为
                                                                    MT_SVR_PLAYER_INVALID_HDL，SuPlayer内部会自动创建avplayer和window */
    MT_HANDLE hVSink;/*@deprecate*/                                                                                                                                                    /**< vsink handle create external */
    MT_HANDLE hASink;/*@deprecate*/                                                                                                                                                    /**< asink handle create external */
    MT_U32 u32SndPort; /*@deprecate*//**< Specified audio port number, 0: master audio, 1: slave audio  */                                                                             /**< CNcomment:指定音频端口号，0代表主音，1代表辅音*/
    MT_U32 u32Display; /*@deprecate*//**< Type of display, value is::MT_UNF_DISP_E */                                                                                                  /**< CNcomment:display 类型，取值参考::MT_UNF_DISP_E */
    MT_U32 u32VDecErrCover; /*@deprecate*/                                                                                                                                             /**<Error concealment threshold of the output frames of a video decoder. The value 0 indicates that no frames are output if an error occurs; the value 100 indicates that all frames are output no matter whether errors occur.*/
                                                                                                                                                                         /**<CNcomment: 视频解码器的输出帧错误隐藏门限，0:出现错误即不输出；100:不管错误比例全部输出*/
    MT_HANDLE hDRMClient;/*@deprecate*/                                                                                                                                                /**< DRM client created externally. */
    MT_HANDLE hWindow;/*@deprecate*/                                                                                                                                                   /**< window handle create external */
    void *suplayer_status;
    void *pb_heap_mem_start;/*input memory buffer for file play*/
    MT_U32 heapMemSize;   /*input memory size for file play*/
    MT_U8 audio_output;
    void *transport_desdec_ctx; /* transport stream descramble or decryption context */
    /*
     * callback function : descramble or decrypt bytes after transport_scrambling_control flag.
     * and we send whole transport packet to caller.
     *
     * ctx [in] user callback context.
     * info[in] information maybe used by caller, call be NULL.
     * in  [in] scrambled or encrypted data.
     * out [in] descrambled or decrypted data.
     * len [in] scrambled or encrypted data data bytes.
     *
     * retval ::MT_SUCCESS if descramble or decrypt bytes success.
     * retval ::MT_FAILURE if descramble or decrypt bytes fail.
     */
    int (*transport_desdec_func) (void *ctx, const void *info, const unsigned char *in, unsigned char *out, int len);
} MT_SVR_PLAYER_PARAM_S;

/*
 * External user defined i/o context
 *
 * Set NULL if callback functions not available
*/
typedef struct MT_SVR_PLAYER_EXTIO_CONTEXT {
    /**
     * External user defined i/o context handle, passed to the read/seek/... functions..
     */
    void *opaque;
    /**
     * Initialize user defined input/output context
     *
     * @param opaque     A private user io handle pointer, passed to the read/seek/... functions..
     * @return >=0 on success or negative on error.
     */
    int (*init) (void *opaque);
    /**
     * Close the resource accessed by user defined input/output context and free it.
     *
     * @param opaque     A private user io handle pointer
     * @return >=0 on success or negative on error.
     */
    int (*deinit) (void *opaque);
    /**
     * Read size bytes from opaque context into buf.
     *
     * @param opaque     A private user io handle pointer
     * @param buf        buffer for keeping data
     * @param buf_size   buffer size
     *
     * @return number of bytes read on success or negative on error.
     */
    int (*read) (void *opaque, unsigned char *buf, int buf_size);
    /**
     * Seek to a given byte position in stream with the specified whence
     * as fseek defined in header <stdio.h>:SEEK_CUR/SEEK_SET/SEEK_END
     *
     * @param opaque     A private user io handle pointer
     * @param offset     A specified byte position in stream
     * @param whence     position to which offset is added.
     *                   It can have one of the following values: SEEK_SET, SEEK_CUR, SEEK_END
     *                      SEEK_SET:argument to seek indicating seeking from beginning of the file
     *                      SEEK_CUR:argument to seek indicating seeking from the current file position
     *                      SEEK_END:argument to seek indicating seeking from end of the file
     *
     * @return new position or negative on error.
     */
    long long (*seek) (void *opaque, long long offset, int whence);
    /**
     * Checks if the end of the given stream has been reached.
     *
     * @param opaque     A private user io handle pointer
     *
     * @return non zero if data EOF, otherwise 0
     */
    int (*eof) (void *opaque);
    /**
     * Get the filesize of the given stream.
     *
     * @param opaque     A private user io handle pointer
     *
     * @return the filesize upon success, or negative on error.
     */
    long long (*get_size) (void *opaque);
    /**
     * Obtains the file position for the file stream and stores them in the object pointed to by pos.
     *
     * @param opaque     A private user io handle pointer
     * @param pos        Pointer to a long long object to store the file position indicator to
     *
     * return 0 upon success, nonzero value otherwise.
     */
    int (*get_pos) (void *opaque, long long *pos);
    /**
     * Checks the given stream seekable or not.
     *
     * @param opaque     A private user io handle pointer
     *
     * @return non zero if seekable, otherwise 0
     */
    int (*get_seekable) (void *opaque);
    /**
     * Get the file type of the given stream.
     *
     * @param opaque     A private user io handle pointer
     *
     * @return media type on success or negative on error.
     */
    int (*get_media_type) (void *opaque);
} MT_SVR_PLAYER_EXTIO_CONTEXT_S;

/** Input media file */
/** CNcomment:输入的媒体文件 */
typedef struct mtSVR_PLAYER_MEDIA_S
{
    /*
     * Support local absolute file location,
     * stream url--http,https,rtsp and so on, and seperat url defined by user.
     *
     * User defined url contains seperat audio url, video url,
     * and start with "mtsurl://" prefix, should follow rules below:
     *
     * Format:
     * mtsurl://KEY="Value"
     *
     * Whitespace and potential commas can be used as the sperator, Value must be included in '\"' pairs.
     *
     * example:
     * mtsurl://VIDEO="http://10.10.128.121/media/video.mp4",AUDIO="http://10.10.128.121/media/audio.mp4",TYPE="vod"
     *
     * Keyword:
     *
     * [VIDEO]   : video url    [optional],
     *             Can be network http or https file url or m3u8 Media playlist, cannot be master playlist
     * [AUDIO]   : audio url    [optional],
     *             Can be network http or https file url or m3u8 Media playlist, cannot be master playlist
     * TYPE:
     *      "vod"  : media is on demand
     *      "live" : media is living stream
     *               Not support other type, default "vod"
     */
    MT_CHAR aszUrl[MT_FORMAT_MAX_URL_LEN];
    MT_S32 s32PlayMode; /* @deprecate *//**< Set the mode of the player. The parameter is ::MT_SVR_PLAYER_PLAY_MODE_E */                                     /**< CNcomment:设置播放模式,参数::MT_SVR_PLAYER_PLAY_MODE_E */
    MT_U32 u32ExtSubNum; /* @deprecate *//**< Number of subtitle files */                                                                                    /**< CNcomment:字幕文件个数 */
    MT_CHAR aszExtSubUrl[MT_FORMAT_MAX_LANG_NUM][MT_FORMAT_MAX_URL_LEN]; /* @deprecate *//**< Absolute path of a subtitle file, such as /mnt/filename.ts. */ /**< CNcomment:字幕文件路径，绝对路径，如/mnt/filename.ts */
    MT_U32 u32UserData;/* @deprecate */                                                                                                                      /**< Create a handle by calling the fmt_open function, send the user data to the DEMUX by calling the fmt_invoke, and then call the fmt_find_stream function. */
                                                                                                                                             /**< CNcomment:用户数据，SuPlayer仅作透传，调用解析器fmt_open之后，通过fmt_invoke接口传递给解析器，再调用fmt_find_stream接口 */
    MT_CHAR aszCertPath[MT_FORMAT_MAX_URL_LEN]; /**< File path, absolute file path, such as /mnt/filename.ts. */                             /**< CNcomment:文件路径，绝对路径，如/mnt/filename.ts */
    MT_SVR_PLAYER_IN_PARA_S para; /* para for player */
    MT_SVR_PLAYER_EXTIO_CONTEXT_S *extio_cb; /* external user defined i/o context */
} MT_SVR_PLAYER_MEDIA_S;

/** IDs of the AV and subtitle streams to be switched. */
/** CNcomment:要切换到的音视频、字幕流ID  */
typedef struct mtSVR_PLAYER_STREAMID_S
{
    MT_U16 u16ProgramId;   /*@deprecate*//**< Program ID. The value is the subscript of the pastProgramInfo array in the ::MT_FORMAT_FILE_INFO_S structure and ranges from 0 to (u32ProgramNum - 1). */
                           /**< CNcomment:节目id，值为::MT_FORMAT_FILE_INFO_S结构pastProgramInfo数组下标，0-(u32ProgramNum - 1) */
    MT_U16 u16VidStreamId; /*@deprecate*//**< Video stream ID. The value is the subscript of the pastVidStream array in the ::MT_FORMAT_PROGRAM_INFO_S structure and ranges from 0 to (u32VidStreamNum - 1). */
                           /**< CNcomment:视频流id，值为::MT_FORMAT_PROGRAM_INFO_S结构pastVidStream数组下标，0-(u32VidStreamNum - 1)*/
    MT_U16 u16AudStreamId; /*@deprecate*//**< Audio stream ID. The value is the subscript of the pastAudStream array in the ::MT_FORMAT_PROGRAM_INFO_S structure and ranges from 0 to (u32AudStreamNum - 1).*/
                           /**< CNcomment:音频流id，值为::MT_FORMAT_PROGRAM_INFO_S结构pastAudStream数组下标，0-(u32AudStreamNum - 1)*/
    MT_U16 u16SubStreamId; /*@deprecate*//**< Subtitle ID. The value is the subscript of the pastSubStream array in the ::MT_FORMAT_PROGRAM_INFO_S structure and ranges from 0 to (u32SubStreamNum - 1). */
                           /**< CNcomment:字幕id，值为::MT_FORMAT_PROGRAM_INFO_S结构astSubTitle数组下标，0-(u32SubStreamNum - 1) */
} MT_SVR_PLAYER_STREAMID_S;

/** Player information */
/** CNcomment:播放器信息 */
typedef struct mtSVR_PLAYER_INFO_S
{
    MT_U32 film_duration; /**< file duration, in the unit of ms. */               /**< CNcomment:文件时间，单位ms */
    MT_U64 u64TimePlayed; /**< Elapsed time, in the unit of ms. */                /**< CNcomment:已播放时间，单位ms */
    MT_S32 s32Speed; /**< Playing speed */                                        /**< CNcomment:播放速率 */
    MT_SVR_PLAYER_STATE_E eStatus; /**< Playing status */                         /**< CNcomment:播放状态 */
} MT_SVR_PLAYER_INFO_S;

#ifndef FILE_SEQ_SUBTITLE_LEN
#define FILE_SEQ_SUBTITLE_LEN 20
#endif

#ifndef FILE_SEQ_SUBTITLE_CNT
#define FILE_SEQ_SUBTITLE_CNT 20
#endif

typedef struct {
    /*!
      language about subtitle
      */
    MT_CHAR lang[FILE_SEQ_SUBTITLE_LEN];
    /*!
    title string
    */
    MT_CHAR title[FILE_SEQ_SUBTITLE_LEN];
    /*!
    codec type
    */
    MT_CHAR code[FILE_SEQ_SUBTITLE_LEN];
    /*!
    subtitle index id
    */
    MT_S32    id;
} MT_SVR_PLAYER_SUBTITLE_S;

/*!
  xxxxxxxx
  */
typedef struct {
    MT_S32 cnt;
    MT_SVR_PLAYER_SUBTITLE_S subtitle[FILE_SEQ_SUBTITLE_CNT];
} MT_SVR_PLAYER_SUBT_S;

/*!
  xxxxxxxx
  */
typedef struct {
    MT_SVR_PLAYER_SUBT_S *subt_array;
    MT_U32 pts;
} MT_SVR_PLAYER_SUBT_DATA_S;

typedef struct {
        MT_U32   film_duration;
        MT_S32    audio_type;
        MT_S32    video_type;
        MT_S32    audio_track_num;
        MT_S32    video_track_num;
        MT_BOOL  canTrickPlay;
        MT_S32 video_disp_w;
        MT_S32 video_disp_h;
        MT_S32 video_fps;
        MT_S32  video_bps;//bytes per secondes
        MT_U64 file_size;
        MT_S32 audio_track_id;
        MT_CHAR * audio_language; // pointers to a modification forbidden memory space
        MT_S32 audio_bps;
        MT_S32 audio_samplerate;
        MT_UCHAR * file_name; // pointers to a modification forbidden memory space
} MT_SVR_PLAYER_FILM_S;

/*!
  @brief track language info
  */
typedef struct MT_SVR_PLAYER_TRACK_LANG{
    /*!
      track id
      */
    MT_S32 track_id;
    /*!
      language
      */
    MT_CHAR * lang;
    /*!
      title
      */
    MT_CHAR * title;
    /*!
      format
      */
    MT_S32 format;
} MT_SVR_PLAYER_TRACK_LANG_S;

typedef enum {
    MT_SVR_PLAYER_PLAYLIST_FLAGS_NONE   = 0x00,
    MT_SVR_PLAYER_PLAYLIST_FLAGS_ENABLE = 0x01,  /**< Is it enabled, 1:enable */
} MT_SVR_PLAYER_PLAYLIST_FLAGS_E;

/*!
  @brief adaptive playlist information
  */
typedef struct MT_SVR_PLAYER_ADAPTIVE_PLAYLIST {
    int index;
    int width;
    int height;
    int bandwidth;
    struct {
        int num; ///< Numerator
        int den; ///< Denominator
    } framerate;
    /* codec_tag is a "four" character codec identifier for a compression format,
     * A character in this context is a 1 byte/8 bit value,
     * The four characters is generally limited to be within the human readable characters in the ASCII table.
     * Such as "H264", "H264" and so on.
     */
    unsigned int codec_tag;
    MT_SVR_PLAYER_PLAYLIST_FLAGS_E flags;
} MT_SVR_PLAYER_ADAPTIVE_PLAYLIST_S;

typedef struct MT_SVR_PLAYER_FILE_INFO{
        MT_SVR_PLAYER_FILM_S film;
        MT_SVR_PLAYER_SUBT_S *subt;// pointers to a modification forbidden memory space
        MT_SVR_PLAYER_TRACK_LANG_S *aud_lang;
        MT_U32 aud_lang_cnt;
        MT_S32 adaptive_playlist_num; /* total number of adaptive playlist */
        MT_S32 adaptive_playlist_idx; /* current played adaptive playlist index */
} MT_SVR_PLAYER_FILE_INFO_S;

/** @} */ /** <!-- ==== Structure Definition end ==== */

/******************************* API declaration *****************************/
/** \addtogroup      Suplayer */
/** @{ */ /** <!-- [Suplayer]*/

/**
\brief Player event callback function. The ::MT_SVR_PLAYER_RegCallback interface can be called to register the callback function. CNcomment:播放器事件回调函数，调用::MT_SVR_PLAYER_RegCallback接口注册该回调函数CNend
\attention \n
None.
\param[out] hPlayer player handle. CNcomment:播放器句柄CNend
\param[out] pstruEvent event parameter. CNcomment: 事件参数CNend

\retval ::MT_SUCCESS

\see \n
None.
*/
typedef MT_U32 (*MT_SVR_PLAYER_EVENT_FN)(MT_HANDLE hPlayer, MT_SVR_PLAYER_EVENT_S *pstruEvent);

/**
\brief Initialize the player. CNcomment:初始化Player CNend
\attention \n
The MT_S32 MT_SVR_PLAYER_Init interface can be called to return a success message after the initialization is successful. Multiple processes are not supported
. This interface must be called prior to other interfaces.
CNcomment:初始化成功后再调用该接口返回成功，不支持多进程，调用其它接口前必须先调用该接口CNend
\param None. CNcomment:无CNend

\retval ::MT_SUCCESS The initialization is successful. CNcomment:初始化成功CNend
\retval ::MT_FAILURE The initialization fails. CNcomment:初始化失败CNend

\see \n
None.
*/
MT_VOID* MT_SVR_PLAYER_Init(mt_void* args);

/**
\brief Deinitialize the player module by calling the MT_S32 MT_SVR_PLAYER_Deinit interface. The player module is not used any more. CNcomment:去初始化player模块，不再使用player模块，调用该接口去初始化player模块CNend
\attention \n
The ::MT_SVR_PLAYER_Destroy interface must be called to release the created player first. Otherwise, a failure is returned. The Deinit interface does not release the player resource. \n
This interface can be called to return a success message after the deinitialization is successful.
CNcomment:必须先调用::MT_SVR_PLAYER_Destroy接口释放掉创建的播放器，再调用该接口，否则会返回失败，Deinit不负责释放\n
播放器资源。去初始化成功后再调用该接口返回成功CNend
\param None. CNcomment:无CNend

\retval ::MT_SUCCESS The deinitialization is successful. CNcomment:去初始化成功CNend
\retval ::MT_FAILURE The deinitialization fails and the created player is not released. CNcomment:去初始化失败，没有释放掉创建的播放器CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_Deinit(MT_HANDLE hPlayer);

/**
\brief Create a player. CNcomment:创建一个播放器CNend
\attention \n
This interface must be called after initialization. Only one player can be created. \n
CNcomment:该接口必须在初始化后调用，支持最多创建一个播放器\n CNend
\param[in] pstruParam player initialization attribute. CNcomment:播放器初始化属性CNend
\param[out] phPlayer handle of the created player. CNcomment:创建的播放器句柄CNend

\retval ::MT_SUCCESS A player is created successfully and the player handle is valid. CNcomment:创建成功，播放器句柄有效CNend
\retval ::MT_FAILURE A player fails to be created. The parameters are invalid or resources are insufficient. CNcomment:创建失败，参数非法或资源不够CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_Create(const MT_SVR_PLAYER_PARAM_S *pstruParam, MT_HANDLE *phPlayer);

/**
\brief Set the URL of the media file to be played. The URL format is absolute path + media file name. CNcomment:设置要播放的媒体文件url地址，url格式为:绝对路径+媒体文件名CNend
\attention \n
The MT_SVR_PLAYER_SetMedia interface is a synchronous interface. It must be called after the ::MT_SVR_PLAYER_Create interface is called or after the ::
MT_SVR_PLAYER_Stop interface is called to stop the player.
This interface performs the following operations:
1. Queries the media file set by the DEMUX during parsing.
2. Queries subtitle files in the same directory as the audio and video media file automatically, and the queried subtitle files' name are the same as the name of the media file.
3. Performs the following steps if no AVPlay is specified:
   Creates an AVPlay and sets the audio and video attributes based on the parsed file attributes such as the AV encoding type.
   Creates a window, set the window size, and bind the window to the AVPlay.
   Creates a audio track(use device MT_UNF_SND_0),and bind it to the AVPlay.
The window display location and mixheight set for binding audio track to the AVPlay are specified when the ::MT_SVR_PLAYER_Create interface is called.
The window display location and mixheight are invalid if the AVPlay is specified externally.
CNcomment:同步接口，该接口必须在调用::MT_SVR_PLAYER_Create之后调用，或者调用::MT_SVR_PLAYER_Stop接口停止播放器后调用
该接主要执行以下处理:
1、查找解析器解析设置的媒体文件
2、在与媒体文件相同的路径下查找与媒体文件同名的字幕文件
3、如果没有指定avplay则
   创建avplay，根据解析出的文件属性设置音视频属性，如音视频编码类型
   创建window，设置window窗口位置，将window绑定到avplay
   创建audio track(使用设备MT_UNF_SND_0)，并将audio track绑定到avplay
window显示位置，绑定到avplay的audio track的mixheight(如果avplay是外部指定，则显示位置和mixheight参数无效)，
在调用::MT_SVR_PLAYER_Create接口时指定CNend

\param[in] hPlayer handle of the player is created by calling the ::MT_SVR_PLAYER_Create interface. CNcomment:通过调用::MT_SVR_PLAYER_Create接口创建的播放器句柄CNend
\param[in] eType media file. see MT_SVR_PLAYER_MEDIA_SOURCE_TYPE_E
\param[in] pstruMedia media file information. Only absolute path is supported.  \n
           The player searches for a subtitle file whose name is the same as the name of the audio and video media file in the same directory as the audio and video media file automatically.
            CNcomment:媒体文件信息，只支持绝对路径。播放器会自动在音视频媒\n
            体文件所在目录下查找与音视频媒体文件名相同的字幕文件CNend

\retval ::MT_SUCCESS The media file is set successfully. The ::MT_SVR_PLAYER_Play interface can be called to start playing the file. CNcomment:媒体文件设置成功，此时调用::MT_SVR_PLAYER_Play接口可以开始播放CNend
\retval ::MT_FAILURE The media file fails to be set. CNcomment:媒体文件设置失败CNend
\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_SetMedia(MT_HANDLE hPlayer, MT_U32 eType, MT_SVR_PLAYER_MEDIA_S *pstruMedia);

/*@deprecate*/
/**
\brief Switch a new Demux source.This interface is not supported now. CNcomment:重新切换数据源，该接口当前不支持CNend
\attention \n
The MT_SVR_PLAYER_SetSource interface is a synchronous interface. It must be called after the ::MT_SVR_PLAYER_Stop interface is called. Call
::MT_SVR_PLAYER_Play to play the new Demux source.
The handle of the new demux source is created by call the ::MT_SVR_FORMAT_Open function.
CNcomment:调用::MT_SVR_PLAYER_Stop停止播放器后，调用该接口重新绑定一个数据源，调用::MT_SVR_PLAYER_Play接口重新播放新的数据源，新数据源句柄通过调用::MT_SVR_FORMAT_Open函数创建CNend
\param[in] hPlayer player handle. CNcomment:播放器句柄CNend
\param[in] hFormat The handle of the new demux source. CNcomment:新数据源句柄CNend

\retval ::MT_SUCCESS The new demux source is set successfully. The ::MT_SVR_PLAYER_Play interface can be called to start playing the file. CNcomment:新数据源设置成功，此时调用::MT_SVR_PLAYER_Play接口可以开始播放CNend
\retval ::MT_FAILURE The new demux source fails to be set. CNcomment:新数据源设置失败CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_SetSource(MT_HANDLE hPlayer, MT_HANDLE hFormat);

/**
\brief Destroy a player instance. CNcomment:销毁一个播放器实例CNend
\attention \n
The MT_S32 MT_SVR_PLAYER_Destroy interface is called to destroy the player resource after the ::MT_SVR_PLAYER_Create interface is called to create a player.
CNcomment:调用::MT_SVR_PLAYER_Create创建播放器后，调用该接口销毁播放器资源CNend
\param[in] hPlayer player handle. CNcomment:播放器句柄CNend

\retval ::MT_SUCCESS The player is released successfully. CNcomment:播放器释放成功CNend
\retval ::MT_FAILURE The operation is invalid. CNcomment:非法操作CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_Destroy(MT_HANDLE hPlayer);

/**
\brief Set player parameters. CNcomment:设置播放器参数CNend
\attention \n
The following operations are supported:
The ::MT_SVR_PLAYER_ATTR_STREAMID interface can be used to set the ID of the stream to be played. It can be called to set the audio streams to be played for
single-video multi-audio files. \n
Calling the MT_SVR_PLAYER_SetParam interface is an asynchronous operation during playing. Nevertheless, calling this interface is a synchronous operation
after the SetMedia operation is performed prior to playing. Values returned by the interface cannot be used to check whether the operation is successful.
The player notifies the application (APP) of the stream ID setting status by using the ::MT_SVR_PLAYER_EVENT_STREAMID_CHANGED event. The event parameter is ::
MT_SVR_PLAYER_STREAMID_S.
CNcomment:该接口必须在调用::MT_SVR_PLAYER_SetMedia接口后调用CNend
CNcomment:支持的操作如下
::MT_SVR_PLAYER_ATTR_STREAMID : 设置播放的流id，对于单视频、多音频文件，可以通过该操作设置要播放的音频流\n
播放过程中调用该接口为异步操作，播放前，SetMedia后调用该接口为同步操作，异步操作不能通过接口返回值来判断\n
流id是否设置成功，播放器会通过::MT_SVR_PLAYER_EVENT_STREAMID_CHANGED事件通知app，事件参数为::MT_SVR_PLAYER_STREAMID_S。\n
::MT_SVR_PLAYER_ATTR_SYNC : 设置音视频、字幕流时间戳偏移，SuPlayer读取音视频帧后将时间戳加上设置的偏移值再用于同步CNend

\param[in] hPlayer player handle. CNcomment:播放器句柄CNend
\param[in] eAttrId ID of the player parameter to be set. CNcomment:要设置的播放器参数ID CNend
\param[in] pArg player parameter to be set. CNcomment:要设置的播放器参数CNend

\retval ::MT_SUCCESS Parameters are set successfully. CNcomment:参数设置成功CNend
\retval ::MT_FAILURE The operation fails. CNcomment:操作失败CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_SetParam(MT_HANDLE hPlayer, MT_SVR_PLAYER_ATTR_E eAttrId, const MT_VOID *pArg);

/**
\brief Obtain player parameters. CNcomment:获取播放器参数CNend
\attention \n
This interface must be called after the ::MT_SVR_PLAYER_SetMedia interface is called.
The following operations are supported:
::MT_SVR_PLAYER_ATTR_STREAMID: Obtain the ID of the stream that is played currently.
::MT_SVR_PLAYER_ATTR_WINDOW_HDL: Obtain the window handle created by the player.
::MT_SVR_PLAYER_ATTR_AVPLAYER_HDL: Obtain the AVPlay handle created by the player.
::MT_SVR_PLAYER_ATTR_SO_HDL: Obtain the SO module handle created by the player.
::MT_SVR_PLAYER_ATTR_AUDTRACK_HDL:Obtain the audio track handle created by the player.

CNcomment:该接口必须在调用::MT_SVR_PLAYER_SetMedia接口后调用
支持的操作如下
::MT_SVR_PLAYER_ATTR_STREAMID :获取当前播放的流id
::MT_SVR_PLAYER_ATTR_WINDOW_HDL :获取播放器创建的window句柄
::MT_SVR_PLAYER_ATTR_AVPLAYER_HDL :获取播放器创建的avplay句柄
::MT_SVR_PLAYER_ATTR_SO_HDL :获取播放器创建的字幕输出模块(so)句柄
::MT_SVR_PLAYER_ATTR_AUDTRACK_HDL :获取播放器创建的音频track id CNend

\param[in] hPlayer player handle. CNcomment:播放器句柄CNend
\param[in] eAttrId player parameter ID. CNcomment:播放器参数ID CNend
\param[out] pArg obtained player parameters. CNcomment:获取的播放器参数CNend

\retval ::MT_SUCCESS Parameters are obtained successfully. CNcomment:获取参数成功CNend
\retval ::MT_FAILURE The operation fails. CNcomment:操作失败CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_GetParam(MT_HANDLE hPlayer, MT_SVR_PLAYER_ATTR_E eAttrId, MT_VOID *pArg);

/**
\brief Register a player event callback function. CNcomment:注册播放器事件回调函数CNend
\attention \n
This interface must be called after the ::MT_SVR_PLAYER_Create interface is called. This interface is unrelated to the player status. This function cannot be called in any player callback event.
CNcomment:该函数必须在调用::MT_SVR_PLAYER_Create接口后调用，该接口与播放器状态无关，该函数不能在player任何回调事件中调用CNend
\param[in] hPlayer player handle. CNcomment:播放器句柄CNend
\param[in] pfnCallback player event callback function. The callback function definition is ::MT_SVR_PLAYER_EVENT_FN. CNcomment:播放器事件回调函数，回调函数定义::MT_SVR_PLAYER_EVENT_FN CNend

\retval ::MT_SUCCESS The registration is successful. CNcomment:注册成功CNend
\retval ::MT_FAILURE The registration fails. CNcomment:设置失败CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_RegCallback(MT_HANDLE hPlayer, MT_SVR_PLAYER_EVENT_FN pfnCallback);

/**
\brief Start playing. CNcomment:开始播放CNend
\attention \n
This interface is an asynchronous interface. It must be called after the ::MT_SVR_PLAYER_SetMedia interface is called. If this interface is called after successful playing, MT_SUCCESS is returned. \n
Values returned by this interface cannot be used to check whether the playing is successful. The player notifies the APP of playing success or failure by using the ::MT_SVR_PLAYER_EVENT_STATE_CHANGED event. \n
The event parameter value is ::MT_SVR_PLAYER_STATE_PLAY.This interface can be called to restart playing after the playing stops.

CNcomment:异步接口，该接口必须在调用::MT_SVR_PLAYER_SetMedia接口后调用，播放成功后再调用该接口返回MT_SUCCESS，\n
不能通过该接口返回值来判断播放器是否播放成功，播放器会通过::MT_SVR_PLAYER_EVENT_STATE_CHANGED 事件通知\n
app播放成功，事件参数值为::MT_SVR_PLAYER_STATE_PLAY。停止播放后，可以调用该接口重新播放CNend

\param[in] hPlayer player handle. CNcomment:播放器句柄CNend

\retval ::MT_SUCCESS The operation is valid. CNcomment:合法操作CNend
\retval ::MT_FAILURE The operation is invalid. CNcomment:非法操作CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_Play(MT_HANDLE hPlayer, MT_S64 start_msec);

/**
\brief Stop playing. CNcomment:停止播放CNend
\attention \n
This interface is an asynchronous interface. It can be called to stop playing during playing, fast forward, rewind, and pause. Values returned by this interface cannot be used to check whether playing is stopped successfully. \n
The player notifies the APP of stop success or failure by using the ::MT_SVR_PLAYER_EVENT_STATE_CHANGED event. The event parameter value is ::MT_SVR_PLAYER_STATE_STOP. \n
The ::MT_SVR_PLAYER_Play interface can be called to restart playing after playing is stopped.

CNcomment:异步接口，播放、快进、快退、暂停过程中都可以调用该接口停止播放，不能通过该接口返回值来判断播放器\n
是否停止成功，播放器会通过::MT_SVR_PLAYER_EVENT_STATE_CHANGED 事件通知app停止成功，事件参数值为::MT_SVR_PLAYER_STATE_STOP。\n
停止播放后，可以调用::MT_SVR_PLAYER_Play接口重新播放CNend

\param[in] hPlayer player handle. CNcomment:播放器句柄CNend

\retval ::MT_SUCCESS The operation is valid. CNcomment:合法操作CNend
\retval ::MT_FAILURE The operation is invalid. CNcomment:非法操作CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_Stop(MT_HANDLE hPlayer);

/**
\brief Pause playing.  CNcomment:暂停播放CNend
\attention \n
This interface is an asynchronous interface. It can be called to pause playing but cannot be called during fast forward and rewind. Values returned by this interface cannot be used to check whether playing is paused successfully. \n
The player notifies the APP of pause success or failure by using the ::MT_SVR_PLAYER_EVENT_STATE_CHANGED event. The event parameter value is ::MT_SVR_PLAYER_STATE_PAUSE. \n
The ::MT_SVR_PLAYER_Resume interface can be called to resume playing after a successful pause.
CNcomment:异步接口，播放过程中可以调用该接口暂停播放，快进、快退状态下不能调用该接口，不能通过该接口返回值来判断播放器\n
是否暂停成功，播放器会通过::MT_SVR_PLAYER_EVENT_STATE_CHANGED 事件通知app停止成功，事件参数值为::MT_SVR_PLAYER_STATE_PAUSE。\n
暂停成功后，可以调用::MT_SVR_PLAYER_Resume继续播放CNend

\param[in] hPlayer player handle. CNcomment:播放器句柄CNend

\retval ::MT_SUCCESS The operation is valid. CNcomment:合法操作CNend
\retval ::MT_FAILURE The operation is invalid. CNcomment:非法操作CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_Pause(MT_HANDLE hPlayer);

/**
\brief Resume playing. CNcomment:恢复播放CNend
\attention \n
This interface is an asynchronous interface. It can be called to resume playing during pause, fast forward, and rewind. Values returned by this interface cannot be used to check whether playing is resumed successfully. \n
The player notifies the APP of resumption success or failure by using the ::MT_SVR_PLAYER_EVENT_STATE_CHANGED event. The event parameter value is ::MT_SVR_PLAYER_STATE_PLAY.

CNcomment:异步接口，暂停、快进、快退状态下，调用该接口恢复正常播放，不能通过该接口返回值来判断播放器是否恢复播放\n
播放器会通过::MT_SVR_PLAYER_EVENT_STATE_CHANGED 事件通知app恢复成功，事件参数值为::MT_SVR_PLAYER_STATE_PLAY。CNend

\param[in] hPlayer player handle. CNcomment:播放器句柄CNend

\retval ::MT_SUCCESS The operation is valid. CNcomment:合法操作CNend
\retval ::MT_FAILURE The operation is invalid. CNcomment:非法操作CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_Resume(MT_HANDLE hPlayer);

/**
\brief Fast forward and rewind. CNcomment:快进、快退CNend
\attention \n
This interface is an asynchronous interface. It must be called in play, pause or tplay status. The negative value of s32Speed indicates rewind and the
positive value of s32Speed indicates fast forward. Values returned by the interface cannot be used to check whether the fast forward or rewind is successful.
\n
The player notifies the APP of the fast forward or rewind status by using the ::MT_SVR_PLAYER_EVENT_STATE_CHANGED event. \n
The event parameter value is ::MT_SVR_PLAYER_STATE_FORWARD or ::MT_SVR_PLAYER_STATE_BACKWARD. The ::MT_SVR_PLAYER_Resume interface can be called to \n
resume normal playing after fast forward or rewind.
CNcomment:异步接口，该函数必须播放、暂停或快进/快退状态下调用，s32Speed为负表示快退，为正表示快进，不能通过该接口\n
返回值来判断是否快进或快退成功，播放器会通过::MT_SVR_PLAYER_EVENT_STATE_CHANGED 事件通知app，事件参数值为\n
::MT_SVR_PLAYER_STATE_FORWARD或::MT_SVR_PLAYER_STATE_BACKWARD，快进、快退后通过调用::MT_SVR_PLAYER_Resume\n
接口恢复正常播放。CNend

\param[in] hPlayer player handle. CNcomment:播放器句柄CNend
\param[in] s32Speed playing speed. The value is MT_SVR_PLAYER_PLAY_SPEED_E. CNcomment:播放倍数,值为MT_SVR_PLAYER_PLAY_SPEED_E CNend

\retval ::MT_SUCCESS The operation is valid. CNcomment:合法操作CNend
\retval ::MT_FAILURE The operation is invalid. CNcomment:非法操作CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_TPlay(MT_HANDLE hPlayer, MT_S32 s32Speed);

/**
\brief wait trick play finish. CNcomment:等待快进、退完成CNend
\attention \n
This interface is an asynchronous interface. It must be called after the ::MT_SVR_PLAYER_SetMedia interface is called. It can be called to jump to a specified time point for playing.
CNcomment:异步接口，该接口必须在调用::MT_SVR_PLAYER_SetMedia接口后调用，播放、停止状态下，调用该接口跳到指定时间点播放CNend

\param[in] hPlayer player handle. CNcomment:播放器句柄CNend
\param[in] timeout_ms wait finish timeout. CNcomment:等待快进、退完成的时间，单位ms CNend

\retval ::MT_SUCCESS The operation is successful. CNcomment:快进、退成功CNend
\retval ::MT_FAILURE The operation is invalid. CNcomment:非法操作CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_Wait_TPlay_Finish(MT_HANDLE hPlayer, mt_u32 timeout_ms);

/**
\brief Seek to a specified location for playing. CNcomment:跳到指定位置播放CNend
\attention \n
This interface is an asynchronous interface. It must be called after the ::MT_SVR_PLAYER_SetMedia interface is called. It can be called to jump to a specified time point for playing.
CNcomment:异步接口，该接口必须在调用::MT_SVR_PLAYER_SetMedia接口后调用，播放、停止状态下，调用该接口跳到指定时间点播放CNend

\param[in] hPlayer player handle. CNcomment:播放器句柄CNend
\param[in] s64TimeInMs seeking time. The unit is ms. CNcomment:seek时间，单位ms CNend

\retval ::MT_SUCCESS The operation is successful. CNcomment:Seek成功CNend
\retval ::MT_FAILURE The operation is invalid. CNcomment:非法操作CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_Seek(MT_HANDLE hPlayer, mt_s64 s64TimeInMs);

/**
\brief wait seek finish. CNcomment:等待seek完成CNend
\attention \n
This interface is an asynchronous interface. It must be called after the ::MT_SVR_PLAYER_SetMedia interface is called. It can be called to jump to a specified time point for playing.
CNcomment:异步接口，该接口必须在调用::MT_SVR_PLAYER_SetMedia接口后调用，播放、停止状态下，调用该接口跳到指定时间点播放CNend

\param[in] hPlayer player handle. CNcomment:播放器句柄CNend
\param[in] timeout_ms wait finish timeout. CNcomment:等待seek完成的时间，单位ms CNend

\retval ::MT_SUCCESS The operation is successful. CNcomment:Seek成功CNend
\retval ::MT_FAILURE The operation is invalid. CNcomment:非法操作CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_Wait_Seek_Finish(MT_HANDLE hPlayer, mt_u32 timeout_ms);

/** @deprecate */
/**
\brief Seek to a specified location for playing. CNcomment:跳到指定位置播放CNend
\attention \n
This interface is an asynchronous interface. It must be called after the ::MT_SVR_PLAYER_SetMedia interface is called. It can be called to jump to a specified file position  for playing.
CNcomment:异步接口，该接口必须在调用::MT_SVR_PLAYER_SetMedia接口后调用，播放、停止状态下，调用该接口跳到指定文件位置播放CNend

\param[in] hPlayer player handle. CNcomment:播放器句柄CNend
\param[in] s64Offset offset(in bytes) of the starting position. CNcomment:相对文件起始位置的字节偏移CNend

\retval ::MT_SUCCESS The operation is successful. CNcomment:Seek成功CNend
\retval ::MT_FAILURE The operation is invalid. CNcomment:非法操作CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_SeekPos(MT_HANDLE hPlayer, mt_s64 s64Offset);

/** @deprecate */
/**
\brief Invoke operation. Application expand operation. The player will transparently transmits the invoke to the demux. CNcomment:invoke操作，app扩展使用，player会将该操作透传给demux CNend
\attention \n
It must be called after the ::MT_SVR_PLAYER_SetMedia interface is called.
CNcomment:该接口必须在调用::MT_SVR_PLAYER_SetMedia接口后调用CNend

\param[in] hPlayer player handle. CNcomment:播放器句柄CNend
\param[in] u32InvokeId Operation ID of application expanding,the type of u32InvokeId is MT_FORMAT_INVOKE_ID_E. CNcomment:app扩展的操作id, 参考MT_FORMAT_INVOKE_ID_E CNend
\param[in/out] pArg param of the invoking operation. CNcomment:操作参数CNend

\retval ::MT_SUCCESS The operation is successful. CNcomment:操作成功CNend
\retval ::MT_FAILURE The operation is invalid. CNcomment:非法操作CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_Invoke(MT_HANDLE hPlayer, MT_U32 u32InvokeId, MT_VOID *pArg);

/**
\brief Obtain information about the current open file, such as the file size, playing duration, file bit rate, video width, video height, coding format, frame rate, video bit rate, audio encoding, and audio bit rate.
CNcomment:获取当前打开文件信息，如文件大小、文件播放时长、码率等，视频宽、高，编码格式，帧率、码率，音频编码、码率等CNend
\attention \n
This interface must be called after the ::MT_SVR_PLAYER_SetMedia interface is called.
CNcomment:该函数必须在调用::MT_SVR_PLAYER_SetMedia接口后调用CNend

\param[in] hPlayer player handle. CNcomment:播放器句柄CNend
\param[out] not support now. ppstruInfo obtained file information. CNcomment:现在不支持此参数。 获取的文件信息CNend

\retval ::MT_SUCCESS The file information is obtained successfully. CNcomment:获取到文件信息CNend
\retval ::MT_FAILURE The file information fails to be obtained. CNcomment:获取文件信息失败CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_LOADMEDIA_GetFileInfo(MT_HANDLE hPlayer, MT_FORMAT_FILE_INFO_S **ppstruInfo);

/**
\brief Obtain the player information, such as the current playing status, playing progress, and elapsed time. CNcomment:获取播放器信息，如当前播放状态、播放进度、已播放时间等CNend
\attention \n
This interface must be called after the ::MT_SVR_PLAYER_Create interface is called. The playing progress and elapsed time are valid only after the ::MT_SVR_PLAYER_Play interface is called.
CNcomment:该函数必须在调用::MT_SVR_PLAYER_Create接口后调用，播放进度、已播放时间只有在调用::MT_SVR_PLAYER_Play接口后有效
::MT_SVR_PLAYER_SetMedia未返回前不允许调用该接口CNend

\param[in] hPlayer player handle. CNcomment:播放器句柄CNend
\param[out] pstruInfo player information. CNcomment:播放器信息CNend

\retval ::MT_SUCCESS The player information is obtained successfully. CNcomment:播放器信息获取成功CNend
\retval ::MT_FAILURE The player information fails to be obtained. CNcomment:播放器信息获取失败CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_GetPlayerInfo(MT_HANDLE hPlayer, MT_SVR_PLAYER_INFO_S *pstruInfo);

/**
\brief Enable or disable the player debug. CNcomment:打开/关闭player dbg信息CNend
\attention \n
This interface can be called in any state.
CNcomment:该接口可以在任何状态调用CNend

\param[in] bEnable enable value. MT_TRUE: enables the player log. MT_FALSE: disables the player log. CNcomment:使能值，MT_TRUE:打开player日志信息，MT_FALSE:关闭日志信息CNend

\retval :: None.

\see \n
None.
*/
MT_VOID MT_SVR_PLAYER_EnableDbg(MT_BOOL bEnable);

/**
\brief Obtain the player version. CNcomment:获取播放器版本号CNend
\attention \n
This interface can be called in any state.
CNcomment:该接口可以在任何状态调用CNend

\param[out] pstVersion player version. CNcomment:播放器版本号CNend

\retval :: None.

\see \n
None.
*/
MT_VOID MT_SVR_PLAYER_GetVersion(MT_FORMAT_LIB_VERSION_S *pstVersion);

MT_S32 MT_SVR_PLAYER_Set_Aud_Track(MT_HANDLE hPlayer, int track_id);
MT_S32 MT_SVR_PLAYER_Set_Subtitle(MT_HANDLE hPlayer, int sub_id);
MT_S32 MT_SVR_PLAYER_GetAdaptivePlaylist(MT_HANDLE hPlayer,
    int list_array_num, MT_SVR_PLAYER_ADAPTIVE_PLAYLIST_S *list);
MT_S32 MT_SVR_PLAYER_SetAdaptivePlaylist(MT_HANDLE hPlayer, int playlist_index);

/*@deprecate*/
MT_S32 MT_SVR_DUMP_VIDEO_DATA(MT_HANDLE hPlayer);
/*@deprecate*/
MT_S32 MT_SVR_DUMP_AUDIO_DATA(MT_HANDLE hPlayer);

MT_S32 MT_SVR_PLAYER_Get_Media_Info(MT_HANDLE hPlayer, void * pResult);
MT_S32 MT_SVR_PLAYER_Get_Subt_Data(MT_HANDLE hPlayer, void * subt, void *pts, MT_U32 *size);
MT_S32 MT_SVR_PLAYER_SetAvplayHdl(MT_HANDLE hPlayer, MT_HANDLE hAvplay, MT_HANDLE hTrack);
MT_S32 MT_SVR_PLAYER_SetAvplayHdl_Only(MT_HANDLE hPlayer, MT_HANDLE hAvplay);
MT_S32 MT_SVR_PLAYER_SetVoHdl(MT_HANDLE hPlayer,int vHandle);
MT_S32 MT_SVR_PLAYER_GetTagInfo(MT_HANDLE hPlayer, char *p_tag_key, char *p_tag_value, int tag_value_len);
MT_S32 MT_SVR_PLAYER_SetSupportSeekFinish(MT_HANDLE hPlayer);
MT_S32 MT_SVR_PLAYER_Wait_TPlay_Finish(MT_HANDLE hPlayer, mt_u32 timeout_ms);
MT_S32 MT_SVR_PLAYER_Wait_Seek_Finish(MT_HANDLE hPlayer, mt_u32 timeout_ms);
MT_S32 MT_SVR_PLAYER_SET_NETWORK_BUFFERING(MT_HANDLE hPlayer, MT_BOOL open);

MT_S32 MT_SVR_PLAYER_Get_Media_Info_Nolock(
							MT_HANDLE hPlayer, void *pResult);
MT_S32 MT_SVR_PLAYER_Set_T1xbase_Num(MT_HANDLE hPlayer,
								int t1xnum);
MT_S32 MT_SVR_PLAYER_Init_Adec_Param_Ptr(void);
MT_S32 MT_SVR_PLAYER_Get_Adec_Param_Ptr(void *ptr, int size);
MT_S32 MT_SVR_PLAYER_Init_Es_Ptr(int max_frame, int max_asize);
MT_S32 MT_SVR_PLAYER_Get_Es_Magic_Num(void *ves,
				void *aes, void *ses);

typedef struct mtUNF_SUPLAYER_IN_ARG
{
	mtUNF_SUPLAYER_TYPE_E	ptype;
	MT_VOID* pri;
}mtUNF_SUPLAYER_IN_ARG_S;

typedef unsigned int i_os_sem_t;
typedef struct mtUNF_SUPLAYER_STATUS
{
	MT_U32	instance;
	MT_SVR_PLAYER_STATE_E	status;
	mtUNF_SUPLAYER_TYPE_E ptype;
	MT_VOID* pri;
	i_os_sem_t suplock;
}mtUNF_SUPLAYER_STATUS_S;

/*@deprecate*/
#define CRC_BUF_SIZE (4*15*60)
/*@deprecate*/
typedef struct mtUNF_SUPLAYER_AUTOTEST_STATUS{
    mt_u32 crcBegFlag;
    mt_u32 crcbuf[CRC_BUF_SIZE / 4];
    mt_u32 crcEndFlag;
    int cmd_id;
    int a_codec_type;
    int v_codec_type;
    int s_codec_type;
    char adec_param[128];
    unsigned long long aes_magic_num;
    unsigned long long ves_magic_num;
    unsigned long long ses_magic_num;
    int adec_error;
    int vdec_error;
    int avsync_error;
    int display_error;
    mt_u32 adeccrcBegFlag;
#define DEBUG_AUDIO_CRC_BUF_SIZE		(4*15*60)
    mt_u32 adeccrcbuf[DEBUG_AUDIO_CRC_BUF_SIZE>>2];
    mt_u32 adeccrcEndFlag;
}mtUNF_SUPLAYER_AUTOTEST_STATUS_S;
/** @} */ /** <!-- ==== API declaration end ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MTSU_SVR_PLAYER_H__ */
