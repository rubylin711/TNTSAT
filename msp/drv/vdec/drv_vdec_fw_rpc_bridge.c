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
#include <asm/io.h>

//#include "sys_define.h"
#include "drv_vdec_sys_define.h"

#include "vfmw.h"
#include "vfmw_reg.h"
#include "drv_vdec_ext.h"
#include "rpc.h"
#include "rpc_id.h"
#include "mt_drv_ampshm.h"
#include "queue.h"
#include "vconfig.h"
#include "drv_vdec_debug.h"

//#include "VdecAPI.h"

#undef LOG_TAG
#define LOG_TAG							"VFMW_BR"
#include "Log.h"

//see av_rtos/platform/source/firmware/symphony_avcpu/video_vsb/video_lib/vfmw.h
#ifndef DF_FREEZE_SLOT_ID
#define DF_FREEZE_SLOT_ID				0xEE
#endif

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

#define DEBUG_VFMW_BRIDGE				0

/* VFMW RPC Bridge Channel */
struct VFMW_BRIDGE_CHANNEL
{
	/* VES(ES Buffer) Number */
	unsigned int ves_ch;

	/* ES Descriptor share memory for AP/AV CPU */
	VES_INST_S *descriptor;

	//decoded image que(share memory)
	struct queue_t *image_queue;
	//release slot que(share memory)
	struct queue_t *release_queue;

	//Debug
	unsigned long rd_counter;			//read image counter
	unsigned long rls_counter;			//release image slot counter
#ifdef DEBUG
	unsigned int dbg_image_queue[DEBUG_IMAGE_QUEUE_SIZE];	//image queue for debug
#endif
};

/* VFMW RPC Bridge */
struct VFMW_BRIDGE
{
	//vdec callback type define
	SINT32 (*VdecCallback)(SINT32 ChanID, SINT32 type, VOID* p_args);

	//VFMW channels
	struct VFMW_BRIDGE_CHANNEL channel[CFG_VDEC_CHANNEL_COUNT];
};

static struct VFMW_BRIDGE gVFMWBridge;

/* channel id to channel struct pointer */
#define VFMW_BR_ID2CH(id)												\
			do {														\
				if (/*id < 0 ||*/ id >= CFG_VDEC_CHANNEL_COUNT)				\
				{														\
					MLOGE("%s: invalid ch id(%d)\n",__FUNCTION__,id);	\
					WARN(1, "%s: invalid ch id(%d)\n",__FUNCTION__,id);	\
					return -EINVAL;										\
				}														\
				pCh=&gVFMWBridge.channel[id];							\
			} while(0)

/* channel id to channel struct pointer */
#define VOID_VFMW_BR_ID2CH(id)															\
							do {														\
								if (id < 0 || id >= CFG_VDEC_CHANNEL_COUNT) 			\
								{														\
									MLOGE("%s: invalid ch id(%d)\n",__FUNCTION__,id);	\
									WARN(1, "%s: invalid ch id(%d)\n",__FUNCTION__,id); \
									return; 											\
								}														\
								pCh=&gVFMWBridge.channel[id];							\
							} while(0)

//VFMW RPC Bridge????Channel()
const static unsigned int knVFMWBridgeChannelNum = 0;

DEFINE_SPINLOCK(kVFMWBridgeLock);

/* ES Descriptor share memory for AP/AV CPU */
extern VES_INST_S *ves_buffer_inst;

static void vdec_get_chan_ves_buf_state(void *pDechdl, int channel_idx, VES_BUF_STATE_S *pVesBuf);
static void vdec_set_chan_ves_buf_state(void *pDechdl, int channel_idx, VES_BUF_STATE_S *pVesBuf);
static void vdec_reset_chan_ves_buf_state(void *pDechdl, int channel_idx);
static int shm_flush_img_queue(unsigned int ch_id);

//----------------------------------------------------------------------------//

static MT_BOOL vdec_is_dmx_source(unsigned int ch_id)
{
	VDEC_CHANNEL_S *pCh = VDEC_DRV_GetChan(ch_id);

	if (pCh != MT_NULL)
	{
		return (pCh->hDmxVidChn != MT_INVALID_HANDLE);
	}
	else
	{
		return MT_FALSE;
	}
}


//vdec event callback RPC proxy, remote called by AV CPU
static RPC_RETVAL VDEC_EventHandle_Proxy(unsigned int what, unsigned int param1, unsigned int param2, struct rpc_ext_arg_t *ext)
{
	if (DEBUG_VFMW_BRIDGE)
	{
		MLOGD("%s: (%x %x %x)\n",__FUNCTION__,what,param1,param2);
	}

	/* RESOLUTION CHANGE: flush read share image queue */
	if (param2 == EVNT_IMG_SIZE_CHANGE)
	{
		MLOGI("%s: EVNT_IMG_SIZE_CHANGE, flush image queue.\n",__FUNCTION__);
		spin_lock(&kVFMWBridgeLock);
		shm_flush_img_queue(param1);
		spin_unlock(&kVFMWBridgeLock);
	}

	if (gVFMWBridge.VdecCallback != NULL)
	{
		return (gVFMWBridge.VdecCallback)(param1, param2, NULL);
	}
	else
	{
		MLOGE("%s: Error, VdecCallback(%x %x %x) is null!\n",__FUNCTION__,what,param1,param2);
		WARN(1, "%s: Error, VdecCallback is null!\n",__FUNCTION__);
	}

	return MT_FAILURE;
}

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

