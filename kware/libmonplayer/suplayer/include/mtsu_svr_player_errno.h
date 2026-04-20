/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/**
 \file
 \brief Error codes of the server (SVR) player module. CNcomment:svr player模块错误码CNend
 \author Montage Co., Ltd.
 \date 2006-2018
 \version 1.0
 \author
 \date 2011-11-10
 */

#ifndef __MT_SVR_PLAYER_ERRNO_H__
#define __MT_SVR_PLAYER_ERRNO_H__

#include "mt_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
/***************************** Macro Definition ******************************/
/** \addtogroup      Suplayer */
/** @{ */ /** <!--[Suplayer] */

/** Unsupported file formats, MT_SUCCESS = 0 */
/** CNcomment:不支持的文件格式，MT_SUCCESS = 0 */
#define MT_ERRNO_NOT_SUPPORT_FORMAT (MT_SUCCESS + 0x1)

/** Unsupported protocols, MT_SUCCESS = 0 */
/** CNcomment:不支持的协议，MT_SUCCESS = 0 */
#define MT_ERRNO_NOT_SUPPORT_PROTOCOL (MT_SUCCESS + 0x2)

/** Unsupported play speed, MT_SUCCESS = 0 */
/** CNcomment:不支持的播放速度，MT_SUCCESS = 0 */
#define MT_ERRNO_NOT_SUPPORT_PLAYSPEED (MT_SUCCESS + 0x3)

/** @} */ /*! <!-- Macro Definition end */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_SVR_PLAYER_ERRNO_H__ */
