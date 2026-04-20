/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef TDE_EXT
#define TDE_EXT

#include "mt_type.h"
#include "mt_tde_type.h"
#include "mt_drv_dev.h"

typedef mt_s32  (*FN_TDE_Open)(mt_void);
typedef mt_s32  (*FN_TDE_Close)(mt_void);
typedef mt_s32  (*FN_TDE_BeginJob)(TDE_HANDLE *);
typedef mt_s32  (*FN_TDE_EndJob)(TDE_HANDLE, MT_BOOL, mt_u32, MT_BOOL, TDE_FUNC_CB, mt_void*);
typedef mt_s32  (*FN_TDE_CancelJob)(TDE_HANDLE);
typedef mt_s32  (*FN_TDE_WaitForDone)(TDE_HANDLE, mt_u32);
typedef mt_s32  (*FN_TDE_WaitAllDone)(MT_BOOL);
typedef mt_s32  (*FN_TDE_QuickCopy)(TDE_HANDLE, TDE2_SURFACE_S*, TDE2_RECT_S*, TDE2_SURFACE_S*, TDE2_RECT_S*);
typedef mt_s32  (*FN_TDE_QuickFill)(TDE_HANDLE, TDE2_SURFACE_S*, TDE2_RECT_S*, mt_u32);
typedef mt_s32  (*FN_TDE_QuickResize)(TDE_HANDLE, TDE2_SURFACE_S*, TDE2_RECT_S*, TDE2_SURFACE_S*, TDE2_RECT_S*);
typedef mt_s32  (*FN_TDE_QuickFlicker)(TDE_HANDLE, TDE2_SURFACE_S*, TDE2_RECT_S*, TDE2_SURFACE_S*, TDE2_RECT_S*);
typedef mt_s32  (*FN_TDE_Blit)(TDE_HANDLE, TDE2_SURFACE_S*, TDE2_RECT_S*, TDE2_SURFACE_S*, TDE2_RECT_S*, TDE2_SURFACE_S*,
                          TDE2_RECT_S*, TDE2_OPT_S*);
typedef mt_s32  (*FN_TDE_MbBlit)(TDE_HANDLE, TDE2_MB_S*, TDE2_RECT_S*, TDE2_SURFACE_S*, TDE2_RECT_S*, TDE2_MBOPT_S*);
typedef mt_s32  (*FN_TDE_SolidDraw)(TDE_HANDLE, TDE2_SURFACE_S*, TDE2_RECT_S*, TDE2_SURFACE_S*, TDE2_RECT_S*, TDE2_FILLCOLOR_S*, TDE2_OPT_S*);
typedef mt_s32  (*FN_TDE_SetDeflickerLevel)(TDE_DEFLICKER_LEVEL_E);
typedef mt_s32  (*FN_TDE_EnableRegionDeflicker)(MT_BOOL);
typedef mt_s32	(*FN_TDE_SetAlphaThresholdValue)(mt_u8 u8ThresholdValue);
typedef mt_s32  (*FN_TDE_SetAlphaThresholdState)(MT_BOOL bEnAlphaThreshold);
typedef mt_s32  (*FN_TDE_GetAlphaThresholdState)(MT_BOOL *pbEnAlphaThreshold);
typedef mt_s32  (*FN_TDE_CalScaleRect)(const TDE2_RECT_S*, const TDE2_RECT_S*, TDE2_RECT_S*, TDE2_RECT_S*);
typedef mt_s32  (*FN_TDE_Suspend)(basedev_s *, pm_message_t);
typedef mt_s32  (*FN_TDE_Resume)(basedev_s *);

typedef struct
{
    FN_TDE_Open             pfnTdeOpen;
    FN_TDE_Close            pfnTdeClose;
    FN_TDE_BeginJob         pfnTdeBeginJob;
    FN_TDE_EndJob           pfnTdeEndJob;
    FN_TDE_CancelJob        pfnTdeCancelJob;
    FN_TDE_WaitForDone      pfnTdeWaitForDone;
    FN_TDE_WaitAllDone      pfnTdeWaitAllDone;
    FN_TDE_QuickCopy        pfnTdeQuickCopy;    
    FN_TDE_QuickFill        pfnTdeQuickFill;
    FN_TDE_QuickResize      pfnTdeQuickResize;
    FN_TDE_QuickFlicker     pfnTdeQuickFlicker;
    FN_TDE_Blit             pfnTdeBlit;
    FN_TDE_MbBlit           pfnTdeMbBlit;
    FN_TDE_SolidDraw        pfnTdeSolidDraw;
    FN_TDE_SetDeflickerLevel        pfnTdeSetDeflickerLevel;
    FN_TDE_EnableRegionDeflicker    pfnTdeEnableRegionDeflicker;    
	FN_TDE_SetAlphaThresholdValue   pfnTdeSetAlphaThresholdValue;
	FN_TDE_SetAlphaThresholdState   pfnTdeSetAlphaThresholdState;
	FN_TDE_GetAlphaThresholdState   pfnTdeGetAlphaThresholdState;
    FN_TDE_CalScaleRect     pfnTdeCalScaleRect;
	FN_TDE_Suspend			pfnTdeSuspend;
	FN_TDE_Resume			pfnTdeResume;
} TDE_EXPORT_FUNC_S;

mt_s32 TDE_DRV_ModInit(mt_void);
mt_void  TDE_DRV_ModExit(mt_void);
mt_s32 tde_init_module_k(mt_void);
mt_void tde_cleanup_module_k(mt_void);

#endif


