/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_MTFB_EXT_H__
#define __DRV_MTFB_EXT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "mt_type.h"

#include "drv_pq_define.h"
struct PQ_PARAM_S;

typedef mt_void  (*FN_MTFB_SetLogoLayerEnable)(MT_BOOL);
typedef mt_s32 (*FN_MTFB_UpdatePqData)(mt_u32 u32UpdateType,PQ_PARAM_S * pstPqParam);

typedef struct
{
    FN_MTFB_SetLogoLayerEnable             pfnMtfbSetLogoLayerEnable;
    FN_MTFB_UpdatePqData                   pfnMtfbUpdatePqData; 
} MTFB_EXPORT_FUNC_S;


mt_s32 MTFB_DRV_ModInit(mt_void);
mt_void MTFB_DRV_ModExit(mt_void);

mt_s32 mtfb_init_module_k(mt_void);
mt_void mtfb_cleanup_module_k(mt_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__DRV_MTFB_EXT_H__*/


