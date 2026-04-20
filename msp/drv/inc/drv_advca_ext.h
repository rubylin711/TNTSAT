/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_ADVCA_EXT_H_
#define __DRV_ADVCA_EXT_H_

#include "mt_type.h"
#include "mt_unf_cipher.h"
//#include "mt_unf_advca.h"

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

#define MT_UNF_ADVCA_CA_TARGET_E int

typedef enum mtDRV_ADVCA_CA_TARGET_E
{
	DRV_ADVCA_CA_TARGET_DEMUX         = 0,
	DRV_ADVCA_CA_TARGET_MULTICIPHER,
}DRV_ADVCA_CA_TARGET_E;

typedef struct mtDRV_ADVCA_EXTFUNC_PARAM_S
{
    MT_UNF_CIPHER_CA_TYPE_E enCAType;
    MT_UNF_ADVCA_CA_TARGET_E enTarget;
    MT_U32 AddrID;
    MT_U32 EvenOrOdd;
    MT_U8 *pu8Data;
    MT_BOOL bIsDeCrypt;
}DRV_ADVCA_EXTFUNC_PARAM_S;

typedef MT_S32 (*FN_CA_Crypto)(DRV_ADVCA_EXTFUNC_PARAM_S stParam);
typedef MT_S32 (*FN_CA_Suspend)(MT_VOID);
typedef MT_S32 (*FN_CA_Resume)(MT_VOID);

typedef struct
{
    FN_CA_Crypto pfnAdvcaCrypto;
	FN_CA_Suspend pfnAdvcaSuspend;
	FN_CA_Suspend pfnAdvcaResume;
}ADVCA_EXPORT_FUNC_S;

MT_S32 ADVCA_DRV_ModeInit(MT_VOID);
MT_VOID ADVCA_DRV_ModeExit(MT_VOID);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif   /* _DRV_ADVCA_EXT_H_ */

