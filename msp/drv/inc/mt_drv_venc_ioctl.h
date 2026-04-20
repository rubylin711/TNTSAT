/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
  File Name     : drv_venc_ioctl.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/3
  Description   :
  History       :
  1.Date        : 2015/12/3
    Author      : 
    Modification: Created file

******************************************************************************/

#ifndef __MT_DRV_VENC_IOCTL_H__
#define __MT_DRV_VENC_IOCTL_H__

#include "mt_debug.h"
#include "mt_unf_venc.h"
#include "mt_drv_venc.h"

#include "drv_venc_ext.h"
#include "mt_drv_struct.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


typedef struct mtVENC_BUF_OFFSET_S
{
    mt_u32 u32StrmBufOffset[2];
} VENC_BUF_OFFSET_S;

typedef struct mtVENC_CHN_INFO_S
{
    mt_void*      pStrmBufVirAddr;
    mt_void*      pStrmBufVirAddrJpeg;
    mt_u32        u32StrmBufPhyAddr;
    mt_u32        u32StrmBufPhyAddrJpeg;
    mt_u32        u32BufSize;
    mt_u32        u32BufSizeJpeg;
    mt_handle     handle;
    mt_handle     hSource;
} VENC_CHN_INFO_S;

typedef struct mtVENC_INFO_CREATE_S
{
    mt_handle              hVencChn;
    MT_BOOL                bOMXChn;
    venc_chan_cfg          stAttr;
    VENC_CHN_INFO_S        stVeInfo;
} VENC_INFO_CREATE_S;

typedef struct mt_VENC_INFO_ATTACH_S
{
    mt_handle   hVencChn;
    mt_handle   hSrc;
    mt_mod_id_e enModId;
} VENC_INFO_ATTACH_S;

typedef struct mtVENC_INFO_ACQUIRE_STREAM_S
{
    mt_handle            hVencChn;
    MT_UNF_VENC_STREAM_S stStream;
    mt_u32               u32BlockFlag;
    VENC_BUF_OFFSET_S    stBufOffSet;
    mt_void*             omx_stream_buf;                //for omxvenc
} VENC_INFO_ACQUIRE_STREAM_S;

typedef struct mtVENC_STATE_S
{
    mt_handle hVencChn;
} VENC_STATE_S;

typedef struct mtVENC_FRAME_INFO_S
{
    mt_handle                 hVencChn;
    MT_UNF_VIDEO_FRAME_INFO_S stVencFrame;
    venc_user_buf             stVencFrame_OMX;         //for omxvenc
} VENC_INFO_QUEUE_FRAME_S;


typedef struct mtVENC_SET_SRC_INFO_S
{
    mt_handle                 hVencChn;
    MT_DRV_VENC_SRC_INFO_S    stVencSrcInfo;
} VENC_SET_SRC_INFO_S;

/////////////////////////////////////////////////////// just for omxvenc
typedef struct mtVENC_GET_INFO_S
{
    mt_handle                 hVencChn;
    venc_msginfo              msg_info_omx;
} VENC_INFO_GET_MSG_S;

typedef struct mtVENC_INFO_MMZ_MAP_S
{
    mt_handle                 hVencChn;
    mt_mmz_buf_s              stVencBuf;
} VENC_INFO_MMZ_MAP_S;

typedef struct mtVENC_INFO_FLUSH_PORT_S
{
    mt_handle                 hVencChn;
    mt_u32                    u32PortIndex;
} VENC_INFO_FLUSH_PORT_S;

typedef struct mtVENC_BUFFER_S {
	unsigned int size;
	unsigned int phys_addr;
	unsigned long base;		 /*kernel logical address in use kernel*/
	unsigned long virt_addr; /* virtual user space address */
} VENC_BUFFER_S;


typedef struct mtVENC_BIT_FIRMWARE_INFO_S {
	unsigned int size;		/* size of this structure */
	unsigned int core_idx;
	unsigned int reg_base_offset;
	unsigned short bit_code[512];
} VENC_BIT_FIRMWARE_INFO_S;

typedef struct mtVENC_INST_INFO_S {
	unsigned int core_idx;
	unsigned int inst_idx;
	int inst_open_count;   /* for output only */
} VENC_INST_INFO_S;

/////////////////////////////////////////////////////// end

#define CMD_VENC_SET_CHN_ATTR _IOWR(IOC_TYPE_VENC, 0, VENC_INFO_CREATE_S)
#define CMD_VENC_GET_CHN_ATTR _IOWR(IOC_TYPE_VENC, 1, VENC_INFO_CREATE_S)

