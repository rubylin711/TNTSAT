#include <linux/kthread.h>
#include <linux/timer.h>
#if !defined(CONFIG_LYNX_WM_MANAGER)
#include <net/mac80211.h>
#endif

#include "init.h"
#include "wlan_def.h"
#include "core.h"
#include "lynx_debug.h"

#if !defined(CONFIG_LYNX_WM_MANAGER)
#include "cfg80211.h"
#include "mlme_ops.h"
#else
#include "driver_ops.h"
#include "wm_msg.h"
#endif

#ifdef CONFIG_ANDROID
#define IFNAME "wlan%d"
#else
#define IFNAME "lynx-%d"
#endif

#if defined(CONFIG_LYNX_DEBUG) && defined(CONFIG_LYNX_OS_LINUX)
/* proc file system only for linux */
extern struct lynx_dbg_data dbg_data;
#endif

struct lynx *os_dep_create(void)
{
    struct lynx *lnx=NULL;

#if !defined(CONFIG_LYNX_WM_MANAGER)
    lnx = lynx_cfg80211_create();
#else
	/* TODO: create lnx */
	lnx = (struct lynx *) os_api_alloc(sizeof(struct lynx), GFP_KERNEL);

#endif
	if(lnx)
	{
		/* FIXME: Note it is block operation */
		init_waitqueue_head(&lnx->event_wq);
	}

	return lnx;
}

int os_dep_init(struct lynx *lnx)
{
    int ret = -ENOMEM;
#if !defined(CONFIG_LYNX_WM_MANAGER)
    struct wireless_dev *wdev;

    ret = lynx_cfg80211_init(lnx);
    if (ret < 0) {
        goto out;
    }

#ifdef CONFIG_LYNX_DEBUG
	dbg_data.lnx = lnx;
	//if((ret = lynx_proc_init_fs(lnx)))
	//{
	//	goto proc_err;
	//}
#endif

    lnx->avail_idx_map = (1 << lnx->vif_max) - 1;
	linux_mlme_init(lnx);

    /* FIXME: it should wait the return evt D2H_TGT_RDY to add the vif */
    rtnl_lock();
    /*Add an initial station interface*/
#ifndef NET_NAME_UNKNOWN
#define NET_NAME_UNKNOWN 0
#endif
    wdev = (struct wireless_dev *) lynx_interface_add(lnx, IFNAME, NET_NAME_UNKNOWN, NL80211_IFTYPE_STATION, 0, VIF_STA_MODE);
    rtnl_unlock();

	if(wdev)
		goto out;

	lynx_dbg(LYNX_DBG_ERR, "Failed to add a network device\n");
	ret = -ENOMEM;

#ifdef CONFIG_LYNX_DEBUG
	dbg_data.lnx = NULL;
    //lynx_proc_exit_fs(lnx);

//proc_err:
#endif

	wiphy_unregister(lnx->wiphy);

out:
	return ret;
#else
	ret = os_dep_init_wm_manager(lnx);
	if(!ret)
	{
		wm_mlme_init(lnx);
#ifdef CONFIG_LYNX_DEBUG
		dbg_data.lnx = lnx;
		//if((ret = lynx_proc_init_fs(lnx)))
		//{
    	//	lynx_proc_exit_fs(lnx);
		//}
#endif
	}

	return ret;
#endif
}

void os_dep_deinit(struct lynx *lnx)
{
#if !defined(CONFIG_LYNX_WM_MANAGER)
	lynx_cfg80211_cleanup(lnx);
#ifdef CONFIG_LYNX_DEBUG
//	lynx_proc_exit_fs(lnx);
	dbg_data.lnx = NULL;
#endif	// CONFIG_LYNX_DEBUG
	lynx_cfg80211_destroy(lnx);/*struct lynx will be destroy by this function*/
#else	// CONFIG_LYNX_WM_MANAGER
#ifdef CONFIG_LYNX_DEBUG
//	lynx_proc_exit_fs(lnx);
		dbg_data.lnx = NULL;
#endif	// CONFIG_LYNX_DEBUG
	os_dep_deinit_wm_manager();
#endif	// CONFIG_LYNX_WM_MANAGER
}

