/*=============================================================================+
|                                                                              |
| Copyright 2013                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file 
*   \brief
*   \author Montage
*/

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/firmware.h>
#include <linux/netdevice.h>
#include <linux/delay.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/host.h>
#include <linux/pm_runtime.h>
#include <linux/irq.h>
#include <linux/list.h>
#include <linux/jiffies.h>


#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
#endif
#if !defined(CONFIG_LYNX_WM_MANAGER)
#include <net/mac80211.h>
#endif
//#include <asm/i387.h>

#ifdef CONFIG_LYNX_W2SDIO_FIRMWARE_ARRAY

#ifdef CONFIG_LYNX_ROM3
#include "sdio_firmware_array_3.h"
#endif /*CONFIG_LYNX_ROM3*/

#ifdef CONFIG_LYNX_ROM2
#include "sdio_firmware_array_2.h"
#endif /*CONFIG_LYNX_ROM2*/

#ifdef CONFIG_MP_FW
#ifdef CONFIG_LYNX_ROM3
#include "mp_sdio_firmware_array_3.h"
#endif /*CONFIG_LYNX_ROM3*/

#ifdef CONFIG_LYNX_ROM2
#include "mp_sdio_firmware_array_2.h"
#endif /*CONFIG_LYNX_ROM2*/
#endif /*CONFIG_MP_FW*/

#endif /*CONFIG_LYNX_W2SDIO_FIRMWARE_ARRAY*/

#include "lynx_rev.h" 
#include "init.h"
#include "wlan_def.h"
#include "core.h"
#include "wci.h"
#include "hif.h"
#include "hw.h"
#include "lynx_debug.h"
#include "sdio.h"

#ifdef CONFIG_ANDROID
#define CUSTOMER_02_HW 1 /*Android Ver*/
#else
//#define CUSTOMER_01_HW 1 /*Embbed Ver*/
#define CUSTOMER_X86_HW 1 /*X86 ver*/
#endif

//#define GET_RX_PTR_PAYLOAD
#define SDIO_RX_HIGH_PRITORY 1 

#ifdef CUSTOMER_01_HW
#define  CONFIG_SDIO_IB_RX_BURST	
#elif defined(CUSTOMER_02_HW)
#define  CONFIG_SDIO_OOB_RX_SYNC	
#elif defined(CUSTOMER_X86_HW)
#define  CONFIG_SDIO_IB_RX_DIRECT
//#define  CONFIG_SDIO_IB_RX_SYNC
#else
#define  CONFIG_SDIO_POLLING		
#endif

#define RETRY_DELAY_MS	1
#define RETRY_THRESHOLD 7
#define USE_WMM_QUEUE
#define CPU_AGGREGATION_ENABLE

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#define LYNX_NO_PM  // temp solution
//#define LYNX_SDIO_DBG
#define IF_SDIO_BLOCK_SIZE 512

/* identify firmware images */
#define FIRMWARE_LYNX_2_0_0     "lynx/sdio_app_2.img"
#define FIRMWARE_LYNX_3_0_0     "lynx/sdio_app_3.img"
//#define LYNX_WAIT_DOWNLOAD_FIRMWARE

#define HIF_SDIO_TX_STOP  BIT(0)
#define MAX_RX_BUF_SIZE   (4*1024)
#define ROM_VER_LENGTH 4

/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
enum SDIO_CMD_DIR {
    SDIO_DIR_IN = 0,    /* to host */
    SDIO_DIR_OUT,       /* to device */
    SDIO_DIR_OUT_SYNC   /* to device sync */
};

struct if_sdio_packet {
    struct list_head list;
    struct sk_buff   *skb;
    u16              nb;
    u8               buffer[0] __attribute__((aligned(4)));
};

enum sdio_special_flag {
    SDIO_ACTIVE,
    SDIO_RX_WAIT,
    SDIO_REMOVE,
    SDIO_IRQ,
    SDIO_BURST,
};

enum sdio_rx_method {
    SDIO_RX_POLLING = 0,     /* NO IRQ or MP FW */                        /*conservative*/
    SDIO_RX_IB_IRQ_DIRECT,  /* In-Band IRQ ; process directly */        /*traditional*/
    SDIO_RX_IB_IRQ_BURST,  /* In-Band IRQ : burst mode */              /*creative: low cpu ratio ,low latency and how throuput*/
    SDIO_RX_OOB_IRQ_SYNC, /* Out-of-BAND IRQ -> work queue */   /*good*/
    SDIO_RX_IB_IRQ_SYNC,  /* In-Band IRQ-> work queue */ 
};

struct if_sdio_card {
    struct lynx_hw      hw; // it must be at first

    struct sdio_func    *func;

    struct sdio_fw_info info;
    u32                 db_start;
    u32                 db_size;
    u32                 db_num;

    wait_queue_head_t   rx_wq;
    unsigned long       flag;

    struct workqueue_struct *rx_workqueue;
    struct work_struct      rx_worker;
    struct workqueue_struct *tx_workqueue;
    struct work_struct      tx_worker;

    struct workqueue_struct *cpu_aggregation_workqueue;
    struct work_struct      cpu_aggregation_worker;

    struct workqueue_struct *tx_recycle_workqueue;
    struct work_struct      tx_recycle_worker;
    struct workqueue_struct *rx_decompress_workqueue;
    struct work_struct      rx_decompress_worker;
    struct list_head        packets;

	const char *fw_name;
	const struct firmware *firmware;
	struct completion fw_done;

	struct lynx_hif_ops *hif_ops;

    /* experiment of continuous skb */
    u8 *t_buf_start, *prod, *cons, *t_buf_end;
    spinlock_t t_buf_lock;
    u8 t_buf_full_flag, t_buf_empty_flag;
    u32 t_buf_size;

    u32 rx_buflen;
    u8 *rx_buffer;
    u8  rx_dummy[MAX_RX_BUF_SIZE] __attribute__((aligned(4)));
    u8 app_mode;
    u8 fatal_error;
    struct sk_buff * skb_aggregated;

    /* storage of working entry for tx_wokrqueue*/
    tx_buffer_entry* tx_buf_entry;

    buffer_list tx_buffer_list;
    enum sdio_rx_method rx_method; 

    u32	oob_irq_num;
    u32	oob_irq_flags;

    bool	oob_irq_registered;
    bool	oob_irq_enabled;
    bool	oob_irq_wake_enabled;

    bool	ib_irq_registered;
    bool	ib_irq_enabled;
    //bool	fw_irq_enabled;

    spinlock_t	irq_spinlock;
    wait_queue_head_t   rx_irq_wq;
    void (*ib_irq_handle) (struct sdio_func *func);
    int rom_ver;
};

/*
 * Align transfer data size for workaround some host controller's
 * bug or poor performance
 */
static inline u32 SDIO_SIZE_ALIGN(u32 len, struct if_sdio_card *card) {
    return (len + (card->info.size_align-1)) & ((u32)~(card->info.size_align-1));
}

#ifdef CONFIG_PROC_FS
static struct if_sdio_card *sdio_card = NULL;
#endif

char default_mac_addr[ETH_ALEN] = {0x00, 0x32, 0x11, 0x98, 0x99, 0x01};
char *user_mac_addr = NULL;

static const struct sdio_device_id if_sdio_ids[] = {
#ifdef CONFIG_LYNX_ROM3
    { SDIO_DEVICE(SDIO_VENDOR_ID_MONTAGE,
            SDIO_DEVICE_ID_LYNX_ROM3) },
#endif /*CONFIG_LYNX_ROM3*/

#ifdef CONFIG_LYNX_ROM2
    { SDIO_DEVICE(SDIO_VENDOR_ID_MONTAGE,
            SDIO_DEVICE_ID_LYNX_ROM2) },
#endif /*CONFIG_LYNX_ROM2*/

    { SDIO_DEVICE(SDIO_VENDOR_ID_MONTAGE,
            SDIO_DEVICE_ID_LYNX_FPGA) },
    { /* end: all zeroes */             },
};

static inline bool buf_needs_bounce(u8 *buf)
{
	return ((unsigned long) buf & 0x3) || !virt_addr_valid(buf);
}


#ifdef CONFIG_SUPPORT_MONITOR_MODE
static unsigned int monitor_level = 0;
module_param(monitor_level, uint, 0644);
MODULE_PARM_DESC(monitor_level, " Monitor level 0-5 : (default:0)\n" \
				"                (0: Monitor data frame ; 1: Monitor beacon/probe_req frame ;\n" \
				"                (2: Monitor mgmt frame  ; 3: Monitor all ; 4 : OMNICFG ; 5 : OMNICFG (with beacon)\n");
#endif /*CONFIG_SUPPORT_MONITOR_MODE */

/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/

static int if_sdio_power_off(struct if_sdio_card *card);
int sdio_IB_irq_register(struct if_sdio_card *card, sdio_irq_handler_t *handler);
int sdio_IB_irq_deregister(struct if_sdio_card *card);
static void if_sdio_IB_irq_enable(struct if_sdio_card *card,bool enable);
static void sdio_FW_irq_enable(struct if_sdio_card *card, int enable) ;

void if_sdio_OOB_irq_unregister(struct if_sdio_card *card);
static void if_sdio_OOB_irq_enable(struct if_sdio_card *card,bool enable);

/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/
extern unsigned int mp_test_firm;
extern void lynx_rx_tasklet(unsigned long data);


#ifdef CUSTOMER_02_HW
extern void sdio_reinit(void);
extern void extern_wifi_set_enable(int is_on);
extern int wifi_irq_num(void);
#endif /*CUSTOMER_02_HW*/


u32 wifi_oob_irq_num(void)
{
	u32 host_oob_irq = 0;
	
#ifdef CUSTOMER_02_HW
	host_oob_irq = (u32)wifi_irq_num();
#endif /*CUSTOMER_02_HW*/

	printk("host_oob_irq: %d \n", host_oob_irq);
	return host_oob_irq;
}
u32 wifi_oob_irq_flags(void)
{
	u32 host_oob_irq_flags = 0;
	host_oob_irq_flags =(IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL | IORESOURCE_IRQ_SHAREABLE) 
						& IRQF_TRIGGER_MASK;
	printk("host_oob_irq_flags : %d\n", host_oob_irq_flags);
	return host_oob_irq_flags;
}

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
static void sdio_hexdump(void *buf, u32 len)
{
	print_hex_dump(KERN_INFO, "", DUMP_PREFIX_OFFSET,
			16, 1,
			buf, len, false);
}
	
/*
 * Return:
 *	0:	power on successfully
 *	others: power on failed
 */
#ifdef CUSTOMER_02_HW
int platform_wifi_power_on(void)
{
	int ret = 0;
	printk("######%s: \n",__func__);
	
	printk("--- power_off ---\n");
	extern_wifi_set_enable(0);
	msleep(1000);

	printk("--- power_on ---\n");
	extern_wifi_set_enable(1);
	msleep(1000);
	msleep(500);
	printk("--- sdio_reinit ---\n");
	sdio_reinit();
	return ret;
}

void platform_wifi_power_off(void)
{
	printk("######%s: \n",__func__);
	printk("--- power_off ---\n");
	extern_wifi_set_enable(0);

	/*Push SDIO host remove SDIO card/bus for w7000s*/
	printk("--- sdio_reinit ---\n");
	sdio_reinit();
}
#endif /*CUSTOMER_02_HW*/

static u32 sdio_dma_base_addr = 0;

static u8 last_rx_seq_no = 0xff;
static u8 tx_seq_no = 0;

/********************************************************************/
/* I/O                                                              */
/********************************************************************/
u32 tx_ptr = 0; /* it's sdio addr */
u32 tx_buf_start = 0;
u32 tx_buf_end = 0;
u32 tx_buf_ptr = 0;
u32 tx_buf_reader_ptr = 0;
u32 tx_buf_size = 0;
u32 full = 0;
u32 debug_on = 0;
u32 empty_cnt = 0;
u32 pkt_cnt = 0;
u32 tx_size_align_sh = 0;
u32 rx_buf_start = 0;
u32 rx_buf_end = 0;
u32 rx_buf_reader_ptr = 0;
u32 rx_buf_write_ptr= 0;
u32 rx_buf_size = 0;
u32 rx_full_flag = 0;
u8 int_stop = 0;
#ifdef GET_RX_PTR_PAYLOAD
u8 get_ptr_cmd = 1;
#else
u8 get_ptr_cmd = 0;
#endif /**GET_RX_PTR_PAYLOAD*/
u8 get_ptr_debug = 0;


u8 tx_buffer_list_count(buffer_list* tx_buffer_list) {
    u8 result;
    spin_lock(&tx_buffer_list->t_buf_lock);
    result = tx_buffer_list->buffer_cnt ;
    spin_unlock(&tx_buffer_list->t_buf_lock);
    return result;
}


u8 tx_buffer_list_full(buffer_list* tx_buffer_list) {
    u8 result;
    spin_lock(&tx_buffer_list->t_buf_lock);
    result = (tx_buffer_list->buffer_cnt >= TX_BUFFER_MAX_NUM)? 1:0;
    spin_unlock(&tx_buffer_list->t_buf_lock);
    return result;
}

tx_buffer_entry* tx_buffer_allocate(void) {//struct if_sdio_card *card
    tx_buffer_entry* entry;

    entry = (tx_buffer_entry*)kmalloc(sizeof(*entry), GFP_KERNEL);
    if (!entry) {
        pr_err("%s() %d failed alocating memory\n", __func__, __LINE__);
        goto FAIL;
    }
#if 1
    entry->buf = (u8*)kmalloc(TX_BUFFER_SIZE, GFP_KERNEL);
    if (!entry->buf) {
        pr_err("%s() %d failed alocating memory (size 0x%x)\n", __func__, __LINE__, TX_BUFFER_SIZE);
        goto FAIL;
    }
#else
    entry->buf = dma_alloc_coherent(NULL, TX_BUFFER_SIZE, &entry->dma_handle, GFP_ATOMIC);
    if (!entry->buf) {
        pr_err("%s() %d failed alocating memory (size 0x%x)\n", __func__, __LINE__, TX_BUFFER_SIZE);
        goto FAIL;
    }
#endif
    entry->size = TX_BUFFER_SIZE;
    INIT_LIST_HEAD(&entry->list_head);
    entry->end = 0;
    entry->start = 0;

    return entry;

FAIL:
    if (entry)
        kfree(entry);
    return NULL;
}

void tx_buffer_free(tx_buffer_entry* entry) {
    if (entry->buf) {
#if 1
        kfree(entry->buf);
#else
        dma_free_coherent(NULL, TX_BUFFER_SIZE, entry->buf, entry->dma_handle);
#endif
    }
    if (entry)
        kfree(entry);
}

void tx_buffer_enqueue(tx_buffer_entry* entry, buffer_list* tx_buffer_list) {
    spin_lock(&tx_buffer_list->t_buf_lock);
    list_add_tail(&entry->list_head, &tx_buffer_list->buffer_list);
    tx_buffer_list->buffer_cnt++;
    if (tx_buffer_list->buffer_cnt > TX_BUFFER_MAX_NUM)
        pr_warn("%s() buffer_cnt %d exceed threshold\n", __func__, tx_buffer_list->buffer_cnt);
    spin_unlock(&tx_buffer_list->t_buf_lock);
}

tx_buffer_entry* tx_buffer_dequeue(buffer_list* tx_buffer_list) {
    tx_buffer_entry* tmp = NULL;
    struct list_head* temp;

    spin_lock(&tx_buffer_list->t_buf_lock);
    if (!list_empty(&tx_buffer_list->buffer_list)) {
        temp = tx_buffer_list->buffer_list.next;
        list_del(temp);
        tmp = list_entry(temp, tx_buffer_entry, list_head);
        tx_buffer_list->buffer_cnt--;
    }
    spin_unlock(&tx_buffer_list->t_buf_lock);
    return tmp;
}

static u32 get_tx_reader_ptr(struct if_sdio_card *card)
{
    int err = 0;
    u8 retry = 0;
    u8 tmp;
    u32 tx_consumer_sdio_addr = tx_ptr + 10;


RETRY:

    if (retry > RETRY_THRESHOLD){
        pr_err("%s:%d ERROR (err:%d)\n", __func__, __LINE__, err);
        return -1;
    }

    if (test_bit(SDIO_REMOVE, &card->flag)) 
        return -1;

    tmp = sdio_f0_readb(card->func, tx_consumer_sdio_addr, &err);
    if (err) {
        if (retry++ < RETRY_THRESHOLD) {
            //pr_err("%s:%d error retry (err:%d)\n", __func__, __LINE__, err);
            //pr_err("delay %d ms\n", retry*RETRY_DELAY_MS);
            mdelay(retry*RETRY_DELAY_MS);
            goto RETRY;
        }
        pr_err("%s:%d ERROR (err:%d)\n", __func__, __LINE__, err);
    } else {

	if((tx_buf_start + tmp*card->info.size_align) > tx_buf_end){
		retry ++;
		mdelay(retry*RETRY_DELAY_MS);
		goto RETRY;
	}

	if(tx_buf_reader_ptr != tx_buf_start + tmp*card->info.size_align){
		full = 0;
	}

        tx_buf_reader_ptr = tx_buf_start + tmp*card->info.size_align;
        //pr_err("%s() :tx_buf_reader_ptr 0x%x( %d)\n",__func__, tx_buf_reader_ptr, tmp);

    }
    return err;
}

#if 1
static int set_tx_producer_ptr(struct if_sdio_card *card, u32 buf_ptr)
{
    int err = 0;
    u32 tx_producer_sdio_addr = tx_ptr + 9;
    u8 tmp;
    int retry = 0;

    if (!card->info.fast_tx_commit_enable) {

        tmp = (buf_ptr - tx_buf_start) >> tx_size_align_sh;
RETRY:
        if (test_bit(SDIO_REMOVE, &card->flag)) 
            return -1;

        sdio_f0_writeb(card->func, tmp, tx_producer_sdio_addr, &err);
        if (err) {
            if (retry++ < RETRY_THRESHOLD) {
                //pr_err("%s:%d error retry (err:%d)\n", __func__, __LINE__, err);
                //pr_err("delay %d ms\n", retry*RETRY_DELAY_MS);
                mdelay(retry*RETRY_DELAY_MS);
                goto RETRY;
            }
            pr_err("%s:%d ERROR (err:%d)\n", __func__, __LINE__, err);
        } else
            pr_debug("%s:%d write sucessful : 0x%x\n", __func__, __LINE__, tmp);

            if (tx_buf_ptr == tx_buf_reader_ptr)
                full = 1;
           else
                full = 0;

    }
    return err;
}

#else

static int _set_tx_producer_ptr(struct if_sdio_card *card, u32 buf_ptr)
{
    int err = 0;
    u32 tx_producer_sdio_addr = tx_ptr + 9;
    u8 tmp;
    int retry = 0;

    if (test_bit(SDIO_REMOVE, &card->flag)) 
        return -1;

    if (!card->info.fast_tx_commit_enable) {

        tmp = (buf_ptr - tx_buf_start) >> tx_size_align_sh;
RETRY:
        sdio_f0_writeb(card->func, tmp, tx_producer_sdio_addr, &err);
        if (err) {
            if (retry++ < RETRY_THRESHOLD) {
                //pr_err("%s:%d error retry (err:%d)\n", __func__, __LINE__, err);
                //pr_err("delay %d ms\n", retry*RETRY_DELAY_MS);
                mdelay(retry*RETRY_DELAY_MS);
                goto RETRY;
            }
            pr_err("%s:%d ERROR (err:%d)\n", __func__, __LINE__, err);
        } else
            pr_debug("%s:%d write sucessful : 0x%x\n", __func__, __LINE__, tmp);
    }

    return err;
}

static int set_tx_producer_ptr(struct if_sdio_card *card, u32 buf_ptr)
{
    int err = 0;
    u32 tx_producer_sdio_addr = tx_ptr + 9;
    u8 tmp;
    int retry = 0;

    if (test_bit(SDIO_REMOVE, &card->flag)) 
        return -1;

    err = _set_tx_producer_ptr(card, buf_ptr);

    if (!card->info.fast_tx_commit_enable) {
    
RETRY:
        tmp = sdio_f0_readb(card->func, tx_producer_sdio_addr, &err);
        if (err) {
            if (retry++ < RETRY_THRESHOLD) {
                //pr_err("%s:%d error retry (err:%d)\n", __func__, __LINE__, err);
                //pr_err("delay %d ms\n", retry*RETRY_DELAY_MS);
                mdelay(retry*RETRY_DELAY_MS);
                goto RETRY;
            }
            pr_err("%s:%d ERROR (err:%d)\n", __func__, __LINE__, err);
        } else {
            pr_debug("%s:%d read sucessful : 0x%x\n", __func__, __LINE__, tmp);
            if ((tmp*card->info.size_align + tx_buf_start) != buf_ptr) {
                pr_err("%s:%d FIFO is not empty out, retry again\n", __func__, __LINE__);
                retry = 0;
                goto RETRY;
            } else {
                err = 0;
            }
        }
    
    }

    return err;
}

#endif

