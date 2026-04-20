/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef  __SAMPLE_TELETEXT_OUT_H__
#define __SAMPLE_TELETEXT_OUT_H__

#include  "mt_unf_ttx.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TTX_OSD_WIDTH	(720)
#define TTX_OSD_HEIGHT	(576)

mt_s32	Mtgo_Teletext_Init(mt_void);

mt_void Mtgo_Teletext_DeInit(mt_void);


mt_s32	TTX_SampleCallBack(MT_HANDLE hTTX, MT_UNF_TTX_CB_E enCB, mt_void *pvCBParam);

#ifdef __cplusplus
}
#endif
#endif
