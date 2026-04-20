/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_HAL_AIAO_H__
#define __MT_HAL_AIAO_H__

#include "mt_type.h"
#include "hal_aiao_common.h"
#include "aud_out_aria_reg.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/
/* global function */
mt_s32                  HAL_AIAO_Suspend(mt_void);
mt_s32                  HAL_AIAO_Resume(mt_void);
mt_s32                  HAL_AIAO_PowerOn(mt_void);
mt_void                 HAL_AIAO_PowerOff(mt_void);
mt_s32                  HAL_AIAO_RequestIsr(mt_void);
mt_void                 HAL_AIAO_FreeIsr(mt_void);
mt_s32                  HAL_AIAO_Init(mt_void);
mt_void                 HAL_AIAO_DeInit(mt_void);
mt_void                 HAL_AIAO_GetHwCapability(mt_u32 *pu32Capability);
mt_void                 HAL_AIAO_GetHwVersion(mt_u32 *pu32Version);
mt_void                 HAL_AIAO_DBG_RWReg(AIAO_Dbg_Reg_S *pstReg);
mt_void                 HAL_AIAO_SetTopInt(mt_u32 u32Multibit);
mt_u32                  HAL_AIAO_GetTopIntRawStatus(mt_void);
mt_u32                  HAL_AIAO_GetTopIntStatus(mt_void);

/* port function */
mt_s32                  HAL_AIAO_P_Open_Vsb(AIAO_PORT_ID_E enPortID, const AIAO_PORT_USER_CFG_S *pstConfig);
mt_s32                  HAL_AIAO_P_Open(AIAO_PORT_ID_E enPortID, const AIAO_PORT_USER_CFG_S *pstConfig);

mt_void                 HAL_AIAO_P_Close(AIAO_PORT_ID_E enPortID);
mt_s32                  HAL_AIAO_P_Start(AIAO_PORT_ID_E enPortID);
mt_s32                  HAL_AIAO_P_Stop(AIAO_PORT_ID_E enPortID, AIAO_PORT_STOPMODE_E enStopMode);
mt_s32                  HAL_AIAO_P_Mute(AIAO_PORT_ID_E enPortID, MT_BOOL bMute);
mt_s32                  HAL_AIAO_P_SetSampleRate(AIAO_PORT_ID_E enPortID, MT_UNF_SAMPLE_RATE_E enSampleRate);
mt_s32                  HAL_AIAO_P_SetVolume(AIAO_PORT_ID_E enPortID, mt_u32 u32VolumedB);
mt_s32                  HAL_AIAO_P_SetSpdifCategoryCode(AIAO_PORT_ID_E enPortID, AIAO_SPDIF_CATEGORYCODE_E eCategoryCode);
mt_s32                  HAL_AIAO_P_SetSpdifSCMSMode(AIAO_PORT_ID_E enPortID, AIAO_SPDIF_SCMS_MODE_E eSCMSMode);
mt_s32                  HAL_AIAO_P_SetTrackMode(AIAO_PORT_ID_E enPortID, AIAO_TRACK_MODE_E enTrackMode);
mt_s32                               AIAO_HAL_P_SetBypass(AIAO_PORT_ID_E enPortID, MT_BOOL bByBass);
mt_s32                  HAL_AIAO_P_GetUserCongfig(AIAO_PORT_ID_E enPortID, AIAO_PORT_USER_CFG_S *pstUserConfig);
mt_s32                  HAL_AIAO_P_GetStatus(AIAO_PORT_ID_E enPortID, AIAO_PORT_STAUTS_S *pstProcInfo);
mt_s32                  HAL_AIAO_P_SelectSpdifSource(AIAO_PORT_ID_E enPortID, AIAO_SPDIFPORT_SOURCE_E eSrcChnId);
mt_s32                  HAL_AIAO_P_SetSpdifOutPort(AIAO_PORT_ID_E enPortID, mt_s32 bEn);
mt_s32                  HAL_AIAO_P_SetI2SSdSelect(AIAO_PORT_ID_E enPortID, AIAO_I2SDataSel_S  *pstSdSel);
mt_s32                  HAL_AIAO_P_SetAttr(AIAO_PORT_ID_E enPortID, AIAO_PORT_ATTR_S *pstAttr);
mt_s32                  HAL_AIAO_P_GetAttr(AIAO_PORT_ID_E enPortID, AIAO_PORT_ATTR_S *pstAttr);

