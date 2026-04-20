/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <memory.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <sys/time.h>
#include "mt_common.h"
#include "mt_drv_struct.h"
#include "mt_mpi_stat.h"
#include "drv_stat_ioctl.h"
#include "mt_module.h"
#include "mt_osal.h"
#include "mt_mpi_mem.h"
//#define __STAT_USE_HW_TIMER__

#define CALLING_USED
mt_s32 s_s32StatFd = -1;
#ifdef __STAT_USE_HW_TIMER__
volatile mt_void * g_Timer7_addr = NULL;
#endif

static pthread_mutex_t   s_StatMutex = PTHREAD_MUTEX_INITIALIZER;

#define MT_STAT_LOCK()     (void)pthread_mutex_lock(&s_StatMutex);
#define MT_STAT_UNLOCK()   (void)pthread_mutex_unlock(&s_StatMutex);

mt_u32 stat_get_timer0_addr(mt_void);
mt_s32 MT_MPI_STAT_ThreadUnregister(mt_stat_handle * pHandle);
mt_s32 MT_MPI_STAT_ThreadProbe(mt_stat_handle handle);

mt_u32 stat_get_timer0_addr(mt_void)
{
    mt_sys_version_s stSysChipInfo;
    mt_u32 u32RegisterAddr = 0;

    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));

    if (mt_sys_get_version(&stSysChipInfo))
    {
        return 0;
    }

#ifdef CONFIG_MT_CHIP_ARIA
    if ( (stSysChipInfo.enChipTypeHardWare != MT_CHIP_TYPE_BUTT) && (stSysChipInfo.enChipVersion == MT_CHIP_VERSION_V300) )
    {
        u32RegisterAddr = 0x10206020;
    }
    else
    {
        u32RegisterAddr = 0x101e5020;
    }
#endif

    return u32RegisterAddr;
}

mt_s32 mt_mpi_stat_init(mt_void)
{
#ifdef __STAT_USE_HW_TIMER__
    mt_u32 RegAddr;
    mt_u32 RegValue;
#endif

    MT_STAT_LOCK();

    if(s_s32StatFd == -1)
    {
        s_s32StatFd = open("/dev/"UMAP_DEVNAME_STAT, O_RDWR | O_CLOEXEC);
        if(-1 == s_s32StatFd)
        {
            MT_STAT_UNLOCK();
            MT_ERR_STAT("ERROR: can not open stat device.\n");
            return MT_FAILURE;
        }

#ifdef __STAT_USE_HW_TIMER__
        if(g_Timer7_addr == NULL)
        {
            RegAddr = stat_get_timer0_addr();
            if (0 == RegAddr)
            {
                close(s_s32StatFd);
                s_s32StatFd = -1;
                MT_STAT_UNLOCK();
                return MT_FAILURE;
            }

            g_Timer7_addr = (mt_void *)mt_mmap(RegAddr, 4000);
            if (NULL == g_Timer7_addr)
            {
                close(s_s32StatFd);
                s_s32StatFd = -1;

                MT_STAT_UNLOCK();

                MT_ERR_STAT("ERROR: map timer reg address error.\n");
                return MT_FAILURE;
            }

            RegValue = 0xffffffff;
            MT_REG_WRITE32((mt_u32 *)g_Timer7_addr, RegValue);

            /* config timer control, offset 0x28 */
            MT_REG_READ32((mt_u32 *)((mt_u32)g_Timer7_addr+8), RegValue);

            /* reserved high 24-bit and 4th-bit */
            RegValue &= 0xffffff10;

            /* config timer control register. ref TIMERx_CONTROL */
            RegValue |= 0x000000e2;
            MT_REG_WRITE32((mt_u32 *)((mt_u32)g_Timer7_addr+8), RegValue);

            MT_USLEEP(1*1000);
        }
#endif
    }

    MT_STAT_UNLOCK();

    return MT_SUCCESS;
}

mt_s32 mt_mpi_stat_deinit(mt_void)
{
    mt_s32 s32Ret = MT_SUCCESS;

    MT_STAT_LOCK();

    if(s_s32StatFd != -1)
    {
        s32Ret = close(s_s32StatFd);

        s_s32StatFd = -1;

#ifdef __STAT_USE_HW_TIMER__
        if (g_Timer7_addr)
        {
            mt_munmap((void *)g_Timer7_addr);
            g_Timer7_addr = NULL;
        }
#endif
    }

    MT_STAT_UNLOCK();

    return s32Ret;
}