/* global share memory setup */
static int shm_setup(void)
{
	struct VFMW_BRIDGE_CHANNEL *pCh = NULL;
	//FIXME: support 1 channel only
	VFMW_BR_ID2CH(knVFMWBridgeChannelNum);

	/* video es descriptor use share memory */
	if (ves_buffer_inst == NULL)
	{
		ves_buffer_inst = _Shm_Alloc(CFG_VDEC_VES_DESC_SHM_NAME, CFG_VDEC_VES_INS_COUNT*sizeof(VES_INST_S));
		if (ves_buffer_inst == NULL)
		{
			return -ENOMEM;
		}
		memset(ves_buffer_inst, 0, CFG_VDEC_VES_INS_COUNT*sizeof(VES_INST_S));
		MLOGD("%s: allocate video es descriptor share memory(%p, %d) success.\n",__FUNCTION__,
				ves_buffer_inst,CFG_VDEC_VES_INS_COUNT*sizeof(VES_INST_S));
	}

	//should move to create channel
	pCh->ves_ch = 0;
	pCh->descriptor = &ves_buffer_inst[pCh->ves_ch];

	pCh->image_queue = queue_allocate(CFG_VDEC_IMAGE_QUEUE_NAME, sizeof(IMAGE), CFG_VDEC_IMAGE_QUEUE_SIZE);
	if (pCh->image_queue == NULL)
	{
		MLOGE("shm_setup: Error, allocate image_queue failed!\n");
		return -ENOMEM;
	}

	pCh->release_queue = queue_allocate(CFG_VDEC_RLS_SLOT_QUEUE_NAME, sizeof(MT_DIS_FRAME_SLOT_INFO_T), CFG_VDEC_RLS_SLOT_QUEUE_SIZE);
	if (pCh->release_queue == NULL)
	{
		MLOGE("shm_setup: Error, allocate release_queue failed!\n");
		return -ENOMEM;
	}

	return 0;
}

/* global share memory clear */
static int shm_clear(void)
{
	struct VFMW_BRIDGE_CHANNEL *pCh = NULL;
	//FIXME: support 1 channel only
	VFMW_BR_ID2CH(knVFMWBridgeChannelNum);

	/* video es descriptor */
	if (ves_buffer_inst != NULL)
	{
		_Shm_Free(CFG_VDEC_VES_DESC_SHM_NAME, (void*)ves_buffer_inst, CFG_VDEC_VES_INS_COUNT*sizeof(VES_INST_S));
		ves_buffer_inst = NULL;

		pCh->descriptor = NULL;
	}

	if (pCh->image_queue != NULL)
	{
		queue_free(pCh->image_queue);
		pCh->image_queue = NULL;
	}

	if (pCh->release_queue != NULL)
	{
		queue_free(pCh->release_queue);
		pCh->release_queue = NULL;
	}

	return 0;
}

/* channel share memory reset
 * after stop or before start, it's safe to reset read/write pointers.
 */
static int shm_reset(unsigned int ch_id)
{
	struct VFMW_BRIDGE_CHANNEL *pCh = NULL;
	VFMW_BR_ID2CH(ch_id);

	if (pCh->image_queue != NULL)
	{
		queue_reset(pCh->image_queue);
	}

	if (pCh->release_queue != NULL)
	{
		queue_reset(pCh->release_queue);
	}

	//if dmx playback, do not touch it!
	if (!vdec_is_dmx_source(ch_id))
	{
		//VFMW stop should reset the HW,
		//but test not ok,
		//so do HW reset here too.
		vdec_reset_chan_ves_buf_state(NULL, ch_id);

		if (pCh->descriptor != NULL)
		{
			pCh->descriptor->u32KnlDescWriterOff = 0;
			pCh->descriptor->u32KnDescReadOff = 0;
		}
	}

	pCh->rd_counter = 0;
	pCh->rls_counter = 0;

	return 0;
}