#if defined(CONFIG_LYNX_WM_MANAGER)
extern int wm_manager(void *priv);
struct task_struct *wm_thread=NULL;	/* TODO: move to the right place */
int os_dep_init_wm_manager(struct lynx *lnx)
{
    int ret=-1;
	struct sk_buff *skb;
	struct wm_msg *msg;
	/* static struct task_struct *wm_thread; */

	if(wm_thread == NULL)
	{
		wm_thread = kthread_create(wm_manager, lnx, "wm_manager");
		if(IS_ERR(wm_thread)) 
		{
			ret = PTR_ERR(wm_thread);
			wm_thread = NULL;
		}
		else
		{
			wake_up_process(wm_thread);
			ret = 0;

			msleep(1000);

			if((skb = wm_msgq_alloc(0)))
			{
				msg = (struct wm_msg *)skb->cb;
				msg->type = WM_MSG_CMD;
				msg->u.cmd.cmd = AP_CMD_RELOAD;
				wm_msgq_write(skb);
			}

			lynx_dbg(LYNX_DBG_WM, "create & wakeup wm_manager thread\n");
		}
	}

    return ret;
}

void os_dep_deinit_wm_manager(void)
{
	struct sk_buff *skb;
	struct wm_msg *msg;

	if((skb = wm_msgq_alloc(0)))
	{
		msg = (struct wm_msg *)skb->cb;
		msg->type = WM_MSG_CMD;
		msg->u.cmd.cmd = AP_CMD_SHUTDOWN;
		wm_msgq_write(skb);
	}

	if(wm_thread)
	{
#if defined(CONFIG_LYNX_OS_LINUX)
		kthread_stop(wm_thread);
#endif
		wm_thread = NULL;
	}
}

int os_check_thread_stop(void)
{
	int ret = 0;

#ifdef CONFIG_LYNX_OS_LINUX
	if(kthread_should_stop())
		ret = -1;
#endif

	return ret;
}

void os_thread_exit(void)
{
	/* Linux kernel thread is killed by kthread_stop() */
	return;
}
#endif // CONFIG_LYNX_WM_MANAGER

int os_wait_link_st_complete(struct lynx *lnx)
{
	int ret = 0;

	if((wait_event_interruptible_timeout(lnx->event_wq,
			test_bit(WCI_LINK_ST_RET, &lnx->flag), WCI_TIMEOUT)) <= 0)
	{
		lynx_dbg(LYNX_DBG_ERR, "WCI_TIMEOUT!! \n");
		ret = -ENODEV;
	}
	
	return ret;
}

int os_wait_init_complete(struct lynx *lnx)
{
	int ret = 0;

	if((wait_event_interruptible_timeout(lnx->event_wq,
			test_bit(WCI_READY, &lnx->flag), WCI_TIMEOUT)) <= 0)
	{
		lynx_dbg(LYNX_DBG_ERR, "WCI_TIMEOUT!! \n");
		ret = -ENODEV;
	}
	
	return ret;
}

int os_wakeup_wait_queue(void *wait_queue)
{
    wait_queue_head_t *event_wq = (wait_queue_head_t *)wait_queue;

	wake_up(event_wq);
	
	return 0;
}

char *os_api_alloc(int size, int flags)
{
	return kzalloc(size, flags);
}

void os_api_free(void *buf)
{
	if(buf)
		kfree(buf);
}

void os_init_timer(struct timer_list *timer, void *func, unsigned long data)
{
    timer_setup(timer, func, 0);
}

void os_deinit_timer(struct timer_list *timer)
{
	del_timer_sync(timer);
}

void os_del_timer(struct timer_list *timer)
{
	del_timer_sync(timer);
}

int os_add_timer(struct timer_list *timer, unsigned int interval)
{
	/* interval unit : msec */
	unsigned long expires;

	expires = jiffies + msecs_to_jiffies(interval);

	/* linux modify the original timer expire time, if it is existing. */
	return mod_timer(timer, expires);
}
	
void os_thread_sleep(unsigned int msec)
{
	msleep(msec);
}