#define CMD_VENC_CREATE_CHN _IOWR(IOC_TYPE_VENC, 2, VENC_INFO_CREATE_S)
#define CMD_VENC_DESTROY_CHN _IOWR(IOC_TYPE_VENC, 3, VENC_INFO_CREATE_S)

#define CMD_VENC_ATTACH_INPUT _IOW(IOC_TYPE_VENC, 4, VENC_INFO_ATTACH_S)
#define CMD_VENC_DETACH_INPUT _IOWR(IOC_TYPE_VENC, 5, VENC_INFO_ATTACH_S)

#define CMD_VENC_ACQUIRE_STREAM _IOWR(IOC_TYPE_VENC, 6, VENC_INFO_ACQUIRE_STREAM_S)
#define CMD_VENC_RELEASE_STREAM _IOW(IOC_TYPE_VENC, 7, VENC_INFO_ACQUIRE_STREAM_S)

#define CMD_VENC_START_RECV_PIC _IOW(IOC_TYPE_VENC, 8, mt_handle)
#define CMD_VENC_STOP_RECV_PIC _IOW(IOC_TYPE_VENC, 9, mt_handle)

#define CMD_VENC_SEND_FRAME _IOW(IOC_TYPE_VENC, 0xa, VENC_INFO_ACQUIRE_STREAM_S)
#define CMD_VENC_REQUEST_I_FRAME _IOW(IOC_TYPE_VENC, 0xb, mt_handle)

#define CMD_VENC_QUEUE_FRAME _IOWR(IOC_TYPE_VENC, 0xc, VENC_INFO_QUEUE_FRAME_S)
#define CMD_VENC_DEQUEUE_FRAME _IOWR(IOC_TYPE_VENC, 0xd, VENC_INFO_QUEUE_FRAME_S)

#define CMD_VENC_SET_SRCINFO _IOWR(IOC_TYPE_VENC, 0xe, VENC_SET_SRC_INFO_S)

////////////////////////////////////////////////////////////////////////////// just for omxvenc
#define CMD_VENC_GET_MSG _IOWR(IOC_TYPE_VENC, 0xf,VENC_INFO_GET_MSG_S)
#define CMD_VENC_QUEUE_STREAM _IOWR(IOC_TYPE_VENC, 0x10,VENC_INFO_QUEUE_FRAME_S)

#define CMD_VENC_MMZ_MAP _IOWR(IOC_TYPE_VENC, 0x11, VENC_INFO_MMZ_MAP_S)
#define CMD_VENC_MMZ_UMMAP _IOWR(IOC_TYPE_VENC, 0x12, VENC_INFO_MMZ_MAP_S)

#define CMD_VENC_FLUSH_PORT _IOWR(IOC_TYPE_VENC, 0x13,VENC_INFO_FLUSH_PORT_S)


#define CMD_VENC_ALLOCATE_PHYSICAL_MEMORY	_IOWR(IOC_TYPE_VENC, 0x14, VENC_BUFFER_S)
#define CMD_VENC_FREE_PHYSICALMEMORY		_IOWR(IOC_TYPE_VENC, 0x15, VENC_BUFFER_S)
#define CMD_VENC_WAIT_INTERRUPT			_IOW(IOC_TYPE_VENC, 0x16, mt_u32)
#define CMD_VENC_SET_CLOCK_GATE			_IO(IOC_TYPE_VENC, 0x17)
#define CMD_VENC_RESET					 _IO(IOC_TYPE_VENC, 0x18)
#define CMD_VENC_GET_INSTANCE_POOL			_IOWR(IOC_TYPE_VENC, 0x19, VENC_BUFFER_S)
#define CMD_VENC_GET_COMMON_MEMORY			_IOWR(IOC_TYPE_VENC, 0x20, VENC_BUFFER_S)
#define CMD_VENC_GET_RESERVED_VIDEO_MEMORY_INFO _IO(IOC_TYPE_VENC, 0x21)
#define CMD_VENC_OPEN_INSTANCE				_IOWR(IOC_TYPE_VENC, 0x22, VENC_INST_INFO_S)
#define CMD_VENC_CLOSE_INSTANCE			_IOWR(IOC_TYPE_VENC, 0x23, VENC_INST_INFO_S)
#define CMD_VENC_GET_INSTANCE_NUM			_IOWR(IOC_TYPE_VENC, 0x24, VENC_INST_INFO_S)
/////////////////////////////////////////////////////////////////////////////// end
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif //__MT_DRV_VENC_H__
