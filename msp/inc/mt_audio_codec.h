/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_AUDIO_CODEC_H__
#define __MT_AUDIO_CODEC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "mt_type.h"

/**Define hardware Codec*/
/**CNcomment:定义硬件编解码*/
#define HA_HW_CODEC_SUPPORT

#define VENDOR_MT 0x040
#define VENDOR_NONE 0x010
#define HA_VENDOR_OFFSETK  25 /* (32-7) */
#define HA_VENDOR_MASK     (0x007fUL << HA_VENDOR_OFFSETK)
#define HA_FORMAT_OFFSETK  16 /* (32-7-9) */
#define HA_FORMAT_MASK     (0x01ffUL << HA_FORMAT_OFFSETK)
#define HA_RESERVED_OFFSET 13 /* (32-7-9-3) */
#define HA_RESERVED_MASK   (0x007UL << HA_RESERVED_OFFSET)
#define HA_ID_OFFSET       0 /* (32-7-9-3-13) */
#define HA_ID_MASK         (0x1fffUL<< HA_ID_OFFSET)

#define HA_BUILD_CODEC_ID(vendor, format, id) (((((mt_u32)vendor) << HA_VENDOR_OFFSETK) & HA_VENDOR_MASK) | ((((mt_u32)format) << HA_FORMAT_OFFSETK) & HA_FORMAT_MASK) | (((mt_u32)id) & HA_ID_MASK))
#define HA_GET_VENDOR(codec) ((mt_u32)(codec&HA_VENDOR_MASK)>>HA_VENDOR_OFFSETK)
#define HA_GET_FORMAT(codec) ((mt_u32)(codec&HA_FORMAT_MASK)>>HA_FORMAT_OFFSETK)
#define HA_GET_ID(codec)     ((mt_u32)(codec&HA_ID_MASK)>>HA_ID_OFFSET)

/********************************Macro Definition********************************/
/** \addtogroup      ACODEC */
/** @{ */  /** <!-- 【ACODEC】 */

/** AAC Decoder Config */
#define AUD_MAX_CHANNEL   8

/**Define HA codec maximum audio channel*/
/**CNcomment:定义HA codec 最大音频通道 */
#define HA_AUDIO_MAXCHANNELS 8

typedef enum mtHA_FORMAT_E
{
    FORMAT_MP2 = 0x000,  /**<MPEG audio layer 1, 2.*/ /**<CNcomment:MPEG音频第一层、第二层 */
    FORMAT_MP3, /**<MPEG audio layer 1, 2, 3.*/ /**<CNcomment:MPEG音频第一层、第二层 、第三层*/
    FORMAT_AAC,
    FORMAT_AC3,
    FORMAT_DTS,
    FORMAT_VORBIS,
    FORMAT_DVAUDIO,
    FORMAT_WMAV1,
    FORMAT_WMAV2,
    FORMAT_MACE3,
    FORMAT_MACE6,
    FORMAT_VMDAUDIO,
    FORMAT_SONIC,
    FORMAT_SONIC_LS,
    FORMAT_FLAC,
    FORMAT_MP3ADU,
    FORMAT_MP3ON4,
    FORMAT_SHORTEN,
    FORMAT_ALAC,
    FORMAT_WESTWOOD_SND1,
    FORMAT_GSM,
    FORMAT_QDM2,
    FORMAT_COOK,
    FORMAT_TRUESPEECH,
    FORMAT_TTA,
    FORMAT_SMACKAUDIO,
    FORMAT_QCELP,
    FORMAT_WAVPACK,
    FORMAT_DSICINAUDIO,
    FORMAT_IMC,
    FORMAT_MUSEPACK7,
    FORMAT_MLP,
    FORMAT_GSM_MS, /**<as found in WAV.*/ /**<CNcomment:存在WAV格式中 */
    FORMAT_ATRAC3,
    FORMAT_VOXWARE,
    FORMAT_APE,
    FORMAT_NELLYMOSER,
    FORMAT_MUSEPACK8,
    FORMAT_SPEEX,
    FORMAT_WMAVOICE,
    FORMAT_WMAPRO,
    FORMAT_WMALOSSLESS,
    FORMAT_ATRAC3P,
    FORMAT_EAC3,
    FORMAT_SIPR,
    FORMAT_MP1,
    FORMAT_TWINVQ,
    FORMAT_TRUEHD,
    FORMAT_MP4ALS,
    FORMAT_ATRAC1,
    FORMAT_BINKAUDIO_RDFT,
    FORMAT_BINKAUDIO_DCT,
    FORMAT_DRA,
    FORMAT_OPUS,
    FORMAT_OGG,

    FORMAT_PCM = 0x100,/**<various PCM codecs.*/ /**<CNcomment:PCM格式 */
    FORMAT_PCM_BLURAY = 0x121,

    FORMAT_ADPCM = 0x130,/**<various ADPCM codecs.*/ /**<CNcomment:ADPCM格式 */

    FORMAT_AMR_NB = 0x160,/**<various AMR codecs.*/ /**<CNcomment:AMR格式 */
    FORMAT_AMR_WB,
    FORMAT_AMR_AWB,

    FORMAT_RA_144 = 0x170,/**<RealAudio codecs.*/ /**<CNcomment:RealAudio格式 */
    FORMAT_RA_288,

    FORMAT_DPCM = 0x180,/**<various DPCM codecs.*/ /**<CNcomment:DPCM格式 */

    FORMAT_G711 = 0x190,/**<various G.7xx codecs.*/ /**<CNcomment:G.7xx格式 */
    FORMAT_G722,
    FORMAT_G7231,
    FORMAT_G726,
    FORMAT_G728,
    FORMAT_G729AB,


    FORMAT_MULTI = 0x1f0,/**<support multi codecs.*/ /**<CNcomment:多种格式 */
	FORMAT_VVID = 0x1f1,/**<support VVID codecs.*/

    FORMAT_BUTT = 0x1ff,
} HA_FORMAT_E;

