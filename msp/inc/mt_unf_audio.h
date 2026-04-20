/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_UNF_AUDIO_H__
#define __MT_UNF_AUDIO_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "mt_common.h"
#include "mt_audio_codec.h"

/*************************** Structure Definition ****************************/
/** \addtogroup      Audio_Common */
/** @{ */  /** <!-- [Audio_Common] */


/**CNcomment: 定义音频采样率枚举*/
typedef enum mtUNF_SAMPLE_RATE_E
{
    MT_UNF_SAMPLE_RATE_UNKNOWN=0,       /**<Unknown*/ /**<CNcomment: 未知采样频率 */
    MT_UNF_SAMPLE_RATE_8K    = 8000,    /**<8 kHz sampling rate*/ /**<CNcomment: 8K采样频率 */
    MT_UNF_SAMPLE_RATE_11K   = 11025,   /**<11.025 kHz sampling rate*/ /**<CNcomment: 11.025K采样频率 */
    MT_UNF_SAMPLE_RATE_12K   = 12000,   /**<12 kHz sampling rate*/ /**<CNcomment: 12K采样频率 */
    MT_UNF_SAMPLE_RATE_16K   = 16000,   /**<16 kHz sampling rate*/ /**<CNcomment: 16K采样频率 */
    MT_UNF_SAMPLE_RATE_22K   = 22050,   /**<22.050 kHz sampling rate*/ /**<CNcomment: 22.050K采样频率 */
    MT_UNF_SAMPLE_RATE_24K   = 24000,   /**<24 kHz sampling rate*/ /**<CNcomment: 24K采样频率 */
    MT_UNF_SAMPLE_RATE_32K   = 32000,   /**<32 kHz sampling rate*/ /**<CNcomment: 32K采样频率 */
    MT_UNF_SAMPLE_RATE_44K   = 44100,   /**<44.1 kHz sampling rate*/ /**<CNcomment: 44.1K采样频率 */
    MT_UNF_SAMPLE_RATE_48K   = 48000,   /**<48 kHz sampling rate*/ /**<CNcomment: 48K采样频率 */
    MT_UNF_SAMPLE_RATE_64K= 64000,      /**<64 kHz sampling rate*/ /**<CNcomment: 64K采样频率 */
    MT_UNF_SAMPLE_RATE_88K   = 88200,   /**<88.2 kHz sampling rate*/ /**<CNcomment: 88.2K采样频率 */
    MT_UNF_SAMPLE_RATE_96K   = 96000,   /**<96 kHz sampling rate*/ /**<CNcomment: 96K采样频率 */
    MT_UNF_SAMPLE_RATE_128K= 128000,	/**<128 kHz sampling rate*/ /**<CNcomment: 128K采样频率 */
    MT_UNF_SAMPLE_RATE_176K= 176400,      /**<176 kHz sampling rate*/ /**<CNcomment: 176K采样频率 */
    MT_UNF_SAMPLE_RATE_192K  = 192000,  /**<192 kHz sampling rate*/ /**<CNcomment: 192K采样频率 */

    MT_UNF_SAMPLE_RATE_BUTT
}MT_UNF_SAMPLE_RATE_E;

/**Defines the bit depth during audio sampling.*/
/**CNcomment: 定义音频采样位宽枚举*/
typedef enum mtUNF_BIT_DEPTH_E
{
    MT_UNF_BIT_DEPTH_UNKNOWN =0,/**<Unknown*/ /**<CNcomment: 未知采样位宽  */
    MT_UNF_BIT_DEPTH_8  = 8,    /**< 8-bit depth*/ /**<CNcomment: 8位采样位宽  */
    MT_UNF_BIT_DEPTH_16 = 16,   /**<16-bit depth*/ /**<CNcomment: 16位采样位宽 */
    MT_UNF_BIT_DEPTH_18 = 18,   /**<18-bit depth*/ /**<CNcomment: 18位采样位宽 */
    MT_UNF_BIT_DEPTH_20 = 20,   /**<20-bit depth*/ /**<CNcomment: 20位采样位宽 */
    MT_UNF_BIT_DEPTH_24 = 24,   /**<24-bit depth*/ /**<CNcomment: 24位采样位宽 */
    MT_UNF_BIT_DEPTH_32 = 32,   /**<32-bit depth*/ /**<CNcomment: 32位采样位宽 */

    MT_UNF_BIT_DEPTH_BUTT
}MT_UNF_BIT_DEPTH_E;

