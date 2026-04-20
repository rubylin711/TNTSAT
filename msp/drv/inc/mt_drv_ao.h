/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_DRV_AO_H__
 #define __MT_DRV_AO_H__

#ifdef __cplusplus
#if __cplusplus
 extern "C"{
#endif
#endif /* __cplusplus */

#include "mt_type.h"
#include "mt_debug.h"
#include "mt_unf_sound.h"
#include "mt_unf_avplay.h"

#define  AO_MAX_VIRTUAL_TRACK_NUM  (6)
#define  AO_MAX_REAL_TRACK_NUM     (8)
#define  AO_MAX_TOTAL_TRACK_NUM    AO_MAX_REAL_TRACK_NUM

#define AO_MAX_CAST_NUM (4)

#define AO_MIN_LINEARVOLUME   (0)
#define AO_MAX_LINEARVOLUME   (100)
#define AO_MAX_ABSOLUTEVOLUME (0) /* max 0 dB*/
#define AO_MIN_ABSOLUTEVOLUME (-70) /* min -70 dB*/
#define AO_MAX_ABSOLUTEVOLUMEEXT (18) /* max 18 dB for S5*/
#define AO_MIN_ABSOLUTEVOLUMEEXT (-81) /* min -81 dB for S5*/
#define AO_MAX_ADJUSTSPEED    (100)  //verify

//TO DO
#define AO_PCM_DF_UNSTALL_THD_FRAMENUM 3
#define AO_PCM_MAX_UNSTALL_THD_FRAMENUM 10

#define MT_ID_TRACK 0x00
#define MT_ID_CAST  0x01
#define MT_ID_AEF   0x02
#define AO_TRACK_CHNID_MASK 0xff
#define AO_CAST_CHNID_MASK 0xff
#define AO_AEF_CHNID_MASK 0xff

#define AEF_MAX_INSTANCE_NUM 4

#define AO_TRACK_AIP_START_LATENCYMS 50

#define CHECK_AO_SNDCARD_OPEN(enSound) \
    do                                                         \
    {                                                          \
        CHECK_AO_SNDCARD(enSound);                             \
        if (MT_NULL == s_stAoDrv.astSndEntity[enSound].pCard)     \
        {                                                       \
            MT_WARN_AO(" Invalid snd id %d\n", enSound);        \
            return MT_ERR_AO_SOUND_NOT_OPEN;                       \
        }                                                       \
    } while (0)

#define CHECK_AO_TRACK_ID(Track)                          \
    do {                                                    \
            if((Track & 0xffff0000) != (MT_ID_AO << 16))              \
            {                                               \
                MT_ERR_AO("track(0x%x) is not ao handle!\n", Track);  \
                return MT_ERR_AO_INVALID_PARA;                          \
            }                                               \
            if((Track & 0xff00) != (MT_ID_TRACK << 8))              \
            {                                               \
                MT_ERR_AO("track(0x%x) is not track handle!\n", Track);  \
                return MT_ERR_AO_INVALID_PARA;                          \
            }    \
         } while(0)

#define CHECK_AO_TRACK_OPEN(Track) \
    do                                                         \
    {                                                          \
        CHECK_AO_TRACK(Track);                             \
        if (0 == atomic_read(&s_stAoDrv.astTrackEntity[Track & AO_TRACK_CHNID_MASK].atmUseCnt))   \
        {                                                       \
            MT_WARN_AO(" Invalid track id 0x%x\n", Track);        \
            return MT_ERR_AO_INVALID_PARA;                       \
        }                                                       \
    } while (0)

#define CHECK_AO_AEF_HANDLE(hAef)                          \
    do {                                                    \
            if((hAef & 0xffff0000) != (MT_ID_AO << 16))              \
            {                                               \
                MT_ERR_AO("aef(0x%x) is not ao handle!\n", hAef);  \
                return MT_ERR_AO_INVALID_PARA;                          \
            }                                               \
            if((hAef & 0xff00) != (MT_ID_AEF << 8))              \
            {                                               \
                MT_ERR_AO("aef(0x%x) is not aef handle!\n", hAef);  \
                return MT_ERR_AO_INVALID_PARA;                          \
            }    \
            if((hAef & AO_AEF_CHNID_MASK) >= AEF_MAX_INSTANCE_NUM)     \
            {                                               \
                MT_ERR_AO("invalid aef(0x%x) handle!\n", hAef);  \
                return MT_ERR_AO_INVALID_PARA;                          \
            }    \
         } while(0)

#define CHECK_AO_NULL_PTR(p)                                \
    do {                                                    \
            if(MT_NULL == p)                                \
            {                                               \
                MT_ERR_AO("NULL pointer \n");               \
                return MT_ERR_AO_NULL_PTR;                          \
            }                                               \
         } while(0)

#define CHECK_AO_CREATE(state)                              \
    do                                                      \
    {                                                       \
        if (0 > state)                                      \
        {                                                   \
            MT_WARN_AO("AO  device not open!\n");           \
            return MT_ERR_AO_DEV_NOT_OPEN;                \
        }                                                   \
    } while (0)

#define CHECK_AO_SNDCARD(card)                                  \
    do                                                          \
    {                                                           \
        if (MT_UNF_SND_BUTT <= card)                            \
        {                                                       \
            MT_WARN_AO(" Invalid snd id %d\n", card);           \
            return MT_ERR_AO_INVALID_ID;                       \
        }                                                       \
    } while (0)
/* master & slave only */
#define CHECK_AO_TRACK(track)                                  \
    do                                                          \
    {                                                           \
        if (AO_MAX_TOTAL_TRACK_NUM <= (track & AO_TRACK_CHNID_MASK))                            \
        {                                                       \
            MT_WARN_AO(" Invalid Snd Track 0x%x\n", track);           \
            return MT_ERR_AO_INVALID_PARA;                       \
        }                                                       \
    } while (0)

#define CHECK_AO_CAST(cast)                                  \
            do                                                          \
            {                                                           \
                if (AO_MAX_CAST_NUM <= (cast & AO_CAST_CHNID_MASK))                            \
                {                                                       \
                    MT_WARN_AO(" Invalid Snd Cast 0x%x\n", cast);           \
                    return MT_ERR_AO_INVALID_PARA;                       \
                }                                                       \
            } while (0)

#define CHECK_AO_PORTNUM(num)                                   \
    do                                                          \
    {                                                           \
        if (MT_UNF_SND_OUTPUTPORT_MAX < num)                    \
        {                                                       \
            MT_WARN_AO(" Invalid outport number %d\n", num);       \
            return MT_ERR_AO_INVALID_PARA;                     \
        }                                                       \
    } while (0)

#define CHECK_AO_OUTPORT(port)                                                              \
    do                                                                                      \
    {                                                                                       \
        if ((MT_UNF_SND_OUTPUTPORT_ARC0 < port) && (MT_UNF_SND_OUTPUTPORT_ALL != port))    \
        {                                                                                   \
            MT_WARN_AO(" Invalid outport %d\n", port);                                      \
            return MT_ERR_AO_INVALID_PARA;                                                 \
        }                                                                                   \
    } while (0)

#define CHECK_AO_PORTEXIST(num)                                   \
    do                                                          \
    {                                                           \
        if(0)  /*if (0 >= num)*/                                        \
        {                                                       \
            MT_ERR_AO("Sound dont't attach any port!\n");       \
            return MT_FAILURE;                                  \
        }                                                       \
    } while (0)


#define CHECK_AO_TRACKMODE(mode)                                  \
    do                                                          \
    {                                                           \
        if (MT_UNF_TRACK_MODE_BUTT <= mode)                     \
        {                                                       \
            MT_WARN_AO(" Invalid trackmode %d\n", mode);        \
            return MT_ERR_AO_INVALID_PARA;                       \
        }                                                       \
    } while (0)

#define CHECK_AO_HDMIMODE(mode)                                  \
    do                                                          \
    {                                                           \
        if (MT_UNF_SND_HDMI_MODE_BUTT <= mode)                     \
        {                                                       \
            MT_WARN_AO(" Invalid hdmimode %d\n", mode);        \
            return MT_ERR_AO_INVALID_PARA;                       \
        }                                                       \
    } while (0)

#define CHECK_AO_SPDIFMODE(mode)                                  \
    do                                                          \
    {                                                           \
        if (MT_UNF_SND_SPDIF_MODE_BUTT <= mode)                     \
        {                                                       \
            MT_WARN_AO(" Invalid spdifmode %d\n", mode);        \
            return MT_ERR_AO_INVALID_PARA;                       \
        }                                                       \
    } while (0)


#define CHECK_AO_SPDIFSCMSMODE(scmsmode)                        \
    do                                                          \
    {                                                           \
        if (MT_UNF_SND_SPDIF_SCMSMODE_BUTT <= scmsmode)         \
        {                                                       \
            MT_WARN_AO(" Invalid spdifscmsmode %d\n", scmsmode);    \
            return MT_ERR_AO_INVALID_PARA;                      \
        }                                                       \
    } while (0)

#define CHECK_AO_CATEGORYCODE(categorycode)                           \
    do                                                                \
    {                                                                 \
        switch(categorycode)                                          \
        {                                                             \
        case MT_UNF_SND_SPDIF_CATEGORY_GENERAL:                       \
        case MT_UNF_SND_SPDIF_CATEGORY_BROADCAST_JP:                  \
        case MT_UNF_SND_SPDIF_CATEGORY_BROADCAST_USA:                 \
        case MT_UNF_SND_SPDIF_CATEGORY_BROADCAST_EU:                  \
        case MT_UNF_SND_SPDIF_CATEGORY_PCM_CODEC:                     \
        case MT_UNF_SND_SPDIF_CATEGORY_DIGITAL_SNDSAMPLER:            \
        case MT_UNF_SND_SPDIF_CATEGORY_DIGITAL_MIXER:                 \
        case MT_UNF_SND_SPDIF_CATEGORY_DIGITAL_SNDPROCESSOR:          \
        case MT_UNF_SND_SPDIF_CATEGORY_SRC:                           \
        case MT_UNF_SND_SPDIF_CATEGORY_MD:                            \
        case MT_UNF_SND_SPDIF_CATEGORY_DVD:                           \
        case MT_UNF_SND_SPDIF_CATEGORY_SYNTHESISER:                   \
        case MT_UNF_SND_SPDIF_CATEGORY_MIC:                           \
        case MT_UNF_SND_SPDIF_CATEGORY_DAT:                           \
        case MT_UNF_SND_SPDIF_CATEGORY_DCC:                           \
        case MT_UNF_SND_SPDIF_CATEGORY_VCR:                           \
            break;                                                    \
        default:                                                      \
            MT_WARN_AO("Invalid category code 0x%x\n", categorycode); \
            return MT_ERR_AO_INVALID_PARA;                            \
        }                                                             \
    } while (0)

#define CHECK_AO_FRAME_NOSTANDART_SAMPLERATE(inrate)                   \
	do													\
	{													\
		if(inrate > MT_UNF_SAMPLE_RATE_192K || inrate < MT_UNF_SAMPLE_RATE_8K) 	\
		{																		\
            MT_INFO_AO("don't support this insamplerate(%d)\n", inrate);    	\
            return MT_SUCCESS;                        							\
		}																		\
	 } while (0)



#define CHECK_AO_FRAME_SAMPLERATE(inrate)                   \
    do                                                  \
    {                                                   \
        switch (inrate)                                \
        {                                               \
        case  MT_UNF_SAMPLE_RATE_8K:                    \
        case  MT_UNF_SAMPLE_RATE_11K:                   \
        case  MT_UNF_SAMPLE_RATE_12K:                   \
        case  MT_UNF_SAMPLE_RATE_16K:                   \
        case  MT_UNF_SAMPLE_RATE_22K:                   \
        case  MT_UNF_SAMPLE_RATE_24K:                   \
        case  MT_UNF_SAMPLE_RATE_32K:                   \
        case  MT_UNF_SAMPLE_RATE_44K:                   \
        case  MT_UNF_SAMPLE_RATE_48K:                   \
        case  MT_UNF_SAMPLE_RATE_88K:                   \
        case  MT_UNF_SAMPLE_RATE_96K:                   \
        case  MT_UNF_SAMPLE_RATE_176K:                  \
        case  MT_UNF_SAMPLE_RATE_192K:                  \
            break;                                      \
        default:                                        \
            MT_INFO_AO("don't support this insamplerate(%d)\n", inrate);    \
            return MT_SUCCESS;                        \
        }                                                       \
     } while (0)

#define CHECK_AO_SAMPLERATE(outrate )                   \
    do                                                  \
    {                                                   \
        switch (outrate)                                \
        {                                               \
        case  MT_UNF_SAMPLE_RATE_8K:                    \
        case  MT_UNF_SAMPLE_RATE_11K:                   \
        case  MT_UNF_SAMPLE_RATE_12K:                   \
        case  MT_UNF_SAMPLE_RATE_16K:                   \
        case  MT_UNF_SAMPLE_RATE_22K:                   \
        case  MT_UNF_SAMPLE_RATE_24K:                   \
        case  MT_UNF_SAMPLE_RATE_32K:                   \
        case  MT_UNF_SAMPLE_RATE_44K:                   \
        case  MT_UNF_SAMPLE_RATE_48K:                   \
        case  MT_UNF_SAMPLE_RATE_88K:                   \
        case  MT_UNF_SAMPLE_RATE_96K:                   \
        case  MT_UNF_SAMPLE_RATE_176K:                  \
        case  MT_UNF_SAMPLE_RATE_192K:                  \
            break;                                      \
        default:                                        \
            MT_WARN_AO("invalid sample out rate %d\n", outrate);    \
            return MT_ERR_AO_INVALID_PARA;                        \
            }                                                       \
            } while (0)


#define CHECK_AO_FRAME_BITDEPTH(inbitdepth)                 \
			do												\
			{												\
				switch (inbitdepth) 						\
				{											\
				case  MT_UNF_BIT_DEPTH_32:					\
				case  MT_UNF_BIT_DEPTH_24:					\
				case  MT_UNF_BIT_DEPTH_16:					\
				case  MT_UNF_BIT_DEPTH_8:					\
					break;									\
				default:									\
					MT_INFO_AO("don't support this bit depth(%d)\n", inbitdepth);	\
					return MT_SUCCESS;						\
				}											\
			 } while (0)

#define CHECK_AO_LINEARVOLUME(linvolume)                \
    do                                                  \
    {                                                   \
        if ((linvolume < AO_MIN_LINEARVOLUME) || (linvolume > AO_MAX_LINEARVOLUME))                   \
        {                                               \
            MT_WARN_AO("invalid LinearVolume(%d), Min(%d) Max(%d)\n", linvolume, AO_MIN_LINEARVOLUME, AO_MAX_LINEARVOLUME);   \
            return MT_ERR_AO_INVALID_PARA;            \
        }                                               \
    } while (0)

#define CHECK_AO_ABSLUTEVOLUME(absvolume)               \
    do                                                  \
    {                                                   \
        if ((absvolume < AO_MIN_ABSOLUTEVOLUME) || (absvolume > AO_MAX_ABSOLUTEVOLUME))      \
        {                                               \
            MT_WARN_AO("invalid AbsouluteVolume(%d), min(%d), max(%d)\n", absvolume, AO_MIN_ABSOLUTEVOLUME, AO_MAX_ABSOLUTEVOLUME);   \
            return MT_ERR_AO_INVALID_PARA;            \
        }                                               \
    } while (0)

#define CHECK_AO_ABSLUTEVOLUMEEXT(absvolume)               \
    do                                                  \
    {                                                   \
        if ((absvolume < AO_MIN_ABSOLUTEVOLUMEEXT) || (absvolume > AO_MAX_ABSOLUTEVOLUMEEXT))      \
        {                                               \
            MT_WARN_AO("invalid AbsouluteVolume(%d), min(%d), max(%d)\n", absvolume, AO_MIN_ABSOLUTEVOLUMEEXT, AO_MAX_ABSOLUTEVOLUMEEXT);   \
            return MT_ERR_AO_INVALID_PARA;            \
        }                                               \
    } while (0)

#define   CHECK_AO_SPEEDADJUST(speed)                   \
    do                                                  \
    {                                                   \
        if ((-AO_MAX_ADJUSTSPEED > speed)               \
            || (speed > AO_MAX_ADJUSTSPEED))            \
        {                                               \
            MT_WARN_AO("invalid AO SpeedAdjust(%d) min(%d), max(%d)!\n", speed, -AO_MAX_ADJUSTSPEED, AO_MAX_ADJUSTSPEED); \
            return MT_ERR_AO_INVALID_PARA;            \
        }                                               \
    } while (0)

 /* the type of Adjust Audio */
typedef enum
{
 AO_SND_SPEEDADJUST_SRC,     /**<samplerate convert */
 AO_SND_SPEEDADJUST_PITCH,   /**<Sola speedadjust, reversed */
 AO_SND_SPEEDADJUST_MUTE,    /**<mute */
 AO_SND_SPEEDADJUST_BUTT
} AO_SND_SPEEDADJUST_TYPE_E;

 /* the type of Adjust Audio */
typedef struct
{
    mt_u32                  u32AefId;
    MT_UNF_SND_AEF_TYPE_E   enAefType;
    mt_char                 szName[32];
    mt_char                 szDescription[32];
    MT_BOOL                 bEnable;
} AO_AEF_PROC_ITEM_S;

typedef struct
{
 phys_addr_t  u32BufPhyAddr;
 ulong  u32BufVirAddr;
 mt_u32  u32BufSize;
} AO_BUF_ATTR_S;

/******************* Audio Effect component ID ************************/

#define MT_AEF_GET_AFLTID_CMD 0x100

#define MT_AEF_SET_OUTBUFADDR_CMD 0x1000

typedef struct hiMT_AEF_INPUTBUF_S
{
    mt_u32   u32BufAddr;             /**<Output, buffer addr*/ /**<CNcomment: */
    mt_u32   u32BufSize;                /**<Input, buffer size*/ /**<CNcomment: */
} MT_AEF_INPUTBUF_S;

typedef struct hiMT_AEF_OUTPUTBUF_S
{
    mt_u32   u32BufAddr;              /**<Output, buffer addr*/ /**<CNcomment: */
    mt_u32   u32BufSize;              /**<Output, buffer size*/ /**<CNcomment: */
    mt_u32   u32MaxBufSizePerFrame;      /**<Input, max frame size*/ /**<CNcomment: */
    mt_u32   u32OutFrameNum;             /**<Input, max frame number*/ /**<CNcomment: */
} MT_AEF_OUTPUTBUF_S;

typedef struct hiMT_AFE_INBUF_PROCESS_S
{
    mt_u32   u32BufOffsetAddr1;             /**<Output, buffer offset addr*/ /**<CNcomment: */
    mt_u32   u32BufOffsetAddr2;             /**<Output, buffer offset addr*/ /**<CNcomment: */
    mt_u32   u32BufRequestSize1;            /**<Output, buffer requeset size*/ /**<CNcomment: */
    mt_u32   u32BufRequestSize2;            /**<Output, buffer requeset size*/ /**<CNcomment: */
    mt_u32   u32RequestSize;
} MT_AFE_INBUF_PROCESS_S;

typedef struct hiMT_AEF_OUTPUT_S
{
    mt_u32 u32PcmOutBuf;
    /**mt_s32* ps32PcmOutBuf;       <Input, pointer to the decoded PCM data.
                                     Note: 1) ps32PcmOutBuf must be 32-word aligned.
                                           2) ps32PcmOutBuf is NULL when the decoder is working in HD_DEC_MODE_THRU mode.\n*/
    /**<CNcomment:IN 指向PCM 解码输出的指针. */

    mt_u32  u32PcmOutBufSize;    /**<Input, size (in byte) of the buffer for storing the decoded PCM data
                                     Note: u32PcmOutBufSize is 0 when the decoder is working in HD_DEC_MODE_THRU mode.\n*/
    /**<CNcomment:IN  PCM 解码输出缓冲区大小. unit:Byte */

    mt_u32  u32PcmOutbytesPerFrame; /**<Output, length of output PCM data is processed in byte.*/ /**<CNcomment:OUT. Pcm 输出byte数 */
    mt_u32  u32OutChannels;           /**<Output, number of output channels.*/ /**<CNcomment:OUT 输出声道数*/
    mt_u32  u32OutBitPerSample;
    mt_u32  u32OutSampleRate;         /**<Output, output sampling rate.*/ /**<CNcomment:OUT. 输出采样频率*/
    mt_u32  u32FrameIndex;   /**<Output, output frame index.*/ /**<CNcomment:OUT. 输出帧序号*/
} MT_AEF_OUTPUT_S;

/**Configuration parameters required by an MT audio effect for creating devices.*/
/**CNcomment:MT 音效创建设备的配置参数*/
typedef struct hiAEF_OPENPARAM_S
{
    MT_BOOL  bMaster;  /**<be master aflt or not*/ /**<CNcomment:aflt 的主从模式*/
    mt_void*            pAudEftPrivateData;/**<Input, pointer to private data*/ /**<CNcomment:IN  指向私有配置结构体的指针*/
    mt_u32              u32AudEftPrivateDataSize;/**<Input, size of the private data. */ /**<CNcomment:IN 私有结构体大小*/
} MT_AEF_OPENPARAM_S;

/**Define the error codes of an audio effect.*/
/**CNcomment:定义audio effect 错误码*/
typedef enum hiAEF_ERRORTYPE_E
{
    AEF_ErrorNone = 0,

    AEF_ErrorInsufficientResources = (mt_s32) 0x80002000,/**<The device fails to be created due to insufficient resources.*/ /**<CNcomment:资源不够，创建设备失败 */

    AEF_ErrorInvalidParameter = (mt_s32) 0x80002001, /**<The input parameter is invalid.*/ /**<CNcomment:输入参数非法 */

    AEF_ErrorStreamCorrupt = (mt_s32) 0x80002002, /**<The effect process fails due to incorrect input streams.*/ /**<CNcomment:输入码流错误，音效处理失败 */

    AEF_ErrorNotEnoughData = (mt_s32) 0x80002003,/**<The effect process ends due to insufficient streams.*/ /**<CNcomment:输入码流不够，退出解码 */

    AEF_ErrorMax = 0x9FFFFFFF
} MT_AEF_ERRORTYPE_E;


/******************* Audio Effect Authorize API ************************/
typedef struct hiAEF_AUTHORIZE_S
{
    const mt_pchar szName;/**<Input, description information about an AEF, such as SRS_StudioSound3D or Dolby_Volume. */

    const MT_UNF_SND_AEF_TYPE_E enEffectID;

    const mt_u32 u32Version; /**<Input,  version.*/

    struct hiAEF_AUTHORIZE_S* pstNext;

    /**<CNcomment:隐性客户信息，寄存器可查询，不对外公布 */
    mt_void (*GetAuthKey)(mt_u32 *pu32CustomerAuthKey);               /* 128/256bit key */

    /**<CNcomment:显性客户信息，proc信息可查询 */
    const mt_pchar pszCustomerDescription; /**< detailed information about the user of AEF, less than 64 bytes.*/
} MT_AEF_AUTHORIZE_S;

/******************* Audio Effect component API ************************/
typedef struct
{
    mt_pchar szName;/**<Input, description information about an AEF, such as SRS_StudioSound3D or Dolby_Volume. */

    MT_UNF_SND_AEF_TYPE_E enEffectID;

    mt_u32 u32Version; /**<Input,  version.*/

    const mt_pchar pszCustomerDescription; /**< detailed information about the user of AEF, less than 64 bytes.*/

    MT_AEF_ERRORTYPE_E (*AefCreate)(MT_AEF_AUTHORIZE_S *pstAuthEntry, mt_void *pstAdvAttr, mt_handle *phAef);

    MT_AEF_ERRORTYPE_E (*AefDestroy)(mt_handle hAef);

    MT_AEF_ERRORTYPE_E (*AefSetEnable)(mt_handle hAef, MT_BOOL bEnable);

    MT_AEF_ERRORTYPE_E (*AefGetEnable)(mt_handle hAef, MT_BOOL * pbEnable);

    MT_AEF_ERRORTYPE_E (*AefSetConfig)(mt_handle hAef, mt_u32 u32ConfigIndex, const mt_void *pConfigStructure);

    MT_AEF_ERRORTYPE_E (*AefGetConfig)(mt_handle hAef, mt_u32 u32ConfigIndex, mt_void *pConfigStructure);

    MT_AEF_ERRORTYPE_E (*AefSetParameter)(mt_handle hAef, mt_u32 u32ParamIndex, const mt_void *pParameterStructure);

    MT_AEF_ERRORTYPE_E (*AefGetParameter)(mt_handle hAef, mt_u32 u32ParamIndex, mt_void *pParameterStructure);

    MT_AEF_ERRORTYPE_E (*AefGetMaxPcmInSize)(mt_handle hAef, mt_u32 *pu32InSizes);

    MT_AEF_ERRORTYPE_E (*AefGetMaxPcmOutSize)(mt_handle hAef, mt_u32 *pu32OutSizes);

    MT_AEF_ERRORTYPE_E (*AefInbufInit)(mt_handle hAef, MT_AEF_INPUTBUF_S* pstInBufAttr);

    MT_AEF_ERRORTYPE_E (*AefInBufDeInit)(mt_handle hAef);

    MT_AEF_ERRORTYPE_E (*AefOutbufInit)(mt_handle hAef, MT_AEF_OUTPUTBUF_S* pstOutBufAttr);

    MT_AEF_ERRORTYPE_E (*AefOutBufDeInit)(mt_handle hAef);

    MT_AEF_ERRORTYPE_E (*AefGetBuf)(mt_handle hAef, MT_AFE_INBUF_PROCESS_S* pstGetBuf);

    MT_AEF_ERRORTYPE_E (*AefPutBuf)(mt_handle hAef, MT_AFE_INBUF_PROCESS_S* pstPutBuf);

    MT_AEF_ERRORTYPE_E (*AefReceiveFrame)(mt_handle hAef, MT_AEF_OUTPUT_S* pstAOut);

    MT_AEF_ERRORTYPE_E (*AefReleaseFrame)(mt_handle hAef, mt_u32 u32FrameIndex);

    MT_AEF_ERRORTYPE_E (*AefSetEosFlag)(mt_handle hAef, MT_BOOL bEosFlag);
} MT_AEF_COMPONENT_S;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

 #endif
