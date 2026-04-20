/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : fw_api_rpc_bridge.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2017/12/20
 * Description    : MT VFMW API RPC(with AV CPU) bridge interface.
 * History        :
 * 1.Date         : 2017/12/20
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/kthread.h>

#include "mt_type.h"
#include "rpc.h"
#include "rpc_id.h"
#include "mt_drv_ampshm.h"
#include "pvrconfig.h"
#include "mt_drv_pvr.h"
#include "pvr_index.h"
#include "mt_module_debug.h"
#include "sys_define.h"
#include "ipc.h"
#include "ipc_common.h"
#include "adec/adec_api.h"

#undef LOG_TAG
#define LOG_TAG							"VFMW_BR_PVR"
#include "log_pvr.h"

#if 1			//Release Version
#undef DEBUG
#else			//Debug Version
#ifndef DEBUG
#define DEBUG
#endif
#endif

#define DEBUG_IMAGE_QUEUE_SIZE			1024

//dump for every 100 frames
#define DEBUG_IMAGE_COUNTER				0

#define DEBUG_VFMW_BRIDGE_PVR				0
PVR_INDEX_INST_S *pvr_idx_buffer_inst = NULL;

extern mt_s32 DMX_OsiClearIndexBuf(mt_u8 index, mt_u32 data);

//VFMW RPC Bridge????Channel()


//----------------------------------------------------------------------------//
mt_void reg_set_trpp_ch_idx_wr_addr_trpp_ch_idx_waddr(mt_u8 index, mt_u32 data);
mt_void reg_set_trpp_ch_idx_rd_addr_trpp_ch_idx_raddr(mt_u8 index, mt_u32 data);

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_SECURE_MEDIA_PATH)
/* share memory allocate */
static void *_Shm_Alloc(const char *name, unsigned int size)
{
	int fd;
	int ret;

	fd = mt_ampshm_open(name, 0, 0);
	if (fd < 0)
	{
		MLOGE("_Shm_Alloc: Error, open %s shm failed!\n", name);
		return NULL;
	}

	ret = mt_ampshm_ftruncate(fd, size);
	if (ret != 0)
	{
		MLOGE("_Shm_Alloc: Error, ftruncate (%d, %s) failed!\n",fd,name);
		return NULL;
	}

	return mt_ampshm_mmap(NULL, size, 0, 0, fd, 0);
}

/* share memory free */
static void _Shm_Free(const char *name, void *ptr, unsigned int size)
{
	int ret;

	ret = mt_ampshm_munmap(ptr, size);
	ret |= mt_ampshm_unlink(name);
	if (ret != 0)
	{
		MLOGE("_Shm_Free: Error, munmap or unlink(%p %u %s) failed!\n",ptr,size,name);
	}
}
#endif

static int shm_get_valid_num(void)
{
    int i=0;
    int num=0;
    if(NULL==pvr_idx_buffer_inst){
        return 0;
    }
    for(i=0; i<CFG_PVR_IDX_INS_COUNT; i++){
        if(NULL != pvr_idx_buffer_inst[i].pu8KnlVirAddr){
            num++;
        }
    }
    return num;
}
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_SECURE_MEDIA_PATH)
/* global share memory setup */
static int shm_setup_pvr(void)
{
    if (pvr_idx_buffer_inst == NULL)
    {
        //MLOGI("+++idx:shm_setup_pvr.alloc shm\n");
        pvr_idx_buffer_inst = _Shm_Alloc(CFG_PVR_INDEX_SHM_NAME, CFG_PVR_IDX_INS_COUNT*sizeof(PVR_INDEX_INST_S));
        if (pvr_idx_buffer_inst == NULL)
        {
            return -ENOMEM;
        }
        memset(pvr_idx_buffer_inst, 0, CFG_PVR_IDX_INS_COUNT*sizeof(PVR_INDEX_INST_S));

    }
    return 0;
}
#endif

