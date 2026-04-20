/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __SAMPLE_SUBTITLE_OUT_H__
#define __SAMPLE_SUBTITLE_OUT_H__

#ifdef __cplusplus
extern "C"{
#endif

#ifdef CONFIG_MT_CHIP_SYMPHONY4
mt_s32 Subt_Output_OnClear(mt_u32 u32UserData, mt_void *pArg);
mt_s32 Subt_Output_OnDraw(mt_u32 u32UserData, const MT_UNF_SO_SUBTITLE_INFO_S *pstInfo, mt_void *pArg);
#elif defined CONFIG_MT_CHIP_SYMPHONY6
mt_s32 Subt_Output_OnClear(ulong u32UserData, mt_void *pArg);
mt_s32 Subt_Output_OnDraw(ulong u32UserData, const MT_UNF_SO_SUBTITLE_INFO_S *pstInfo, mt_void *pArg);
#endif

mt_s32 Subt_Output_Init(MT_HANDLE* phOut);

mt_s32 Subt_Output_DeInit(MT_HANDLE hOut);

#ifdef __cplusplus
}
#endif

#endif

