
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
******************************************************************************
File Name     : drv_disp_error.h
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2017/01/30
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/
#ifndef __DRV_DISP_DEBUG_H__
#define __DRV_DISP_DEBUG_H__

#include "mt_type.h"
#include "drv_disp_com.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

int vdp_str2val(char *str, unsigned int *data);

struct file *vdp_k_fopen(const char *filename, int flags, int mode);
void vdp_k_fclose(struct file *filp);
int vdp_k_fread(char *buf, unsigned int len, struct file *filp);
int vdp_k_fwrite(char *buf, int len, struct file *filp);

#define DRV_WIN_PROC_CMD_VALID  0xffffffffU
#define DRV_WIN_PROC_CMD_EN     0x00010001U
#define DRV_WIN_PROC_CMD_DIS    0x00010002U
#define DRV_WIN_PROC_CMD_FRZ_B  0x00010010U
#define DRV_WIN_PROC_CMD_FRZ_S  0x00010011U
#define DRV_WIN_PROC_CMD_UNFRZ  0x00010012U
#define DRV_WIN_PROC_CMD_PAUSE  0x00010021U
#define DRV_WIN_PROC_CMD_RESUME 0x00010022U
#define DRV_WIN_PROC_CMD_SV_YUV 0x00020001U

mt_s32 vdp_k_SaveYUVImg(struct file *pfYUV, MT_DRV_VIDEO_FRAME_S *pstFrame, mt_s32 num);
mt_s32 vdp_DebugSaveYUVImg(MT_DRV_VIDEO_FRAME_S *pstCurFrame, mt_char *buffer, mt_u32 count);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /*  __DRV_DISP_ERROR_H__  */


