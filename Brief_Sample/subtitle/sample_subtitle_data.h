/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __SAMPLE_SUBTITLE__
#define __SAMPLE_SUBTITLE__

#ifdef __cplusplus
extern "C"{
#endif

#ifdef CONFIG_MT_CHIP_SYMPHONY4
typedef mt_s32 (*SUBT_DATA_CALLBACK_FN)(mt_u32 u32UserData, mt_u8 *pu8Data, mt_u32 u32DataLength);

typedef struct tagSUBT_DATA_INSTALL_PARAM_S
{
    mt_u32 u32DmxID;
    mt_u16 u16SubtPID;
    mt_u32 u32UserData;
    MT_UNF_SUBT_DATA_TYPE_E enDataType;
    SUBT_DATA_CALLBACK_FN pfnCallback;
}SUBT_DATA_INSTALL_PARAM_S;
#elif defined CONFIG_MT_CHIP_SYMPHONY6
typedef mt_s32 (*SUBT_DATA_CALLBACK_FN)(ulong u32UserData, mt_u8 *pu8Data, mt_u32 u32DataLength);

typedef struct tagSUBT_DATA_INSTALL_PARAM_S
{
    mt_u32 u32DmxID;
    mt_u16 u16SubtPID;
    ulong u32UserData;
    MT_UNF_SUBT_DATA_TYPE_E enDataType;
    SUBT_DATA_CALLBACK_FN pfnCallback;
}SUBT_DATA_INSTALL_PARAM_S;
#endif



mt_s32 SUBT_Data_Init(mt_u8 u8ProgNo);

mt_s32 SUBT_Data_DeInit(void);

mt_s32 SUBT_Data_Install(SUBT_DATA_INSTALL_PARAM_S *pstInstallParam, MT_HANDLE *hData);

mt_s32 SUBT_Data_Uninstall(MT_HANDLE hData);


#ifdef __cplusplus
}
#endif

#endif

