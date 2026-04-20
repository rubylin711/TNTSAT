#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/sched/clock.h>
#include <linux/list.h>
#include <linux/ctype.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include "mt_type.h"
#include "mt_debug.h"
#include "mt_osal.h"
#include "mt_drv_mmz.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_stat.h"
#include "drv_stat_ioctl.h"
#include "mt_drv_log.h"


/* for userspace stat */
typedef struct
{
    MT_BOOL is_used;
    mt_u32 min_time;    // us
    mt_u32 max_time;    // us
    mt_u32 avg_time;    // us
    mt_s8 name[64];
}stat_userspace_s;


typedef struct
{
	struct timespec64 tv;
	mt_u32 time;
}stat_isr_time_s;

typedef struct
{
    mt_u32 EventMs;
    mt_u32 Value1;
    mt_u32 Value2;
}stat_event_time_s;


#define STAT_MAX_THREAD_USERSPACE   32
/* make sure STAT_USERSPACE_TOTAL_SIZE < 4096 */
#define STAT_USERSPACE_TOTAL_SIZE (sizeof(stat_userspace_s)*STAT_MAX_THREAD_USERSPACE)


static stat_isr_time_s g_IsrTime[STAT_ISR_BUTT];
static MT_BOOL g_stat_isr_enable = MT_FALSE;
static MT_BOOL g_stat_thread_enable = MT_TRUE;

static stat_event_time_s g_EventTime[STAT_EVENT_BUTT];

static dma_addr_t g_stat_thread_phyaddr_base = 0;

/* g_stat_userspace point to STAT_MAX_THREAD_USERSPACE * stat_userspace_s */
static stat_userspace_s * g_stat_thread_kvirt_base = NULL;

static mt_u64 div64(mt_u64 Dividend, mt_u64 Divisor)
{
    do_div(Dividend, Divisor);
    return Dividend;
}

static mt_s32 cmpi_stat_ioctl(struct inode *inode,
                            struct file *file,
                            mt_u32 cmd,
                            mt_void * arg)
{
    mt_u32 tmp;
    int i;

       switch(cmd)
       {
            case UMAPC_CMPI_STAT_REGISTER:
            {
                mt_u32 * p_stat_thread_addr = (mt_u32 * )arg;

                for(i=0; i<STAT_MAX_THREAD_USERSPACE; i++)
                {
                    if(g_stat_thread_kvirt_base[i].is_used == MT_FALSE)
                    {
                        tmp = g_stat_thread_phyaddr_base + i*sizeof(stat_userspace_s);
                        MT_WARN_STAT("get free slab(%d)\n", i);
                        MT_WARN_STAT("g_stat_thread_phyaddr_base = %x, tmp = %x\n", g_stat_thread_phyaddr_base, tmp);

                        *p_stat_thread_addr = tmp;
                        g_stat_thread_kvirt_base[i].is_used = MT_TRUE;

                        return 0;
                    }
                }

                MT_ERR_STAT("no available stat resource\n");
                /* no available stat resource */
                *p_stat_thread_addr = 0;
                return -1;
            }

            case UMAPC_CMPI_STAT_RESETALL:
            {
                for(i=0; i<STAT_MAX_THREAD_USERSPACE; i++)
                {
                    if(g_stat_thread_kvirt_base[i].is_used == MT_TRUE)
                    {
                        g_stat_thread_kvirt_base[i].avg_time = 0;
                        g_stat_thread_kvirt_base[i].min_time = 0;
                        g_stat_thread_kvirt_base[i].max_time = 0;
                    }
                }

                break;
            }

            case UMAPC_CMPI_STAT_EVENT:
            {
                stat_event_s   *pStatEvent;

                pStatEvent = (stat_event_s *)arg;

                mt_drv_stat_event(pStatEvent->enEvent, pStatEvent->Value);

                break;
            }

            case UMAPC_CMPI_STAT_GETTICK:
            {
                mt_u32 *pTick = (mt_u32*)arg;
                *pTick = mt_drv_stat_gettick();
                break;
            }

            case UMAPC_CMPI_STAT_LD_EVENT:
            {
                mt_drv_ld_notify_event((mt_ld_event_s*)arg);
                break;
            }

            default:
                return -1;
       }

    return 0;
}

static long drv_stat_ioctl(struct file *file,
                           mt_u32 cmd,
                           unsigned long arg)
{
    return (long)mt_drv_usercopy(file->f_path.dentry->d_inode, file, cmd, arg, cmpi_stat_ioctl);
}

static mt_s32 drv_stat_release(struct inode * inode, struct file * file)
{
    return 0;
}

static mt_s32 drv_stat_open(struct inode * inode, struct file * file)
{
    return 0;
}

