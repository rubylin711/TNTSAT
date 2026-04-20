#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched/clock.h>
#include "mt_type.h"
#include "mt_drv_stat.h"
#include "drv_stat_ioctl.h"

#ifdef __cplusplus
extern "C"
{
#endif /* End of #ifdef __cplusplus */

static  stat_event_fun   stat_event_call = NULL;

mt_s32 mt_drv_stat_eventfunc_register(mt_void *pFunc)
{
    if (NULL == pFunc)
    {
        return MT_FAILURE;
    }
	stat_event_call = (stat_event_fun)pFunc;
	return MT_SUCCESS;
}

mt_void mt_drv_stat_eventfunc_unregister(mt_void)
{
	stat_event_call = NULL;
	return;
}

mt_void mt_drv_stat_event(STAT_EVENT_E enEvent, mt_u32 value)
{
	if(stat_event_call){
		stat_event_call(enEvent, value);
	}
	return;
}

mt_s32 low_delay_start_statistics(MT_LD_SCENES_E scenes_id, mt_void *filter);
mt_s32 mt_drv_ld_start_statistics(MT_LD_SCENES_E scenes_id, mt_void *filter)
{
	return low_delay_start_statistics(scenes_id, filter);
}

mt_void low_delay_stop_statistics(mt_void);
mt_void mt_drv_ld_stop_statistics(mt_void)
{
    return low_delay_stop_statistics();
}

mt_void low_delay_notify_event( mt_ld_event_s *evt);
mt_void mt_drv_ld_notify_event( mt_ld_event_s *evt)
{
    return low_delay_notify_event(evt);
}

mt_u32 mt_drv_stat_gettick(mt_void)
{
    mt_u64 SysTime;

    SysTime = sched_clock();
    do_div(SysTime, 1000000);
    return (mt_u32)SysTime;
}


mt_s32 mt_drv_stat_kinit(void)
{
	stat_event_call = NULL;
	return MT_SUCCESS;
}

mt_void mt_drv_stat_kexit(void)
{
	stat_event_call = NULL;
	return ;
}

EXPORT_SYMBOL(mt_drv_stat_eventfunc_register);
EXPORT_SYMBOL(mt_drv_stat_eventfunc_unregister);
EXPORT_SYMBOL(mt_drv_stat_event);
EXPORT_SYMBOL(mt_drv_stat_gettick);
EXPORT_SYMBOL(mt_drv_ld_start_statistics);
EXPORT_SYMBOL(mt_drv_ld_stop_statistics);
EXPORT_SYMBOL(mt_drv_ld_notify_event);


#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