/**HA format definition*/
/**CNcomment:HA_Codec定义*/
typedef enum mtHA_CODEC_ID_E
{

    HA_AUDIO_ID_PCM             = HA_BUILD_CODEC_ID(VENDOR_MT, FORMAT_PCM, 0x0000),
    HA_AUDIO_ID_MP2             = HA_BUILD_CODEC_ID(VENDOR_NONE, FORMAT_MP2, 0x0002),
    HA_AUDIO_ID_MP3             = HA_BUILD_CODEC_ID(VENDOR_NONE, FORMAT_MULTI, 0x0003),
    HA_AUDIO_ID_AAC             = HA_BUILD_CODEC_ID(VENDOR_MT, FORMAT_AAC, 0x001),
    HA_AUDIO_ID_BLYRAYLPCM      = HA_BUILD_CODEC_ID(VENDOR_MT, FORMAT_PCM_BLURAY, 0x021),
    HA_AUDIO_ID_COOK            = HA_BUILD_CODEC_ID(VENDOR_MT, FORMAT_COOK, 0x0009),
    HA_AUDIO_ID_DRA             = HA_BUILD_CODEC_ID(VENDOR_MT, FORMAT_DRA, 0x007),
    HA_AUDIO_ID_WMA9STD         = HA_BUILD_CODEC_ID(VENDOR_NONE, FORMAT_MULTI, 0x0006),
    HA_AUDIO_ID_VORBIS          = HA_BUILD_CODEC_ID(VENDOR_NONE, FORMAT_VORBIS, 0x008),
    HA_AUDIO_ID_OGG             = HA_BUILD_CODEC_ID(VENDOR_NONE, FORMAT_OGG, 0x009),
    HA_AUDIO_ID_FLAC            = HA_BUILD_CODEC_ID(VENDOR_NONE, FORMAT_FLAC, 0x00a),
    HA_AUDIO_ID_APE             = HA_BUILD_CODEC_ID(VENDOR_NONE, FORMAT_APE, 0x00b),
    HA_AUDIO_ID_OPUS            = HA_BUILD_CODEC_ID(VENDOR_NONE, FORMAT_OPUS, 0x00c),
    HA_AUDIO_ID_AMRNB           = HA_BUILD_CODEC_ID(VENDOR_MT, FORMAT_AMR_NB, 0x0100),
    HA_AUDIO_ID_AMRWB           = HA_BUILD_CODEC_ID(VENDOR_MT, FORMAT_AMR_WB, 0x0110),
    HA_AUDIO_ID_G711            = HA_BUILD_CODEC_ID(VENDOR_NONE, FORMAT_G711, 0x0102),
    HA_AUDIO_ID_G722            = HA_BUILD_CODEC_ID(VENDOR_NONE, FORMAT_G722, 0x0105),
    HA_AUDIO_ID_TRUEHD          = HA_BUILD_CODEC_ID(VENDOR_NONE, FORMAT_TRUEHD, 0x0008),
    HA_AUDIO_ID_AC3PASSTHROUGH  = HA_BUILD_CODEC_ID(VENDOR_MT, FORMAT_MULTI, 0x0055),
    HA_AUDIO_ID_EAC3PASSTHROUGH  = HA_BUILD_CODEC_ID(VENDOR_MT, FORMAT_MULTI, 0x0056),
    HA_AUDIO_ID_DTSPASSTHROUGH  = HA_BUILD_CODEC_ID(VENDOR_MT, FORMAT_DTS, 0x1025),

    /** DOLBY/DTS IPR Codec*/
    HA_AUDIO_ID_DOLBY_PLUS      = HA_BUILD_CODEC_ID(VENDOR_MT, FORMAT_MULTI, 0x1010),
    HA_AUDIO_ID_DOLBY_TRUEHD    = HA_BUILD_CODEC_ID(VENDOR_NONE, FORMAT_TRUEHD, 0x1011),
    HA_AUDIO_ID_DOLBY_CONVERT   = HA_BUILD_CODEC_ID(VENDOR_NONE, FORMAT_EAC3, 0x1012),
    HA_AUDIO_ID_DOLBY_AC4		= HA_BUILD_CODEC_ID(VENDOR_MT, FORMAT_MULTI, 0x1013),
    HA_AUDIO_ID_DTSHD           = HA_BUILD_CODEC_ID(VENDOR_NONE, FORMAT_DTS, 0x1020),
    HA_AUDIO_ID_DTSM6           = HA_BUILD_CODEC_ID(VENDOR_NONE, FORMAT_DTS, 0x1030),

    /** FFMPEG Codec*/
    HA_AUDIO_ID_FFMPEG_DECODE   = HA_BUILD_CODEC_ID(VENDOR_MT, FORMAT_MULTI,  0x03ff),
    HA_AUDIO_ID_FFMPEG_WMAPRO   = HA_BUILD_CODEC_ID(VENDOR_MT, FORMAT_WMAPRO, 0x041f),

    /** CUSTOMER Codec*/
    HA_AUDIO_ID_CUSTOM_0        = HA_BUILD_CODEC_ID(VENDOR_MT, FORMAT_MULTI, 0x0400),
    HA_AUDIO_ID_CUSTOM_1        = HA_BUILD_CODEC_ID(VENDOR_MT, FORMAT_MULTI, 0x0401),
	HA_AUDIO_ID_VVID			   = HA_BUILD_CODEC_ID(VENDOR_MT, FORMAT_VVID, 0x0402),
    HA_AUDIO_ID_INVALID         = 0xFFFFFFFF,
} HA_CODEC_ID_E;


/**Define the error codes of an HA codec.*/
/**CNcomment:定义HA codec 错误码*/
typedef enum mtHA_ERRORTYPE_E
{
    HA_ErrorNone = 0, /**<None error.*/ /**<CNcomment:没有错误 */

    HA_ErrorInsufficientResources = (mt_s32) 0x80001000,/**<The device fails to be created due to insufficient resources.*/ /**<CNcomment:资源不够，创建设备失败 */

    HA_ErrorInvalidParameter = (mt_s32) 0x80001001, /**<The input parameter is invalid.*/ /**<CNcomment:输入参数非法 */

    HA_ErrorStreamCorrupt = (mt_s32) 0x80001002, /**<The decoding fails due to incorrect input streams.*/ /**<CNcomment:输入码流错误，解码失败 */

    HA_ErrorNotEnoughData = (mt_s32) 0x80001003,/**<The decoding ends due to insufficient streams.*/ /**<CNcomment:输入码流不够，退出解码 */

    HA_ErrorDecodeMode = (mt_s32) 0x80001004,/**<The decoding mode is not supported.*/ /**<CNcomment:解码模式不支持 */

    HA_ErrorNotSupportCodec = (mt_s32) 0x80001005,/**<The codec is not supported.*/ /**<CNcomment:解码器不支持 */

    HA_ErrorInBufFull = (mt_s32) 0x80001006,/**<Input buffer is full.*/ /**<CNcomment:输入缓存满 */

    HA_ErrorOutBufEmpty = (mt_s32) 0x80001007,/**<Output buffer is empty.*/ /**<CNcomment:输出缓存空 */

    HA_ErrorMax = 0x9FFFFFFF
} MT_HA_ERRORTYPE_E;