#ifdef __STAT_USE_HW_TIMER__
// 270MHz / 256
//#define TIMER_PRE 1054687
#define TIMER_PRE 27000000
#endif

#ifdef CALLING_USED
mt_s32 stat_fast_get_time(struct timeval * tv);

mt_s32 stat_fast_get_time(struct timeval * tv)
{
#ifdef __STAT_USE_HW_TIMER__
    /* use TIMER0: 270MHz, 256 de-frequency*/
    mt_u32 time_now;
    long long time_diff;

    time_now = *((mt_u32 *)(g_Timer7_addr+4));
    time_diff = 0xffffffff - time_now;
//    MT_ERR_STAT("time_now = %x, time_diff = %d\n", time_now, time_diff);
    tv->tv_sec = (time_diff / TIMER_PRE);
    tv->tv_usec = (time_diff % TIMER_PRE) * 1000000 / TIMER_PRE;
#else
    /*use gettimeofday first, may be change to read hardware timer to get time subsequently */
    (mt_void)gettimeofday(tv, NULL);
#endif

    return MT_SUCCESS;
}


mt_s32 mt_mpi_stat_thread_reset(mt_stat_handle handle)
{
    handle->stat_thread_uvirtaddr->avg_time = 0;
    handle->stat_thread_uvirtaddr->min_time = 0;
    handle->stat_thread_uvirtaddr->max_time = 0;

    handle->counter = 0;
    handle->time_total = 0;

    memset(&handle->tv_last, 0, sizeof(struct timeval));

    return MT_FAILURE;
}

mt_s32 mt_mpi_stat_thread_reset_all(mt_void)
{
    mt_s32 ret;
    MT_BOOL bNeedOpen = MT_FALSE;

	MT_STAT_LOCK();

    if(s_s32StatFd == -1)
    {
        bNeedOpen = MT_TRUE;
        mt_mpi_stat_init();
    }

    ret = ioctl(s_s32StatFd, UMAPC_CMPI_STAT_RESETALL);
    if(ret != MT_SUCCESS)
    {
        MT_STAT_UNLOCK();
        MT_ERR_STAT("ioctl of UMAPC_CMPI_STAT_RESETALL err = %x\n", ret);
        return MT_FAILURE;
    }

    MT_STAT_UNLOCK();

    if(bNeedOpen == MT_TRUE)
    {
        mt_mpi_stat_deinit();
    }

    return MT_SUCCESS;
}

mt_s32 mt_mpi_stat_thread_register(mt_s8 * name, mt_stat_handle *pHandle)
{
	mt_s32 ret;

	*pHandle = (mt_stat_handle)mt_malloc(MT_ID_MEM, sizeof(mt_stat_handle_s));
	if (NULL == * pHandle)
	{
		return MT_FAILURE;
	}

	MT_STAT_LOCK();
	ret = ioctl(s_s32StatFd, UMAPC_CMPI_STAT_REGISTER, &((*pHandle)->stat_thread_phyaddr));
	if(ret != MT_SUCCESS)
	{
		MT_STAT_UNLOCK();

		MT_ERR_STAT("ioctl of UMAPC_CMPI_STAT_REGISTER err = %x\n", ret);

		free(*pHandle);
		*pHandle = NULL;

		return MT_FAILURE;
	}

	MT_STAT_UNLOCK();

	(*pHandle)->stat_thread_uvirtaddr = (stat_userspace_s *)mt_mmap((*pHandle)->stat_thread_phyaddr, sizeof(stat_userspace_s));
	if (NULL == ((*pHandle)->stat_thread_uvirtaddr))
	{
		free(*pHandle);
		*pHandle = NULL;
		return MT_FAILURE;
	}
	mt_osal_strncpy((mt_char*)(*pHandle)->stat_thread_uvirtaddr->name, (mt_char*)name, sizeof(THREAD_NAME)-1);
	(*pHandle)->stat_thread_uvirtaddr->name[sizeof(THREAD_NAME)-1] = '\0';

	return MT_SUCCESS;
}


