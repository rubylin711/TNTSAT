/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
  File Name     : drv_vdec_usrdata.h
  Version       : Initial Draft
  Author        : Montage MA-SW
  Created       : 2016/01/06
  Description   :
  History       :
  1.Date        : 2016/01/06
    Author      :
    Modification: Created file

******************************************************************************/



#ifndef __DRV_VDEC_USRDATA_H__
#define __DRV_VDEC_USRDATA_H__

/******************************* Include Files *******************************/

/* add include here */
#include "mt_type.h"
#include "mt_unf_common.h"
#include "mt_drv_vdec_ioctl.h"
#include "vfmw.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/

#define VDEC_USERDATA_IDENTIFIER_DVB1   (0x34394147)
#define VDEC_USERDATA_IDENTIFIER_AFD    (0x31475444)

#define VDEC_USERDATA_TYPE_DVB1_CC      (0x03)
#define VDEC_USERDATA_TYPE_DVB1_BAR     (0x06)
#define VDEC_USERDATA_NEED_ARRANGE (1)
/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/
mt_s32 USRDATA_Init(mt_void);
mt_s32 USRDATA_DeInit(mt_void);
mt_s32 USRDATA_Alloc(mt_handle hHandle, MT_DRV_VDEC_USERDATABUF_S* pstBuf);
mt_s32 USRDATA_SetUserAddr(mt_handle hHandle, ulong u32Addr);
mt_s32 USRDATA_Free(mt_handle hHandle);
mt_s32 USRDATA_Start(mt_handle hHandle);
mt_s32 USRDATA_Stop(mt_handle hHandle);
mt_s32 USRDATA_Reset(mt_handle hHandle);
mt_s32 USRDATA_Acq(mt_handle hHandle, MT_UNF_VIDEO_USERDATA_S* pstUsrData, MT_UNF_VIDEO_USERDATA_TYPE_E* penType);
mt_s32 USRDATA_Rls(mt_handle hHandle, MT_UNF_VIDEO_USERDATA_S* pstUsrData);
mt_s32 USRDATA_SetEosFlag(mt_handle hHandle);
mt_s32 USRDATA_Arrange(mt_handle hHandle, MT_VDEC_USRDAT_S* pstUsrData);
mt_s32 USRDATA_Put(mt_handle hHandle, MT_VDEC_USRDAT_S* pstUsrData, MT_UNF_VIDEO_USERDATA_TYPE_E enType);
#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __DRV_VDEC_USRDATA_H__ */