/**Definition of the distribution mode of the channels of an HA codec.*/
/**CNcomment:定义HA codec 声道分布模式*/
typedef enum mtHA_CHANNELTYPE_E
{
    HA_AUDIO_ChannelNone = 0x0,    /**< Unused or empty.*/ /**<CNcomment:未使用或为空*/
    HA_AUDIO_ChannelLF  = 0x1,     /**< Left front.*/ /**<CNcomment:左前*/
    HA_AUDIO_ChannelRF  = 0x2,     /**< Right front.*/ /**<CNcomment:右前*/
    HA_AUDIO_ChannelCF  = 0x3,     /**< Center front.*/ /**<CNcomment:中置*/
    HA_AUDIO_ChannelLS  = 0x4,     /**< Left surround.*/ /**<CNcomment:左环绕*/
    HA_AUDIO_ChannelRS  = 0x5,     /**< Right surround.*/ /**<CNcomment:右环绕*/
    HA_AUDIO_ChannelLFE = 0x6,     /**< Low frequency effects.*/ /**<CNcomment:低音*/
    HA_AUDIO_ChannelCS  = 0x7,     /**< Back surround.*/ /**<CNcomment:后环绕*/
    HA_AUDIO_ChannelLR  = 0x8,     /**< Left rear.*/ /**<CNcomment:左后*/
    HA_AUDIO_ChannelRR  = 0x9,     /**< Right rear.*/ /**<CNcomment:右后*/
    HA_AUDIO_ChannelMax = 0x7FFFFFFF
} MT_HA_CHANNELTYPE_E;

/**Definition of the decoding mode of an HA decoder.*/
/**CNcomment:定义HA 解码器解码模式*/
typedef enum mtHA_DECODEMODE_E
{
    HD_DEC_MODE_RAWPCM = 0,        /**<PCM decoding mode.*/ /**<CNcomment:PCM 解码模式*/
    HD_DEC_MODE_THRU,              /**<SPIDF61937 passthrough decoding mode only, such as AC3/DTS.*/ /**<CNcomment:透传解码模式*/
    HD_DEC_MODE_SIMUL,             /**<PCM and passthrough decoding mode.*/ /**<CNcomment:PCM + 透传解码模式*/
    #ifdef CONFIG_MT_AUDIO_AD
    HD_DEC_MODE_PCMPCM=4,          /**dolby dual decode.*/ /**CNcomment:dolby两路解码:PCM + PCM*/
    #endif
    HD_DEC_MODE_BUTT = 0x7FFFFFFF
} MT_HA_DECODEMODE_E;

/**Definition of the HA codec version.*/
/**CNcomment:HA codec 版本定义*/
typedef union mtMT_HAAUDIO_VERSIONTYPE_U
{
    struct
    {
        mt_u8 u8VersionMajor;      /**< Major version.*/ /**<CNcomment:主版本号 */
        mt_u8 u8VersionMinor;      /**< Minor version.*/ /**<CNcomment:副版本号 */
        mt_u8 u8Revision;          /**< Revision version.*/ /**<CNcomment:修订版本 */
        mt_u8 u8Step;              /**< Step version.*/ /**<CNcomment:阶段性版本 */
    } s;
    mt_u32 u32Version;
} MT_HAAUDIO_VERSIONTYPE_U;

/**Definition of the data types of the PCM module in an HA codec HA codec.*/
/**CNcomment:pcm 数据类型结构体定义*/
typedef struct mtHA_PCMMODETYPE_S
{
    mt_u32              u32DesiredOutChannels;/**<Input,number of expected output channels.If the number of original channels is not equal to the number of expected output channels,
                                                  the decoded automatically performs down-mixing or up-mixing,In this way,
                                                  the number of output channels is equal to the value of u32DesiredOutChannels,1: mono; 2 stereo. \n*/
                                              /**<CNcomment:IN 期望输出声道数目. 如果码流原始声道不等于
                                                  期望输出声道，解码器自动进行down-mix 或up-mix 处理，使得输出
                                                  声道等于u32DesiredOutChannels.  1: mono, 2 for stereo*/
    MT_BOOL             bInterleaved;/**<Whether the PCM data is interleaved.
                                         MT_TRUE: interleaved mode (L/R/L/R.. .L/R/L/R).
                                         MT_FALSE: non-interleaved mode (L/L/L.../R/R/R...). \n*/
                                     /**<CNcomment:IN PCM 数据是否交织模式
                                         MT_TRUE:  交织模式: L/R/L/R...L/R/L/R.
                                         MT_FALSE: 非交织模式: L/L/L......../R/R/R....... */
    mt_u32              u32BitPerSample;/**<Input Bit width of the PCM data.Only 16-bit or 24-bit width is supported.For the data of 16-bit width, 16-bit memory is used,
                                            For the data of 24-bit width, 32-bit memory is used,The active bits of the 24-bit PCM data are upper bits, and the lower eight bits are padded with 0s. \n*/
                                        /**<CNcomment:IN PCM 数据位宽,仅支持16 或24 数据位宽 16bit 位宽占用16bit 内存,
                                            24bit 位宽占用32比特内存.24bit PCM数据有效为在高位，低8 位补0*/
    mt_u32              u32DesiredSampleRate; /**<Input, expected output sampling rate.*/ /**<CNcomment:IN 期望输出采样频率*/
    MT_HA_CHANNELTYPE_E enChannelMapping[HA_AUDIO_MAXCHANNELS]; /**<Input, distribution mode of output channels.*/ /**<CNcomment:IN 输出声道分布模式*/
} MT_HA_PCMMODETYPE_S;

/**Configuration parameters required by an HA codec for creating devices.*/
/**CNcomment:HA 解码器创建设备的配置参数*/
typedef struct mtHADECODE_OPENPARAM_S
{
    mt_u32              u32CodecId; /**<Codec type.*/ /**<CNcomment: 编码类型*/
    MT_HA_DECODEMODE_E  enDecMode;  /**<IN Decode Mode.*/ /**<CNcomment:解码模式*/    
    MT_HA_PCMMODETYPE_S sPcmformat; /**<IN data types of pcm module.*/ /**<CNcomment:PCM数据类型结构体*/
    mt_void *           pCodecPrivateData;/**<Input, pointer to private data.If the decoder does not contain private data, this parameter is set to 0. */
                                          /**<CNcomment:IN  指向私有配置结构体的指针，如果解码器没有私有配置，设置为0*/
    mt_u32              u32CodecPrivateDataSize;/**<Input, size of the private data.If the decoder does not contain private data, this parameter is set to 0. */
                                                /**<CNcomment:IN 私有结构体大小, 如果解码器没有私有配置，设置为0*/
    mt_u32              u32DolbyDownmixMode; /* Downmix mode: 0-Lo/Ro, 1-Lt/Rt */   
} MT_HADECODE_OPENPARAM_S;

/**Define avc feature parameter**/
typedef struct mtHADECODE_AVC_PARAM_S 
{
    MT_BOOL  bAVCOn;  /**<enable/disable avc feature, range: 0-disable(default), 1-enable*/ /**<CNcomment: 使能/关闭avc 功能*/
    mt_s16 s16AVCLevelCfg; /**<set avc config level,range: 0 ~ 20; */ /**<CNcomment: 设置avc 等级，范围:  0  ~ 20*/
} MT_HADECODE_AVC_PARAM_S;


