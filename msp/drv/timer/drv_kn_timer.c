/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/mutex.h>

#include "drv_timer.h"

//#define TM_VERB							printk
//#define TM_ERROR						printk
#define TM_VERB							MT_INFO_TIMER
#define TM_ERROR						MT_ERR_TIMER

#define MT_DRV_KN_TIMER_NAME_LEN		32
#define MT_DRV_KN_TIMER_COUNT			64

/* kernel timer */
struct mt_drv_kn_timer_st
{
	struct timer_list timer;

	char name[MT_DRV_KN_TIMER_NAME_LEN];
	int cycled;
	int cycle_ms;
	timer_fn func;
	void *param;
};

static DEFINE_MUTEX(timer_lock);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)

static struct mt_drv_kn_timer_st mt_drv_kn_timers[MT_DRV_KN_TIMER_COUNT] = {0};

//---------------------------------------------------------------------------//

static void mt_drv_kn_timer_func(struct timer_list *data)
{

	struct mt_drv_kn_timer_st *pTimer;
	struct timer_list *timer;

	TM_VERB("%s: data 0x%lx\n",__FUNCTION__,data);

	if (data == 0)
	{
		TM_ERROR("[ERROR]mt_drv_kn_timer_func: invalid parameter(1)!\n");
		return;
	}

	pTimer = (struct mt_drv_kn_timer_st *)data;

	if (pTimer->func == NULL)
	{
		TM_ERROR("[ERROR]mt_drv_kn_timer_func: invalid parameter(1)!\n");
		return;
	}

	pTimer->func(pTimer->param);

	if (pTimer->cycled)
	{
		timer = &pTimer->timer;

		timer->expires = jiffies + msecs_to_jiffies(pTimer->cycle_ms);
		timer->function = mt_drv_kn_timer_func;
		timer->data = (unsigned long)pTimer;

		add_timer(timer);
	}

}

static struct mt_drv_kn_timer_st *alloc_timer(void)
{
	int i;

	for (i=0; i<MT_DRV_KN_TIMER_COUNT; i++)
	{
		if (mt_drv_kn_timers[i].func == NULL)
		{
			TM_VERB("%s: %d\n",__FUNCTION__,i);
			return &mt_drv_kn_timers[i];
		}
	}

	TM_ERROR("[ERROR]allocate timer failed!\n");
	return NULL;
}
#endif

//---------------------------------------------------------------------------//

ulong mt_drv_kn_timer_request(int ms, timer_fn pfn, void *para, int cycled, char *name)
{
	struct mt_drv_kn_timer_st *pTimer = NULL;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	struct timer_list *timer;

	TM_VERB("%s: %p, %p, %d, %d, %s\n",__FUNCTION__,
		pfn, para, ms, cycled, name);

	if (pfn == NULL)
	{
		TM_ERROR("[ERROR]invalid parameter(2)!\n");
		return (-1);
	}

	if (ms <= 0)
	{
		TM_ERROR("[ERROR]invalid parameter(1)!\n");
		return (-1);
	}

	mutex_lock(&timer_lock);

	pTimer = alloc_timer();
	if (pTimer == NULL)
	{
		mutex_unlock(&timer_lock);
        BUG();
		return (-1);
	}

	if (name != NULL)
	{
		strncpy(pTimer->name, name, MT_DRV_KN_TIMER_NAME_LEN);
		pTimer->name[MT_DRV_KN_TIMER_NAME_LEN-1] = '\0';
	}
	else
	{
		pTimer->name[0] = '\0';
	}
	pTimer->cycled = cycled;
	pTimer->cycle_ms = ms;
	pTimer->func = pfn;
	pTimer->param = para;

	timer = &pTimer->timer;
	init_timer(timer);

	timer->expires = jiffies + msecs_to_jiffies(ms);
	timer->function = mt_drv_kn_timer_func;
	timer->data = (unsigned long)pTimer;

	add_timer(timer);

	mutex_unlock(&timer_lock);
#endif
	return (ulong)pTimer;
}
EXPORT_SYMBOL(mt_drv_kn_timer_request);

mt_s32  mt_drv_kn_timer_release(int id)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	struct mt_drv_kn_timer_st *pTimer;

	TM_VERB("%s: id 0x%x\n",__FUNCTION__,(unsigned)id);

	if (id == 0 || id == -1)
	{
		TM_ERROR("[ERROR]invalid parameter(1)!\n");
		return MT_FAILURE;
	}

	mutex_lock(&timer_lock);

	pTimer = (struct mt_drv_kn_timer_st *)id;

	if (pTimer->func == NULL)
	{
		TM_ERROR("[ERROR]invalid parameter(1)!\n");
		mutex_unlock(&timer_lock);
		return MT_FAILURE;
	}

	del_timer(&pTimer->timer);

	pTimer->func = NULL;

	mutex_unlock(&timer_lock);
#endif
	return MT_SUCCESS;
}
EXPORT_SYMBOL(mt_drv_kn_timer_release);

//---------------------------------------------------------------------------//

static unsigned int sample_timer_data[1] = {0};

static void sample_timer_cb(void *param)
{
	unsigned int *data = (unsigned int *)param;

	printk("%s: %u\n",__FUNCTION__,*data);

	(*data) ++;
}

void mt_drv_kn_timer_sample(void)
{
	mt_drv_kn_timer_request(2000, sample_timer_cb, (void*)sample_timer_data, 1,
							"mt_drv_kn_timer_sample");
}

