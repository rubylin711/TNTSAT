/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 File Name     : mt_drv_descrambler.h
 Version       : Initial Draft
 Author        : Montage multimedia software group
 Created       : 2013/04/16
 Description   :
******************************************************************************/

#ifndef __MT_DRV_DESCRAMBLER_H__
#define __MT_DRV_DESCRAMBLER_H__

#include "mt_type.h"

#include "mt_unf_descrambler.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

mt_s32  MT_DRV_DMX_CreateDescrambler(mt_u32 DmxId, MT_UNF_DMX_DESCRAMBLER_ATTR_S *DesrAttr, mt_handle *KeyHandle, ulong file);
mt_s32 MT_DRV_DMX_CreateDescramblerPro(mt_u32 DmxId, MT_UNF_DMX_DESCRAMBLER_PRO_ATTR_S *DescAttr, mt_handle *KeyHandle, ulong file);

mt_s32  MT_DRV_DMX_DestroyDescrambler(mt_handle KeyHandle);
mt_s32  MT_DRV_DMX_GetDescramblerAttr(mt_handle KeyHandle, MT_UNF_DMX_DESCRAMBLER_ATTR_S *pstDecsramblerAttr);
mt_s32  MT_DRV_DMX_SetDescramblerAttr(mt_handle KeyHandle, MT_UNF_DMX_DESCRAMBLER_ATTR_S *pstDescramblerAttr);
mt_s32 MT_DRV_DMX_SetDescramblerEvenKeySlot(mt_handle KeyHandle, mt_u8 KeySlot);
mt_s32 MT_DRV_DMX_SetDescramblerOddKeySlot(mt_handle KeyHandle, mt_u8 KeySlot);
mt_s32  MT_DRV_DMX_SetDescramblerEvenKey(mt_handle KeyHandle, mt_u8 *Key);
mt_s32  MT_DRV_DMX_SetDescramblerOddKey(mt_handle KeyHandle, mt_u8 *Key);
mt_s32  MT_DRV_DMX_SetDescramblerEvenIVKey(mt_handle KeyHandle, mt_u8 *Key);
mt_s32  MT_DRV_DMX_SetDescramblerOddIVKey(mt_handle KeyHandle, mt_u8 *Key);
mt_s32  MT_DRV_DMX_AttachDescrambler(mt_handle KeyHandle, mt_handle ChanHandle);
mt_s32  MT_DRV_DMX_DetachDescrambler(mt_handle KeyHandle, mt_handle ChanHandle);
mt_s32  MT_DRV_DMX_GetDescramblerKeyHandle(mt_handle ChanHandle, mt_handle *KeyHandle);
mt_s32  MT_DRV_DMX_GetFreeDescramblerKeyCount(mt_u32 DmxId, mt_u32 *FreeCount);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  /* __MT_DRV_DESCRAMBLER_H__ */