/**Input streams of an HA decoder.*/
/**CNcomment:HA 解码器输入码流结构体*/
typedef struct mtHADECODE_INPACKET_S
{
    mt_u32  u32PtsMs;  /**<Input, presentation time stamp (PTS) (in ms).Note: The HA decoder must discard this information. */ /**<CNcomment:当前时间戳*/
    mt_u64  u64PtsUs;  /**<Input, presentation time stamp (PTS) (in us).Note: The HA decoder must discard this information. */ /**<CNcomment:当前时间戳*/
    mt_u8  *pu8Data;   /**<Input/output, pointer to input streams.The decoder update this pointer after decoding. */ /**<CNcomment:IN/OUT 输入码流指针，解码完成后解码器更新该指针*/
    mt_s32  s32Size;   /**<Input, size (in byte) of input streams.*/ /**<CNcomment:IN 输入码流大小. unit: Byte*/
    MT_BOOL bEndOfFrm; /**<Input, the last frame or not.*/ /**<CNcomment:IN 是否最后一帧*/
} MT_HADECODE_INPACKET_S;

/**Get Open parameter of an HA decoder.*/
/**CNcomment:获取HA 解码器打开参数*/
typedef struct
{
    mt_u32  enCmd;
    MT_HADECODE_OPENPARAM_S *pstDecodeParams;
} HA_CODEC_GET_DECOPENPARAM_PARAM_S;

/**Definition of the decoder output pts infomations.*/
/**CNcomment:pts输出信息*/
typedef struct mtMT_HA_OUTPTSINFO_S
{
    union
    {
        mt_u32 u32SwDecoderBytesLeft;   /**<left bytes of software decoder.*/ /**<CNcomment:软解剩余字节数 */
        mt_u32 u32HwDecoderPtsReadPos;  /**<pts read position of software decoder.*/ /**<CNcomment:硬解pts读位置 */
    } unPts;
} MT_HA_OUTPTSINFO_S;

/**audio stream info.*/
typedef struct mtMT_HA_AUDIO_STREAM_INFO_S
{
	/*common*/
	mt_u32 aud_type;
	mt_u32 origin_m_channum;	//original main channels
	char hdmi_mode;				//hdmi mode by user
	char spdif_mode;			//spdif mode by user

	/*dolby*/
	char dolby_dd_ddp;		//1-dd, 2-dd+
	char dolby_dualmono;	//0-non dualmono; 1-dualmono
	char dolby_bsid;		//flag of bsid
	char dolby_acmod;		//flag of acmod
	char dolby_lfeon;		//flag of lfeon
	
	char reserved[9];
} MT_HA_AUDIO_STREAM_INFO_S;

/**Output structure of the HA decoder.*/
/**CNcomment:HA 解码器输出结构体*/
typedef struct mtMT_HADECODE_OUTPUT_S
{
    ulong *ps32PcmOutBuf;       /**<Input, pointer to the decoded PCM data.
                                     Note: 1) ps32PcmOutBuf must be 32-word aligned.
                                           2) ps32PcmOutBuf is NULL when the decoder is working in HD_DEC_MODE_THRU mode.\n*/
                                 /**<CNcomment:IN 指向PCM 解码输出的指针. */

    mt_u32  u32PcmOutBufSize;    /**<Input, size (in byte) of the buffer for storing the decoded PCM data
                                     Note: u32PcmOutBufSize is 0 when the decoder is working in HD_DEC_MODE_THRU mode.\n*/
                                 /**<CNcomment:IN  PCM 解码输出缓冲区大小. unit:Byte */

    ulong *ps32BitsOutBuf;      /**<Input, pointer to the decoded passthrough data.
                                     Note: 1) ps32BitsOutBuf must be 32-word aligned.
                                           2) ps32BitsOutBuf is NULL when the decoder is working in HD_DEC_MODE_RAWPCM mode.\n*/
                                 /**<CNcomment:IN 指向透传解码输出的指针. */

    mt_u32  u32BitsOutBufSize;   /**<Input, size (in byte) of the buffer for storing the decoded passthrough data (IEC61937)
                                     Note: u32BitsOutBufSize is 0 when the decoder is working in HD_DEC_MODE_RAWPCM mode.\n*/
                                 /**<CNcomment:IN 透传(IEC61937) 解码输出缓冲区大小. unit:Byte */

    mt_u32  u32PcmOutSamplesPerFrame; /**<Output, number of output sampling points after the PCM data is decoded.*/ /**<CNcomment:OUT. 解码Pcm 输出样点数 */
    MT_BOOL bInterleaved;             /**<Output, interleaved mode for PCM decoding.*/ /**<CNcomment:OUT PCM  解码交织模式*/
    mt_u32  u32BitPerSample;          /**<Output, bit per sampling.*/ /**<CNcomment:OUT 位宽*/
    mt_u32  u32BitsOutBytesPerFrame;  /**<Output, size (in byte) of the passthrough frame.*/ /**<CNcomment:OUT 透传输出帧大小. unit: Byte*/
    mt_u32  u32OutChannels;           /**<Output, number of output channels.*/ /**<CNcomment:OUT 输出声道数*/
    mt_u32  u32OutChannelsExist;  
    mt_u32  u32OutSampleRate;         /**<Output, output sampling rate.*/ /**<CNcomment:OUT. 输出采样频率*/
    mt_u32  u32OrgChannels;           /**<Output, number of original channels.*/ /**<CNcomment:OUT 码流原始声道数*/
    mt_u32  u32OrgSampleRate;         /**<Output, original sampling rate.*/ /**<CNcomment:OUT 码流原始采样频率*/
    mt_u32  u32BitRate;               /**<Output, bit rate (in bit/s) of the compressed streams.*/ /**<CNcomment:OUT 码流压缩率unit: bit/S*/
    MT_HA_OUTPTSINFO_S stPtsInfo;     /**<Output, pts information.*/ /**<CNcomment:OUT. pts信息*/
    mt_u32  u32FrameIndex;   /**<Output, output frame index.*/ /**<CNcomment:OUT. 输出帧序号*/
    MT_BOOL  b_eos;  
    MT_BOOL bEac4TimeSampleRate;
	MT_HA_AUDIO_STREAM_INFO_S stAudInfo; /**Output, audio stream information**/
} MT_HADECODE_OUTPUT_S;

#ifdef HA_HW_CODEC_SUPPORT
/**Hardware decode input buffer attribute.*/
/**CNcomment:硬解输入缓存属性*/
typedef struct mtMT_HA_INPUTBUF_S
{
    mt_u32   u32BufAddr;             /**<Output, buffer addr*/ /**<CNcomment: 缓存地址*/
    mt_u32   u32BufSize;                /**<Input, buffer size*/ /**<CNcomment: 缓存大小*/
    mt_u32   u32PtsBoundary;            /**<Input, pts read pointers wrap*/ /**<CNcomment:pts读指针边界 */
} MT_HA_INPUTBUF_S;

