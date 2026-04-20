/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/**
 * \file
 * \brief Describes the information about the personal video recorder (PVR). CNcomment: 提供PVR的相关信息 CNend
 */
#ifndef __MT_UNF_PVR_H__
#define __MT_UNF_PVR_H__

#include "mt_type.h"
#include "mt_debug.h"
#ifdef CONFIG_MT_CHIP_SYMPHONY1
#include "mt_unf_cipher.h"
#else
#include "mt_unf_cipher_v2.h"
#endif
#include "mt_unf_common.h"
#include "mt_unf_avplay.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* End of #ifdef __cplusplus */

//#define DEF_ADVCA_PVR  			      1                 //support nagra adv ca pvr
//#define NGR_ADVCA_PVR  			      1                 //support nagra adv ca pvr
//#define VMX_ADVCA_PVR  			             1                 //support vmx adv ca pvr

#if 1
#define vmx_pvr_rec_printf(...)      do{}while(0)
#define vmx_pvr_play_printf(...)    do{}while(0)
#define PVR_EVENT_DD
#define PVR_EVENT_D(...)                do{}while(0)
#endif
/* for debug */
//#define PVR_UNF_DEBUG
/*************************** Structure Definition ****************************/
/** \addtogroup    PVR */
/** @{ */  /** <!--[PVR]*/

/**Recommended value of the recording attribute u32ScdBufSize for vmx*/
/** CNcomment: 录制属性中u32ScdBufSize的推荐值,给vmx使用，需满足0x2f0000整数倍 */
#define PVR_STUB_SC_BUF_SZIE   (188*4096*4)    //(188*4096*4*2)

/**Recommended value of the recording attribute u32DavBufSize,size of ts buffer*/
/** CNcomment:  录制属性中u32DavBufSize的推荐值 ts buffer 大小，如果使能了dio又没启用vmx，需要满足0x2f0000的整数倍*/
#define PVR_STUB_TSDATA_SIZE   0x2f0000

/**Recommended value of the recording attribute u32IdxBufSize*/
/** CNcomment:  录制属性中u32IdxBufSize的推荐值 index 的缓冲buffer大小*/
#define PVR_STUB_IDX_SHM_SIZE (12*2048)

/*****************Macro Definition*******************************/
/**Maximum length of the file name, in byte*//** CNcomment:  文件名最大长度，单位:字节*/
#define PVR_MAX_FILENAME_LEN            256
/**Maximum length of the encryption key, in byte. Note: The valid key length may not be the maximum length, and depends on the encryption algorithm.*/
/** CNcomment: 加密密钥最大长度，单位:字节。注意: 密钥的有效长度不一定是最大长度，而是由加密的算法决定 */
#define PVR_MAX_CIPHER_KEY_LEN          128
/**Maximum length of the user data, in byte*//** CNcomment:  用户数据最大长度， 单位:字节 */
#define PVR_MAX_USERDATA_LEN            (1024*1024)
#define PVR_MAX_CADATA_LEN              (1*1024)

#define MPVR_BIT0 (1<<0)
#define MPVR_BIT1 (1<<1)
#define MPVR_BIT2 (1<<2)
#define MPVR_BIT3 (1<<3)
#define MPVR_BIT4 (1<<4)
#define MPVR_BIT5 (1<<5)
#define MPVR_BIT6 (1<<6)
#define MPVR_BIT7 (1<<7)

/*Macro Definition for timeshift event */
#define MT_INVALID_EVENT_INDEX                                   (0xff)
#define MT_INVALED_REC_CHNID                                       (0xff)

#define PVR_DEFAULT_FILE_NODE_SIZE_FOR_EVENT       (2000LLU*1024*1024) 
#define PVR_MIN_FILE_NODE_SIZE_FOR_EVENT               (20LLU*1024*1024)   //20 m
#define PVR_MIN_LOOP_REC_TIME_FOR_EVENT                 (30*60)   //30*60 s 
#define PVR_TIMESHIFT_EVENT_NAME_LEN                       (128)
#define PVR_TIMESHIFT_EVENT_SAVE_TIMES                    (128)
/***********************************************************
                Data Structure Definition
 ***********************************************************/

typedef enum mtpvr_timeshift_event_type
{
    TIMESHIFT_EVENT_START_NORMAL, /*start a event*//*CNcomment:启动一个事件*/
    TIMESHIFT_EVENT_STOP_START,   /*stop this and start another event*//*CNcomment:停止当前事件并启动新事件*/
    TIMESHIFT_EVENT_BUTT
}PVR_TIMESHIFT_EVENT_TYPE;

typedef enum pvr_thread_type
{
    PVR_INDEX_READ,               /*thread of reading index*//*CNcomment:读index 线程*/
    PVR_INDEX_WRITE,              /*thread of writing index*//*CNcomment:写index 线程*/
    PVR_TS_WRITE,                 /*thread of writing data*//*CNcomment:写数据 线程*/
    PVR_TS_READ                   /*thread of reading data*//*CNcomment:读数据 线程*/
}PVR_THREAD_TYPE;
/***** Common Data Structures*****/

/**Cyclic control data structure of the PVR index file*//** CNcomment: PVR index文件中循环控制结构信息 */
typedef struct mtPVR_CYC_HEADER_INFO_S
{
    MT_U32 u32StartFrame;     /**<Offset of the start frame in the index file. The start frame is at u32StartFrame.*//**<CNcomment: 码流的起始帧在index文件中的偏移, 起始帧放在u32StartFrame的位置*/
    MT_U32 u32EndFrame;       /**<Offset of the end frame in the index file. The end frame is at u32EndFrame - 1.*//**<CNcomment: 码流的结束帧在index文件中的偏移, 结束帧放在u32EndFrame-1的位置*/
    MT_U32 u32LastFrame;      /**<Offset of the last frame in the index file. The last frame is at u32LastFrame - 1.*//**<CNcomment: index文件中结尾帧的偏移, 结尾帧放在u32LastFrame-1的位置*/
    MT_U32 u32IsRewind;       /**<Whether to record the file in rewind mode. 0: no rewind; 1: rewind*//**<CNcomment: 文件是否回绕录制, 0 非回绕, 1 回绕*/
    MT_U32 u32TimeShiftEventLoopFirstNode;
}PVR_CYC_HEADER_INFO_S;

typedef struct mtPVR_IDEX_SHARE_EVENT_S
{
    MT_CHAR  shareFileName[PVR_TIMESHIFT_EVENT_NAME_LEN]; 
    MT_U64     u64ShareFileOffset;    
}PVR_IDX_SHARE_EVENT_S;

/**Structure of the file header in the PVR index file*//** CNcomment: PVR index文件中文件头信息结构 */
typedef struct mtPVR_IDX_HEADER_INFO_S
{
    MT_U32 u32StartCode;                          /**<Start code (0x5A5A5A5A)*//**<CNcomment: 起始码(0x5A5A5A5A) */
    MT_U32 u32HeaderLen;                         /**<Length from u32StartCode to UsrData*//**<CNcomment: 从u32StartCode开始到UsrData结束的长度 */
    PVR_CYC_HEADER_INFO_S stCycInfo;              /**<Records the cyclic control information in the index file*//**<CNcomment: index中录制环回控制信息 */
    MT_U32 u32Reserved;                           /**<Reserved, for alignment*//**<CNcomment:保留，用作对齐 */
    MT_U64 u64ValidSize;                           /**<Size of the valid content in a file, in byte*//**<CNcomment:文件中有效内容大小 单位Byte */
    MT_U32 u32UsrDataInfoLen;                 /**<User information length*//**<CNcomment: 用户信息长度 */
    MT_U32 u32CADataInfoLen;
    MT_U16  shareFlagTsFile;                  /*bit valule:  1: share, 0: no share */
    MT_U16  u8ShareEventCount;
    PVR_IDX_SHARE_EVENT_S  shareEvent[PVR_TIMESHIFT_EVENT_SAVE_TIMES];
}PVR_IDX_HEADER_INFO_S;

/**Index data saved in the PVR index file*//** CNcomment: PVR index文件中保存的索引数据结构 */
typedef struct mtPVR_INDEX_ENTRY_S
{
    MT_U16 u16FrameTypeAndGop;                    /**<2-bit frame type and 14-bit group of pictures (GOP)*/
    MT_U8  u16UpFlowFlag;                         /**<DAV overflow flag*/
    MT_U8  u8StcOffsinTs;                         /**stc offset in tspacket*/
    MT_U32 s32CycTimes;                           /**<Number of times that the file is rewound*/
    MT_U64 u64GlobalOffset;                       /**<Total global offset (rewind ignored), in byte*/
    MT_U32 u32DisplayTimeMs;                      /**<Display time (in ms), no rewind*/
    MT_U32 u32FrameSize;                          /**<Frame size*/
    MT_U64 u64Offset;                             /**<Frame offset in a stream file*/
    MT_U32 u32PtsMs;                              /**<Presentation time stamp (PTS), in ms*/
    MT_U16 u16IndexType;                          /**<Index type*/
    MT_U16 u161stFrameOfTT;                       /**<Useless*/
}PVR_INDEX_ENTRY_S,*PTR_PVR_INDEX_ENTRY;


/**Encryption or decryption configuration of the PVR*//** CNcomment:  PVR加解密配置 */
typedef struct mtUNF_PVR_CIPHER_S
{
    MT_BOOL             bDoCipher;                             /**<Whether to perform encryption or decryption. When this parameter is set to MT_FALSE, other parameters are invalid.*//**<CNcomment:  是否进行加密或者解密，此项配置为MT_FALSE时，其他项没有意义 */
    MT_CIPHER_ALGORITHM_E enType;                                /**<Algorithm for encryption or decryption*//**<CNcomment: 加密或者解码的算法类型 */
    MT_U8               au8Key[PVR_MAX_CIPHER_KEY_LEN];        /**<In normal PVR,it's cipher key.If advance security PVR, it's Keyladder's first session key*//**<CNcomment: 普通PVR方案中，作为Cipher的密钥，高安全PVR方案中，作为Keyladder的一级会话密钥 */
    MT_U32              u32KeyLen;                             /**<Cipher key length*//**<CNcomment: 密钥长度 */
}MT_UNF_PVR_CIPHER_S;

/**Type of the PVR event*//** CNcomment:  PVR事件类型 */
typedef enum mtUNF_PVR_EVENT_E
{

    MT_UNF_PVR_EVENT_PLAY_EOF        = 0x001,          /**<A file is played at the end of file (EOF).*//**<CNcomment:  播放到文件尾 */
    MT_UNF_PVR_EVENT_PLAY_SOF        = 0x002,          /**<A file is played to the start of file (SOF).*//**<CNcomment:  回退到文件头 */
    MT_UNF_PVR_EVENT_PLAY_ERROR      = 0x003,          /**<An internal error occurs during playing.*//**<CNcomment:  播放内部错误 */
    MT_UNF_PVR_EVENT_PLAY_REACH_REC  = 0x004,          /**<The playing speed reaches the recording speed during time shift.*//**<CNcomment:  时移的时候播放追上录制 */
    MT_UNF_PVR_EVENT_PLAY_RESV       = 0x00f,          /**<Reserved*//**<CNcomment:  保留 */
    MT_UNF_PVR_EVENT_REC_DISKFULL    = 0x010,          /**<The disk is full.*//**<CNcomment:  磁盘满 */
    MT_UNF_PVR_EVENT_REC_ERROR       = 0x011,          /**<An internal error occurs during recording.*//**<CNcomment:  录制内部错误 */
    MT_UNF_PVR_EVENT_REC_OVER_FIX    = 0x012,          /**<The length of the recorded data reaches the specified length. This event is available only in non-rewind mode.*//**<CNcomment: 录制长度达到指定的长度,只有非回绕录制才有这个事件 */
    MT_UNF_PVR_EVENT_REC_REACH_PLAY  = 0x013,          /**<The recording speed reaches the playing speed during time shift.*//**<CNcomment:  时移的时候录制追上播放 */
    MT_UNF_PVR_EVENT_REC_DISK_SLOW   = 0x014,          /**<The storage speed of the disk is slower than the recording speed.*//**<CNcomment: 磁盘存储速度慢于录制速度 */
    MT_UNF_PVR_EVENT_REC_DMX_CREATE  = 0x015,          /**<PVR created demux rec channel notify, as project get rec demux channel buf to config .*//**<CNcomment: 创建dmx rec channel通知,高安项目根据此消息获取录制通道buffer进行配置 */
    MT_UNF_PVR_EVENT_REC_RESV        = 0x01f,          /**<Reserved*//**<CNcomment:  保留 */
    MT_UNF_PVR_EVENT_BUTT            = 0x020           /**<Invalid event type*//**<CNcomment: 无效的事件类型 */
} MT_UNF_PVR_EVENT_E;

/**Callback function of PVR event*//** CNcomment: PVR事件回调函数 */
typedef void (*eventCallBack)(MT_U32 u32ChnID, MT_UNF_PVR_EVENT_E EventType, MT_S32 s32EventValue, MT_VOID *args);

/**Type of recorded and played streams of the PVR*//** CNcomment: PVR录制、播放的码流类型 */
typedef enum mtUNF_PVR_STREAM_TYPE_E
{
    MT_UNF_PVR_STREAM_TYPE_TS,        /**<Transport stream (TS)*//**<CNcomment:  TS流 */
    MT_UNF_PVR_STREAM_TYPE_PES,       /**<Packetized elementary stream (PES)*//**<CNcomment: PES流 */
    MT_UNF_PVR_STREAM_TYPE_ALL_TS,    /**<All types of streams. That is, the streams at all frequencies are recorded.*//**<CNcomment: 全码流录制，录制整个频点的所有码流 */
    MT_UNF_PVR_STREAM_TYPE_OTHER,     /**<Invalid*//**<CNcomment:  无效的类型 */
    MT_UNF_PVR_STREAM_TYPE_BUTT
} MT_UNF_PVR_STREAM_TYPE_E;

