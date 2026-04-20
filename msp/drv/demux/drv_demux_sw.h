/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_DEMUX_SW_H__
#define __DRV_DEMUX_SW_H__

#include "drv_demux_define.h"

#ifdef DMX_USE_ECM

#define DMX_SW_CHNBUF_MINLEN    (32 * 1024)
#define DMX_SW_CHNBUF_MAXLEN    (2 * 1024 * 1024)
#define DMX_SW_INVALID_CC       (0xff)
#define DMX_SW_OVERFL_PERCENT   (5) //if channel buffer < 5%,do not read data

mt_s32 MT_DMX_SwInit(mt_void);
mt_s32 MT_DMX_SwNewChannel(mt_u32 u32ChannelId);
mt_s32 MT_DMX_SwDestoryChannel(mt_u32 u32ChannelId);
mt_s32 MT_DMX_SwOpenChannel(mt_u32 u32ChannelId);
mt_s32 MT_DMX_SwCloseChannel(mt_u32 u32ChannelId);
mt_s32 MT_DMX_SwReadDataRequest(mt_u32         u32ChId,
                                mt_u32         u32AcqNum,
                                mt_u32 *       pu32AcqedNum,
                                DMX_UserMsg_S* psMsgList,
                                mt_u32         u32TimeOutMs);
mt_s32 MT_DMX_SwPeekDataRequest(mt_u32 u32ChId, mt_u32 u32PeekLen, DMX_UserMsg_S* psMsgList);
mt_s32 MT_DMX_SwReleaseReadData(mt_u32 u32ChId, mt_u32 u32RelNum, DMX_UserMsg_S* psMsgList);
mt_s32 MT_DMX_SwGetChannelDataStatus(mt_u32 u32ChId);
mt_s32 MT_DMX_SwGetChannelSwBufAddr(mt_u32 u32ChId, mmz_buffer_s* pstSwBuf);

#endif

#endif