mt_void                 HAL_AIAO_P_GetTxI2SDfAttr(AIAO_PORT_ID_E enPortID, AIAO_PORT_USER_CFG_S *pAttr);
mt_void HAL_AIAO_P_SetTxI2SDfAttr(AIAO_PORT_ID_E enPortID, AIAO_IsrFunc      *pIsrFunc);//i2s only card set proc func MT_ALSA_I2S_ONLY_SUPPORT
mt_void                 HAL_AIAO_P_GetHdmiHbrDfAttr(AIAO_PORT_ID_E enPortID, AIAO_PORT_USER_CFG_S *pAttr);
mt_void                 HAL_AIAO_P_GetHdmiI2SDfAttr(AIAO_PORT_ID_E enPortID, AIAO_PORT_USER_CFG_S *pAttr);
mt_void                 HAL_AIAO_P_GetTxSpdDfAttr(AIAO_PORT_ID_E enPortID, AIAO_PORT_USER_CFG_S *pAttr);
mt_void                 HAL_AIAO_P_GetRxAdcDfAttr(AIAO_PORT_ID_E enPortID, AIAO_PORT_USER_CFG_S *pAttr);
mt_void                 HAL_AIAO_P_GetRxSifDfAttr(AIAO_PORT_ID_E enPortID, AIAO_PORT_USER_CFG_S *pAttr);
mt_void                 HAL_AIAO_P_GetRxHdmiDfAttr(AIAO_PORT_ID_E enPortID, AIAO_PORT_USER_CFG_S *pAttr);

/* port function for verification*/
mt_s32                  HAL_AIAO_P_Open_Veri(AIAO_PORT_ID_E enPortID, const AIAO_PORT_USER_CFG_S *pstConfig);
mt_void                 HAL_AIAO_P_Close_Veri(AIAO_PORT_ID_E enPortID);

/*
    todo : alsa + unf isr
    AIAO_IsrFunc alsa_unf_isr()
    {
        ... call alsa_isr_func
        ... HAL_AIAO_P_ProcStatistics        
    }
*/
#ifdef MT_ALSA_AI_SUPPORT
mt_u32  HAL_AIAO_P_ALSA_QueryReadPos(AIAO_PORT_ID_E enPortID);
mt_u32  HAL_AIAO_P_ALSA_QueryWritePos(AIAO_PORT_ID_E enPortID);
mt_u32  HAL_AIAO_P_ALSA_FLASH(AIAO_PORT_ID_E enPortID);
mt_u32  HAL_AIAO_P_ALSA_UpdateRptr(AIAO_PORT_ID_E enPortID, mt_u8 * pu32Dest, mt_u32 u32DestSize);
mt_u32  HAL_AIAO_P_ALSA_UpdateWptr(AIAO_PORT_ID_E enPortID, mt_u8 * pu32Dest, mt_u32 u32DestSize);
#endif
mt_void HAL_AIAO_P_ProcStatistics(AIAO_PORT_ID_E enPortID, mt_u32 u32IntStatus,void * pst);

/* port buffer function */
mt_u32 HAL_AIAO_P_ReadData_NotUpRptr(AIAO_PORT_ID_E enPortID, mt_u8 *pu32Dest, mt_u32 u32DestSize, mt_u32 *pu32Rptr, mt_u32 *pu32Wptr);
mt_u32 HAL_AIAO_P_ReadData(AIAO_PORT_ID_E enPortID, mt_u8 *pu32Dest, mt_u32 u32DestSize);
mt_u32 HAL_AIAO_P_ReadData_Vsb(AIAO_PORT_ID_E enPortID, mt_u8 *pu32Dest, mt_u32 u32DestSize);
mt_u32 HAL_AIAO_P_WriteData(AIAO_PORT_ID_E enPortID, mt_u8 *pu32Src, mt_u32 u3SrcLen, mt_u32 ChanExist);
mt_u32 HAL_AIAO_P_ReadData_Vsb(AIAO_PORT_ID_E enPortID, mt_u8 *pu32Dest, mt_u32 u32DestSize);
mt_u32 HAL_AIAO_P_PrepareData(AIAO_PORT_ID_E enPortID, mt_u8 *pu32Src, mt_u32 u3SrcLen);
mt_u32 HAL_AIAO_P_QueryBufData_ProvideRptr(AIAO_PORT_ID_E enPortID, mt_u32 *pu32Rptr);
mt_u32 HAL_AIAO_P_QueryBufData(AIAO_PORT_ID_E enPortID);
mt_u32 HAL_AIAO_P_QueryBufFree(AIAO_PORT_ID_E enPortID);
mt_u32 HAL_AIAO_P_UpdateRptr(AIAO_PORT_ID_E enPortID, mt_u8 *pu32Dest, mt_u32 u32DestSize);
mt_u32 HAL_AIAO_P_UpdateWptr(AIAO_PORT_ID_E enPortID, mt_u8 *pu32Src, mt_u32 u3SrcLen);
mt_void HAL_AIAO_P_GetDelayMs(AIAO_PORT_ID_E enPortID, mt_u32 *pu32Delayms); //todo