/**Index type*//** CNcomment: 索引类型 */
typedef enum mtUNF_PVR_REC_INDEX_TYPE_E
{
    MT_UNF_PVR_REC_INDEX_TYPE_NONE,      /**<No index is created. You need to select this type when recording all streams.*//**<CNcomment: <不建索引。全码流录制的时候请使用此类型  */
    MT_UNF_PVR_REC_INDEX_TYPE_VIDEO,     /**<The index is created based on videos.*//**<CNcomment: <针对视频建立索引。 */
    MT_UNF_PVR_REC_INDEX_TYPE_AUDIO,     /**<The index is created based on audios. *//**<CNcomment: <针对音频建立索引。 */
    MT_UNF_PVR_REC_INDEX_TYPE_BUTT       /**<Invalid*//**<CNcomment: <无效的索引类型。   */
} MT_UNF_PVR_REC_INDEX_TYPE_E;

/**Information about a stream file recorded by the PVR*//** CNcomment:  PVR录制的码流文件信息 */
typedef struct mtUNF_PVR_FILE_ATTR_S
{
    MT_UNF_PVR_REC_INDEX_TYPE_E enIdxType;          /**<Index type*//**<CNcomment:  索引类型 */
    MT_U32                      u32FrameNum;        /**<For a video file: number of frames in the file. For an audio file: number of PESs in the file*//**<CNcomment:  对视频文件：文件帧数;对音频文件：文件中的PES数 */
    MT_U32                      u32StartTimeInMs;   /**<Start time of the file, in ms*//**<CNcomment: 文件内容的起始时间，起始帧PTS，单位ms */
    MT_U32                      u32EndTimeInMs;     /**<End time of the file, in ms*//**<CNcomment: 文件内容的结束时间，结束帧PTS，单位ms  */
    MT_U64                      u64ValidSizeInByte; /**<Valid data length of the file, in byte *//**<CNcomment:  文件的有效数据长度，单位byte */
    MT_U64                      u64CurWPos_Glb;     /**<Size of the glob writing, in byte*//**<CNcomment: 一共录制了多少数据（单位：byte）。 */
} MT_UNF_PVR_FILE_ATTR_S;

/***** Data Structures for Recording  *****/
/** Record statuses*/
/**INVALID -> INIT -> RUNNING -> STOPPING ->    STOP ->    INVALID  (saveIndex)  (saveStream)*/

/**Status of a recording channel*//** CNcomment: 录制通道的状态 */
typedef enum mtUNF_PVR_REC_STATE_E
{
  MT_UNF_PVR_REC_STATE_INVALID,        /**<Not initialized*//**<CNcomment:  未初始化。    */
  MT_UNF_PVR_REC_STATE_INIT,           /**<Initialized*//**<CNcomment:  初始化。      */
  MT_UNF_PVR_REC_STATE_RUNNING,        /**<Recording*//**<CNcomment:  录制中。      */
  MT_UNF_PVR_REC_STATE_PAUSE,          /**<Pause*//**<CNcomment:  暂停中        */
  MT_UNF_PVR_REC_STATE_STOPPING,       /**<Stopping*//**<CNcomment: 正在停止。    */
  MT_UNF_PVR_REC_STATE_STOP,           /**<<Stopped*//**<CNcomment: 已经停止。    */
  MT_UNF_PVR_REC_STATE_BUTT            /**<Invalid*//**<CNcomment: 无效的状态值。*/
} MT_UNF_PVR_REC_STATE_E;

typedef enum mtUNF_PVR_CA_TYPE_E
{
  MT_UNF_PVR_REC_FREE=0,                /**<origin stream*//**<CNcomment:  原始流。 */
  MT_UNF_PVR_REC_DEFALT,              /**<default crypto*//**<CNcomment:  默认加密。 */
  MT_UNF_PVR_REC_COMMON_CRAMBLE,       /**<default cramble*//**<CNcomment:  默认加密。 */
  MT_UNF_PVR_REC_VMX,                 /**<vmx cramble*//**<CNcomment:  VMX 加扰。    */
  MT_UNF_PVR_REC_NAGRA,               /**<nagra crpto*//**<CNcomment:  nagra 加密。  */
  MT_UNF_PVR_REC_CONAX,               /**<conax crpto*//**<CNcomment:  conax 加密。  */
  MT_UNF_PVR_REC_IRDETO,              /**<irdeto crpto*//**<CNcomment:  irdeto 加密。  */
  MT_UNF_PVR_INVAID_MAX,              /**max invalid*//**<CNcomment:  无效。  */
} MT_UNF_PVR_REC_CA_TYPE_E;

/**TS Record modes*//** CNcomment: 索引类型 */
typedef enum mtUNF_PVR_REC_TS_REC_MODE_E
{
    MT_UNF_PVR_REC_FULL_TS_WITH_NULL_PACKET,      /**<Full TS Record with Null Packet.*//**<CNcomment: <全流录制带空包。 */
    MT_UNF_PVR_REC_FULL_TS_WITHOUT_NULL_PACKET,     /**<Full TS Record without Null Packet.*//**<CNcomment: <全流录制不带空包。 */
    MT_UNF_PVR_REC_TS_REC_MODE_BUTT       /**<Invalid*//**<CNcomment: <无效的索引类型。   */
} MT_UNF_PVR_REC_TS_REC_MODE_E;


/**Attributes of a recording channel*//** CNcomment: 录制通道属性数据结构 */
typedef struct mtUNF_PVR_REC_ATTR_S
{
    MT_UNF_PVR_REC_INDEX_TYPE_E enIndexType;                           /**<Index type, static attribute.*//**<CNcomment:  索引类型，静态属性。 */
    MT_UNF_VCODEC_TYPE_E        enIndexVidType;                        /**<Video encoding protocol with which the stream (for which an index is to be created) complies. The protocol (static attribute) needs to be set only when the index type is MT_UNF_PVR_REC_INDEX_TYPE_VIDEO.*/
                                                                       /**<CNcomment: 待建立索引的码流的视频编码协议, 索引类型是MT_UNF_PVR_REC_INDEX_TYPE_VIDEO时才需要配置,静态属性  */
    MT_U32                      u32IndexPid;                           /**<Index PID, static attribute.*//**<CNcomment:  索引PID，静态属性*/
    MT_U32                      u32DemuxID;                           /**<Recording DEMUX ID, static attribute.*//**<CNcomment:  录制DEMUX号，静态属性。*/
    MT_U32                      u32DavBufSize;                        /**<Size (in byte) of a data stream buffer, static attribute.*//**<CNcomment:  数据流缓冲区大小，单位为Byte，静态属性。  */
    MT_U32                      u32ScdBufSize;                         /**<Size (in byte) of an index data buffer, static attribute.*//**<CNcomment:  索引数据缓存区大小，单位为Byte，静态属性。*/
    MT_U32                      u32IdxBufSize;
    MT_U32                      u32UsrDataInfoSize;                 /**<Size (in byte) of the private user data, static attribute.*//**<CNcomment:  用户私有数据大小，单位为Byte，静态属性。  */
    MT_BOOL                    bIsClearStream;                        /**<Whether to record streams as clear streams, static attribute. This item indicates that the stored streams are clear streams or scrambled streams. If the programs to be recorded are scrambled streams, the clear streams can be recorded only after a descrambler is configured for the audio/video channel of the DEMUX.
                                                                       The recording module of the PVR does not descramble the scrambled streams.*/
                                                                       /**<CNcomment: 是否把码流录制成清流，静态属性。此项的意思是存下来的码流是清流还是加扰流。如果待录制的节目是加扰流，需要在录制DEMUX上启动一路CA才能录制清流,PVR录制模块并不负责对加扰流的解扰操作。*/
    MT_UNF_PVR_STREAM_TYPE_E    enStreamType;         /**<Type of the data to be recorded, static attribute.*//**<CNcomment: 待录制的数据类型，静态属性。*/
    MT_UNF_PVR_CIPHER_S         stEncryptCfg;                 /**<Stream encryption configuration, static attribute.*//**<CNcomment: 码流加密配置信息，静态属性。*/
    MT_U64                      u64MaxFileSize;                        /**<Size of the file to be recorded, static attribute. If the value is 0, it indicates that there is no limit on the file size. The minimum file size is 5242880 bytes (5 MB). If the disk space is sufficient, it is recommended to set the file size to a value greater than 512 MB. The file size cannot be 0 in rewind mode. */
                                                                       /**<CNcomment: 待录制的文件大小，为0时表示无限制，静态属性。最小值为50M,如果磁盘空间足够的话，推荐配置为512M以上。配置为回绕时不允许把文件大小和文件播放时间都配置为0。 */
    MT_U64                      u64MaxTimeInMs;    /**<Max play time of the file to be recorded, static attribute. If the value is 0, it indicates that there is no limit on the file play time. The minimum file time is 60*1024 MS (1 Min).  The file size  and file time cannot both be 0 in rewind mode.*/
                                                                       /**<CNcomment: 待录制的文件的最长播放时间长度，为0时表示无限制，静态属性。最小播放时间为60*1024毫秒(1分钟)。回绕模式下，不能将文件最大大小和文件最长播放时间同时设置为0。*/
    MT_BOOL                   bRewind;                 /**<Whether to rewind, static attribute. If this item is set to MT_TRUE, the PVR rewinds to the file header to record streams after the recorded file size reaches u64MaxFileSize. If this item is set to MT_FALSE, the PVR stops recording after the recorded file size reaches u64MaxFileSize.*/
                                                                       /**<CNcomment: 是否回绕，静态属性。如配置为MT_TRUE，录制文件大小到达u64MaxFileSize后，会回绕到文件头继续进行录制；如配置为MT_FALSE，则录制文件大小到达u64MaxFileSize后，会停止录制。*/
    MT_CHAR                   szFileName[PVR_MAX_FILENAME_LEN];      /**<Name of a stream file, static attribute*//**<CNcomment: 码流文件名，静态属性。*/
    MT_U32                      u32FileNameLen;                      /**<Length of a stream file name, static attribute. You need to set the item to strlen (szFileName).*//**<CNcomment: 码流文件名长度，静态属性。取值为strlen（szFileName）即可 */
    MT_BOOL                     u32DIO;                              /**use dio**//*<CNcomment:  使用dio写数据。 */
    MT_BOOL                    bEnable_CTrec;                        /**<whether continue rec.*//**<CNcomment: 是否启用断点续录,启用将会接着录，不启用则覆盖已有数据*/
    MT_UNF_PVR_REC_CA_TYPE_E   bSupportAdvCa;
    MT_BOOL                    bSupportTimeShiftEvent;               /*support timeshift event function*/
    MT_U32                       u32TimeShiftEventDataUnitSize;
    MT_U32                       u32TimeShiftEventLoopTimeInMs;
    MT_BOOL                    bNoInterval_InRec;                   /*no interval in recording if no signal*//**<CNcomment: 没信号录制时间仍然连续*/
    MT_BOOL                    bPvr_timeshift;                      /* is timeshift pvr*/
    MT_UNF_PVR_REC_TS_REC_MODE_E         enTsRecMode; /**<Demux TS Record modes for TS FULL record, static attribute.*//**<CNcomment: 全流录制模式，静态属性。*/
}MT_UNF_PVR_REC_ATTR_S;

/**Status of a PVR buffer*//** CNcomment:定义PVR缓冲区的状态信息 */
typedef struct mtUNF_PVR_BUF_STATUS_S
{
    MT_U32 u32BufSize;       /**<Buffer size*//**<CNcomment: 缓冲区大小       */
    MT_U32 u32UsedSize;      /**<Used buffer size*//**<CNcomment: 缓冲区已使用大小 */
} MT_UNF_PVR_BUF_STATUS_S;

/**Status of a recording channel.
The following describes the relationships among u32CurTimeInMs, u32StartTimeInMs, and u32EndTimeInMs by taking examples:
1. Assume that the total recording time is 10000 ms, and no rewind occurs. In this case, u32StartTimeInMs is 0 ms, u32EndTimeInMs is 10000 ms, and u32CurTimeInMs is 10000 ms.
2. Assume that the total recording time is 10000 ms, and rewind occurs at 8000 ms. In this case, u32StartTimeInMs is 2000 ms, u32EndTimeInMs is 10000 ms, and u32CurTimeInMs is 8000 ms.
*/
/** CNcomment:录制通道状态数据结构.
对u32CurTimeInMs、u32StartTimeInMs、u32EndTimeInMs三者的关系举例说明如下：
1·假设从开始录制后一共录制了10000毫秒，如果没有发生回绕，那么u32StartTimeInMs 为0，u32EndTimeInMs 为10000， u32CurTimeInMs为10000。
2·假设从开始录制后一共录制了10000毫秒，并且在第8000毫秒的时候发生了回绕，那么u32StartTimeInMs 为2000，u32EndTimeInMs 为10000， u32CurTimeInMs为8000。
*/
typedef struct mtUNF_PVR_REC_STATUS_S
{
    MT_UNF_PVR_REC_STATE_E      enState;                 /**<Current status of a channel*//**<CNcomment: 通道所处的状态。    */
    MT_U32                      u32Reserved;             /**<Reserved, for alignment*//**<CNcomment: 保留，用作对齐。    */
    MT_U64                      u64CurWritePos;          /**<Size of the currently recorded file, in byte*//**<CNcomment: 当前录制文件的大小（单位：byte）。 */
    MT_U32                      u32CurWriteFrame;        /**<Number of currently recorded frames*//**<CNcomment: 当前录制的帧数。                  */
    MT_U32                      u32CurTimeInMs;          /**<Current recording time (in ms). The value is the valid time for recording the file.*//**<CNcomment: 当前录制的毫秒数。其值为录制文件的实际有效时长。*/
    MT_U32                      u32StartTimeInMs;        /**<Actual start time for recording files. Before the recording is rewound, the time is that of the first frame; after the recording is rewound, the time is the rewind time.*//**<CNcomment: 录制文件的实际起始点。录制回绕前，为第一帧时间；录制回绕后，为回绕点的时间。 */
    MT_U32                      u32EndTimeInMs;          /**<Time of the end frame in the recorded file. Before the recording is rewound, the time is that of the last frame.*//**<CNcomment:录制文件中结束帧的时间，没有环回前即最后一帧的时间。    */
    MT_UNF_PVR_BUF_STATUS_S     stRecBufStatus;          /**<Status of the recording channel buffer*//**<CNcomment:录制通道缓冲区状态。    */
    MT_U32                      u32AppendRecMs;          /**<real time in recording*//**<CNcomment:真实录制时间，append rec 有效    */
    MT_U64                      u64CurWPos_Glb;          /**<Size of the globe writing, in byte*//**<CNcomment: 一共录制了多少数据（单位：byte）。 */
}MT_UNF_PVR_REC_STATUS_S;

