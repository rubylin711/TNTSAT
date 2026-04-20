/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /*cplusplus*/
#include "drv_demux_osal.h"
#include <linux/hrtimer.h>

mt_void DMX_AcrtUsSleep(mt_u32 us)
{
    mt_s32 ret;
    ktime_t expires;
    expires = ktime_add_ns(ktime_get(), us*1000);
    set_current_state(TASK_UNINTERRUPTIBLE);
    ret = schedule_hrtimeout(&expires, HRTIMER_MODE_ABS);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*cplusplus*/