//Bug: long time ago, RD/WR PTR might not be zero after Stop/Reset!
static void CHECK_VDEC_REG(unsigned int ch_id, const char *func, int line)
{
	u32 reg_rd;
	u32 reg_wr;
	u32 reg_usd;

	reg_rd = VFMW_REG_READ(REG_VDEC_CH_0_RD_PTR);
	reg_wr = VFMW_REG_READ(REG_VDEC_CH_0_WR_PTR) & 0x7FFFFFFF;
	reg_usd = VFMW_REG_READ(REG_VDEC_CH_0_USED_SZ);

	if (reg_rd != 0 || reg_wr != 0 || reg_usd != 0)
	{
		MLOGE("[*BUG]%s@%d: bad RD(0x%x) WR(0x%x) USD(0x%x) DMX WR(0x%x)!!!\n",func,line,
				VFMW_REG_READ(REG_VDEC_CH_0_RD_PTR),
				VFMW_REG_READ(REG_VDEC_CH_0_WR_PTR),
				VFMW_REG_READ(REG_VDEC_CH_0_USED_SZ),
				VFMW_REG_READ(REG_DMX_CH_0_WR_PTR));
		WARN(1, "[*BUG]%s@%d: bad RD(0x%x) WR(0x%x) USD(0x%x) DMX WR(0x%x)!!!\n",func,line,
				VFMW_REG_READ(REG_VDEC_CH_0_RD_PTR),
				VFMW_REG_READ(REG_VDEC_CH_0_WR_PTR),
				VFMW_REG_READ(REG_VDEC_CH_0_USED_SZ),
				VFMW_REG_READ(REG_DMX_CH_0_WR_PTR));
	}
}

//when received EVNT_IMG_SIZE_CHANGE, still need release all images
#if 0
/* channel share memory flush all
 * it should be safe to reset read or write pointer while still playing!
 */
static int shm_flush(unsigned int ch_id)
{
	//unused
	//VES_BUF_STATE_S buf_state;

	struct VFMW_BRIDGE_CHANNEL *pCh = NULL;
	VFMW_BR_ID2CH(ch_id);

//VFMW already do flush
/*	if (pCh->descriptor != NULL)
	{
		pCh->descriptor->u32KnlDescWriterOff = pCh->descriptor->u32KnDescReadOff;
		//pCh->descriptor->u32KnDescReadOff = 0;	//do NOT touch read pointer, it could be modified only by VFMW!
	}
*/
	if (pCh->image_queue != NULL)
	{
		queue_set_read_ptr(pCh->image_queue, queue_write_ptr(pCh->image_queue));
	}

	if (pCh->release_queue != NULL)
	{
		queue_set_write_ptr(pCh->release_queue, queue_read_ptr(pCh->release_queue));
	}

//VFMW already do flush
	//set write_pointer=read_pointer
/*	if (AVPLAY_GetStreamType() == MT_UNF_AVPLAY_STREAM_TYPE_ES)
	{
		vdec_get_chan_ves_buf_state(NULL, ch_id, &buf_state);
		buf_state.write_pointer = buf_state.read_pointer;	//do NOT touch read pointer, it could be modified only by VFMW!
		vdec_set_chan_ves_buf_state(NULL, ch_id, &buf_state);
	}
*/
	pCh->rd_counter = 0;
	pCh->rls_counter = 0;

	return 0;
}
#endif

#if 1
/* channel share memory flush image and release slot queue
 * it should be safe to reset read or write pointer while still playing!
 */
static int shm_flush_queue(unsigned int ch_id)
{
	struct VFMW_BRIDGE_CHANNEL *pCh = NULL;
	VFMW_BR_ID2CH(ch_id);

	MLOGD("%s: flush ch[%d] queue!\n",__FUNCTION__,ch_id);

	if (pCh->image_queue != NULL)
	{
		queue_set_read_ptr(pCh->image_queue, queue_write_ptr(pCh->image_queue));
	}

	if (pCh->release_queue != NULL)
	{
		if(!queue_empty(pCh->release_queue))
		{
			MLOGW("[%s] release_queue not empty, %d remain!\n",__FUNCTION__,queue_size(pCh->release_queue));
		}
		queue_set_write_ptr(pCh->release_queue, queue_read_ptr(pCh->release_queue));
	}

	if (pCh->rls_counter != pCh->rd_counter)
	{
		MLOGE("%s: Error, rls_counter(%lu) != rd_counter(%lu)\n",__FUNCTION__,pCh->rls_counter,pCh->rd_counter);
		WARN(1, "rls_counter(%lu) != rd_counter(%lu)", pCh->rls_counter,pCh->rd_counter);
	}

	pCh->rd_counter = 0;
	pCh->rls_counter = 0;

	return 0;
}

/* channel share memory flush image queue when received EVNT_IMG_SIZE_CHANGE event
 */
static int shm_flush_img_queue(unsigned int ch_id)
{
	struct VFMW_BRIDGE_CHANNEL *pCh = NULL;
	VFMW_BR_ID2CH(ch_id);

	if (pCh->image_queue != NULL)
	{
		MLOGD("%s: flush ch[%d] img queue, %d remain!\n",__FUNCTION__,ch_id,queue_size(pCh->image_queue));

		queue_set_read_ptr(pCh->image_queue, queue_write_ptr(pCh->image_queue));
	}

	//pCh->rd_counter = 0;

	return 0;
}
#endif

