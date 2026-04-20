/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_JPGE_EXT_H__
#define __DRV_JPGE_EXT_H__

#include "mt_drv_dev.h"
//#include "jpge_ext.h"
#include "mt_jpge_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef mt_s32  (*FN_JPGE_EncodeFrame)(mt_u32 EncHandle, Venc2Jpge_EncIn_S * pEncIn, MT_BOOL *pBufferFull);
typedef mt_s32  (*FN_JPGE_CreateChn) (mt_u32 *pEncHandle, Jpge_EncCfg_S *pEncCfg );
typedef mt_s32  (*FN_JPGE_DestroyChn)( mt_u32   EncHandle );
typedef mt_s32  (*FN_JPGE_Suspend)(basedev_s *, pm_message_t);
typedef mt_s32  (*FN_JPGE_Resume)(basedev_s *);

typedef struct
{
	//FN_JPGE_Init			pfnJpgeInit;
	//FN_JPGE_DeInit		pfnJpgeDeInit;
	FN_JPGE_CreateChn		pfnJpgeCreateChn;
	FN_JPGE_DestroyChn		pfnJpgeDestroyChn;
	FN_JPGE_EncodeFrame		pfnJpgeEncodeFrame;
	FN_JPGE_Suspend			pfnJpgeSuspend;
	FN_JPGE_Resume			pfnJpgeResume;
} JPGE_EXPORT_FUNC_S;


typedef struct
{
    struct clk *jencclk;
}JPGE_PRIV_DATA_S;


mt_s32 JPGE_DRV_ModInit(mt_void);
mt_void JPGE_DRV_ModExit(mt_void);

/*************************************************************************************/



/* Open & Close Hardware */
mt_s32 Jpge_Open ( mt_void );
mt_s32 Jpge_Close( mt_void );

/* Create & Destroy One Encoder Channel */
mt_s32 Jpge_Create ( mt_u32 *pEncHandle, Jpge_EncCfg_S *pEncCfg );
mt_s32 Jpge_Destroy( mt_u32   EncHandle );

/* Encode One Frame */
mt_s32 Jpge_Encode ( mt_u32 EncHandle, Jpge_EncIn_S *pEncIn, Jpge_EncOut_S *pEncOut );

/* for VENC*/
mt_s32 Jpge_Create_toVenc(mt_u32 * pEncHandle, Jpge_EncCfg_S * pEncCfg);
mt_s32 Jpge_Encode_toVenc( mt_u32 EncHandle, Venc2Jpge_EncIn_S *pEncIn,MT_BOOL *pBufferFull);

MT_BOOL Jpge_Reset(mt_void);
/*************************************************************************************/






#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__DRV_JPGE_EXT_H__*/