/**Hardware decode input buffer process attribute.*/
/**CNcomment:硬解输入缓存处理属性*/
typedef struct mtMT_HA_INBUF_PROCESS_S
{
    mt_u32   u32BufOffsetAddr1;             /**<Output, buffer offset addr 1*/ /**<CNcomment: 缓存偏移地址1*/
    mt_u32   u32BufOffsetAddr2;             /**<Output, buffer offset addr 2*/ /**<CNcomment: 缓存偏移地址2*/
    mt_u32   u32BufRequestSize1;            /**<Output, buffer requeset size 1*/ /**<CNcomment:缓存大小1*/
    mt_u32   u32BufRequestSize2;            /**<Output, buffer requeset size 2*/ /**<CNcomment: 缓存大小2*/
    mt_u32   u32RequestSize;                /**<Input, requeset buffer size*/ /**<CNcomment: 请求的缓存大小*/
} MT_HA_INBUF_PROCESS_S;

/**Hardware decode output buffer attribute.*/
/**CNcomment:硬解输出缓存属性*/
typedef struct mtMT_HA_OUTPUTBUF_S
{
    mt_u32   u32BufAddr;              /**<Output, buffer addr*/ /**<CNcomment: 缓存地址*/
    mt_u32   u32BufSize;              /**<Output, buffer size*/ /**<CNcomment: 缓存大小*/
    mt_u32   u32MaxBufSizePerFrame;      /**<Input, max frame size*/ /**<CNcomment: 每帧的缓存大小*/
    mt_u32   u32OutFrameNum;             /**<Input, max frame number*/ /**<CNcomment: 总帧数*/
} MT_HA_OUTPUTBUF_S;

/**Hardware decode status information.*/
/**CNcomment:硬解状态信息*/
typedef struct mtMT_HA_STATUS_INFO_S
{
    mt_u32      u32InBufReadPos;   /**<Output, input buffer read position*/ /**<CNcomment: 输入缓存读位置*/
    mt_u32      u32InBufWritePos;  /**<Output, input buffer write position*/ /**<CNcomment: 输入缓存写位置*/

    mt_u32      u32OutBufReadIdx;  /**<Output, output buffer read index, 0~N-1, N is output buffer total frame number*/ 
                                   /**<CNcomment: 输出缓存读位置下标索引，0~N-1，N为输出缓存总帧数*/
    mt_u32      u32OutBufReadWrap; /**<Output, output buffer read wrap, 0 or 1*/ /**<CNcomment: 输出缓存读下标回绕标志，0或者1*/
    mt_u32      u32OutBufWriteIdx; /**<Output, output buffer write index, 0~N-1, N is output buffer total frame number*/ 
                                   /**<CNcomment: 输出缓存写位置下标索引，0~N-1，N为输出缓存总帧数*/
    mt_u32      u32OutBufWriteWrap;/**<Output, output buffer write wrap, 0 or 1*/ /**<CNcomment: 输出缓存写下标回绕标志，0或者1*/

    mt_u32      u32TryExeCnt;      /**<Output, try decode count*/ /**<CNcomment: 尝试解码次数*/
    mt_u32      u32FrameNum;       /**<Output, decode correct frame count*/ /**<CNcomment: 解码出正确帧的个数*/
    mt_u32      u32ErrFrameNum;    /**<Output, decode incorrect frame count*/ /**<CNcomment: 解码出错误帧的个数*/
    MT_BOOL     bEndOfFrame;       /**<Output, the last frame or not.*/ /**<CNcomment:是否最后一帧*/

    mt_u32      u32ExeTimeOutCnt;  /**<Output, execute timeout count.*/ /**<CNcomment:执行超时次数*/
    mt_u32      u32ScheTimeOutCnt; /**<Output, schedule timeout count.*/ /**<CNcomment:调度超时次数*/

    mt_u32      u32StreamReadPos;  /**<Output, stream virtual read position based on boundary.*/ /**<CNcomment:基于boundary的码流读位置*/
} MT_HA_STATUS_INFO_S;
#endif

