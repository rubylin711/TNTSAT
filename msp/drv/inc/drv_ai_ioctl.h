/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_AI_IOCTL_H__
 #define __DRV_AI_IOCTL_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "mt_type.h"
#include "mt_unf_ai.h"

typedef struct hiAI_GetDfAttr_Param_S
{
    MT_UNF_AI_E              enAiPort;
    MT_UNF_AI_ATTR_S         stAttr;
} AI_GetDfAttr_Param_S, *AI_GetDfAttr_Param_S_PTR;

typedef struct hiAI_Create_Param_S
{
    MT_UNF_AI_E              enAiPort;
    MT_UNF_AI_ATTR_S         stAttr;
    mt_handle                hAi;
//#ifdef MT_ALSA_AI_SUPPORT
    MT_BOOL                  bAlsaUse;      //if bAlsaTrack = MT_TRUE ALSA, or UNF
    //AI_ALSA_Param_S           stAlsapara;
    mt_void                  *pAlsaPara;
//#endif 
} AI_Create_Param_S, *AI_Create_Param_S_PTR;

typedef struct hiAI_Enable_Param_S
{
    mt_handle          hAi;
    MT_BOOL            bAiEnable;
} AI_Enable_Param_S, *AI_Enable_Param_S_PTR;

typedef struct hiAI_GetFrame_Param_S
{
    mt_handle          hAi;
    MT_UNF_AO_FRAMEINFO_S   stAiFrame;
} AI_Frame_Param_S, *AI_Frame_Param_S_PTR;

typedef struct hiAI_Attr_Param_S
{
    mt_handle                hAi;
    MT_UNF_AI_ATTR_S         stAttr;
} AI_Attr_Param_S, *AI_Attr_Param_S_PTR;

typedef struct hiAI_Buf_Param_S
{
    mt_handle                hAi;
    AI_BUF_ATTR_S            stAiBuf;
} AI_Buf_Param_S, *AI_Buf_Param_S_PTR;

typedef struct hiAI_DelayComps_Param_S
{
    mt_handle                       hAi;
    MT_UNF_AI_DELAY_S               stDelayComps;
} AI_DelayComps_Param_S, *AI_DelayComps_Param_S_PTR;

/*AI Device command code*/
#define CMD_AI_GEtDEFAULTATTR _IOWR  (MT_ID_AI, 0x00, AI_GetDfAttr_Param_S)
#define CMD_AI_CREATE _IOWR  (MT_ID_AI, 0x01, AI_Create_Param_S)
#define CMD_AI_DESTROY _IOW  (MT_ID_AI, 0x02, mt_handle)
#define CMD_AI_SETENABLE _IOW  (MT_ID_AI, 0x03, AI_Enable_Param_S)
#define CMD_AI_GETENABLE _IOWR  (MT_ID_AI, 0x04, AI_Enable_Param_S)
#define CMD_AI_ACQUIREFRAME _IOWR  (MT_ID_AI, 0x05, AI_Frame_Param_S)
#define CMD_AI_RELEASEFRAME _IOW  (MT_ID_AI, 0x06, AI_Frame_Param_S)
#define CMD_AI_SETATTR _IOW  (MT_ID_AI, 0x07, AI_Attr_Param_S)
#define CMD_AI_GETATTR _IOWR  (MT_ID_AI, 0x08, AI_Attr_Param_S)
#define CMD_AI_GETBUFINFO _IOWR  (MT_ID_AI, 0x09, AI_Buf_Param_S)
#define CMD_AI_SETBUFINFO _IOW  (MT_ID_AI, 0x0a, AI_Buf_Param_S)
#define CMD_AI_SETDELAYCOMPS _IOW  (MT_ID_AI, 0x0b, AI_DelayComps_Param_S)
#define CMD_AI_GETDELAYCOMPS _IOWR  (MT_ID_AI, 0x0c, AI_DelayComps_Param_S)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif 
 