/** Data structures for playing */

/**Playing direction of the PVR: forward*//** CNcomment:PVR播放方向:往前播放 */
#define MT_UNF_PVR_PLAY_DIRECTION_FORWARD             (1)
/**Playing direction of the PVR: backward*//** CNcomment: PVR播放方向:往后播放 */
#define MT_UNF_PVR_PLAY_DIRECTION_BACK                (-1)

/**Type of the playing position*//** CNcomment: 播放位置类型 */
typedef enum mtUNF_PVR_PLAY_POS_TYPE_E
{
  MT_UNF_PVR_PLAY_POS_TYPE_SIZE,              /**<The playing position is expressed by size (in byte). This type is not supported currently.*//**<CNcomment: 用大小（单位：byte）表示位置（暂不支持） */
  MT_UNF_PVR_PLAY_POS_TYPE_TIME,              /**<The playing position is expressed by time (in ms).*//**<CNcomment: 用时间（单位：ms）数表示位置 */
  MT_UNF_PVR_PLAY_POS_TYPE_FRAME,             /**<The playing position is expressed by the number of frames or PESs. This type is not supported currently.*//**<CNcomment: 用帧数（或者PES数）表示位置（暂不支持）  */
  MT_UNF_PVR_PLAY_POS_TYPE_BUTT               /**<Invalid*//**<CNcomment:无效的位置类型*/
} MT_UNF_PVR_PLAY_POS_TYPE_E;

/**Status of a playing channel*//** CNcomment: 播放通道的状态  */
typedef enum mtUNF_PVR_PALY_STATE_E
{
    MT_UNF_PVR_PLAY_STATE_INVALID,       /**<Not initialized*//**<CNcomment: 未初始化            */
    MT_UNF_PVR_PLAY_STATE_INIT,          /**<Initialized*//**<CNcomment: 初始化              */
    MT_UNF_PVR_PLAY_STATE_PLAY,          /**<Normal playing*//**<CNcomment: 正常播放中          */
    MT_UNF_PVR_PLAY_STATE_PAUSE,         /**<Pause*//**<CNcomment: 暂停                */
    MT_UNF_PVR_PLAY_STATE_FF,            /**<Fast forward*//**<CNcomment: 快进                */
    MT_UNF_PVR_PLAY_STATE_FB,            /**<Fast backward*//**<CNcomment: 快退\慢退                */
    MT_UNF_PVR_PLAY_STATE_SF,            /**<Slow forward*//**<CNcomment: 慢放                */
    MT_UNF_PVR_PLAY_STATE_STEPF,         /**<Step forward*//**<CNcomment:逐帧播放            */
    MT_UNF_PVR_PLAY_STATE_STEPB,         /**<Step backward*//**<CNcomment: 逐帧倒放            */
    MT_UNF_PVR_PLAY_STATE_STOP,          /**<Stop*//**<CNcomment: 停止                */
    MT_UNF_PVR_PLAY_STATE_BUTT           /**<Invalid*//**<CNcomment: 无效的状态值        */
} MT_UNF_PVR_PLAY_STATE_E;

/**Identifier of the playing speed*//** CNcomment: 播放速度标识 */
typedef enum mtUNF_PVR_PLAY_SPEED_E
{
    MT_UNF_PVR_PLAY_SPEED_NORMAL            = 1024,                                     /**<Normal playing speed*//**<CNcomment:正常速度播放。    */
    MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD   = 2 * MT_UNF_PVR_PLAY_SPEED_NORMAL,         /**<Fast forward at two times the normal speed*//**<CNcomment:2倍速快放。            */
    MT_UNF_PVR_PLAY_SPEED_4X_FAST_FORWARD   = 4 * MT_UNF_PVR_PLAY_SPEED_NORMAL,         /**<Fast forward at four times the normal speed*//**<CNcomment:4倍速快放。            */
    MT_UNF_PVR_PLAY_SPEED_8X_FAST_FORWARD   = 8 * MT_UNF_PVR_PLAY_SPEED_NORMAL,         /**<Fast forward at eight times the normal speed*//**<CNcomment:8倍速快放。            */
    MT_UNF_PVR_PLAY_SPEED_16X_FAST_FORWARD  = 16 * MT_UNF_PVR_PLAY_SPEED_NORMAL,        /**<Fast forward at 16 times the normal speed*//**<CNcomment:16倍速快放。           */
    MT_UNF_PVR_PLAY_SPEED_32X_FAST_FORWARD  = 32 * MT_UNF_PVR_PLAY_SPEED_NORMAL,        /**<Fast forward at 32 times the normal speed*//**<CNcomment:32倍速快放。           */
    MT_UNF_PVR_PLAY_SPEED_64X_FAST_FORWARD  = 64 * MT_UNF_PVR_PLAY_SPEED_NORMAL,        /**<Fast forward at 64 times the normal speed*//**<CNcomment:64倍速快放。           */
    MT_UNF_PVR_PLAY_SPEED_1X_FAST_BACKWARD  = -1 * MT_UNF_PVR_PLAY_SPEED_NORMAL,        /**<Fast backward at 1 times the normal speed*//**<CNcomment:1倍速快退。            */
    MT_UNF_PVR_PLAY_SPEED_2X_FAST_BACKWARD  = -2 * MT_UNF_PVR_PLAY_SPEED_NORMAL,        /**<Fast backward at two times the normal speed*//**<CNcomment:2倍速快退。            */
    MT_UNF_PVR_PLAY_SPEED_4X_FAST_BACKWARD  = -4 * MT_UNF_PVR_PLAY_SPEED_NORMAL,        /**<Fast backward at four times the normal speed*//**<CNcomment:4倍速快退。            */
    MT_UNF_PVR_PLAY_SPEED_8X_FAST_BACKWARD  = -8 * MT_UNF_PVR_PLAY_SPEED_NORMAL,        /**<Fast backward at eight times the normal speed*//**<CNcomment:8倍速快退。            */
    MT_UNF_PVR_PLAY_SPEED_16X_FAST_BACKWARD = -16 * MT_UNF_PVR_PLAY_SPEED_NORMAL,       /**<Fast backward at 16 times the normal speed*//**<CNcomment:16倍速快退。           */
    MT_UNF_PVR_PLAY_SPEED_32X_FAST_BACKWARD = -32 * MT_UNF_PVR_PLAY_SPEED_NORMAL,       /**<Fast backward at 32 times the normal speed*//**<CNcomment:32倍速快退。           */
    MT_UNF_PVR_PLAY_SPEED_64X_FAST_BACKWARD = -64 * MT_UNF_PVR_PLAY_SPEED_NORMAL,       /**<Fast backward at 64 times the normal speed*//**<CNcomment:64倍速快退。           */

    MT_UNF_PVR_PLAY_SPEED_2X_SLOW_FORWARD   = MT_UNF_PVR_PLAY_SPEED_NORMAL / 2,         /**<Slow forward at 1/2 times the normal speed*//**<CNcomment:1/2倍速慢放。          */
    MT_UNF_PVR_PLAY_SPEED_4X_SLOW_FORWARD   = MT_UNF_PVR_PLAY_SPEED_NORMAL / 4,         /**<Slow forward at 1/4 times the normal speed*//**<CNcomment:1/4倍速慢放。          */
    MT_UNF_PVR_PLAY_SPEED_8X_SLOW_FORWARD   = MT_UNF_PVR_PLAY_SPEED_NORMAL / 8,         /**<Slow forward at 1/8 times the normal speed*//**<CNcomment:1/8倍速慢放。          */
    MT_UNF_PVR_PLAY_SPEED_16X_SLOW_FORWARD  = MT_UNF_PVR_PLAY_SPEED_NORMAL / 16,        /**<Slow forward at 1/16 times the normal speed*//**<CNcomment:1/16倍速慢放。         */
    MT_UNF_PVR_PLAY_SPEED_32X_SLOW_FORWARD  = MT_UNF_PVR_PLAY_SPEED_NORMAL / 32,        /**<Slow forward at 1/32 times the normal speed*//**<CNcomment:1/32倍速慢放。         */
	MT_UNF_PVR_PLAY_SPEED_64X_SLOW_FORWARD  = MT_UNF_PVR_PLAY_SPEED_NORMAL / 64,        /**<Slow forward at 1/64 times the normal speed*//**<CNcomment:1/64倍速慢放。         */
    MT_UNF_PVR_PLAY_SPEED_43_SLOW_FORWARD   = MT_UNF_PVR_PLAY_SPEED_NORMAL*3 / 4,       /**<Slow forward at 3/4 times the normal speed*//**<CNcomment:3/4倍速慢放。         */

    MT_UNF_PVR_PLAY_SPEED_2X_SLOW_BACKWARD  = MT_UNF_PVR_PLAY_SPEED_NORMAL / (-2),      /**<Slow forward at 1/2 times the normal speed*//**<CNcomment:1/2倍速慢放。         */
    MT_UNF_PVR_PLAY_SPEED_4X_SLOW_BACKWARD  = MT_UNF_PVR_PLAY_SPEED_NORMAL / (-4),      /**<Slow forward at 1/4 times the normal speed*//**<CNcomment:1/4倍速慢放。         */
    MT_UNF_PVR_PLAY_SPEED_8X_SLOW_BACKWARD  = MT_UNF_PVR_PLAY_SPEED_NORMAL / (-8),      /**<Slow forward at 1/8 times the normal speed*//**<CNcomment:1/8倍速慢放。         */
    MT_UNF_PVR_PLAY_SPEED_16X_SLOW_BACKWARD = MT_UNF_PVR_PLAY_SPEED_NORMAL / (-16),     /**<Slow forward at 1/16 times the normal speed*//**<CNcomment:1/16倍速慢放。       */
    MT_UNF_PVR_PLAY_SPEED_32X_SLOW_BACKWARD = MT_UNF_PVR_PLAY_SPEED_NORMAL / (-32),     /**<Slow forward at 1/32 times the normal speed*//**<CNcomment:1/32倍速慢放。       */
    MT_UNF_PVR_PLAY_SPEED_64X_SLOW_BACKWARD = MT_UNF_PVR_PLAY_SPEED_NORMAL / (-64),     /**<Slow forward at 1/64 times the normal speed*//**<CNcomment:1/64倍速慢放。       */
    MT_UNF_PVR_PLAY_SPEED_43_SLOW_BACKWARD  = MT_UNF_PVR_PLAY_SPEED_NORMAL*3/ (-4),     /**<Slow forward at 3/4 times the normal speed*//**<CNcomment:3/4倍速慢放。         */
    MT_UNF_PVR_PLAY_SPEED_BUTT                                                          /**<Invalid value*//**<CNcomment:无效的速度值。         */
} MT_UNF_PVR_PLAY_SPEED_E;

/**Trick mode*//** CNcomment: 特技播放的模式  */
typedef struct mtUNF_PVR_PLAY_MODE_S
{
    MT_UNF_PVR_PLAY_SPEED_E               enSpeed;    /**<For details about the playing speed, see the description of MT_UNF_PVR_PLAY_SPEED_E.*//**<CNcomment: 速度，见MT_UNF_PVR_PLAY_SPEED_E的定义 */
}MT_UNF_PVR_PLAY_MODE_S;

/**Attributes of a playing channel*//** CNcomment:播放通道属性 */
typedef struct mtUNF_PVR_PLAY_ATTR_S
{
    MT_UNF_PVR_STREAM_TYPE_E    enStreamType;                         /**<Type of the played stream, static attributes. At present, only the TS is supported.*//**<CNcomment: 播放码流类型，目前只支持TS，静态属性。*/
    MT_BOOL                     bIsClearStream;                       /**<Whether the stream to be played is the clear stream, static attribute. Scrambled streams can be played in normal mode only rather than in trick mode.*//**<CNcomment: 待播放的码流是否清流，静态属性。加扰流不支持特技播放，只支持正常播放 */
    MT_UNF_PVR_CIPHER_S         stDecryptCfg;                         /**<Decryption configuration, static attribute                  *//**<CNcomment: 解密配置，静态属性。                  */
    MT_CHAR                     szFileName[PVR_MAX_FILENAME_LEN];     /**<Name of the file that contains the streams to be played, static attribute        *//**<CNcomment: 待播放码流的文件名，静态属性。        */
    MT_U32                      u32FileNameLen;                       /**<Length of the file name, static attribute. You need to set the item to strlen (szFileName).*//**<CNcomment: 文件名长度，静态属性。取值为strlen（szFileName）即可 */
    MT_UNF_PVR_REC_CA_TYPE_E    bSupportAdvCa;
    MT_BOOL                    bSupportTimeShiftEvent;           /*support timeshift event function*/
    MT_U32                       u32TimeShiftEventDataUnitSize;
    MT_BOOL                    bIsTimeShiftEventFile;
    MT_U32                       u32StartTimeOffset; /*for irdeto pvr support config start play on specified time offset(ms)*/
}MT_UNF_PVR_PLAY_ATTR_S;


/**Description of the playing position*//** CNcomment: 播放位置描述  */
typedef struct mtUNF_PVR_PLAY_POSITION_S
{
    MT_UNF_PVR_PLAY_POS_TYPE_E  enPositionType;       /**<Position type*//**<CNcomment: 位置类型。           */
    MT_S32                      s32Whence;            /**<Offset type, including SEEK_SET, SEEK_CUR, or SEEK_END*//**<CNcomment:偏移类型，取值范围为SEEK_SET/SEEK_CUR/SEEK_END。*/
    long long                      s64Offset;            /**<Offset. If the offset type is time, the offset is in the unit of millisecond; if the offset type is frame, the offset is in the unit of frame.*//**<CNcomment:偏移。 偏移类型为时间时，单位毫秒；偏移类型为帧时，单位为一帧 */
}MT_UNF_PVR_PLAY_POSITION_S;

/**Information required for creating an index file*//** CNcomment: 创建索引文件需要的信息 */
typedef struct mtUNF_PVR_GEN_IDX_ATTR_S
{
    MT_U32                      u32UsrDataInfoLen;         /**<Length of the user data*//**<CNcomment: 用户数据长度 */
    MT_U32                      u32IdxPid;                 /**<Index PID*//**<CNcomment: 针对哪个Pid建立索引 */
    MT_UNF_PVR_REC_INDEX_TYPE_E enIdxType;                 /**<Index type*//**<CNcomment: 索引类型 */
}MT_UNF_PVR_GEN_IDX_ATTR_S;

/**Status of a playing channel*//** CNcomment:播放通道的状态 */
typedef struct mtUNF_PVR_PLAY_STATUS_S
{
    MT_UNF_PVR_PLAY_STATE_E     enState;                              /**<Status of a playing channel*//**<CNcomment:播放通道状态。                                  */
    MT_UNF_PVR_PLAY_SPEED_E     enSpeed;                              /**<Playing speed of a playing channel*//**<CNcomment: 播放通道的播放速度。                            */
    MT_U64                      u64CurPlayPos;                        /**<Start position (byte) of the current frame in the file*//**<CNcomment:当前播放帧的起始在文件中的位置（byte数）。      */
    MT_U32                      u32CurPlayFrame;                      /**<Start position (number of frames or PESs) of the current frame in the file*//**<CNcomment:当前播放帧在文件中的位置（帧数或者PES数）       */
    MT_U32                      u32CurPlayTimeInMs;                   /**<PTS of the current frame, in ms*//**<CNcomment: 当前播放帧的PTS, 单位毫秒。0:invalid value*/
}MT_UNF_PVR_PLAY_STATUS_S;

/**<Data file attribution *//** CNcomment:数据文件属性*/
typedef struct mtUNF_PVR_DATA_ATTR_S
{
    MT_U32     u32ChnID;                              /**<PVR channel, just consider one channel*//**<CNcomment: PVR通道，现在可以只考虑一种通道的情况 */
    MT_U32     u32DmxID;                              /**<PVR dmx id*//**<CNcomment: PVR 通道 demux 号 */
    MT_CHAR    CurFileName[PVR_MAX_FILENAME_LEN];     /**<Current reading file 's name include direction.If there are multi-node, the file name maybe different*//**<CNcomment: 当前正在读取的文件名，包括绝对路径。多个节点情况下，该文件名有可能会变。  */
    MT_CHAR    IdxFileName[PVR_MAX_FILENAME_LEN+5];   /*name of index*//*<CNcomment:索引文件名*/
    MT_U64     u64FileStartPos;                       /**<Position of start frame ,it should be considered if rewind record ,0 invalid*//**<CNcomment:文件的开始位置，主要在文件环回的情况下需要考虑,为0时表示无效  */
    MT_U64     u64FileEndPos;                         /**<Position of end frame ,it should be considered if rewind record ,0 invalid*//**<CNcomment:文件的结束位置，主要在文件环回的情况下需要考虑,为0时表示无效   */
    MT_U64     u64GlobalOffset;                       /*global offset of recording file*//*<CNcomment:录制文件的全局偏移*/
    MT_U64     u64FileReadOffset;                     /*local offset of recording file*//*<CNcomment:录制文件的文件内偏移*/
    MT_UNF_PVR_CIPHER_S usercfg;                      /*infor of encrypting*//*CNcomment:加解密信息*/
    MT_U32     u32FirstPush;                          /*First Push*//**CNcomment:第一笔推送1:是，0:否*/
} MT_UNF_PVR_DATA_ATTR_S;

/**<Read or write data extra callback *//** CNcomment:外部读、写数据回调函数*/
typedef enum mtUNF_PVR_EXTRA_CALLBACK_E
{
    MT_UNF_PVR_EXTRA_READ_CALLBACK,          /**<Read data callback *//**<CNcomment:读数据回调函数*/
    MT_UNF_PVR_EXTRA_WRITE_CALLBACK,         /**<Write data callback*//**<CNcomment:写数据回调函数*/
    MT_UNF_PVR_EXTRA_CALLBACK_BUTT           /**<Invalid*//**<CNcomment: 无效的状态值        */
} MT_UNF_PVR_EXTRA_CALLBACK_E;

/**<Read or write data extra callback type *//** CNcomment:外部读、写数据回调函数类型*/
//typedef MT_S32 ( *ExtraCallBack)(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr, MT_U8 *pu8DataAddr, MT_U32 u32PhyAddr, MT_U32 u32Offset, MT_U32 u32DataSize);
typedef MT_S32 ( *ExtraCallBack)(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr, 
								MT_U8 *pu8DestDataVirAddr, ulong ulongDestDataPhyAddr, 
								MT_U8 *pu8SrcDataVirAddr,   ulong ulongSrcDataPhyAddr, 
								MT_U32 u32Offset, 
								MT_U32 *u32DataSize);


/** @} */  /** <!-- ==== Structure Definition end ==== */

/******************************* API declaration *****************************/
/** \addtogroup      PVR  */
/** @{ */  /** <!-- [PVR]*/

/***** Application programming interfaces (APIs) for the PVR recoding module*****/

/**
\brief Initializes the PVR recording module. CNcomment:初始化PVR录制模块 CNend
\attention \n
If you initialize the PVR recording module repeatedly, the error code MT_SUCCESS is returned.
Before using the APIs of the PVR recording module, you must call this API. Otherwise, other APIs are unavailable.
CNcomment: 重复初始化返回成功.
每个进程使用PVR录制模块接口前，必须先调用此接口，否则其他接口均无法使用 CNend
\param N/A CNcomment: 无 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_FAILURE This API fails to be called due to system errors. CNcomment: 出现系统错误，API调用失败 CNend
\retval ::MT_ERR_PVR_NOT_INIT  The PVR recording module is not initialized. CNcomment: 录制模块未初始化 CNend
\see \n
::MT_UNF_PVR_RecDeInit
*/
MT_S32 MT_UNF_PVR_RecInit(MT_VOID);

/**
\brief Deinitializes the PVR recording module. CNcomment: 去初始化PVR录制模块 CNend
\attention \n
If you deinitialize the PVR recording module repeatedly, the error code MT_SUCCESS is returned.\n
Before deintializing the PVR recording module, you must release all the recording channels. Otherwise,
the error code MT_ERR_PVR_BUSY is returned.
CNcomment:重复去初始化返回成功\n
去初始化PVR录制模块前务必保证本进程内所有录制通道都已经释放，如果还有录制通道没有释放，此接口将会返回错误码MT_ERR_PVR_BUSY CNend
\param N/A CNcomment:无 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_PVR_BUSY  The PVR recording module is being used. CNcomment: 录制模块还在使用中 CNend
\see \n
::MT_UNF_PVR_RecInit
*/
MT_S32 MT_UNF_PVR_RecDeInit(MT_VOID);

/**
\brief Creates a recording channel. CNcomment: 创建1路录制通道 CNend
\attention \n
The PVR recording module automatically generates an .idx index file based on the file name specified in pAttr. \n
The PVR recording module splits the recorded streams files to ensure that the size of each file is not greater than 4 GB (the size of each file is about 3.99 GB). \n
The names of streams files are szFileName, szFileName.0001, szFileName.0002, szFileName.0003, szFileName.0004.\n
If the file specified in pAttr already exists, it is replaced.\n
If the index file to be created by the PVR recording module already exists, the file is also replaced.\n
If the PVR calls the APIs of other modules (such as the APIs of the audio module, video module, and DEMUX module), the error codes of other modules may be returned.\n
the error codes of other modules may be returned. For details, see the definitions of the error codes of corresponding modules.This description is applicable to the following APIs.
This description is applicable to the following APIs.
CNcomment:PVR录制模块会根据pAttr中指定的文件名自动生成一个后缀名为".idx"索引文件\n
PVR录制模块会对录制的码流文件进行分割，以确保码流文件不超过4G的大小（每个文件的大小约为3.99G），
生成的码流文件依次为szFileName，szFileName.0001，szFileName.0002，szFileName.0003，szFileName.0004···\n
如果pAttr中指定的文件已经存在，那么这个文件会被覆盖\n
如果PVR录制模块将要创建的索引文件已经存在，则索引文件也会被覆盖\n
由于PVR使用到了其他模块的API接口函数（比如音视频API接口和DEMUX API接口），因此返回的错误码有可能是其他模块的错误码，
具体错误码值请参考相应模块错误码值定义。后面的其他API函数均会有此种情况  CNend
\param[in] pu32ChnID   ID of the obtained recording channel  CNcomment: 获取到的录制通道ID CNend
\param[in] pstRecAttr  Pointer to the attributes of a channel   CNcomment:指针类型，指向通道属性配置 CNend
\retval ::MT_SUCCESS  Success CNcomment:成功 CNend
\retval ::MT_ERR_PVR_REC_INVALID_RECID. recid is invalid. CNcomment:无效的录制id CNend
\retval ::MT_ERR_PVR_NO_MEM  There is no memory.   CNcomment:内存不足 CNend
\retval ::MT_ERR_PVR_NO_CHN_LEFT  There is no available channel.   CNcomment:已经没有空闲的通道供分配 CNend
\retval ::MT_ERR_PVR_FILE_CANT_OPEN  The file cannot be opened.  CNcomment: 无法打开文件 CNend
\retval ::MT_ERR_PVR_INDEX_CANT_MKIDX  The index file cannot be opened.  CNcomment: 无法打开索引文件 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_RecCreateChn(MT_U32 *pu32ChnID, const MT_UNF_PVR_REC_ATTR_S *pstRecAttr);
/**
\brief Copy a recording channel. CNcomment: 复制并创建1路录制通道,不需要再addpid CNend
\attention \n
The PVR recording module automatically generates an .idx index file based on the file name specified in pAttr. \n
The PVR recording module splits the recorded streams files to ensure that the size of each file is not greater than 4 GB (the size of each file is about 3.99 GB). \n
The names of streams files are szFileName, szFileName.0001, szFileName.0002, szFileName.0003, szFileName.0004.\n
If the file specified in pAttr already exists, it is replaced.\n
If the index file to be created by the PVR recording module already exists, the file is also replaced.\n
If the PVR calls the APIs of other modules (such as the APIs of the audio module, video module, and DEMUX module), the error codes of other modules may be returned.\n
the error codes of other modules may be returned. For details, see the definitions of the error codes of corresponding modules.This description is applicable to the following APIs.
This description is applicable to the following APIs.
CNcomment:PVR录制模块会根据pAttr中指定的文件名自动生成一个后缀名为".idx"索引文件\n
PVR录制模块会对录制的码流文件进行分割，以确保码流文件不超过4G的大小（每个文件的大小约为3.99G），
生成的码流文件依次为szFileName，szFileName.0001，szFileName.0002，szFileName.0003，szFileName.0004···\n
如果pAttr中指定的文件已经存在，那么这个文件会被覆盖\n
如果PVR录制模块将要创建的索引文件已经存在，则索引文件也会被覆盖\n
由于PVR使用到了其他模块的API接口函数（比如音视频API接口和DEMUX API接口），因此返回的错误码有可能是其他模块的错误码，
具体错误码值请参考相应模块错误码值定义。后面的其他API函数均会有此种情况  CNend
\param[in] pu32ChnID   ID of the obtained recording channel  CNcomment: 获取到的录制通道ID CNend
\param[in] u32_SRC_ChnID  source channel   CNcomment:录制通道源 CNend
\param[in] pstRecAttr  Pointer to the attributes of a channel   CNcomment:指针类型，指向通道属性配置 CNend
\retval ::MT_SUCCESS  Success CNcomment:成功 CNend
\retval ::MT_ERR_PVR_REC_INVALID_RECID. recid is invalid. CNcomment:无效的录制id CNend
\retval ::MT_ERR_PVR_NO_MEM  There is no memory.   CNcomment:内存不足 CNend
\retval ::MT_ERR_PVR_NO_CHN_LEFT  There is no available channel.   CNcomment:已经没有空闲的通道供分配 CNend
\retval ::MT_ERR_PVR_FILE_CANT_OPEN  The file cannot be opened.  CNcomment: 无法打开文件 CNend
\retval ::MT_ERR_PVR_INDEX_CANT_MKIDX  The index file cannot be opened.  CNcomment: 无法打开索引文件 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_RecCopyChn(MT_U32 *pu32ChnID, MT_U32 u32_SRC_ChnID, const MT_UNF_PVR_REC_ATTR_S *pstRecAttr);

/**
\brief  Releases a specified recording channel. CNcomment: 释放指定的录制通道 CNend
\attention \n
CNcomment:If the channel to be released is not stopped, the error code MT_ERR_PVR_BUSY is returned.
如果待释放的通道没有停止，将返回错误码::MT_ERR_PVR_BUSY CNend
\param[in] u32ChnID   ID of the channel to be released CNcomment: 需要释放的通道号 CNend
\retval ::MT_SUCCESS Success CNcomment: 成功 CNend
\retval ::MT_ERR_PVR_BUSY  The current operation is forbidden because the channel is busy. CNcomment: 通道忙，不允许进行当前操作 CNend
\retval ::MT_ERR_PVR_NOT_INIT  The PVR recording module is not initialized. CNcomment: PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID  The channel ID is invalid.  CNcomment: 通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT   The channel ID is not initialized. CNcomment: 通道号未初始化 CNend
\retval ::MT_ERR_PVR_FREEMEM_ERR    Free memory failure. CNcomment: 释放内存失败 CNend
\retval ::MT_ERR_PVR_REC_FAIL_DESTROY  fail to destory recchan. CNcomment: 释放录制通道失败 CNend
\see \n 
N/A
*/
MT_S32 MT_UNF_PVR_RecDestroyChn(MT_U32 u32ChnID);

/**
\brief Sets the attributes of a recording channel. CNcomment: 设置录制通道属性 CNend
\attention \n
You can modify only the dynamic attributes by calling this API. 
CNcomment:只有动态属性才能用此接口改变设置，由于目前属性都是静态的，所以此接口暂时不支持功能设置 CNend
\param[in] u32ChnID   Channel ID CNcomment: 通道号 CNend
\param[in] pstRecAttr  Attribute to be set  CNcomment:待设置的属性 CNend
\retval ::MT_SUCCESS Success CNcomment: 成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT  The PVR recording module is not initialized. CNcomment:PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID  The channel ID is invalid.  CNcomment: 通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT   The channel ID is not initialized. CNcomment: 通道号未初始化 CNend
\retval ::MT_ERR_PVR_NUL_PTR        The pointer is null.  CNcomment:指针参数为空 CNend
\retval ::MT_ERR_PVR_NOT_SUPPORT  The function is not supported. CNcomment:功能不支持 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_RecSetChn(MT_U32 u32ChnID, const MT_UNF_PVR_REC_ATTR_S *pstRecAttr);  /*invalid*//*功能无效*/

