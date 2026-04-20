/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : pvri_index.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2017/12/21
 * Description    : MT PVR DRV internal configuration.
 * History        :
 * 1.Date         : 2017/12/21
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __PVR_INDEX_H__
#define __PVR_INDEX_H__

typedef struct pvrinputParam
{
    MT_S32 (*PvrIdxCallback)(MT_S32 ChanID, MT_S32 type, mt_void* p_args);
}pvrinputParam_t;

/* external specified operations(method) */
typedef struct
{
    MT_S32 (*PvrIdxCallback)(MT_S32 ChanID, MT_S32 type, mt_void* p_args);
    pvrinputParam_t inputParam;
} PVR_IDX_OPERATION_S;

/* VDEC control command id, different function have different CID. */
typedef enum mtPVR_CID_E
{
    PVR_CID_GET_GLOBAL_STATE,      /* 0. get global state */
    PVR_CID_GET_CAPABILITY,        /* 1. get the capacity of the decoder */
    PVR_CID_GET_GLOBAL_CFG,        /* 2. get the configured info of the decoder */
    PVR_CID_CFG_DECODER,           /* 3. congfig the decoder */

    PVR_CID_STOP_CHAN

} PVR_CID_E;

/* Describe a pvr index buffer instance */
typedef struct mtPVRINDEXINST_S
{
    mt_u32        u32PhyAddr;         //start PhyAddr
    mt_u8*        pu8KnlVirAddr;      //start krnAddr
    mt_u32        u32Size;            //size of buffer
    mt_u32        status;             //1:start 0:invalid 2:running.3:stoped.app set 0 or 1. av set 2 from 1
    mt_u32        index_num;          //cell max num    maxnum=size/sizeof(cell)
    mt_u32        index_rp;           //cell rp
    mt_u32        index_wp;           //cell wp
    mt_u32        rec_dmxid;          //dmxid
    mt_u32        video_type;         //vdec type
    #if (defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)) && !defined(CONFIG_MT_SECURE_MEDIA_PATH)
    struct semaphore  IdxSem;         //protect data
    #endif
} PVR_INDEX_INST_S;

#endif