static struct file_operations DRV_stat_Fops=
{
    .owner      = THIS_MODULE,
    .open       = drv_stat_open,
		.unlocked_ioctl      = drv_stat_ioctl,
    .release    = drv_stat_release,
};

static mt_s32 stat_proc_read(struct seq_file *s, mt_void *pArg)
{
    int i;

    PROC_PRINT(s, "----------- host isr stat -----------\n");
    if(g_stat_isr_enable == MT_FALSE)
    {
        PROC_PRINT(s, "isr stat is disabled!\n");
    }
    else
    {
        PROC_PRINT(s, "audio isr time = %u us\n", g_IsrTime[STAT_ISR_AUDIO].time);
        PROC_PRINT(s, "video isr time = %u us\n", g_IsrTime[STAT_ISR_VIDEO].time);
        PROC_PRINT(s, "demux isr time = %u us\n", g_IsrTime[STAT_ISR_DEMUX].time);
        PROC_PRINT(s, "sync isr time  = %u us\n", g_IsrTime[STAT_ISR_SYNC].time);
        PROC_PRINT(s, "vo isr time    = %u us\n", g_IsrTime[STAT_ISR_VO].time);
        PROC_PRINT(s, "tde isr time   = %u us\n", g_IsrTime[STAT_ISR_TDE].time);
    }

    PROC_PRINT(s, "----------- host thread stat ----------\n");
    if(g_stat_thread_enable == MT_FALSE)
    {
        PROC_PRINT(s, "thread stat is disabled!\n");
    }
    else
    {
        for(i=0; i<STAT_MAX_THREAD_USERSPACE; i++)
        {
            if(g_stat_thread_kvirt_base[i].is_used == MT_TRUE)
            {
                PROC_PRINT(s, "thread(%s) min=%08dus, avg=%08dus, max=%08d\n",
                                g_stat_thread_kvirt_base[i].name,
                                g_stat_thread_kvirt_base[i].min_time,
                                g_stat_thread_kvirt_base[i].avg_time,
                                g_stat_thread_kvirt_base[i].max_time);
            }
        }
    }

    PROC_PRINT(s, "----------- host event stat ----------\n");

    PROC_PRINT(s, "KEYIN          = %-10u (keyvalue 0x%x)\n", g_EventTime[STAT_EVENT_KEYIN].EventMs, g_EventTime[STAT_EVENT_KEYIN].Value1);
    PROC_PRINT(s, "KEYOUT         = %-10u (keyvalue 0x%x)\n", g_EventTime[STAT_EVENT_KEYOUT].EventMs, g_EventTime[STAT_EVENT_KEYOUT].Value1);
    PROC_PRINT(s, "VSTOP_IN       = %-10u\n", g_EventTime[STAT_EVENT_VSTOP_IN].EventMs);
    PROC_PRINT(s, "VSTOP          = %-10u\n", g_EventTime[STAT_EVENT_VSTOP].EventMs);
    PROC_PRINT(s, "ASTOP_IN       = %-10u\n", g_EventTime[STAT_EVENT_ASTOP_IN].EventMs);
    PROC_PRINT(s, "ASTOP          = %-10u\n", g_EventTime[STAT_EVENT_ASTOP].EventMs);
    PROC_PRINT(s, "CONNECT        = %-10u\n", g_EventTime[STAT_EVENT_CONNECT].EventMs);
    PROC_PRINT(s, "LOCKED         = %-10u\n", g_EventTime[STAT_EVENT_LOCKED].EventMs);
    PROC_PRINT(s, "VSTART_IN      = %-10u\n", g_EventTime[STAT_EVENT_VSTART_IN ].EventMs);
    PROC_PRINT(s, "VSTART         = %-10u\n", g_EventTime[STAT_EVENT_VSTART].EventMs);
	PROC_PRINT(s, "ASTART_IN      = %-10u\n", g_EventTime[STAT_EVENT_ASTART_IN].EventMs);
    PROC_PRINT(s, "ASTART         = %-10u\n", g_EventTime[STAT_EVENT_ASTART].EventMs);
    PROC_PRINT(s, "CWSET          = %-10u\n", g_EventTime[STAT_EVENT_CWSET].EventMs);
    PROC_PRINT(s, "STREAMIN       = %-10u\n", g_EventTime[STAT_EVENT_STREAMIN].EventMs);
    PROC_PRINT(s, "ISTREAMGET     = %-10u (size %d)\n", g_EventTime[STAT_EVENT_ISTREAMGET].EventMs,g_EventTime[STAT_EVENT_ISTREAMGET].Value1);
    PROC_PRINT(s, "FRAMEDECED     = %-10u\n", g_EventTime[STAT_EVENT_IFRAMEOUT].EventMs);

	PROC_PRINT(s, "VPSSGET        = %-10u\n", g_EventTime[STAT_EVENT_VPSSGETFRM].EventMs);
    PROC_PRINT(s, "VPSSOUT        = %-10u\n", g_EventTime[STAT_EVENT_VPSSOUTFRM].EventMs);
    PROC_PRINT(s, "AVPLAYGET      = %-10u\n", g_EventTime[STAT_EVENT_AVPLAYGETFRM].EventMs);

	PROC_PRINT(s, "PRESYNC        = %-10u\n", g_EventTime[STAT_EVENT_PRESYNC].EventMs);
    PROC_PRINT(s, "BUFREADY       = %-10u (type %d)\n", g_EventTime[STAT_EVENT_BUFREADY].EventMs,g_EventTime[STAT_EVENT_BUFREADY].Value1);
    PROC_PRINT(s, "FRAMESYNCOK    = %-10u\n", g_EventTime[STAT_EVENT_FRAMESYNCOK].EventMs);
	
    PROC_PRINT(s, "VOGET          = %-10u\n", g_EventTime[STAT_EVENT_VOGETFRM].EventMs);
    PROC_PRINT(s, "OPENSCREEN     = %-10u\n", g_EventTime[STAT_EVENT_OPENSCREEM].EventMs);
    PROC_PRINT(s, "SYNCVIDEO      = %-10u\n", g_EventTime[STAT_EVENT_SYNCVIDEO].EventMs);
	
    PROC_PRINT(s, "SYNCED         = %-10u\n", g_EventTime[STAT_EVENT_SYNCDONE].EventMs);
    PROC_PRINT(s, "TOTAL          = %-10u\n", g_EventTime[STAT_EVENT_VOGETFRM].EventMs - g_EventTime[STAT_EVENT_KEYIN].EventMs);
    PROC_PRINT(s, "TOTAL_AV       = %-10u\n", g_EventTime[STAT_EVENT_SYNCVIDEO].EventMs - g_EventTime[STAT_EVENT_VSTOP].EventMs);
    PROC_PRINT(s, "SYNC_DONE      = %-10u\n", g_EventTime[STAT_EVENT_SYNCDONE].EventMs - g_EventTime[STAT_EVENT_VSTOP].EventMs);
    return 0;
}

#if 0
static mt_s32 stat_proc_write(struct file * file,
    const char __user * buf, size_t count, loff_t *ppos)
{
    return 0;
}
#endif

/* ------------- low delay statistics ------------------- */

struct low_delay_evt_ext
{
    struct list_head list;
    mt_ld_event_s  evt;
};

struct low_delay_evt_queue
{
    mt_s32                              head, tail;
    mt_s32                              active;
    spinlock_t                          lock;
    struct low_delay_evt_ext     queue[MAX_EVENT_QUEUE_SIZE];
};

static struct low_delay_evt_head
{
    enum {
        ON = 0,
        OFF = 1,
    }                                      state;
    struct mutex                      mutex;
    MT_LD_SCENES_E                 scenes_id;
    ulong                              filter_handle;
    struct low_delay_evt_queue evt_queue[MAX_EVENT_TYPE_NR];
} g_active_evt_head = {
    .state                   = OFF,
    .mutex                  = __MUTEX_INITIALIZER(g_active_evt_head.mutex),
    .evt_queue            = {
        [0 ... (MAX_EVENT_TYPE_NR -1)] = {
           .lock   =  __SPIN_LOCK_UNLOCKED(evt_queue.lock),
        }
    },
};

static mmz_buffer_s mmz_buffer;

#define traverse_scenes_each_event(index, evt_id, scenes_id) \
    for (index = 0, evt_id = g_scenes_desc[scenes_id][index]; \
        index < MAX_EVENT_TYPE_NR && scenes_id < SCENES_LD_BUTT && evt_id != EVENT_LD_BUTT ;\
        index++, evt_id = g_scenes_desc[scenes_id][index < MAX_EVENT_TYPE_NR ? index : 0])

/* tscancode */
#if 0
#define traverse_each_event_queue(index, evt_queue) \
    for(index = 0, evt_queue = &(g_active_evt_head.evt_queue[index]);\
        index < MAX_EVENT_TYPE_NR; \
        index ++, evt_queue = &(g_active_evt_head.evt_queue[index/*tscancode: index overflow!!!*/]))
#endif

#define traverse_queue_valid_entry(index, evt_ext, evt_queue) \
    for(index = evt_queue->head, evt_ext = &(evt_queue->queue[index]);\
        evt_queue->tail != index % MAX_EVENT_QUEUE_SIZE; \
        index = (index + 1) % MAX_EVENT_QUEUE_SIZE, evt_ext = &(evt_queue->queue[index]))
/*
 * reset evt statistics state and queue
 */
static mt_void reset_evt_statistics_nolock(mt_void)
{
    mt_u32 index = 0;
    unsigned long flags;
    struct low_delay_evt_queue * evt_queue;

    g_active_evt_head.state = OFF;

    /* reset ring queue */
    /* tscancode: index overflow!!! */
    //traverse_each_event_queue(index, evt_queue)
    for(index = 0;\
        index < MAX_EVENT_TYPE_NR; \
        index ++)
    {
    	evt_queue = &(g_active_evt_head.evt_queue[index]);

        spin_lock_irqsave(&(evt_queue->lock), flags);
        evt_queue->active = 0;
        evt_queue->head = 0;
        evt_queue->tail   = 0;
        spin_unlock_irqrestore(&(evt_queue->lock), flags);
    }
}

 /*
  * start new scenes statistics, reset first.
  */
 mt_s32 low_delay_start_statistics(MT_LD_SCENES_E scenes_id, mt_void *  filter)
 {
    mt_u32 index;
    MT_LD_EVENT_ID_E evt_id;
    unsigned long flags;

    if (!(scenes_id < SCENES_LD_BUTT))
    {
        MT_ERR_STAT("invalid scenes parameter.\n");
        return MT_FAILURE;
    }

    mutex_lock(&(g_active_evt_head.mutex));

    /* disable and reset current statistics */
    reset_evt_statistics_nolock();

    /* save new scenes_id and filter */
    g_active_evt_head.scenes_id     = scenes_id;
    g_active_evt_head.filter_handle  = (ulong)filter;

    /* set evt queue active tag according to scenes */
    traverse_scenes_each_event(index, evt_id, scenes_id)
    {
        struct low_delay_evt_queue * evt_queue = &(g_active_evt_head.evt_queue[evt_id]);

        spin_lock_irqsave(&(evt_queue->lock), flags);
        evt_queue->active = 1;
        spin_unlock_irqrestore(&(evt_queue->lock), flags);
    }

    /* enable statistics */
    g_active_evt_head.state = ON;

    mutex_unlock(&(g_active_evt_head.mutex));

    return MT_SUCCESS;
 }

/*
 * stop current scenes statistics.
  */
 mt_void low_delay_stop_statistics(mt_void)
 {
    mutex_lock(&(g_active_evt_head.mutex));

    reset_evt_statistics_nolock();

    mutex_unlock(&(g_active_evt_head.mutex));
 }

/*
 * capture event statistics snapshot
 */
 static mt_s32 capture_statistics_snapshot(struct low_delay_evt_head* dup)
{
    if (!dup)
        return MT_FAILURE;

    mutex_lock(&(g_active_evt_head.mutex));

    if (OFF == g_active_evt_head.state)
    {
        goto fail;
    }

    /* duplicate entire evt queue */
    memcpy(dup, &g_active_evt_head, sizeof(struct low_delay_evt_head));

    mutex_unlock(&(g_active_evt_head.mutex));

    return MT_SUCCESS;

fail:
    mutex_unlock(&(g_active_evt_head.mutex));
    return MT_FAILURE;
}

/*
 * display event head name
 */
 static mt_void show_event_name(struct seq_file *s, struct low_delay_evt_head* head)
{
    mt_u32 index;
    MT_LD_EVENT_ID_E evt_id;

    if (!s || !head)
        return ;

    PROC_PRINT(s, "%8s    ", " ");
    traverse_scenes_each_event(index, evt_id, head->scenes_id)
    {
        PROC_PRINT(s, "%10s    ", g_event_name[evt_id]);
    }
    PROC_PRINT(s, "%10s    ", "Total_diff");
    PROC_PRINT(s, "\n");
}

/*
 * relink all event entry from queue into one list.
 */
static mt_s32 relink_low_delay_evt(struct low_delay_evt_head* head, struct list_head * relink_head)
{
    mt_u32 i, j ;
    MT_LD_EVENT_ID_E evt_id;
    struct low_delay_evt_queue * evt_queue;
    struct low_delay_evt_ext * evt_ext;

    if (!head || !relink_head)
        return MT_FAILURE;

    INIT_LIST_HEAD(relink_head);

    traverse_scenes_each_event(i, evt_id, head->scenes_id)
    {
        evt_queue = &(head->evt_queue[evt_id]);

        traverse_queue_valid_entry(j, evt_ext, evt_queue)
        {
            if (head->filter_handle == evt_ext->evt.handle)
            {
                list_add_tail(&(evt_ext->list), relink_head);
            }
            else
            {
                MT_ERR_STAT("[WARN] id:%u, handle:%u, frame:%u, time:%u dismatch filter handle(%u) .\n", evt_ext->evt.evt_id, evt_ext->evt.handle, evt_ext->evt.frame, evt_ext->evt.time, head->filter_handle);
            }

        }
    }

    return MT_SUCCESS;
}

/*
 * display collected detail data.
 */
