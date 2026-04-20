/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_DF_DISP_H__
#define __DRV_DF_DISP_H__

#include "vo_fw.h"

mt_s32 DF_Init(void);
mt_s32 DF_Fin(void);

mt_s32 DF_Create(mt_handle* pstDispPool, mt_u32 u32BufNumber, MT_BOOL bCloseHdDI);
mt_s32 DF_Destroy(mt_handle pstDispPool );
mt_s32 DF_Reset(mt_handle pstDispPool);

//transfor decoder info to display info
mt_s32 DF_TransformDec2Disp_sinfo(mt_handle pstDispPool, MT_DRV_VIDEO_FRAME_S * pFrameInfo , MT_DF_VIDEO_FRAME_S * pstVideoFrame);
//win buffer management API
mt_s32 DF_TSK_enQueue(mt_handle pstDispPool , MT_DF_VIDEO_FRAME_S * pstVideoFrame , MT_BOOL * pbNeedNewData);
mt_void DF_ISR_Update(mt_handle pstDispPool, DF_DRV_SETTING_S *pstDrvSetting);//receive drv config attribute //update/release/delete/config the display node

//interface
MT_DF_STATUS DF_GetStatus(mt_handle pstDispPool);
mt_s32 DF_SetCmd(mt_handle pstDispPool, MT_DF_CMD eCMD, MT_VOID* pArgs);
mt_void DF_Wait_For_FreezeCopy_Done(mt_handle pstDispPool);
mt_void DF_FlushDisp(mt_handle pstDispPool, MT_DF_FLUSH_TYPE_E eType);

mt_void DF_SetFreezeBuffer(mt_handle pstDispPool, phys_addr_t u32PhyAddr, MT_U32 u32Size);
mt_u32 DF_TestFreezeDone(mt_handle pstDispPool);

mt_u32 DF_GetVideoCoeffTable(VIDEO_SCALE_COEFF_TABLE_E table);

mt_s32 DF_SetSource(mt_handle pstDispPool, DISP_SOURCE_INFO_S *pstSrc);

mt_s32 DF_AvsyncGetVptsInfo(mt_handle pstDispPool, DF_AVSYNC_PTS_S *pInfo);
mt_s32 DF_AvsyncGetInputRate(mt_handle pstDispPool, mt_u32 *pInputRate);
mt_s32 DF_AvsyncSetSkipFrame(mt_handle pstDispPool, mt_u32 skip_num);
mt_s32 DF_AvsyncSetRepeatFrame(mt_handle pstDispPool);
mt_s32 DF_AvsyncSetPause(mt_handle pstDispPool, MT_BOOL  is_play);

MT_BOOL DF_CheckFifoEmpty(mt_handle pstDispPool);

mt_s32 DF_GetCurrVideoInfo(mt_handle pstDispPool, MT_DF_VIDEO_INFO* vid_info);

mt_void DF_GraScaler_Update(DF_DRV_SETTING_S * pDrvSetting);
mt_void DF_OsdScaler_Update(MT_U32 src_width, MT_U32 src_height, MT_U32 dst_width, MT_U32 dst_height);
mt_void DF_StillScaler_Update(mt_handle pstDispPool, MT_U32 src_width, MT_U32 src_height, MT_U32 dst_width, MT_U32 dst_height, MT_BOOL bProgressive);
mt_void DF_DumpScaler_Update(mt_handle pstDispPool, MT_U32 src_width, MT_U32 src_height, MT_U32 dst_width, MT_U32 dst_height);

mt_void DF_PostProcess(MT_DF_POST_PROCESS_CFG_S* cfg);

mt_s32 DF_GetDispInfo(MT_U8 u8ChId, MT_DF_VIDEO_INFO* vid_info);

mt_u32 DF_GetSlHdrVersion(void);

mt_void DF_UpdateOsdCsc(mt_u32 dst_tv_mode);
mt_void DF_UpdateSdCsc(mt_u32 dst_tv_mode);

#ifdef __cplusplus
}
#endif

#endif