/**Defines the mode of audio channels.*/
/**CNcomment: 定义音频声道模式枚举*/
typedef enum mtUNF_TRACK_MODE_E
{
    MT_UNF_TRACK_MODE_STEREO = 0,          /**<Stereo*/ /**<CNcomment: 立体声*/
    MT_UNF_TRACK_MODE_DOUBLE_MONO,         /**<Data is output after being mixed in the audio-left channel and audio-right channel.*/ /**<CNcomment: 左右声道混合后输出*/
    MT_UNF_TRACK_MODE_DOUBLE_LEFT,         /**<The audio-left channel and audio-right channel output the data of the audio-left channel.*/ /**<CNcomment: 左右声道输出左声道数据*/
    MT_UNF_TRACK_MODE_DOUBLE_RIGHT,        /**<The audio-left channel and audio-right channel output the data of the audio-right channel.*/ /**<CNcomment: 左右声道输出右声道数据*/
    MT_UNF_TRACK_MODE_EXCHANGE,            /**<Data is output after being exchanged in the audio-left channel and audio-right channel.*/ /**<CNcomment: 左右声道数据交换输出 */
    MT_UNF_TRACK_MODE_ONLY_RIGHT,          /**<Only the data in the audio-right channel is output.*/ /**<CNcomment: 只输出右声道数据*/
    MT_UNF_TRACK_MODE_ONLY_LEFT,           /**<Only the data in the audio-left channel is output.*/ /**<CNcomment: 只输出左声道数据*/
    MT_UNF_TRACK_MODE_MUTED,               /**<Mute*/ /**<CNcomment: 静音*/

    MT_UNF_TRACK_MODE_BUTT
} MT_UNF_TRACK_MODE_E;

/**Defines the attributes of the audio decoder.*/
/**CNcomment: 定义音频解码器属性结构*/
typedef struct mtUNF_ACODEC_ATTR_S{
    HA_CODEC_ID_E        enType;                    /**<Audio decoder type*/ /**<CNcomment: 音频解码类型*/
    MT_HADECODE_OPENPARAM_S stDecodeParam;   /**<Audio decoder parameter*/ /**<CNcomment: 音频解码参数*/
}MT_UNF_ACODEC_ATTR_S;

/**Defines the attributes of audio encoder.*/
/**CNcomment: 定义音频编码器属性结构*/
typedef struct mtUNF_AENC_ATTR_S
{
    mt_u32         enAencType;              /**<Audio encoder type*/ /**<CNcomment: 音频编码类型*/
    MT_HAENCODE_OPENPARAM_S   sOpenParam;   /**<Audio encoder parameter*/ /**<CNcomment: 音频编码参数*/
}MT_UNF_AENC_ATTR_S;

/**Defines the information about audio streams.*/
/**CNcomment: 定义音频码流信息结构*/
typedef struct mtUNF_ACODEC_STREAMINFO_S
{
    mt_u32   enACodecType;                 /**<Audio encoding type*/ /**<CNcomment: 音频编码类型*/
    mt_u32               enSampleRate;     /**<Audio sampling rate*/ /**<CNcomment: 音频采样率*/
    MT_UNF_BIT_DEPTH_E   enBitDepth;       /**<Bit depth during audio sampling*/ /**<CNcomment: 音频采样位宽*/
	mt_u32               u32Channel;       /**<Audio output channel*//**<CNcomment: 音频输出声道数*/
}MT_UNF_ACODEC_STREAMINFO_S;

/**Defines the I2S(Inter-IC Sound) mode.*/
/**CNcomment: 定义音频I2S接口模式枚举*/
typedef enum mtMT_UNF_I2S_MODE_E
{
    MT_UNF_I2S_STD_MODE = 0,     /**<I2S standard mode*/ /**<CNcomment: I2S标准模式*/
    MT_UNF_I2S_PCM_MODE,         /**<pcm mode*/ /**<CNcomment: PCM模式*/
    MT_UNF_I2S_MODE_BUTT
} MT_UNF_I2S_MODE_E;