static mt_s32 show_event_detail_data(struct seq_file *s, struct low_delay_evt_head* head)
{
    mt_u32 current_key, i, j;
    MT_BOOL cal_diff_time;
    struct list_head relink_head;
    MT_LD_EVENT_ID_E evt_id;
    struct low_delay_evt_queue * evt_queue;
    struct low_delay_evt_ext * evt_ext, *first_evt_ext = NULL, *last_evt_ext = NULL;

    if (!head)
        return MT_FAILURE;

    /* relink new event list */
    if (MT_SUCCESS != relink_low_delay_evt(head, &relink_head))
    {
        MT_ERR_STAT("relink event list failed.\n");
        goto out;
    }

    while(!list_empty(&(relink_head)))
    {
        cal_diff_time = MT_TRUE;
        evt_ext = list_first_entry(&(relink_head), struct low_delay_evt_ext, list);
        first_evt_ext = evt_ext;
        current_key = evt_ext->evt.frame;

        PROC_PRINT(s, "%08d    ", current_key);

        traverse_scenes_each_event(i, evt_id, head->scenes_id)
        {
            MT_BOOL found = MT_FALSE;
            evt_queue = &(head->evt_queue[evt_id]);

            traverse_queue_valid_entry(j, evt_ext, evt_queue)
            {
                if (head->filter_handle == evt_ext->evt.handle && current_key == evt_ext->evt.frame && !list_empty(&(evt_ext->list)))
                {
                    PROC_PRINT(s, "%10d    ", evt_ext->evt.time);

                    last_evt_ext = evt_ext;

                    /* this evt is still in the queue, we should escape it next time. see prev list_empty checking */
                    list_del_init(&(evt_ext->list));
                    found = MT_TRUE;
                    break;
                }
            }

            if (MT_FALSE == found)
            {
                PROC_PRINT(s, "%10s    ", "-");
                cal_diff_time = MT_FALSE;
            }

        }

        /* calculate time difference from first event occur to final event finished*/
        if (MT_TRUE == cal_diff_time)
        {
            PROC_PRINT(s, "%10d    ", last_evt_ext ? last_evt_ext->evt.time - first_evt_ext->evt.time : 0);
        }
        else
        {
            PROC_PRINT(s, "%10s    ", "-");
        }

        PROC_PRINT(s, "\n");
    }

    return MT_SUCCESS;

out:
    return  MT_FAILURE;
}

/*
 * show statistics data by 'cat /proc/msp/low_delay_statistics' command
 */
static mt_s32 low_delay_proc_read(struct seq_file *s, mt_void *pArg)
{
    struct low_delay_evt_head * dup;

    dup = kmalloc(sizeof(struct low_delay_evt_head), GFP_KERNEL);
    if (!dup)
    {
        MT_ERR_LOG("create dup evt head failed.\n");
        goto out;
    }

    if(MT_SUCCESS != capture_statistics_snapshot(dup))
    {
        PROC_PRINT(s, "low delay statistics disabled now.\n");
        goto out;
    }

    show_event_name(s, dup);

    show_event_detail_data(s, dup);

out:

    if (dup)
        kfree(dup);
    return 0;
}

/*
 * add new event into low delay framework
 */
mt_void low_delay_notify_event(mt_ld_event_s * evt)
{
    unsigned long flags;
    struct low_delay_evt_queue * evt_queue;
    mt_ld_event_s * dup;

    if (unlikely(!evt))
        return ;

    /* statistics disabled ? */
    if (likely(OFF == g_active_evt_head.state))
    {
        return ;
    }

    /* filter */
    if (evt->handle != g_active_evt_head.filter_handle)
    {
        MT_DBG_STAT("filter invalid evt[name:'%s', handle:%u, frame:%u, time:%u].\n", g_event_name[evt->evt_id], evt->handle, evt->frame, evt->time);
        goto out;
    }

    evt_queue = &(g_active_evt_head.evt_queue[evt->evt_id]);

    /* evt is invalid for current scenes */
    if ( !evt_queue->active)
    {
        MT_DBG_STAT("filter inactive evt[name:'%s', handle:%u, frame:%u, time:%u].\n", g_event_name[evt->evt_id], evt->handle, evt->frame, evt->time);
        goto out;
    }

    spin_lock_irqsave(&(evt_queue->lock), flags);

    /* ring queue is full */
    if ( ( (evt_queue->tail + 1) % MAX_EVENT_QUEUE_SIZE == evt_queue->head ))
    {
        evt_queue->head = (evt_queue->head + 1) % MAX_EVENT_QUEUE_SIZE;
    }

    /* add evt into ring queue */
    dup = &( (evt_queue->queue[evt_queue->tail]).evt);
    memcpy(dup, evt, sizeof(mt_ld_event_s));

    evt_queue->tail = (evt_queue->tail + 1) % MAX_EVENT_QUEUE_SIZE;

    spin_unlock_irqrestore(&(evt_queue->lock), flags);

out:
    return;
}

