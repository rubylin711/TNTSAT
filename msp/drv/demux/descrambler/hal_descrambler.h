/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 File Name     : hal_descrambler.h
 Version       : Initial Draft
 Author        : Montage multimedia software group
 Created       : 2013/04/16
 Description   :
******************************************************************************/

#ifndef __HAL_DESCRAMBLER_H__
#define __HAL_DESCRAMBLER_H__

#include "drv_demux_define.h"
#include "drv_descrambler.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*add by yuwu*/
mt_void DmxHalDescramblerSetMode(DescModeSetting_t  *DescramblerMode);

mt_void DmxHalDescramblerSetModeForSym6(mt_s32 ch, MT_UNF_DMX_DESCRAMBLER_PRO_ATTR_S  *attr);

mt_void DmxHalSetChannelCWIndex(mt_u32 ChanId, mt_u32 cwIndex);
mt_void DmxHalSetChannelDsc(mt_u32 ChanId, MT_BOOL Enable);

mt_void DmxHalSetEntropyReduction(mt_u32 KeyId, MT_UNF_DMX_CA_ENTROPY_E EntropyReduction);

mt_u32  DmxHalGetOptCAType(mt_void);
mt_u32  DmxHalGetOptDescramblerType(mt_void);
mt_void DmxHalSetCAType(mt_u32 u32KeyId, MT_BOOL bAdvance);
mt_void DmxHalSetDescramblerType(mt_u32 KeyId, MT_BOOL bHigh);
mt_void DmxHalSetCWWord(mt_u32 u32CWId, mt_u32 u32WordId, mt_u32 u32Data, mt_u32 u32EvenOdd);

#ifdef DMX_DESCRAMBLER_VERSION_1
mt_void DmxHalInitSpeCWOrder(mt_void);
mt_void DmxHalInitTdesCWOrder(mt_void);
mt_void DmxHalSetCWWord1(
        mt_u32                          GroupId,
        mt_u32                          WordId,
        mt_u32                          Key,
        DMX_KEY_TYPE_E                  KeyType,
        DMX_CW_TYPE                     CWType,
        MT_UNF_DMX_DESCRAMBLER_TYPE_E   DescType
    );
mt_void DmxHalSetChanCwTabId(mt_u32 ChanId, mt_u32 TabId);
mt_void DmxHalSetDmxIvEnable(mt_u32 DmxId, MT_BOOL Enable);
mt_void DmxHalSetCSA3Reset(MT_BOOL Enable);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  // __HAL_DESCRAMBLER_H__