/**Defines the I2S(Inter-IC Sound) master clock.*/
/**CNcomment: 定义音频I2S接口工作时钟枚举*/
typedef enum mtMT_UNF_I2S_MCLK_SEL_E
{
    MT_UNF_I2S_MCLK_128_FS = 0,      /* mclk 128*fs  */
    MT_UNF_I2S_MCLK_256_FS,          /* mclk 256*fs */
    MT_UNF_I2S_MCLK_384_FS,          /* mclk 384*fs */
    MT_UNF_I2S_MCLK_512_FS,          /* mclk 512*fs */
    MT_UNF_I2S_MCLK_768_FS,          /* mclk 768*fs */
    MT_UNF_I2S_MCLK_1024_FS,         /* mclk 1024*fs */
    MT_UNF_I2S_MCLK_BUTT
} MT_UNF_I2S_MCLK_SEL_E;

/**Defines the I2S(Inter-IC Sound) bclk clock.*/
typedef enum mtMT_UNF_I2S_BCLK_SEL_E
{
     MT_UNF_I2S_BCLK_1_DIV = 1,           /* bclk=mclk/1  */
     MT_UNF_I2S_BCLK_2_DIV = 2,           /* bclk=mclk/2  */
     MT_UNF_I2S_BCLK_3_DIV = 3,           /* bclk=mclk/3  */
     MT_UNF_I2S_BCLK_4_DIV = 4,           /* bclk=mclk/4  */
     MT_UNF_I2S_BCLK_6_DIV = 6,           /* bclk=mclk/6  */
     MT_UNF_I2S_BCLK_8_DIV = 8 ,          /* bclk=mclk/8  */
     MT_UNF_I2S_BCLK_12_DIV= 12,          /* bclk=mclk/12 */
     MT_UNF_I2S_BCLK_24_DIV= 24,          /* bclk=mclk/24 */
     MT_UNF_I2S_BCLK_32_DIV= 32,          /* bclk=mclk/32 */
     MT_UNF_I2S_BCLK_48_DIV= 48,          /* bclk=mclk/48 */
     MT_UNF_I2S_BCLK_64_DIV= 64,          /* bclk=mclk/64 */
     MT_UNF_I2S_BCLK_BUTT
} MT_UNF_I2S_BCLK_SEL_E;

/**Defines the I2S(Inter-IC Sound) channels.*/
/**CNcomment: 定义音频I2S接口音频通道枚举*/
typedef enum mtMT_UNF_I2S_CHNUM_E
{
    MT_UNF_I2S_CHNUM_1  = 1,
    MT_UNF_I2S_CHNUM_2  = 2,              /* only work for I2S mode */
    MT_UNF_I2S_CHNUM_8  = 8,              /* only work for I2S mode */
    MT_UNF_I2S_BUTT,
} MT_UNF_I2S_CHNUM_E;

/**Defines the I2S(Inter-IC Sound) bit depth.*/
typedef enum mtMT_UNF_I2S_BITDEPTH_E
{
    MT_UNF_I2S_BIT_DEPTH_16 = 16,
    MT_UNF_I2S_BIT_DEPTH_24 = 24,         /* only work for I2S mode */
} MT_UNF_I2S_BITDEPTH_E;

/**Defines the I2S(Inter-IC Sound) data valid after frame sync clock at PCM mode .*/
/**CNcomment: 定义音频I2S接口，数据有效延迟周期，仅PCM模式有效*/
typedef enum mtMT_UNF_I2S_PCMDELAY_E
{
    MT_UNF_I2S_PCM_0_DELAY = 0,           /* 0  bclk cycles delay*/
    MT_UNF_I2S_PCM_1_DELAY = 1,           /* 1  bclk cycles delay*/
    MT_UNF_I2S_PCM_8_DELAY = 8,           /* 8  bclk cycles delay*/
    MT_UNF_I2S_PCM_16_DELAY = 16,         /* 16 bclk cycles delay*/
    MT_UNF_I2S_PCM_17_DELAY = 17,         /* 17 bclk cycles delay*/
    MT_UNF_I2S_PCM_24_DELAY = 24,         /* 24 bclk cycles delay*/
    MT_UNF_I2S_PCM_32_DELAY = 32,         /* 32 bclk cycles delay*/
    MT_UNF_I2S_PCM_DELAY_BUTT
} MT_UNF_I2S_PCMDELAY_E;


