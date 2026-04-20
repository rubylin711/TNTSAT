/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_PVR_INTF_H__
#define __MT_PVR_INTF_H__

#include "mt_pvr_priv.h"
//#include "mt_unf_pvr.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* End of #ifdef __cplusplus */

/* do event call back function                                              */
extern MT_VOID PVR_Intf_DoEventCallback(MT_U32 u32ChnID, MT_UNF_PVR_EVENT_E enEventType, MT_S32 s32EnvetValue);

MT_S32 PVRIntfInitEvent(MT_VOID);
MT_VOID PVRIntfDeInitEvent(MT_VOID);
mt_u32 MT_PVR_SysGetTimeStampMs(mt_u32 *ms);
MT_S32 PVRIntfEventDirectCall(MT_U32 u32ChnId,MT_UNF_PVR_EVENT_E  enEventType,MT_S32 s32EventValue);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* End of #ifndef __MT_PVR_INTF_H__ */
