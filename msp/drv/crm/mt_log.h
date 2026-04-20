/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2022, Montage LZ Technology Co., Ltd.
 *
 * File Name      : mt_log.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2022/06/29
 * Description    : MT log header file.
 * History        :
 * 1.Date         : 2022/06/29
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __INC_MT_LOG_H__
#define __INC_MT_LOG_H__

#ifdef __UBOOT__
#define MT_LOGV(...)		do{}while(0)
#define MT_LOGD(...)		do{}while(0)

#define MT_LOGE				printf
#define MT_LOGF				printf

#define MT_BUG				printf

#if 0
#define MT_LOGI				printf
#define MT_LOGW				printf

#define PRINTF				printf

#else
#define MT_LOGI(...)		do{}while(0)
#define MT_LOGW(...)		do{}while(0)

#define PRINTF(...)			do{}while(0)
#endif

/* dump log */
#define DP_LOG				printf

#elif defined(__KERNEL__)

#include <linux/printk.h>

#define MT_LOGV(...)		do{}while(0)
#define MT_LOGD(...)		do{}while(0)
#define MT_LOGI(...)		do{}while(0)
#define MT_LOGW(...)		do{}while(0)
#define MT_LOGE				printk
#define MT_LOGF				printk

#define MT_BUG				printk

#define PRINTF(...)			do{}while(0)

/* dump log */
#define DP_LOG(...)			seq_printf(s, __VA_ARGS__)

#else
/* RTOS */
#include <sys_types.h>
#include <mtos_printk.h>
#include <sys_define.h>

#define MT_LOGV(...)		do{}while(0)
#define MT_LOGD(...)		do{}while(0)
#define MT_LOGI(...)		do{}while(0)
#define MT_LOGW(...)		do{}while(0)
#define MT_LOGE				OS_PRINTF
#define MT_LOGF				OS_PRINTF

#define MT_BUG				OS_PRINTF

#define PRINTF(...)			do{}while(0)

/* dump log */
#define DP_LOG				OS_PRINTF
//#define DP_LOG(...)			do{}while(0)

#endif

#endif

