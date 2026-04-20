/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_DEMUX_H__
#define __DRV_DEMUX_H__

#include "mt_drv_mmz.h"
#include "mt_unf_demux.h"
#include "drv_demux_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/***************************** Macro Definition ******************************/

/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/

#ifdef DMX_USE_ECM
mt_s32  DMX_OsrGetChannelSwFlag(mt_handle hChannel, mt_u32 *pu32SwFlag);
mt_s32  DMX_OsrGetChannelSwBufAddr(mt_handle hChannel, DMX_MMZ_BUF_S* pstSwBuf);
#endif

#ifdef MT_DEMUX_PROC_SUPPORT
/* control function entry by proc file system*/
typedef enum
{
    DMX_DEBUG_CMD_SAVE_ES       = 1,
    DMX_DEBUG_CMD_SAVE_ALLTS,
    DMX_DEBUG_CMD_SAVE_IPTS,
    DMX_DEBUG_CMD_SAVE_DMXTS,
    DMX_DEBUG_CMD_SAVE_VES,
    DMX_DEBUG_CMD_SAVE_AES,
    DMX_DEBUG_CMD_SAVE_REC,
    DMX_DEBUG_CMD_PRINT_INFO,
} DMX_DEBUG_CMD;

typedef enum
{
    DMX_DEBUG_CMD_STOP        = 0,
    DMX_DEBUG_CMD_START       = 1,
} DMX_DEBUG_CMD_CTRl;

typedef struct
{
    mt_u32                  ChanFile[DMX_CHANNEL_CNT]; /* the proccess hannle of channel */
    mt_u32                 u32TsBufProcessHandle[DMX_RAMPORT_CNT]; /* the proccess handle of ts buffer */
    mt_u32                  RecFile[DMX_CNT];
    mt_u32                 u32FilterProcessHandle[DMX_FILTER_CNT]; /* the proccess handle of filter */
    mt_u32                 u32PcrChProcessHandle[DMX_PCR_CHANNEL_CNT]; /* the proccess hanele of pcr channel */
} DMX_DEV_OSR_S;

mt_s32  DMX_OsrDebugCtrl(mt_u32 cmd, DMX_DEBUG_CMD_CTRl cmdctrl, mt_u32 param, char *save_path);

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  // __DRV_DEMUX_H__

