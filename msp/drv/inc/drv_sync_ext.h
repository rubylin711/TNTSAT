/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _SYNC_EXT_H_
#define _SYNC_EXT_H_


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


#include "mt_type.h"
#include "mt_drv_sync.h"


typedef mt_s32  (*FN_SYNC_VidProc)(mt_handle,  SYNC_VID_INFO_S *, SYNC_VID_OPT_S *);
typedef mt_void (*FN_SYNC_PcrProc)(mt_handle, mt_u32);
typedef MT_BOOL (*FN_SYNC_VerifyHandle)(mt_handle);


typedef struct tagSYNC_EXPORT_FUNC_S
{
    FN_SYNC_VidProc             pfnSYNC_VidProc;
    FN_SYNC_PcrProc             pfnSYNC_PcrProc;
    FN_SYNC_VerifyHandle        pfnSYNC_VerifyHandle;
    
}SYNC_EXPORT_FUNC_S;


mt_s32 SYNC_DRV_ModInit(mt_void);
mt_void SYNC_DRV_ModExit(mt_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif

