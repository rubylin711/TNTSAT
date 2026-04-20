/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_HAL_AOE_COMMON_H__
#define __MT_HAL_AOE_COMMON_H__

#include "mt_type.h"
//#include "mt_audsp_aoe.h"

#define AOE_MAX_AIP_NUM    8    
#define AOE_MAX_AOP_NUM    8    
#define AOE_MAX_ENGINE_NUM  6    


/* AOE AIP Definition */
typedef enum
{
    AOE_AIP0  = 0x00,
    AOE_AIP1  = 0x01,
    AOE_AIP2  = 0x02,
    AOE_AIP3  = 0x03,
    AOE_AIP4  = 0x04,
    AOE_AIP5  = 0x05,
    AOE_AIP6  = 0x06,
    AOE_AIP7  = 0x07,

    AOE_AIP_BUTT = AOE_MAX_AIP_NUM,
} AOE_AIP_ID_E;

/* AOE AOP Definition */
typedef enum
{
    AOE_AOP0 = 0x00,
    AOE_AOP1 = 0x01,
    AOE_AOP2 = 0x02,
    AOE_AOP3 = 0x03,
    AOE_AOP4 = 0x04,
    AOE_AOP5 = 0x05,
    AOE_AOP6 = 0x06,
    AOE_AOP7 = 0x07,

    AOE_AOP_BUTT = AOE_MAX_AOP_NUM,
} AOE_AOP_ID_E;

/* AOE ENGINE Definition */
typedef enum
{
    AOE_ENGINE0 = 0x00,
    AOE_ENGINE1 = 0x01,
    AOE_ENGINE2 = 0x02,
    AOE_ENGINE3 = 0x03,
    AOE_ENGINE4 = 0x04,
    AOE_ENGINE5 = 0x05,

    AOE_ENGINE_BUTT = AOE_MAX_ENGINE_NUM,
} AOE_ENGINE_ID_E;

/**Defines the  status of an AIP.*/
typedef enum
{
    AOE_AIP_STATUS_STOP = 0, /**<Stop*//**<CNcomment: 停止 */
    AOE_AIP_STATUS_START, /**<Start*//**<CNcomment: 运行 */
    AOE_AIP_STATUS_PAUSE,

    AIP_STATUS_BUTT
} AOE_AIP_STATUS_E;

/**Defines the  status of an AOP.*/
typedef enum
{
    AOE_AOP_STATUS_STOP = 0, /**<Stop*//**<CNcomment: 停止 */
    AOE_AOP_STATUS_START, /**<Start*//**<CNcomment: 运行 */

    AOE_AOP_STATUS_BUTT
} AOE_AOP_STATUS_E;

/**Defines the  status of an Engine.*/
typedef enum
{
    AOE_ENGINE_STATUS_STOP = 0, /**<Stop*//**<CNcomment: 停止 */
    AOE_ENGINE_STATUS_START, /**<Start*//**<CNcomment: 运行 */

    AOE_ENGINE_STATUS_BUTT
} AOE_ENGINE_STATUS_E;

typedef struct
{
    phys_addr_t  u32BufPhyAddr;  // hw aoe
    phys_addr_t  u32BufPhyWptr;  // hw aoe
    phys_addr_t  u32BufPhyRptr;  // hw aoe 
    ulong   u32BufVirAddr;  // sw aoe
    ulong   u32BufVirWptr;  // sw aoe
    ulong   u32BufVirRptr;  // sw aoe
    mt_u32  u32BufWptrRptrFlag;  /* 0: u32BufWptr & u32BufRptr located at AIP Reg, else: */
    mt_u32  u32BufSize;
    MT_BOOL b_spdif_mode;
} AOE_RBUF_ATTR_S;

typedef struct
{
    AOE_RBUF_ATTR_S stRbfAttr;
    mt_u32 u32BufBitPerSample; /**<I/O, bit per sampling*//**<CNcomment:OUT. Bit per sample */
    mt_u32 u32BufChannels; /**<I/O, number of channels*//**<CNcomment:OUT. 输出声道数  */
    mt_u32 u32BufSampleRate; /**<I/O, sampling rate*//**<CNcomment:OUT. 输出采样频率 */
    mt_u32 u32BufDataFormat;          /**<I/O, 0, linear pcm, 1, iec61937 */
    mt_u32 u32BufLatencyThdMs;   /* 40 ~ 1000 ms */
    mt_u32  u32FadeinMs;
    mt_u32  u32FadeoutMs;
    MT_BOOL bFadeEnable;
    MT_BOOL bAlsaEnable;  
    MT_BOOL bMixPriority;  /* TRUE: high priority */

} AOE_AIP_INBUF_ATTR_S;

typedef struct
{
    mt_u32 u32FifoBitPerSample; /**<I/O, bit per sampling*/ /**<CNcomment:OUT. Bit per sample */
    mt_u32 u32FifoChannels; /**<I/O, number of channels*/ /**<CNcomment:OUT. 输出声道数  */
    mt_u32 u32FifoSampleRate; /**<I/O, sampling rate*/ /**<CNcomment:OUT. 输出采样频率 */
    mt_u32 u32FifoDataFormat;               /**<I/O, 0, linear pcm, 1, iec61937 */
    mt_u32 u32FiFoLatencyThdMs; /* 10 ~ 40 ms */
} AOE_AIP_OUTFIFO_ATTR_S;

typedef struct
{
    ulong  u32StartVirAddr;
    phys_addr_t u32StartPhyAddr;
    mt_u32  u32Size;
    //MT_BOOL b_spdif_mode;
    mt_u32  CBType;
} AOE_AIP_CHN_ATTR_NEW_S;

typedef struct
{
    AOE_AIP_INBUF_ATTR_S   stBufInAttr;   
    AOE_AIP_OUTFIFO_ATTR_S stFifoOutAttr;
    mt_u32 sound_type;
} AOE_AIP_CHN_ATTR_S;

typedef struct
{
    AOE_RBUF_ATTR_S stRbfAttr;
    mt_u32  u32BufBitPerSample; /**<I/O, bit per sampling*//**<CNcomment:OUT. Bit per sample */
    mt_u32  u32BufChannels; /**<I/O, number of channels*//**<CNcomment:OUT. 输出声道数  */
    mt_u32  u32BufSampleRate; /**<I/O, sampling rate*//**<CNcomment:OUT. 输出采样频率 */
    mt_u32  u32BufDataFormat;          /**<I/O, 0, linear pcm, 1, iec61937 */
    mt_u32  u32BufLatencyThdMs;        /* 10 ~ 40 ms */
    MT_BOOL bRbfHwPriority; /* TRUE: high priority */
} AOE_AOP_OUTBUF_ATTR_S;


typedef struct
{
    AOE_AOP_OUTBUF_ATTR_S stRbfOutAttr;
} AOE_AOP_CHN_ATTR_S;

typedef struct
{
    mt_u32 u32BitPerSample; /**<I/O, bit per sampling*/ /**<CNcomment:OUT. Bit per sample */
    mt_u32 u32Channels; /**<I/O, number of channels*/ /**<CNcomment:OUT. 输出声道数  */
    mt_u32 u32SampleRate; /**<I/O, sampling rate*/ /**<CNcomment:OUT. 输出采样频率 */
    mt_u32 u32DataFormat;               /**<I/O, 0, linear pcm, 1, iec61937 */
} AOE_ENGINE_CHN_ATTR_S;

#endif  // __MT_HAL_AOE_COMMON_H__