/* get video es buffer state */
static void vdec_get_chan_ves_buf_state(void *pDechdl, int channel_idx, VES_BUF_STATE_S *pVesBuf)
{
	u32 es_buf_rp_offset;
	u32 ves_buf_wr;
	u32 ves_buf_use;
	u32 es_buf_size;
	struct VFMW_BRIDGE_CHANNEL *pCh = NULL;

	if (channel_idx < 0 || channel_idx >= CFG_VDEC_CHANNEL_COUNT)
	{
		MLOGE("%s: invalid channel_idx(%d)\n",__FUNCTION__,channel_idx);
		WARN(1, "%s: invalid channel_idx(%d)\n",__FUNCTION__,channel_idx);
		return;
	}

	VOID_VFMW_BR_ID2CH(channel_idx);

	//FIXME: ????Channel0
	es_buf_rp_offset = VFMW_REG_READ(REG_VDEC_CH_0_RD_PTR);
	ves_buf_wr = VFMW_REG_READ(REG_VDEC_CH_0_WR_PTR);
	ves_buf_use = VFMW_REG_READ(REG_VDEC_CH_0_USED_SZ);
	//Bug: for REG_VDEC_CH_0_SZ_8 might not update yet!
	//es_buf_size = ves_buffer_inst[pCh->ves_ch].u32Size;
	BUG_ON(pCh->descriptor == NULL);
	es_buf_size = pCh->descriptor->u32Size;

	pVesBuf->read_pointer = (es_buf_rp_offset&0x3fffff)<<3;
	pVesBuf->write_pointer = (ves_buf_wr&0x3fffff)<<3;
	pVesBuf->buf_used = (ves_buf_use&0x3fffff)<<3;
	pVesBuf->buf_size = es_buf_size;
	pVesBuf->buf_left = pVesBuf->buf_size - pVesBuf->buf_used;

	if (DEBUG_VFMW_BRIDGE)
	{
		MLOGD("vdec_get_chan_ves_buf_state(ch%d):\n",channel_idx);
		MLOGD("  read_pointer: 0x%x\n",pVesBuf->read_pointer);
		MLOGD("  write_pointer: 0x%x\n",pVesBuf->write_pointer);
		MLOGD("  buf_used: 0x%x\n",pVesBuf->buf_used);
		MLOGD("  buf_size: 0x%x\n",pVesBuf->buf_size);
		MLOGD("  buf_left: 0x%x\n",pVesBuf->buf_left);
	}
}

/* set video es buffer state */
static void vdec_set_chan_ves_buf_state(void *pDechdl, int channel_idx, VES_BUF_STATE_S *pVesBuf)
{
	u32 val = (pVesBuf->write_pointer>>3)|(0x80000000);

	if (channel_idx < 0 || channel_idx >= CFG_VDEC_CHANNEL_COUNT)
	{
		MLOGE("%s: invalid channel_idx(%d)\n",__FUNCTION__,channel_idx);
		WARN(1, "%s: invalid channel_idx(%d)\n",__FUNCTION__,channel_idx);
		return;
	}

	//FIXME: ????Channel0
	VFMW_REG_WRITE(REG_VDEC_CH_0_WR_PTR, val);

	if (DEBUG_VFMW_BRIDGE)
	{
		MLOGD("vdec_set_chan_ves_buf_state(ch%d):\n",channel_idx);
		MLOGD("  write_pointer: 0x%x\n",pVesBuf->write_pointer);
	}
}

#if 1
/* reset video es buffer state */
static void vdec_reset_chan_ves_buf_state(void *pDechdl, int channel_idx)
{
	//u32 val = 0x80000000;
	VES_BUF_STATE_S buf_state;

	if (channel_idx < 0 || channel_idx >= CFG_VDEC_CHANNEL_COUNT)
	{
		MLOGE("%s: invalid channel_idx(%d)\n",__FUNCTION__,channel_idx);
		WARN(1, "%s: invalid channel_idx(%d)\n",__FUNCTION__,channel_idx);
		return;
	}

#if 0
	//FIXME: ????Channel0
	VFMW_REG_WRITE(REG_VDEC_CH_0_RD_PTR, 0);
	VFMW_REG_WRITE(REG_VDEC_CH_0_WR_PTR, val);
#else
	vdec_get_chan_ves_buf_state(NULL, channel_idx, &buf_state);
	buf_state.write_pointer = buf_state.read_pointer;	//do NOT touch read pointer, it could be modified only by VFMW!
	vdec_set_chan_ves_buf_state(NULL, channel_idx, &buf_state);
#endif
}
#endif