mt_void stat_event_call(STAT_EVENT_E enEvent, mt_u32 Value);



#if 0
static mt_device_s    g_srtuStatDev;

mt_s32 mt_drv_stat_init(mt_void)
{
    mt_proc_entry_t *item ;
    int i;

    memset((void *)g_IsrTime, 0, sizeof(g_IsrTime));

	if (mt_drv_mmz_alloc_and_map("stat", NULL, STAT_USERSPACE_TOTAL_SIZE, 0, &mmz_buffer) != MT_SUCCESS) {
		return -1;
	}
	g_stat_thread_kvirt_base = mmz_buffer.startVirAddr;
	g_stat_thread_phyaddr_base = mmz_buffer.startPhyAddr;
    memset((void *)g_stat_thread_kvirt_base, 0, STAT_USERSPACE_TOTAL_SIZE);

    for(i=0; i<STAT_MAX_THREAD_USERSPACE; i++)
    {
        g_stat_thread_kvirt_base[i].is_used = MT_FALSE;
    }

    mt_osal_snprintf(g_srtuStatDev.devfs_name, sizeof(g_srtuStatDev.devfs_name), "%s", UMAP_DEVNAME_STAT);
    g_srtuStatDev.fops = &DRV_stat_Fops;
    g_srtuStatDev.minor = UMAP_MIN_MINOR_STAT;
    g_srtuStatDev.owner  = THIS_MODULE;
    g_srtuStatDev.drvops = NULL;
    if(mt_drv_dev_register(&g_srtuStatDev) < 0)
    {
        MT_ERR_STAT("cann't register stat dev\n");
        goto err0;
    }

    item = mt_drv_proc_add_module(MT_MOD_STAT, NULL, NULL);
    if (! item)
    {
        goto err1;
    }
    item->read = stat_proc_read;
    item->write = NULL;

    mt_drv_stat_eventfunc_register(stat_event_call);

    /* register low delay statistics proc interface */
    item = mt_drv_proc_add_module(MT_MOD_EVENT, NULL, NULL);
    if (!item)
    {
        MT_ERR_STAT("cann't register low delay statistics interface.\n");
        goto err1;
    }
    item->read = low_delay_proc_read;
    item->write = NULL;

    return 0;

err1:
    mt_drv_dev_unregister(&g_srtuStatDev);

err0:
	mt_drv_mmz_unmap_and_release(&mmz_buffer);
    return -1;

}


mt_void mt_drv_stat_exit(mt_void)
{
    mt_drv_proc_rm_module(MT_MOD_EVENT);
    mt_drv_stat_eventfunc_unregister();
    mt_drv_proc_rm_module(MT_MOD_STAT);
    mt_drv_dev_unregister(&g_srtuStatDev);
	mt_drv_mmz_unmap_and_release(&mmz_buffer);
	return;
}
#endif




struct mt_drv_stat_device {
	struct device *dev;
	void *platdata;
	dev_t devt;
	int id;
};

struct mt_drv_stat_driver {
	struct cdev cdev;
	dev_t devt;
	dev_t major;
	dev_t minor;
	u32 minors;
};

static struct mt_drv_stat_driver *mt_drv_stat_drv;

