/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include "mt_type.h"
#include "mtos_misc.h"

#include "mt_osal.h"
#include "drv_os.h"

void mtos_os_init(mtos_cfg_t *p_cfg)
{
    return;
}

void nucleus_os_init()
{
    return;
}

void mtos_start()
{
    return;
}

void mtos_stat_init()
{
    return;
}

MT_BOOL mtos_ticks_init(u32 cpu_freq)
{
    return mt_ticks_init(cpu_freq);
}

u32 mtos_ticks_get()
{
    return mt_ticks_get();
}

u32 mtos_cpu_freq_get()
{
    return 0;
}

u32 mtos_hw_ticks_get()
{
    return 0;
}

void mtos_systime_get(u64 *s, u32 *ms, u32 *us)
{
    SYS_GetSysTime(s, ms, us);
    return;
}

u32 mtos_hw_loop_get()
{
    return 0;
}

void mtos_task_delay_ms(u32 ms)
{
    SYS_TaskDelay(ms);
    return;
}

void mtos_task_delay_us(u32 us)
{
    SYS_DelayUS(us);
    return;
}

void mtos_critical_enter(u32 *p_sr)
{
    //TODO
    return;
}

void mtos_critical_exit(u32 sr)
{
    //TODO
    return;
}