/**
\brief Obtains the attributes of a recording channel. CNcomment: 获取录制通道属性 CNend
\attention \n
N/A 
\param[in] u32ChnID   Channel ID CNcomment: 通道号 CNend
\param[out] pstRecAttr   Pointer to the obtained channel attributes   CNcomment:指针类型，指向获取到的通道属性配置 CNend
\retval ::MT_SUCCESS  Success  CNcomment:成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT    The PVR recording module is not initialized.  CNcomment:PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID  The channel ID is invalid.  CNcomment: 通道号非法 CNend
\retval ::MT_ERR_PVR_NUL_PTR        The pointer is null.  CNcomment:指针参数为空 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT  The specified channel is not initialized.   CNcomment:指定的通道未初始化 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_RecGetChn(MT_U32 u32ChnID, MT_UNF_PVR_REC_ATTR_S *pstRecAttr);

/**
\brief Starts a recording channel. CNcomment:启动录制通道 CNend
\attention \n
N/A
\param[in] u32ChnID Channel ID   CNcomment:通道号 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT   The PVR recording module is not initialized. CNcomment:PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID    The channel ID is invalid.  CNcomment:通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT   The specified channel is not initialized.  CNcomment: 指定的通道未初始化 CNend
\retval ::MT_ERR_PVR_ALREADY       The specified channel has been started.  CNcomment: 指定的通道已经启动 CNend
\retval ::MT_ERR_PVR_CREAT_THREAD_ERR  fails to create thread.  CNcomment:创建线程 CNend
\retval ::MT_ERR_PVR_REC_FAIL_START_DEMUX  fail to start demux.  CNcomment:启动 demux 失败 CNend
\see \n
*/
MT_S32 MT_UNF_PVR_RecStartChn(MT_U32 u32ChnID);

/**
\brief Stops a recording channel. CNcomment: 停止录制通道 CNend
\attention \n
If a recording channel is working in time-shift playing mode, the channel cannot be stopped until the time-shift operation stops.
In this case, the error code MT_ERR_PVR_BUSY is returned if you call this API.
CNcomment:如果1个录制通道正在进行时移播放操作，则在停止时移播放之前，不允许停止该录制通道 \n
如果该通道正在进行时移播放操作，将返回错误码MT_ERR_PVR_BUSY CNend
\param[in] u32ChnID   Channel ID  CNcomment:通道号 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT    The PVR recording module is not initialized.  CNcomment: PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID     The channel ID is invalid.    CNcomment:通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT     The specified channel is not initialized. CNcomment:指定的通道未初始化 CNend
\retval ::MT_ERR_PVR_REC_FAIL_STOP_DEMUX  fail to stop demux.  CNcomment:停止 demux 失败 CNend
\see \n
::MT_UNF_PVR_RecStartChn
*/
MT_S32 MT_UNF_PVR_RecStopChn(MT_U32 u32ChnID);

/**
\brief Pauses a recording channel. CNcomment: 暂停录制通道 CNend
\attention \n
It should be called under recording mode and can be repeated call.
CNcomment:只有录制状态调用该接口才有作用，允许重复调用 CNend
\param[in] u32ChnID   Channel ID  CNcomment:通道号 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_PVR_REC_INVALID_STATE  PVR isn't playing or pausing. CNcomment: PVR非播放或者暂停状态 CNend
\retval ::MT_ERR_PVR_NOT_INIT    The PVR recording module is not initialized.  CNcomment: PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID     The channel ID is invalid.    CNcomment:通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT     The specified channel is not initialized. CNcomment:指定的通道未初始化 CNend
\retval ::MT_ERR_PVR_REC_FAIL_STOP_DEMUX  fail to stop demux.  CNcomment:停止 demux 失败 CNend
\see \n
::MT_UNF_PVR_RecResumeChn
*/
MT_S32 MT_UNF_PVR_RecPauseChn(MT_U32 u32ChnID);  /*invalid*//*功能无效*/

/**
\brief Resumes a recording channel. CNcomment: 恢复录制通道 CNend
\attention \n
It should be called under recording mode and can be repeated call.
CNcomment:只有录制状态调用该接口才有作用，允许重复调用 CNend
\param[in] u32ChnID   Channel ID  CNcomment:通道号 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_PVR_REC_INVALID_STATE  PVR isn't playing or paused.CNcomment: PVR非播放或者暂停状态 CNend
\retval ::MT_ERR_PVR_NOT_INIT    The PVR recording module is not initialized.  CNcomment: PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID     The channel ID is invalid.    CNcomment:通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT     The specified channel is not initialized. CNcomment:指定的通道未初始化 CNend
\retval ::MT_ERR_PVR_REC_FAIL_STOP_DEMUX  fail to stop demux.  CNcomment:停止 demux 失败 CNend
\see \n
::MT_UNF_PVR_RecPauseChn
*/
MT_S32 MT_UNF_PVR_RecResumeChn(MT_U32 u32ChnID);  /*invalid*//*功能无效*/

/**
\brief Obtains the status of a recording channel.CNcomment:  获取录制通道状态 CNend
\attention \n
N/A  
\param[in] u32ChnID  Channel ID  CNcomment:通道号 CNend
\param[out] pstRecStatus   Pointer to the recording status   CNcomment:录制状态信息指针 CNend
\retval ::MT_SUCCESS Success  CNcomment: 成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT    The PVR recording module is not initialized.  CNcomment:PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID   The channel ID is invalid.   CNcomment: 通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT    The specified channel is not initialized.  CNcomment:指定的通道未初始化 CNend
\retval ::MT_ERR_PVR_NUL_PTR        The pointer is null. CNcomment:指针参数为空 CNend
\retval ::MT_ERR_PVR_FILE_CANT_OPEN  can't open idx file. CNcomment:不能打开index文件 CNend
\retval ::MT_ERR_PVR_FILE_CANT_READ  can't read idx file. CNcomment:不能读index文件 CNend
\retval ::MT_ERR_PVR_INDEX_FORMAT_ERR format is error. CNcomment:格式不正确 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_RecGetStatus(MT_U32 u32ChnID, MT_UNF_PVR_REC_STATUS_S *pstRecStatus);

/**
\brief Set pid.CNcomment:  设置pid CNend
\attention \n
N/A  
\param[in] u32ChnID  Channel ID  CNcomment:通道号 CNend
\param[in] u32DmxId  Demux ID  CNcomment:demux号 CNend
\param[in] pid  Pid  CNcomment:音视频号 CNend
\retval ::MT_SUCCESS Success  CNcomment: 成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT    The PVR recording module is not initialized.  CNcomment:PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID   The channel ID is invalid.   CNcomment: 通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT    The specified channel is not initialized.  CNcomment:指定的通道未初始化 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_RecSetPid(mt_u32 chnId, mt_u32 u32DmxId, int pid);
    
/**
\brief Get pid.CNcomment:  获取录制pid CNend
\attention \n
N/A  
\param[in] u32ChnID  Channel ID  CNcomment:通道号 CNend
\param[out] pids  array[128]  CNcomment:pid 数据组，调用者分配空间，最好是128个 CNend
\param[out] *num  number of pids  CNcomment:pid 个数 CNend
\retval ::MT_SUCCESS Success  CNcomment: 成功 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_RecGetPids(mt_u32 chnId, int *pids,int *num);

/**
\brief add or del recording pid.CNcomment:  动态增减录制pid CNend
\attention \n
N/A  
\param[in] u32ChnID  Channel ID  CNcomment:通道号 CNend
\param[in] u32DmxId  demuxid   CNcomment:录制demux号 CNend
\param[in] pid      programme id   CNcomment:节目号 CNend
\param[in] flag_adddel 1:add or 0:del    CNcomment:1增，0减 CNend
\param[in] flag_index  1:index pid 0:not   CNcomment:1:主索引pid，0:普通pid CNend
\retval ::MT_SUCCESS Success  CNcomment: 成功 CNend
\retval ::MT_FAILURE failure  CNcomment:操作失败 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_AddDelPid_Ex(mt_u32 chnId, mt_u32 u32DmxId, int pid,MT_BOOL flag_adddel,MT_BOOL flag_index);
    

/***** APIs for the PVR playing module *****/
/**
\brief Initializes the PVR playing module. CNcomment: 初始化PVR播放模块 CNend
\attention \n
If you initialize the PVR playing module repeatedly, the error code MT_SUCCESS is returned.\n
Before using the APIs of the PVR playing module, you must call this API. Otherwise, other APIs are unavailable.
CNcomment:重复初始化返回成功 \n
每个进程使用PVR播放模块接口前，必须先调用此接口，否则其他接口均无法使用 CNend
\param N/A
\retval ::MT_SUCCESS         Success CNcomment:成功 CNend
\retval ::MT_ERR_PVR_FILE_CANT_OPEN  can't open file.    CNcomment:打开文件失败 CNend
\retval ::MT_ERR_PVR_CREAT_MUTEX_ERR  can't create mutex.    CNcomment:创建互斥锁失败 CNend
\see \n
::MT_UNF_PVR_PlayDeInit
*/
MT_S32 MT_UNF_PVR_PlayInit(MT_VOID);

/**
\brief Deinitializes the PVR playing module.  CNcomment:去初始化PVR播放模块 CNend
\attention \n
If you deinitialize the PVR recording module repeatedly, the error code MT_SUCCESS is returned.\n
Before deintializing the PVR playing module, you must release all the playing channels. Otherwise, the error code MT_ERR_PVR_BUSY is returned.
CNcomment:重复去初始化返回成功 \n
去初始化PVR播放模块前务必保证本进程内所有播放通道都已经释放，如果还有播放通道没有释放，此接口将会返回错误码::MT_ERR_PVR_BUSY CNend
\param N/A 
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_PVR_BUSY  The PVR playing module is being used.  CNcomment: PVR播放模块还在使用中 CNend
\see \n
::MT_UNF_PVR_PlayInit
*/
MT_S32 MT_UNF_PVR_PlayDeInit(MT_VOID);

