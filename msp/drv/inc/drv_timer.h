/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_M_TIMER_H__
#define __DRV_M_TIMER_H__

#include "mt_debug.h"

typedef void (*timer_fn)(void *para); 

typedef struct
{
	timer_fn p_fn;
	void *p_para;
	int  cycled;
	int timeout;
	int used;
	char * name;
    int timer_id;
	int cascade;
}mt_symp_timer_info;


typedef struct _tagtimer_cnt_s
{
	mt_u32	u32TimerIndex;
	mt_s32	u32cnt;
}timer_cnt_s;


#define MT_FATAL_TIMER(fmt...) MT_FATAL_PRINT(MT_ID_TIMER, fmt)
#define MT_ERR_TIMER(fmt...) MT_ERR_PRINT(MT_ID_TIMER, fmt)
#define MT_WARN_TIMER(fmt...) MT_WARN_PRINT(MT_ID_TIMER, fmt)
#define MT_INFO_TIMER(fmt...) MT_INFO_PRINT(MT_ID_TIMER, fmt)

#define TIMER_IOCTL_BASE MT_ID_TIMER

#define TIMERIOC_SET_START _IOR(TIMER_IOCTL_BASE, 4, int)
#define TIMERIOC_SET_STOP _IOR(TIMER_IOCTL_BASE, 5, int)
#define TIMERIOC_SET_RELEASE _IOR(TIMER_IOCTL_BASE, 6, int)
#define TIMERIOC_GET_CNT _IOR(TIMER_IOCTL_BASE, 7, timer_cnt_s)
#define TIMERIOC_REQUEST _IOR(TIMER_IOCTL_BASE, 8, mt_symp_timer_info)

mt_s32  mt_drv_timer_start(int id);
mt_s32  mt_drv_timer_stop(int id);
mt_u32  mt_drv_timer_read_cnt(int id);
mt_s32  mt_drv_timer_request(int ms, timer_fn pfn, void *para, int cycled, char *name);
mt_s32  mt_drv_timer_release(int id);

ulong mt_drv_kn_timer_request(int ms, timer_fn pfn, void *para, int cycled, char *name);
mt_s32  mt_drv_kn_timer_release(int id);

#endif