int os_wait_for_completion_interruptible(struct completion *x, int timeout)
{
	if(timeout)
		return wait_for_completion_interruptible_timeout(x, timeout);
	else
		return wait_for_completion_interruptible(x);
}

int os_complete(struct completion *x)
{
	complete(x);
	return 0;
}

void os_completion_done(struct completion *x)
{
	completion_done(x);
}

int os_init_completion(OS_COMPLETION *comp)
{
	init_completion(comp);
	
	return 0;
}

unsigned int os_current_time(void)
{
	struct timespec64 now;
	unsigned long val;

	ktime_get_real_ts64(&now);
	val = (now.tv_sec * 100) + (now.tv_nsec / 10000000);
	
	return val;	// unit = 10 msec
}

int os_api_isr_lock(OS_LOCK_TYPE *lock, unsigned long *flags)
{
#if defined(CONFIG_LYNX_OS_LINUX)
    spin_lock_irqsave(lock, *flags);
#endif

	return 0;
}

int os_api_isr_unlock(OS_LOCK_TYPE *lock, unsigned long *flags)
{
#if defined(CONFIG_LYNX_OS_LINUX)
    spin_unlock_irqrestore(lock, *flags);
#endif

	return 0;
}

int os_api_dsr_lock(OS_LOCK_TYPE *lock)
{
#if defined(CONFIG_LYNX_OS_LINUX)
    spin_lock_bh(lock);
#endif

	return 0;
}

int os_api_dsr_unlock(OS_LOCK_TYPE *lock)
{
#if defined(CONFIG_LYNX_OS_LINUX)
    spin_unlock_bh(lock);
#endif
	
	return 0;
}

int os_api_lock(OS_LOCK_TYPE *lock)
{
#ifdef CONFIG_LYNX_OS_LINUX
    spin_lock(lock);
#endif
	
	return 0;
}

int os_api_unlock(OS_LOCK_TYPE *lock)
{
#ifdef CONFIG_LYNX_OS_LINUX
    spin_unlock(lock);
#endif
	
	return 0;
}
    
int os_api_lock_init(OS_LOCK_TYPE *lock)
{
#ifdef CONFIG_LYNX_OS_LINUX
	spin_lock_init(lock);
#elif defined(CONFIG_LYNX_OS_UCOS)
#ifdef CONFIG_LOCK_BY_ISR
	/* Do nothing */
#else	// CONFIG_LOCK_BY_ISR
	int ret = 0;
	if(mtos_sem_create(lock, TRUE) == FALSE)	// TRUE = create mutex
		ret = -1;
#endif	// CONFIG_LOCK_BY_ISR
#endif
	
	return 0;
}

int os_api_lock_deinit(OS_LOCK_TYPE *lock)
{
#if defined(CONFIG_LYNX_OS_UCOS)
#ifdef CONFIG_LOCK_BY_ISR
	/* FIXME: uCoS doesn't has spinlock & mutex can't be used in the ISR. Disable ISR to patch */
	/* DO nothing */
#else	// CONFIG_LOCK_BY_ISR
 	mtos_sem_destroy(lock, 0);
#endif	// CONFIG_LOCK_BY_ISR
#endif
	
	return 0;
}

int os_api_rand(unsigned int seed)
{
#if defined(CONFIG_LYNX_OS_LINUX)
	return (int)get_random_int();
#else	// CONFIG_LYNX_OS_LINUX
#ifndef RAND_MAX
#define RAND_MAX  0x7FFFFF
#endif
    int retval;
    unsigned int uret;

    seed = (seed * 1103515245) + 12345; // permutate seed
    // Only use top 11 bits
    uret = seed & 0xffe00000;
    
    seed = (seed * 1103515245) + 12345; // permutate seed
    // Only use top 14 bits
    uret += (seed & 0xfffc0000) >> 11;
    
    seed = (seed * 1103515245) + 12345; // permutate seed
    // Only use top 7 bits
    uret += (seed & 0xfe000000) >> (11+14);
    
    retval = (int)(uret & RAND_MAX);

    return retval;
#endif	// CONFIG_LYNX_OS_LINUX
}