static u32 tx_buffer_len_remain(struct if_sdio_card *card, buf_len_remain* bf){
    s32 remain = 0;

    if (full) {
        return 0;
    }

    if (tx_buf_ptr == tx_buf_reader_ptr) {
        /* empty */
        remain = tx_buf_size;
    } else {
        if (tx_buf_ptr > tx_buf_reader_ptr)
            remain = tx_buf_size - (tx_buf_ptr - tx_buf_reader_ptr);
        else
            remain = tx_buf_reader_ptr - tx_buf_ptr;
    }
    remain -= card->info.size_align;
    if (remain < 0)
        remain = 0;

    if (bf) {
        bf->buf_remain = remain;
        if ((tx_buf_ptr + bf->buf_remain) <= tx_buf_end) {
            bf->remain1 = bf->buf_remain;
            bf->remain2 = 0;
        } else {
            bf->remain1 = tx_buf_end - tx_buf_ptr;
            bf->remain2 = bf->buf_remain - bf->remain1;
        }
    }

     return remain;
}

static int tx_buffer_is_available(struct if_sdio_card *card, u32 len){
    u32 remain = 0;

    if (len > tx_buf_size) {
        pr_err("%s() packet is too large to transfer!\n", __func__);
        return false;
    }

    /* if the full is new fresh, it should be considered as high prority */
    if (full) {
        return false;
    }

    if (tx_buf_ptr == tx_buf_reader_ptr) {
        /* empty */
        //pr_err("%s() it's empty (full %d)\n", __func__, full);
        empty_cnt++;
        return (!full);
    } else {
        if (tx_buf_ptr > tx_buf_reader_ptr) {
            remain = tx_buf_reader_ptr - tx_buf_ptr + tx_buf_size;
            //pr_err("%s() negative (remain 0x%x)\n", __func__, remain);
            return (len <= (remain-card->info.size_align));
        } else {
            remain = tx_buf_reader_ptr - tx_buf_ptr;
            //pr_err("%s() positive (remain 0x%x)\n", __func__, remain);
            return (len <= (remain-card->info.size_align));
        }
    }
}
static void tx_buffer_update(struct if_sdio_card *card, u32 len) {

	if (len != 0){
	    if ((tx_buf_ptr + len) == tx_buf_end) {
	        tx_buf_ptr = tx_buf_start;
	        //pr_err("%s() %d\n", __func__, __LINE__);
	    } else if ((tx_buf_ptr + len) > tx_buf_end) {
	        tx_buf_ptr = tx_buf_start +
	            (tx_buf_ptr + len) - tx_buf_end;
	        //pr_err("%s() %d\n", __func__, __LINE__);
	    } else {
	        tx_buf_ptr += len;
	        //pr_err("%s() %d\n", __func__, __LINE__);
	    }
	}


    //pr_err("%s() tx_buf_ptr 0x%x full 0x%x", __func__, tx_buf_ptr, full);
}

static void tx_buffer_dump(void){
    pr_err("%s() start 0x%x, end 0x%x, tx_buf_ptr 0x%x, tx_buf_reader_ptr 0x%x, tx_buf_size 0x%x, full %d\n",
        __func__, tx_buf_start, tx_buf_end,
        tx_buf_ptr, tx_buf_reader_ptr, tx_buf_size, full);
}

static int tx_commit(struct if_sdio_card *card)
{
    int err = 0;
    int retry = 0;

    if (!card->info.fast_tx_commit_enable) {
RETRY:
        if (test_bit(SDIO_REMOVE, &card->flag)) 
            return -1;

        sdio_f0_writeb(card->func, (unsigned char)SDIO_HINT_SW_TX_DONE, SDIO_CCCR_HINT_SET, &err);
        if (err) {
            if (retry++ < RETRY_THRESHOLD) {
                //pr_err("%s:%d error retry (err:%d)\n", __func__, __LINE__, err);
                //pr_err("delay %d ms\n", retry*RETRY_DELAY_MS);
                mdelay(retry*RETRY_DELAY_MS);
                goto RETRY;
            }
            pr_err("%s:%d ERROR (err:%d)\n", __func__, __LINE__, err);
        }
    }

    return err;
}

static int get_rx_full_flag_ptr(struct if_sdio_card *card)
{
	int err = 0;
	u8 retry = 0;
	u8 tmp=0;
	u32 rx_full_flag_sdio_addr = tx_ptr + 19;

RETRY:
	if (retry > RETRY_THRESHOLD){
		pr_err("%s:%d ERROR (err:%d)\n", __func__, __LINE__, err);
		return -1;
	}
	
	if (test_bit(SDIO_REMOVE, &card->flag)) 
		return -1;

	tmp = sdio_f0_readb(card->func, rx_full_flag_sdio_addr, &err);
	if (err) {
		if (retry++ < RETRY_THRESHOLD) {
			//pr_err("%s:%d error retry (err:%d)\n", __func__, __LINE__, err);
			//pr_err("delay %d ms\n", retry*RETRY_DELAY_MS);
			mdelay(retry*RETRY_DELAY_MS);
			goto RETRY;
		}
		pr_err("%s:%d ERROR (err:%d)\n", __func__, __LINE__, err);
	} else {

		if((0 !=tmp) && (1 !=tmp)){
			retry ++;
			mdelay(retry*RETRY_DELAY_MS);
			goto RETRY;
		}

		rx_full_flag = tmp;
		//pr_err("%s() :rx_full_flag %d\n",__func__, rx_full_flag);
	}
	
	return err;
}

static int get_rx_producer_ptr(struct if_sdio_card *card)
{
    int err = 0;
    u8 retry = 0;
    u8 tmp;
    u32 rx_producer_sdio_addr = tx_ptr + 20;

RETRY:

    if (retry > RETRY_THRESHOLD){
        pr_err("%s:%d ERROR (err:%d)\n", __func__, __LINE__, err);
        return -1;
    }
	
    if (test_bit(SDIO_REMOVE, &card->flag)) 
        return -1;

    tmp = sdio_f0_readb(card->func, rx_producer_sdio_addr, &err);
    if (err) {
        if (retry++ < RETRY_THRESHOLD) {
            //pr_err("%s:%d error retry (err:%d)\n", __func__, __LINE__, err);
            //pr_err("delay %d ms\n", retry*RETRY_DELAY_MS);
            mdelay(retry*RETRY_DELAY_MS);
            goto RETRY;
        }
        pr_err("%s:%d ERROR (err:%d)\n", __func__, __LINE__, err);
    } else {
        if((rx_buf_start + tmp*card->info.size_align) > rx_buf_end){
           retry ++;
           mdelay(retry*RETRY_DELAY_MS);
           goto RETRY;
        }
        rx_buf_write_ptr = (rx_buf_start + tmp*card->info.size_align);
        //pr_err("%s() :rx_buf_write_ptr 0x%x( %d)\n",__func__, rx_buf_write_ptr, tmp);

        if (rx_buf_write_ptr != rx_buf_reader_ptr){
           rx_full_flag = 0;
        }

    }

    return err;
}

static int set_rx_consumer_ptr(struct if_sdio_card *card, u32 buf_ptr)
{
    int err = 0;
    u32 rx_consumer_sdio_addr = tx_ptr + 21;
    u8 tmp;
    int retry = 0;

    if (!card->info.fast_rx_commit_enable) {

        tmp = (buf_ptr - rx_buf_start) >> tx_size_align_sh;
RETRY:
        if (test_bit(SDIO_REMOVE, &card->flag)) 
            return -1;

        sdio_f0_writeb(card->func, tmp, rx_consumer_sdio_addr, &err);
        if (err) {
            if (retry++ < RETRY_THRESHOLD) {
                //pr_err("%s:%d error retry (err:%d)\n", __func__, __LINE__, err);
                //pr_err("delay %d ms\n", retry*RETRY_DELAY_MS);
                mdelay(retry*RETRY_DELAY_MS);
                goto RETRY;
            }
            pr_err("%s:%d ERROR (err:%d)\n", __func__, __LINE__, err);
        } else
            pr_debug("%s:%d write sucessful : 0x%x\n", __func__, __LINE__, tmp);
    }

    return err;
}

static u32 rx_buffer_len_remain(struct if_sdio_card *card,u8 sync){
    s32 remain = 0;
 
    if((card->app_mode == 1) && ((rx_buf_reader_ptr < rx_buf_start) || (rx_buf_reader_ptr > rx_buf_end))){
      pr_err("rx_buffer_len_remain():  rx_buf_reader_ptr %x error\n", rx_buf_reader_ptr);
      return 0;
    }

    if((card->app_mode == 1) && ((rx_buf_write_ptr < rx_buf_start) || (rx_buf_write_ptr > rx_buf_end))){
      pr_err("rx_buffer_len_remain():  rx_buf_write_ptr %x error\n", rx_buf_write_ptr);
      return 0;
    }

    if (rx_buf_write_ptr == rx_buf_reader_ptr) {
        //check if full
        if(sync)
            get_rx_full_flag_ptr(card);

        if (rx_full_flag){
            remain = rx_buf_size;
        }else{
            remain = 0;
            //sync rx consumer ptr
            if(sync)
                set_rx_consumer_ptr(card, rx_buf_reader_ptr);
        }
    } else {
        if (rx_buf_write_ptr > rx_buf_reader_ptr)
            remain = rx_buf_write_ptr - rx_buf_reader_ptr;
        else
            remain = rx_buf_size - (rx_buf_reader_ptr - rx_buf_write_ptr);
    }

    return remain;
}

static int rx_buffer_update(struct if_sdio_card *card, u32 len) {
    u32 temp = rx_buf_reader_ptr;
    if ((rx_buf_reader_ptr + len) == rx_buf_end){
        rx_buf_reader_ptr = rx_buf_start;
    }else if((rx_buf_reader_ptr + len) > rx_buf_end){
        temp = rx_buf_start +(rx_buf_reader_ptr + len) - rx_buf_end;
        if((card->app_mode == 1) && ((temp < rx_buf_start) || (temp > rx_buf_end))){
          pr_err("rx_buffer_update(): new rx_buf_reader_ptr %x error, rx_buf_reader_ptr %x \n", temp,rx_buf_reader_ptr); 
          return -1;
        }
        rx_buf_reader_ptr = temp ;
    }else{
        temp += len;
        if((card->app_mode == 1) && ((temp < rx_buf_start) || (temp > rx_buf_end))){
          pr_err("rx_buffer_update(): new rx_buf_reader_ptr %x error, rx_buf_reader_ptr %x \n", temp,rx_buf_reader_ptr );
          return -1;
        }
        rx_buf_reader_ptr = temp ;
    }

    //rx_full_flag = 0;
    return 0;
}

static void rx_buffer_dump(void){
    pr_err("%s() start 0x%x, end 0x%x, rx_buf_write_ptr 0x%x, rx_buf_reader_ptr 0x%x, rx_buf_size 0x%x, full %d\n",
        __func__, rx_buf_start, rx_buf_end,
        rx_buf_write_ptr, rx_buf_reader_ptr, rx_buf_size, rx_full_flag);
}

static int rx_commit(struct if_sdio_card *card)
{
    int err = 0;
    int retry = 0;

    if (!card->info.fast_rx_commit_enable) {
RETRY:
        if (test_bit(SDIO_REMOVE, &card->flag)) 
            return -1;

        sdio_f0_writeb(card->func, (unsigned char)SDIO_HINT_SW_RX_DONE, SDIO_CCCR_HINT_SET, &err);
        if (err) {
            if (retry++ < RETRY_THRESHOLD) {
                //pr_err("%s:%d error retry (err:%d)\n", __func__, __LINE__, err);
                //pr_err("delay %d ms\n", retry*RETRY_DELAY_MS);
                mdelay(retry*RETRY_DELAY_MS);
                goto RETRY;
            }
            pr_err("%s:%d ERROR (err:%d)\n", __func__, __LINE__, err);
        }
    }
    return err;
}

int sdio_transfer_scatter_list(struct if_sdio_card *card, struct scatterlist* sg, u32 seg_num, u32 sg_tot_len)
{
    struct mmc_request mmc_req;
    struct mmc_command cmd;
    struct mmc_data data;
    int status;

    memset(&mmc_req, 0, sizeof(struct mmc_request));
    memset(&cmd, 0, sizeof(struct mmc_command));
    memset(&data, 0, sizeof(struct mmc_data));

    /* set data */
    data.blksz = le32_to_cpu(card->info.size_align);
    data.blocks = sg_tot_len / data.blksz;
    data.flags = MMC_DATA_WRITE;// MMC_DATA_READ
    data.sg = sg;
    data.sg_len = seg_num;

    /* set command */
    set_cmd53_arg(&cmd.arg, CMD53_ARG_WRITE,
        1, //func number
        CMD53_ARG_BLOCK_BASIS,
        CMD53_ARG_INCR_ADDRESS,
        tx_buf_ptr - sdio_dma_base_addr, //sdio addr
        data.blocks);
    cmd.opcode = SD_IO_RW_EXTENDED;
    cmd.flags = MMC_RSP_SPI_R5 | MMC_RSP_R5 | MMC_CMD_ADTC;
    mmc_req.cmd = &cmd;
    mmc_req.data = &data;

    sdio_claim_host(card->func);

    mmc_set_data_timeout(&data, card->func->card);

    /* synchronous call to process request */
    mmc_wait_for_req(card->func->card->host, &mmc_req);

    sdio_release_host(card->func);

    status = cmd.error ? cmd.error : data.error;
    if (status)
        pr_err("Scatter write request failed:%d\n", status);

    return status;
}


int sdio_transfer(struct if_sdio_card *card, void *buf,
	unsigned int sdio_addr, int len, bool read)
{
    int err = -1, retry = 0;;

    if (!len) {
        pr_err("%s() len can't be set to zero and return\n", __func__);
        return err;
    }
    if (len & (card->info.size_align-1)) {
        pr_err("%s() len is not aligned\n", __func__);
        return err;
    }
 
    if (buf_needs_bounce(buf))
        pr_err("%s() buffer isn't DMA-able (from %pS())\n",
            __func__, __builtin_return_address(0));

RETRY:

    if (test_bit(SDIO_REMOVE, &card->flag)){
        pr_err("%s() card is removed\n", __func__);
        return 0;
    }

    if (read) {
        err = sdio_memcpy_fromio(card->func, buf, sdio_addr, len);
    } else {
        //pr_err("%s()write data size %u buf 0x%p sdio_addr 0x%x (direction %d from %pS)\n", __func__, len, buf, sdio_addr, read, __builtin_return_address(0));
        err = sdio_memcpy_toio(card->func, sdio_addr, buf, len);
    }

    if (err) {
        if (retry++ < RETRY_THRESHOLD) {
            //pr_err("%s:%d error retry (err:%d)\n", __func__, __LINE__, err);
            //pr_err("data size %u buf 0x%p sdio_addr 0x%x ret %d retry %d (direction %d from %pS)\n",
            //	        len, buf, sdio_addr, err, retry, read, __builtin_return_address(0));
            //pr_err("delay %d ms\n", retry*RETRY_DELAY_MS);
            mdelay(retry*RETRY_DELAY_MS);
            goto RETRY;
        }
        pr_err("%s:%d ERROR (err:%d)\n", __func__, __LINE__, err);
        pr_err("data size %u buf 0x%p sdio_addr 0x%x ret %d retry %d (direction %d from %pS)\n",
			        len, buf, sdio_addr, err, retry, read, __builtin_return_address(0));
    }

    return err;
}

/* Get data from SDIO interface to rx_compress_queue*/
/* INPUT  :  set "exist" as true for return -1 when no get data  */
/* INPUT  : set "noloop" as true for juest query rx date once*/
/* OUTPUT: "exist" means get data or not                  */
/* OUTPUT: "load" means high load                  */
static int if_sdio_card_to_host(struct if_sdio_card *card,bool* exist,bool* load,bool* noloop)
{
	struct sdio_func *func = card->func;
	struct lynx_hif_device *hdev = sdio_get_drvdata(func);
	int err = 0;
	u32 bytes = 0;
	struct sk_buff *skb1 = NULL, *skb2 = NULL;
	u32 seg1, seg2, size1, size2, transfer_len;
	int rev_cnt = 0;

	int get_ok = 0;
	u8 tmp= 0xff;
	sdio_header* hdr;
	u8 *data;

	if(card->fatal_error) {
		pr_err("%s %d: can't work anymore\n", __func__, __LINE__);
		return 0;
	}

	if( NULL != load ){
		*load = false;/*default value for not get data*/
	}

	sdio_claim_host(card->func);

	while(1){
		if (test_bit(SDIO_REMOVE, &card->flag)) {
			sdio_release_host(card->func);
			goto end;
		}
		//pr_debug("%s() start to collect compressed skb\n", __func__);

		if((0 == card->app_mode ) || (!get_ok) || (!get_ptr_cmd)){
			/* boot mode or init or not get value*/
			/* old style*/
			get_rx_producer_ptr(card);
			bytes = rx_buffer_len_remain(card,1);/* resync */
		}else{
			/* app mode and get value */
			/* new style : get rx_buf_write_ptr/rx_full_flag from skb header*/
			/* rx_buf_write_ptr/rx_full_flag update already */
			bytes = rx_buffer_len_remain(card,0); /*not need sync,fast*/
		}

		if(bytes == 0) {
			//pr_debug("%s() rx is empty\n", __func__);
			err = 0;
			break;
		}

		get_ok = 0;

		//rx_buffer_dump();
		transfer_len = 0;
		if((rx_buf_reader_ptr + bytes) > rx_buf_end) {
			seg1 = rx_buf_reader_ptr;
			size1 = rx_buf_end - rx_buf_reader_ptr;
			seg2 = rx_buf_start;
			size2 = (rx_buf_reader_ptr + bytes) - rx_buf_end;
		}else{
			seg1 = rx_buf_reader_ptr;
			size1 = bytes;
			size2 = 0;
		}

		skb1 = dev_alloc_skb(size1);
		if(!skb1) {
			pr_err("%s() failed to allocated skb1,%d \n", __func__,  size1);
			err = -1;
			//pr_err("%s() invoke rx_decompress_worker cause no mem\n", __func__);
			//queue_work(card->rx_decompress_workqueue, &card->rx_decompress_worker);
			break;
		}
		skb_put(skb1, size1);
		//pr_debug("%s() transfer data (seg1)! rx_buf_reader_ptr 0x%x transfer_len 0x%x\n", __func__, seg1, size1);
		err = sdio_transfer(card, skb1->data, seg1 - sdio_dma_base_addr, size1, 1);
		if(!err){
			transfer_len += size1;

			pkt_cnt++;
			//pr_debug("%s() %d transfer done\n", __func__, __LINE__);
		}else{
			//pr_err("%s() %d err %d\n", __func__, __LINE__, err);
			/* dont go further any more */
			size2 = 0;
		}

		if(size2) {
			skb2 = dev_alloc_skb(size2);
			if (!skb2) {
				pr_err("%s() failed to allocated skb2\n", __func__);
				dev_kfree_skb(skb1);
				err = -1;
				break;
			}
			
			skb_put(skb2, size2);
			//pr_debug("%s() transfer data (seg2)! rx_buf_reader_ptr 0x%x transfer_len 0x%x\n", __func__, seg2, size2);
			err = sdio_transfer(card, skb2->data, seg2 - sdio_dma_base_addr, size2, 1);
			
			if (!err) {
			transfer_len += size2;
			pkt_cnt++;
			// pr_debug("%s() %d transfer done\n", __func__, __LINE__);
			} else {
				pr_err("%s() %d err %d\n", __func__, __LINE__, err);
			}
		}

		/* update local pointer for TX buffer producer */
		if((!err) && (transfer_len >= 0)){

			/* new style : get rx_buf_write_ptr from skb header*/
			if((card->app_mode == 1)&&(get_ptr_cmd)){
				data = skb1->data;
				hdr = (sdio_header*)data;
				get_ok = 1;

				tmp = (u8)(((hdr->res) & 0xff00) >> 8) ;
				if((tmp != 0xff) && 
					((rx_buf_start + tmp*card->info.size_align) <= rx_buf_end)){
					
					rx_buf_write_ptr = (rx_buf_start + tmp*card->info.size_align);
					if(get_ptr_debug)
						pr_err("%s() :rx_buf_write_ptr 0x%x( %d)\n",__func__,rx_buf_write_ptr, tmp);
				}else{
					get_ok = 0;
				}

				tmp = (u8)((hdr->res) & 0x00ff);
				if(( 0 == tmp ) || ( 1 == tmp )){
					rx_full_flag = tmp;
					if(get_ptr_debug)
						pr_err("%s() :rx_full_flag %d\n",__func__, tmp);
				}else{
					get_ok = 0;
				}

				tmp = hdr->offset;
				if((tmp != 0xff) &&
					((tx_buf_start + tmp*card->info.size_align) <= tx_buf_end)){

					if(tx_buf_reader_ptr != tx_buf_start + tmp*card->info.size_align){
						full = 0;
					}

					tx_buf_reader_ptr = (tx_buf_start + tmp*card->info.size_align);
					if(get_ptr_debug)
						pr_err("%s() :tx_buf_reader_ptr 0x%x( %d)\n",__func__, tx_buf_reader_ptr, tmp);
				}else{
					get_ok = 0;
				}

			}

			//pr_debug("%s() %d update ring buffer pointer\n", __func__, __LINE__);
			spin_lock(&hdev->rx_compress_lock);
			__skb_queue_tail(&hdev->rx_compress_queue, skb1);
			if(size2)
				__skb_queue_tail(&hdev->rx_compress_queue, skb2);

			if( NULL != load ){
				if(skb_queue_len(&hdev->rx_compress_queue) >=2)
					*load = true;
			}

			spin_unlock(&hdev->rx_compress_lock);
			rev_cnt++;
			err = rx_buffer_update(card, transfer_len);
			
			if(err){
				break;
			}else{
					err = set_rx_consumer_ptr(card, rx_buf_reader_ptr);
					
					if(err){
						card->fatal_error = 1;
						pr_err("%s() %d this is fatal error \n", __func__, __LINE__);
					}else{

						if (rx_buf_write_ptr != rx_buf_reader_ptr){
							rx_full_flag = 0;
						}
						
						err = rx_commit(card);

						if (err) {
							card->fatal_error = 1;
							pr_err("%s() %d this is fatal error \n", __func__, __LINE__);
						}
					}
			}
		} else {
			if(skb1)
				dev_kfree_skb(skb1);
			if(skb2)
				dev_kfree_skb(skb2);
		}

		/* queue data */
		if(err){
			//pr_err("%s() recevice error (err : %d)\n", __func__, err);
			break;
		}

		//pr_debug("%s() %d invoke rx_decompress_worker\n", __func__, __LINE__);
		queue_work(card->rx_decompress_workqueue, &card->rx_decompress_worker);
		
		if( NULL != noloop ){
			if (true == *noloop) {
				break;
			}
		}

		
	}/*while(1)*/

	sdio_release_host(card->func);

end:

	if( NULL != exist ){
		if ((true == *exist) && ( 0 == rev_cnt )){
			err = -1;
		}

		if( rev_cnt > 0 ){
			*exist = true;
		}else{
			*exist = false;
		}
	}

	//if (!pkt_cnt){
	//pr_info("%s() there is a unnecessary interrupt that inform us with no data\n", __func__);
	//} 

	return err;
}