static int mt_drv_stat_probe(struct platform_device *pdev)
{
	mt_proc_entry_t *item = NULL;
	struct mt_drv_stat_device *mt_drv_stat_dev = NULL;
	int ret = 0;
	int i;

	memset((void *)g_IsrTime, 0, sizeof(g_IsrTime));

	if (mt_drv_mmz_alloc_and_map("stat", NULL, STAT_USERSPACE_TOTAL_SIZE, 0, &mmz_buffer) != MT_SUCCESS) {
		ret = -ENOMEM;
		goto fail_mt_drv_mmz_alloc_and_map;
	}
	g_stat_thread_kvirt_base = mmz_buffer.startVirAddr;
	g_stat_thread_phyaddr_base = mmz_buffer.startPhyAddr;
    memset((void *)g_stat_thread_kvirt_base, 0, STAT_USERSPACE_TOTAL_SIZE);

    for (i = 0; i < STAT_MAX_THREAD_USERSPACE; i++) {
        g_stat_thread_kvirt_base[i].is_used = MT_FALSE;
    }

	mt_drv_stat_dev = kzalloc(sizeof(struct mt_drv_stat_device), GFP_KERNEL);
	if (!mt_drv_stat_dev) {
		pr_err("Error kzalloc mt_drv_stat_dev\n\n");
		ret = -ENOMEM;
		goto fail_kzalloc_mt_drv_stat_dev;
	}
	mt_drv_stat_dev->id = pdev->id;
	mt_drv_stat_dev->devt = mt_drv_stat_drv->devt + mt_drv_stat_dev->id;
	mt_drv_stat_dev->platdata = dev_get_platdata(&pdev->dev);

#if 0
	mt_drv_stat_dev->dev = device_create(mt_class, &pdev->dev, mt_drv_stat_dev->devt, NULL, UMAP_DEVNAME_STAT);
#else
	mt_drv_stat_dev->dev = device_create(mt_class, NULL, mt_drv_stat_dev->devt, NULL, UMAP_DEVNAME_STAT);
#endif
	if (IS_ERR(mt_drv_stat_dev->dev)) {
		pr_err("Error device_create\n\n");
		ret = PTR_ERR(mt_drv_stat_dev->dev);
		goto fail_device_create;
	}

	platform_set_drvdata(pdev, mt_drv_stat_dev);
	dev_set_drvdata(mt_drv_stat_dev->dev, mt_drv_stat_dev);

    item = mt_drv_proc_add_module(MT_MOD_STAT, NULL, NULL);
    if (!item) {
        goto fail_mt_drv_proc_add_module1;;
    }
    item->read = stat_proc_read;
    item->write = NULL;

    mt_drv_stat_eventfunc_register(stat_event_call);

    /* register low delay statistics proc interface */
    item = mt_drv_proc_add_module(MT_MOD_EVENT, NULL, NULL);
    if (!item) {
        MT_ERR_STAT("cann't register low delay statistics interface.\n");
        goto fail_mt_drv_proc_add_module2;
    }
    item->read = low_delay_proc_read;
    item->write = NULL;

	return 0;

fail_mt_drv_proc_add_module2:
	mt_drv_stat_eventfunc_unregister();
	mt_drv_proc_rm_module(MT_MOD_STAT);
fail_mt_drv_proc_add_module1:
	device_destroy(mt_class, mt_drv_stat_dev->devt);
fail_device_create:
	kfree(mt_drv_stat_dev);
	mt_drv_stat_dev = NULL;
fail_kzalloc_mt_drv_stat_dev:
	mt_drv_mmz_unmap_and_release(&mmz_buffer);
fail_mt_drv_mmz_alloc_and_map:
	return ret;
}

static int mt_drv_stat_remove(struct platform_device *pdev)
{
	struct mt_drv_stat_device *mt_drv_stat_dev = platform_get_drvdata(pdev);

    mt_drv_proc_rm_module(MT_MOD_EVENT);
    mt_drv_stat_eventfunc_unregister();
    mt_drv_proc_rm_module(MT_MOD_STAT);
	mt_drv_mmz_unmap_and_release(&mmz_buffer);

	device_destroy(mt_class, mt_drv_stat_dev->devt);
	mt_drv_stat_dev->dev = NULL;

	platform_set_drvdata(pdev, NULL);

	if (mt_drv_stat_dev) {
		kfree(mt_drv_stat_dev);
		mt_drv_stat_dev = NULL;
	}

	return 0;
}

static int mt_drv_stat_suspend(struct platform_device *pdev, pm_message_t stState)
{
	return 0;
}

