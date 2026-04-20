/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 File Name     : drv_descrambler_func.h
 Version       : Initial Draft
 Author        : Montage multimedia software group
 Created       : 2013/04/16
 Description   :
******************************************************************************/

#ifndef __DRV_DESCRAMBLER_FUNC_H__
#define __DRV_DESCRAMBLER_FUNC_H__

#include "mt_unf_descrambler.h"

#include "drv_demux_define.h"

#ifdef __cplusplus
extern "C" {
#endif

mt_void DescramblerReset(mt_u32 KeyId, DMX_KeyInfo_S *KeyInfo);
mt_void DescInitHardFlag(mt_void);
mt_void DmxDescramblerResume(mt_void);

mt_s32  DMX_OsiDescramblerCreate(mt_u32 *KeyId, const MT_UNF_DMX_DESCRAMBLER_ATTR_S *DescAttr);
mt_s32  DMX_OsiDescramblerDestroy(mt_u32 KeyId);
mt_s32  DMX_OsiDescramblerGetAttr(mt_u32 KeyId, MT_UNF_DMX_DESCRAMBLER_ATTR_S *pstDesramblerAttr);
mt_s32  DMX_OsiDescramblerSetAttr(mt_u32 KeyId, MT_UNF_DMX_DESCRAMBLER_ATTR_S *pstDesramblerAttr);
mt_s32  DMX_OsiDescramblerSetKeySlot(mt_u32 KeyId, mt_u32 KeyType, mt_u8 KeySlot);
mt_s32  DMX_OsiDescramblerSetKey(mt_u32 KeyId, mt_u32 KeyType, mt_u8 *Key);
mt_s32  DMX_OsiDescramblerSetIVKey(mt_u32 KeyId, mt_u32 KeyType, mt_u8 *Key);
mt_s32  DMX_OsiDescramblerAttach(mt_u32 KeyId, mt_u32 ChanId);
mt_s32  DMX_OsiDescramblerDetach(mt_u32 KeyId, mt_u32 ChanId);
mt_s32  DMX_OsiDescramblerGetFreeKeyNum(mt_u32 *FreeCount);
mt_s32  DMX_OsiDescramblerGetKeyId(mt_u32 ChanId, mt_u32 *KeyId);

#ifdef MT_DEMUX_PROC_SUPPORT
DMX_KeyInfo_S* DMX_OsiGetKeyProc(mt_u32 KeyId);
#endif

#ifdef __cplusplus
}
#endif

#endif  // __DRV_DESCRAMBLER_FUNC_H__