/*
 * host to card sync function
 */
static int if_sdio_host_to_card(struct if_sdio_card *card, void *data, int len, bool commit)
{
    int err = 0;
    u32 size1, size2, seg1, seg2;
    int cnt = 0;
    sdio_header *ptr;
    u32 transfer_len = 0;

    if (!card)
        return -EINVAL;

    sdio_claim_host(card->func);

    len = SDIO_SIZE_ALIGN(len, card);

retry:

    if (test_bit(SDIO_REMOVE, &card->flag)){
        sdio_release_host(card->func);
        return -1;
    }

    if (!tx_buffer_is_available(card, len)) {
		 if (commit) {   
				if (cnt > 0) {
				    pr_err("%s() buffer not enough! wait retry cnt %d (reqest len 0x%x)\n", __func__, cnt, len);
				    tx_buffer_dump();
				}
				// refresh buffer info 
				get_tx_reader_ptr(card);
				cnt++;
				if (cnt < 10) {
				    if (cnt > 2) {
				        // first retry need not wait 
				        mdelay(cnt*10);
				    }
				    goto retry;
				}
		}
        sdio_release_host(card->func);
        return -1;
    }

    ptr = (sdio_header*)data;
    pr_debug("%s() pkt seq_no : 0x%x\n", __func__, ptr->seq_no);
    if ((tx_buf_ptr + len) > tx_buf_end) {
        seg1 = tx_buf_ptr;
        size1 = tx_buf_end - tx_buf_ptr;
        seg2 = tx_buf_start;
        size2 = (tx_buf_ptr + len) - tx_buf_end;

        pr_debug("%s() transfer data (seg1)! tx_buf_ptr 0x%x transfer_len 0x%x\n", __func__, seg1, size1);
        err = sdio_transfer(card, data, seg1 - sdio_dma_base_addr, size1, 0);
        if (!err){
            pr_debug("%s() transfer data (seg2)! tx_buf_ptr 0x%x transfer_len 0x%x\n", __func__, seg2, size2);
            err = sdio_transfer(card, (data + size1), seg2 - sdio_dma_base_addr, size2, 0);
            if (!err) {
                transfer_len = size1 + size2;
                pkt_cnt++;
            }
        }
    } else {
        pr_debug("%s() transfer data! tx_buf_ptr 0x%x transfer_len 0x%x\n", __func__, tx_buf_ptr, len);
        err = sdio_transfer(card, data, tx_buf_ptr- sdio_dma_base_addr, len, 0);
        if (!err){
            transfer_len = len;
            pkt_cnt++;
        }
    }

    /* update local pointer for TX buffer producer */
    if (!err){
      tx_buffer_update(card, transfer_len);
    }

    if (commit) {        
        /* sync TX buffer producer to lynx */
        set_tx_producer_ptr(card, tx_buf_ptr);
        tx_commit(card);
    }
    sdio_release_host(card->func);
    return err;
}

/*
 * host/card async function
 */
static int sdio_bulk_msg(struct if_sdio_card *card, int dir,
         void *data, int len, int *actual_length, int timeout)
{
    struct sdio_func *func = card->func;
    struct lynx_hif_device *hdev = sdio_get_drvdata(func);
    int err = -EAGAIN, rc = 0, i;
    sdio_header header;
    u8 *ptr;
    struct sk_buff *skb;
    int retry = 5;

    if (hdev->flags & HIF_SDIO_TX_STOP)
        return 0;

    if (dir == SDIO_DIR_OUT) {
        pr_err("%s() SDIO_DIR_OUT is not supported\n", __func__);
        return -1;
    }

    if (dir == SDIO_DIR_OUT_SYNC) {
        /* prepare SDIO transfer header except the seq_no */
        header.payload_len = cpu_to_be16(len + SDIO_HDR_SIZE);
        header.res = cpu_to_be16(SDIO_SIZE_ALIGN(be16_to_cpu(header.payload_len), card) - be16_to_cpu(header.payload_len));
        header.id = SDIO_CMD_DATA;
        if (card->info.sdio_cksum_enable) {
            header.cksum = 0;
            for (i = 0, ptr = (u8*)data; i < len; i++) {
                header.cksum+= ptr[i];
            }
            pr_debug("%s() pkt: payload_len 0x%x\n", __func__,
                    be16_to_cpu(header.payload_len));
            pr_debug("%s() pkt: cksum 0x%x\n", __func__, header.id);
        }

        //skb = dev_alloc_skb(MAX_RX_BUF_SIZE);
        skb = dev_alloc_skb(SDIO_SIZE_ALIGN(be16_to_cpu(header.payload_len), card));
        if (!skb) {
            pr_err("%s() unable to allocate skb\n", __func__);
            return -1;
        } else {
             pr_debug("%s() allocate : head room %d, tail room %d\n", __func__,
                    skb_headroom(skb),
                    skb_tailroom(skb));
        }

        pr_debug("%s() pkt: tx_seq_no 0x%x\n", __func__, tx_seq_no);
        header.seq_no= tx_seq_no;
        tx_seq_no++;
        memcpy(skb_put(skb, SDIO_HDR_SIZE), &header, SDIO_HDR_SIZE);
        memcpy(skb_put(skb, len), data, len);

        pr_debug("%s() after cp data : head room %d, tail room %d\n", __func__,
                skb_headroom(skb),
                skb_tailroom(skb));

        //pr_err("%s() send firmware_version_get or send firmware\n", __func__);
        err = if_sdio_host_to_card(card, skb->data, skb->len, true);
        dev_kfree_skb(skb);
    }
    else if (dir == SDIO_DIR_IN) {
        set_bit(SDIO_RX_WAIT, &card->flag);

        /* poll rx becasue we don't have RX ISR notification so far */
        while (if_sdio_card_to_host(card,NULL,NULL,NULL) && retry)
            retry--;

        rc = wait_event_interruptible_timeout(card->rx_wq, !test_bit(SDIO_RX_WAIT, &card->flag), msecs_to_jiffies(timeout));
        if (rc == 0) {
            err = -ETIMEDOUT;
            pr_err("%s() %d err = %d\n", __func__, __LINE__, err);
            clear_bit(SDIO_RX_WAIT, &card->flag);
        }
        else if (rc < 0) {
            err = rc;
            pr_err("%s() %d err = %d\n", __func__, __LINE__, err);
            clear_bit(SDIO_RX_WAIT, &card->flag);
        }
        else {
            if (card->rx_buflen != len) {
                pr_err("%s() received length %d NOT equals request length %d \n", __func__,card->rx_buflen , len);
                memcpy(data, card->rx_buffer, card->rx_buflen);
                err = -ENOBUFS;
            }
            else {
                memcpy(data, card->rx_buffer, len);
                err = 0;
            }

            clear_bit(SDIO_RX_WAIT, &card->flag);
        }
    }
    else {
        err = -EINVAL;
    }

    if (actual_length && (!err))
        *actual_length = card->rx_buflen;

    return err;
}

/******************* hif function ***********************/
static int hif_sdio_init(void *hif)
{
    struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif;
    struct if_sdio_card *card = hdev_priv(hdev);
    struct sdio_func *func = card->func;
    int ret;

    ret = lynx_hw_init(hdev);
    if (ret) {
        dev_err(&func->dev, "%s: Unable to match lynx hw\n", __func__);
        return ret;
    }

    /* init member of hdev */
    spin_lock_init(&hdev->rx_lock);
    spin_lock_init(&hdev->tx_lock); //wmm queue
    spin_lock_init(&hdev->tx_sdio_lock); //sdio tx_skb_queue
    spin_lock_init(&hdev->tx_recycle_lock);
    spin_lock_init(&hdev->sender_lock);
    spin_lock_init(&hdev->rx_compress_lock);
    mutex_init(&hdev->tx_mutex);
    __skb_queue_head_init(&hdev->rx_skb_queue);
    __skb_queue_head_init(&hdev->tx_skb_queue);
    __skb_queue_head_init(&hdev->sender_queue);
    __skb_queue_head_init(&hdev->tx_recycle_queue);
    __skb_queue_head_init(&hdev->rx_compress_queue);

    hdev->init = 1;

    return 0;
}

static void hif_sdio_deinit(void *hif)
{
    struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif;

    if (!hdev->init)
        return;

    hdev->init = 0;
}

static void hif_sdio_start(void *hif)
{
    struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif;
    unsigned long flags;

    spin_lock_irqsave(&hdev->tx_lock, flags);
    hdev->flags &= ~HIF_SDIO_TX_STOP;
    spin_unlock_irqrestore(&hdev->tx_lock, flags);
}

static void hif_sdio_stop(void *hif)
{
    struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif;
    unsigned long flags;

    if (!hdev->init)
        return;

    spin_lock_irqsave(&hdev->tx_lock, flags);
    hdev->flags |= HIF_SDIO_TX_STOP;
    spin_unlock_irqrestore(&hdev->tx_lock, flags);
}

static int hif_sdio_send(void *hif, struct sk_buff *skb)
{
    struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif;
    struct if_sdio_card *card = hdev_priv(hdev);
    unsigned long flags;
    int err = 0;
    int i;
#ifndef CPU_AGGREGATION_ENABLE
    int pad = 0;
    struct sk_buff *tmp = NULL;
    //u8 need_bounce;
#endif
    sdio_header* hdr;
    u8 *ptr;
    u32 queue_cnt = 0;
    u8 offset = 0;
    bool not_aligned = false, invalid = false, pad_insufficient = false;
	//pr_info("%s() %d\n", __func__, __LINE__);

    if (debug_on) {
        pr_info("%s() %d\n", __func__, __LINE__);
        mdelay(500);
    }
    spin_lock_irqsave(&hdev->tx_lock, flags);

    if (hdev->flags & HIF_SDIO_TX_STOP) {
        spin_unlock_irqrestore(&hdev->tx_lock, flags);
        lynx_dbg(LYNX_DBG_SDIO | LYNX_DBG_ERR, "%s:%d Error(NODEV)\n", __func__, __LINE__);
        return -ENODEV;
    }

    //lynx_dbg(LYNX_DBG_SDIO, "%s:%d tid=%d\n", __func__, __LINE__, SKB_TID(hdev, skb));
    lynx_dbg_dump(LYNX_DBG_SDIO, "hif_sdio_send", "tx ", skb->data, skb->len);

    spin_unlock_irqrestore(&hdev->tx_lock, flags);

    if (!skb) {
        pr_err("%s() %d null skb?\n", __func__, __LINE__);
        return -1;
    }

    /* won't queue skb if tx list exceed the limit */
    spin_lock_irqsave(&hdev->tx_sdio_lock, flags);
    queue_cnt = skb_queue_len(&hdev->tx_skb_queue);
    spin_unlock_irqrestore(&hdev->tx_sdio_lock, flags);

    if (queue_cnt >= MAX_TX_QUEUE_NUM) {
        if (debug_on) {
            pr_info("%s() can't add item to tx_queue anymore\n", __func__);
            mdelay(500);
        }
#ifdef USE_WMM_QUEUE
        err = -ENOMEM; //-ENOMEM;
#else
        err = -1;
#endif
    } else {

        if (debug_on) {
            pr_info("%s() %d going to queue item\n", __func__, __LINE__);
            mdelay(500);
        }

        if (skb_headroom(skb) < (SDIO_HDR_SIZE + DMA_ADDRESS_ALIGNMENT)) {
            pr_err("%s() profiling: do cpu copy becasue not enough headroom (size %d)\n",
                __func__, skb->len);
            skb = skb_realloc_headroom(skb, SDIO_HDR_SIZE + DMA_ADDRESS_ALIGNMENT);
            if (!skb) {
                pr_err("%s() %d failed to do skb_realloc_headroom()\n", __func__, __LINE__);
                return -1;
            }
        }
#if 0
        /*
         * use offset make HC' DMA able to read 4byte aligned memory but make
         * lynx suffer when remove sdio header and offset
         */
        if ((u32)skb->data & (DMA_ADDRESS_ALIGNMENT-1)) {
            offset = DMA_ADDRESS_ALIGNMENT - ((u32)skb->data & (DMA_ADDRESS_ALIGNMENT-1));
            skb_push(skb, offset);
        }
#else
        offset = 0;
        skb_push(skb, offset);
#endif

        skb_push(skb, SDIO_HDR_SIZE);

        if ((unsigned long)(skb->data) & (DMA_ADDRESS_ALIGNMENT-1)) {
            pr_debug("%s() addr no aligned (skb->len %d)\n", __func__, skb->len);
            not_aligned = true;
        }
        if (!virt_addr_valid(skb->data)) {
            pr_err("%s() virtual addr invalid (skb->len %d)\n", __func__, skb->len);
            invalid = true;
        }
        if (skb_tailroom(skb) < SDIO_SIZE_ALIGN(skb->len, card) - skb->len) {
            pr_debug("%s() tail room %d is not enough (skb->len %d)\n",
                __func__, skb_tailroom(skb), skb->len);
            pad_insufficient = true;
        }

#ifndef CPU_AGGREGATION_ENABLE
        if (not_aligned || invalid || pad_insufficient) {
            tmp = dev_alloc_skb(0 + skb->len + pad);
            if (!tmp) {
                pr_err("%s() failed to allocated tmp\n", __func__);
                return -1;
            }
            
            /* data copy */
            skb_put(tmp, skb->len);
            memcpy(tmp->data, skb->data, skb->len);
            dev_kfree_skb(skb);
            skb = tmp;
        }
#endif

        /* prepare header */
        hdr = (sdio_header*)skb->data;
        hdr->id = SDIO_CMD_DATA;
        hdr->offset = offset;
        hdr->payload_len = cpu_to_be16(skb->len);
        hdr->res = cpu_to_be16(0); /* should be modified later in tx work queue*/
        if (card->info.sdio_cksum_enable) {
            hdr->cksum = 0;
            for (i = 0, ptr = ((u8*)skb->data + SDIO_HDR_SIZE + offset); i < (skb->len - SDIO_HDR_SIZE -offset); i++) {
                hdr->cksum+= ptr[i];
            }
            //pr_err("%s() pkt: payload_len 0x%x\n", __func__,
                   // be16_to_cpu(hdr->payload_len));
            //pr_err("%s() pkt: cksum 0x%x\n", __func__, hdr->id);
            //sdio_hexdump(((u8*)skb->data + SDIO_HDR_SIZE + offset), (skb->len - SDIO_HDR_SIZE -offset));
        }

        /* queue skb for tx transfering worker */
        spin_lock_irqsave(&hdev->tx_sdio_lock, flags);
        ((sdio_header*)(skb->data))->seq_no = tx_seq_no;
        tx_seq_no++;
        __skb_queue_tail(&hdev->tx_skb_queue, skb);
        spin_unlock_irqrestore(&hdev->tx_sdio_lock, flags);

        err = 0;
#ifndef CPU_AGGREGATION_ENABLE
        queue_work(card->tx_workqueue, &card->tx_worker);
#else
        if (!tx_buffer_list_full(&card->tx_buffer_list))
            queue_work(card->cpu_aggregation_workqueue, &card->cpu_aggregation_worker);
#endif
    }

    return err;
}

static int hif_sdio_recv(void *hif, struct sk_buff *skb)
{
    struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif;
    struct if_sdio_card *card = hdev_priv(hdev);

    if (test_bit(SDIO_ACTIVE, &card->flag)) {
        lynx_rx_tasklet((unsigned long)hdev);
    }
    else

    {
        struct sk_buff *skb = NULL;
        unsigned long flags;
        while (1) {
            spin_lock_irqsave(&hdev->rx_lock, flags);
            skb = __skb_dequeue(&hdev->rx_skb_queue);
            spin_unlock_irqrestore(&hdev->rx_lock, flags);
            if(skb == NULL)
                break;
            kfree_skb(skb);
        }

        {
            struct sdio_func *func = card->func;
            dev_err(&func->dev, "%s:%d sdio recv packet, but not active!!\n", __func__, __LINE__);
        }

    }

    return 0;
}

static int hif_sdio_wmm_lock(void *hif, unsigned long *flags)
{
	struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif;
#if 1
	if(flags)
		spin_lock_irqsave(&hdev->tx_lock, *flags);
	else
		spin_lock(&hdev->tx_lock);
#else
    mutex_lock(&hdev->tx_mutex);
#endif
    return 0;
}

static int hif_sdio_wmm_unlock(void *hif, unsigned long *flags)
{
	struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif;
#if 1
	if(flags)
		spin_unlock_irqrestore(&hdev->tx_lock, *flags);
	else
		spin_unlock(&hdev->tx_lock);
#else
    mutex_unlock(&hdev->tx_mutex);
#endif
	return 0;
}

static struct lynx_hif_ops hif_sdio = {
    .name   = "lynx_hif_sdio",

    .init   = hif_sdio_init,
    .deinit = hif_sdio_deinit,
    .start  = hif_sdio_start,
    .stop   = hif_sdio_stop,
    .send   = hif_sdio_send,
    .recv   = hif_sdio_recv,
    .tx_lock   = hif_sdio_wmm_lock,
    .tx_unlock   = hif_sdio_wmm_unlock,

};
/******************* hif function ***********************/

/********************************************************************/
/* Firmware                                                         */
/********************************************************************/
static int if_sdio_is_boot_rom(struct if_sdio_card *card)
{
    struct sdio_func_tuple *tuple = card->func->tuples;
    int ret = 0;
    while (tuple) {
        if (((tuple->code == SDIO_TUPLE_BOOT_ROM2) ||
           (tuple->code == SDIO_TUPLE_BOOT_ROM3) )
            && (tuple->size >= 4)) {
            pr_info("SDIO boot from [%s]!\n", tuple->data);
            ret = tuple->code;
        }
        tuple = tuple->next;
    }
    return ret;
}

