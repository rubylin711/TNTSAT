/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __SAMPLE_CC__
#define __SAMPLE_CC__

#ifdef __cplusplus
extern "C"{
#endif

typedef mt_s32 (*CC_DATA_CALLBACK_FN)(mt_u32 u32UserData, mt_u8 *pu8Data, mt_u32 u32DataLength);

typedef struct tagCC_DATA_INSTALL_PARAM_S
{
    mt_u32 u32DmxID;
    mt_u16 u16CCPID;
    mt_u32 u32UserData;
    CC_DATA_CALLBACK_FN pfnCallback;
}CC_DATA_INSTALL_PARAM_S;

mt_s32 CC_Data_Init(MT_VOID);

mt_s32 CC_Data_DeInit(MT_VOID);

mt_s32 CC_Data_Install(CC_DATA_INSTALL_PARAM_S *pstInstallParam, mt_handle *hData);

mt_s32 CC_Data_Uninstall(mt_handle hData);


#ifdef __cplusplus
}
#endif

#endif

