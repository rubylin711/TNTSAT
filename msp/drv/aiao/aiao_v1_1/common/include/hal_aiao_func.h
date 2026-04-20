/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_AIAO_FUNC_H__
#define __MT_AIAO_FUNC_H__

#include "hi_type.h"
#include "hal_aiao_common.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/

/* global function */
mt_s32					iHAL_AIAO_Init(mt_void);
mt_void					iHAL_AIAO_DeInit(mt_void);
mt_void					iHAL_AIAO_GetHwCapability(mt_u32 *pu32Capability);
mt_void					iHAL_AIAO_GetHwVersion(mt_u32 *pu32Version);
mt_void					iHAL_AIAO_DBG_RWReg(AIAO_Dbg_Reg_S *pstReg);
mt_void					iHAL_AIAO_SetTopInt(mt_u32 u32Multibit);
mt_u32					iHAL_AIAO_GetTopIntRawStatus(mt_void);
mt_u32					iHAL_AIAO_GetTopIntStatus(mt_void);

/*****************************************************************************
 Description  : AIAO TX/RX Port DSP Control HAL API
*****************************************************************************/
mt_void					iHAL_AIAO_P_SetInt(AIAO_PORT_ID_E enPortID, mt_u32 u32Multibit);
mt_void					iHAL_AIAO_P_ClrInt(AIAO_PORT_ID_E enPortID, mt_u32 u32Multibit);
mt_u32					iHAL_AIAO_P_GetIntStatusRaw(AIAO_PORT_ID_E enPortID);
mt_u32					iHAL_AIAO_P_GetIntStatus(AIAO_PORT_ID_E enPortID);

/* global port function */
mt_s32					iHAL_AIAO_P_Open(const AIAO_PORT_ID_E enPortID, const AIAO_PORT_USER_CFG_S *pstConfig,
                                        mt_handle *phandle, AIAO_IsrFunc** pIsr);


mt_void					iHAL_AIAO_P_Close(mt_handle handle);
mt_s32					iHAL_AIAO_P_Start(mt_handle handle);
mt_s32					iHAL_AIAO_P_Stop(mt_handle handle, AIAO_PORT_STOPMODE_E enStopMode);
mt_s32					iHAL_AIAO_P_Mute(mt_handle handle, MT_BOOL bMute);
mt_s32					iHAL_AIAO_P_SetVolume(mt_handle handle, mt_u32 u32VolumedB);
mt_s32                  iHAL_AIAO_P_SetSpdifCategoryCode(mt_handle handle, AIAO_SPDIF_CATEGORYCODE_E eCategoryCode);
mt_s32                  iHAL_AIAO_P_SetSpdifSCMSMode(mt_handle handle, AIAO_SPDIF_SCMS_MODE_E eSCMSMode);
mt_s32					iHAL_AIAO_P_SetTrackMode(mt_handle handle, AIAO_TRACK_MODE_E enTrackMode);
mt_s32 iAIAO_HAL_P_SetBypass(mt_handle handle, MT_BOOL bByBass);
mt_s32					iHAL_AIAO_P_GetUserCongfig(mt_handle handle, AIAO_PORT_USER_CFG_S *pstUserConfig);
mt_s32					iHAL_AIAO_P_GetStatus(mt_handle handle, AIAO_PORT_STAUTS_S *pstProcInfo);
mt_s32					iHAL_AIAO_P_SelectSpdifSource(mt_handle handle, AIAO_SPDIFPORT_SOURCE_E eSrcChnId);
mt_s32					iHAL_AIAO_P_SetSpdifOutPort(mt_handle handle, mt_s32 bEn);
mt_s32					iHAL_AIAO_P_SetI2SSdSelect(mt_handle handle, AIAO_I2SDataSel_S  *pstSdSel);
mt_s32 iHAL_AIAO_P_SetAttr(mt_handle handle, AIAO_PORT_ATTR_S *pstAttr);
mt_s32 iHAL_AIAO_P_GetAttr(mt_handle handle, AIAO_PORT_ATTR_S *pstAttr);
mt_void iHAL_AIAO_P_ProcStatistics(mt_handle handle, mt_u32 u32IntStatus);
mt_s32 iHAL_AIAO_P_SetI2SMasterClk(AIAO_PORT_ID_E enPortID, AIAO_IfAttr_S *pstIfAttr);
mt_s32 iHAL_AIAO_P_SetI2SlaveClk(AIAO_PORT_ID_E enPortID, AIAO_IfAttr_S *pstIfAttr);