/**Defines the I2S(Inter-IC Sound) attribute .*/
/**CNcomment: 定义音频I2S接口配置属性*/
typedef struct mtMT_UNF_I2S_ATTR_S
{
    MT_BOOL               bMaster;           /* MT_TRUE:master, MT_FALSE: slave */
    MT_UNF_I2S_MODE_E     enI2sMode;
    MT_UNF_I2S_MCLK_SEL_E enMclkSel;
    MT_UNF_I2S_BCLK_SEL_E enBclkSel;
    MT_UNF_I2S_CHNUM_E    enChannel;
    MT_UNF_I2S_BITDEPTH_E enBitDepth;
    MT_BOOL               bPcmSampleRiseEdge; /* only work for PCM mode. MT_TRUE:sample data at positive edge of bclk, MT_FALSE: sample data at  negative edge of bclk */
    MT_UNF_I2S_PCMDELAY_E enPcmDelayCycle;    /* only work for PCM mode.  */
} MT_UNF_I2S_ATTR_S;

/**Defines the information about audio frames.*/
/**CNcomment: 定义音频帧信息结构*/
typedef struct mtUNF_AO_FRAMEINFO_S
{
    /** s32BitPerSample: (PCM) Data depth, and format of storing the output data
          If the data depth is 16 bits, 16-bit word memory is used.
          If the data depth is greater than 16 bits, 32-bit word memory is used, and data is stored as left-aligned data. That is, the valid data is at upper bits.
     */
    /**CNcomment: s32BitPerSample: (PCM) 数据位宽设置. 输出存放格式
          等于16bit:   占用16bit word内存
          大于16bit:   占用32bit word内存, 数据左对齐方式存放(有效数据在高位)
     */
    mt_u64  u64PtsMs;              /**<Presentation time stamp (PTS)*/ /**<CNcomment: 时间戳*/
    mt_s32  s32BitPerSample;       /**<Data depth*/ /**<CNcomment: 数据位宽*/
    MT_BOOL bInterleaved;          /**<Whether the data is interleaved*/ /**<CNcomment: 数据是否交织*/
    mt_u32  u32SampleRate;         /**<Sampling rate*/ /**<CNcomment: 采样率*/
    mt_u32  u32Channels;           /**<Number of channels*/ /**<CNcomment: 通道数量*/
    mt_u32  u32ChannelsExist;
    ulong *ps32PcmBuffer;         /**<Pointer to the buffer for storing the pulse code modulation (PCM) data*/ /**<CNcomment: PCM数据缓冲指针*/
    ulong *ps32BitsBuffer;        /**<Pointer to the buffer for storing the stream data*/ /**<CNcomment: 码流数据缓冲指针*/
    mt_u32  u32PcmSamplesPerFrame; /**<Number of sampling points of the PCM data*/ /**<CNcomment: PCM数据采样点数量*/
    mt_u32  u32BitsBytesPerFrame;  /**<IEC61937 data size*/ /**<CNcomment: IEC61937数据长度*/
    mt_u32  u32FrameIndex;         /**<Frame ID*/ /**<CNcomment: 帧序号 */
	mt_u32  u32IEC61937DataType;      /**<IEC61937 Data Type*/ /**<CNcomment: IEC61937数据类型标识，低8bit为IEC数据类型 */
    mt_u32  u32FrameCounter;
    MT_BOOL b_eos;
    MT_BOOL bEac4TimeSampleRate;
    mt_u32  u32chan;
    mt_u32 adectype;            //adec type
    mt_u32  u32DebugCrc;			/**<PCM CRC*/ /**<CNcomment: PCM数据CRC校验 */
    mt_u32  u32ADFrameIndex;
    mt_u32  u32DecErrCnt;
    mt_u32  u32DropCnt;
	MT_HA_AUDIO_STREAM_INFO_S stAudInfo; /**<AUD INFO, feedback from decoder**/
} MT_UNF_AO_FRAMEINFO_S;

typedef struct mtUNF_AVPLAY_OBJINFO_S
{
	mt_u16 obj_id;
	char obj_name[24];
	mt_u16 interact;
} MT_UNF_AVPLAY_OBJINFO_S;

typedef struct mtUNF_AVPLAY_METAINFO_S
{
	mt_u16 obj_num;
	MT_UNF_AVPLAY_OBJINFO_S obj_info[8];
	mt_u16 complementary_object_num[4];
	mt_u16 complementary_object_id[4][8];
} MT_UNF_AVPLAY_METARINFO_S;


typedef struct mtUNF_AVPLAY_AC4_LANG_S
{    
    char  ac4_1st_lang[4];
    char  ac4_2nd_lang[4];
}MT_UNF_AVPLAY_AC4_LANG_S;

/** @} */  /** <!-- ==== Structure Definition End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_UNF_AUDIO_ H*/
