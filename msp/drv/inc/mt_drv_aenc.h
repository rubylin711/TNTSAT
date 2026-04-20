/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MPI_PRIV_AENC_H__
#define __MPI_PRIV_AENC_H__

/* add include here */
#include "mt_mpi_aenc.h"
#include "mt_unf_sound.h"
#include "mt_module.h"
#include "mt_debug.h"
#ifdef __cplusplus
extern "C" {
#endif

/***************************** Macro Definition ******************************/
#define AENC_INSTANCE_MAXNUM 3     /* max encoder instance */

#define AENC_MIN_INPUT_BUFFER_SIZE (1024 * 256)
#define AENC_MAX_INPUT_BUFFER_SIZE (1024 * 512 * 4)
#define AENC_DEFAULT_INPUT_BUFFER_SIZE (1024 * 512)
#define AENC_DEFAULT_OUTBUF_NUM 32

#define AENC_MAX_SRC_FRAC (48000 / 8000)
#define AENC_MAX_CHANNELS 2
#define ANEC_MAX_SMAPLEPERFRAME (2048)
#define AENC_MAX_POSTPROCESS_FRAME (ANEC_MAX_SMAPLEPERFRAME * AENC_MAX_CHANNELS * AENC_MAX_SRC_FRAC)
#define AENC_WORK_BUFFER_NUM 2

typedef enum 
{
    ANEC_SOURCE_AI         = 0,
    ANEC_SOURCE_CAST,
    ANEC_SOURCE_VIRTRACK,
    ANEC_SOURCE_BUTT
} ANEC_SOURCE_TYPE_E;
typedef struct mt_AENC_INFO_ATTACH_S
{
    ANEC_SOURCE_TYPE_E eType;
    mt_handle hSource;
}AENC_INFO_ATTACH_S;

typedef enum
{
    AENC_CMD_CTRL_START = 0,
    AENC_CMD_CTRL_STOP,
    AENC_CMD_CTRL_BUTT
} AENC_CMD_CTRL_E;     


typedef enum
{
    AENC_CMD_PROC_SAVE_PCM = 0,
    AENC_CMD_PROC_SAVE_ES,
    AENC_CMD_PROC_BUTT
} AENC_CMD_SAVE_E;     

typedef struct hiAENC_PROC_ITEM_S
{
    AENC_INFO_ATTACH_S stAttach;
    MT_BOOL bAdecWorkEnable;
    mt_u32  u32CodecID;
    mt_char        szCodecType[32];
    mt_u32  u32SampleRate;
    mt_u32  u32BitWidth;
    mt_u32  u32Channels;

    MT_BOOL bAutoSRC;
    mt_u32  u32EncFrame;
    mt_u32  u32ErrFrame;

    mt_u32 u32InBufSize;
    mt_u32 u32InBufRead;
    mt_u32 u32InBufWrite;

    mt_u32 u32OutFrameNum;
    mt_u32 u32OutFrameRIdx;
    mt_u32 u32OutFrameWIdx;
    mt_u32 u32DbgSendBufCount_Try;
    mt_u32 u32DbgSendBufCount;
    mt_u32 u32DbgReceiveStreamCount_Try;
    mt_u32 u32DbgReceiveStreamCount;
    mt_u32 u32DbgReleaseStreamCount_Try;
    mt_u32 u32DbgReleaseStreamCount;
    mt_u32 u32DbgTryEncodeCount;

    AENC_CMD_CTRL_E    enPcmCtrlState;
    AENC_CMD_CTRL_E    enEsCtrlState;
    mt_u32                  u32SavePcmCnt;
    mt_u32                  u32SaveEsCnt;
    mt_char              filePath[512];           
} AENC_PROC_ITEM_S;

/* use macro to check parameter */
#define  MT_MPI_AENC_RetUserErr(DrvErrCode, aenc_mutex) \
    do                                                  \
    {                                                   \
        mt_s32 retvalerr;                               \
        if (MT_SUCCESS != DrvErrCode)                   \
        {                                               \
            switch (DrvErrCode)                         \
            {                                           \
            case  MT_ERR_AENC_IN_BUF_FULL:           \
            case  MT_ERR_AENC_DEV_NOT_OPEN:          \
            case  MT_ERR_AENC_NULL_PTR:          	 \
            case  MT_ERR_AENC_INVALID_PARA:          \
            case  MT_ERR_AENC_OUT_BUF_FULL:          \
            case  MT_ERR_AENC_INVALID_OUTFRAME:      \
            case  MT_ERR_AENC_DATASIZE_EXCEED:       \
            case  MT_ERR_AENC_OUT_BUF_EMPTY:         \
                retvalerr = DrvErrCode;              \
                break;                               \
            default:                                 \
                retvalerr = MT_FAILURE;              \
                break;                               \
            }                                        \
            if (MT_ERR_AENC_IN_BUF_FULL == retvalerr)          \
                MT_INFO_AENC(" DriverErrorCode =0x%x\n",retvalerr); \
            else if (MT_ERR_AENC_OUT_BUF_EMPTY == retvalerr)   \
                MT_INFO_AENC(" DriverErrorCode =0x%x\n",retvalerr); \
            else                                               \
                MT_ERR_AENC(" DriverErrorCode =0x%x\n",retvalerr);  \
            AENC_UNLOCK(aenc_mutex); \
            return retvalerr;                        \
        }                                            \
    } while (0)

#define  MT_MPI_AENC_RetUserErr2(DrvErrCode, aenc_mutex) \
    do                                                   \
    {                                                    \
   	    MT_ERR_AENC(" DriverErrorCode =0x%x\n",DrvErrCode); \
        AENC_UNLOCK(aenc_mutex); \
        return DrvErrCode; \
    } while (0)			

#define CHECK_AENC_CH_CREATE(hAenc) \
    do                                                  \
    {                                                   \
        if (!g_s32AencInitCnt)  \
        {  \
            MT_ERR_AENC("AENC  device state err: please int aenc init first\n");  \
            return MT_FAILURE;  \
        }  \
        if (hAenc >= AENC_INSTANCE_MAXNUM)             \
        {                                               \
            MT_ERR_AENC(" AENC  device not open handleAenc=%d !\n",  hAenc);          \
            return MT_ERR_AENC_DEV_NOT_OPEN;            \
        }                                                \
        if (MT_FALSE == g_pstAencChan[hAenc]->beAssigned)  \
        {                                               \
            MT_ERR_AENC("AENC  device not open!\n");          \
            return MT_ERR_AENC_DEV_NOT_OPEN;              \
        }                                               \
    } while (0)

#define CHECK_AENC_NULL_PTR(ptr) \
    do                                                  \
    {                                                   \
        if (NULL == ptr)                             \
        {                                               \
            MT_ERR_AENC("invalid NULL poiner!\n");          \
            return MT_ERR_AENC_NULL_PTR;                  \
        }                                               \
    } while (0)

#define CHECK_AENC_OPEN_FORMAT(rate, ch, width, bInterleaved) \
    do                                                  \
    {                                                   \
        if (rate <  MT_UNF_SAMPLE_RATE_16K || rate > MT_UNF_SAMPLE_RATE_48K)    \
        {                                           \
            MT_ERR_AENC("invalid  Pcm Format: HA Encoder only support 16K~48K samplerate \n");   \
            return MT_ERR_AENC_INVALID_PARA;           \
        }         \
        if (MT_FALSE == bInterleaved)    \
        {                                           \
            MT_ERR_AENC("invalid  Pcm Format: HA Encoder only support 16bit-Interleaved format \n");   \
            return MT_ERR_AENC_INVALID_PARA;           \
        }                                           \
        if (16 != width)    \
        {                                           \
            MT_ERR_AENC("invalid  Pcm Format: HA Encoder only support 16bit-Interleaved format \n");   \
            return MT_ERR_AENC_INVALID_PARA;           \
        }                                           \
        if (2 != ch)                  \
        {                                           \
            MT_ERR_AENC("invalid Pcm Format: HA Encoder only support 2 channel\n");   \
            return MT_ERR_AENC_INVALID_PARA;           \
        }                                           \
    } while (0)

#define CHECK_AENC_PCM_SAMPLESIZE(PcmSamplesPerFrame) \
    do                                                  \
    {                                                   \
        if (PcmSamplesPerFrame > ANEC_MAX_SMAPLEPERFRAME)    \
        {                                           \
            MT_ERR_AENC("invalid  AO Pcm Format: Pcm SamplesPerFrame  =%d \n",PcmSamplesPerFrame);   \
            return MT_ERR_AENC_INVALID_PARA;           \
        }                                           \
    } while (0)

#if 0
#define CHECK_AENC_PCM_FORMAT(ch, bInterleaved) \
    do                                                  \
    {                                                   \
        if (MT_FALSE == bInterleaved)    \
        {                                           \
            if (ch > 2)                  \
            {                                           \
                MT_ERR_AENC("invalid  Pcm Format: if none-Interleaved, must sure channel <=2 ! \n");   \
                return MT_ERR_AENC_INVALID_PARA;           \
            }                                           \
        }                                           \
        if (ch > 2)                  \
        {                                           \
            if ((ch != 6) && (ch != 8))                  \
            {                                           \
                MT_ERR_AENC("invalid Pcm Format: HA Encoder  only support 5.1 or 7.1 format channel=%d\n", ch);   \
                return MT_ERR_AENC_INVALID_PARA;           \
            }                                           \
        }                                           \
    } while (0)
#endif 

#define CHECK_AENC_PCM_CHANNEL(ch) \
    do                                                  \
    {                                                   \
        if (ch > 2)                  \
        {                                           \
            MT_ERR_AENC("invalid  pcm channel[%d]: must sure channel <=2 ! \n", ch);   \
            return MT_ERR_AENC_INVALID_PARA;           \
        }                                           \
    } while (0)

#define CHECK_AENC_PCM_SAMPLERATE(rate) \
     do                                                  \
    {                                                   \
        switch (rate)                                \
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
            MT_WARN_AO("invalid samplerate[%d]\n", rate);    \
            return MT_ERR_AO_INVALID_PARA;                        \
            }                                                       \
        } while (0)   