/**
\brief Creates a playing channel. CNcomment:创建1路播放通道 CNend
\attention \n
The PVR playing module automatically searches for an .idx index file based on the file name specified in pAttr.
If the index file does not exist, the PVR playing module attempts to play the stream file, but cannot support some features,
such as fast forward, fast backward, or seek. The features vary according to versions of the software developer's kit (SDK).
Only the playing function is supported in the current version.
The played file may be the file that is being recorded (that is, time-shift playing).
The audio/video player (AVPLAY) indicated by hAVPLAY must be stopped.
CNcomment:PVR播放模块会根据pAttr中指定的文件名自动寻找后缀名为".idx"的索引文件 \n
如果索引文件不存在，PVR播放模块也会尝试播放这个文件，但是可能无法支持某些特性，
比如快进快退或者SEEK操作（根据版本的不同而有所差别，当前版本只支持播放）\n
播放的文件可以是正在录制的文件（即进行时移播放）
hAVPlay所代表的avplay必须处于stop状态。 CNend
\param[out] pu32ChnID  ID of the obtained playing channel  CNcomment: 获取到的播放通道ID CNend
\param[in] pstPlayAttr  Pointer to the channel attributes CNcomment: 通道属性配置指针 CNend
\param[in] hAvplay      AVPLAY handle. It specifies the APVPLAY to be used. CNcomment: avplay句柄，标识用哪个avplay来播放 CNend
\param[in] hTsBuffer  TS buffer handle. It specifies that TS buffer that is used for playback. In general, the TS port corresponding to the TS buffer must be attached to the DEMUX corresponding to the AVPLAY.
tCNcomment:s buffer句柄，标识用哪个ts buffer来回放，一般来说，此ts buffer对应的ts端口应该是绑定在hAVPlay对应的demux上的 CNend
\retval ::MT_SUCCESS   Success  CNcomment:成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT       The PVR recording module is not initialized.  CNcomment: PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_NUL_PTR            The pointer is null. CNcomment:  指针参数为空 CNend
\retval ::MT_ERR_DMX_INVALID_PARA        The parameter is invalid.  CNcomment: 参数非法 CNend
\retval ::MT_ERR_PVR_FILE_INVALID_FNAME   The file name is invalid.   CNcomment:文件名参数非法 CNend
\retval ::MT_ERR_PVR_FILE_EXIST          The file already exists.  CNcomment:文件已经存在 CNend
\retval ::MT_ERR_PVR_NO_CHN_LEFT        There is no available channel. CNcomment:  已经没有空闲的通道供分配 CNend
\retval ::MT_ERR_PVR_FILE_CANT_OPEN      The file cannot be opened.   CNcomment:  无法打开文件 CNend
\retval ::MT_ERR_PVR_FILE_NOT_EXIST      The file cannot be played because it does not exist. CNcomment: 文件不存在，无法播放 CNend
\retval ::MT_ERR_PVR_NO_MEM             The buffer required for playing cannot be allocated due to insufficient memory. CNcomment: 内存不足，无法分配播放需要的Buffer CNend
\retval ::MT_ERR_PVR_FILE_CANT_READ   The file cannot be read.  CNcomment:   文件读取出错 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_PlayCreateChn(MT_U32 *pu32ChnID, const MT_UNF_PVR_PLAY_ATTR_S *pstPlayAttr, MT_HANDLE hAvplay, MT_HANDLE hTsBuffer);

/**
\brief Releases a specified playing channel. CNcomment:释放指定的播放通道 CNend
\attention \n
If the channel to be released is not stopped, the error code MT_ERR_PVR_BUSY is returned.
CNcomment:如果待释放的通道没有停止，将返回错误码::MT_ERR_PVR_BUSY CNend
\param[in] u32ChnID  ID of the channel to be released CNcomment:需要释放的通道号 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT     The PVR recording module is not initialized.   CNcomment: PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID       The channel ID is invalid.  CNcomment: 通道号非法 CNend
\retval ::MT_ERR_PVR_PLAY_INVALID_STATE  running or no initial.  CNcomment: 处于不能销毁状态 CNend
\retval ::MT_ERR_PVR_FREEMEM_ERR  fail to free mem.  CNcomment: 释放内存失败 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_PlayDestroyChn(MT_U32 u32ChnID);

/**
\brief
Starts a time-shift playing channel.
You can call this API to start a time-shift playing channel based on the entered IDs of recording channels. To be specific, this API automatically initializes a playing channel to play files based on the parameter configuration of the corresponding recording channel.
CNcomment:启动时移播放通道。调用此接口可以根据输入的录制通道号，启动1路时移播放。此接口将根据对应录制通道的参数自动初始化1个播放通道，并开始播放 CNend
\attention \n
This API is not recommended. It is used to be compatible with other interfaces. You are recommended to call MT_UNF_PVR_PlayCreateChn and MT_UNF_PVR_PlayStartChn in sequence to start to play. If you want to enable the time-shift playing mode,
you only need to set same names for the file to be played and recorded file. Then the PVR automatically identifies the same names, and enters the time-shift playing mode.
After MT_UNF_PVR_PlayStartTimeShift is called, MT_UNF_PVR_PlayCreateChn is automatically called to allocate a playing channel. Therefore, you do not need to call MT_UNF_PVR_PlayNewChn.\n
In addition, before calling MT_UNF_PVR_PlayStartTimeShift, you must start the corresponding recording channel. Otherwise, the error code MT_ERR_PVR_REC_INVAL_STATE is returned.\n
After you start a time-shift playing channel by calling this API, the corresponding recording channel cannot be stopped or destroyed until time-shift playing stops.
CNcomment:不推荐使用此接口，保留此接口只是为了保持与老接口的兼容，推荐使用MT_UNF_PVR_PlayCreateChn后再调用MT_UNF_PVR_PlayStartChn启动播放，如果要进行时移，
只需要设置播放的文件名与录制的文件名一样即可，PVR会自动识别相同的文件名并进入时移状态。
调用此接口可以直接启动1路时移播放，接口内部会自动调用MT_UNF_PVR_PlayCreateChn分配播放通道，用户不需要再先调用MT_UNF_PVR_PlayCreateChn创建播放通道 \n
调用此接口前请确保对应的录制通道已经启动，否则将会返回错误码MT_ERR_PVR_REC_INVAL_STATE \n
调用此接口启动1路时移后，对应的录制通道将不能被停止或者销毁直到时移播放停止 CNend
\param[out] pu32PlayChnID   ID of a playing channel CNcomment:播放通道号 CNend
\param[in] u32RecChnID    ID of the recording channel to be time-shifted CNcomment: 需要时移的录制通道号 CNend
\param[in] hAvplay     AVPLAY handle. It specifies the APVPLAY to be used.   CNcomment:avplay句柄，标识用哪个avplay来播放 CNend
\param[in] hTsBuffer    TS buffer handle. It specifies that TS buffer that is used for playback. In general, the TS port corresponding to the TS buffer must be attached to the DEMUX corresponding to the AVPLAY.
CNcomment:ts buffer句柄，标识用哪个ts buffer来回放，一般来说，此ts buffer对应的ts端口应该是绑定在hAVPlay对应的demux上的 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_PVR_NUL_PTR The pointer is null.  CNcomment:指针参数为空 CNend
\retval ::MT_ERR_PVR_PLAY_INVALID_DMXID The DEMUX ID received during playing is incorrect. CNcomment:播放时传入了错误的DEMUX ID CNend
\retval ::MT_ERR_PVR_INVALID_CHNID The channel ID is invalid. CNcomment: 通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT The specified channel is not initialized. CNcomment: 指定的通道未初始化 CNend
\retval ::MT_ERR_PVR_ALREADY  The specified channel has been started. CNcomment:指定的通道已经启动 CNend
\retval ::MT_ERR_PVR_NO_CHN_LEFT  There is no available playing channel.CNcomment: 已经没有空闲的播放通道供分配 CNend
\retval ::MT_ERR_PVR_REC_INVALID_STATE  The recording channel is not started.CNcomment:  录制通道没有启动 CNend
\retval ::MT_ERR_PVR_NO_MEM   The buffer required for playing cannot be allocated due to insufficient memory.CNcomment: 内存不足，无法分配播放需要的Buffer CNend
\retval ::MT_FAILURE This API fails to be called due to system errors. CNcomment:出现系统错误，API调用失败 CNend
\see \n
::MT_UNF_PVR_PlayStopTimeShift
*/
MT_S32 MT_UNF_PVR_PlayStartTimeShift(MT_U32 *pu32PlayChnID, MT_U32 u32RecChnID, MT_HANDLE hAvplay, MT_HANDLE hTsBuffer);

/**
\brief Stops a playing channel.  CNcomment:停止播放通道 CNend
\attention \n
This API must work with ::MT_UNF_PVR_PlayStartTimeShift, and this API can be only used to stop the playing channel that is started by calling ::MT_UNF_PVR_PlayStartTimeShift.
You must call ::MT_UNF_PVR_PlayStopTimeShift rather than ::MT_UNF_PVR_PlayStopChn to stop the playing channel that is started by calling ::MT_UNF_PVR_PlayStartTimeShift.

CNcomment:此接口与::MT_UNF_PVR_PlayStartTimeShift接口必须配对使用，此接口只能停止使用::MT_UNF_PVR_PlayStartTimeShift启动的播放通道 \n
通过::MT_UNF_PVR_PlayStartTimeShift接口启动的播放通道必须使用此接口来停止，而不允许用::MT_UNF_PVR_PlayStopChn接口来停止 CNend
\param[in] u32PlayChnID Channel ID  CNcomment:通道号 CNend
\param[in] pstStopOpt  Pointer to the attribute structure when the AVPLAY stops CNcomment:停止的属性设置 CNend
\retval ::MT_SUCCESS  SuccessCNcomment: 成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT        The PVR recording module is not initialized. CNcomment: PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID        The channel ID is invalid. CNcomment:  通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT       The specified channel is not initialized.   CNcomment: 指定的通道未初始化 CNend
\retval ::MT_ERR_PVR_ALREADY            The specified channel has been started.  CNcomment: 指定的通道已经启动 CNend
\retval ::MT_ERR_PVR_PLAY_INVALID_STATE    The channel status is incorrect. For example, you call this API to stop the playing channel that is started by calling MT_UNF_PVR_PlayStartChn.
CNcomment:通道状态错误。例如：对用::MT_UNF_PVR_PlayStartChn启动的播放通道调用此接口 CNend
\see \n
::MT_UNF_PVR_PlayStartTimeShift
*/
MT_S32 MT_UNF_PVR_PlayStopTimeShift(MT_U32 u32PlayChnID, const MT_UNF_AVPLAY_STOP_OPT_S *pstStopOpt);

/**
\brief Sets the attributes of a playing channel. CNcomment:设置播放通道属性 CNend
\attention \n
You can set the dynamic attributes only by calling this API.
CNcomment:只有动态属性才能通过此接口进行设置，由于目前都是动态属性，所以此接口暂时不提供设置功能。 CNend
\param[in] u32ChnID    Channel ID CNcomment: 通道号 CNend
\param[in] pstPlayAttr  Pointer to the channel attributes to be set  CNcomment:指针类型，指向要设置的通道属性配置 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT  The PVR recording module is not initialized. CNcomment:PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID The channel ID is invalid.  CNcomment:通道号非法 CNend
\retval ::MT_ERR_PVR_NOT_SUPPORT  The function is not supported. CNcomment:功能不支持 CNend
\see \n
::MT_UNF_PVR_PlayGetChn
*/
MT_S32 MT_UNF_PVR_PlaySetChn(MT_U32 u32ChnID, const MT_UNF_PVR_PLAY_ATTR_S *pstPlayAttr);  /*invalid*//*功能无效*/

/**
\brief  Obtains the attributes of a playing channel. CNcomment:获取播放通道属性 CNend
\attention \n
N/A
\param[in] u32ChnID   Channel ID CNcomment:CNcomment:通道号 CNend
\param[out] pstPlayAttr   Pointer to the obtained channel attributes CNcomment:指针类型，指向获取到的通道属性配置 CNend
\retval ::MT_SUCCESS  Success CNcomment:成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT   The PVR recording module is not initialized.CNcomment: PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID   The channel ID is invalid. CNcomment:  通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT   The specified channel is not initialized. CNcomment: 指定的通道未初始化 CNend
\retval ::MT_ERR_PVR_NUL_PTR      The pointer is null.  CNcomment:  指针参数为空 CNend
\see \n
::MT_UNF_PVR_PlaySetChn
*/
MT_S32 MT_UNF_PVR_PlayGetChn(MT_U32 u32ChnID, MT_UNF_PVR_PLAY_ATTR_S *pstPlayAttr);

/**
\brief  Starts a playing channel. CNcomment:启动播放通道 CNend
\attention \n
You can call this API to start the channel that is created by calling ::MT_UNF_PVR_PlayCreateChn only once. To pause or resume playing,
you need to call ::MT_UNF_PVR_PlayPauseChn and ::MT_UNF_PVR_PlayResumeChn respectively.
CNcomment:调用::MT_UNF_PVR_PlayCreateChn申请的通道只能通过此接口启动一次，如果想要暂停/恢复播放，
需使用::MT_UNF_PVR_PlayPauseChn和::MT_UNF_PVR_PlayResumeChn接口 CNend
\param[in] u32ChnID Channel ID  CNcomment:通道号 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT     The PVR recording module is not initialized.  CNcomment:PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID     The channel ID is invalid.  CNcomment: 通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT    The specified channel is not initialized. CNcomment:  指定的通道未初始化 CNend
\retval ::MT_ERR_PVR_ALREADY        The specified channel has been started.   CNcomment: 指定的通道已经启动 CNend
\retval ::MT_FAILURE                 This API fails to be called due to system errors. CNcomment: 出现系统错误，API调用失败 CNend
\see \n
::MT_UNF_PVR_PlayStopChn
*/
MT_S32 MT_UNF_PVR_PlayStartChn(MT_U32 u32ChnID);

/**
\brief  Stops a playing channel. CNcomment:停止播放通道 CNend
\attention \n
This API must work with ::MT_UNF_PVR_PlayStartChn, and this API can be only used to stop the playing channel that is started by calling MT_UNF_PVR_PlayStartChn.
You must call ::MT_UNF_PVR_PlayStopChn rather than MT_UNF_PVR_PlayStopTimeShift to stop the playing channel that is started by calling MT_UNF_PVR_PlayStartChn.
CNcomment:此接口与::MT_UNF_PVR_PlayStartChn接口必须配对使用，此接口只能停止使用::MT_UNF_PVR_PlayStartChn启动的播放通道 \n
通过::MT_UNF_PVR_PlayStartChn接口启动的播放通道必须使用此接口来停止，而不允许用::MT_UNF_PVR_PlayStopTimeShift接口来停止 CNend
\param[in] u32ChnID  Channel ID CNcomment:通道号 CNend
\param[in] pstStopOpt  Pointer to the attribute structure when the AVPLAY stops  CNcomment:停止的属性设置 CNend
\retval ::MT_SUCCESS  Success CNcomment:成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT    The PVR recording module is not initialized. CNcomment: PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID     The channel ID is invalid.  CNcomment: 通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT    The specified channel is not initialized.  CNcomment: 指定的通道未初始化 CNend
\retval ::MT_ERR_PVR_ALREADY        The specified channel has been started.  CNcomment:  指定的通道已经启动 CNend
\retval ::MT_ERR_PVR_PLAY_INVALID_STATE The channel status is incorrect. For example, you call this API to stop the playing channel that is started by calling MT_UNF_PVR_PlayStartTimeShift.
CNcomment:通道状态错误。例如：对用::MT_UNF_PVR_PlayStartTimeShift启动的播放通道调用此接口 CNend
\see \n
::MT_UNF_PVR_PlayStartChn
*/
MT_S32 MT_UNF_PVR_PlayStopChn(MT_U32 u32ChnID, const MT_UNF_AVPLAY_STOP_OPT_S *pstStopOpt);

/**
\brief  Pauses a playing channel. CNcomment:暂停播放通道 CNend
\attention \n
If you call this API when the channel is paused, the error code MT_SUCCESS is returned.\n
If you call this API is called when the channel is stopped, the error code MT_ERR_PVR_PLAY_INVALID_STATE is returned.
CNcomment:在通道已经暂停的情况下再次调用::MT_UNF_PVR_PlayPauseChn暂停通道将返回成功 \n
在通道已经被停止的情况下调用此接口暂停通道将返回错误码::MT_ERR_PVR_PLAY_INVALID_STATE CNend
\param[in] u32ChnID  Channel ID  CNcomment:通道号 CNend
\retval ::MT_SUCCESS  Success CNcomment:成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT      The PVR recording module is not initialized. CNcomment:  PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID     The channel ID is invalid.  CNcomment:  通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT       The specified channel is not initialized.  CNcomment:指定的通道未初始化 CNend
\retval ::MT_ERR_PVR_PLAY_INVALID_STATE   The channel status is incorrect, that is , the channel has been stopped. CNcomment: 通道状态错误：通道已经停止运行 CNend
\see \n
::MT_UNF_PVR_PlayResumeChn
*/
MT_S32 MT_UNF_PVR_PlayPauseChn(MT_U32 u32ChnID);

/**
\brief Resumes a playing channel. You can resume a playing channel or enable a playing channel to exit the pause mode by calling this API.
CNcomment:恢复播放通道。调用此接口可以恢复播放通道，解除播放通道的暂停状态 CNend
\attention \n
This API must work with MT_UNF_PVR_PlayPauseChn. In addition, a channel can be resumed only when it is paused.
CNcomment:此接口与::MT_UNF_PVR_PlayPauseChn接口必须配对使用，只有在通道已经暂停的情况下才允许恢复通道 CNend
\param[in] u32ChnID Channel ID  CNcomment:通道号 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT       The PVR recording module is not initialized.  CNcomment:PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID      The channel ID is invalid.   CNcomment:通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT     The specified channel is not initialized.   CNcomment:指定的通道未初始化 CNend
\retval ::MT_ERR_PVR_PLAY_INVALID_STATE The specified channel is not paused.   CNcomment:指定的通道不在暂停状态 CNend
\see \n
::MT_UNF_PVR_PlayPauseChn
*/
MT_S32 MT_UNF_PVR_PlayResumeChn(MT_U32 u32ChnID);

