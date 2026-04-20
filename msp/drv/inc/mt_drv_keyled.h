/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef  __MT_DRV_KEYLED_H__
#define  __MT_DRV_KEYLED_H__

#include "mt_unf_keyled.h"
#include "mt_module_debug.h"

mt_s32 MT_DRV_KEYLED_Init(mt_void);
mt_s32 MT_DRV_KEYLED_DeInit(mt_void);
mt_s32 MT_DRV_KEYLED_SelectType(MT_UNF_KEYLED_TYPE_E enKeyLedType);
mt_s32 MT_DRV_KEY_Open(mt_void);
mt_s32 MT_DRV_KEY_Close(mt_void);
mt_s32 MT_DRV_LED_Open(mt_void);
mt_s32 MT_DRV_LED_Close(mt_void);
mt_s32 MT_DRV_LED_Display(mt_u32 u32CodeValue);
mt_s32 MT_DRV_LED_DisplayTime(MT_UNF_KEYLED_TIME_S stLedTime);
mt_s32 MT_DRV_KEY_GetValue(mt_u32 * pu32PressStatus, mt_u32 * pu32KeyId);
mt_s32 MT_DRV_KEY_IsKeyUp(mt_u32 u32IsKeyUp);
mt_s32 MT_DRV_KEY_IsRepKey(mt_u32 u32IsRepKey);
mt_s32 MT_DRV_KEY_SetBlockTime(mt_u32 u32BlockTimeMs);
mt_s32 MT_DRV_KEY_RepKeyTimeoutVal(mt_u32 u32RepTimeMs);

#endif  /*  __MT_DRV_KEYLED_H__ */