#define CHECK_AENC_PCM_BITWIDTH(bitwidth) \
            do                                                  \
            {                                                   \
                if (16 != bitwidth && 24 != bitwidth)                  \
                {                                           \
                    MT_ERR_AENC("invalid  pcm Bitwidth[%d], must sure 16bit or 24bit\n", bitwidth);   \
                    return MT_ERR_AENC_INVALID_PARA;           \
                }                                           \
            } while (0)

        
/*Define Debug Level For MT_ID_AO                     */
#define MT_FATAL_AENC(fmt...) \
    MT_FATAL_PRINT(MT_ID_AENC, fmt)

#define MT_ERR_AENC(fmt...) \
    MT_ERR_PRINT(MT_ID_AENC, fmt)

#define MT_WARN_AENC(fmt...) \
    MT_WARN_PRINT(MT_ID_AENC, fmt)

#define MT_INFO_AENC(fmt...) \
    MT_INFO_PRINT(MT_ID_AENC, fmt)

/********************** Global Variable declaration **************************/
#define DRV_AENC_DEVICE_NAME "mt_aenc"

/* 'IOC_TYPE_ADEC' means ADEC magic macro */
#define     DRV_AENC_PROC_INIT _IOW(MT_ID_AENC, 0, AENC_PROC_ITEM_S *)
#define     DRV_AENC_PROC_EXIT _IO(MT_ID_AENC, 1)

