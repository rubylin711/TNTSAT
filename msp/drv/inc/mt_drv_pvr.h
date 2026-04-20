/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_DRV_PVR_H__
#define __MT_DRV_PVR_H__

#include "mt_type.h"
#ifdef __KERNEL__
#include "mt_drv_mmz.h"
#else
#include "mt_common.h"
#include "drv_userproc_ioctl.h"
#endif
#include "mt_unf_demux.h"

/* definition of max play channel */
#define PVR_PLAY_MAX_CHN_NUM           5 

/* definition of max record channel */
#define PVR_REC_MAX_CHN_NUM            10 
#define PVR_REC_START_NUM               PVR_PLAY_MAX_CHN_NUM

#define CMD_PVR_INIT_PLAY               _IOR(MT_ID_PVR, 0x01, MT_U32)
#define CMD_PVR_CREATE_PLAY_CHN         _IOR(MT_ID_PVR, 0x02, MT_U32)
#define CMD_PVR_DESTROY_PLAY_CHN        _IOW(MT_ID_PVR, 0x03, MT_U32)

#define CMD_PVR_INIT_REC                _IOR(MT_ID_PVR, 0x11, MT_U32)
#define CMD_PVR_CREATE_REC_CHN          _IOR(MT_ID_PVR, 0x12, MT_U32)
#define CMD_PVR_DESTROY_REC_CHN         _IOW(MT_ID_PVR, 0x13, MT_U32)

#define CMD_PVR_REC_CREATE_IDX_SHM         _IOW(MT_ID_PVR, 0x21, MT_U32)
#define CMD_PVR_REC_DESTROY_IDX_SHM         _IOW(MT_ID_PVR, 0x22, MT_U32)
#define CMD_PVR_REC_WRITE_DATA         _IOWR(MT_ID_PVR, 0x23, MT_UNF_DMX_REC_INDEX_S)
#define CMD_PVR_REC_READ_DATA         _IOWR(MT_ID_PVR, 0x24, MT_UNF_DMX_REC_INDEX_S)
#define CMD_PVR_REC_CONFIG_IDX_SHM         _IOW(MT_ID_PVR, 0x25, PVRINDEXCFG_S)
#define CMD_PVR_REC_READ_DATA_EX         _IOWR(MT_ID_PVR, 0x26, MT_UNF_DMX_REC_INDEX_ARRY_S)
#define CMD_PVR_REC_READ_LAST_INDEX _IOWR(MT_ID_PVR, 0x27, MT_UNF_DMX_REC_INDEX_S)

/**index data*/
/**CNcomment: Ë÷ÒýÊý¾Ý10*/
typedef struct mtUNF_DMX_REC_INDEX_ARRY_S
{
    MT_UNF_DMX_REC_INDEX_S index[10];               /*arry of index*/
} MT_UNF_DMX_REC_INDEX_ARRY_S;

typedef enum mtPVRShmDataType{
    PVR_SHM_DATA_IDX,
    PVR_SHM_DATA_BUFF
}mtPVRShmDataType_T;

typedef enum mtPVRShmMemOwnerType{
    PVR_SHM_MEM_INIT,
    PVR_SHM_MEM_AP,
    PVR_SHM_MEM_AV
}mtPVRShmMemOwnerType_T;

#endif

