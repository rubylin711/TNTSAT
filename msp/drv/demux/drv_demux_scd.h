/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __PVR_SCD_H__
#define __PVR_SCD_H__

#include "mt_unf_demux.h"
#include "mt_mpi_demux.h"

#include "drv_demux_index.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* start code type definition(data from SCD buffer) */
#define DMX_INDEX_SC_TYPE_TS         0x1      /* ts packet header */
#define DMX_INDEX_SC_TYPE_PTS        0x2      /* pes packet header */
#define DMX_INDEX_SC_TYPE_PAUSE      0x3      /* pause flag */
#define DMX_INDEX_SC_TYPE_PIC        0x4      /* the start 00 00 01 of frame data */
#define DMX_INDEX_SC_TYPE_PIC_SHORT  0x5      /* the short head 00 01 of frame data */
#define DMX_INDEX_SC_TYPE_PES_ERR    0xf      /* the header of PES syntax error */

mt_void DmxRecUpdateFrameInfo(mt_u32 *Param, FRAME_POS_S *IndexInfo);
mt_s32  DmxScdToVideoIndex(MT_BOOL bUseTimeStamp,const DMX_IDX_DATA_S *ScData, FINDEX_SCD_S *pstFidx);
mt_s32  DmxScdToAudioIndex(MT_UNF_DMX_REC_INDEX_S *LastFrame, const DMX_IDX_DATA_S *ScData);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

