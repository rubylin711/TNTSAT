/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __SAMPLE_TTX_DATA__
#define __SAMPLE_TTX_DATA__

#include "mt_unf_ttx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagCmdLinePara
{
    mt_u32 			       ttx_Type;
    mt_u32 			       ttx_Pid;	
    mt_u8*                 pszFileName;
    mt_u16                 u16Pid;
    mt_u8*                 pszLanguage;
    MT_UNF_TTX_PAGE_ADDR_S stInitpageAddr;
} CMD_LINE_ST;

typedef mt_s32 (*TTX_DATA_CALLBACK_FN)(mt_u32 u32UserData, mt_u8 *pu8Data, mt_u32 u32DataLength);

typedef struct tagTTX_DATA_INSTALL_PARAM_S
{
    mt_u32 u32DmxID;
    mt_u16 u16TtxPID;
    mt_u32 u32UserData;
    TTX_DATA_CALLBACK_FN pfnCallback;
} TTX_DATA_INSTALL_PARAM_S;

typedef struct tagTTX_DATA_S
{
    MT_BOOL bEnable;
    TTX_DATA_INSTALL_PARAM_S stInstallParam;
    MT_HANDLE hChannelID;
    MT_HANDLE hFilterID;
} TTX_DATA_S;

mt_s32 Ttx_Data_Init(void);

mt_s32 Ttx_Data_DeInit(void);

mt_s32 Ttx_Data_Install(TTX_DATA_INSTALL_PARAM_S *pstInstallParam, MT_HANDLE *hData);

mt_s32 Ttx_Data_Uninstall(MT_HANDLE hData);


#ifdef __cplusplus
}
#endif

#endif

