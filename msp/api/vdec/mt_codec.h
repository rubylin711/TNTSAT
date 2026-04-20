/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/******************************************************************************
  File Name     : mt_codec.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/3
  Description   : Definitions of MT_CODEC.
  History       :
  1.Date        : 2015/12/3
    Author      : 
    Modification: Created file

*******************************************************************************/

#ifndef __MT_CODEC_H__
#define __MT_CODEC_H__

/******************************* Include Files *******************************/

#include "mt_video_codec.h"
#include "mt_error_mpi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/****************************** Macro Definition *****************************/

#ifndef MT_VDEC_REG_CODEC_SUPPORT
#define MT_VDEC_REG_CODEC_SUPPORT (0)
#endif

#define MT_ERR_CODEC_NOT_INIT           (MT_ERR_VDEC_NOT_INIT)

/* HANDLE: 
 * BITS[31-16]  USED BY PLATFORM
 * BITS[15-8]   USED BY MT_CODEC
 * BITS[7-0]    USED BY EVERY CODEC 
 */
#define MT_CODEC_LIB_HANDLE(hInst)  ((hInst>>8) & 0xFF)
#define MT_CODEC_INST_HANDLE(hInst) (hInst & 0xFF)

#define MT_CODEC_MAX_NUMBER         (16)

/*************************** Structure Definition ****************************/
/** \addtogroup       */
/** @{ */  /** <!--  */


/** @} */  /** <!-- ==== Structure Definition End ==== */

/******************************* API Declaration *****************************/
/** \addtogroup       */
/** @{ */  /** <!--  */

mt_s32 MT_CODEC_Init(mt_void);
mt_s32 MT_CODEC_DeInit(mt_void);
#if (MT_VDEC_REG_CODEC_SUPPORT == 1)
mt_s32 MT_CODEC_RegisterLib(const mt_char *pszCodecDllName);
mt_s32 MT_CODEC_UnRegisterLib(const mt_char *pszCodecDllName);
#endif
mt_s32 MT_CODEC_Register(MT_CODEC_S* pstCodec);
mt_s32 MT_CODEC_UnRegister(const MT_CODEC_S* pstCodec);
MT_CODEC_S* MT_CODEC_Create(mt_handle* phInst, const MT_CODEC_OPENPARAM_S * pstParam);
mt_s32 MT_CODEC_Destory(mt_handle hInst);
const mt_char* MT_CODEC_GetName(mt_handle hInst);
MT_BOOL MT_CODEC_SupportDecode(const MT_CODEC_S* pstCodec, MT_CODEC_ID_E enID);
#if 0
MT_CODEC_VERSION_U MT_CODEC_GetVersion(mt_handle hInst);
#endif
MT_BOOL MT_CODEC_NeedFrameBuf(mt_handle hInst);


/** @} */  /** <!-- ==== API declaration end ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_CODEC_H__ */