mt_s32 MT_MPI_STAT_ThreadUnregister(mt_stat_handle * pHandle)
{
	if (NULL == (*pHandle)->stat_thread_uvirtaddr)
	{
		return MT_FAILURE;
	}
    memset((*pHandle)->stat_thread_uvirtaddr, 0, sizeof(stat_userspace_s));
    if ((void *)(*pHandle)->stat_thread_uvirtaddr)
    {
        (mt_void)mt_munmap((void *)(*pHandle)->stat_thread_uvirtaddr);
    }
    mt_free(MT_ID_MEM, *pHandle);
    (*pHandle) = NULL;

    return MT_SUCCESS;
}

#define STAT_TIME_COST(tv_s,tv_e)    \
    ((tv_e.tv_sec-tv_s.tv_sec)*1000000 + (tv_e.tv_usec - tv_s.tv_usec))

mt_s32 MT_MPI_STAT_ThreadProbe(mt_stat_handle handle)
{
    struct timeval tv_tmp;
    mt_u32 time_cost;

    stat_fast_get_time(&tv_tmp);

    if((handle->tv_last.tv_sec == 0) && (handle->tv_last.tv_usec == 0))
    {
        /*first probe*/
        handle->tv_last.tv_sec = tv_tmp.tv_sec;
        handle->tv_last.tv_usec = tv_tmp.tv_usec;

        return MT_SUCCESS;
    }

    handle->counter++;
    time_cost = (mt_u32)STAT_TIME_COST(handle->tv_last, tv_tmp);

    handle->tv_last.tv_sec = tv_tmp.tv_sec;
    handle->tv_last.tv_usec = tv_tmp.tv_usec;

    if(time_cost > handle->stat_thread_uvirtaddr->max_time)
    {
        handle->stat_thread_uvirtaddr->max_time = time_cost;
    }

    if((handle->stat_thread_uvirtaddr->min_time == 0) ||
        (time_cost < handle->stat_thread_uvirtaddr->min_time))
    {
        handle->stat_thread_uvirtaddr->min_time = time_cost;
    }

    handle->time_total += (mt_u64)time_cost;
    handle->stat_thread_uvirtaddr->avg_time = (mt_u32)((handle->time_total)/((mt_u64)(handle->counter)));

    return MT_FAILURE;
}
#endif

mt_s32 mt_mpi_stat_event(STAT_EVENT_E enEvent, mt_u32 Value)
{
    stat_event_s   StatEvent;
    mt_s32         Ret;

    StatEvent.enEvent = enEvent;
    StatEvent.Value = Value;

    MT_STAT_LOCK();

    Ret = ioctl(s_s32StatFd, UMAPC_CMPI_STAT_EVENT, &StatEvent);
    if(Ret != MT_SUCCESS)
    {
        MT_STAT_UNLOCK();

        MT_ERR_STAT("ioctl of UMAPC_CMPI_STAT_EVENT err = %x\n", Ret);
        return MT_FAILURE;
    }

    MT_STAT_UNLOCK();

    return MT_SUCCESS;
}

#ifdef CALLING_USED
mt_u32 mt_mpi_stat_get_tick(mt_void)
{
    mt_s32 Ret;
    mt_u32 Tick;

    MT_STAT_LOCK();

    Ret = ioctl(s_s32StatFd, UMAPC_CMPI_STAT_GETTICK, &Tick);
    if(Ret != MT_SUCCESS)
    {
        MT_STAT_UNLOCK();

        MT_ERR_STAT("ioctl of UMAPC_CMPI_STAT_GETTICK err = %x\n", Ret);
        return 0;
    }

    MT_STAT_UNLOCK();

    return Tick;
}
#endif


mt_s32 mt_mpi_stat_notify_low_delay_event (mt_ld_event_s * evt)
{
    mt_s32 Ret;

    if (!evt)
        return MT_FAILURE;

    Ret = ioctl(s_s32StatFd, UMAPC_CMPI_STAT_LD_EVENT, evt);
    if(Ret != MT_SUCCESS)
    {
        MT_ERR_STAT("ioctl of UMAPC_CMPI_STAT_LD_EVENT err = %x\n", Ret);
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}