/**
\brief Pause or resume a trick playing channel.
CNcomment:调用此接口可以暂停或者恢复快进快退的播放通道 CNend
\attention \n
This API must work with MT_UNF_PVR_PlayTPlay.
CNcomment:此接口与::MT_UNF_PVR_PlayTPlay接口必须配合使用 CNend
\param[in] u32ChnID Channel ID	CNcomment: 通道号 CNend
\param[in] bPause   TRUE:Pause，FALSE:resume	CNcomment: 暂停或者恢复 CNend
\retval ::MT_SUCCESS Success	CNcomment:成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT       The PVR recording module is not initialized.  CNcomment:PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID      The channel ID is invalid.   CNcomment:通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT     The specified channel is not initialized.   CNcomment:指定的通道未初始化 CNend
\retval ::MT_ERR_PVR_PLAY_INVALID_STATE The specified channel is not paused.   CNcomment:指定的通道不在暂停状态 CNend
*/
MT_S32 MT_UNF_PVR_TPlayPauseResume(MT_U32 u32ChnID, MT_BOOL bPause);


/**
\brief Starts the trick playing mode. You can call this API to start the trick playing mode. In this mode, the operations include fast forward, fast backward, slow playing, and normal playing.
CNcomment:进行特技播放。调用此接口可以进行特技播放，即进行快进、快退、慢放、正常播放等操作 CNend
\attention \n
If you want to switch the trick playing mode to the normal playing mode, you only need to the playing speed in trick playing mode to PVR_PLAY_SPEED_NORMAL.
CNcomment:进入特技播放模式后，如果想退回正常播放模式，只需要设置特技播放模式的速度为PVR_PLAY_SPEED_NORMAL CNend
\param[in] u32ChnID      Channel ID CNcomment: 通道号 CNend
\param[in] pstTrickMode   Pointer to the information about the trick playing mode  CNcomment:特技播放模式信息指针 CNend
\retval ::MT_SUCCESS   Success  CNcomment:成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT        The PVR recording module is not initialized.   CNcomment:PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID     The channel ID is invalid.      CNcomment:通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT       The specified channel is not initialized.   CNcomment:指定的通道未初始化 CNend
\retval ::MT_ERR_PVR_PLAY_INVALID_STATE   The channel status is incorrect, that is , the channel has been stopped.  CNcomment:通道状态错误：通道已经停止运行 CNend
\retval ::MT_ERR_DMX_INVALID_PARA      The parameter is invalid.     CNcomment:参数非法 CNend
\retval ::MT_ERR_PVR_NOT_SUPPORT    The trick playing mode is not supported.  CNcomment:不支持的特技播放模式 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_PlayTPlay(MT_U32 u32ChnID, const MT_UNF_PVR_PLAY_MODE_S *pstTrickMode);

/**
\brief Seeks a specified position to play, seeks by size and frame is not supported currently. CNcomment:定位到指定位置进行播放,暂不支持按照大小和帧数进行定位 CNend
\attention \n
If you call this API when the channel is disabled, the error code :MT_ERR_PVR_PLAY_INVALID_STATE is returned.\n
If the sought position is out of the boundary, the position is adjusted to the boundary automatically.\n
If you seek the position backward or from the start of a file, the next I frame in the corresponding position is sought. Otherwise, the previous I frame is sought.\n
If you seek the position forward or from the end of a file, the previous I frame in the corresponding position is sought. Otherwise, the next I frame is sought.\n
If no I frame is found, an error code is returned.
CNcomment:在通道已经停止的情况下调用本接口将返回错误码::MT_ERR_PVR_PLAY_INVALID_STATE\n
seek时如果超过边界，自动调整到边界\n
向后seek和从开始seek，则seek到对应位置的下一个I帧，不成功则找前一个I帧\n
向前seek和从结尾seek，则seek到对应位置的前一个I帧，不成功则找下一个I帧\n
找不到I帧会返回错误 CNend
\param[in] u32ChnID  Channel ID  CNcomment:通道号 CNend
\param[in] pstPosition Pointer to the playing position.  CNcomment:播放位置信息指针 CNend
\retval ::MT_SUCCESS  Success  CNcomment: 成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT        The PVR recording module is not initialized.  CNcomment:PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID        The channel ID is invalid.   CNcomment:通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT       The specified channel is not initialized.   CNcomment:指定的通道未初始化 CNend
\retval ::MT_ERR_PVR_PLAY_INVALID_STATE  The channel status is incorrect, that is , the channel has been stopped.  CNcomment: 通道状态错误：通道已经停止运行 CNend
\retval ::MT_ERR_DMX_INVALID_PARA       The parameter is invalid.   CNcomment: 参数非法 CNend
\retval ::MT_ERR_PVR_NOT_SUPPORT     The seek mode is not supported.   CNcomment:不支持的Seek模式 CNend
\retval ::MT_ERR_PVR_FILE_TILL_END       No corresponding frame is sought till the end of a file.  CNcomment:Seek到文件尾没有找到相应的帧 CNend
\retval ::MT_ERR_PVR_FILE_TILL_START   No corresponding frame is sought till the start of a file.    CNcomment:Seek到文件头没有找到相应的帧 CNend
\retval ::MT_ERR_PVR_FILE_CANT_READ     The file cannot be read.   CNcomment:读取文件失败 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_PlaySeek(MT_U32 u32ChnID, const MT_UNF_PVR_PLAY_POSITION_S *pstPosition);

/**
\brief Plays streams by frame. You can call this API to start step forward mode or step backward mode.
CNcomment: 逐帧播放。调用此接口可以进行逐帧倒退或者逐帧前进播放 CNend
\attention \n
Step backward mode is not supported currently. CNcomment:目前暂时不支持逐帧倒播 CNend
\param[in] u32ChnID     Channel ID  CNcomment:通道号 CNend
\param[in] s32Direction  Playing direction. If the value is greater than or equal to 0, the direction is forward; if the value is less than 0, the direction is backward. CNcomment:  播放方向。大于等于0表示前进；小于0表示后退 CNend
\retval ::MT_SUCCESS  Success  CNcomment:成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT        The PVR recording module is not initialized. CNcomment: PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID      The channel ID is invalid.    CNcomment: 通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT       The specified channel is not initialized.  CNcomment: 指定的通道未初始化 CNend
\retval ::MT_ERR_PVR_PLAY_INVALID_STATE   The channel status is incorrect, that is , the channel has been stopped. CNcomment: 通道状态错误：通道已经停止运行 CNend
\retval ::MT_ERR_PVR_NOT_SUPPORT   This function is not supported.  CNcomment:不支持的功能 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_PlayStep(MT_U32 u32ChnID, MT_S32 s32Direction);

/**
\brief Obtains the playing status of the PVR. CNcomment:获取PVR回放的状态 CNend
\attention \n
Before calling this API to obtain the playing status, you must create a player.
CNcomment:获取播放的状态，注意在播放器创建之后调用 CNend
\param[in] u32ChnID
\param[in] pstStatus,  u32CurPlayTimeInMs is invalid ,if pstStatus->u32CurPlayTimeInMs==0
\retval ::MT_SUCCESS Success  CNcomment: 成功 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_PlayGetStatus(MT_U32 u32ChnID, MT_UNF_PVR_PLAY_STATUS_S *pstStatus);

/**
\brief Obtains the information about the stream file recorded by the PVR based on the ID of the playing channel.  CNcomment:通过播放通道ID，获取PVR录制的码流文件信息 CNend
\attention \n
You can call this API to obtain the fixed information about the file to be played. However, the information is variable in the time-shift scenario.
You can call ::MT_UNF_PVR_PlayGetStatus to obtain the current playing position during playing.\n
You can call ::MT_UNF_PVR_RecGetStatus to obtain the current recording position during time shifting.
In addition, if both recording and playing do not start, you can call ::MT_UNF_PVR_GetFileAttrByFileName to obtain the attributes of a file.
CNcomment: 调用此接口用来获取播放文件的固定信息，但是对于时移场景这些信息是不断变化的
在播放过程中可以通过::MT_UNF_PVR_PlayGetStatus接口获取当前的播放位置 \n
在时移过程中可以通过::MT_UNF_PVR_RecGetStatus接口获取当前的录制位置
此外，如果既没有启动录制也没有启动播放，可以通过::MT_UNF_PVR_GetFileAttrByFileName接口获取文件属性 CNend
\param[in] u32ChnID  Channel ID  CNcomment:通道号 CNend
\param[in] pAttr Pointer to the playing status
\param[in] pAttr   CNcomment:播放状态信息指针 CNend
\retval ::MT_SUCCESS  Success  CNcomment:成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT            The PVR playing module is not initialized.  CNcomment:PVR播放模块未初始化 CNend
\retval ::MT_ERR_PVR_NUL_PTR           The pointer is null.   CNcomment: 空指针错误 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID      The channel ID is invalid.   CNcomment:通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT        The specified channel is not initialized. CNcomment: 指定的通道未初始化 CNend
\retval ::MT_ERR_PVR_FILE_CANT_OPEN      The file cannot be opened.  CNcomment:无法打开文件 CNend
\retval ::MT_ERR_PVR_FILE_CANT_READ    The file cannot be read.   CNcomment: 无法读文件 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_PlayGetFileAttr(MT_U32 u32ChnID, MT_UNF_PVR_FILE_ATTR_S *pAttr);

/**
\brief
Obtains the information about a stream file.
By calling this API, you can obtain the information about a specified stream file without creating a playing channel.
CNcomment:获取码流文件信息。调用此接口可以在不申请播放通道的情况下获取指定码流文件的信息 CNend
\attention \n
pFileName points to the name of a stream file rather than an index file.\n
You can call this API to obtain the attributes of a stream file at any time.
CNcomment:指向码流文件名而不是索引文件名.任何时候都可以调用此接口获取码流文件的属性 CNend
\param[in] pFileName Name of a stream file  CNcomment:码流文件名 CNend
\param[in] pAttr   Pointer to the playing status   CNcomment:播放状态信息指针 CNend
\retval ::MT_SUCCESS  Success  CNcomment:成功 CNend
\retval ::MT_ERR_PVR_NUL_PTR       The pointer is null.    CNcomment:空指针错误 CNend
\retval ::MT_ERR_PVR_FILE_CANT_OPEN  The file cannot be opened.  CNcomment:无法打开文件 CNend
\retval ::MT_ERR_PVR_FILE_CANT_READ   The file cannot be read. CNcomment:无法读文件 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_GetFileAttrByFileName(const MT_CHAR *pFileName, MT_UNF_PVR_FILE_ATTR_S *pAttr);

/**
\brief  Writes the user data. CNcomment: 写入用户数据 CNend
\attention \n
pFileName points to the name of a stream file rather than an index file.\n
The value of u32UsrDataLen cannot be greater than that of u32UsrDataInfoSize. The value of u32UsrDataInfoSize (one of recording channel attributes) is set before a file is recorded.
The user data is saved in the index file.\n
You need to write the user data after starting a recording channel.
CNcomment:pFileName指向码流文件名而不是索引文件名 \n
u32UsrDataLen的取值不能大于录制此文件时在录制通道属性里面指定的u32UsrDataInfoSize的取值 \n
用户数据被保存在索引文件中 \n
请在启动录制通道后再写入用户数据信息 CNend
\param[in] pFileName Name of a stream file CNcomment: 码流文件名 CNend
\param[in] pInfo    Pointer to the user data. The memory is allocated by customers. CNcomment:用户数据指针，空间由用户分配 CNend
\param[in] u32UsrDataLen   Length of the user data CNcomment: 用户数据长度 CNend
\retval ::MT_SUCCESS  Success  CNcomment:成功 CNend
\retval ::MT_ERR_PVR_NUL_PTR       The pointer is null.   CNcomment: 空指针错误 CNend
\retval ::MT_ERR_PVR_FILE_CANT_OPEN   The file cannot be opened.CNcomment:无法打开文件 CNend
\retval ::MT_ERR_PVR_FILE_CANT_READ  The file cannot be read.  CNcomment:无法读文件 CNend
\retval ::MT_ERR_DMX_INVALID_PARA    The parameter is invalid.  CNcomment: 参数非法 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_SetUsrDataInfoByFileName(const MT_CHAR *pFileName, MT_U8 *pInfo, MT_U32 u32UsrDataLen);

/**
\brief  Reads the user data. CNcomment: 读取用户数据 CNend
\attention \n
pFileName points to the name of a stream file rather than an index file.\n
The return value of pUsrDataLen indicates the length of the obtained user data. The value is the smaller one between the values of u32BufLen and actual length of the user data.
The actual length of the user data is greater than or equal to the maximum length of the user data set by calling MT_UNF_PVR_RecCreateChn. In this way, all user data can be read.\n
The PVR does not save the length of the user data set by calling MT_UNF_PVR_SetUsrDataInfoByFileName, but save the maximum length of the user data set by calling MT_UNF_PVR_RecCreateChn.
CNcomment:pFileName指向码流文件名而不是索引文件名 \n
pUsrDataLen返回读取到的用户数据的长度，其取值为u32BufLen和"实际用户数据长度"二者中的较小值。
"实际用户数据长度"将不小于MT_UNF_PVR_RecNewChn时配置的最大用户数据长度，以保证能够把全部用户数据读出 \n
PVR模块不保存MT_UNF_PVR_SetUsrDataInfoByFileName时传入的用户数据长度，只保存MT_UNF_PVR_RecNewChn时配置的最大用户数据长度 CNend
\param[in] pFileName    Name of a stream file CNcomment:码流文件名 CNend
\param[in] pInfo    User data buffer. The memory is allocated by customers.   CNcomment:用户数据缓冲区，空间由用户分配 CNend
\param[in] u32BufLen   Buffer length .CNcomment:缓冲区长度 CNend
\param[in] pUsrDataLen Actual length of the obtained user data .CNcomment:实际读取到的用户数据长度 CNend
\retval ::MT_SUCCESS  Success  CNcomment:成功 CNend
\retval ::MT_ERR_PVR_NUL_PTR        The pointer is null.      CNcomment: 空指针错误 CNend
\retval ::MT_ERR_PVR_FILE_CANT_OPEN    The file cannot be opened. CNcomment:无法打开文件 CNend
\retval ::MT_ERR_PVR_FILE_CANT_READ    The file cannot be read.  CNcomment:无法读文件 CNend
\retval ::MT_ERR_DMX_INVALID_PARA      The parameter is invalid.  CNcomment: 参数非法 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_GetUsrDataInfoByFileName(const MT_CHAR *pFileName, MT_U8 *pInfo, MT_U32 u32BufLen, MT_U32* pUsrDataLen);

/**
\brief  Registers an event callback function. You can call this API to register a callback function for PVR event. In this way, you can handle the events occurred during PVR recording and playing.
CNcomment: 注册事件回调函数。调用此接口可以注册PVR事件的回调函数，从而对PVR录制、播放过程中产生的事件进行处理 CNend
\attention \n
You can call ::MT_UNF_PVR_UnRegisterEvent to deregister an event. If a registered event is not deregistered, it is always valid.
For the PVR_EVENT_PLAY_REACH_REC message, it is recommended to stop the playing of the PVR by calling the registered callback function, and switch the playing mode to live mode of the AVPLAY.
If the recording attribute is set to non-rewind, the PVR_EVENT_REC_OVER_FIX message is reported at the end of recording when the recording length reaches the specified length.
When the recording attribute is set to rewind, the PVR_EVENT_REC_OVER_FIX message is reported during each rewind. In this case, the value of s32EventValue of the message handling callback function is the times of rewind (counting from 1).
The PVR_EVENT_REC_REACH_PLAY message is reported when the recording speed will reach the playing speed. In this case, the value of s32EventValue is the number of bytes between the recording position and the playing position.
CNcomment: 可以使用::MT_UNF_PVR_UnRegisterEvent来反注册1个事件，否则已经注册的事件将一直保持有效
对于PVR_EVENT_PLAY_REACH_REC消息，建议用户在注册的回调函数中停止PVR播放，并切换到直播模式用AVPLAY进行播放。
当录制属性配置为不回绕的时候，PVR_EVENT_REC_OVER_FIX消息在录制长度到达指定长度，即将停止录制的时候上报；
当录制属性配置为回绕的时候，PVR_EVENT_REC_OVER_FIX消息在每次回绕的时候都会上报，此时，消息处理回调函数的s32EventValue参数值为回绕的次数(从1开始计数)。
PVR_EVENT_REC_REACH_PLAY消息在录制即将追上播放的时候上报，此时，消息处理回调函数的s32EventValue参数值为录制位置距离播放位置的字节数。 CNend
\param[in] enEventType  Type of the event to be registered.  CNcomment: 想要注册的事件类型 CNend
\param[in] callBack   Event handling callback function.   CNcomment: 事件处理函数 CNend
\param[in] args      Parameters of a customized callback function.    CNcomment:用户自定义回调函数的参数 CNend
\retval ::MT_SUCCESS   Success CNcomment: 成功 CNend
\retval ::MT_ERR_PVR_INTF_EVENT_INVAL   The event is invalid.  CNcomment: 非法的事件 CNend
\retval ::MT_ERR_PVR_ALREADY         The registering event has been registered.    CNcomment: 尝试对已经注册的事件再次注册 CNend
\retval ::MT_ERR_PVR_INTF_EVENT_NOREG    Registration fails because the callback function pointer is incorrect. CNcomment:  回调函数指针出错，无法注册成功 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_RegisterEvent(MT_UNF_PVR_EVENT_E enEventType, eventCallBack callBack, MT_VOID *args);

/**
\brief  Deregisters a PVR event. CNcomment: 去注册PVR事件的回调函数 CNend
\attention \n
You can call this API to deregister an event. If a registered event is not deregistered, it is always valid.
CNcomment: 可以使用此接口来去注册1个事件，否则已经注册的事件将一直保持有效 CNend
\param[in] enEventType      Type of the event to be deregistered    CNcomment: 想要去注册的事件类型 CNend
\retval ::MT_SUCCESS   Success  CNcomment:成功 CNend
\retval ::MT_ERR_PVR_INTF_EVENT_INVAL  The event is invalid. CNcomment: 非法的事件 CNend
\retval ::MT_ERR_PVR_INTF_EVENT_NOREG  The deregistering event is not registered. CNcomment: 对没有注册的事件进行去注册 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_UnRegisterEvent(MT_UNF_PVR_EVENT_E enEventType);

/**
\brief Create index file by ts file. CNcomment:根据码流文件创建索引文件 CNend
\attention \n
This function is not supported currently. 
CNcomment: 暂时不支持此功能 CNend
\param[in] pstTsFileName
\param[in] pstIdxFileName
\param[in] pAttr
\retval ::MT_SUCCESS Success CNcomment:  成功 CNend
\retval ::MT_ERR_PVR_NOT_SUPPORT  This function is not supported. CNcomment: 不支持此功能 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_CreateIdxFile(MT_U8* pstTsFileName, MT_U8* pstIdxFileName, MT_UNF_PVR_GEN_IDX_ATTR_S* pAttr);

/**
\brief Controls whether to output debugging information.  CNcomment:配置是否输出调试信息 CNend
\attention \n
The debugging information about the severity level higher than the error level is output.
CNcomment:默认输出error 以上级别的调试信息。注意和全局打印级别配合使用，即此级别需要小于等于全局打印级别才有效 CNend
\param[in] enDebugLevel  Output level of the debugging information CNcomment: 调试信息输出级别 CNend
\retval none
\see \n
N/A
*/
MT_VOID MT_UNF_PVR_ConfigDebugInfo(mt_log_level_e enDebugLevel);