static int if_sdio_get_fw_info(struct if_sdio_card *card)
{
    struct sdio_fw_info_tuple fw_tuple;
    struct sdio_fw_info_tuple *tuple = &fw_tuple;
    struct sdio_fw_info *info = &tuple->info;
    unsigned i, ptr = 0;
    u8 *data = (u8 *)tuple;
    int err = -EINVAL;
    u32 tmp;

    sdio_dma_base_addr = 0;

    sdio_claim_host(card->func);

    for (i = 0; i < 3; i++) {
        unsigned char x, fn = card->func->num;
        x = sdio_f0_readb(card->func, SDIO_FBR_BASE(fn) + SDIO_FBR_DMABASE + i, &err);
        if (err) {
            pr_err("%s:%d Read SDIO_FBR_DMABASE failed!\n", __func__, __LINE__);
            sdio_release_host(card->func);
            return err;
        }
        sdio_dma_base_addr |= x << (i * 8);
    }
    pr_info("%s: dma_base is 0x%x\n", __func__, sdio_dma_base_addr);

    for (i = 0; i < 3; i++) {
        unsigned char x, fn = card->func->num;
        x = sdio_f0_readb(card->func, SDIO_FBR_BASE(fn) + SDIO_FBR_CIS + i, &err);
        if (err) {
            pr_err("%s:%d Read SDIO_FBR_CIS failed!\n", __func__, __LINE__);
            sdio_release_host(card->func);
            return err;
        }
        ptr |= x << (i * 8);
    }
    tx_ptr = ptr + 14;
    for (i = 0; i < sizeof(struct sdio_fw_info_tuple); i++, data++) {
        *data = sdio_f0_readb(card->func, ptr++, &err);
        if (err) {
            pr_err("%s:%d Read Func tuple failed!\n", __func__, __LINE__);
            sdio_release_host(card->func);
            return err;
        }
    }


    if ((tuple->code == SDIO_TUPLE_FIRMWARE_INFO) && (tuple->size >= sizeof(struct sdio_fw_info))) {
        pr_info("Firmware Information:\n");
        pr_info("datablock_start = 0x%x\n", le32_to_cpu(info->datablock_start));
        pr_info("datablock_size  = 0x%x\n", le32_to_cpu(info->datablock_size));
        pr_info("datablock_num   = 0x%x\n", le32_to_cpu(info->datablock_num));
        pr_info("tx ring ptr   = 0x%x\n", le32_to_cpu(info->tx_ring_ptr));
        tx_buf_start = le32_to_cpu(info->tx_ring_ptr);
        tx_buf_ptr = tx_buf_start;
        tx_buf_reader_ptr = tx_buf_start;
        pr_info("tx buf size   = 0x%x\n", le32_to_cpu(info->tx_buf_size));
        tx_buf_size = le32_to_cpu(info->tx_buf_size);
        tx_buf_end = tx_buf_start + tx_buf_size;
        pr_info("tx full flag   = 0x%x\n", le32_to_cpu(info->tx_full_flag));
        pr_info("tx tx_producer   = 0x%x\n", info->tx_producer);
        pr_info("tx tx_consumer   = 0x%x\n", info->tx_consumer);
        pr_info("rx ring ptr (rx_buf_start) = 0x%x\n", le32_to_cpu(info->rx_buf_start));
        rx_buf_start = le32_to_cpu(info->rx_buf_start);
        rx_buf_write_ptr = rx_buf_start;
        rx_buf_reader_ptr = rx_buf_start;
        pr_info("rx_buf_write_ptr   = 0x%x\n", rx_buf_write_ptr);
        pr_info("rx_buf_reader_ptr  = 0x%x\n", rx_buf_reader_ptr);
        pr_info("rx buf size   = 0x%x\n", le32_to_cpu(info->rx_buf_size));
        rx_buf_size = le32_to_cpu(info->rx_buf_size);
        rx_buf_end = rx_buf_start + rx_buf_size;
        pr_info("rx_buf_end  = 0x%x\n", rx_buf_end);
        pr_info("rx full flag   = 0x%x\n", le32_to_cpu(info->rx_full_flag));
        pr_info("rx rx_producer   = 0x%x\n", info->rx_producer);
        pr_info("rx rx_consumer   = 0x%x\n", info->rx_consumer);
        pr_info("align size   = 0x%x\n", le32_to_cpu(info->size_align));
        pr_info("sdio_cksum_enable   = 0x%x\n", info->sdio_cksum_enable);
        pr_info("fast_tx_commit_enable   = 0x%x\n", info->fast_tx_commit_enable);
        pr_info("fast_rx_commit_enable   = 0x%x\n", info->fast_rx_commit_enable);

        if (le32_to_cpu(info->size_align)) {
                //sdio_claim_host(card->func);
                sdio_set_block_size(card->func, le32_to_cpu(info->size_align));
                //sdio_release_host(card->func);
        }

        if (le32_to_cpu(info->size_align)) {
            tmp = info->size_align;
            for (i = 0 ; ; i++) {
                if (tmp >> i == 0x1)
                    break;
            }
            tx_size_align_sh = i;
        }

        memcpy(&card->info, info, sizeof(struct sdio_fw_info));
/*
Firmware Information:
datablock_start = 0x10000
datablock_size  = 0x200
datablock_num   = 0x80
*/

        card->db_start  = le32_to_cpu(card->info.datablock_start);
        //card->db_size   = le32_to_cpu(card->info.datablock_size);
        if (le32_to_cpu(info->size_align)) {
            card->db_size = le32_to_cpu(info->size_align);
        }
        card->db_num    = le32_to_cpu(card->info.datablock_num);

    }
    else {
        pr_err("%s:%d No tuple: SDIO_TUPLE_FIRMWARE_INFO!\n", __func__, __LINE__);
        err = -1;
    }

    sdio_release_host(card->func);
    return err;
}

static int if_sdio_get_fw_verion(struct if_sdio_card *card)
{
    struct sdio_func *func = card->func;
    struct lynx_hif_device *hdev = sdio_get_drvdata(func);
    struct lynx_wci_hdr lynx_cmd = {
         .cmd_id = WCI_H2D_FW_INFO, .seq_no = 0, .payload_len = 0
    };
    int actual = 0;
    int transfer = 0, err = 0;
    u8 *buf = os_api_alloc(512, GFP_KERNEL);

    if (!buf) {
        err = -ENOMEM;
        goto err_fw;
    }

    memcpy(buf, &lynx_cmd, sizeof(struct lynx_wci_hdr));
    transfer = sizeof(struct lynx_wci_hdr);
    err = sdio_bulk_msg(card, SDIO_DIR_OUT_SYNC, buf, transfer, 0, 1000);
    if (err) {
        dev_err(&func->dev, "%s:%d failed to send cmd : WCI_H2D_FW_INFO!\n", __func__, __LINE__);
        err = -EIO;
        goto err_fw;
    }

    err = sdio_bulk_msg(card, SDIO_DIR_IN, buf, transfer+ROM_VER_LENGTH, &actual, 1000);
    if (err < 0 || actual == 0) {
        dev_err(&func->dev, "%s:%d failed to get lynx ROM version!\n", __func__, __LINE__);
        err = -EIO;
        goto err_fw;
    }

    if (actual != sizeof(struct lynx_wci_hdr) + ROM_VER_LENGTH) {
        dev_err(&func->dev, "%s:%d ROM version length not correct!\n", __func__, __LINE__);
        err = -EIO;
        goto err_fw;
    } 

    hdev->fw_version = be32_to_cpu(*(u32 *)(buf+sizeof(struct lynx_wci_hdr)));
    kfree(buf);

    dev_info(&func->dev, "%s() Lynx ROM version: 0x%x\n", __func__, hdev->fw_version);

    return 0;

err_fw:
    if (buf)
        kfree(buf);
    dev_err(&func->dev, "%s:%d Can't get lynx ROM version!\n", __func__, __LINE__);
    dev_err(&func->dev, "Please power reset module!\n");
    return err;
}

static int lynx_sdio_download_fw(struct lynx_hif_device *hdev)
{
    struct if_sdio_card *card = hdev_priv(hdev);
    struct sdio_func *func = card->func;
    struct lynx_wci_hdr lynx_cmd = {
         .cmd_id = WCI_H2D_FW_DOWNLOAD, .seq_no = 0, .payload_len = 0
    };
    int transfer = 0, err = 0;
    u8 *buf;
    const void *data ;
    size_t len ;

#ifndef CONFIG_LYNX_W2SDIO_FIRMWARE_ARRAY
	data = hdev->firmware->data;
	len = hdev->firmware->size;
#else

#ifdef CONFIG_LYNX_ROM3
	if(hdev->fw_version == 0x03){
		data = lynx_static_firmware_3;
		len = FIRMWARE_ARRAY_SIZE_3;
	}
#endif /*CONFIG_LYNX_ROM3*/


#ifdef CONFIG_LYNX_ROM2
	if(hdev->fw_version == 0x02){
		data = lynx_static_firmware_2;
		len = FIRMWARE_ARRAY_SIZE_2;
	}
#endif /*CONFIG_LYNX_ROM2*/

	
	if(mp_test_firm)
	{
#ifdef CONFIG_MP_FW

#ifdef CONFIG_LYNX_ROM3
	if(hdev->fw_version == 0x03){
		data = lynx_static_mp_firmware_3;
		len = MP_FIRMWARE_ARRAY_SIZE_3;
	}
#endif /*CONFIG_LYNX_ROM3*/

#ifdef CONFIG_LYNX_ROM2
	if(hdev->fw_version == 0x02){
		data = lynx_static_mp_firmware_2;
		len = MP_FIRMWARE_ARRAY_SIZE_2;
	}
#endif /*CONFIG_LYNX_ROM2*/

#else
        err = -EIO;
		return err;
#endif /*CONFIG_MP_FW*/

	}
#endif /*CONFIG_LYNX_W2SDIO_FIRMWARE_ARRAY*/

    buf = os_api_alloc(512, GFP_KERNEL);
    if (!buf) {
        err = -ENOMEM;
        goto err_fw;
    }

    lynx_cmd.payload_len = cpu_to_be16((len / 512) + ((len % 512) ? 1 : 0));

    memcpy(buf, &lynx_cmd, sizeof(struct lynx_wci_hdr));
    transfer = sizeof(struct lynx_wci_hdr);
    err = sdio_bulk_msg(card, SDIO_DIR_OUT_SYNC, buf, transfer, 0, 3000);
    if (err) {
        err = -EIO;
        goto err_fw;
    }

    while (len) {
        transfer = min_t(size_t, len, 512);
        memcpy(buf, data, transfer);
        err = sdio_bulk_msg(card, SDIO_DIR_OUT_SYNC, buf, transfer, 0, 3000);
        if (err < 0) {
            goto err_fw;
        }

        len -= transfer;
        data += transfer;
    }

    /*
     * Issue FW download complete command to firmware.
     */
    lynx_cmd.cmd_id = WCI_H2D_FW_COMP;
    memcpy(buf, &lynx_cmd, sizeof(struct lynx_wci_hdr));
    transfer = sizeof(struct lynx_wci_hdr);
    err = sdio_bulk_msg(card, SDIO_DIR_OUT_SYNC, buf, transfer, 0, 3000);
    if (err) {
        err = -EIO;
        goto err_fw;
    }

    kfree(buf);

#ifndef CONFIG_LYNX_W2SDIO_FIRMWARE_ARRAY
    dev_info(&func->dev, "lynx firmware download: Transferred FW: %s, size: %ld\n",
         hdev->fw_name, (unsigned long) hdev->firmware->size);
#else
	if(mp_test_firm){
#ifdef CONFIG_MP_FW
#ifdef CONFIG_LYNX_ROM2
		if(hdev->fw_version == 0x02){
			dev_info(&func->dev, "7000s mp firmware download: Transferred FW: %s, rev: %d, size: %ld\n",
				"static array", LYNX_MP_FW_REV_2, (unsigned long)MP_FIRMWARE_ARRAY_SIZE_2);
		}
#endif /*CONFIG_LYNX_ROM2*/
#ifdef CONFIG_LYNX_ROM3
		if(hdev->fw_version == 0x03){
			dev_info(&func->dev, "6700s mp firmware download: Transferred FW: %s, rev: %d, size: %ld\n",
				"static array", LYNX_MP_FW_REV_3, (unsigned long)MP_FIRMWARE_ARRAY_SIZE_3);
		}
#endif /*CONFIG_LYNX_ROM3*/
#endif /*CONFIG_MP_FW*/
	}else{

#ifdef CONFIG_LYNX_ROM2
		if(hdev->fw_version == 0x02){
			dev_info(&func->dev, "7000s firmware download: Transferred FW: %s, rev: %d, size: %ld\n",
				"static array", LYNX_FW_REV_2, (unsigned long)FIRMWARE_ARRAY_SIZE_2);
		}
#endif /*CONFIG_LYNX_ROM2*/
#ifdef CONFIG_LYNX_ROM3
			if(hdev->fw_version == 0x03){
				dev_info(&func->dev, "6700s firmware download: Transferred FW: %s, rev: %d, size: %ld\n",
					"static array", LYNX_FW_REV_3, (unsigned long)FIRMWARE_ARRAY_SIZE_3);
			}
#endif /*CONFIG_LYNX_ROM3*/

	}
#endif /*CONFIG_LYNX_W2SDIO_FIRMWARE_ARRAY*/

    /*
     * wait for firmware complete
     */

    pr_info("%s() %d Wait FW boot up 1 sec\n", __func__, __LINE__);
    msleep(1000);
    
    return 0;

err_fw:
    if (buf)
        kfree(buf);
    return err;
}

/* FIXME: update firmware feature */
#ifdef LYNX_WAIT_DOWNLOAD_FIRMWARE
static void lynx_sdio_firmware_fail(struct lynx_hif_device *hdev)
{
    struct if_sdio_card *card = hdev_priv(hdev);
    struct sdio_func *func = card->func;
    struct device *parent = usb->dev->parent;

    if (parent)
        device_lock(parent);

    device_release_driver(&func->dev);

    if (parent)
        device_unlock(parent);

    dev_err(&func->dev, "lynx: %s failed\n", __func__);
}

static void lynx_sdio_firmware_cb(const struct firmware *fw, void *context)
{
    struct lynx_hif_device *hdev = context;
    struct if_sdio_card *card = hdev_priv(hdev);
    struct sdio_func *func = card->func;
    int ret;

    if (!fw) {
        dev_err(&func->dev, "lynx: Failed to get firmware %s\n", hdev->fw_name);
        goto err_fw;
    }

    hdev->firmware = fw;
    ret = lynx_sdio_download_fw(hdev);
    if (ret) {
        dev_err(&func->dev, "lynx: Firmware - %s download failed\n", hdev->fw_name);
        goto err_fw;
    }

err_no:
    if (hdev->firmware)
        release_firmware(hdev->firmware);
    hdev->firmware = NULL;

    complete(&hdev->fw_done);

    return;

err_fw:
    lynx_sdio_firmware_fail(hdev);
    goto err_no;
}
#endif

static int lynx_sdio_switch_on(struct lynx_hif_device *hdev)
{
	int retry = 0;

	/* init hif device */
	if(hif_device_init(hdev)){
		printk(KERN_ERR "hw init failed at hif_device_init!\n");
		return -ENODEV;
	}
	
	for(retry = 0; retry < 3; retry++){
		if(hif_device_start(hdev)){
			printk(KERN_ERR "hw init failed at hif_device_start! retry %d\n",retry);
		}else{
			return 0;
		}
	}

	hif_device_deinit(hdev);
	return -ENODEV;
}

static int lynx_sdio_switch_off(struct lynx_hif_device *hdev)
{
    if (hdev) {
        hif_device_stop(hdev);
        hif_device_deinit(hdev);
    }
    return 0;
}

static int lynx_sdio_device_detached(struct lynx_hif_device *hdev)
{
    struct if_sdio_card *card = hdev_priv(hdev);
    struct sk_buff *skb;
    unsigned long flags;
    tx_buffer_entry* tmp;

    lynx_dbg(LYNX_DBG_WARN, "%s(): hdev=0x%x\n", __FUNCTION__, (uintptr_t)hdev);

    int_stop = 0;

    if (hdev) {
        set_bit(SDIO_REMOVE, &card->flag);
        clear_bit(SDIO_ACTIVE, &card->flag);

        if( card->rx_method == SDIO_RX_IB_IRQ_BURST){
          /*Push work queue to end*/
          set_bit(SDIO_IRQ, &card->flag);
          wake_up_interruptible(&card->rx_irq_wq);
         }

        lynx_sdio_switch_off(hdev);

        cancel_work_sync(&card->rx_worker);
        flush_workqueue(card->rx_workqueue);
        destroy_workqueue(card->rx_workqueue);

        cancel_work_sync(&card->tx_worker);
        flush_workqueue(card->tx_workqueue);
        destroy_workqueue(card->tx_workqueue);

        cancel_work_sync(&card->cpu_aggregation_worker);
        flush_workqueue(card->cpu_aggregation_workqueue);
        destroy_workqueue(card->cpu_aggregation_workqueue);

        cancel_work_sync(&card->tx_recycle_worker);
        flush_workqueue(card->tx_recycle_workqueue);
        destroy_workqueue(card->tx_recycle_workqueue);

        cancel_work_sync(&card->rx_decompress_worker);
        flush_workqueue(card->rx_decompress_workqueue);
        destroy_workqueue(card->rx_decompress_workqueue);

        if (hdev->lynx) {
            lynx_core_cleanup(hdev->lynx);
            wci_progress_check(1, 0);
            os_dep_deinit(hdev->lynx);
        }

        spin_lock_irqsave(&hdev->rx_lock, flags);
        while (1) {
            if (!(skb = __skb_dequeue(&hdev->rx_skb_queue)))
		    	break;
            kfree_skb(skb);
        }
        spin_unlock_irqrestore(&hdev->rx_lock, flags);
        spin_lock_irqsave(&hdev->tx_sdio_lock, flags);
        while (1) {
            if (!(skb = __skb_dequeue(&hdev->tx_skb_queue)))
		    	break;
            kfree_skb(skb);
        }
        spin_unlock_irqrestore(&hdev->tx_sdio_lock, flags);
        spin_lock_irqsave(&hdev->sender_lock, flags);
        while (1) {
            if (!(skb = __skb_dequeue(&hdev->sender_queue)))
		    	break;
            kfree_skb(skb);
        }
        spin_unlock_irqrestore(&hdev->sender_lock, flags);
        spin_lock_irqsave(&hdev->rx_compress_lock, flags);
        while (1) {
            if (!(skb = __skb_dequeue(&hdev->rx_compress_queue)))
		    	break;
            kfree_skb(skb);
        }
        spin_unlock_irqrestore(&hdev->rx_compress_lock, flags);
        spin_lock_irqsave(&hdev->tx_recycle_lock, flags);
        while (1) {
            if (!(skb = __skb_dequeue(&hdev->tx_recycle_queue)))
		    	break;
            kfree_skb(skb);
        }
        spin_unlock_irqrestore(&hdev->tx_recycle_lock, flags);

        while (!list_empty(&card->tx_buffer_list.buffer_list)) {
            tmp = tx_buffer_dequeue(&card->tx_buffer_list);
            tx_buffer_free(tmp);
        }
    }
    return 0;
}

static int if_sdio_prog_firmware(struct if_sdio_card *card)
{
    struct sdio_func *func = card->func;
    struct lynx_hif_device *hdev = sdio_get_drvdata(func);
    int ret = 0;

    if ((ret = if_sdio_get_fw_info(card)))
        return ret;

    card->rom_ver = if_sdio_is_boot_rom(card);
    if (card->rom_ver) {

        dev_info(&func->dev, "Lynx [boot mode] now attached\n");
        card->app_mode = 0;

        ret = if_sdio_get_fw_verion(card);
        if (ret)
            goto err_fw;

        init_completion(&hdev->fw_done);

        /* Assign which firmware to load */
        if(hdev->fw_version == 0x03)
            hdev->fw_name = FIRMWARE_LYNX_3_0_0;
        else if(hdev->fw_version == 0x02)
            hdev->fw_name = FIRMWARE_LYNX_2_0_0;
        else
            goto err_fw;

        /*
         * for some version linux kernel, compat_request_firmware_nowait will be used
         * need to add rules for udev like that:
         * add udev rule to handle uevent be sended by compat_request_firmware function
         * create file /etc/udev/rules.d/50-firmware.rules
         * file content:
         *     # firmware-class requests, copies files into the kernel
         *     SUBSYSTEM=="compat_firmware", ACTION=="add", RUN+="firmware --firmware=$env{FIRMWARE} --devpath=$env{DEVPATH}"
         * by terry
         */
#ifdef LYNX_WAIT_DOWNLOAD_FIRMWARE
        ret = request_firmware_nowait(THIS_MODULE, true, hdev->fw_name,
            &func->dev, GFP_KERNEL,
            hdev, lynx_sdio_firmware_cb);
        if (ret) {
            dev_err(&func->dev, "%s: Async request for firmware %s failed\n", __func__, hdev->fw_name);
            goto err_fw;
        }
        wait_for_completion(&hdev->fw_done);
        if (hdev->firmware) {
            release_firmware(hdev->firmware);
            hdev->firmware = NULL;
        }
#else
#ifndef CONFIG_LYNX_W2SDIO_FIRMWARE_ARRAY
        ret = request_firmware(&hdev->firmware, hdev->fw_name, &func->dev);
        if (ret) {
            dev_err(&func->dev, "%s: request for firmware %s failed\n", __func__, hdev->fw_name);
            goto err_fw;
        }
        ret = lynx_sdio_download_fw(hdev);
        if (ret) {
            dev_err(&func->dev, "%s: download for firmware %s failed\n", __func__, hdev->fw_name);
            //mdelay(5000);
            goto err_fw;
        }
        release_firmware(hdev->firmware);
        hdev->firmware = NULL;
#else
        ret = lynx_sdio_download_fw(hdev);
        if (ret) {
            dev_err(&func->dev, "%s: download for static firmware %s failed\n", __func__, hdev->fw_name);
            //mdelay(5000);
            goto err_fw;
        }
        hdev->firmware = NULL;
#endif
#endif

        dev_info(&func->dev, "%s: Firmware %s download OK\n", __func__, hdev->fw_name);
        dev_info(&func->dev, "%s: Re-initialize!!\n", __func__);
        msleep(1000);
        ret = if_sdio_get_fw_info(card);
        if (ret) {
            dev_err(&func->dev, "%s: Re-initialize for firmware %s failed\n", __func__, hdev->fw_name);
            goto err_fw;
        }

        dev_info(&func->dev, "%s: Firmware %s requested\n", __func__, hdev->fw_name);
        card->app_mode = 1;
    }
    else {
        ret = if_sdio_get_fw_info(card);
        if (ret) {
            dev_err(&func->dev, "%s: Re-initialize for firmware %s failed\n", __func__, hdev->fw_name);
            goto err_fw;
        }
        dev_info(&func->dev, "Lynx [app mode] now attached\n");
        card->app_mode = 1;
    }
    return 0;

err_fw:
    return ret;
}

/********************************************************************/
/* Power management                                                 */
/********************************************************************/
static int if_sdio_power_on(struct if_sdio_card *card)
{
    struct sdio_func *func = card->func;
    int ret = 0;

    sdio_claim_host(func);

    ret = sdio_enable_func(func);
    if (ret) {
        pr_err("%s() %d error\n", __func__, __LINE__);
        goto release;
    }

    sdio_set_block_size(card->func, IF_SDIO_BLOCK_SIZE);

    sdio_release_host(func);

    ret = if_sdio_prog_firmware(card);
    if (ret)
        goto release_func;

    return 0;

release_func:
    sdio_claim_host(func);
    sdio_disable_func(func);
release:
    sdio_release_host(func);
    return ret;
}

