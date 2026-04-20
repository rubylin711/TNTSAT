
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
******************************************************************************
File Name     : drv_disp_isr.h
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2017/01/30
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/
#ifndef __DRV_DISP_ISR_H__
#define __DRV_DISP_ISR_H__

#include "drv_disp_com.h"
#include "drv_disp_hal.h"


#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */


#define DEF_DISP_ISR_LIST_LENGTH 20


typedef struct tagDISP_ISR_C_LIST_S
{
    mt_u32 u32NodeFlag;
    MT_DRV_DISP_CALLBACK_S stNode[DEF_DISP_ISR_LIST_LENGTH];
}DISP_ISR_C_LIST_S;

//typedef mt_void (*PF_DISP_PRE_PROC)(MT_DRV_DISPLAY_E enDisp, const MT_DRV_DISP_CALLBACK_INFO_S *pstInfo);

typedef struct tagDISP_ISR_CHN_S
{
    MT_DRV_DISPLAY_E enDisp;
    volatile MT_BOOL bEnable;

    volatile DISP_ISR_C_LIST_S stList[MT_DRV_DISP_C_TYPE_BUTT];
    MT_DRV_DISP_CALLBACK_INFO_S stCBInfo;

    //volatile MT_BOOL bDispInfoUp;
    //volatile MT_BOOL bDispInfoIsSet;
    //volatile MT_DISP_DISPLAY_INFO_S stDispInfoNew;
}DISP_ISR_CHN_S;

typedef struct tagDISP_ISR_M_S
{
    DISP_ISR_CHN_S stDispChn[MT_DRV_DISPLAY_BUTT+1];
    mt_u32 u32ChnNumber;
    mt_u32 u32IntMaskSave4Suspend;
}DISP_ISR_M_S;

mt_s32 DISP_ISR_Init(mt_void);
mt_s32 DISP_ISR_DeInit(mt_void);

mt_s32 DISP_ISR_Suspend(mt_void);
mt_s32 DISP_ISR_Resume(mt_void);

mt_s32 DISP_ISR_OpenChn(MT_DRV_DISPLAY_E enDisp);
mt_s32 DISP_ISR_CloseChn(MT_DRV_DISPLAY_E enDisp);

mt_s32 DISP_ISR_RegCallback(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CALLBACK_TYPE_E eType,
                            MT_DRV_DISP_CALLBACK_S *pstCB);
mt_s32 DISP_ISR_UnRegCallback(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CALLBACK_TYPE_E eType,
                              MT_DRV_DISP_CALLBACK_S *pstCB);

mt_s32 DISP_ISR_SetEvent(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CALLBACK_EVENT_E eEvent);
mt_s32 DISP_ISR_SetDispInfo(MT_DRV_DISPLAY_E enDisp, MT_DISP_DISPLAY_INFO_S *pstDispInfo);

mt_s32 DISP_ISR_Main(mt_s32 irq, mt_void *dev_id);
irqreturn_t aria_disp_isr_PS_end(mt_s32 irq, mt_void *dev_id);
irqreturn_t aria_disp_isr_group0(mt_s32 irq, mt_void *dev_id);
irqreturn_t aria_disp_isr_group1(mt_s32 irq, mt_void *dev_id);
irqreturn_t aria_disp_isr_top_sd(mt_s32 irq, mt_void *dev_id);
irqreturn_t aria_disp_isr_bot_sd(mt_s32 irq, mt_void *dev_id);
irqreturn_t aria_disp_isr_op_ree(mt_s32 irq, mt_void *dev_id);
irqreturn_t aria_disp_isr_osdc(mt_s32 irq, mt_void *dev_id);

mt_s32 DISP_ISR_Main_VB(mt_void);


#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /*  __DRV_DISP_ISR_H__  */