static int mt_drv_stat_resume(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver mt_drv_stat_platform_driver = {
	.probe = mt_drv_stat_probe,
	.remove = mt_drv_stat_remove,
	.suspend = mt_drv_stat_suspend,
	.resume = mt_drv_stat_resume,
	.driver = {
		.name = UMAP_DEVNAME_STAT,
		.owner = THIS_MODULE,
	}
};

static void mt_drv_stat_release_device(struct device *pdev) {  }

static struct platform_device mt_drv_stat_platform_device = {
	.name = UMAP_DEVNAME_STAT,
	.id = 0,
	.dev= {
		.platform_data = NULL,
		.release = mt_drv_stat_release_device,
	},
};

int __init mt_drv_stat_init(void)
{
	int ret;

	mt_drv_stat_drv = kzalloc(sizeof(struct mt_drv_stat_driver), GFP_KERNEL);
	if (!mt_drv_stat_drv) {
		pr_err("Error kzalloc mt_drv_stat_drv\n\n");
		ret = -ENOMEM;
		goto fail_kzalloc_mt_drv_stat_drv;
	}

	mt_drv_stat_drv->major = MT_DEVICE_MAJOR;
	mt_drv_stat_drv->minor = UMAP_MIN_MINOR_STAT;
	mt_drv_stat_drv->minors = UMAP_DEV_NUM_STAT;
	mt_drv_stat_drv->devt = MKDEV(mt_drv_stat_drv->major, mt_drv_stat_drv->minor);
	cdev_init(&mt_drv_stat_drv->cdev, &DRV_stat_Fops);
	mt_drv_stat_drv->cdev.owner = THIS_MODULE;

	ret = cdev_add(&mt_drv_stat_drv->cdev, mt_drv_stat_drv->devt, mt_drv_stat_drv->minors);
	if (ret) {
		pr_err("Error cdev_add\n\n");
		ret = -EINVAL;
		goto fail_cdev_add;
	}

	ret = platform_driver_register(&mt_drv_stat_platform_driver);
	if (ret) {
		pr_err("Error platform_driver_register\n\n");
		goto fail_platform_driver_register;
	}

	ret = platform_device_register(&mt_drv_stat_platform_device);
	if (ret) {
		pr_err("Error platform_device_register\n\n");
		goto fail_platform_device_register;
	}

	return 0;

fail_platform_device_register:
	platform_driver_unregister(&mt_drv_stat_platform_driver);
fail_platform_driver_register:
	cdev_del(&mt_drv_stat_drv->cdev);
fail_cdev_add:
	kfree(mt_drv_stat_drv);
	mt_drv_stat_drv = NULL;
fail_kzalloc_mt_drv_stat_drv:
	return ret;
}

void mt_drv_stat_exit(void)
{
	platform_driver_unregister(&mt_drv_stat_platform_driver);
	platform_device_unregister(&mt_drv_stat_platform_device);
	kfree(mt_drv_stat_drv);
	mt_drv_stat_drv = NULL;
}




#if defined(MT_STAT_ISR_SUPPORTED)
mt_void mt_drv_stat_isrreset(mt_void)
{
    memset((void *)g_IsrTime, 0, sizeof(g_IsrTime));
}

mt_void mt_drv_stat_isrenable(mt_void)
{
    g_stat_isr_enable = MT_TRUE;
}

mt_void mt_drv_stat_isrdisable(mt_void)
{
    g_stat_isr_enable = MT_FALSE;
}

mt_void mt_drv_stat_isrbegin(STAT_ISR_E isr)
{
    if(g_stat_isr_enable == MT_FALSE)
        return ;

	if(isr < STAT_ISR_BUTT) {
		ktime_get_real_ts64(&g_IsrTime[isr].tv);
	}
}

mt_void mt_drv_stat_isrend(STAT_ISR_E isr)
{
    if(g_stat_isr_enable == MT_FALSE)
        return ;

    if(isr < STAT_ISR_BUTT)
    {
        struct timespec64 tv, tv2;
        mt_u32 time = 0;

        tv = g_IsrTime[isr].tv;

		ktime_get_real_ts64(&tv2);

        if(tv2.tv_sec > tv.tv_sec)
        {
            time = (tv2.tv_sec - tv.tv_sec)*1000000 + (tv2.tv_nsec - tv.tv_nsec)/1000;
        }
        else
        {
            time = (tv2.tv_nsec - tv.tv_nsec)/1000;
        }

        if(time > g_IsrTime[isr].time)
            g_IsrTime[isr].time = time;
    }
}
#endif

mt_void stat_event_call(STAT_EVENT_E enEvent, mt_u32 Value)
{
    mt_u64   SysTime;

    if ( enEvent >= STAT_EVENT_BUTT )
    {
        return;
    }

    SysTime = sched_clock();

    SysTime = div64(SysTime, 1000000);

    switch ( enEvent )
    {
        case STAT_EVENT_KEYIN :
        case STAT_EVENT_KEYOUT:
        case STAT_EVENT_ISTREAMGET:
        case STAT_EVENT_BUFREADY:
        case STAT_EVENT_FRAMESYNCOK:
            {
                g_EventTime[enEvent].Value1 = Value;
            }
            break;
        case STAT_EVENT_CONNECT:
            {
                g_EventTime[STAT_EVENT_LOCKED].EventMs = 0;
            }
            break;
        case STAT_EVENT_VSTART:
            {
                mt_s32 Index;
                for (Index = STAT_EVENT_CWSET; Index < STAT_EVENT_BUTT; Index++ )
                {
                    g_EventTime[Index].EventMs = 0;
                    g_EventTime[Index].Value1 = 0;
                    g_EventTime[Index].Value2 = 0;
                }
            }
            break;
        case STAT_EVENT_CWSET :
        case STAT_EVENT_LOCKED :
            {
                if (0 != g_EventTime[enEvent].EventMs)
                {
                    return;
                }
            }
            break;
        case STAT_EVENT_IFRAMEINTER:
            {
                g_EventTime[enEvent].Value1 = Value - g_EventTime[enEvent].Value2;
                g_EventTime[enEvent].Value2 = Value;
            }
            break;
        default:
            break;
    }

    g_EventTime[enEvent].EventMs = (mt_u32)SysTime;

    return;
}

#if defined(MT_STAT_ISR_SUPPORTED)
EXPORT_SYMBOL(mt_drv_stat_isrbegin);
EXPORT_SYMBOL(mt_drv_stat_isrend);
#endif