/* port buffer function */
mt_u32                  iHAL_AIAO_P_ReadData_NotUpRptr(mt_handle handle, mt_u8 * pu32Dest, mt_u32 u32DestSize, mt_u32 *pu32Rptr, mt_u32 *pu32Wptr);
mt_u32					iHAL_AIAO_P_ReadData(mt_handle handle, mt_u8 * pu32Dest, mt_u32 u32DestSize);
mt_u32					iHAL_AIAO_P_WriteData(mt_handle handle, mt_u8 * pu32Src, mt_u32 u3SrcLen);
mt_u32					iHAL_AIAO_P_PrepareData(mt_handle handle, mt_u8 * pu32Src, mt_u32 u3SrcLen);
mt_u32                  iHAL_AIAO_P_QueryBufData_ProvideRptr(mt_handle handle, mt_u32 *pu32Rptr);
mt_u32					iHAL_AIAO_P_QueryBufData(mt_handle handle);
mt_u32					iHAL_AIAO_P_QueryBufFree(mt_handle handle);
mt_u32					iHAL_AIAO_P_UpdateRptr(mt_handle handle, mt_u8 * pu32Dest, mt_u32 u32DestSize);
mt_u32					iHAL_AIAO_P_UpdateWptr(mt_handle handle, mt_u8 * pu32Src, mt_u32 u3SrcLen);
mt_s32                  iHAL_AIAO_P_GetRbfAttr(mt_handle handle, AIAO_RBUF_ATTR_S *pstRbfAttr);
mt_void iHAL_AIAO_P_GetDelayMs(mt_handle handle, mt_u32 * pu32Delayms);
#ifdef MT_ALSA_AI_SUPPORT
mt_u32 iHAL_AIAO_P_ALSA_UpdateRptr(mt_handle handle, mt_u8 * pu32Dest, mt_u32 u32DestSize);
mt_u32 iHAL_AIAO_P_ALSA_QueryWritePos (mt_handle handle);
mt_u32 iHAL_AIAO_P_ALSA_QueryReadPos (mt_handle handle);

mt_u32 iHAL_AIAO_P_ALSA_UpdateWptr(mt_handle handle, mt_u8 * pu32Dest, mt_u32 u32DestSize);
mt_u32 iHAL_AIAO_P_ALSA_FLASH(mt_handle handle);
#endif
#ifdef MT_AIAO_TIMER_SUPPORT
mt_s32 iHAL_AIAO_T_Create(AIAO_TIMER_ID_E enTimerID,const AIAO_Timer_Create_S *pstParam, mt_handle *phandle,AIAO_TimerIsrFunc** pIsr);
mt_void iHAL_AIAO_T_Destroy(mt_handle handle);
mt_s32 iHAL_AIAO_T_SetTimerAttr(mt_handle handle,const AIAO_TIMER_Attr_S *pstAttrParam);
mt_s32 iHAL_AIAO_T_SetTimerEnalbe(mt_handle handle,MT_BOOL bEnalbe);
mt_s32 iHAL_AIAO_T_GetStatus(mt_handle handle, AIAO_TIMER_Status_S *pstStatus);
mt_void iHAL_AIAO_T_TIMERProcess(mt_handle handle);
#endif
#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif  // __MT_AIAO_FUNC_H__