/**
\brief  Deletes the stream files and index files generated during recording. CNcomment:删除录制时生成的码流文件和索引文件 CNend
\attention \n
pFileName points to the name of a stream file rather than an index file.\n
After this API is called, the stream files and index files generated during stream recording are deleted.
If a stream file is greater than 4 GB, multiple files named data.ts, data.ts.0001, data.ts.0002, data.ts.0003, and ... are generated. To delete these files, you only need to enter the file name data.ts.
CNcomment:pFileName指向码流文件名而不是索引文件名\n
调用此接口后，录制此码流时生成的码流文件和索引文件都将被删除\n
如果码流文件大于4G将会生成data.ts, data.ts.0001, data.ts.0002, data.ts.0003 ...\n
等多个文件，但是删除时只需要传入文件名"data.ts"就可以了。 CNend
\param[in] pFileName  Name of a stream file  CNcomment:码流文件名 CNend
\retval none
\see \n
N/A
*/
MT_VOID MT_UNF_PVR_RemoveFile(const MT_CHAR *pFileName);


/**
\brief
Register extra write callback function.When the stream file need to be extra processed  CNcomment: 注册外部读、写数据回调函数。当文件需要特殊处理时注册，如私有协议加扰的节目。 CNend
\attention \n
N/A
\param[in] u32ChnID    Channel ID  CNcomment:通道号 CNend
\param[in] enExtraCallbackType  Callback function type CNcomment: 回调函数类型 CNend
\retval ::MT_SUCCESS Success  CNcomment: 成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT  The PVR recording module is not initialized CNcomment: PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID The channel ID is invalid.  CNcomment:通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT   Specified channel is not initialized   CNcomment:   指定的通道未初始? CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_RegisterExtraCallback(MT_U32 u32ChnID, MT_UNF_PVR_EXTRA_CALLBACK_E enExtraCallbackType, ExtraCallBack fCallback, MT_VOID *args);

/**
\brief Deregisters extra write callback function.When the stream file need to be extra processed  CNcomment:去注册外部读、写数据回调函数。 CNend
\attention \n
N/A
\param[in] u32ChnID  Channel ID  CNcomment:通道号 CNend
\param[in] enExtraCallbackType  Callback function type CNcomment: 回调函数类型 CNend
\retval ::MT_SUCCESS Success  CNcomment: 成功 CNend
\retval ::MT_ERR_PVR_NOT_INIT  The PVR recording module is not initialized CNcomment: PVR录制模块未初始化 CNend
\retval ::MT_ERR_PVR_INVALID_CHNID The channel ID is invalid.  CNcomment:通道号非法 CNend
\retval ::MT_ERR_PVR_CHN_NOT_INIT   Specified channel is not initialized   CNcomment:   指定的通道未初始化 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_UnRegisterExtraCallBack(MT_U32 u32ChnID, MT_UNF_PVR_EXTRA_CALLBACK_E enExtraCallbackType);

/** @} */  /** <!-- ==== API Declaration End ==== */

/*
set pvr thread attr
\param[in] type:thread type.0:index read.1:index write.2:ts record.3:read to play
\param[in] schedpolicy:SCHED_FIFO(default) SCHED_RR SCHED_OTHER
\param[in] priority:1-99
\param[in] stacksize:0(auto) \16k-1M
\retval ::0 sucess,!0 failure
*/
MT_S32 MT_UNF_PVR_SetThreadAttr(PVR_THREAD_TYPE type, MT_S32 schedpolicy,  MT_S32 priority, MT_S32 stacksize);

/*start new event recording while timeshift!
\param[in] u32ChnID  Channel ID  CNcomment:通道号 CNend
\param[in] pEeventName  pEeventName CNcomment: 事件名
\retval ::0 sucess,!0 failure

*/
MT_S32 MT_UNF_PVR_Start_Event_Rec(MT_U32 u32ChnID, MT_CHAR *u8EeventName, MT_U32  u32StartTimeMS,  PVR_TIMESHIFT_EVENT_TYPE  type);

/*start new event recording while timeshift!
\param[in] u32ChnID  Channel ID  CNcomment:通道号 CNend
\retval ::MT_ERR_PVR_NOT_INIT  The PVR recording module is not initialized CNcomment: PVR录制模块未初始化 CNend
\retval ::0 sucess,!0 failure
*/
MT_S32 MT_UNF_PVR_Stop_Event_Rec(MT_U32 u32ChnID, MT_CHAR *u8EeventName);

/*save event to pvr!
\param[in] u32ChnID  Channel ID  CNcomment:通道号 CNend
\retval ::MT_ERR_PVR_INVALID_PARA  parameter is invalid CNcomment: 无效参数 CNend
\retval ::0 sucess,!0 failure
*/
MT_S32 MT_UNF_PVR_Save_Event_Rec(MT_U32 u32ChnID, MT_CHAR *u8EeventName);

/*delete event pvr!
\param[in] u8TimeShiftPath : event path  CNcomment: 事件存储路径 CNend
\param[in] u8EeventName   : event name CNcomment: 事件的名字 CNend
\retval ::MT_ERR_PVR_NOT_INIT  The PVR recording module is not initialized CNcomment: PVR录制模块未初始化 CNend
\retval ::0 sucess,!0 failure
*/
MT_S32 MT_UNF_PVR_Delete_Event_Rec(MT_U32 u32ChnID, MT_CHAR *u8TimeShiftPath, MT_CHAR *u8EeventName);

/*delete event pvr!
\param[in] u32ChnID  Channel ID  CNcomment:通道号 CNend
\retval ::MT_ERR_PVR_NOT_INIT  The PVR recording module is not initialized CNcomment: PVR录制模块未初始化 CNend
\retval ::0 sucess,!0 failure
*/
MT_S32 MT_UNF_PVR_StopCache_Event_Rec(MT_U32 u32ChnID);

/*brief Set PVR CA data
\param[in] pIdxFileName  Index file name CNcomment:索引数据文件名 CNend
\param[in] pInfo  CA data buffer  CNcomment:CA数据buffer CNend
\param[in] u32CADataLen  CA data length CNcomment:CA数据长度 CNend
\param[in] u8IsAPPEND Append or replace  CNcomment:追加还是替换 CNend
\retval ::MT_SUCCESS Success  CNcomment: 成功 CNend
\retval ::MT_FAILURE failure  CNcomment:操作失败 CNend
*/
MT_S32 MT_UNF_PVR_SetCAData(const MT_CHAR *pIdxFileName, MT_U8 *pInfo, MT_U32 u32CADataLen, MT_U8 u8IsAPPEND);

/*brief Get PVR CA data
\param[in] pIdxFileName  Index file name CNcomment:索引数据文件名 CNend
\param[in] pInfo  CA data buffer  CNcomment:CA数据buffer CNend
\param[in] u32BufLen  The length of CA data buffer  CNcomment:CA数据buffer长度 CNend
\param[out] u32CADataLen CA data length  CNcomment:CA数据有效长度 CNend
\retval ::MT_SUCCESS Success  CNcomment: 成功 CNend
\retval ::MT_FAILURE failure  CNcomment:操作失败 CNend
*/
MT_S32 MT_UNF_PVR_GetCAData(const MT_CHAR *pIdxFileName, MT_U8 *pInfo, MT_U32 u32BufLen, MT_U32* u32CADataLen);

/**
\brief Set PVR TS file size. Call this function before MT_UNF_PVR_RecCreateChn to make effect.
CNcomment:调用此接口设置录制单个TS文件大小
\attention \n
\param[in] u32NewFileSizeInMBytes  The new TS file size wiht Mbytes unit.CNcomment:设置单个 TS文件大小 M 为单位 CNend
\retval ::MT_S32 The size before set the new one. CNcomment:返回原来的文件大小设置 CNend
*/
MT_U32 MT_UNF_PVR_SetRecFileSize( MT_U32 u32NewFileSizeInMBytes);

/**
\brief pvr play change audio pid . CNcomment:PVR切换audio pid CNend
\attention \n
Before calling this API to obtain the playing status, you must create a player.
CNcomment:PVR回播切换音轨，回播前先调用MT_UNF_AVPLAY_ATTR_ID_MULTIAUD CNend
\param[in] u32ChnID  Channel ID  CNcomment:通道号 CNend
\param[in] aorv restart audio decode and set adec attr CNcomment:是否重置adec attr CNend
\param[in] type Audio decoder type(HA_CODEC_ID_E) CNcomment:音频解码类型 CNend
\param[in] pid audio track pid CNcomment:音轨对应PID CNend
\retval ::MT_SUCCESS Success  CNcomment: 成功 CNend
\see \n
N/A
*/
MT_S32 MT_UNF_PVR_PlayChangeAudioPid(MT_U32 u32ChnID, MT_BOOL aorv, MT_U32 type, MT_U32 pid);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* End of #ifndef __MT_UNF_PVR_H__ */
