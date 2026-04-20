/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : Log.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2017/12/09
 * Description    : MT Log.
 * History        :
 * 1.Date         : 2017/12/09
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __LOG_PVR_H__
#define __LOG_PVR_H__

#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/timekeeping.h>
#endif

/* uCOS
u32 s, ms, us;
mtos_systime_get(&s,&ms,&us)
*/
#define LOG_TIME_FMT			"%u.%03u%03u "
/* Linux Kernel
getrawmonotonic(struct timespec *ts)
*/
#define LOG_TIME_FMT2			"%u.%06u "
//#define LOG_TIME				"10.123456 "

//level
#define LOG_LV_D				"D/"
#define LOG_LV_I				"I/"
#define LOG_LV_W				"W/"
#define LOG_LV_E				"E/"

//cpu
#define LOG_CPU_AP				"[AP]"
#define LOG_CPU_AV				"[AV]"

//module tag
#ifndef LOG_TAG
#define LOG_TAG					"null"
#endif

//task id
#define LOG_TID_FMT				"(%3u)"

#ifdef __KERNEL__
#define LOG_TID					current->pid

#define MMLOG_TS(LOG_LV, LOG_CPU, fmt, ...)	do {									\
												u32 s=0, us=0;						\
												struct timespec tp;					\
												getrawmonotonic(&tp);				\
												s = tp.tv_sec;						\
												us = tp.tv_nsec / 1000;				\
												printk(LOG_TIME_FMT2 LOG_LV LOG_CPU LOG_TAG LOG_TID_FMT ": " fmt, s, us, LOG_TID, ## __VA_ARGS__);	\
											} while(0)

//no timestamp
#define MMLOG(LOG_LV, LOG_CPU, fmt, ...)	do {									\
												printk(LOG_LV LOG_CPU LOG_TAG LOG_TID_FMT ": " fmt, LOG_TID, ## __VA_ARGS__);	\
											} while(0)

											

#define MLOGD(fmt, ...)			MMLOG(LOG_LV_D, LOG_CPU_AP, fmt, ## __VA_ARGS__)
#define MLOGI(fmt, ...)			MMLOG(LOG_LV_I, LOG_CPU_AP, fmt, ## __VA_ARGS__)
#define MLOGW(fmt, ...)			MMLOG(LOG_LV_W, LOG_CPU_AP, fmt, ## __VA_ARGS__)
#define MLOGE(fmt, ...)			MMLOG(LOG_LV_E, LOG_CPU_AP, fmt, ## __VA_ARGS__)
#else
#define LOG_TID					mtos_task_get_curn_prio()

#define MMLOG(LOG_LV, LOG_CPU, fmt, ...)	do {									\
												u32 s=0, ms=0, us=0;				\
												mtos_systime_get(&s,&ms,&us);		\
												AV_PRINTF(LOG_TIME_FMT LOG_LV LOG_CPU LOG_TAG LOG_TID_FMT ": " fmt, s, ms, us, LOG_TID, ## __VA_ARGS__);	\
											} while(0)

#define MLOGD(fmt, ...)			MMLOG(LOG_LV_D, LOG_CPU_AV, fmt, ## __VA_ARGS__)
#define MLOGI(fmt, ...)			MMLOG(LOG_LV_I, LOG_CPU_AV, fmt, ## __VA_ARGS__)
#define MLOGW(fmt, ...)			MMLOG(LOG_LV_W, LOG_CPU_AV, fmt, ## __VA_ARGS__)
#define MLOGE(fmt, ...)			MMLOG(LOG_LV_E, LOG_CPU_AV, fmt, ## __VA_ARGS__)
#endif

#endif