/**Definition of the HA decoder.*/
/**CNcomment:HA 解码器设备定义*/
typedef struct mtHA_DECODE_S
{
    const mt_pchar szName;/**<Input, description information about a decoder, such as MP3 or AAC. */
                          /**<CNcomment:IN 解码器描述名字，如MP3,AAC*/

    const mt_u32 enCodecID;/**<Input, decoder ID.Note: This ID is the identifier of a decoder, and must be unique. */
                           /**<CNcomment:IN 解码器标识，注意: 此标识作为解码器唯一身份标识，必须唯一而不能有重复*/

    const MT_HAAUDIO_VERSIONTYPE_U uVersion;/**<Input, decoder version.*/ /**<CNcomment:IN 解码器版本描述*/

    const mt_pchar pszDescription;/**<Input, detailed information about a decoder.*/ /**<CNcomment:IN 解码器详细描述信息*/



    struct mtHA_DECODE_S *pstNext;/**<Output, pointer to the next decoder.This member variable is maintained by the client that calls the HA codec,
                                      Therefore, the developers of the HA decoder can ignore this member variable. \n*/
                                  /**<CNcomment:OUT  指向下个解码器设备的指针.  该成员变量由调用HA Codec 的客户端维护, HA 解码器开发者可以忽略该成员*/

    mt_void  * pDllModule;/**<Output, pointer to the dll symbol of an HA decoder,This member variable is maintained by the client that calls the HA codec,
                              Therefore, the developers of the HA decoder can ignore this member variable.\n*/
                          /**<CNcomment:OUT 该成员变量由调用HA Codec 的客户端维护, HA 解码器开发者可以忽略该成员*/

    /**
    \brief Initializes a decoder. CNcomment:初始化解码器 CNend
    \attention \n
    \param[in] pstOpenParam pointer of the open params CNcomment:open参数结构体指针 CNend
    \param[out] phDecoder   pointer of the decoder handle CNcomment:解码器句柄指针 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::MT_HA_ERRORTYPE_E FAILURE CNcomment:失败 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*DecInit)(mt_void * *phDecoder,
                                 const MT_HADECODE_OPENPARAM_S * pstOpenParam);

    /**
    \brief DeInitializes a decoder. CNcomment:去初始化解码器 CNend
    \attention \n
    \param[in] hDecoder   pointer of the decoder handle CNcomment:解码器句柄指针 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::MT_HA_ERRORTYPE_E FAILURE CNcomment:失败 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*DecDeInit)(mt_void* hDecoder);

    /**
    \brief Configure a decoder dynamically. You can call this API when a decoder works.
    CNcomment:解码器动态配置方法，用户可以在解码器运行时调用该接口 CNend
    \attention \n
    \param[in] hDecoder   the decoder handle CNcomment:解码器句柄 CNend
    \param[in] pstConfigStructure pointer to application allocated structure to be used for initialization by the decoder CNcomment:解码器初始化结构体 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::MT_HA_ERRORTYPE_E FAILURE CNcomment:失败 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*DecSetConfig)(mt_void* hDecoder, mt_void* pstConfigStructure);

    /**
    \brief Maximum size of the buffer required for storing the PCM data decoded by the decoder,
    The memory needs to be allocated based on the size on the client.
    CNcomment:解码器PCM输出所需最大缓冲区size. 客户端需要根据该方法分配内存 CNend
    \attention \n
    \param[in] hDecoder   the decoder handle CNcomment:解码器句柄 CNend
    \param[out] pu32OutSizes pointer to the max size of the pcm audio frame. unit:Byte CNcomment:最大PCM输出缓冲区大小 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::MT_HA_ERRORTYPE_E FAILURE CNcomment:失败 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*DecGetMaxPcmOutSize)(mt_void* hDecoder,
            mt_u32* pu32OutSizes);

    /**
    \brief Maximum size of the buffer required for storing the passthrough data decoded by the decoder,
    The memory needs to be allocated based on the size on the client.
    CNcomment:解码器透传输出所需最大缓冲区size.客户端需要根据该方法分配内存 CNend
    \attention \n
    \param[in] hDecoder   the decoder handle CNcomment:解码器句柄 CNend
    \param[out] pu32OutSizes pointer to the max size of the iec61937 audio frame. unit:Byte CNcomment:最大透传输出缓冲区大小 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::MT_HA_ERRORTYPE_E FAILURE CNcomment:失败 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*DecGetMaxBitsOutSize)(mt_void* hDecoder,
            mt_u32* pu32OutSizes);

    /**
    \brief This method is used to decode a frame. CNcomment:解码器解码一帧方法 CNend
    \attention \n
    \param[in] hDecoder   the decoder handle CNcomment:解码器句柄 CNend
    \param[in] pstApkt pointer to audio stream packet CNcomment:音频输入流指针 CNend
    \param[out] pstAOut pointer to audio output CNcomment:音频输出流指针 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::MT_HA_ERRORTYPE_E FAILURE CNcomment:失败 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*DecDecodeFrame)(mt_void* hDecoder,
                                        MT_HADECODE_INPACKET_S* pstApkt,
                                        MT_HADECODE_OUTPUT_S* pstAOut);

#ifdef HA_HW_CODEC_SUPPORT

    /**
    \brief Enable or disable hardware decode. CNcomment:启动或者停止硬解码 CNend
    \attention \n
    \param[in] hDecoder   the decoder handle CNcomment:解码器句柄 CNend
    \param[in] bEnable    enable or not CNcomment:启动或者停止 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::AEF_ErrorInvalidParameter invalid parameter CNcomment:无效参数 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*HwSetEnable)(mt_void* hDecoder, MT_BOOL bEnable);

    /**
    \brief Initialize input buffer of hardware decode. CNcomment:硬解码输入缓存初始化 CNend
    \attention \n
    \param[in] hDecoder   the decoder handle CNcomment:解码器句柄 CNend
    \param[in] pstInBufAttr    input buffer attribute CNcomment:输入缓存属性 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::AEF_ErrorInvalidParameter invalid parameter CNcomment:无效参数 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*HwInbufInit)(mt_void* hDecoder,
                                     MT_HA_INPUTBUF_S* pstInBufAttr);

    /**
    \brief DeInitializes input buffer of hardware decode. CNcomment:硬解码输入缓存去初始化 CNend
    \attention \n
    \param[in] hDecoder   the decoder handle CNcomment:解码器句柄 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::AEF_ErrorInvalidParameter invalid parameter CNcomment:无效参数 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*HwInBufDeInit)(mt_void* hDecoder);

    /**
    \brief Initialize output buffer of hardware decode. CNcomment:硬解码输出缓存初始化 CNend
    \attention \n
    \param[in] hDecoder   the decoder handle CNcomment:解码器句柄 CNend
    \param[in] pstOutBufAttr    output buffer attribute CNcomment:输出缓存属性 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::AEF_ErrorInvalidParameter invalid parameter CNcomment:无效参数 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*HwOutbufInit)(mt_void* hDecoder,
                                      MT_HA_OUTPUTBUF_S* pstOutBufAttr);

    /**
    \brief Deinitialize output buffer of hardware decode. CNcomment:硬解码输出缓存去初始化 CNend
    \attention \n
    \param[in] hDecoder   the decoder handle CNcomment:解码器句柄 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::AEF_ErrorInvalidParameter invalid parameter CNcomment:无效参数 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*HwOutBufDeInit)(mt_void* hDecoder);

    /**
    \brief Get the free space address of hardware decode input buffer. CNcomment:获取硬解码输入缓存空闲空间地址 CNend
    \attention \n
    \param[in] hDecoder   the decoder handle CNcomment:解码器句柄 CNend
    \param[out] pstGetBuf free space address and size of input buffer CNcomment:输入缓存空闲空间地址和大小 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::AEF_ErrorInvalidParameter invalid parameter CNcomment:无效参数 CNend
    \retval ::HA_ErrorInBufFull input buffer is full CNcomment:输入缓存满 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*HwGetBuf)(mt_void* hDecoder,
                                  MT_HA_INBUF_PROCESS_S* pstGetBuf);

    /**
    \brief Update the read pointer of hardware decode input buffer. CNcomment:更新硬解码输入缓存读指针 CNend
    \attention \n
    \param[in] hDecoder   the decoder handle CNcomment:解码器句柄 CNend
    \param[out] pstPutBuf data size put input buffer CNcomment:填入输入缓存的数据大小 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::AEF_ErrorInvalidParameter invalid parameter CNcomment:无效参数 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*HwPutBuf)(mt_void* hDecoder,
                                  MT_HA_INBUF_PROCESS_S* pstPutBuf);

    /**
    \brief Receive a frame after hardware decode. CNcomment:获取硬解码后的一帧 CNend
    \attention \n
    \param[in] hDecoder   the decoder handle CNcomment:解码器句柄 CNend
    \param[out] pstAOut   a frame struction CNcomment:一帧数据 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::AEF_ErrorInvalidParameter invalid parameter CNcomment:无效参数 CNend
    \retval ::HA_ErrorOutBufEmpty output buffer is empty CNcomment:输出缓存空 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*HwReceiveFrame)(mt_void* hDecoder,
                                        MT_HADECODE_OUTPUT_S* pstAOut);

    /**
    \brief release a frame. CNcomment:释放一帧数据 CNend
    \attention \n
    \param[in] hDecoder   the decoder handle CNcomment:解码器句柄 CNend
    \param[in] u32FrameIndex   frame index CNcomment:帧索引 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::AEF_ErrorInvalidParameter invalid parameter CNcomment:无效参数 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*HwReleaseFrame)(mt_void* hDecoder,
                                        mt_u32 u32FrameIndex);

    /**
    \brief Set file end flag of hardware decode. CNcomment:设置硬解码文件结束标志 CNend
    \attention \n
    \param[in] hDecoder   the decoder handle CNcomment:解码器句柄 CNend
    \param[in] bEosOfFlag   file end flag CNcomment:文件结束标志 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::AEF_ErrorInvalidParameter invalid parameter CNcomment:无效参数 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*HwSetEosFlag)(mt_void* hDecoder,
                                        MT_BOOL bEosOfFlag);

    /**
    \brief Get status information of hardware decode. CNcomment:设置硬解码的状态信息 CNend
    \attention \n
    \param[in] hDecoder   the decoder handle CNcomment:解码器句柄 CNend
    \param[out] pstStatusInfo   status information CNcomment:状态信息 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::AEF_ErrorInvalidParameter invalid parameter CNcomment:无效参数 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*HwGetStatusInfo)(mt_void* hDecoder,
                                         MT_HA_STATUS_INFO_S* pstStatusInfo);
#endif

} MT_HA_DECODE_S;

/**Parameters for creating an HA encoder.*/
/**CNcomment:HA 编码器创建配置参数*/
typedef struct mtHAENCODE_OPENPARAM_S
{
    mt_u32      u32DesiredOutChannels; /**<IN Number of channels (e.g 2 for stereo).*/ /**<CNcomment:声道数*/
    MT_BOOL     bInterleaved;       /**<IN interleave or not.*/ /**<CNcomment:交织模式*/
    mt_s32      s32BitPerSample;    /**<IN bitwidth per sample.*/ /**<CNcomment:位宽*/
    mt_u32      u32DesiredSampleRate; /**<IN desired samplerate.*/ /**<CNcomment:期望采样率*/
    mt_u32      u32SamplePerFrame;  /**<IN Pcm samples per frame for encoder.*/ /**<CNcomment:每帧采样点*/
    mt_void*    pCodecPrivateData;  /**<IN Pointer of decoder private open parameters note: if there is none private parameters, pCodecPrivateData=0.*/
                                    /**<CNcomment:私有参数*/
    mt_u32      u32CodecPrivateDataSize;/**<IN Size of decoder private open parameters note: if there is none private parameters, u32CodecPrivateDataSize=0.*/
                                        /**<CNcomment:私有参数大小*/
} MT_HAENCODE_OPENPARAM_S;

/**HA input pcm  packet struct.*/
/**CNcomment:packet模式结构体*/
typedef struct mtHAENCODE_INPACKET_S
{
    mt_u32  u32PtsMs;       /**<IN  PTS (unit:MS) Note: HA encoder should discard this infomation.*/
                            /**<CNcomment:PTS，单位:毫秒*/
    mt_u8*  pu8Data;        /**<IN/OUT pointer to input auduo pcm data note: HA encoder would update pu8Data after encode.*/
                            /**<CNcomment:输入数据指针*/
    mt_u32  u32Size;        /**<IN/OUT size of the input auduo data. unit: Byte.*/ /**<CNcomment:输入数据大小*/
} MT_HAENCODE_INPACKET_S;

/**HA encoder output struct.*/
/**CNcomment:编码器输出结构体*/
typedef struct mtHAENCODE_OUTPUT_S
{
    mt_s32* ps32BitsOutBuf;         /**<IN the pointer to encoded bitstream output buffer note: ps32BitsOutBuf must  be word32-aligned.*/
                                    /**<CNcomment:编码数据输出buffer*/
    mt_u32  u32BitsOutBufSize;      /**<IN the buffer size of bitstream output buffer. unit:Byte.*/ /**<CNcomment:编码器输出buffer大小*/
    mt_u32  u32BitsOutBytesPerFrame;/**<IN size of the encoded audio data frame ,unit: Byte.*/ /**<CNcomment:编码数据帧大小，单位:Byte*/
    mt_u32  u32BitRate;             /**<IN compress bit rate of the audio stream.*/ /**<CNcomment:比特率*/
#ifdef HA_HW_CODEC_SUPPORT
    mt_u32  u32PtsReadPos;          /**<Hardware encode pts read position .*/ /**<CNcomment:硬编码pts读位置*/
#endif
} MT_HAENCODE_OUTPUT_S;

/**HA encoder struct define.*/
/**CNcomment:定义编码器结构体*/
typedef struct mtHA_ENCODE_S
{
    const mt_pchar szName;/**<Input, description information about audio encoder. */
    /**<CNcomment:IN 编码器描述名字*/

    const mt_u32 enCodecID;/**<Input, encoder ID.Note: This ID is the identifier of a encoder, and must be unique. */
    /**<CNcomment:IN 编码器标识，注意: 此标识作为编码器唯一身份标识，必须唯一而不能有重复*/

    const MT_HAAUDIO_VERSIONTYPE_U uVersion;/**<Input, encoder version.*/ /**<CNcomment:IN 编码器版本描述*/

    const mt_pchar pszDescription;/**<Input, Description infomation of the audio encoder. */ /**<CNcomment:IN 编码器详细描述信息*/

    struct mtHA_ENCODE_S* pstNext;/**<OUT pointer to next HA enocder.manager by client.client:The layer of software that invokes the methods of the HA encoder. */
    /**<CNcomment:OUT 下个编码器结构体指针，由客户管理 */

    mt_void*   pDllModule;/**<Output, pointer to the dll symbol of an HA encoder.This member variable is maintained by the client that calls the HA codec
                              Therefore, the developers of the HA encoder can ignore this member variable. \n*/
    /**<CNcomment:OUT 该成员变量由调用HA Codec 的客户端维护, HA 编码器开发者可以忽略该成员*/

    /**
    \brief Initializes a encoder. CNcomment:初始化编码器 CNend
    \attention \n
    \param[in] pstOpenParam pointer of the open params CNcomment:open参数结构体指针 CNend
    \param[out] phEncoder   pointer of the encoder handle CNcomment:编码器句柄指针 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::MT_HA_ERRORTYPE_E FAILURE CNcomment:失败 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*EncodeInit)(mt_void * *phEncoder,
                                    const MT_HAENCODE_OPENPARAM_S* pstOpenParam);

    /**
    \brief DeInitializes a encoder. CNcomment:去初始化编码器 CNend
    \attention \n
    \param[in] hEncoder   pointer of the encoder handle CNcomment:编码器句柄 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::MT_HA_ERRORTYPE_E FAILURE CNcomment:失败 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*EncodeDeInit)(mt_void* hEncoder);

    /**
    \brief Configure a encoder dynamically. It can be invoked anytime after the Encoder has been loaded.
    CNcomment:编码器动态配置方法。用户可以在编码器运行时调用该接口 CNend
    \attention \n
    \param[in] hEncoder   the encoder handle CNcomment:编码器句柄 CNend
    \param[in] pstConfigStructure pointer to application allocated structure to be used for initialization by the encoder CNcomment:编码器初始化结构体 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::MT_HA_ERRORTYPE_E FAILURE CNcomment:失败 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*EncodeSetConfig)(mt_void* hEncoder, mt_void* pstConfigStructure);

    /**
    \brief Maximum output size of an encoded bitstream CNcomment:编码器输出所需最大缓冲区size CNend
    \attention \n
    \param[in] hEncoder   the decoder handle CNcomment:解码器句柄 CNend
    \param[out] pu32OutSizes pointer to the max size of the encoded audio frame. unit:Byte CNcomment:最大编码帧输出缓冲区大小 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::MT_HA_ERRORTYPE_E FAILURE CNcomment:失败 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*EncodeGetMaxBitsOutSize)(mt_void* hEncoder,
            mt_u32* pu32OutSizes);

    /**
    \brief This method is used to encode a frame. CNcomment:编码器编码一帧方法 CNend
    \attention \n
    \param[in] hEncoder   the decoder handle CNcomment:编码器句柄 CNend
    \param[in] pstApkt pointer to audio stream packet CNcomment:音频输入流指针 CNend
    \param[out] pstAOut pointer to audio output CNcomment:音频输出流指针 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::MT_HA_ERRORTYPE_E FAILURE CNcomment:失败 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*EncodeFrame)(mt_void* hEncoder,
                                     MT_HAENCODE_INPACKET_S* pstApkt,
                                     MT_HAENCODE_OUTPUT_S* pstAOut);
#ifdef HA_HW_CODEC_SUPPORT

    /**
    \brief Enable or disable hardware encode. CNcomment:启动或者停止硬解码 CNend
    \attention \n
    \param[in] hEncoder   the encode handle CNcomment:编码器句柄 CNend
    \param[in] bEnable    enable or not CNcomment:启动或者停止 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::AEF_ErrorInvalidParameter invalid parameter CNcomment:无效参数 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*EncodeHwSetEnable)(mt_void* hEncoder, MT_BOOL bEnable);

    /**
    \brief Initialize input buffer of hardware encode. CNcomment:硬编码输入缓存初始化 CNend
    \attention \n
    \param[in] hEncoder   the encode handle CNcomment:编码器句柄 CNend
    \param[in] pstInBufAttr    input buffer attribute CNcomment:输入缓存属性 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::AEF_ErrorInvalidParameter invalid parameter CNcomment:无效参数 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*EncodeHwInitInbuf)(mt_void* hEncoder,
                                           MT_HA_INPUTBUF_S* pstInBufAttr);

    /**
    \brief DeInitializes input buffer of hardware encode. CNcomment:硬编码输入缓存去初始化 CNend
    \attention \n
    \param[in] hEncoder   the encode handle CNcomment:编码器句柄 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::AEF_ErrorInvalidParameter invalid parameter CNcomment:无效参数 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*EncodeHwDeInitInbuf)(mt_void* hEncoder, mt_u32 u32BufAddr);

    /**
    \brief Initialize output buffer of hardware encode. CNcomment:硬编码输出缓存初始化 CNend
    \attention \n
    \param[in] hEncoder   the encode handle CNcomment:编码器句柄 CNend
    \param[in] pstOutBufAttr    output buffer attribute CNcomment:输出缓存属性 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::AEF_ErrorInvalidParameter invalid parameter CNcomment:无效参数 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*EncodeHwInitOutbuf)(mt_void* hEncoder,
                                            MT_HA_OUTPUTBUF_S* pstOutBufAttr);

    /**
    \brief Deinitialize output buffer of hardware encode. CNcomment:硬编码输出缓存去初始化 CNend
    \attention \n
    \param[in] hEncoder   the encode handle CNcomment:编码器句柄 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::AEF_ErrorInvalidParameter invalid parameter CNcomment:无效参数 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*EncodeHwDeInitOutbuf)(mt_void* hEncoder, mt_u32 u32BufAddr);

    /**
    \brief Get the free space address of hardware encode input buffer. CNcomment:获取硬编码输入缓存空闲空间地址 CNend
    \attention \n
    \param[in] hEncoder   the encode handle CNcomment:编码器句柄 CNend
    \param[out] pstGetBuf free space address and size of input buffer CNcomment:输入缓存空闲空间地址和大小 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::AEF_ErrorInvalidParameter invalid parameter CNcomment:无效参数 CNend
    \retval ::HA_ErrorInBufFull input buffer is full CNcomment:输入缓存满 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*EncodeHwGetBuf)(mt_void* hEncoder,
                                        MT_HA_INBUF_PROCESS_S* pstGetBuf);

    /**
    \brief Update the read pointer of hardware encode input buffer. CNcomment:更新硬编码输入缓存读指针 CNend
    \attention \n
    \param[in] hEncoder   the encode handle CNcomment:编码器句柄 CNend
    \param[out] pstPutBuf data size put input buffer CNcomment:填入输入缓存的数据大小 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::AEF_ErrorInvalidParameter invalid parameter CNcomment:无效参数 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*EncodeHwPutBuf)(mt_void* hEncoder,
                                        MT_HA_INBUF_PROCESS_S* pstPutBuf);

    /**
    \brief Receive a frame after hardware encode. CNcomment:获取硬编码后的一帧 CNend
    \attention \n
    \param[in] hEncoder   the encode handle CNcomment:编码器句柄 CNend
    \param[out] pstAOut   a frame struction CNcomment:一帧编码数据 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::AEF_ErrorInvalidParameter invalid parameter CNcomment:无效参数 CNend
    \retval ::HA_ErrorOutBufEmpty output buffer is empty CNcomment:输出缓存空 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*EncodeHwReceiveStream)(mt_void* hEncoder,
                                        MT_HAENCODE_OUTPUT_S* pstAOut);

    /**
    \brief release a frame. CNcomment:释放一帧编码数据 CNend
    \attention \n
    \param[in] hEncoder   the encode handle CNcomment:编码器句柄 CNend
    \retval ::HA_ErrorNone  SUCCESS CNcomment:成功 CNend
    \retval ::AEF_ErrorInvalidParameter invalid parameter CNcomment:无效参数 CNend
    \see \n
    N/A
    */
    MT_HA_ERRORTYPE_E (*EncodeHwReleaseStream)(mt_void* hEncoder);
#endif
} MT_HA_ENCODE_S;

/** @} */  /** <!-- ==== Structure Definition end ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* #ifndef __MT_AUDIO_CODEC_H__ */