/* VFMW read image bridge function */
static mt_s32 VDEC_Bridge_ReadImage(SINT32 InstID, IMAGE *pImage)
{
	void *item;
	struct VFMW_BRIDGE_CHANNEL *pCh = NULL;
	VFMW_BR_ID2CH(InstID);

	spin_lock(&kVFMWBridgeLock);
	if (pCh->image_queue == NULL)
	{
		MLOGE("%s: image_queue is null!\n",__FUNCTION__);
		spin_unlock(&kVFMWBridgeLock);
		return VDEC_NO_DISP_BUFFER;
	}

	item = queue_dequeue(pCh->image_queue);
	if (item == NULL)
	{
		//MLOGW("%s: queue_dequeue image_queue failed!\n",__FUNCTION__);
		spin_unlock(&kVFMWBridgeLock);
		return VDEC_NO_DISP_BUFFER;
	}
	else
	{
		memcpy(pImage, item, sizeof(IMAGE));
	}

	if (pCh->rd_counter == 0
		|| (DEBUG_IMAGE_COUNTER != 0
			&& (pCh->rd_counter % 25) == 0))
	{
		/*MLOGI("Read - Ch[%d] Cnt %d, Rd %d, Wr %d, Slot %d, size %d\n", InstID,
				pCh->rd_counter,
				queue_read_ptr(pCh->image_queue),
				queue_write_ptr(pCh->image_queue),
				pImage->slotInfo.filedInfoTop.slot_idx,
				sizeof(IMAGE));*/
		MLOGI("Read - Ch[%d] Cnt %lu Rd %02u Wr %02u Slot %02u\n", InstID,
				pCh->rd_counter,
				queue_read_ptr(pCh->image_queue),
				queue_write_ptr(pCh->image_queue),
				pImage->slotInfo.filedInfoTop.slot_idx);

		if (DEBUG_VFMW_BRIDGE)
		{
			vdec_dump_image(pImage);
		}
	}

//Trace Image Flow
//AP Kernel Image start point::
#ifdef CONFIG_MT_DEBUG_V_IMG_FLOW
	VTRACE("\n[BEGIN][%s](%lu): counter %lu, frm_cnt %u, slot %u, pts %llu, eos %u, fps %u, rd %u, wr %u\n",__FUNCTION__,gettid(),
			pCh->rd_counter,
			pImage->slotInfo.frm_cnt,
			pImage->slotInfo.filedInfoTop.slot_idx,
			pImage->slotInfo.pts,
			pImage->slotInfo.end_of_stream_flag,
			pImage->slotInfo.input_rate,
			queue_read_ptr(pCh->image_queue),
			queue_write_ptr(pCh->image_queue));
#endif

#ifdef DEBUG
	pCh->dbg_image_queue[pCh->rd_counter % DEBUG_IMAGE_QUEUE_SIZE]
		= pImage->slotInfo.filedInfoTop.slot_idx;
#endif
	pCh->rd_counter ++;
	spin_unlock(&kVFMWBridgeLock);

	//dump_stack();

	return VDEC_OK;
}

//Note: maybe called in Display ISR!
//      And should not call down/up!
/* VFMW release image slot bridge function */
static SINT32 VDEC_Bridge_ReleaseImage(SINT32 InstID, void *pInfo)
{
#ifdef DEBUG
	int i;
	int idx;
#endif
	int ret;
	int check_ignore = 0;	//for special cases, ignore counter checking
	MT_DIS_FRAME_SLOT_INFO_T *pImageSlotInfo = (MT_DIS_FRAME_SLOT_INFO_T*)pInfo;
	struct VFMW_BRIDGE_CHANNEL *pCh = NULL;
	VFMW_BR_ID2CH(InstID);

	if (InstID < 0 || InstID >= CFG_VDEC_CHANNEL_COUNT)
	{
		MLOGE("%s: invalid InstID(%d)\n",__FUNCTION__,InstID);
		WARN(1, "%s: invalid InstID(%d)\n",__FUNCTION__,InstID);
		return VDEC_OK;
	}

	if (pCh->release_queue == NULL)
	{
		MLOGE("%s: release_queue is null!\n",__FUNCTION__);
		return VDEC_OK;
	}

	//special process for release freeze frame buffer
	//for freeze frame buffer could not been released in DISP ISR, or the IPC INT may lost!
	if (pImageSlotInfo->filedInfoTop.slot_idx == DF_FREEZE_SLOT_ID)
	{
		check_ignore = 1;
	}

	/* VFMW VDEC stop - sync,
	 * but Display Stop/Clean all frames is async,
	 * after VFMW VDEC stopped,
	 * Display may call release image!
	 */
	if (pCh->rls_counter >= pCh->rd_counter && !check_ignore)
	{
		MLOGW("%s: Warning, InstID(%d) - Release counter(%lu) >= Read counter(%lu)!\n",__FUNCTION__,InstID,
				pCh->rls_counter, pCh->rd_counter);
		//WARN(1, "%s: Error, InstID(%d) - Release counter >= Read counter!\n",__FUNCTION__,InstID);
		return VDEC_OK;
	}

//Modified-0623
//too much print log in Display ISR, will effect IPC ISR!
#ifdef DEBUG
	if ((pCh->dbg_image_queue[pCh->rls_counter % DEBUG_IMAGE_QUEUE_SIZE]
		!= pImageSlotInfo->filedInfoTop.slot_idx)
		&& !check_ignore)
	{
		MLOGW("%s: Warning, InstID(%d) - Release counter(%lu)'s slot index(%u) != image queue's slot index(%u)!\n",__FUNCTION__,InstID,
				pCh->rls_counter,
				pImageSlotInfo->filedInfoTop.slot_idx,
				pCh->dbg_image_queue[pCh->rls_counter % DEBUG_IMAGE_QUEUE_SIZE]);
		//WARN(1, "%s: Error, InstID(%d) - Release slot index != image queue slot index!\n",__FUNCTION__,InstID);

		i = pCh->rls_counter;
		for (; i<pCh->rd_counter; i++)
		{
			idx = i % DEBUG_IMAGE_QUEUE_SIZE;
			MLOGD("%d[%d]: slot idx %u\n",i,idx,pCh->dbg_image_queue[idx]);
		}
		return VDEC_OK;
	}
#endif

	ret = queue_enqueue(pCh->release_queue, (void*)pImageSlotInfo, sizeof(MT_DIS_FRAME_SLOT_INFO_T));
	if (ret != 0)
	{
		MLOGE("%s: queue_enqueue release_queue failed!\n",__FUNCTION__);
		//WARN(1, "%s: queue_enqueue release_queue failed!\n",__FUNCTION__);
	}

	if (pCh->rls_counter == 0
		|| (DEBUG_IMAGE_COUNTER != 0
			&& (pCh->rls_counter % 25) == 0)
		|| (pImageSlotInfo->filedInfoTop.slot_idx == DF_FREEZE_SLOT_ID))
	{
		MLOGI("Rls  - Ch[%d] Cnt %lu Rd %02u Wr %02u Slot %02u\n",InstID,
				pCh->rls_counter,
				queue_read_ptr(pCh->release_queue),
				queue_write_ptr(pCh->release_queue),pImageSlotInfo->filedInfoTop.slot_idx);
	}
//Trace Image Flow
#ifdef CONFIG_MT_DEBUG_V_IMG_FLOW
	VTRACE("[END][%s](%lu): counter %lu, frm_cnt %u, slot %u, pts %llu, eos %u, rd %u, wr %u\n",__FUNCTION__,0/*isr*/,
			pCh->rls_counter,
			pImageSlotInfo->frm_cnt,
			pImageSlotInfo->filedInfoTop.slot_idx,
			pImageSlotInfo->pts,
			pImageSlotInfo->end_of_stream_flag,
			queue_read_ptr(pCh->release_queue),
			queue_write_ptr(pCh->release_queue));
#endif

	if (!check_ignore)
		pCh->rls_counter ++;

	//dump_stack();

	return VDEC_OK;
}

