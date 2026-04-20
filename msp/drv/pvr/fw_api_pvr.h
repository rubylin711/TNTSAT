/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __FW_API_PVR_H__
#define __FW_API_PVR_H__
#include "pvr_index.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  PVR_OK               = 0x0,
  PVR_ERROR            = 0x1,
  PVR_UNSUPPORTED      = 0x2,
  PVR_NO_FRAME_BUFFER  = 0x3,
  PVR_NO_DISP_BUFFER   = 0x4,
  PVR_CMD_STOP_ACK     = 0x5,
  PVR_ERR_NO_DATA      = 0x80000000,
  PVR_TIME_OUT         = 0x80000001,
  PVR_ERR_NO_ISR       = 0x80000002,
  PVR_ERR_SYNTAX       = 0x80000004,
  PVR_NO_ERR_DECODING  = 0x40000000,
  PVR_NO_SEQ_HEADER    = 0x20000000,
  PVR_RET_BUTT

}pvr_return_id_t;

MT_S32 PVR_InitWithOperation(PVR_IDX_OPERATION_S *pArgs);
MT_S32 PVR_Exit(MT_U32 chn_cum);
#ifdef __cplusplus
}
#endif

#endif