mt_s32                  HAL_AIAO_P_GetRbfAttr(AIAO_PORT_ID_E enPortID, AIAO_RBUF_ATTR_S *pstRbfAttr);
#if defined (MT_I2S0_SUPPORT) || defined (MT_I2S1_SUPPORT)
mt_void                 HAL_AIAO_P_GetBorardTxI2SDfAttr(mt_u32 u32BoardI2sNum,
                                                                     MT_UNF_I2S_ATTR_S  *pstI2sAttr,
                                                                     AIAO_PORT_ID_E *penPortID,
                                                                     AIAO_PORT_USER_CFG_S *pAttr);
mt_void                 HAL_AIAO_P_GetBorardRxI2SDfAttr(mt_u32 u32BoardI2sNum,
                                                                     AIAO_PORT_ID_E *penPortID,
                                                                     AIAO_PORT_USER_CFG_S *pAttr);

mt_s32                  HAL_AIAO_P_CheckBorardI2SOpenAttr(AIAO_PORT_ID_E enPortID,
                                                                       const AIAO_IfAttr_S     *pstNewIfAttr);
mt_void                 HAL_AIAO_P_CrateBorardI2SOpenAttr(AIAO_PORT_ID_E enPortID,
                                                                       const AIAO_IfAttr_S     *pstNewIfAttr);
mt_void                 HAL_AIAO_P_DestroyBorardI2SOpenAttr(AIAO_PORT_ID_E enPortID);

#endif

#ifdef MT_AIAO_TIMER_SUPPORT
mt_s32   HAL_AIAO_T_Create(AIAO_TIMER_ID_E enTimerID, const AIAO_Timer_Create_S *pstParam);
mt_void  HAL_AIAO_T_Destroy(AIAO_TIMER_ID_E enTimerID);
mt_s32   HAL_AIAO_T_SetTimerAttr(AIAO_TIMER_ID_E enTimerID,const AIAO_TIMER_Attr_S *pstAttrParam);
mt_s32   HAL_AIAO_T_SetTimerEnable(AIAO_TIMER_ID_E enTimerID, MT_BOOL bEnalbe);
mt_s32   HAL_AIAO_T_GetStatus(AIAO_TIMER_ID_E enTimerID, AIAO_TIMER_Status_S *Param);
mt_void  HAL_AIAO_T_TIMERProcess(AIAO_TIMER_ID_E enTimerID, void * pst);
#endif



mt_u32 HAL_AI_RequestIsr(mt_void);
mt_u32 HAL_AO_RequestIsr(mt_void);
mt_void    HAL_AI_FreeIsr(mt_void);
mt_void    HAL_AO_FreeIsr(mt_void);
//audio_in reset, clk gate, release clk, release reset
mt_void HAL_AI_Hw_Reset(mt_void);
//audio_in I/O cfg
mt_void HAL_AI_IO_config(mt_void);
mt_void HAL_AI_Clk_Srcsel(mt_void)  ;//mt_void HAL_AI_Clk_src_sel()


mt_void HAL_AI_Inter_En(mt_void);
mt_void HAL_AI_Hw_Config(AIAO_PORT_ID_E enPortID);

mt_u32 HAL_AI_Hw_Start(mt_void);

mt_void HAL_AI_Hw_stop(mt_void);
mt_void HAL_AI_Hw_Pause(mt_void);
mt_void HAL_AI_Hw_Pause_Recovery(mt_void);

mt_void HAL_AO_Hw_Start(mt_void);
mt_void HAL_AO_Hw_Config(AIAO_PORT_ID_E enPortID);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif  // __MT_HAL_AIAO_H__