//----------------------------------------------------------------------------//

/* VFMW Initialize */
SINT32 KERN_VDEC_InitWithOperation(VDEC_OPERATION_S *pArgs)
{
	SINT32 ret;
	struct rpc_ext_arg_t rpc_arg;

	ENTER_FUNCTION;
	memset(&rpc_arg, 0, sizeof(rpc_arg));

	memset(&gVFMWBridge, 0, sizeof(gVFMWBridge));
	gVFMWBridge.VdecCallback = pArgs->inputParam.VdecCallback;

	//register remote call proxy functions
	rpc_register(RPC_AP_FN_ID_VDEC_CALLBACK, VDEC_EventHandle_Proxy);

	shm_setup();

	rpc_arg.size = sizeof(VDEC_OPERATION_S);
	rpc_arg.arg = pArgs;

	ret = rpc_call_to(RPC_AV_FN_ID_VDEC_FW_INIT,
		  				0, 0, 0,
		  				&rpc_arg,
		  				RPC_CALL_TIMEOUT_DEFAULT);

	LEAVE_FUNCTION;
	return ret;
}

/* VFMW IO Control */
SINT32 KERN_VDEC_Control(SINT32 ChanID, VDEC_CID_E eCmdID, VOID *pArgs)
{
	SINT32 ret;
	unsigned long flags;
	struct rpc_ext_arg_t rpc_arg;

	if (DEBUG_VFMW_BRIDGE)
	{
		MLOGD("%s: (%x %x %p)\n",__FUNCTION__,ChanID,eCmdID,pArgs);
	}

	memset(&rpc_arg, 0, sizeof(rpc_arg));

	switch (eCmdID)
	{
		case VDEC_CID_GET_CAPABILITY:
		{
			rpc_arg.arg = pArgs;
			rpc_arg.size = sizeof(VDEC_CAP_S);
			break;
		}

		case VDEC_CID_CREATE_CHAN_WITH_OPTION:
		{
			struct VFMW_BRIDGE_CHANNEL *pCh = NULL;
			//FIXME: only transfer ChannelInfo_t, lost other 2 arguments,
			//maybe cause issue later?
			u32 *u32Ptr = (u32*)pArgs;
			ChannelInfo_t *pChannelInfo = (ChannelInfo_t*)u32Ptr[2];

			//vdec_dump_chopt(pChannelInfo);

			rpc_arg.arg = (void*)pChannelInfo;
			rpc_arg.size = sizeof(ChannelInfo_t);

			//update VES channel id
			//FIXME: support 1 channel only
			VFMW_BR_ID2CH(knVFMWBridgeChannelNum);
			pCh->ves_ch = pChannelInfo->es_buffer.chan_id & 0x0F;
			if (/*pCh->ves_ch < 0 ||*/ pCh->ves_ch >= CFG_VDEC_VES_INS_COUNT)
			{
				MLOGE("%s: Invalid VES channel id(%x)!\n",__FUNCTION__,pChannelInfo->es_buffer.chan_id);
				WARN(1, "Invalid VES channel id(%x)!",pChannelInfo->es_buffer.chan_id);
				return MT_FAILURE;
			}
			pCh->descriptor = &ves_buffer_inst[pCh->ves_ch];

			//FIXME: support 1 channel only
			//argument 0 return channel number
			u32Ptr[0] = knVFMWBridgeChannelNum;
			break;
		}

		case VDEC_CID_DESTROY_CHAN_WITH_OPTION:
		{
			//do nothing
			break;
		}

		case VDEC_CID_GET_CHAN_CFG:
		case VDEC_CID_CFG_CHAN:
		{
			if (VDEC_CID_CFG_CHAN == eCmdID)
			{
				//vdec_dump_chcfg((VDEC_CHAN_CFG_S*)pArgs);
			}

			rpc_arg.arg = pArgs;
			rpc_arg.size = sizeof(VDEC_CHAN_CFG_S);
			break;
		}

		case VDEC_CID_START_CHAN:
		{
			//Bug: long time ago, RD/WR PTR might not be zero before Start!
			CHECK_VDEC_REG(ChanID, __FUNCTION__, __LINE__);

			//if stop/start, need reset shm
			//if dmx playback, do not touch r/w pointer register!
			//if (AVPLAY_GetStreamType() == MT_UNF_AVPLAY_STREAM_TYPE_ES)
			{
				shm_reset(ChanID);
			}

			rpc_arg.arg = pArgs;
			rpc_arg.size = sizeof(MEM_DESC_S);
			break;
		}

		case VDEC_CID_STOP_CHAN:
		case VDEC_CID_RESET_CHAN:
		case VDEC_CID_SET_VDEC_SEEK_DONE:
		{
			//do nothing
			break;
		}

		case VDEC_CID_GET_IMAGE_INTF:
		{
			IMAGE_INTF_S *pstImageIntf = (IMAGE_INTF_S*)pArgs;
			pstImageIntf->image_provider_inst_id = ChanID;
			pstImageIntf->read_image = VDEC_Bridge_ReadImage;
			pstImageIntf->release_image = VDEC_Bridge_ReleaseImage;

			rpc_arg.arg = pArgs;
			rpc_arg.size = sizeof(IMAGE_INTF_S);
			break;
		}

		case VDEC_CID_GET_CHAN_DETAIL_MEMSIZE_WITH_OPTION:
		{
			//FIXME: only transfer DETAIL_MEM_SIZE, lose other 2 arguments,
			//maybe cause issue later?
			int ret;
			u32 *u32Ptr = (u32*)pArgs;
			/*
			  union {
			      u32 data[32];
			      DETAIL_MEM_SIZE dt_mem_size;
			  };
			 */
			u32 data[32];
			DETAIL_MEM_SIZE *pdtMemSize = (DETAIL_MEM_SIZE*)data;

          memset(data, 0, sizeof(data));

			/* pointer to value */
			data[0] = u32Ptr[0];			/* not used */
			data[1] = u32Ptr[1];			/* not used */
			data[2] = *((u32*)u32Ptr[2]);
			data[3] = *((u32*)u32Ptr[3]);
			data[4] = *((u32*)u32Ptr[4]);
			data[5] = *((u32*)u32Ptr[5]);
			data[6] = *((u32*)u32Ptr[6]);
			data[7] = *((u32*)u32Ptr[7]);

			rpc_arg.arg = (void*)data;
			rpc_arg.size = sizeof(data);

			ret = rpc_call(RPC_AV_FN_ID_VDEC_FW_IO_CTRL,
							0, ChanID, eCmdID,
							&rpc_arg);

			//copy back
			if (ret == 0)
			{
				memcpy(pArgs, pdtMemSize, sizeof(DETAIL_MEM_SIZE));
			}
			else
			{
				MLOGE("%s: rpc_call(GET_CHAN_DETAIL_MEMSIZE) failed, return %d\n",__FUNCTION__,ret);
			}

			return ret;
			break;
		}

		case VDEC_CID_GET_CHAN_VES_BUF_STATE:
		{
			vdec_get_chan_ves_buf_state(NULL, ChanID, (VES_BUF_STATE_S*)pArgs);
			return MT_SUCCESS;
			break;
		}

		case VDEC_CID_SET_CHAN_VES_BUF_STATE:
		{
			vdec_set_chan_ves_buf_state(NULL, ChanID, (VES_BUF_STATE_S*)pArgs);
			return MT_SUCCESS;
			break;
		}

		case VDEC_CID_GET_VDEC_ISR:
		{
			//do nothing
			break;
		}

		case VDEC_CID_SET_VDEC_RESOLUTION_DONE:
		{
//when received EVNT_IMG_SIZE_CHANGE, still need release all images
#if 1
			//flush queue only
			{
				spin_lock_irqsave(&kVFMWBridgeLock, flags);
				shm_flush_queue(ChanID);
				//shm_flush(ChanID);
				spin_unlock_irqrestore(&kVFMWBridgeLock, flags);
			}
#endif
			rpc_arg.arg = pArgs;
			rpc_arg.size = sizeof(MT_BOOL);
			MLOGD("%s: VDEC_CID_SET_VDEC_RESOLUTION_DONE\n",__FUNCTION__);
			break;
		}

		case VDEC_CID_GET_DCE_PERCENT:
		case VDEC_CID_SET_FORCE_FRAME_RATE:
		{
			rpc_arg.arg = pArgs;
			rpc_arg.size = sizeof(u32);
			break;
		}

        case VDEC_CID_START_USRDAT:
		{
			rpc_arg.arg = pArgs;
			rpc_arg.size = sizeof(u32);
			break;
		}

		case VDEC_CID_PAUSE_CHAN:
		case VDEC_CID_RESUME_CHAN:
		{
			//do nothing
			break;
		}

		case VDEC_CID_AC_FREEZE_BUFF:
		case VDEC_CID_RLS_FREEZE_BUFF:
		{
			rpc_arg.arg = pArgs;
			rpc_arg.size = sizeof(MT_DIS_FRAME_SLOT_INFO_T);
			break;
		}
		case VDEC_CID_GET_DECODING_CAPABILITY:
		{
			rpc_arg.arg = pArgs;
			rpc_arg.size = sizeof(FW_VDEC_CAPABILITY_INFO_S);
			break;
		}
		case VDEC_CID_SET_DEC_MODE:
		{
			rpc_arg.arg = pArgs;
			rpc_arg.size = sizeof(DEC_MODE_E);
			break;
		}
		case VDEC_CID_SET_TRICK_MODE:
		{
			rpc_arg.arg = pArgs;
			rpc_arg.size = sizeof(fw_trick_mode_info_t);
			break;
		}
		case VDEC_CID_GET_CHAN_STATE:
		{
			rpc_arg.arg = pArgs;
			rpc_arg.size = sizeof(VDEC_CHAN_STATE_S);
			break;
		}
		default:
			MLOGW("%s: Warning, unsupport cmd(%d)\n",__FUNCTION__,eCmdID);
			WARN(1, "%s: Warning, unsupport cmd(%d)\n",__FUNCTION__,eCmdID);
			return MT_FAILURE;
			break;
	}

	ret = rpc_call(RPC_AV_FN_ID_VDEC_FW_IO_CTRL,
					0, ChanID, eCmdID,
					&rpc_arg);

	//post process
	//if dmx playback, do not touch r/w pointer register!
	if (eCmdID == VDEC_CID_STOP_CHAN
		/*&& AVPLAY_GetStreamType() == MT_UNF_AVPLAY_STREAM_TYPE_ES*/)
	{
		//Bug: long time ago, RD/WR PTR might not be zero after Stop/Reset!
		CHECK_VDEC_REG(ChanID, __FUNCTION__, __LINE__);

		//disable Display's ISR when operate share memory,
		//for Display ISR call VDEC_Bridge_ReleaseImage() while operate share memory.
		spin_lock_irqsave(&kVFMWBridgeLock, flags);
		shm_reset(ChanID);
		spin_unlock_irqrestore(&kVFMWBridgeLock, flags);
	}

//VFMW reset still needs Display release images.
#if 0
	//if dmx playback, do not touch r/w pointer register!
	if (eCmdID == VDEC_CID_RESET_CHAN
		/*&& AVPLAY_GetStreamType() == MT_UNF_AVPLAY_STREAM_TYPE_ES*/)
	{
		//disable Display's ISR when operate share memory,
		//for Display ISR call VDEC_Bridge_ReleaseImage() while operate share memory.
		spin_lock_irqsave(&kVFMWBridgeLock, flags);
		shm_flush(ChanID);
		spin_unlock_irqrestore(&kVFMWBridgeLock, flags);
	}
#endif

	return ret;
}

/* VFMW Exit */
SINT32 VDEC_Exit(VOID)
{
	SINT32 ret;
	unsigned long flags;

	ENTER_FUNCTION;

	ret = rpc_call(RPC_AV_FN_ID_VDEC_FW_EXIT,
	  				0, 0, 0,
	  				NULL);

	//disable Display's ISR when clear share memory,
	//for Display ISR call VDEC_Bridge_ReleaseImage() while clear share memory.
	spin_lock_irqsave(&kVFMWBridgeLock, flags);
	shm_clear();
	spin_unlock_irqrestore(&kVFMWBridgeLock, flags);

	LEAVE_FUNCTION;
	return ret;
}

