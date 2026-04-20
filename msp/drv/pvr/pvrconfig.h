/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : pvrconfig.h
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
#ifndef __PVRCONFIG_H__
#define __PVRCONFIG_H__

/* for Symphony, only support 1 VDEC Channel */
#define CFG_PVR_CHANNEL_COUNT				1

/* Video ES Buffer Descriptor instance count */
#define CFG_PVR_IDX_INS_COUNT				4

/* Video Descriptor Buffer Size */
#define CFG_PVR_VES_DESC_BUFF_SIZE			(256 * 1024)
//For Test
//#define CFG_PVR_VES_DESC_BUFF_SIZE			(4 * 1024)

/* Video ES Buffer Descriptor Queue share memory name */
#define CFG_PVR_INDEX_SHM_NAME			"PVRIDX"

/* Shared Decoded Image queue */
/* VFMW VDEC buffers 32 frames, and queue reserved 1 */
#define CFG_PVR_IMAGE_QUEUE_SIZE			33
#define CFG_PVR_IMAGE_QUEUE_NAME			"VFMWIMG"

/* Shared Release Image Slot queue */
#define CFG_PVR_RLS_SLOT_QUEUE_SIZE		33
#define CFG_PVR_RLS_SLOT_QUEUE_NAME		"VFMWSLT"

/* Display needs buffer image slot count */
#define CFG_DISPLAY_BUFFER_IMG_SLOT_COUNT	7

/* Dump Video ES data file name */
#define CFG_DUMP_V_ES_BUF_FILE  			"/media/sda1/dump_v.es"

/* Video Driver debug log file name */
#define CFG_DEBUG_V_LOG_FILE  				"/media/sda1/drv_video.log"

#endif