/******************************* API declaration *****************************/
extern mt_s32 AENC_Close (mt_u32 hAenc);
extern mt_s32 AENC_deInit(mt_void);
extern mt_s32 AENC_Init(const mt_char* pszodecNameTable[]);
extern mt_s32 AENC_Open(mt_handle *phAenc, const MT_UNF_AENC_ATTR_S *pstAencAttr);
extern mt_s32 AENC_Pull(mt_handle hAenc);
extern mt_s32 AENC_ReceiveStream (mt_handle hAenc, AENC_STREAM_S *pstStream, mt_u32 u32TimeoutMs);
extern mt_s32 AENC_ReleaseStream(mt_handle hAenc, const AENC_STREAM_S *pstStream);
extern mt_s32 AENC_Reset(mt_handle hAenc);
extern mt_s32 AENC_SendBuffer (mt_handle hAenc, const MT_UNF_AO_FRAMEINFO_S *pstAOFrame);
extern mt_s32 AENC_SetAutoSRC (mt_handle hAenc, MT_BOOL bEnable);
extern mt_s32 AENC_SetConfigEncoder(mt_handle hAenc, mt_void *pstConfigStructure);
extern mt_s32 AENC_RegisterEncoder(const mt_char *pszCodecDllName);
extern mt_s32 AENC_ShowRegisterEncoder(mt_void);
extern mt_s32 AENC_ResetBuf(mt_handle hAenc, mt_u32 u32BufType);
extern mt_u32 AENC_GetInBufDataSize(mt_handle hAenc);
extern mt_u32 AENC_GetEncodeInDataSize(mt_handle hAenc);
extern mt_s32 AENC_SetEnable(mt_handle hAenc, MT_BOOL bEnable);
extern mt_s32 AENC_AttachInput(mt_handle hAenc, mt_handle hSource);
extern mt_s32 AENC_GetAttachSrc(mt_handle hAenc, mt_handle* hSrc);
extern mt_s32 AENC_DetachInput(mt_handle hAenc);
extern mt_s32 AENC_SetAttr(mt_handle hAenc, const MT_UNF_AENC_ATTR_S *pstAencAttr);
extern mt_s32 AENC_GetAttr(mt_handle hAenc, MT_UNF_AENC_ATTR_S *pstAencAttr);

#ifdef __cplusplus
}
#endif
#endif /* __MPI_PRIV_AENC_H__ */