static int if_sdio_power_off(struct if_sdio_card *card)
{
	struct sdio_func *func = card->func;

	if (card->rx_method == SDIO_RX_OOB_IRQ_SYNC){
		pr_info("%s() %d going to release OOB irq\n", __func__, __LINE__);
		if_sdio_OOB_irq_unregister(card);
	}
	else if((card->rx_method == SDIO_RX_IB_IRQ_DIRECT)
		||(card->rx_method == SDIO_RX_IB_IRQ_BURST)
		||(card->rx_method == SDIO_RX_IB_IRQ_SYNC)
		){
		/* Unregister SDIO In-Band IRQ handler */
		pr_info("%s() %d going to release IB irq\n", __func__, __LINE__);
		sdio_IB_irq_deregister(card);
	}

	sdio_claim_host(func);
	sdio_disable_func(func);
	sdio_release_host(func);

	return 0;
}

/*******************************************************************/
/* SDIO callbacks                                                  */
/*******************************************************************/

static void sdio_FW_irq_enable(struct if_sdio_card *card, int enable) 
{
	int err = 0;
	int retry = 0;

	//boot mode not support
	if( 0 == card->app_mode)
		return;

	sdio_claim_host(card->func);
	
RETRY:
	if (test_bit(SDIO_REMOVE, &card->flag)){
		sdio_release_host(card->func);
		return;
	}
	
	//if (card->fw_irq_enabled != enable) {
	if (1) {
		if (true == enable){
			sdio_f0_writeb(card->func, (unsigned char)0x03, SDIO_CCCR_IENx, &err);
		}else{
			sdio_f0_writeb(card->func, (unsigned char)0x00, SDIO_CCCR_IENx, &err);
		}
		//card->fw_irq_enabled = enable;
	}else{
		sdio_release_host(card->func);
		return;
	}

	if (err) {
		if (retry++ < RETRY_THRESHOLD) {
			pr_err("%s:%d error retry (err:%d)\n", __func__, __LINE__, err);
			pr_err("delay %d ms\n", retry*RETRY_DELAY_MS);
			mdelay(retry*RETRY_DELAY_MS);
			goto RETRY;
		}
		pr_err("%s:%d ERROR (err:%d)\n", __func__, __LINE__, err);
	}

	sdio_release_host(card->func);
	return;
}


static void if_sdio_IB_irq_handler_wrap(struct sdio_func *func)
{
    struct lynx_hif_device *hdev = sdio_get_drvdata(func);
    struct if_sdio_card *card;
    if (!hdev)
        return;

    card = hdev_priv(hdev);
    if (!card)
        return;

    if(NULL != card->ib_irq_handle){
       card->ib_irq_handle(func);
    }

    return;
}


static int irq_cnt=0;
static int heavyload_cnt=0;
static int heavyload_ratio=0;

/* SDIO In-Band IRQ handler */
/*Get data from SDIO interface to rx_compress_queue*/
static void if_sdio_IB_irq_handler(struct sdio_func *func)
{
	struct lynx_hif_device *hdev = sdio_get_drvdata(func);
	struct if_sdio_card *card= hdev_priv(hdev);

	bool exist = false; /*not set busyloop*/
	int retry = 3;
	bool load = false;

	if (test_bit(SDIO_REMOVE, &card->flag)){
		pr_info("%s() sdio remove\n", __func__);
		return ;
	}

	if(card->rx_method == SDIO_RX_IB_IRQ_BURST){
		if(test_bit(SDIO_BURST, &card->flag)){
			irq_cnt=0;
			heavyload_cnt=0;
			heavyload_ratio=0;
			//printk(KERN_ERR "\nC\n");
			return ;
		}
	}

	//Disable IB irq get from Host SDIO and sent by FW
	if_sdio_IB_irq_enable(card,false);

	if (card->rx_method == SDIO_RX_IB_IRQ_BURST){
		irq_cnt++;
		exist = false; /* Not set busyloop*/
		retry = 3;
		while( (retry) && (if_sdio_card_to_host(card,&exist,&load,NULL) )) {
			retry--;
			if (test_bit(SDIO_REMOVE, &card->flag)) 
				return;
			else if (retry > 0)
				mdelay(retry*RETRY_DELAY_MS);
		}


#ifdef CPU_AGGREGATION_ENABLE
		/* Cause already get tx consumer ptr*/
		if( tx_buffer_len_remain(card, NULL)&&
			(tx_buffer_list_count(&card->tx_buffer_list))){
			queue_work(card->tx_workqueue, &card->tx_worker);
		}
#endif
		

		if(true == load)
			heavyload_cnt++;

		if(irq_cnt >= 200){ 
				heavyload_ratio = (heavyload_cnt*100)/irq_cnt;
				irq_cnt=0;
				heavyload_cnt=0;
				//pr_info("H%d\n", heavyload_ratio);
		}

		/*idle:0,connect:2~3 , ping 5,32k:2,64k:11,128k:14~15,
				512K:15~16,1M:16 ,2M:16~17 ,4M:17~18 */

		if (heavyload_ratio>=17){ /*more than 2M ,or more than 512K when cpu busy*/
			//printk(KERN_ERR "\nA%d\n", heavyload_ratio);
			heavyload_ratio = 0;
			if_sdio_IB_irq_enable(card,true);
			set_bit(SDIO_IRQ, &card->flag);
			wake_up_interruptible(&card->rx_irq_wq);

			return;
		}else{
			if_sdio_IB_irq_enable(card,true);
			return;
		}
	}else if (card->rx_method == SDIO_RX_IB_IRQ_DIRECT) {
		//sdio_FW_irq_enable(card, false) ;
		exist = true; /* set busyloop*/
		retry = 3;
		while( (retry) && (if_sdio_card_to_host(card,&exist,NULL,NULL) )) {
			retry--;
			if (test_bit(SDIO_REMOVE, &card->flag)) 
				return;
			else if (retry > 0)
				mdelay(retry*RETRY_DELAY_MS);
		}

#ifdef CPU_AGGREGATION_ENABLE
		/* Cause already get tx consumer ptr*/
		if( tx_buffer_len_remain(card, NULL)&&
			(tx_buffer_list_count(&card->tx_buffer_list))){
			queue_work(card->tx_workqueue, &card->tx_worker);
		}
#endif
		
		
		//sdio_FW_irq_enable(card, true) ;
		if_sdio_IB_irq_enable(card,true);
	}else if (card->rx_method == SDIO_RX_IB_IRQ_SYNC) {
		queue_work(card->rx_workqueue, &card->rx_worker);
	}
}

static void if_sdio_IB_irq_enable(struct if_sdio_card *card,bool enable)
{
	unsigned long flags;
	if (!card){
		pr_err("%s: card is NULL\n", __FUNCTION__);
		return;
	}
	
	spin_lock_irqsave(&card->irq_spinlock, flags);
	if (card->ib_irq_enabled != enable) {
		if (true == enable){
			card->ib_irq_handle = if_sdio_IB_irq_handler;
		}else{
			card->ib_irq_handle = NULL;
		}
		card->ib_irq_enabled = enable;
	}
	spin_unlock_irqrestore(&card->irq_spinlock, flags);
	return;
}

/* Register SDIO In-Band IRQ handler */
int sdio_IB_irq_register(struct if_sdio_card *card, sdio_irq_handler_t *handler) {
    int err = 0;
    int retry = 0;

    if (card->ib_irq_registered) {
       pr_err("%s: irq is already registered\n", __FUNCTION__);
       return -EBUSY;
    }

RETRY:
    if (test_bit(SDIO_REMOVE, &card->flag)) 
        return -1;

     card->ib_irq_handle = NULL;
     card->ib_irq_enabled = false;

    sdio_claim_host(card->func);
    err = sdio_claim_irq(card->func, handler);
    sdio_release_host(card->func);
    if (err) {
        if (retry++ < (RETRY_THRESHOLD*2)) {
            //pr_err("%s:%d error retry (err:%d)\n", __func__, __LINE__, err);
            //pr_err("delay %d ms\n", retry*(RETRY_DELAY_MS*2));
            mdelay(retry*(RETRY_DELAY_MS*2));
            goto RETRY;
        }
        pr_err("%s:%d ERROR (err:%d)\n", __func__, __LINE__, err);
    } else{
        card->ib_irq_registered = true;
        card->ib_irq_enabled = false;
        //card->fw_irq_enabled = true;

        //pr_err("%s:%d successfull (err:%d)\n", __func__, __LINE__, err);
    }

    return err;
}

/* Unregister SDIO In-Band IRQ handler */
int sdio_IB_irq_deregister(struct if_sdio_card *card) {
    int err = 0;
    int retry = 0;

    if (!card->ib_irq_registered) {
         pr_err("%s: IB irq is not registered\n", __FUNCTION__);
         return err;
    }

RETRY:
    sdio_claim_host(card->func);
    err = sdio_release_irq(card->func);
    sdio_release_host(card->func);
    if (err) {
        if (retry++ < (RETRY_THRESHOLD*2)) {
            //pr_err("%s:%d error retry (err:%d)\n", __func__, __LINE__, err);
            //pr_err("delay %d ms\n", retry*(RETRY_DELAY_MS*2));
            mdelay(retry*(RETRY_DELAY_MS*2));
            goto RETRY;
        }
        pr_err("%s:%d ERROR (err:%d)\n", __func__, __LINE__, err);
    } else {
        //pr_err("%s:%d successfull (err:%d)\n", __func__, __LINE__, err);
        card->ib_irq_registered = false;
        card->ib_irq_enabled = false;
       // card->fw_irq_enabled = false;
    }
    return err;
}

static void if_sdio_OOB_irq_enable(struct if_sdio_card *card,bool enable)
{
	unsigned long flags;

	if (!card){
		pr_err("%s: card is NULL\n", __FUNCTION__);
		return;
	}
	
	if (test_bit(SDIO_REMOVE, &card->flag)){
		pr_info("%s() sdio remove\n", __func__);
		return;
	}

	spin_lock_irqsave(&card->irq_spinlock, flags);
	if (card->oob_irq_enabled != enable) {
	
		if (true == enable){
			enable_irq(card->oob_irq_num);
		}else{
			disable_irq_nosync(card->oob_irq_num);
		}
		card->oob_irq_enabled = enable;
	}
	spin_unlock_irqrestore(&card->irq_spinlock, flags);

	return;
}

static irqreturn_t if_sdio_OOB_irq_handler(int irq, void *dev_id)
{
	struct if_sdio_card *card = (struct if_sdio_card *)dev_id;
	if (!card){
		return IRQ_HANDLED;
	}
	
	if (test_bit(SDIO_REMOVE, &card->flag)){
		pr_info("%s() sdio remove\n", __func__);
		return IRQ_HANDLED;
	}

	//Disable OOB irq
	if_sdio_OOB_irq_enable(card,false);

	//if (card->rx_method == SDIO_RX_OOB_IRQ_SYNC) 
		queue_work(card->rx_workqueue, &card->rx_worker);
	//else
	//	pr_err("%s() :Not use RX OOB IRQ mode\n", __func__);
	
	return IRQ_HANDLED;
}

int if_sdio_OOB_irq_register(struct if_sdio_card *card)
{
	int err = 0;
	if (!card){
		pr_err("%s: card is NULL\n", __FUNCTION__);
		return -EINVAL;
	}

	if (card->oob_irq_registered) {
		pr_err("%s: irq is already registered\n", __FUNCTION__);
		return -EBUSY;
	}

	pr_info("%s : enable_irq ,OOB irq=%d flags=%X\n", __FUNCTION__,
	(int)card->oob_irq_num, (int)card->oob_irq_flags);
	
	err = request_irq(card->oob_irq_num, if_sdio_OOB_irq_handler,
		card->oob_irq_flags, "lynx_sdio", card);

	if (err) {
		pr_err("%s: request_irq failed with %d\n", __FUNCTION__, err);
		return err;
	}
	
	card->oob_irq_enabled = true;
	card->oob_irq_registered = true;
	return err;
}

void if_sdio_OOB_irq_unregister(struct if_sdio_card *card)
{
	int err = 0;
	if (!card){
		pr_err("%s: card is NULL\n", __FUNCTION__);
		return;
	}

	if (!card->oob_irq_registered) {
		pr_err("%s: OOB irq is not registered\n", __FUNCTION__);
		return;
	}

	if (card->oob_irq_wake_enabled) {
		err = disable_irq_wake(card->oob_irq_num);
		if (!err)
			card->oob_irq_wake_enabled = false;
	}

	if (card->oob_irq_enabled) {
		disable_irq(card->oob_irq_num);
		card->oob_irq_enabled = false;
	}

	free_irq(card->oob_irq_num, card);
	card->oob_irq_registered = false;
	return;
}

/* Set short period for lower latency */
#define TX_AGGRE_TIMEOUT_MS 25

static void sdio_cpu_aggregation_workqueue(struct work_struct *work)
{
	struct if_sdio_card *card = container_of(work, struct if_sdio_card, cpu_aggregation_worker);
	struct sdio_func *func;
	struct lynx_hif_device *hdev;
	u32 remain;
	struct sk_buff *skb = NULL;
	unsigned long flags;
	sdio_header* hdr;
	tx_buffer_entry* tx_buf_entry = NULL;
	unsigned long timeout;

	if (!card){
		return;
	}

	func = card->func;
	hdev = sdio_get_drvdata(card->func);
	timeout = msecs_to_jiffies(TX_AGGRE_TIMEOUT_MS) + jiffies;

	while(1){

		if (test_bit(SDIO_REMOVE, &card->flag)){
			pr_info("%s() sdio remove\n", __func__);
			goto FREE_ENTRY;
		}

		if (tx_buffer_list_full(&card->tx_buffer_list)) {
			pr_debug("%s() list is full (tx buf cnt in list : %d)\n", __func__, card->tx_buffer_list.buffer_cnt);
			/* whoever consume the the tx list must wake up this worker */
			return;
		}

		//pr_debug("%s() allocate new aggre buffer to put skb into\n", __func__);
		if (!tx_buf_entry){
			tx_buf_entry = tx_buffer_allocate();
			if (!tx_buf_entry) {
				pr_err("%s() %d failed to allocate memory\n", __func__, __LINE__);
				return;
			}
		}
		
		spin_lock_irqsave(&hdev->tx_sdio_lock, flags);
		skb = __skb_dequeue(&hdev->tx_skb_queue);
		spin_unlock_irqrestore(&hdev->tx_sdio_lock, flags);

		if(!skb) {
			pr_debug("%s() no more skb\n", __func__);
			if (tx_buf_entry->end) {
				pr_debug("%s() enqueue tx buffer with data (end 0x%x start 0x%x size 0x%x)\n", __func__
				, tx_buf_entry->end , tx_buf_entry->start , tx_buf_entry->size);
				goto SEND_ENTRY;
			} else {
				pr_debug("%s() free tx buffer with no data\n", __func__);
				goto FREE_ENTRY;
			}
		}

		remain = tx_buf_entry->size - tx_buf_entry->end;

		if (SDIO_SIZE_ALIGN(skb->len, card) <= remain) {
			pr_debug("%s() copy skb to tx buffer\n", __func__);

			/* copy transfer data to skb ring */
			hdr = (sdio_header*)skb->data;
			hdr->res = cpu_to_be16(SDIO_SIZE_ALIGN(be16_to_cpu(hdr->payload_len), card) -
						be16_to_cpu(hdr->payload_len));

			memcpy(tx_buf_entry->buf + tx_buf_entry->end, skb->data, skb->len);
			tx_buf_entry->end += SDIO_SIZE_ALIGN(skb->len, card);

			pr_debug("%s() updated end 0x%x (size 0x%x)\n", __func__, tx_buf_entry->end, tx_buf_entry->size);

			/* queue skb for recycle task to free */
			spin_lock_irqsave(&hdev->tx_recycle_lock, flags);
			__skb_queue_tail(&hdev->tx_recycle_queue, skb);
			spin_unlock_irqrestore(&hdev->tx_recycle_lock, flags);

			queue_work(card->tx_recycle_workqueue, &card->tx_recycle_worker);

			/* Consider CPU time limited */
			if(time_after(jiffies, timeout)){
				/*over TX_AGGRE_TIMEOT_MS*/
				/*push current aggre buffer to send*/
				goto SEND_ENTRY;
			}
			
			/* Get more SKB to fill current tx_buf_entry*/
			/* Keep current tx_buf_entry*/

		} else {
			/*restore back current tx SKB*/
			spin_lock_irqsave(&hdev->tx_sdio_lock, flags);
			__skb_queue_head(&hdev->tx_skb_queue, skb);
			spin_unlock_irqrestore(&hdev->tx_sdio_lock, flags);
			pr_debug("%s() enqueue full tx buffer with data (end 0x%x start 0x%x size 0x%x)\n",
					__func__, tx_buf_entry->end , tx_buf_entry->start , tx_buf_entry->size);

			/*push current aggre buffer to send*/
			tx_buffer_enqueue(tx_buf_entry, &card->tx_buffer_list);
			queue_work(card->tx_workqueue, &card->tx_worker);
			/* Set tx_buf_entry NULL for get New one*/
			tx_buf_entry = NULL;
			
			/* Consider CPU time limited */
			if(time_after(jiffies, timeout)){
				/*over TX_AGGRE_TIMEOUT_MS*/
				return;
			}

			/* Re allocate new aggre buffer to fill skb*/
		}
			
	} /* while(1) */

	return;

SEND_ENTRY:

	tx_buffer_enqueue(tx_buf_entry, &card->tx_buffer_list);
	queue_work(card->tx_workqueue, &card->tx_worker);
	return;

FREE_ENTRY:
	if(tx_buf_entry){
		tx_buffer_free(tx_buf_entry);
		tx_buf_entry = NULL;
	}
	return;
}

/*Get data from rx_compress_queue to rx_skb_queue (filter SDIO header) */
static void  sdio_rx_decompress_workqueue(struct work_struct *work)
{
    struct if_sdio_card *card = container_of(work, struct if_sdio_card, rx_decompress_worker);
    struct sdio_func *func;
    struct lynx_hif_device *hdev;
    unsigned long flags;
    struct sk_buff *compressed_skb;
    u8 *data, *ptr;
    sdio_header* hdr;
    struct sk_buff *skb;
    int debug_cnt = 0, i;
    u8 cksum;

    if (!card)
        return;

    pr_debug("%s() is started\n", __func__);

    func = card->func;
    hdev = sdio_get_drvdata(card->func);

    while (1) {
        if (test_bit(SDIO_REMOVE, &card->flag)){
            pr_info("%s() sdio remove\n", __func__);
            break;
        }
        spin_lock_irqsave(&hdev->rx_compress_lock, flags);
        compressed_skb = __skb_dequeue(&hdev->rx_compress_queue);
        spin_unlock_irqrestore(&hdev->rx_compress_lock, flags);
        if (!compressed_skb) {
            pr_debug("%s() no compressed skb\n", __func__);
            break;
        }

        if (debug_on) {
            pr_info("%s() get a compressed skb\n", __func__);
            msleep(300);
        }

        data = compressed_skb->data;
        debug_cnt = 0;
        while (data < skb_tail_pointer(compressed_skb)) {
            u32 len;

            if (test_bit(SDIO_REMOVE, &card->flag)){
                pr_err("sdio remove!\n");
                break;
            }

            if (debug_cnt > 200) {
                pr_err("%s() compressed_skb contains abnormal number of skb\n", __func__);
                break;
            }
            pr_debug("%s() loop of processing compressed_skb, data %p skb_tail %p\n",
                __func__, data, skb_tail_pointer(compressed_skb));

            /* get sdio header */
            hdr = (sdio_header*)data;
            len = be16_to_cpu(hdr->payload_len);
            if (len > compressed_skb->len) {
                pr_err("%s() invalid payload_len\n", __func__);
                break;
            }

            if (hdr->id & ~SDIO_CMD_MASK) {
                pr_err("%s() corrupted packet (last_seq_no 0x%x)\n",__func__,  last_rx_seq_no);
                rx_buffer_dump();
                break;
            }

            if (debug_on) {
                pr_info("%s() payload id 0x%x, seq_no 0x%x, len %d\n", __func__, hdr->cksum, hdr->seq_no, len);
                msleep(300);
            }

            if ((card->info.sdio_cksum_enable) && (hdr->id & SDIO_CMD_DATA)){
                cksum = 0;
                for (i = 0, ptr = (u8*)(data + SDIO_HDR_SIZE); i < (len - SDIO_HDR_SIZE); i++) {
                    cksum += ptr[i];
                }
                if (cksum != hdr->cksum) {
                    pr_err("%s() checksum 0x%x  ERROR !!!!(id 0x%x seq_no 0x%x len %u (0x%x))\n",
                        __func__, cksum, hdr->cksum, hdr->seq_no, len, len);
                    rx_buffer_dump();
                    break;
                }
            }
		if (hdr->id & SDIO_CMD_RX_TEST) {
			pr_err("%s() RX_TEST packet\n",__func__);
			goto PROCESS_DONE;
		}
		
		if (!(hdr->id & SDIO_CMD_DATA)) {
			pr_debug("%s() not a data, so drop this pkt\n",__func__);
			goto PROCESS_DONE;
		}

            /* check sequential number */
            if ((((int)hdr->seq_no - (int)last_rx_seq_no) != 1) && (((int)hdr->seq_no - (int)last_rx_seq_no) != -255) ){
/*
                pr_err("%s() dis-continuous seq number: seq_no 0x%x last_seq_no 0x%x\n",
                    __func__, hdr->seq_no, last_rx_seq_no);
                rx_buffer_dump();
*/
            }

            /* queue skb for precessing */
            if (test_bit(SDIO_RX_WAIT, &card->flag)) {
                //pr_info("%s() copy firmware data\n", __func__);
                card->rx_buflen = len - SDIO_HDR_SIZE;
                memcpy(card->rx_buffer, data + SDIO_HDR_SIZE , len - SDIO_HDR_SIZE);
                //sdio_hexdump(card->rx_buffer, len - SDIO_HDR_SIZE);

                //pr_info("%s() clear SDIO_RX_WAIT\n", __func__);
                clear_bit(SDIO_RX_WAIT, &card->flag);
                //pr_info("%s() wake_up process in card->rx_wq\n", __func__);
                wake_up(&card->rx_wq);

            } else if (test_bit(SDIO_ACTIVE, &card->flag)) {
                //pr_err("%s() copy to hif\n", __func__);
                skb = dev_alloc_skb(len - SDIO_HDR_SIZE);
                if (!skb) {
                    hdev->rx_skb_alloc_fail_cnt++;
                    pr_err("%s:%d Error(NOMEM) RX alloc fail count (%d)\n", __func__, __LINE__, hdev->rx_skb_alloc_fail_cnt);
                } else {
                    if (debug_on) {
                        pr_info("%s() copy to hif\n", __func__);
                        msleep(300);
                    }
                    memcpy(skb_put(skb, len - SDIO_HDR_SIZE), data + SDIO_HDR_SIZE, len - SDIO_HDR_SIZE);

                    spin_lock(&hdev->rx_lock);
                    __skb_queue_tail(&hdev->rx_skb_queue, skb);
                    spin_unlock(&hdev->rx_lock);

                    hif_device_recv(hdev, 0);
                }
            } else {
                pr_info("%s() copy to rx_dummy\n", __func__);
                memcpy(card->rx_dummy, data + SDIO_HDR_SIZE, len - SDIO_HDR_SIZE);
            }
PROCESS_DONE:
            last_rx_seq_no = hdr->seq_no;
            
            /* forward to next skb */
            data += SDIO_SIZE_ALIGN(len, card);
            pr_debug("%s() forward to next skb (after updated : data 0x%p, skb_tail 0x%p)\n",
                __func__, data, skb_tail_pointer(compressed_skb));

            /* for debugging */
            debug_cnt ++;
        }

        dev_kfree_skb(compressed_skb);

        if (skb_queue_len(&hdev->rx_skb_queue) > 0)
            hif_device_recv(hdev, 0);

    }
    pr_debug("%s() is ended\n", __func__);

}

