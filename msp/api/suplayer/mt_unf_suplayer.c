/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_unf_suplayer.c
  Version       : Initial Draft
  Author        : Montage software group
  Created       : 2015/11/25
  Description   : Common definitions of MT_CODEC(video).
                  The codec wants to register to MT_CODEC need to adapt to MT_CODEC_S.
  History       :
  1.Date        : 2015/11/25
    Author      :
    Modification: Created file

********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "mt_mpi_suplayer.h"
#include "mt_unf_suplayer.h"
#include "mt_mpi_vdec.h"
#include "mt_module_debug.h"

#define OFFSETOF(TYPE, MEMBER, OFF) \
    TYPE temp;              \
    OFF = (unsigned long)(&(temp.MEMBER)) - (unsigned long)(&(temp));

mt_s32 MT_UNF_SUPLAYER_Init(mt_void)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_SUPLAYER_Init();

    return s32Ret;
}

mt_s32 MT_UNF_SUPLAYER_DeInit(mt_void)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_SUPLAYER_DeInit();

    return s32Ret;
}

mt_s32 MT_UNF_SUPLAYER_Create(/*const MT_UNF_SUPLAYER_ATTR_S *pstAvAttr*/void *pstAvAttr, mt_handle *phSuplayer)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_SUPLAYER_Create(pstAvAttr, phSuplayer);

    return s32Ret;
}

mt_s32 MT_UNF_SUPLAYER_Destroy(mt_handle hSuplayer)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_SUPLAYER_Destroy(hSuplayer);

    return s32Ret;
}

mt_s32 MT_UNF_SUPLAYER_Get_Push_Aud_Flg(mt_handle hSuplayer)
{
    mt_s32 s32Ret = 1;
    SUPLAYER_S *info = NULL;
    hSuplayer = hSuplayer;
    s32Ret = MT_MPI_SUPLAYER_Get_Proc_Info(hSuplayer, &info);
    if(info == NULL){
        printf("\n[%s_%d] cant get suplayer handle !!!\n",__func__,__LINE__);
    }
    else
        s32Ret = (mt_s32)info->ctrl.send_aud;
    return s32Ret;
}

mt_s32 MT_UNF_SUPLAYER_Get_Push_Vid_Flg(mt_handle hSuplayer)
{
    mt_s32 s32Ret = 1;
    SUPLAYER_S *info = NULL;
    hSuplayer = hSuplayer;
    s32Ret = MT_MPI_SUPLAYER_Get_Proc_Info(hSuplayer, &info);
    if(info == NULL){
        printf("\n[%s_%d] cant get suplayer handle !!!\n",__func__,__LINE__);
    }
    else
        s32Ret = (mt_s32)info->ctrl.send_vid;
    return s32Ret;
}

mt_s32 MT_UNF_SUPLAYER_Get_Push_Subt_Flg(mt_handle hSuplayer)
{
    mt_s32 s32Ret = 1;
    SUPLAYER_S *info = NULL;
    hSuplayer = hSuplayer;
    s32Ret = MT_MPI_SUPLAYER_Get_Proc_Info(hSuplayer, &info);
    if(info == NULL){
        printf("\n[%s_%d] cant get suplayer handle !!!\n",__func__,__LINE__);
    }
    else
        s32Ret = (mt_s32)info->ctrl.send_subt;
    return s32Ret;
}

mt_s32 MT_UNF_SUPLAYER_Get_Dump_Aud_Flg(mt_handle hSuplayer)
{
    mt_s32 s32Ret = 1;
    SUPLAYER_S *info = NULL;
    hSuplayer = hSuplayer;
    s32Ret = MT_MPI_SUPLAYER_Get_Proc_Info(hSuplayer, &info);
    if(info == NULL){
        printf("\n[%s_%d] cant get suplayer handle !!!\n",__func__,__LINE__);
    }
    else
        s32Ret = (mt_s32)info->ctrl.dump_aes;
    return s32Ret;
}

mt_s32 MT_UNF_SUPLAYER_Get_Dump_Vid_Flg(mt_handle hSuplayer)
{
    mt_s32 s32Ret = 1;
    SUPLAYER_S *info = NULL;
    hSuplayer = hSuplayer;
    s32Ret = MT_MPI_SUPLAYER_Get_Proc_Info(hSuplayer, &info);
    if(info == NULL){
        printf("\n[%s_%d] cant get suplayer handle !!!\n",__func__,__LINE__);
    }
    else
        s32Ret = (mt_s32)info->ctrl.dump_ves;
    return s32Ret;
}

mt_s32 MT_UNF_SUPLAYER_Get_Dump_Subt_Flg(mt_handle hSuplayer)
{
    mt_s32 s32Ret = 1;
    SUPLAYER_S *info = NULL;
    hSuplayer = hSuplayer;
    s32Ret = MT_MPI_SUPLAYER_Get_Proc_Info(hSuplayer, &info);
    if(info == NULL){
        printf("\n[%s_%d] cant get suplayer handle !!!\n",__func__,__LINE__);
    }
    else
        s32Ret = (mt_s32)info->ctrl.dump_ses;
    return s32Ret;
}

mt_s32 MT_UNF_SUPLAYER_Get_Log_Level(mt_handle hSuplayer)
{
    mt_s32 s32Ret = 1;
    SUPLAYER_S *info = NULL;
    hSuplayer = hSuplayer;
    s32Ret = MT_MPI_SUPLAYER_Get_Proc_Info(hSuplayer, &info);
    if(info == NULL){
        printf("\n[%s_%d] cant get suplayer handle !!!\n",__func__,__LINE__);
    }
    else
        s32Ret = (mt_s32)info->ctrl.log_level;
    return s32Ret;
}

