/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_VPSS_EXT_H__
#define __DRV_VPSS_EXT_H__

#include "mt_type.h"
#include "mt_drv_vpss.h"
#include "mt_drv_dev.h"
#include "drv_pq_define.h"

typedef mt_s32  (*FN_VPSS_GlobalInit)(mt_void);
typedef mt_s32  (*FN_VPSS_GlobalDeInit)(mt_void);
typedef mt_s32  (*FN_VPSS_GetDefaultCfg)(MT_DRV_VPSS_CFG_S *pstVpssCfg);
typedef mt_s32  (*FN_VPSS_CreateVpss)(MT_DRV_VPSS_CFG_S *pstVpssCfg,VPSS_HANDLE *phVPSS);
typedef mt_s32  (*FN_VPSS_DestroyVpss)(VPSS_HANDLE hVPSS);


typedef mt_s32  (*FN_VPSS_SetVpssCfg)(VPSS_HANDLE hVPSS, MT_DRV_VPSS_CFG_S *pstVpssCfg);
typedef mt_s32  (*FN_VPSS_GetVpssCfg)(VPSS_HANDLE hVPSS, MT_DRV_VPSS_CFG_S *pstVpssCfg);

typedef mt_s32  (*FN_VPSS_GetDefaultPortCfg)(MT_DRV_VPSS_PORT_CFG_S *pstVpssPortCfg);
typedef mt_s32  (*FN_VPSS_CreatePort)(VPSS_HANDLE hVPSS,MT_DRV_VPSS_PORT_CFG_S *pstVpssPortCfg,VPSS_HANDLE *phPort);
typedef mt_s32  (*FN_VPSS_DestroyPort)(VPSS_HANDLE hPort);

typedef mt_s32  (*FN_VPSS_GetPortCfg)(VPSS_HANDLE hPort, MT_DRV_VPSS_PORT_CFG_S *pstVpssPortCfg);
typedef mt_s32  (*FN_VPSS_SetPortCfg)(VPSS_HANDLE hPort, MT_DRV_VPSS_PORT_CFG_S *pstVpssPortCfg);

typedef mt_s32  (*FN_VPSS_EnablePort)(VPSS_HANDLE hPort, MT_BOOL bEnable);

typedef mt_s32  (*FN_VPSS_SendCommand)(VPSS_HANDLE hVPSS, MT_DRV_VPSS_USER_COMMAND_E eCommand, mt_void *pArgs);

typedef mt_s32  (*FN_VPSS_GetPortFrame)(VPSS_HANDLE hPort, MT_DRV_VIDEO_FRAME_S *pstVpssFrame);

typedef mt_s32  (*FN_VPSS_RelPortFrame)(VPSS_HANDLE hPort, MT_DRV_VIDEO_FRAME_S *pstVpssFrame);

typedef mt_s32  (*FN_VPSS_GetPortBufListState)(VPSS_HANDLE hPort, MT_DRV_VPSS_PORT_BUFLIST_STATE_S *pstVpssBufListState);
typedef mt_s32  (*FN_VPSS_CheckPortBufListFul)(VPSS_HANDLE hPort);
typedef mt_s32  (*FN_VPSS_SetSourceMode)(VPSS_HANDLE hVPSS,
                          MT_DRV_VPSS_SOURCE_MODE_E eSrcMode,
                          MT_DRV_VPSS_SOURCE_FUNC_S* pstRegistSrcFunc);

typedef mt_s32  (*FN_VPSS_PutImage)(VPSS_HANDLE hVPSS,MT_DRV_VIDEO_FRAME_S *pstImage);
typedef mt_s32  (*FN_VPSS_GetImage)(VPSS_HANDLE hVPSS,MT_DRV_VIDEO_FRAME_S *pstImage);
typedef mt_s32  (*FN_VPSS_RegistHook)(VPSS_HANDLE hVPSS, mt_handle hDst, PFN_VPSS_CALLBACK pfVpssCallback);
//typedef mt_s32  (*FN_VPSS_UpdatePqData)(mt_u32 u32UpdateType,PQ_PARAM_S * pstPqParam);

typedef mt_s32  (*FN_VPSS_Suspend)(basedev_s *pdev, pm_message_t state);
typedef mt_s32  (*FN_VPSS_Resume)(basedev_s *pdev);

typedef struct
{
    FN_VPSS_GlobalInit      pfnVpssGlobalInit;
    FN_VPSS_GlobalDeInit    pfnVpssGlobalDeInit;

    FN_VPSS_GetDefaultCfg   pfnVpssGetDefaultCfg;
    FN_VPSS_CreateVpss      pfnVpssCreateVpss;
    FN_VPSS_DestroyVpss     pfnVpssDestroyVpss;
    FN_VPSS_SetVpssCfg      pfnVpssSetVpssCfg;
    FN_VPSS_GetVpssCfg      pfnVpssGetVpssCfg;

    FN_VPSS_GetDefaultPortCfg   pfnVpssGetDefaultPortCfg;
    FN_VPSS_CreatePort      pfnVpssCreatePort;
    FN_VPSS_DestroyPort     pfnVpssDestroyPort;
    FN_VPSS_GetPortCfg      pfnVpssGetPortCfg;
    FN_VPSS_SetPortCfg      pfnVpssSetPortCfg;
    FN_VPSS_EnablePort      pfnVpssEnablePort;

    FN_VPSS_SendCommand     pfnVpssSendCommand;

    FN_VPSS_GetPortFrame    pfnVpssGetPortFrame;
    FN_VPSS_RelPortFrame    pfnVpssRelPortFrame;

    FN_VPSS_GetPortBufListState     pfnVpssGetPortBufListState;
    FN_VPSS_CheckPortBufListFul     pfnVpssCheckPortBufListFul;

    FN_VPSS_SetSourceMode   pfnVpssSetSourceMode;
    FN_VPSS_PutImage        pfnVpssPutImage;
    FN_VPSS_GetImage        pfnVpssGetImage;

    FN_VPSS_RegistHook      pfnVpssRegistHook;
    //FN_VPSS_UpdatePqData  pfnVpssUpdatePqData;

    FN_VPSS_Resume          pfnVpssResume;
    FN_VPSS_Suspend          pfnVpssSuspend;

} VPSS_EXPORT_FUNC_S;


mt_s32 MT_DRV_VPSS_Init(mt_void);
mt_void MT_DRV_VPSS_Exit(mt_void);

mt_s32 VPSS_DRV_ModInit(mt_void);
mt_void VPSS_DRV_ModExit(mt_void);

#endif