static void sdio_tx_recycle_workqueue(struct work_struct *work)
{
    struct if_sdio_card *card = container_of(work, struct if_sdio_card, tx_recycle_worker);
    struct sdio_func *func;
    struct lynx_hif_device *hdev;
    unsigned long flags;
    struct sk_buff *skb;
    u32 tx_cnt_before = 0;
    u32 tx_cnt_after = 0;

    if (!card)
        return;

    func = card->func;
    hdev = sdio_get_drvdata(card->func);

    while (1) {

       if (test_bit(SDIO_REMOVE, &card->flag)){
            pr_info("%s() sdio remove\n", __func__);
            break;
        }
        spin_lock_irqsave(&hdev->tx_sdio_lock, flags);
        tx_cnt_before = skb_queue_len(&hdev->tx_skb_queue);
        spin_unlock_irqrestore(&hdev->tx_sdio_lock, flags);

        spin_lock_irqsave(&hdev->tx_recycle_lock, flags);
        skb = __skb_dequeue(&hdev->tx_recycle_queue);
        spin_unlock_irqrestore(&hdev->tx_recycle_lock, flags);
        if (!skb)
            break;
        
        /* free tx skb */
        //hdev->sdio_tx_callback = NULL;
        if (!hdev->sdio_tx_callback) {
            pr_err("no sdio_tx_callback found!\n");
            dev_kfree_skb(skb);
        } else {
#ifdef USE_WMM_QUEUE
            skb_pull(skb, SDIO_HDR_SIZE);
            hdev->sdio_tx_callback(hdev, skb, 0, true);
#else
            dev_kfree_skb(skb);
#endif
        }
        spin_lock_irqsave(&hdev->tx_sdio_lock, flags);
        tx_cnt_after = skb_queue_len(&hdev->tx_skb_queue);
        spin_unlock_irqrestore(&hdev->tx_sdio_lock, flags);
        if ((!tx_cnt_before) && tx_cnt_after) {
            pr_debug("tx might go rest, try to schedule tx again\n");
#ifndef CPU_AGGREGATION_ENABLE
            queue_work(card->tx_workqueue, &card->tx_worker);
#else
            queue_work(card->cpu_aggregation_workqueue, &card->cpu_aggregation_worker);
#endif            
        }

    }
}

static void sdio_rx_workqueue(struct work_struct *work)
{
	struct if_sdio_card *card = container_of(work, struct if_sdio_card, rx_worker);
	int retry = 3;
	int poll_period = 10;
	int rc = 0;
	bool exist = false;/*NOT set busyloop*/
	bool load = false;
	int timeout = 30;
	int poll_cnt=0;
	int lightload_cnt=0;
	int lightload_ratio=0;
	//pr_info("%s() start\n", __func__);

RE_WAIT:
	poll_cnt=0;
	lightload_cnt=0;
	lightload_ratio=0;
	
	if(card->rx_method == SDIO_RX_IB_IRQ_BURST){
		if (test_bit(SDIO_REMOVE, &card->flag)){
			pr_info("%s() sdio remove\n", __func__);
			return;
		}

		rc = wait_event_interruptible_timeout(card->rx_irq_wq, test_bit(SDIO_IRQ, &card->flag), msecs_to_jiffies(timeout));
		if (rc >= 1) { /* condition true */
			//printk(KERN_ERR "\nB\n");
			set_bit(SDIO_BURST, &card->flag);
			clear_bit(SDIO_IRQ, &card->flag);
			sdio_FW_irq_enable(card, false) ;
			if_sdio_IB_irq_enable(card,false);
		}else {
			goto RE_WAIT;
		}
	}

RE_DO:

	if (test_bit(SDIO_REMOVE, &card->flag)){
		pr_info("%s() sdio remove\n", __func__);
		return;
	}
	
	poll_cnt++;
	retry = 3;


	/* INPUT  :  set "exist" as true for return -1 when no get data   */
	/* OUTPUT: "exist" means get data or not                 */
	while( (retry) && (if_sdio_card_to_host(card,&exist,&load,NULL) )) {
		retry--;
		if (test_bit(SDIO_REMOVE, &card->flag)) 
			return;
		else if (retry > 0)
			msleep(retry*RETRY_DELAY_MS);
	}

#ifdef CPU_AGGREGATION_ENABLE
		/* Cause already get tx consumer ptr*/
	if( tx_buffer_len_remain(card, NULL)&&
			(tx_buffer_list_count(&card->tx_buffer_list))){
		queue_work(card->tx_workqueue, &card->tx_worker);
	}
#endif
		
	//enable OOB irq cause disable it when get IRQ
	if((card->rx_method == SDIO_RX_OOB_IRQ_SYNC)){
		
		if_sdio_OOB_irq_enable(card,true);
		return;
	}

	else if((card->rx_method == SDIO_RX_IB_IRQ_SYNC)){
		
		if_sdio_IB_irq_enable(card,true);
		return;
	}

	else if(card->rx_method == SDIO_RX_POLLING){
		// polling rx mode take a rest then poll again 
		msleep(poll_period);
		exist = false;/*NOT set busyloop*/
		goto RE_DO;
	}else if(card->rx_method == SDIO_RX_IB_IRQ_BURST){

		if(false == load)
			lightload_cnt++;

		if(poll_cnt >= 400){ /*6sec*/
			lightload_ratio = (lightload_cnt*100)/poll_cnt;
			poll_cnt=0;
			lightload_cnt=0;
			//pr_info("L%d\n", lightload_ratio);
		}

		/*idle:100,ping 100,32k:99,64k:98,
			128k:96,512K:86,1M:72*/
		if (lightload_ratio>96){ /*lower than 128K*/
			//printk(KERN_ERR "\nD%d\n", lightload_ratio);
			if_sdio_IB_irq_enable(card,true);
			sdio_FW_irq_enable(card, true) ;
			clear_bit(SDIO_BURST, &card->flag);
			goto RE_WAIT;
		}
		
		// burst rx mode take a rest then poll again 
		msleep(poll_period);
		exist = false;/*NOT set busyloop*/
		goto RE_DO;
	}

}


#ifdef CPU_AGGREGATION_ENABLE

#define TX_SEND_TIMEOUT_MS 200
static void sdio_tx_workqueue(struct work_struct *work)
{
	struct if_sdio_card *card = container_of(work, struct if_sdio_card, tx_worker);
	struct sdio_func *func;
	struct lynx_hif_device *hdev;
	int err = 0;
	u32 buf_remain = 0;
	u32 trans_remain = 0;
	u32 trans_length = 0;
	unsigned long timeout;
	
#ifdef SDIO_RX_HIGH_PRITORY
	bool exist = false;
	bool noloop = true;
#endif

	if (!card) {
		pr_err("%s %d: no card structure\n", __func__, __LINE__);
		return;
	}

	func = card->func;
	hdev = sdio_get_drvdata(card->func);
	timeout = msecs_to_jiffies(TX_SEND_TIMEOUT_MS) + jiffies;

	/* Consider CPU time limited */
	while (time_before(jiffies, timeout)) {

		if (card->fatal_error) {
			dev_err(&func->dev, "%s %d: tx can't work anymore\n", __func__, __LINE__);
			goto FREE_TX_BUFFER;
		}

		if (test_bit(SDIO_REMOVE, &card->flag)){
			pr_info("%s() sdio remove\n", __func__);
			goto FREE_TX_BUFFER;
		}

		/* Get tx_buf_entry if have no completed tx_buf_entry */
		/* card->tx_buf_entry is a storage for working */

		if (! card->tx_buf_entry){
			card->tx_buf_entry = tx_buffer_dequeue(&card->tx_buffer_list);
			if (! card->tx_buf_entry) {
				//pr_debug("%s()  no aggregated tx_buffer, invoke cpu_aggregation_worker now\n", __func__);
				queue_work(card->cpu_aggregation_workqueue, &card->cpu_aggregation_worker);

#ifdef SDIO_RX_HIGH_PRITORY
				/* After TX complete ,help to check rx if data */
				if_sdio_card_to_host(card,&exist,NULL,NULL) ;
#endif
				goto EXIT;
			}
		}

		trans_remain = (card->tx_buf_entry->end - card->tx_buf_entry->start);
		if (0 == trans_remain){
			pr_info("%s() trans_remain is zero?!(end 0x%x, start 0x%x size 0x%x)\n",__func__,
				card->tx_buf_entry->end,card->tx_buf_entry->start,card->tx_buf_entry->size);
			goto FREE_TX_BUFFER;
		}

#ifdef SDIO_RX_HIGH_PRITORY
		buf_remain = tx_buffer_len_remain(card, NULL);
		if (0 == buf_remain) {
			/* Rx also help get tx consumer ptr*/
			if_sdio_card_to_host(card,&exist,NULL,&noloop) ;
		}
#endif

		/* foresee the remain len of lynx if appropriate number skb can be dequeue */
		buf_remain = tx_buffer_len_remain(card, NULL);
		
		if (0 == buf_remain) {
			sdio_claim_host(card->func);
			get_tx_reader_ptr(card);
			sdio_release_host(card->func);
			buf_remain = tx_buffer_len_remain(card, NULL);
		}


		if (0 == buf_remain) {
			/*
			pr_info("%s() lynx has no room for TX data\n", __func__);
			//this interrupt might help TX_DONE interrupt lost 
			sdio_claim_host(card->func);
			sdio_f0_writeb(card->func, (unsigned char)SDIO_HINT_SW_RESERVE5, SDIO_CCCR_HINT_SET, &err);
			sdio_release_host(card->func);
			*/
			goto RETRY_TX;
		}

		if (trans_remain > buf_remain) {
			trans_length = buf_remain;
		} else {
			trans_length = trans_remain;
		}

		//pr_debug("%s() %d trans_length 0x%x\n", __func__, __LINE__, trans_length);

		/* start to send aggregated data */

		//sdio_claim_host(card->func);
		err = if_sdio_host_to_card(card, card->tx_buf_entry->buf + card->tx_buf_entry->start, trans_length, false);
		//sdio_release_host(card->func);
		
		if (err) {
			dev_err(&func->dev, "%s %d: sdio_tx fail!\n", __func__, __LINE__);
			goto RETRY_TX;
		}
		
		/*update tx_buf_entry*/
		card->tx_buf_entry->start += trans_length;
		
		
		sdio_claim_host(card->func);
		/* sync TX buffer producer to lynx */
		if (set_tx_producer_ptr(card, tx_buf_ptr)) {
			sdio_release_host(card->func);
			goto RETRY_TX;
		}


		if (tx_commit(card)){
			dev_err(&func->dev, "%s %d: This ia un-recoverable fatal error!\n", __func__, __LINE__);
			card->fatal_error = 1;
			/* no need to queue TX worker anymore */
			sdio_release_host(card->func);
			goto FREE_TX_BUFFER;
		}
		
		sdio_release_host(card->func);
		/* Success : call aggregation worker to refill*/
		queue_work(card->cpu_aggregation_workqueue, &card->cpu_aggregation_worker);
		

		/*Re calucate trans_remain*/
		trans_length = 0;
		trans_remain = card->tx_buf_entry->end - card->tx_buf_entry->start;
		/*Check trans_remain*/
		if (!trans_remain) {
			//pr_debug("%s() all data in aggregated buffer is transfered\n", __func__);
			/* all data in aggregated buffer is transfered */
			if(card->tx_buf_entry){
				tx_buffer_free(card->tx_buf_entry);
				card->tx_buf_entry = NULL;
			}
			
			/* Clean tx_buf_entry ; Get New tx_buf_entry from list*/
		}

	}/* while (time_before(jiffies, timeout) */

	/* if timeout ,retry later */
RETRY_TX:
	/* card->tx_buf_entry still exist for next try*/
	queue_work(card->tx_workqueue, &card->tx_worker);
	goto EXIT;

FREE_TX_BUFFER:
	/* Clean card->tx_buf_entry */
	if(card->tx_buf_entry){
		tx_buffer_free(card->tx_buf_entry);
		card->tx_buf_entry = NULL;
	}

EXIT:
	return;
}

#else /* no skb ring */

#if 1 /* no aggregation, transfer single skb at a time */
static void sdio_tx_workqueue(struct work_struct *work)
{
    struct if_sdio_card *card = container_of(work, struct if_sdio_card, tx_worker);
    struct sdio_func *func;
    struct lynx_hif_device *hdev;
    int err = 0;
    u32 buf_remain = 0;
    unsigned long flags;
    u32 queue_cnt = 0;
    struct sk_buff *skb = NULL;
    int full_cnt = 0;

#ifdef SDIO_RX_HIGH_PRITORY
	int retry = 3;
	bool exist = false;/*NOT set busyloop*/
	if (!card)
		return;

	while( (retry) && (if_sdio_card_to_host(card,&exist,NULL,NULL) )) {
		retry--;
		if (test_bit(SDIO_REMOVE, &card->flag)) 
			return;
		else if (retry > 0)
			msleep(retry*RETRY_DELAY_MS);
	}
#endif

    if (!card)
        return;
    func = card->func;
    hdev = sdio_get_drvdata(card->func);

GET_SKBS:

    if (test_bit(SDIO_REMOVE, &card->flag)){
       pr_info("%s() sdio remove\n", __func__);
       return ;
    }
    /* detect if tx buffer full event is abnormal */
    if (full_cnt > 1000) {
        //pr_err("%s() might need to detect the lost of tx event\n", __func__);
        if (debug_on) {
            //pr_info("%s() might need to detect the lost of tx event\n", __func__);
            msleep(1000);
        }
        full_cnt = 0;

        /* this interrupt might help TX_DONE interrupt lost */
        sdio_claim_host(card->func);
        sdio_f0_writeb(card->func, (unsigned char)SDIO_HINT_SW_RESERVE5, SDIO_CCCR_HINT_SET, &err);
        sdio_release_host(card->func);
    }

    /* foresee the remain len of lynx if appropriate number skb can be dequeue */
    sdio_claim_host(card->func);
    get_tx_reader_ptr(card);
    buf_remain = tx_buffer_len_remain(card, NULL);
    sdio_release_host(card->func);

    if (debug_on) {
        pr_info("%s() buf_remain : %d\n", __func__, buf_remain);
        msleep(1000);
    }

    /* get multiple skb */
    dev_dbg(&func->dev, "%s() start getting skbs.\n", __func__);
    spin_lock_irqsave(&hdev->tx_sdio_lock, flags);
    while (1) {

        skb = __skb_dequeue(&hdev->tx_skb_queue);

        if (skb && (SDIO_SIZE_ALIGN(skb->len, card) <= buf_remain)) {
            sdio_header* hdr = (sdio_header*)skb->data;
            hdr->res = cpu_to_be16(SDIO_SIZE_ALIGN(be16_to_cpu(hdr->payload_len), card) -
                    be16_to_cpu(hdr->payload_len));

            /* queue skb to sender */
            spin_lock_irqsave(&hdev->sender_lock, flags);
            __skb_queue_tail(&hdev->sender_queue, skb);
            spin_unlock_irqrestore(&hdev->sender_lock, flags);
            queue_cnt++;
            buf_remain -= SDIO_SIZE_ALIGN(skb->len, card);
            dev_dbg(&func->dev, "%s() put pkt in sender queue(queue_cnt %d buf_remain %d).\n", __func__, queue_cnt, buf_remain);
            full_cnt = 0;
        } else if (skb && (SDIO_SIZE_ALIGN(skb->len, card) > buf_remain) && queue_cnt) {
            /* got best count of skb already, and start sender */
            __skb_queue_head(&hdev->tx_skb_queue, skb);
            dev_dbg(&func->dev, "%s() start sender (queue_cnt %d buf_remain %d).\n", __func__, queue_cnt, buf_remain);
            full_cnt = 0;
            break;
        } else if (skb && (SDIO_SIZE_ALIGN(skb->len, card) > buf_remain) && !queue_cnt) {
            /*
             * lynx don't have enough space for tx, and no sender pkt.
             * queue skb back and take a breath.
             */
            __skb_queue_head(&hdev->tx_skb_queue, skb);
            spin_unlock_irqrestore(&hdev->tx_sdio_lock, flags);
            if (debug_on) {
                pr_info("%s() lynx has no room in tx buffer, retry again later\n", __func__);
                msleep(1000);
            }
            full_cnt++;
            goto GET_SKBS;
        } else if (!skb && queue_cnt) {   
            /* no rest skb and go send now */
            dev_dbg(&func->dev, "%s() no rest pkt and go send now (tx_skb_queue_cnt 0x%x)\n", __func__, skb_queue_len(&hdev->tx_skb_queue));
            full_cnt = 0;
            break;
        } else if (!skb && !queue_cnt) {
            /* no rest pkt and no sender, mission accomplish */
            dev_dbg(&func->dev, "%s() exit tx mission (tx_skb_queue_cnt 0x%x)\n", __func__, skb_queue_len(&hdev->tx_skb_queue));
            spin_unlock_irqrestore(&hdev->tx_sdio_lock, flags);
            full_cnt = 0;
            return;
        }
    }
    spin_unlock_irqrestore(&hdev->tx_sdio_lock, flags);
    /* end of getting multiple skb */

    /* start sender task */
    dev_dbg(&func->dev, "%s: sender pkt count %d\n", __func__, queue_cnt);
    sdio_claim_host(card->func);
    while (1) {
        spin_lock_irqsave(&hdev->sender_lock, flags);
        skb = __skb_dequeue(&hdev->sender_queue);
        spin_unlock_irqrestore(&hdev->sender_lock, flags);
        if (!skb) {
            dev_dbg(&func->dev, "%s: no pkt in sender queue!\n", __func__);
            break;
        }

        err = if_sdio_host_to_card(card, skb->data, skb->len, false);
        if (err) {
            dev_err(&func->dev, "%s: sdio_tx fail and drop pkt!\n", __func__);
        } else {
            dev_dbg(&func->dev, "%s() send one pkt.\n", __func__);
        }

        /* queue skb for recycle task to free */
        spin_lock_irqsave(&hdev->tx_recycle_lock, flags);
        __skb_queue_tail(&hdev->tx_recycle_queue, skb);
        spin_unlock_irqrestore(&hdev->tx_recycle_lock, flags);
    }
    /* sync TX buffer producer to lynx */
    if (!err) {
        if (!set_tx_producer_ptr(card, tx_buf_ptr)) {
            if (!tx_commit(card))
                dev_dbg(&func->dev, "%s() tx commit successfully.\n", __func__);
            else
                dev_err(&func->dev, "%s() Fatal error!! : failed to do tx commit.\n", __func__);
        } else {
            dev_err(&func->dev, "%s() failed to updated tx producer pointer to lynx.\n", __func__);
        }
    }
    sdio_release_host(card->func);

    queue_work(card->tx_recycle_workqueue, &card->tx_recycle_worker);
    queue_cnt = 0;
    goto GET_SKBS;
}

#else
/* use hardware aggregation engine to read scatter list*/

