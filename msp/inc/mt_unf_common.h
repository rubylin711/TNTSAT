/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_UNF_COMMON_H__
#define __MT_UNF_COMMON_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "mt_common.h"
#include "mt_unf_audio.h"
#include "mt_unf_video.h"

/*************************** Structure Definition ****************************/
/** \addtogroup      Media_Common */
/** @{ */  /** <!-- [Media_Common] */

/**Defines the stream buffer.*/
/**CNcomment: 定义码流缓冲结构体 */
typedef struct mtUNF_STREAM_BUF_S
{
    mt_u8   *pu8Data;        /**<Data pointer*/ /**<CNcomment: 数据指针 */
    mt_u32  u32Size;         /**<Data size*/ /**<CNcomment: 数据长度 */
    phys_addr_t  u32PhyData;
    mt_u8   *pu8Data2;		 /**<Injection is devided into 2 parts if not null --Added by BobbyLin*/
    mt_u32  u32Size2;        /**<Data size part2 if has*/ /**<CNcomment: 数据长度 */
    phys_addr_t  u32PhyData2;
} MT_UNF_STREAM_BUF_S;

/**Defines the structure of the ES buffer.*/
/**CNcomment: 定义ES码流缓冲结构体 */
typedef struct mtUNF_ES_BUF_S
{
    mt_u8 * pu8Buf;    /**<User-state virtual address of the buffer*/ /**<CNcomment: buffer的用户态虚地址*/
    mt_u32 u32BufLen;  /**<Buffer length*/ /**<CNcomment: buffer的长度*/
    mt_u64 u64PtsMs;   /**<Presentation time stamp (PTS) value corresponding to the start of the stream. The invalid value is 0xFFFFFFFF.*/
                       /**<CNcomment: 码流开始处对应的PTS值，无效为0xffffffff*/
	phys_addr_t  es_data_phy_addr;
}MT_UNF_ES_BUF_S;

/* Crop parameter */
typedef struct mtUNF_CROP_RECT_S
{
    mt_u32 u32LeftOffset;
    mt_u32 u32TopOffset;
    mt_u32 u32RightOffset;
    mt_u32 u32BottomOffset;
}MT_UNF_CROP_RECT_S;

/**Defines the signal status of the input source.*/
/**CNcomment: 定义输入源的信号状态 */
typedef enum mtUNF_SIG_STATUS_E
{
    MT_UNF_SIG_SUPPORT = 0,  /**<Stable signal*/            /**<CNcomment:识别稳定信号 */
    MT_UNF_SIG_NO_SIGNAL,    /**<No signal*/                /**<CNcomment:无信号 */
    MT_UNF_SIG_NOT_SUPPORT,  /**<Not support the signal*/   /**<CNcomment:信号不支持 */
    MT_UNF_SIG_UNSTABLE,     /**<Unstable signal*/          /**<CNcomment:信号不稳定 */
    MT_UNF_SIG_BUTT          /**<Invalid value*/            /**<CNcomment:非法边界值 */
} MT_UNF_SIG_STATUS_E;

/** @} */  /** <!-- ==== Structure Definition End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_UNF_COMMON_ H*/
