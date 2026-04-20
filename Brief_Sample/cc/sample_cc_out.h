/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __SAMPLE_CC_OUT_H__
#define __SAMPLE_CC_OUT_H__

#include "mt_unf_cc.h"
#ifdef __cplusplus
extern "C"{
#endif

mt_s32 CC_Output_Init(mt_handle* phOut);

mt_s32 CC_Output_DeInit(mt_handle hOut);

#ifdef CONFIG_MT_CHIP_SYMPHONY4
mt_s32 CC_Output_OnDraw(mt_u32 u32UserData, MT_UNF_CC_DISPLAY_PARAM_S *pstDisplayParam);
mt_s32 CC_Output_GetTextSize(mt_u32 u32Userdata, mt_u16 *u8Str, mt_s32 StrNum, mt_s32 *width, mt_s32 *heigth);
mt_s32 CC_Output_Blit(mt_u32 u32UserPrivatData, MT_UNF_CC_RECT_S *SrcRect, MT_UNF_CC_RECT_S *DestRect);
mt_s32 CC_Output_VBIOutput(mt_u32 u32UserData, MT_UNF_CC_VBI_DADA_S *pstVBIData);
#elif defined CONFIG_MT_CHIP_SYMPHONY6
mt_s32 CC_Output_OnDraw(ulong u32UserData, MT_UNF_CC_DISPLAY_PARAM_S *pstDisplayParam);
mt_s32 CC_Output_GetTextSize(ulong u32Userdata, mt_u16 *u8Str, mt_s32 StrNum, mt_s32 *width, mt_s32 *heigth);
mt_s32 CC_Output_Blit(ulong u32UserPrivatData, MT_UNF_CC_RECT_S *SrcRect, MT_UNF_CC_RECT_S *DestRect);
mt_s32 CC_Output_VBIOutput(ulong u32UserData, MT_UNF_CC_VBI_DADA_S *pstVBIData);
#endif



#ifdef __cplusplus
}
#endif

#endif

