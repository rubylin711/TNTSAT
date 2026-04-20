/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : vconfig.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2017/12/21
 * Description    : MT VDEC DRV internal configuration.
 * History        :
 * 1.Date         : 2017/12/21
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __MT_VDEC_IN_CONFIG_H__
#define __MT_VDEC_IN_CONFIG_H__

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
#define CFG_VFMW_ON_AVCPU					1
#else
/* Aria & Symphony4 */
#define CFG_VFMW_ON_AVCPU					0
#endif

/* just for clean compile warnings */
#define MT_VDEC_HD_SIMPLE					0
#define MT_VDEC_SVDEC_SUPPORT				0
#ifndef MT_VDEC_VPU_SUPPORT
#define MT_VDEC_VPU_SUPPORT					0
#endif

/* Aria/Symphony4 */
#define CFG_VDEC_RUN_CPU0					1
#define CFG_VDEC_THREAD_PRO					99
#define CFG_VDEC_THREAD_SCH					SCHED_RR

#ifdef CONFIG_MT_CHIP_ARIA
/* Aria VDEC IRQ Number */
#define VDEC_IRQ_NUM						(62 + 32)
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
#define VDEC_IRQ_NUM						IRQ_VDEC_ID
#elif defined(CONFIG_MT_CHIP_SYMPHONY6)   
#define VDEC_IRQ_NUM						IRQ_VDEC_ID
#endif

#ifdef CONFIG_MT_CHIP_ARIA
#define CFG_VDEC_VDH_STATIC_ALLOCATE		0
#else
/* for Symphony, VES VDH buffer static allocated and never free */
#define CFG_VDEC_VDH_STATIC_ALLOCATE		1
#endif

/* for Symphony, only support 1 VDEC Channel */
#define CFG_VDEC_CHANNEL_COUNT				1

/* Video ES Buffer Descriptor instance count */
#define CFG_VDEC_VES_INS_COUNT				8

/* Video Descriptor Buffer Size */
#ifdef CONFIG_MT_CHIP_ARIA
#define CFG_VDEC_VES_DESC_BUFF_SIZE			(1024 * 1024)
#else
#define CFG_VDEC_VES_DESC_BUFF_SIZE			(256 * 1024)
#endif

#define CFG_VDEC_USERDATA_CC_BUFFER_SIZE	(140*1024)

/* Video ES Buffer Descriptor Queue share memory name */
#define CFG_VDEC_VES_DESC_SHM_NAME			"VESDESC"

/* Shared Decoded Image queue */
/* VFMW VDEC buffers 32 frames, and queue reserved 1 */
#define CFG_VDEC_IMAGE_QUEUE_SIZE			33
#define CFG_VDEC_IMAGE_QUEUE_NAME			"VFMWIMG"

/* Shared Release Image Slot queue */
#define CFG_VDEC_RLS_SLOT_QUEUE_SIZE		33
#define CFG_VDEC_RLS_SLOT_QUEUE_NAME		"VFMWSLT"

/* Dump Video file name prefix */
#define CFG_DUMP_FILE_PREFIX  				"/media/sda1/"

/* Dump Video ES data file name */
#define CFG_DUMP_V_ES_BUF_FILE  			CFG_DUMP_FILE_PREFIX "dump_v.es"

/* Video Driver debug log file name */
#define CFG_DEBUG_V_LOG_FILE  				CFG_DUMP_FILE_PREFIX "drv_video.log"

/*****************************************************************************/
/* VFMW Parameters */

/*
 * Display Quality - Video presentation fluency(0-3)
 * 0: less fluently
 * 3: most fluently, but need more slot frame buffer.
 * 128M: default 0,
 * 256M: default 1.
 */
#ifndef CONFIG_MT_VIDEO_FLUENCY
#define CONFIG_MT_VIDEO_FLUENCY				0
#endif
#define VFMW_VIDEO_FLUENCY					CONFIG_MT_VIDEO_FLUENCY

/* Display needs buffer image slot count */
#define VFMW_DISP_SLOT_COUNT_BASE			4	/* 5 frame at least */
#define VFMW_PARAMETER_DISP_SLOT_COUNT		(VFMW_DISP_SLOT_COUNT_BASE \
											 + VFMW_VIDEO_FLUENCY)

/* hevc */
#ifdef CONFIG_MT_VDEC_HEVC
#define VFMW_PARAMETER_HEVC					1
#else
#define VFMW_PARAMETER_HEVC					0
#endif

/* hevc 10bit */
#ifdef CONFIG_MT_VDEC_HEVC_10BIT
#define VFMW_PARAMETER_HEVC_10BIT			1
#else
#define VFMW_PARAMETER_HEVC_10BIT			0
#endif

/* vp9 */
#ifdef CONFIG_MT_VDEC_VP9
#define VFMW_PARAMETER_VP9					1
#else
#define VFMW_PARAMETER_VP9					0
#endif

/* lossy compress */
#ifdef CONFIG_MT_VIDEO_LOSSY_COMPRESS
#define VFMW_PARAMETER_LOSSY_COMPRESS		1
#else
#define VFMW_PARAMETER_LOSSY_COMPRESS		0
#endif

#ifdef CONFIG_MT_VDEC_DVIEW
#define CONFIG_SUPPORT_VDEC_DVIEW			1
#else
#define CONFIG_SUPPORT_VDEC_DVIEW			0
#endif

#ifdef CONFIG_MT_VDEC_4K
#define VFMW_PARAMETER_SUPPORT_4K			1
#else
#define VFMW_PARAMETER_SUPPORT_4K			0
#endif
/*
 * lossy compress quality level
 *   0: best quality,
 *   2: worst quality
 *   default: 0
 */
#ifndef CONFIG_MT_VIDEO_LOSSY_QUALITY
#define CONFIG_MT_VIDEO_LOSSY_QUALITY		0
#endif
#define VFMW_PARAMETER_LOSSY_QUALITY		CONFIG_MT_VIDEO_LOSSY_QUALITY

/* DASH dynamic resotion change */
#ifdef CONFIG_MT_VIDEO_DYNAMIC_RESOLUTION
#define VFMW_PARAMETER_DYNAMIC_RES			1
#else
#define VFMW_PARAMETER_DYNAMIC_RES			0
#endif

#endif