/* global share memory clear */
static int shm_clear_pvr(MT_U32 chn_num)
{
    mmz_buffer_s  mmzbuffer;
#if (defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)) && !defined(CONFIG_MT_SECURE_MEDIA_PATH)
#else
	int count = 0;
#endif
    int rec_id = 0;
    if(CFG_PVR_IDX_INS_COUNT <= chn_num){
        //MLOGI("+++idx:shm_clear_pvr.err chn_num=%x\n",chn_num);
        return -1;
    }
    /* video es descriptor */
    if((NULL != pvr_idx_buffer_inst) && pvr_idx_buffer_inst[chn_num].pu8KnlVirAddr)
    {
        mmzbuffer.startPhyAddr=pvr_idx_buffer_inst[chn_num].u32PhyAddr;
        mmzbuffer.startVirAddr=(void  *)pvr_idx_buffer_inst[chn_num].pu8KnlVirAddr;
        mmzbuffer.size=pvr_idx_buffer_inst[chn_num].u32Size;
        #if (defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)) && !defined(CONFIG_MT_SECURE_MEDIA_PATH)
          down(&pvr_idx_buffer_inst[chn_num].IdxSem);
          pvr_idx_buffer_inst[chn_num].pu8KnlVirAddr=NULL;
          up(&pvr_idx_buffer_inst[chn_num].IdxSem);
        #else
          pvr_idx_buffer_inst[chn_num].pu8KnlVirAddr=NULL;
          do {
              msleep(4);    //wait avcpu run over!
              count++;
          }while((count < 100) && (pvr_idx_buffer_inst[chn_num].status != 3));
        #endif
        //printk("+++ap.wait av stop:%x,%x\n",count,pvr_idx_buffer_inst[chn_num].status);
        mt_drv_mmz_unmap_and_release(&mmzbuffer); //release buffer
    }

    if(NULL != pvr_idx_buffer_inst){
  	    rec_id = pvr_idx_buffer_inst[chn_num].rec_dmxid;
  	    if(rec_id != 0xffffffff) {        
            DMX_OsiClearIndexBuf(rec_id, 0);
	    }
    }
    #if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_SECURE_MEDIA_PATH)
    if((NULL!=pvr_idx_buffer_inst) && (0==shm_get_valid_num())){
        _Shm_Free(CFG_PVR_INDEX_SHM_NAME, (void*)pvr_idx_buffer_inst, CFG_PVR_IDX_INS_COUNT*sizeof(PVR_INDEX_INST_S));
        pvr_idx_buffer_inst = NULL;
    }
    #endif
    return 0;
}

/* VFMW Initialize */
MT_S32 PVR_InitWithOperation(PVR_IDX_OPERATION_S *pArgs)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_SECURE_MEDIA_PATH)
    MT_INFO_PVR("-------------------\n");
    if(NULL==pvr_idx_buffer_inst){
		ipc_msg_t msg_send;
		mt_u32 ack;
        shm_setup_pvr();
		msg_send.msg_id = IPC_MSG_AUD_FW_PVR_START;
		msg_send.time_out_ms = 30000;
		ack = 1;
		return ap_send_to_av(AP_SYS_DEV_AUD, &msg_send, ack);
    }
#endif
    return 0;
}

/* VFMW Exit */
MT_S32 PVR_Exit(MT_U32 chn_cum)
{
  	MT_S32 ret = 0;
    MT_S32 flag_need_free_av=0;

    if(1==shm_get_valid_num()){
        flag_need_free_av=1;
    }
  	//disable Display's ISR when clear share memory,
  	//for Display ISR call VDEC_Bridge_ReleaseImage() while clear share memory.
  	shm_clear_pvr(chn_cum);
    
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)  || defined(CONFIG_MT_SECURE_MEDIA_PATH)
    if(1==flag_need_free_av){   //only one
		ipc_msg_t msg_send;
		mt_u32 ack;
		msg_send.msg_id = IPC_MSG_AUD_FW_PVR_STOP;
		msg_send.time_out_ms = 30000;
		ack = 1;
		return ap_send_to_av(AP_SYS_DEV_AUD, &msg_send, ack);
    }
#endif

  	return ret;
}