static void sdio_tx_workqueue(struct work_struct *work)
{
#define SG_ENTRY_MAX 128
    struct if_sdio_card *card = container_of(work, struct if_sdio_card, tx_worker);
    struct sdio_func *func;
    struct lynx_hif_device *hdev;
    int err = 0;
    unsigned long flags;
    u32 sg_total_len1 = 0, sg_total_len2 = 0;
    u32 queue_cnt = 0, queue_cnt1 = 0, queue_cnt2 = 0;
    struct sk_buff *skb = NULL;
    struct scatterlist *sg1, *sg2;
    sdio_header *dummy, *ptr;
    buf_len_remain bf = {0};
    int full_cnt = 0;

    if (!card)
        return;
    func = card->func;
    hdev = sdio_get_drvdata(card->func);

    /* prepare a dummy NOP packet */
    dummy = kmalloc(4096, GFP_ATOMIC);
    if (!dummy) {
        pr_err("%s() failed to allocate dummy\n", __func__);
        return;
    }

    /* prepare scatter list 1*/
    sg1 = os_api_alloc(sizeof(*sg1) * SG_ENTRY_MAX, GFP_ATOMIC);
    if (!sg1) {
        if (dummy)
            kfree(dummy);
        pr_err("%s() sg1 NULL\n", __func__);
        return;
    }
    sg_init_table(sg1, SG_ENTRY_MAX);

    /* prepare scatter list 2*/
    sg2 = os_api_alloc(sizeof(*sg2) * SG_ENTRY_MAX, GFP_ATOMIC);
    if (!sg2) {
        if (sg1)
            kfree(sg1);
        if (dummy)
            kfree(dummy);
        pr_err("%s() sg2 NULL\n", __func__);
        return;
    }
    sg_init_table(sg2, SG_ENTRY_MAX);

GET_SKBS:
    sdio_claim_host(card->func);

    /* detect if tx buffer full event is abnormal */
    if (full_cnt > 1000) {
        //pr_err("%s() might need to detect the lost of tx event\n", __func__);
        full_cnt = 0;
        /* this interrupt might help TX_DONE interrupt lost */
        sdio_f0_writeb(card->func, (unsigned char)SDIO_HINT_SW_RESERVE5, SDIO_CCCR_HINT_SET, &err);
        //mdelay(2);
    }

    /* foresee the remain len of lynx if appropriate number skb can be dequeue */
    get_tx_reader_ptr(card);
    tx_buffer_len_remain(card, &bf);

    /* get multiple skb */
    dev_dbg(&func->dev, "%s() start getting skbs.\n", __func__);
    spin_lock_irqsave(&hdev->tx_lock, flags);
    if (skb_queue_len(&hdev->tx_skb_queue) <= 5) {
        dev_dbg(&func->dev, "%s() warning! queued skb is not enough for burst tranfer (cnt:%d).\n",
            __func__, skb_queue_len(&hdev->tx_skb_queue));
    }
    while (1) {

        skb = __skb_dequeue(&hdev->tx_skb_queue);

        if (skb && (SDIO_SIZE_ALIGN(skb->len, card) <= bf.buf_remain)) {
            full_cnt = 0;

            /* assemble scatter list */
            ptr = (sdio_header *)skb->data;
            ptr->res = cpu_to_be16(SDIO_SIZE_ALIGN(be16_to_cpu(ptr->payload_len), card) -
                    be16_to_cpu(ptr->payload_len));
            if (bf.remain1 >= SDIO_SIZE_ALIGN(skb->len, card)) {
                sg_set_buf(&sg1[queue_cnt1], skb->data, SDIO_SIZE_ALIGN(skb->len, card));
                //dev_err(&func->dev, "%s() sg1[%d] skb->data %p align(skb->len) 0x%x seq 0x%x cksum 0x%x payload_len 0x%x\n",
                    //__func__, queue_cnt1, skb->data, SDIO_SIZE_ALIGN(skb->len, card), ptr->seq_no, ptr->cksum, be16_to_cpu(ptr->payload_len));
                sg_total_len1 += SDIO_SIZE_ALIGN(skb->len, card);
                queue_cnt++;
                queue_cnt1++;
                bf.remain1 -= SDIO_SIZE_ALIGN(skb->len, card);
                bf.buf_remain -= SDIO_SIZE_ALIGN(skb->len, card);

                /* queue skb to sender */
                spin_lock_irqsave(&hdev->sender_lock, flags);
                __skb_queue_tail(&hdev->sender_queue, skb);
               spin_unlock_irqrestore(&hdev->sender_lock, flags);

            } else if ((bf.remain1 < SDIO_SIZE_ALIGN(skb->len, card)) && (bf.remain1)) {
#if 1 /* one strategy is to insert nop rather than tranfering one pkt two times */
                sg_set_buf(&sg1[queue_cnt1], skb->data, bf.remain1);
                //dev_err(&func->dev, "%s() dis:sg1[%d] skb->data %p partial_len 0x%x seq 0x%x cksum 0x%x payload_len 0x%x\n",
                    //__func__, queue_cnt1, skb->data, bf.remain1, ptr->seq_no, ptr->cksum, be16_to_cpu(ptr->payload_len));
                sg_total_len1 += bf.remain1;
                queue_cnt++;
                queue_cnt1++;
                bf.buf_remain -= bf.remain1;
                /* bf.remain1 = 0; */ /* should do it later*/

                sg_set_buf(&sg2[queue_cnt2], (u8*)(skb->data) + bf.remain1,
                    SDIO_SIZE_ALIGN(skb->len, card) - bf.remain1);
                //dev_err(&func->dev, "%s() dis:sg2[%d] skb->data+offset: %p partial_len 0x%x seq 0x%x cksum 0x%x payload_len 0x%x\n",
                    //__func__, queue_cnt2, skb->data + bf.remain1, SDIO_SIZE_ALIGN(skb->len, card) - bf.remain1,
                    //ptr->seq_no, ptr->cksum, be16_to_cpu(ptr->payload_len));
                sg_total_len2 += SDIO_SIZE_ALIGN(skb->len, card) - bf.remain1;
                queue_cnt++;
                queue_cnt2++;
                bf.remain2 -= (SDIO_SIZE_ALIGN(skb->len, card) - bf.remain1);
                bf.buf_remain -= (SDIO_SIZE_ALIGN(skb->len, card) - bf.remain1);

                /* now we can do it */
                bf.remain1 = 0;

                /* queue skb to sender */
                spin_lock_irqsave(&hdev->sender_lock, flags);
                __skb_queue_tail(&hdev->sender_queue, skb);
                spin_unlock_irqrestore(&hdev->sender_lock, flags);

#else /* insert nop */
                __skb_queue_head(&hdev->tx_skb_queue, skb);
                if (bf.remain1 != SDIO_SIZE_ALIGN(bf.remain1, card)) {
                    pr_err("len of bf.remain1 might be invalid for nop pkt\n");
                    while(1);
                }
                if (bf.remain1 > 4096) {
                    pr_err("nop is larger than 4096\n");
                    while(1);
                }
                dummy->id = SDIO_CMD_NOP;
                dummy->seq_no = 0x0;
                dummy->payload_len = cpu_to_be16(bf.remain1);
                dummy->res = cpu_to_be16(0);

                sg_set_buf(&sg1[queue_cnt1], dummy, bf.remain1);
                dev_err(&func->dev, "%s() dummy:sg1[%d] skb->data %p partial_len 0x%x seq 0x%x cksum 0x%x payload_len 0x%x\n",
                    __func__, queue_cnt1, dummy, bf.remain1, dummy->seq_no, dummy->cksum, be16_to_cpu(dummy->payload_len));
                sg_total_len1 += bf.remain1;
                queue_cnt++;
                queue_cnt1++;
                bf.buf_remain -= bf.remain1;
                bf.remain1 = 0;
#endif
            } else {
                sg_set_buf(&sg2[queue_cnt2], skb->data, SDIO_SIZE_ALIGN(skb->len, card));
                //dev_err(&func->dev, "%s() sg2[%d] skb->data %p align(skb->len) 0x%x seq 0x%x cksum 0x%x payload_len 0x%x\n",
                    //__func__, queue_cnt2, skb->data, SDIO_SIZE_ALIGN(skb->len, card), ptr->seq_no, ptr->cksum, be16_to_cpu(ptr->payload_len));
                sg_total_len2 += SDIO_SIZE_ALIGN(skb->len, card);
                queue_cnt++;
                queue_cnt2++;
                bf.remain2 -= SDIO_SIZE_ALIGN(skb->len, card);
                bf.buf_remain -= SDIO_SIZE_ALIGN(skb->len, card);

                /* queue skb to sender */
                spin_lock_irqsave(&hdev->sender_lock, flags);
                __skb_queue_tail(&hdev->sender_queue, skb);
                spin_unlock_irqrestore(&hdev->sender_lock, flags);
            }
            dev_dbg(&func->dev, "%s() put pkt in sender queue(queue_cnt %d buf_remain 0x%x).\n",
                    __func__, queue_cnt, bf.buf_remain);
        } else if (skb && (SDIO_SIZE_ALIGN(skb->len, card) > bf.buf_remain) && queue_cnt) {
            /* got best count of skb already, and start sender */
            full_cnt = 0;
            __skb_queue_head(&hdev->tx_skb_queue, skb);
            dev_dbg(&func->dev, "%s() start sender (queue_cnt %d buf_remain 0x%x).\n",
                    __func__, queue_cnt, bf.buf_remain);
            break;
        } else if (skb && (SDIO_SIZE_ALIGN(skb->len, card) > bf.buf_remain) && !queue_cnt) {
            full_cnt++;
            /*
             * lynx don't have enough space for tx, and no sender pkt.
             * queue skb back and take a breath.
             */
            __skb_queue_head(&hdev->tx_skb_queue, skb);
            spin_unlock_irqrestore(&hdev->tx_lock, flags);
            dev_dbg(&func->dev, "%s() refresh buf_remain:0x%x and continue polling\n", __func__, bf.buf_remain);
            sdio_release_host(card->func);
            goto GET_SKBS;
        } else if (!skb && queue_cnt) {
            full_cnt = 0;
            /* no rest skb and go send now */
            dev_dbg(&func->dev, "%s() no rest pkt and go send now (remain 0x%x bytes) (tx_skb_queue_cnt 0x%x)\n",
		__func__, bf.buf_remain, skb_queue_len(&hdev->tx_skb_queue));
            break;
        } else if (!skb && !queue_cnt) {
            full_cnt = 0;
            /* no rest pkt and no sender, mission accomplish */
            dev_dbg(&func->dev, "%s() exit tx mission (tx_skb_queue_cnt 0x%x)\n", __func__, skb_queue_len(&hdev->tx_skb_queue));
            spin_unlock_irqrestore(&hdev->tx_lock, flags);
            if (sg1)
                kfree(sg1);
            if (sg2)
                kfree(sg2);
            if (dummy)
                kfree(dummy);
            sdio_release_host(card->func);
            return;
        }
    }
    spin_unlock_irqrestore(&hdev->tx_lock, flags);
    /* end of getting multiple skb */

    /* start sender task */
    dev_dbg(&func->dev, "%s: sender pkt count %d\n", __func__, queue_cnt);
    if (!sg_total_len2) {
        u32 len_tmp = SDIO_SIZE_ALIGN(sg_total_len1, card) - sg_total_len1;
        if (len_tmp) { /* this section is for experiment */
            dev_err(&func->dev, "%s() add nop to sg1\n", __func__);
            sg_set_buf(&sg1[queue_cnt1], dummy, len_tmp);
            sg_total_len1 += len_tmp;
            queue_cnt++;
            queue_cnt1++;
            bf.remain1 -= len_tmp;
            bf.buf_remain -= len_tmp;
            dummy->id = SDIO_CMD_NOP;
            dummy->seq_no = 0x0;
            dummy->payload_len = cpu_to_be16(len_tmp);
            dummy->res = cpu_to_be16(0);
            sdio_release_host(card->func);
            return;
        }

        //dev_err(&func->dev, "%s() sg_total_len1 0x%x\n", __func__, sg_total_len1);
        err = sdio_transfer_scatter_list(card, sg1, queue_cnt1, sg_total_len1);
        if (!err)
            tx_buffer_update(card, sg_total_len1);
        else
            dev_err(&func->dev, "%s() failed to write sg\n", __func__);

    } else {
        u32 len_tmp = SDIO_SIZE_ALIGN(sg_total_len2, card) - sg_total_len2;

        //dev_err(&func->dev, "%s() sg_total_len1 0x%x\n", __func__, sg_total_len1);
        err = sdio_transfer_scatter_list(card, sg1, queue_cnt1, sg_total_len1);
        if (!err)
            tx_buffer_update(card, sg_total_len1);
        else
            dev_err(&func->dev, "%s() failed to write sg\n", __func__);

        if (len_tmp) { /* this section is for experiment */
            dev_err(&func->dev, "%s() add nop to sg2\n", __func__);
            sg_set_buf(&sg2[queue_cnt2], dummy, len_tmp);
            sg_total_len2 += len_tmp;
            queue_cnt++;
            queue_cnt2++;
            bf.remain2 -= len_tmp;
            bf.buf_remain -= len_tmp;
            dummy->id = SDIO_CMD_NOP;
            dummy->seq_no = 0x0;
            dummy->payload_len = cpu_to_be16(len_tmp);
            dummy->res = cpu_to_be16(0);
            sdio_release_host(card->func);
            return;
        }

        //dev_err(&func->dev, "%s() sg_total_len2 0x%x\n", __func__, sg_total_len2);
        err = sdio_transfer_scatter_list(card, sg2, queue_cnt2, sg_total_len2);
        if (!err)
            tx_buffer_update(card, sg_total_len2);
        else
            dev_err(&func->dev, "%s() failed to write sg\n", __func__);
    }

    /* sync TX buffer producer to lynx */
    if (!set_tx_producer_ptr(card, tx_buf_ptr)) {
        if (tx_commit(card))
            dev_err(&func->dev, "%s: tx commit error\n", __func__);
    } else {
        dev_err(&func->dev, "%s: failed to set tx producer\n", __func__);
    }
    sdio_release_host(card->func);

    /* queue transfered skb to recycle list */
    while (1) {
        spin_lock_irqsave(&hdev->sender_lock, flags);
        skb = __skb_dequeue(&hdev->sender_queue);
        spin_unlock_irqrestore(&hdev->sender_lock, flags);
        if (!skb) {
            dev_dbg(&func->dev, "%s: no pkt in sender queue!\n", __func__);
            break;
        }
        spin_lock_irqsave(&hdev->tx_recycle_lock, flags);
        __skb_queue_tail(&hdev->tx_recycle_queue, skb);
        spin_unlock_irqrestore(&hdev->tx_recycle_lock, flags);
    }

    /* call recycle worker and continue transfering task */
    queue_work(card->tx_recycle_workqueue, &card->tx_recycle_worker);
    queue_cnt = 0;
    queue_cnt1 = 0;
    queue_cnt2 = 0;
    sg_total_len1 = 0;
    sg_total_len2 = 0;
    goto GET_SKBS;

}
#endif
#endif /*define CPU_AGGREGATION_ENABLE*/
static int if_sdio_probe(struct sdio_func *func,
        const struct sdio_device_id *id)
{
    struct lynx_hif_device *hdev;
    struct if_sdio_card *card;
    struct lynx *lynx = NULL;
    int vendor_id, product_id;
    int ret = 0;
    int i;

    last_rx_seq_no = 0xff;
    tx_seq_no = 0;

    vendor_id = func->vendor;
    product_id = func->device;

    lynx_dbg(LYNX_DBG_SDIO, "sdio new func %d vendor 0x%x device 0x%x block 0x%x/0x%x\n",
            func->num, func->vendor, func->device,
            func->max_blksize, func->cur_blksize);

    pr_info("\nfunc info start:\n");
#ifdef USE_WMM_QUEUE
    pr_info("WMM support : Yes\n");
#else
    pr_info("WMM support : No\n");
#endif
#ifdef CPU_AGGREGATION_ENABLE
    pr_info("Host controller aggrgation support type : cpu aggregation\n");
#else
    pr_info("Host controller aggrgation support type : unkown\n");
#endif
    pr_info("Host controller sg support: %s\n", (func->card->host->max_segs > 1)? "Yes" : "No");
    pr_info("Host controller SDIO IRQ: %d\n", !!(func->card->host->caps & MMC_CAP_SDIO_IRQ));
    pr_info("card supported clock %d\n", func->card->cis.max_dtr);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,13,0)
    pr_info("class 0x%x vendor 0x%x, device 0x%x, max_blk_size 0x%x cur_blksize 0x%x\n"
        "enable_timeout 0x%x function_state 0x%x bus_speed 0x%x\n"
        "f_min %u f_max %u f_init %u\n"
        "actual_clock %u \n"
        "max_current_180 %u max_current_300 %u max_current_180 %u\n"
        "MMC_CAP_MMC_HIGHSPEED %d, MMC_CAP_SD_HIGHSPEED %d\n"
        "1_8V_DDR %d, 1_2V_DDR %d, SDR12 %d, SDR25 %d, SDR50 %d, SDR104 %d, DDR50 %d\n"
        "HS200_1_8V_SDR %d, HS200_1_2V_SDR %d\n"
        "sg supprt %d, max_segs %d, max_seg_size %d, max_req_size %d\n"
        "ocr_avail 0x%x, ocr_avail_sdio 0x%x, ocr_avail_sd 0x%x, ocr_avail_mmc 0x%x\n"
        "func->card->host->ios.timing 0x%x\n",
        func->class, func->vendor, func->device, func->max_blksize, func->cur_blksize,
        func->enable_timeout, func->state,      func->card->sd_bus_speed,
        func->card->host->f_min, func->card->host->f_max, func->card->host->f_init,
        func->card->host->actual_clock,
        func->card->host->max_current_180, func->card->host->max_current_300,
        func->card->host->max_current_180,
        func->card->host->caps & MMC_CAP_MMC_HIGHSPEED,
        func->card->host->caps & MMC_CAP_SD_HIGHSPEED,
        func->card->host->caps & MMC_CAP_1_8V_DDR,
        func->card->host->caps & MMC_CAP_1_2V_DDR,
        func->card->host->caps & MMC_CAP_UHS_SDR12,
        func->card->host->caps & MMC_CAP_UHS_SDR25,
        func->card->host->caps & MMC_CAP_UHS_SDR50,
        func->card->host->caps & MMC_CAP_UHS_SDR104,
        func->card->host->caps & MMC_CAP_UHS_DDR50,
        func->card->host->caps2 & MMC_CAP2_HS200_1_8V_SDR,
        func->card->host->caps2 & MMC_CAP2_HS200_1_2V_SDR,
        (func->card->host->max_segs > 1),
        func->card->host->max_segs,
        func->card->host->max_seg_size,
        func->card->host->max_req_size,
        func->card->host->ocr_avail,
        func->card->host->ocr_avail_sdio,
        func->card->host->ocr_avail_sd,
        func->card->host->ocr_avail_mmc,
        func->card->host->ios.timing);
#endif
    pr_info("func info end:\n\n");

    for (i = 0;i < func->card->num_info;i++) {
        lynx_dbg(LYNX_DBG_SDIO, "info[%d]=%s\n", i, func->card->info[i]);
    }

    if (func->card->num_info == 0) {
        pr_err("unable to identify card model\n");
        return -ENODEV;
    }

    func->card->quirks |= MMC_QUIRK_LENIENT_FN0;

    hdev = lynx_alloc_hdev(sizeof(struct if_sdio_card));
    if(hdev == NULL) {
        ret = -ENOMEM;
        goto err_alloc_hdev;
    }
    hdev->vendor_id = vendor_id;
    hdev->device_id = product_id;
    hdev->hif_ops = &hif_sdio;
    hdev->hif_type = LYNX_HIF_TYPE_SDIO;

    card = hdev_priv(hdev);

#ifdef CONFIG_PROC_FS
    sdio_card = card;
#endif

    INIT_LIST_HEAD(&card->tx_buffer_list.buffer_list);
    spin_lock_init(&card->tx_buffer_list.t_buf_lock);
    card->tx_buf_entry = NULL;

    card->func = func;

    card->fatal_error = 0;

    card->rx_buffer = kmalloc(MAX_RX_BUF_SIZE, GFP_KERNEL);
    if(!card->rx_buffer) {
        pr_err("%s() failed to allocate rx buffer\n", __func__);
        ret = -ENOMEM;
        goto err_alloc_hdev;
    }

#define AGGREGATTION_BUF_SIZE   (512*1024)
#if 0
    /* t_buf data structure */
    card->skb_aggregated = dev_alloc_skb(AGGREGATTION_BUF_SIZE);
    if (!card->skb_aggregated) {
        pr_err("%s() failed to allocate rx buffer\n", __func__);
        ret = -ENOMEM;
        goto err_alloc_hdev;
    }
    skb_put(card->skb_aggregated, AGGREGATTION_BUF_SIZE);
    card->t_buf_start = (u8*)card->skb_aggregated->data;
#else
    card->t_buf_start = (u8*)kmalloc(AGGREGATTION_BUF_SIZE, GFP_KERNEL);