void* MT_UNF_SUPLAYER_Get_Control(mt_handle hSuplayer)
{
   // mt_s32 s32Ret = 1;
    SUPLAYER_S *info = NULL;
    
    hSuplayer = hSuplayer;
    MT_MPI_SUPLAYER_Get_Proc_Info(hSuplayer, &info);
    if(info)
        return (void*)(&(info->ctrl));
    else
        return NULL;
}

mt_s32 MT_UNF_SUPLAYER_Set_File_Info(mt_handle hSuplayer, SUPLAYER_OUT_S *in_info)
{
    mt_s32 s32Ret = 1;
    SUPLAYER_S *info = NULL;
    hSuplayer = hSuplayer;
    s32Ret = MT_MPI_SUPLAYER_Get_Proc_Info(hSuplayer, &info);
    if(info == NULL){
        printf("\n[%s_%d] cant get suplayer handle !!!\n",__func__,__LINE__);
    }
    else{
        if(sizeof(SUPLAYER_FILE_INFO_OUT_T) != sizeof(SUPLAYER_FILE_INFO_T))
            printf("\n[%s_%d] Please check structure!!!\n",__func__,__LINE__);
        else
	    memcpy(&info->file, &in_info->file, sizeof(SUPLAYER_FILE_INFO_OUT_T));
        s32Ret = 0;
    }
    return s32Ret;
}

mt_s32 MT_UNF_SUPLAYER_Set_Control_Info(mt_handle hSuplayer, SUPLAYER_OUT_S *in_info)
{
    mt_s32 s32Ret = 1;
    SUPLAYER_S *info = NULL;
    hSuplayer = hSuplayer;
    s32Ret = MT_MPI_SUPLAYER_Get_Proc_Info(hSuplayer, &info);
    if(info == NULL){
        printf("\n[%s_%d] cant get suplayer handle !!!\n",__func__,__LINE__);
    }
    else{
        if(sizeof(SUPLAYER_CONTROL_INFO_OUT_T) != sizeof(SUPLAYER_CONTROL_INFO_T))
            printf("\n[%s_%d] Please check structure!!!\n",__func__,__LINE__);
        else{
            #if 0
	    memcpy(&info->ctrl.vid, &in_info->ctrl.vid, sizeof(SUPLAYER_CONTROL_AV_INFO_OUT_T));
            memcpy(&info->ctrl.aud, &in_info->ctrl.aud, sizeof(SUPLAYER_CONTROL_AV_INFO_OUT_T));
            memcpy(&info->ctrl.subt, &in_info->ctrl.subt, sizeof(SUPLAYER_CONTROL_AV_INFO_OUT_T));
            memcpy(&info->ctrl.chip_type, &in_info->ctrl.chip_type, (&info->ctrl.vid) - (&info->ctrl.chip_type));
            info->ctrl.chip_type = in_info->ctrl.chip_type;
            info->ctrl.fill_es_states = in_info->ctrl.fill_es_states;
            info->ctrl.vdec_policy = in_info->ctrl.vdec_policy;
            info->ctrl.aud_out_mode = in_info->ctrl.aud_out_mode;
            #else
            int tmp;
            OFFSETOF(SUPLAYER_CONTROL_INFO_OUT_T,chip_type,tmp);
            memcpy(&info->ctrl.chip_type, &in_info->ctrl.chip_type, (int)sizeof(SUPLAYER_CONTROL_INFO_OUT_T) - tmp);
            #endif
        }
        s32Ret = 0;
    }
    return s32Ret;
}

mt_s32 MT_UNF_SUPLAYER_Set_Format_Info(mt_handle hSuplayer, SUPLAYER_OUT_S *in_info)
{
    mt_s32 s32Ret = 1;
    SUPLAYER_S *info = NULL;
    hSuplayer = hSuplayer;
    s32Ret = MT_MPI_SUPLAYER_Get_Proc_Info(hSuplayer, &info);
    if(info == NULL){
        printf("\n[%s_%d] cant get suplayer handle !!!\n",__func__,__LINE__);
    }
    else{
        if(sizeof(SUPLAYER_FORMAT_INFO_OUT_T) != sizeof(SUPLAYER_FORMAT_INFO_T))
            printf("\n[%s_%d] Please check structure!!!\n",__func__,__LINE__);
        else{
            memcpy(&info->format, &in_info->format, sizeof(SUPLAYER_FORMAT_INFO_OUT_T));
        }
        s32Ret = 0;
    }
    return s32Ret;
}

mt_s32 MT_UNF_SUPLAYER_Set_Adapter_Info(mt_handle hSuplayer, SUPLAYER_OUT_S *in_info)
{
    mt_s32 s32Ret = 1;
    SUPLAYER_S *info = NULL;
    hSuplayer = hSuplayer;
    s32Ret = MT_MPI_SUPLAYER_Get_Proc_Info(hSuplayer, &info);
    if(info == NULL){
        printf("\n[%s_%d] cant get suplayer handle !!!\n",__func__,__LINE__);
    }
    else{
        if(sizeof(SUPLAYER_ADAPTER_INFO_OUT_T) != sizeof(SUPLAYER_ADAPTER_INFO_T))
            printf("\n[%s_%d] Please check structure!!!\n",__func__,__LINE__);
        else{
            memcpy(&info->adapter, &in_info->adapter, sizeof(SUPLAYER_ADAPTER_INFO_OUT_T));
        }
        s32Ret = 0;
    }
    return s32Ret;
}

