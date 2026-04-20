/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef  __MPI_AEF_H__
#define  __MPI_AEF_H__

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif

typedef struct
{
    mt_handle                hEntry;
    mt_handle                hHaEffect;
    MT_UNF_SND_E             enSnd;
    MT_BOOL                  bEnable;
    AO_AEF_PROC_ITEM_S       *pstProcItem;
} AEF_CHANNEL_S;

mt_s32 MT_MPI_AO_AEF_RegisterAuthLib(const mt_char *pAefLibFileName);
mt_s32 MT_MPI_AO_AEF_Create(MT_UNF_SND_E enSound, MT_UNF_SND_AEF_TYPE_E enAefType, mt_void *pstAdvAttr, mt_handle *phAef);
mt_s32 MT_MPI_AO_AEF_Destroy(mt_handle hAef);
mt_s32 MT_MPI_AO_AEF_SetEnable(mt_handle hAef, MT_BOOL bEnable);
mt_s32 MT_MPI_AO_AEF_GetEnable(mt_handle hAef, MT_BOOL *pbEnable);
mt_s32 MT_MPI_AO_AEF_SetParams(mt_handle hAef, mt_u32 u32ParamType, const mt_void *pstParms);
mt_s32 MT_MPI_AO_AEF_GetParams(mt_handle hAef, mt_u32 u32ParamType, mt_void *pstParms);
mt_s32 MT_MPI_AO_AEF_SetConfig(mt_handle hAef, mt_u32 u32CfgType, const mt_void *pstConfig);
mt_s32 MT_MPI_AO_AEF_GetConfig(mt_handle hAef, mt_u32 u32CfgType, mt_void *pstConfig);


#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif
#endif