#endif

    card->t_buf_size = AGGREGATTION_BUF_SIZE;
    card->prod = card->t_buf_start;
    card->cons = card->t_buf_start;
    card->t_buf_end = card->t_buf_start + card->t_buf_size;
    card->t_buf_full_flag = 0;
    card->t_buf_empty_flag = 1;
    spin_lock_init(&card->t_buf_lock);
    
    INIT_LIST_HEAD(&card->packets);
    init_waitqueue_head(&card->rx_wq);
    init_waitqueue_head(&card->rx_irq_wq);
    spin_lock_init(&card->irq_spinlock);

    card->rx_workqueue = create_workqueue("lynx_sdio_rx");
    INIT_WORK(&card->rx_worker, sdio_rx_workqueue);
    card->tx_workqueue = create_workqueue("lynx_sdio_tx");
    INIT_WORK(&card->tx_worker, sdio_tx_workqueue);

    card->cpu_aggregation_workqueue = create_workqueue("lynx_sdio_cpu_aggregation");
    INIT_WORK(&card->cpu_aggregation_worker, sdio_cpu_aggregation_workqueue);

    card->tx_recycle_workqueue = create_workqueue("lynx_sdio_tx_recycle");
    INIT_WORK(&card->tx_recycle_worker, sdio_tx_recycle_workqueue);
    card->rx_decompress_workqueue = create_workqueue("lynx_sdio_rx_decompress");
    INIT_WORK(&card->rx_decompress_worker, sdio_rx_decompress_workqueue);

    /* pre-init member of hdev for hdev not ready */
    spin_lock_init(&hdev->rx_lock);
    spin_lock_init(&hdev->tx_lock);//wmm queue
    spin_lock_init(&hdev->tx_sdio_lock); //sdio tx_skb_queue
    spin_lock_init(&hdev->tx_recycle_lock);
    spin_lock_init(&hdev->sender_lock);
    spin_lock_init(&hdev->rx_compress_lock);
    mutex_init(&hdev->tx_mutex);
    __skb_queue_head_init(&hdev->rx_skb_queue);
    __skb_queue_head_init(&hdev->tx_skb_queue);
    __skb_queue_head_init(&hdev->sender_queue);
    __skb_queue_head_init(&hdev->tx_recycle_queue);
    __skb_queue_head_init(&hdev->rx_compress_queue);

    sdio_set_drvdata(func, hdev);

    ret = if_sdio_power_on(card);
    if (ret)
        goto err_activate_card;

    card->flag = 0;
    set_bit(SDIO_ACTIVE, &card->flag);

    /* init wlan interface */
    lynx = lynx_core_create();
    if (lynx == NULL) {
        lynx_dbg(LYNX_DBG_ERR, "Failed to alloc lynx core\n");
        ret = -ENOMEM;
        goto err_hdev_destroy;
    }

    hdev->lynx = lynx;
    lynx->hif_priv = hdev;

#ifdef CONFIG_SDIO_POLLING
	if (1){
#else
	if (mp_test_firm){
#endif
		/*for some SDIO host controller not support IB IRQ or not work and*/
		/*for mp firmware ,use polling mode directly*/	
		pr_info("%s: Use SDIO RX POLLING mode directly\n", __func__);
		card->rx_method = SDIO_RX_POLLING ;
		queue_work(card->rx_workqueue, &card->rx_worker);
		msleep(1000);
	}else{
	
#ifdef CONFIG_SDIO_IB_RX_BURST
		pr_info("%s: Use SDIO RX SDIO_RX_IB_IRQ_BURST mode\n", __func__);
		card->rx_method = SDIO_RX_IB_IRQ_BURST ;
#elif defined(CONFIG_SDIO_IB_RX_DIRECT)
		pr_info("%s: Use SDIO RX SDIO_RX_IB_IRQ_DIRECT mode\n", __func__);
		card->rx_method = SDIO_RX_IB_IRQ_DIRECT ;

#elif defined(CONFIG_SDIO_IB_RX_SYNC)
		pr_info("%s: Use SDIO RX SDIO_RX_IB_IRQ_SYNC mode\n", __func__);
		card->rx_method = SDIO_RX_IB_IRQ_SYNC;

#elif defined(CONFIG_SDIO_OOB_RX_SYNC)
		pr_info("%s: Use SDIO RX SDIO_RX_OOB_IRQ_SYNC mode\n", __func__);
		card->rx_method = SDIO_RX_OOB_IRQ_SYNC ;
#else
		pr_info("%s: Not assigned.Use SDIO RX POLLING mode\n", __func__);
		card->rx_method = SDIO_RX_POLLING ;
		queue_work(card->rx_workqueue, &card->rx_worker);
		msleep(1000);
#endif

		if((card->rx_method == SDIO_RX_IB_IRQ_DIRECT)
			||(card->rx_method == SDIO_RX_IB_IRQ_BURST)
			||(card->rx_method == SDIO_RX_IB_IRQ_SYNC)
			){
			/* Register SDIO In-Band IRQ handler */
			ret = sdio_IB_irq_register(card, if_sdio_IB_irq_handler_wrap);
			if (ret){
				pr_err("%s: SDIO In-band irq register fail!\n", __func__);
				pr_info("%s: Use SDIO RX POLLING mode\n", __func__);
				card->rx_method = SDIO_RX_POLLING ;
				queue_work(card->rx_workqueue, &card->rx_worker);
				msleep(1000);
			}else{
				pr_info("%s: Use SDIO RX In-Band IRQ\n", __func__);
				if_sdio_IB_irq_enable(card,false);
				
				clear_bit(SDIO_IRQ, &card->flag);
				if(card->rx_method == SDIO_RX_IB_IRQ_BURST){
					queue_work(card->rx_workqueue, &card->rx_worker);
					msleep(1000);
				}
				if_sdio_IB_irq_enable(card,true);

#ifdef CUSTOMER_02_HW
				if(card->rx_method == SDIO_RX_IB_IRQ_BURST){
					set_bit(SDIO_IRQ, &card->flag);
					wake_up_interruptible(&card->rx_irq_wq);
				}
#endif
			}

		}else if(card->rx_method == SDIO_RX_OOB_IRQ_SYNC){
			/*for HW support OOB IRQ ,use OOB IRQ */
			card->oob_irq_num = wifi_oob_irq_num() ;
			card->oob_irq_flags = wifi_oob_irq_flags();
			
			if(card->oob_irq_num <= 0) {
				pr_err("%s: Host OOB irq is not defined\n", __func__);
				pr_info("%s: Use SDIO RX POLLING mode\n", __func__);
				card->rx_method = SDIO_RX_POLLING ;
				queue_work(card->rx_workqueue, &card->rx_worker);
				msleep(1000);
			}else{
				if(if_sdio_OOB_irq_register(card)){
					pr_err("%s: Host OOB irq register fail!\n", __func__);
					pr_info("%s: Use SDIO RX POLLING mode\n", __func__);
					card->rx_method = SDIO_RX_POLLING ;
					queue_work(card->rx_workqueue, &card->rx_worker);
					msleep(1000);
				}else{
					pr_info("%s: Use SDIO RX OOB IRQ\n", __func__);
					if_sdio_OOB_irq_enable(card,false);
					clear_bit(SDIO_IRQ, &card->flag);
					if_sdio_OOB_irq_enable(card,true);
				}
			}
		}
	}
	
	/* let wci can work */
	wci_progress_check(1, 1);
	ret = lynx_sdio_switch_on(hdev);
	if(ret) {
		dev_err(&func->dev, "%s: Test RX failed (lynx_sdio_switch_on)  \n", __func__);
		dev_err(&func->dev, "%s: Current RX_Method: %d \n", __func__,card->rx_method);
		wci_progress_check(1, 0);
		goto err_lynx_core;
	}else{
		dev_info(&func->dev, "%s: Test RX OK (lynx_sdio_switch_on)  \n", __func__);
		dev_info(&func->dev, "%s: RX_Method: %d \n", __func__,card->rx_method);
	}

    ret = lynx_core_init(lynx);
    if (ret) {
        dev_err(&func->dev, "%s: lynx core init failed\n", __func__);
        /* FIXME: Does it work? Are there other resource needed to be free? */
        goto err_lynx_core;
    }

#ifdef CONFIG_SUPPORT_MONITOR_MODE
    lynx->monitor_level=monitor_level ;
#endif /*CONFIG_SUPPORT_MONITOR_MODE */

out:
    lynx_dbg(LYNX_DBG_SDIO, "%s ret=%d\n", __func__, ret);

    return ret;

err_activate_card:

err_lynx_core:
    set_bit(SDIO_REMOVE, &card->flag);
    clear_bit(SDIO_ACTIVE, &card->flag);
    if_sdio_power_off(card);
    lynx_sdio_device_detached(hdev);
err_hdev_destroy:
    if (hdev->firmware) {
        release_firmware(hdev->firmware);
        hdev->firmware = NULL;
    }

    lynx_free_hdev(hdev);
    hdev = NULL;
#ifdef CONFIG_PROC_FS
    sdio_card = NULL;
#endif
    sdio_set_drvdata(func, NULL);
err_alloc_hdev:

    goto out;
}

static void if_sdio_remove(struct sdio_func *func)
{
    struct lynx_hif_device *hdev = sdio_get_drvdata(func);
    struct if_sdio_card *card = hdev_priv(hdev);

    if (!hdev)
        return;

    /* deinit wlan interface */

    if (if_sdio_is_boot_rom(card)) {
        dev_info(&func->dev, "Lynx [boot mode] detached\n");
#ifdef LYNX_WAIT_DOWNLOAD_FIRMWARE
        wait_for_completion(&hdev->fw_done);
        if (hdev->firmware) {
            release_firmware(hdev->firmware);
            hdev->firmware = NULL;
        }
#endif
    }
    else
    {
        dev_info(&func->dev, "Lynx [app mode] detached\n");
    }

    if_sdio_power_off(card);
    lynx_sdio_device_detached(hdev);

    if(card->rx_buffer) {
        kfree(card->rx_buffer);
    }
#if 0
    if(card->skb_aggregated) {
        dev_kfree_skb(card->skb_aggregated);
    }
 #else
    if(card->t_buf_start) {
        kfree(card->t_buf_start);
    }
 #endif
    lynx_free_hdev(hdev);
    hdev = NULL;
#ifdef CONFIG_PROC_FS
    sdio_card = NULL;
#endif
    sdio_set_drvdata(func, NULL);

    /* Undo decrement done above in if_sdio_probe */
    pm_runtime_get_noresume(&func->dev);

    dev_info(&func->dev, "lynx: SDIO layer deinitialized\n");
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
static int if_sdio_suspend(struct device *dev)
{
    struct sdio_func *func = dev_to_sdio_func(dev);
//    struct lynx_hif_device *hdev = sdio_get_drvdata(func);
//    struct if_sdio_card *card = hdev_priv(hdev);
    int ret;

    mmc_pm_flag_t flags = sdio_get_host_pm_caps(func);

    dev_info(dev, "%s: suspend: PM flags = 0x%x\n",
         sdio_func_id(func), flags);

    if (!(flags & MMC_PM_KEEP_POWER)) {
        dev_err(dev, "%s: cannot remain alive while host is suspended\n",
            sdio_func_id(func));
        return -ENOSYS;
    }

    ret = sdio_set_host_pm_flags(func, MMC_PM_KEEP_POWER);
    if (ret)
        return ret;

    return sdio_set_host_pm_flags(func, MMC_PM_WAKE_SDIO_IRQ);
}

static int if_sdio_resume(struct device *dev)
{
    struct sdio_func *func = dev_to_sdio_func(dev);
//    struct lynx_hif_device *hdev = sdio_get_drvdata(func);
//    struct if_sdio_card *card = hdev_priv(hdev);
    int ret = 0;

    dev_info(dev, "%s: resume: we're back\n", sdio_func_id(func));

    return ret;
}

static const struct dev_pm_ops if_sdio_pm_ops = {
    .suspend    = if_sdio_suspend,
    .resume     = if_sdio_resume,
};
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)) */

static struct sdio_driver if_sdio_driver = {
    .name       = "lynx_sdio",
    .id_table   = if_sdio_ids,
    .probe      = if_sdio_probe,
    .remove     = if_sdio_remove,
#ifdef LYNX_NO_PM
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
    .drv = {
        .pm = &if_sdio_pm_ops,
    },
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)) */
#endif
};

/*******************************************************************/
/* Module functions                                                */
/*******************************************************************/

#ifdef CONFIG_PROC_FS

#define MAX_ARGV 8
static int get_args(const char *string, char *argvs[])
{
    char *p;
    int n;

    argvs[0]=0;
    n = 0;
//  memset ((void*)argvs, 0, MAX_ARGV * sizeof (char *));
    p = (char *) string;
    while (*p == ' ')
        p++;
    while (*p)
    {
        argvs[n] = p;
        while (*p != ' ' && *p)
            p++;
        if (0==*p)
            goto out;
        *p++ = '\0';
        while (*p == ' ' && *p)
            p++;
out:
        n++;
        if (n == MAX_ARGV)
            break;
    }
    return n;
}

static unsigned int sdio_test_rate(uint64_t bytes, struct timespec *ts)
{
    uint64_t ns;

    ns = ts->tv_sec;
    ns *= 1000000000;
    ns += ts->tv_nsec;

    bytes *= 1000000000;

    while (ns > UINT_MAX) {
        bytes >>= 1;
        ns >>= 1;
    }

    if (!ns)
        return 0;

    do_div(bytes, (uint32_t)ns);

    return bytes;
}   

static int sdio_cmd(int argc, char *argv[])
{
    if (!sdio_card)
        goto fail;

    if (argc < 1)
    {
        goto fail;
    }
    else if (argc == 1)
    {
        if (!strcmp(argv[0], "on"))
        {
            debug_on = 1;
        } else if (!strcmp(argv[0], "tx"))
        {
             struct if_sdio_card *card = sdio_card;
             struct lynx_hif_device *hdev = sdio_get_drvdata(card->func);
            tx_buffer_dump();
            rx_buffer_dump();
            pr_err("tx pkt number in queue = %d.\n",
                    skb_queue_len(&hdev->tx_skb_queue));     

        } else if (!strcmp(argv[0], "off")) {
            debug_on = 0;
        } else if (!strcmp(argv[0], "e")) {
            u32 e_rate = 0;

            e_rate =  (empty_cnt /pkt_cnt) * 1000000;
            pkt_cnt = 0;
            empty_cnt = 0;
            pr_err("empty rate = %d\n", e_rate);
        } else if (!strcmp(argv[0], "int")) {
            struct if_sdio_card *card = sdio_card;
            u8 data;
            int ret;
            sdio_claim_host(card->func);
            data = sdio_f0_readb(card->func, SDIO_CCCR_INTx, &ret);
            sdio_release_host(card->func);
            pr_err("SDIO_CCCR_INTx 0x%x\n", data);
        } else if (!strcmp(argv[0], "td")) {
            int err = 0;
            struct if_sdio_card *card = sdio_card;
            pr_err("%s() try to recover the lost of tx event\n", __func__);
            sdio_claim_host(card->func);
            sdio_f0_writeb(card->func, (unsigned char)SDIO_HINT_SW_RESERVE5, SDIO_CCCR_HINT_SET, &err);
            sdio_release_host(card->func);
            mdelay(1000);
        } else if (!strcmp(argv[0], "event")) {
            struct if_sdio_card *card = sdio_card;
            pr_err("%s() try to queue rx event\n", __func__);
            queue_work(card->rx_workqueue, &card->rx_worker);
        } else if (!strcmp(argv[0], "ring")) {
            int err = 0;
            struct if_sdio_card *card = sdio_card;
            pr_err("%s() try to get ring pointer\n", __func__);
            sdio_claim_host(card->func);
            sdio_f0_readb(card->func, tx_ptr + 10, &err);
            sdio_release_host(card->func);
        } else if (!strcmp(argv[0], "stop")) {
            int_stop = !int_stop;
            pr_err("%s() control interrupt to do transfer or not (int_stop : %d)\n", __func__, int_stop);
        } else if (!strcmp(argv[0], "set_get_ptr")) {
            get_ptr_cmd =1;
            pr_err("%s() : Set get_ptr to %d \n", __func__, get_ptr_cmd);
        } else if (!strcmp(argv[0], "dis_get_ptr")) {
            get_ptr_cmd =0;
            pr_err("%s() : Disable get_ptr to %d \n", __func__, get_ptr_cmd);
        } else if (!strcmp(argv[0], "set_get_ptr_debug")) {
            get_ptr_debug =1;
            pr_err("%s() : Set get_ptr_debug to %d \n", __func__, get_ptr_debug);
        } else if (!strcmp(argv[0], "dis_get_ptr_debug")) {
            get_ptr_debug =0;
            pr_err("%s() : Disable get_ptr_debug to %d \n", __func__, get_ptr_debug);
        }
    }
    else if (argc == 2)
    {
        ;
    }
    return 0;

fail:
    printk(KERN_ERR "sdio cmd is failed!!\n");
    return 0;
}
static ssize_t proc_sdio_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    struct if_sdio_card *card;
    struct lynx_hif_device *hdev = sdio_get_drvdata(sdio_card->func);
    char *start, *p;
    int ret;
    
    if (!sdio_card)
        return 0;
    if (!(start = os_api_alloc(PAGE_SIZE, GFP_KERNEL)))
        return -ENOMEM;

    card = sdio_card;

    p = start;
    p += sprintf(p, "TX debug Information:\n");
    p += sprintf(p, "tx_skb_queue_cnt = [%d]\n", skb_queue_len(&hdev->tx_skb_queue));
    p += sprintf(p, "Firmware Information:\n");
    p += sprintf(p, "datablock_start = [0x%x]\n", card->db_start);
    p += sprintf(p, "datablock_size  = [0x%x]\n", card->db_size);
    p += sprintf(p, "datablock_num   = [0x%x]\n", card->db_num);

    p += sprintf(p, "tx_ptr         = [0x%x]\n", tx_ptr);
    p += sprintf(p, "tx_buf_start   = [0x%x]\n", tx_buf_start);
    p += sprintf(p, "tx_buf_end     = [0x%x]\n", tx_buf_end);
    p += sprintf(p, "tx_buf_ptr        = [0x%x]\n", tx_buf_ptr);
    p += sprintf(p, "tx_buf_reader_ptr = [0x%x]\n", tx_buf_reader_ptr);
    p += sprintf(p, "tx_buf_size       = [0x%x]\n", tx_buf_size);

    p += sprintf(p, "rx_buf_start   = [0x%x]\n", rx_buf_start);
    p += sprintf(p, "rx_buf_end     = [0x%x]\n", rx_buf_end);
    p += sprintf(p, "rx_buf_reader_ptr   = [0x%x]\n", rx_buf_reader_ptr);
    p += sprintf(p, "rx_buf_write_ptr    = [0x%x]\n", rx_buf_write_ptr);
    p += sprintf(p, "rx_buf_size         = [0x%x]\n", rx_buf_size);
    p += sprintf(p, "rx_full_flag        = [0x%x]\n",rx_full_flag);

    ret = simple_read_from_buffer(buf, count, ppos, start, strlen(start));
    kfree(start); 
    return ret;
}
static ssize_t proc_sdio_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    char buf[300];
    int rc;
    int argc ;
    char * argv[MAX_ARGV] ;

    if (count > 0 && count < 299) {
        if (copy_from_user(buf, buffer, count))
            return -EFAULT;
        buf[count-1] = '\0';
        argc = get_args( (const char *)buf, argv );
        rc = sdio_cmd(argc, argv);
    }
    return count;
}

static const struct file_operations sdio_proc_fops = {
    .read       = proc_sdio_read,
    .write      = proc_sdio_write,
    .owner      = THIS_MODULE,
};
#endif

static int __init if_sdio_init_module(void)
{
    int ret = 0;

#ifdef CONFIG_PROC_FS
    struct proc_dir_entry *res;

    if(!(res = proc_create("sdio", S_IWUSR | S_IRUGO, NULL, &sdio_proc_fops)))
        return -ENOMEM;
#endif

    printk(KERN_INFO "Montage SDIO: Lynx SDIO driver\n");
	printk("lynx driver rev. = %s\n", LYNX_DRIVER_REV);

#ifdef CUSTOMER_02_HW
/*Android WIFI HAL (multi-wifi ver)  will help do it */
/*Android WIFI HAL (single-wifi ver) will NOT help do it */
    ret = platform_wifi_power_on();
    if (ret){
        printk("%s: power on failed!!(%d)\n", __FUNCTION__, ret);
        ret = -1;
        return ret;
    }else{
        printk("%s: power on OK!\n", __FUNCTION__);
    }
#endif /*CUSTOMER_02_HW*/

    ret = sdio_register_driver(&if_sdio_driver);

    (void) sdio_hexdump;
    (void) sdio_test_rate;

#ifdef CONFIG_LYNX_DEBUG
	if (lynx_proc_init_fs())
	{
		printk("lynx proc init error.\n");
	}
#endif

    return ret;
}

static void __exit if_sdio_exit_module(void)
{
#ifdef CONFIG_PROC_FS
    remove_proc_entry("sdio", NULL);
#endif
    sdio_unregister_driver(&if_sdio_driver);
    printk(KERN_INFO "Power OFF Montage SDIO WiFi!!\n");
	
#ifdef CUSTOMER_02_HW
/*Android WIFI HAL NOT help do it ,so do it by driver*/
    platform_wifi_power_off();//Power off and triger SDIO HOST remove card
#endif /*CUSTOMER_02_HW*/

#ifdef CONFIG_LYNX_DEBUG
	lynx_proc_exit_fs();
#endif

    printk(KERN_INFO "Montage SDIO: Lynx SDIO driver unload!!\n");
}

module_init(if_sdio_init_module);
module_exit(if_sdio_exit_module);

module_param(user_mac_addr, charp, 0644);

MODULE_AUTHOR("Montage");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Lynx driver for 802.11n wireless devices");
MODULE_FIRMWARE(FIRMWARE_LYNX_2_0_0);
MODULE_FIRMWARE(FIRMWARE_LYNX_3_0_0);
MODULE_VERSION(LYNX_DRIVER_REV);
MODULE_DEVICE_TABLE(sdio, if_sdio_ids);
