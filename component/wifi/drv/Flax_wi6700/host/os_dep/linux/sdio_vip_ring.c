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

#include <linux/list.h>
//#include <linux/seq_file.h>

#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
#endif


#if 0 /* copy from core.h */
struct lynx {
    struct device   *dev;

    /* USB/SDIO */
    void *hif_priv;
};
#endif



#if 1 /* copy from wci.h */

struct lynx_wci_hdr {
    u8  cmd_id;
    u8  seq_no;
    u16 payload_len;
};

enum WCI_CMD_ID {
	/* "1" reserved */

    /* managment cmd */
    WCI_H2D_MGT_TX = 2,
    WCI_H2D_MGT_TX_ACK,
    WCI_H2D_DEV_START,
    WCI_H2D_DEV_STOP = 5,
    WCI_H2D_DEV_SCAN = 6,
    WCI_H2D_ACCESS_MEMORY,
    WCI_H2D_GET_FW_VERSION,
    WCI_H2D_VIF_ATTACH,
    WCI_H2D_VIF_DETACH = 10,
    WCI_H2D_VIF_UPDATE,
	WCI_H2D_START_AP,
    WCI_H2D_STA_ADD,
    WCI_H2D_STA_REMOVE,
    WCI_H2D_STA_CONNECT = 15,
    WCI_H2D_STA_DISCONNECT,
    WCI_H2D_ALIVE,
	WCI_H2D_WDS_PEER_ADD,
    /* config cmd */
    WCI_H2D_SET_MAC,
    WCI_H2D_SET_CHANNEL = 20,
    WCI_H2D_SET_TX_POWER,
    WCI_H2D_SET_TXQ,
    WCI_H2D_SET_ERP,
    WCI_H2D_SET_HT,
    WCI_H2D_SET_KEY = 25,
    WCI_H2D_SET_DEF_KEY,
    WCI_H2D_SET_BSS_WEP,
    WCI_H2D_SET_BSS_PSK,
    WCI_H2D_SET_TSF,
    WCI_H2D_SET_BITRATE_MASK = 30,
    WCI_H2D_SET_BEACON,
    WCI_H2D_SET_SLOTTIME,
    WCI_H2D_SET_MON,
    WCI_H2D_SET_IE_POOL,
    WCI_H2D_SET_BSS_UPDATE = 35,
    WCI_H2D_SET_PMKSA, 
    WCI_H2D_SET_RTS_THRESHOLD,
    WCI_H2D_SET_2040_COEX,
    /* get info cmd */
    WCI_H2D_GET_VIF,
    WCI_H2D_GET_STA = 40,
    WCI_H2D_GET_TSF,
    WCI_H2D_GET_IES,
    WCI_H2D_GET_TX_POWER,
    WCI_H2D_GET_VIF_ADDR,
    WCI_H2D_GET_VIF_LINK_ST = 45,
    WCI_H2D_GET_MON,
	/* special control cmd */
    WCI_H2D_SEND_ADDBA,

    WCI_H2D_MP_TEST_CMD = 127,
    /* data packet */
    WCI_H2D_PACKET = 128,
	/* reserve 128 ~ 135 for data packet per BSS */
	WCI_H2D_PACKET_END = 135,	
	WCI_H2D_FW_INFO = 250,
	WCI_H2D_FW_DOWNLOAD = 254,
	WCI_H2D_FW_COMP = 255,
};


#endif


#if 1 /* copy from lynx_debug.h */

#define LYNX_PROCFS_NAME "lynx"
enum LYNX_DEBUG_MASK {
    LYNX_DBG_CREDIT         = 0x1,
    /* hole */
    LYNX_DBG_WLAN_TX        = 0x4,          /* wlan tx */
    LYNX_DBG_WLAN_RX        = 0x8,          /* wlan rx */

    /* hole */
    /* hole */
    LYNX_DBG_HIF            = 0x40,
    LYNX_DBG_IRQ            = 0x80,         /* interrupt processing */
    
    /*WCI*/
    LYNX_DBG_WCI            = 0x100,        /* wci cmd to device */
    LYNX_DBG_WCI_EVENT      = 0x200,        /* wci event from device */
    /* hole */
    /* hole */

    LYNX_DBG_SCATTER        = 0x1000,       /* hif scatter tracing */
    LYNX_DBG_WLAN_CFG       = 0x2000,       /* cfg80211 i/f file tracing */
    LYNX_DBG_RAW_BYTES      = 0x4000,       /* dump tx/rx frames */
    LYNX_DBG_AGGR           = 0x8000,       /* aggregation */

    /*SDIO*/
    LYNX_DBG_SDIO           = 0x10000,
    LYNX_DBG_SDIO_DUMP      = 0x20000,
    LYNX_DBG_BOOT           = 0x40000,      /* driver init and fw boot */
    /* hole */

    /*USB*/
    LYNX_DBG_USB_PROBE      = 0x100000,
    LYNX_DBG_USB_RX         = 0x200000,
    LYNX_DBG_USB_TX         = 0x400000,
    LYNX_DBG_RECOVERY       = 0x800000,

    /* generic msg */
    LYNX_DBG_ERR            = 0x1000000,    /* Show err situation */
    LYNX_DBG_WARN           = 0x2000000,    /* Show warn situation */

	/* wlan manager */
	LYNX_DBG_WM				= 0x10000000,	/* wm_manager thread tracing */

    LYNX_DBG_ANY            = 0xffffffff    /* enable all logs */
};

#define LYNX_DEBUG_DEFAULT LYNX_DBG_ANY
#if 0//def CONFIG_LYNX_DEBUG
enum LYNX_DEBUG_LEVEL {
    LYNX_DBG_OFF         = 0x00000000,
    LYNX_DBG_ETH_TX      = 0x00000004,
    LYNX_DBG_ETH_RX      = 0x00000008,
    LYNX_DBG_VFC         = 0x00000010,
    LYNX_DBG_MAC80211_CB = 0x00000040,
    LYNX_DBG_USB_TX2     = 0x00000100,
    LYNX_DBG_USB_RX2     = 0x00000200,
    LYNX_DBG_ERROR       = 0x80000000,
};
void lynx_dbg(enum LYNX_DEBUG_MASK lvl, const char *fmt, ...);
void lynx_dbg_dump(enum LYNX_DEBUG_MASK lvl, const char *msg, const char *prefix,
                   char *buf, size_t len);
#else
#define lynx_dbg(fmt, ...)
#define lynx_dbg_dump(lvl, msg, prefix, buf, len)
#endif

#endif /* end of debug .h */


#include "sdio.h"

typedef void (*tx_cb)(void *target, struct sk_buff *skb, u8 id, bool txok);

enum lynx_mpif_type {
    LYNX_MPIF_TYPE_SDIO,
    LYNX_MPIF_TYPE_USB,
};

struct lynx_mpif_ops {
	const char *name;
	int (*init) (void *hif_handle);
	void (*deinit) (void *hif_handle);
	void (*start) (void *hif_handle);
	void (*stop) (void *hif_handle);
	int (*send) (void *hif_handle, struct sk_buff *buf);
	int (*recv) (void *hif_handle, struct sk_buff *buf);

	int (*tx_lock)(void *hif_handle, unsigned long *flags);
	int (*tx_unlock)(void *hif_handle, unsigned long *flags);
};

struct lynx_mpif_device {
//    struct lynx_device *own;
    u16 vendor_id;
    u16 device_id;

    u32 fw_version;
    const char *fw_name;
    const struct firmware *firmware;
    struct completion fw_done;
    struct lynx_mpif_ops *hif_ops;
    enum lynx_mpif_type hif_type;
    //struct lynx *lynx;

    struct mutex tx_mutex;
    spinlock_t tx_lock;
    spinlock_t rx_lock;
    spinlock_t tx_recycle_lock;
    spinlock_t sender_lock;
    u8 init;
    u8 flags;
    u16 tx_buf_cnt;
    u32 rx_skb_alloc_fail_cnt;
    struct sk_buff_head tx_skb_queue;
    struct sk_buff_head sender_queue;
    struct sk_buff_head tx_recycle_queue;
    int tx_skb_queue_cnt;

#define MAX_TX_QUEUE_NUM    512
    tx_cb sdio_tx_callback;

    struct list_head tx_buf;
    struct list_head tx_pending;
    struct sk_buff_head rx_skb_queue;
#ifdef CONFIG_LYNX_OS_LINUX
    struct tasklet_struct rx_tasklet;
#endif

    struct sk_buff_head rx_compress_queue;
    spinlock_t rx_compress_lock;

    unsigned long       private[0] ____cacheline_aligned;
};


static inline void *mpdev_priv(struct lynx_mpif_device *hdev)
{
    if (hdev)
        return (void *)hdev->private;
    else
        return 0;
}


//#define USE_WMM_QUEUE
#define SKB_RING

#define SDIO_CMD_NOP            0x01
#define SDIO_CMD_LOOPBACK   0x02
#define SDIO_CMD_DATA           0x04
#define SDIO_CMD_MASK           0x7
typedef struct SDIO_HEADER {
    u8 id;
    u8 seq_no;
    u16 payload_len;
    u8 cksum;
    u8 tx_consumer;
    u16 res;
} sdio_header;
#define SDIO_HDR_SIZE   (sizeof(sdio_header))

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
//#define SDIO_LOOPBACK_ENABLE
#define SDIO_LOOPBACK_SIZE  (1600)
#define SDIO_LOOPBACK_NUM   (10 * 1000)

#define LYNX_NO_PM  // temp solution
//#define LYNX_SDIO_DBG
#define IF_SDIO_BLOCK_SIZE 512

/* identify firmware images */
#define FIRMWARE_LYNX_2_0_0     "mp_test_2.img"
#define FIRMWARE_LYNX_3_0_0     "mp_test_3.img"

//#define LYNX_WAIT_DOWNLOAD_FIRMWARE

#define HIF_SDIO_TX_STOP  BIT(0)
#define MAX_RX_BUF_SIZE   (4*1024)
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
};

struct if_sdio_card {
    struct sdio_func    *func;

    struct sdio_fw_info info;
    u32                 db_start;
    u32                 db_size;
    u32                 db_num;

#ifdef SDIO_LOOPBACK_ENABLE
    u32                 lb_rx_data_num;
#endif

    wait_queue_head_t   rx_wq;
    unsigned long       flag;

    struct workqueue_struct *rx_workqueue;
    struct work_struct      rx_worker;
    struct workqueue_struct *tx_workqueue;
    struct work_struct      tx_worker;
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
    int rom_ver;
};

/*
 * Align transfer data size for workaround some host controller's
 * bug or poor performance
 */
static inline u32 SDIO_SIZE_ALIGN(u32 len, struct if_sdio_card *card) {
    return (len + le32_to_cpu(card->info.size_align)-1) & ((u32)~(le32_to_cpu(card->info.size_align)-1));
}

#ifdef CONFIG_PROC_FS
static struct if_sdio_card *sdio_card = NULL;
#endif

char default_mac_addr[ETH_ALEN] = {0x00, 0x32, 0x11, 0x98, 0x99, 0x01};
char *user_mac_addr = NULL;
static char wt_cmd[256];

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


#if 0
static inline bool buf_needs_bounce(u8 *buf)
{
	return ((unsigned long) buf & 0x3) || !virt_addr_valid(buf);
}
#endif
/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
static void if_sdio_interrupt(struct sdio_func *func);

static int if_sdio_power_off(struct if_sdio_card *card);

/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/
extern void lynx_rx_tasklet(unsigned long data);

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
static void sdio_hexdump(void *buf, u32 len)
{
	print_hex_dump(KERN_INFO, "", DUMP_PREFIX_OFFSET,
			16, 1,
			buf, len, false);
}

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
static u32 get_tx_reader_ptr(struct if_sdio_card *card)
{
    int err;
    u8 retry = 0;
    u8 tmp;
    u32 tx_consumer_sdio_addr = tx_ptr + 10;

    sdio_claim_host(card->func);
#if 0
    /* make lynx read tx buffer producer pointerin ISR */
    sdio_f0_writeb(card->func, (unsigned char)SDIO_HINT_SW_RESERVE4, SDIO_CCCR_HINT_SET, &err);
    if (err) {
            pr_err("%s:%d write error\n", __func__, __LINE__);
    }
#endif

    full = 0;

RETRY:
    /* example code for cmd52 reading buffer pointer */
    tmp = sdio_f0_readb(card->func, tx_consumer_sdio_addr, &err);
    if (err) {
        pr_err("%s:%d read error (err:%d)\n", __func__, __LINE__, err);
        retry ++;
        if (retry < 10)
            goto RETRY;
    }
    tx_buf_reader_ptr = tx_buf_start + tmp*le32_to_cpu(card->info.size_align);

    sdio_release_host(card->func);

    return tmp;
}

static int set_tx_producer_ptr(struct if_sdio_card *card, u32 buf_ptr)
{
    int err = 0;
    u32 tx_producer_sdio_addr = tx_ptr + 9;
    u8 tmp;
    int retry = 0;

    if (!card->info.fast_tx_commit_enable) {
        sdio_claim_host(card->func);

        tmp = (buf_ptr - tx_buf_start) >> tx_size_align_sh;
RETRY:
        sdio_f0_writeb(card->func, tmp, tx_producer_sdio_addr, &err);
        if (err) {
            pr_err("%s:%d write error\n", __func__, __LINE__);
            retry ++;
            if (retry < 10)
                goto RETRY;
        } else {
            pr_debug("%s:%d write sucessful : 0x%x\n", __func__, __LINE__, tmp);
        }

        sdio_release_host(card->func);
    }

    return err;
}

#ifndef SKB_RING
static u32 tx_buffer_len_remain_for_sg(struct if_sdio_card *card){
    s32 remain = 0;

    if (full) {
        return 0;
    }

    if (tx_buf_ptr == tx_buf_reader_ptr) {
        /* empty */
        remain = tx_buf_size;
    } else {
        if (tx_buf_ptr > tx_buf_reader_ptr) {
            remain = tx_buf_size - (tx_buf_ptr - tx_buf_reader_ptr);
            pr_err("%s() return seg1 0x%x size 0x%x ", __func__, tx_buf_ptr,
                tx_buf_size - (tx_buf_ptr - tx_buf_start));
            return (tx_buf_size - (tx_buf_ptr - tx_buf_start));
        } else
            remain = tx_buf_reader_ptr - tx_buf_ptr;
    }
    remain -= card->info.size_align;

     return remain > 0? (u32)remain : (u32)0;
}
#endif
static u32 tx_buffer_len_remain(struct if_sdio_card *card){
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

     return remain > 0? (u32)remain : (u32)0;
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

    if (tx_buf_ptr == tx_buf_reader_ptr)
        full = 1;
    //pr_err("%s() tx_buf_ptr 0x%x full 0x%x", __func__, tx_buf_ptr, full);
}

static void tx_buffer_dump(void){
    pr_err("%s() start 0x%x, end 0x%x, tx_buf_ptr 0x%x, tx_buf_reader_ptr 0x%x, tx_buf_size 0x%x, full %d\n",
        __func__, tx_buf_start, tx_buf_end,
        tx_buf_ptr, tx_buf_reader_ptr, tx_buf_size, full);
}

static int get_rx_producer_ptr(struct if_sdio_card *card)
{
    int err;
    u8 retry = 0;
    u8 tmp;
    u32 rx_producer_sdio_addr = tx_ptr + 20;

    sdio_claim_host(card->func);
#if 0
    /* example code for cmd52 reading buffer pointer */
    for (i = 0, tmp = 0; i < 4; i++) {
        tmp |= (sdio_f0_readb(card->func, tx_full_flag+i, &err) << (i*8));
        if (err) {
            pr_err("%s:%d read error\n", __func__, __LINE__);
        }
    }
    full = tmp;
    //pr_err("%s:%d tx full flag 0x%x\n", __func__, __LINE__, tmp);
#else
    rx_full_flag = 0;
#endif

RETRY:
    /* example code for cmd52 reading buffer pointer */
    tmp = sdio_f0_readb(card->func, rx_producer_sdio_addr, &err);
    if (err) {
        pr_err("%s:%d read error (err:%d)\n", __func__, __LINE__, err);
        retry ++;
        if (retry < 10)
            goto RETRY;
    }
    rx_buf_write_ptr = rx_buf_start + tmp*le32_to_cpu(card->info.size_align);

    sdio_release_host(card->func);

    return err;
}

static int set_rx_consumer_ptr(struct if_sdio_card *card, u32 buf_ptr)
{
    int err = 0;
    u32 rx_consumer_sdio_addr = tx_ptr + 21;
    u8 tmp;
    int retry = 0;

    if (!card->info.fast_rx_commit_enable) {
        sdio_claim_host(card->func);

        tmp = (buf_ptr - rx_buf_start) >> tx_size_align_sh;
RETRY:
        sdio_f0_writeb(card->func, tmp, rx_consumer_sdio_addr, &err);
        if (err) {
            pr_err("%s:%d write error\n", __func__, __LINE__);
            retry ++;
            if (retry < 10)
                goto RETRY;
        } else {
            pr_debug("%s:%d write sucessful : 0x%x\n", __func__, __LINE__, tmp);
        }

        sdio_release_host(card->func);
    }

    return err;
}

static u32 rx_buffer_len_remain(struct if_sdio_card *card){
    s32 remain = 0;

    if (rx_buf_write_ptr == rx_buf_reader_ptr) {
        if (rx_full_flag)
            remain = rx_buf_size;
        else
            remain = 0;
    } else {
        if (rx_buf_write_ptr > rx_buf_reader_ptr)
            remain = rx_buf_write_ptr - rx_buf_reader_ptr;
        else
            remain = rx_buf_size - (rx_buf_reader_ptr - rx_buf_write_ptr);
    }

     return remain;
}

static void rx_buffer_update(struct if_sdio_card *card, u32 len) {
    if ((rx_buf_reader_ptr + len) == rx_buf_end) {
        rx_buf_reader_ptr = rx_buf_start;
    } else if ((rx_buf_reader_ptr + len) > rx_buf_end) {
        rx_buf_reader_ptr = rx_buf_start +
            (rx_buf_reader_ptr + len) - rx_buf_end;
    } else {
        rx_buf_reader_ptr += len;
    }

    rx_full_flag = 0;
}

static void rx_buffer_dump(void){
    pr_err("%s() start 0x%x, end 0x%x, rx_buf_write_ptr 0x%x, rx_buf_reader_ptr 0x%x, rx_buf_size 0x%x, full %d\n",
        __func__, rx_buf_start, rx_buf_end,
        rx_buf_write_ptr, rx_buf_reader_ptr, rx_buf_size, rx_full_flag);
}

static int rx_commit(struct if_sdio_card *card)
{
    int err = 0;
    if (!card->info.fast_rx_commit_enable) {
        sdio_claim_host(card->func);
        sdio_f0_writeb(card->func, (unsigned char)SDIO_HINT_SW_RX_DONE, SDIO_CCCR_HINT_SET, &err);
        if (err) {
            pr_err("%s:%d write error\n", __func__, __LINE__);
        }
        sdio_release_host(card->func);
    }
    return err;
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
            return err;
        }
        ptr |= x << (i * 8);
    }
    tx_ptr = ptr + 14;
    for (i = 0; i < sizeof(struct sdio_fw_info_tuple); i++, data++) {
        *data = sdio_f0_readb(card->func, ptr++, &err);
        if (err) {
            pr_err("%s:%d Read Func tuple failed!\n", __func__, __LINE__);
            return err;
        }
    }

    sdio_release_host(card->func);   

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
        pr_info("rx ring ptr   = 0x%x\n", le32_to_cpu(info->rx_buf_start));
        rx_buf_start = le32_to_cpu(info->rx_buf_start);
        rx_buf_write_ptr = rx_buf_start;
        rx_buf_reader_ptr = rx_buf_start;
        pr_info("rx buf size   = 0x%x\n", le32_to_cpu(info->rx_buf_size));
        rx_buf_size = le32_to_cpu(info->rx_buf_size);
        rx_buf_end = rx_buf_start + rx_buf_size;
        pr_info("rx full flag   = 0x%x\n", le32_to_cpu(info->rx_full_flag));
        pr_info("rx rx_producer   = 0x%x\n", info->rx_producer);
        pr_info("rx rx_consumer   = 0x%x\n", info->rx_consumer);
        pr_info("align size   = 0x%x\n", le32_to_cpu(info->size_align));
        pr_info("sdio_cksum_enable   = 0x%x\n", info->sdio_cksum_enable);
        pr_info("fast_tx_commit_enable   = 0x%x\n", info->fast_tx_commit_enable);
        pr_info("fast_rx_commit_enable   = 0x%x\n", info->fast_rx_commit_enable);

        if (le32_to_cpu(info->size_align)) {
                sdio_claim_host(card->func);
                sdio_set_block_size(card->func, le32_to_cpu(info->size_align));
                sdio_release_host(card->func);   
        }

        if (le32_to_cpu(info->size_align)) {
            tmp = le32_to_cpu(info->size_align);
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
    return err;
}

int sdio_transfer(struct if_sdio_card *card, void *buf,
	unsigned int sdio_addr, int len, bool read)
{
    int err = 0, retry = 0;;

    if (!len) {
        pr_err("%s() len can't be set to zero and return\n", __func__);
        return err;
    }
    if (len & (le32_to_cpu(card->info.size_align)-1)) {
        pr_err("%s() len is not aligned\n", __func__);
        return err;
    }
#if 0
    if (buf_needs_bounce(buf))
        pr_err("%s() buffer isn't DMA-able (from %pS())\n",
            __func__, __builtin_return_address(0));
#endif

resend:

    if (read) {
        err = sdio_memcpy_fromio(card->func, buf, sdio_addr, len);
    } else {
        //pr_err("%s()write data size %u buf 0x%p sdio_addr 0x%x (direction %d from %pS)\n", __func__, len, buf, sdio_addr, read, __builtin_return_address(0));
        err = sdio_memcpy_toio(card->func, sdio_addr, buf, len);
    }

    if (err) {
        retry++;
        if (retry < 5) {
            pr_err("retry: data size %u buf 0x%p sdio_addr 0x%x ret %d retry %d (direction %d from %pS)\n",
			        len, buf, sdio_addr, err, retry, read, __builtin_return_address(0));
            //mdelay(5);
            goto resend;
        } else {
            pr_err("error: data size %u buf 0x%p sdio_addr 0x%x ret %d retry %d (direction %d from %pS)\n",
			        len, buf, sdio_addr, err, retry, read, __builtin_return_address(0));
        }
    }

    return err;
}

static int tx_commit(struct if_sdio_card *card)
{
    int err = 0;

    if (!card->info.fast_tx_commit_enable) {
        sdio_claim_host(card->func);
        sdio_f0_writeb(card->func, (unsigned char)SDIO_HINT_SW_TX_DONE, SDIO_CCCR_HINT_SET, &err);
        if (err) {
            pr_err("%s:%d write error\n", __func__, __LINE__);
        }
        sdio_release_host(card->func);
    }

    return err;
}

static int if_sdio_card_to_host(struct if_sdio_card *card)
{
    struct sdio_func *func = card->func;
    struct lynx_mpif_device *hdev = sdio_get_drvdata(func);
    //int cnt = 0;
    //int len = 0;
    int err = 0;
    u32 bytes;
    struct sk_buff *skb = NULL;
    u8* buf = NULL;
    u32 seg1, seg2, size1, size2, transfer_len;
    //int debug_cnt = 0;

    while (1) {
        pr_err("%s() start to collect compressed skb\n", __func__);
        sdio_claim_host(card->func);
        get_rx_producer_ptr(card);
        sdio_release_host(card->func);
        bytes = rx_buffer_len_remain(card);
        if (bytes == 0) {
            pr_err("%s() rx is empty\n", __func__);
            break;
        }
        rx_buffer_dump();
        skb = dev_alloc_skb(bytes);
        buf = skb_put(skb, bytes);
        
        sdio_claim_host(card->func);
        transfer_len = 0;
        if ((rx_buf_reader_ptr + bytes) > rx_buf_end) {
            seg1 = rx_buf_reader_ptr;
            size1 = rx_buf_end - rx_buf_reader_ptr;
            seg2 = rx_buf_start;
            size2 = (rx_buf_reader_ptr + bytes) - rx_buf_end;

            pr_err("%s() transfer data (seg1)! rx_buf_reader_ptr 0x%x transfer_len 0x%x\n", __func__, seg1, size1);
            err = sdio_transfer(card, buf, seg1 - sdio_dma_base_addr, size1, 1);
            if (!err){
                pr_err("%s() transfer data (seg2)! rx_buf_reader_ptr 0x%x transfer_len 0x%x\n", __func__, seg2, size2);
                err = sdio_transfer(card, (buf + size1), seg2 - sdio_dma_base_addr, size2, 1);
                if (!err) {
                    transfer_len = size1 + size2;
                    //pkt_cnt++;
                } else {
                    pr_err("%s() %d err %d\n", __func__, __LINE__, err);
                }
                
            } else {
                pr_err("%s() %d err %d\n", __func__, __LINE__, err);
            }
        } else {
            pr_err("%s() transfer data! rx_buf_reader_ptr 0x%x transfer_len 0x%x\n", __func__, rx_buf_reader_ptr, bytes);
            err = sdio_transfer(card, buf, rx_buf_reader_ptr- sdio_dma_base_addr, bytes, 1);
            if (!err){
                transfer_len = bytes;
                //pkt_cnt++;
            } else {
                pr_err("%s() %d err %d\n", __func__, __LINE__, err);
            }
        }

        /* update local pointer for TX buffer producer */
        if (transfer_len) {
            rx_buffer_update(card, transfer_len);
            err = set_rx_consumer_ptr(card, rx_buf_reader_ptr);
            if (err)
                pr_err("%s() %d err %d\n", __func__, __LINE__, err);
            err = rx_commit(card);
            if (err)
                pr_err("%s() %d err %d\n", __func__, __LINE__, err);
        }
        //rx_buffer_dump();
        sdio_release_host(card->func);

        /* queue data */
        if (!err) {
            spin_lock(&hdev->rx_compress_lock);
            __skb_queue_tail(&hdev->rx_compress_queue, skb);
            spin_unlock(&hdev->rx_compress_lock);
            queue_work(card->rx_decompress_workqueue, &card->rx_decompress_worker);
        } else {
            pr_err("%s() recevice error (err : %d)\n", __func__, err);
            dev_kfree_skb(skb);
            break;
        }
    }

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
    if (!tx_buffer_is_available(card, len)) {
        if (cnt > 0) {
            pr_err("%s() buffer not enough! wait retry cnt %d (reqest len 0x%x)\n", __func__, cnt, len);
            tx_buffer_dump();
        }
        /* refresh buffer info */
        get_tx_reader_ptr(card);
        cnt++;
        if (cnt < 10) {
            if (cnt > 2) {
                /* first retry need not wait */
                mdelay(cnt*10);
            }
            goto retry;
        }
        sdio_release_host(card->func);
        return -1;
    }

    ptr = (sdio_header*)data;
    if (debug_on)
        pr_err("%s() pkt seq_no : 0x%x\n", __func__, ptr->seq_no);
    if ((tx_buf_ptr + len) > tx_buf_end) {
        seg1 = tx_buf_ptr;
        size1 = tx_buf_end - tx_buf_ptr;
        seg2 = tx_buf_start;
        size2 = (tx_buf_ptr + len) - tx_buf_end;

        if (debug_on)
            pr_err("%s() transfer data (seg1)! tx_buf_ptr 0x%x transfer_len 0x%x\n", __func__, seg1, size1);
        err = sdio_transfer(card, data, seg1 - sdio_dma_base_addr, size1, 0);
        if (!err){
            if (debug_on)
                pr_err("%s() transfer data (seg2)! tx_buf_ptr 0x%x transfer_len 0x%x\n", __func__, seg2, size2);
            err = sdio_transfer(card, (data + size1), seg2 - sdio_dma_base_addr, size2, 0);
            if (!err) {
                transfer_len = size1 + size2;
                pkt_cnt++;
            }
        }
    } else {
        if (debug_on)
            pr_err("%s() transfer data! tx_buf_ptr 0x%x transfer_len 0x%x\n", __func__, tx_buf_ptr, len);
        err = sdio_transfer(card, data, tx_buf_ptr- sdio_dma_base_addr, len, 0);
        if (!err){
            transfer_len = len;
            pkt_cnt++;
        }
    }

    /* update local pointer for TX buffer producer */
    tx_buffer_update(card, transfer_len);

    if (commit) {        
        /* sync TX buffer producer to lynx */
        set_tx_producer_ptr(card, tx_buf_ptr);
        tx_commit(card);
    }
    sdio_release_host(card->func);
    return err;
}
#ifdef SKB_RING
static void t_buf_update_cons(struct if_sdio_card *card, u32 len)
{
    spin_lock(&card->t_buf_lock);

    card->t_buf_full_flag = 0;

    if ((card->cons + len) == card->t_buf_end) {
        card->cons = card->t_buf_start;
        //pr_err("%s() %d\n", __func__, __LINE__);
    } else if ((card->cons + len) > card->t_buf_end) {
        card->cons = card->t_buf_start +
            ((card->cons + len) - card->t_buf_end);
        //pr_err("%s() %d\n", __func__, __LINE__);
    } else {
        card->cons += len;
        //pr_err("%s() %d\n", __func__, __LINE__);
    }

    if (card->cons == card->prod)
        card->t_buf_empty_flag = 1;

    spin_unlock(&card->t_buf_lock);
}

static void t_buf_update_prod(struct if_sdio_card *card, u32 len)
{
    spin_lock(&card->t_buf_lock);

    card->t_buf_empty_flag = 0;

    if ((card->prod + len) == card->t_buf_end) {
        card->prod = card->t_buf_start;
        //pr_err("%s() %d\n", __func__, __LINE__);
    } else if ((card->prod + len) > card->t_buf_end) {
        card->prod = card->t_buf_start +
            ((card->prod + len) - card->t_buf_end);
        //pr_err("%s() %d\n", __func__, __LINE__);
    } else {
        card->prod += len;
        //pr_err("%s() %d\n", __func__, __LINE__);
    }

    if (card->prod == card->cons)
        card->t_buf_full_flag = 1;

    spin_unlock(&card->t_buf_lock);
}
static s32 t_buf_len_remain_cons(struct if_sdio_card *card)
{
    s32 remain = 0;

    spin_lock(&card->t_buf_lock);

    if (card->t_buf_empty_flag) {
        spin_unlock(&card->t_buf_lock);
        return 0;
    }

    if (card->prod == card->cons) {
        /* full */
        remain = card->t_buf_size;
    } else {
        if (card->prod > card->cons)
            remain = card->prod - card->cons;
        else
            remain = card->t_buf_size - (card->cons - card->prod);
    }

    spin_unlock(&card->t_buf_lock);

    return remain > 0? (u32)remain : (u32)0;
}
static s32 t_buf_len_remain_prod(struct if_sdio_card *card)
{
    s32 remain = 0;

    spin_lock(&card->t_buf_lock);

    if (card->t_buf_full_flag) {
        spin_unlock(&card->t_buf_lock);
        return 0;
    }

    if (card->prod == card->cons) {
        /* empty */
        remain = card->t_buf_size;
    } else {
        if (card->prod > card->cons)
            remain = card->t_buf_size - (card->prod - card->cons);
        else
            remain = card->cons - card->prod;
    }

    spin_unlock(&card->t_buf_lock);

    return remain > 0? (u32)remain : (u32)0;
}

static void t_buf_dump(struct if_sdio_card *card)
{
    pr_err("%s() t_buf_start 0x%p, t_buf_size %d\n", __func__, card->t_buf_start, card->t_buf_size);
    pr_err("%s() producer 0x%p, consumer 0x%p\n", __func__, card->prod, card->cons);
    pr_err("%s() full %d, empty %d\n", __func__, card->t_buf_full_flag, card->t_buf_empty_flag);
}
#endif
/*
 * host/card async function
 */
static int sdio_bulk_msg(struct if_sdio_card *card, int dir,
         void *data, int len, int *actual_length, int timeout)
{
    struct sdio_func *func = card->func;
    struct lynx_mpif_device *hdev = sdio_get_drvdata(func);
    int err = -EAGAIN, rc = 0, i, ret;
    sdio_header header;
    u8 *ptr;
    struct sk_buff *skb;
#ifdef SKB_RING
    u8 *seg1, *seg2;
    u32 len1, len2, remain;
#else
    u32 queue_cnt = 0;
    unsigned long flags;
#endif

    if (hdev->flags & HIF_SDIO_TX_STOP)
        return 0;

    if ((dir == SDIO_DIR_OUT) || (dir == SDIO_DIR_OUT_SYNC)) {
        /* prepare SDIO transfer header except the seq_no */
        header.payload_len = cpu_to_be16(len + SDIO_HDR_SIZE);
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

        if (dir == SDIO_DIR_OUT) {
#ifndef SKB_RING
            spin_lock_irqsave(&hdev->tx_lock, flags);
            queue_cnt = hdev->tx_skb_queue_cnt;
            spin_unlock_irqrestore(&hdev->tx_lock, flags);
            if (queue_cnt >= MAX_TX_QUEUE_NUM) {
                pr_debug("%s() can't add item to tx_queue anymore\n", __func__);
#ifdef USE_WMM_QUEUE
                err = -ENOMEM; //-ENOMEM;
#else
                err = -1;
#endif
                dev_kfree_skb(skb);
            } else {
                memcpy(skb_put(skb, SDIO_HDR_SIZE), &header, SDIO_HDR_SIZE);
                memcpy(skb_put(skb, len), data, len);

                pr_debug("%s() after cp data : head room %d, tail room %d\n", __func__,
                    skb_headroom(skb),
                    skb_tailroom(skb));

                spin_lock_irqsave(&hdev->tx_lock, flags);
                ((sdio_header*)(skb->data))->seq_no = tx_seq_no;
                tx_seq_no++;
                __skb_queue_tail(&hdev->tx_skb_queue, skb);
                hdev->tx_skb_queue_cnt++;
                spin_unlock_irqrestore(&hdev->tx_lock, flags);
                err = 0;

                queue_work(card->tx_workqueue, &card->tx_worker);
            }
#else
            /* set mutex rather than spinlock for memcpy CPU-bound operation */
            mutex_lock(&hdev->tx_mutex);
            remain = t_buf_len_remain_prod(card);
            seg1 = card->prod;
            len1 = card->t_buf_end - card->prod;

            if (SDIO_SIZE_ALIGN(len + SDIO_HDR_SIZE, card)
                <= remain) {
                /* copy transfer data to skb ring */
                pr_debug("%s() pkt: tx_seq_no 0x%x\n", __func__, tx_seq_no);
                header.seq_no= tx_seq_no;
                tx_seq_no++;
                if ((len + SDIO_HDR_SIZE) <= len1) {
                    memcpy(card->prod, &header, SDIO_HDR_SIZE);
                    memcpy(card->prod + SDIO_HDR_SIZE, data, len);
                } else {
                    if (len1 <= SDIO_HDR_SIZE) {
                        pr_err("%s() exception : len1 smaller than header\n", __func__);
                        t_buf_dump(card);
                    }
                    seg2 = card->t_buf_start;
                    len2 = card->prod + (len + SDIO_HDR_SIZE) - card->t_buf_end;
                    memcpy(seg1, &header, SDIO_HDR_SIZE);
                    memcpy(seg1 + SDIO_HDR_SIZE, data, len1-SDIO_HDR_SIZE);

                    memcpy(seg2, (u8*)data + (len1-SDIO_HDR_SIZE), len2);
                }
                t_buf_update_prod(card, SDIO_SIZE_ALIGN(len + SDIO_HDR_SIZE, card));
                err = 0;
                queue_work(card->tx_workqueue, &card->tx_worker);
            } else {
                /* not enough room for transfer data in skb ring */
                err = -1;
            }
            mutex_unlock(&hdev->tx_mutex);
            dev_kfree_skb(skb);
#endif
        }
        else {
            pr_debug("%s() pkt: tx_seq_no 0x%x\n", __func__, tx_seq_no);
            header.seq_no= tx_seq_no;
            tx_seq_no++;
            memcpy(skb_put(skb, SDIO_HDR_SIZE), &header, SDIO_HDR_SIZE);
            memcpy(skb_put(skb, len), data, len);

            pr_debug("%s() after cp data : head room %d, tail room %d\n", __func__,
                    skb_headroom(skb),
                    skb_tailroom(skb));

            //pr_err("%s:%d\n", __func__, __LINE__);
            err = if_sdio_host_to_card(card, skb->data, skb->len, true);
            dev_kfree_skb(skb);
        }

    }
    else if (dir == SDIO_DIR_IN) {
        set_bit(SDIO_RX_WAIT, &card->flag);
		//queue_work(card->rx_workqueue, &card->rx_worker);
#if 1//for disable SDIO rx irq
        sdio_claim_host(func);
        ret = sdio_claim_irq(card->func, if_sdio_interrupt);
        if (ret) {
            pr_err("%s() %d error\n", __func__, __LINE__);
        }
        sdio_release_host(func);
#endif
        rc = wait_event_interruptible_timeout(card->rx_wq, !test_bit(SDIO_RX_WAIT, &card->flag), timeout);
        if (rc == 0) {
            err = -ETIMEDOUT;
            sdio_claim_host(func);
            sdio_release_irq(card->func);
            sdio_release_host(func);
            clear_bit(SDIO_RX_WAIT, &card->flag);
            printk(KERN_INFO "%s:%d\n", __func__, __LINE__);
        }
        else if (rc < 0) {
            err = rc;
            sdio_claim_host(func);
            sdio_release_irq(card->func);
            sdio_release_host(func);
            clear_bit(SDIO_RX_WAIT, &card->flag);
            printk(KERN_INFO "%s:%d\n", __func__, __LINE__);
        }
        else {
            if (card->rx_buflen <= len) {
                //err = -ENOBUFS;
                err = 0;
                sdio_claim_host(func);
                sdio_release_irq(card->func);
                sdio_release_host(func);
                memcpy(data, card->rx_buffer, card->rx_buflen);
                printk(KERN_INFO "%s warning: buf len bigger than actcual data len.\n", __func__, __LINE__);
            }
            else {
                sdio_claim_host(func);
                sdio_release_irq(card->func);
                sdio_release_host(func);
                memcpy(data, card->rx_buffer, len);
                err = 0;
                printk(KERN_INFO "%s:%d\n", __func__, __LINE__);
            }
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
    struct lynx_mpif_device *hdev = (struct lynx_mpif_device *)hif;
#if 0
    struct if_sdio_card *card = mpdev_priv(hdev);
    struct sdio_func *func = card->func;
    int ret;

    ret = lynx_hw_init(hdev);
    if (ret) {
        dev_err(&func->dev, "%s: Unable to match lynx hw\n", __func__);
        return ret;
    }
#endif

    /* init member of hdev */
    spin_lock_init(&hdev->rx_lock);
    spin_lock_init(&hdev->tx_lock);
    spin_lock_init(&hdev->tx_recycle_lock);
    spin_lock_init(&hdev->sender_lock);
    spin_lock_init(&hdev->rx_compress_lock);
    mutex_init(&hdev->tx_mutex);
    __skb_queue_head_init(&hdev->rx_skb_queue);
    __skb_queue_head_init(&hdev->tx_skb_queue);
    __skb_queue_head_init(&hdev->sender_queue);
    __skb_queue_head_init(&hdev->tx_recycle_queue);
    __skb_queue_head_init(&hdev->rx_compress_queue);
    hdev->tx_skb_queue_cnt = 0;

    hdev->init = 1;

    return 0;
}

static void hif_sdio_deinit(void *hif)
{
    struct lynx_mpif_device *hdev = (struct lynx_mpif_device *)hif;

    if (!hdev->init)
        return;

    hdev->init = 0;
}

static void hif_sdio_start(void *hif)
{
    struct lynx_mpif_device *hdev = (struct lynx_mpif_device *)hif;
    unsigned long flags;

    spin_lock_irqsave(&hdev->tx_lock, flags);
    hdev->flags &= ~HIF_SDIO_TX_STOP;
    spin_unlock_irqrestore(&hdev->tx_lock, flags);
}

static void hif_sdio_stop(void *hif)
{
    struct lynx_mpif_device *hdev = (struct lynx_mpif_device *)hif;
    unsigned long flags;

    if (!hdev->init)
        return;

    spin_lock_irqsave(&hdev->tx_lock, flags);
    hdev->flags |= HIF_SDIO_TX_STOP;
    spin_unlock_irqrestore(&hdev->tx_lock, flags);
}

static int hif_sdio_send(void *hif, struct sk_buff *skb)
{
    struct lynx_mpif_device *hdev = (struct lynx_mpif_device *)hif;
    struct if_sdio_card *card = mpdev_priv(hdev);
    unsigned long flags;
    int err = 0;

    spin_lock_irqsave(&hdev->tx_lock, flags);

    if (hdev->flags & HIF_SDIO_TX_STOP) {
        spin_unlock_irqrestore(&hdev->tx_lock, flags);
        //lynx_dbg(LYNX_DBG_SDIO | LYNX_DBG_ERROR, "%s:%d Error(NODEV)\n", __func__, __LINE__);
        printk(KERN_INFO "%s:%d Error(NODEV)\n", __func__, __LINE__);
        return -ENODEV;
    }

    //lynx_dbg(LYNX_DBG_SDIO, "%s:%d tid=%d\n", __func__, __LINE__, SKB_TID(hdev, skb));
    printk(KERN_INFO "%s:%d tid=\n", __func__, __LINE__);
    lynx_dbg_dump(LYNX_DBG_SDIO, "hif_sdio_send", "tx ", skb->data, skb->len);

    spin_unlock_irqrestore(&hdev->tx_lock, flags);

    {
        int actual = 0;
        int transfer = 0;
        u8 *buf = skb->data;
        transfer = skb->len;
#if 0
        pr_err("%s() head room %ld, data room %ld, tail room %ld\n", __func__,
                (long int)(skb->data - skb->head),
                (long int)(skb->tail - skb->data),
                (long int)(skb->end - skb->tail));
#endif
        err = sdio_bulk_msg(card, SDIO_DIR_OUT, buf, transfer, &actual, 3000);
        if (err) {
            /* wmm will queue this skb when error returned */
            //dev_err(&card->func->dev, "%s:%d sdio_bulk_msg out failed!!(err %d)\n", __func__, __LINE__, err);
        } else {
            kfree_skb(skb);
        }
    }

    return err;
}

static int hif_sdio_recv(void *hif, struct sk_buff *skb)
{
    struct lynx_mpif_device *hdev = (struct lynx_mpif_device *)hif;
    struct if_sdio_card *card = mpdev_priv(hdev);

#ifndef SDIO_LOOPBACK_ENABLE
    if (test_bit(SDIO_ACTIVE, &card->flag)) {
        lynx_rx_tasklet((unsigned long)hdev);
    }
    else
#endif
    {
        struct sk_buff *skb = NULL;
        unsigned long flags;
        while (1) {
            spin_lock_irqsave(&hdev->rx_lock, flags);
            skb = __skb_dequeue(&hdev->rx_skb_queue);
            spin_unlock_irqrestore(&hdev->rx_lock, flags);
            if(skb == NULL)
                break;
#ifdef SDIO_LOOPBACK_ENABLE
            card->lb_rx_data_num += skb->len;
#endif
            kfree_skb(skb);
        }
#ifndef SDIO_LOOPBACK_ENABLE
        {
            struct sdio_func *func = card->func;
            dev_err(&func->dev, "%s:%d sdio recv packet, but not active!!\n", __func__, __LINE__);
        }
#endif
    }

    return 0;
}

	int mpif_sdio_init (void *hif_handle) {return 0;}
	void mpif_sdio_deinit (void *hif_handle) {return;}
	void mpif_sdio_start (void *hif_handle) {return;}
	void mpif_sdio_stop (void *hif_handle) {return;}
	int mpif_sdio_send (void *hif_handle, struct sk_buff *buf) {return 0;}
	int mpif_sdio_recv (void *hif_handle, struct sk_buff *buf) {return 0;}

static struct lynx_mpif_ops hif_sdio = {
    .name   = "lynx_mpif_sdio",

    .init   = mpif_sdio_init,
    .deinit = mpif_sdio_deinit,
    .start  = mpif_sdio_start,
    .stop   = mpif_sdio_stop,
    .send   = mpif_sdio_send,
    .recv   = mpif_sdio_recv,
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

static int if_sdio_get_fw_verion(struct if_sdio_card *card)
{
    struct sdio_func *func = card->func;
    struct lynx_mpif_device *hdev = sdio_get_drvdata(func);
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
    err = sdio_bulk_msg(card, SDIO_DIR_OUT_SYNC, buf, transfer, 0, 3000);
    if (err) {
        dev_err(&func->dev, "%s:%d \n", __func__, __LINE__);
        err = -EIO;
        goto err_fw;
    }

    err = sdio_bulk_msg(card, SDIO_DIR_IN, buf, transfer, &actual, 3000);
    if (err < 0 || actual == 0) {
        dev_err(&func->dev, "%s:%d\n", __func__, __LINE__);
        err = -EIO;
        goto err_fw;
    }
    if (actual < sizeof(struct lynx_wci_hdr) + 4) {
        dev_err(&func->dev, "%s:%d info not enough!\n", __func__, __LINE__);
        err = -EIO;
        goto err_fw;
    }

    hdev->fw_version = be32_to_cpu(*(u32 *)(buf+sizeof(struct lynx_wci_hdr)));

    kfree(buf);

    dev_info(&func->dev, "lynx firmware version: 0x%x\n", hdev->fw_version);

    return 0;

err_fw:
    if (buf)
        kfree(buf);
    dev_err(&func->dev, "%s:%d Can't get firmware infomation!\n", __func__, __LINE__);
    return err;
}

static int lynx_sdio_download_fw(struct lynx_mpif_device *hdev)
{
    struct if_sdio_card *card = mpdev_priv(hdev);
    struct sdio_func *func = card->func;
    struct lynx_wci_hdr lynx_cmd = {
         .cmd_id = WCI_H2D_FW_DOWNLOAD, .seq_no = 0, .payload_len = 0
    };
    const void *data = hdev->firmware->data;
    size_t len = hdev->firmware->size;
    int transfer = 0, err = 0;
    u8 *buf = os_api_alloc(512, GFP_KERNEL);

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

    dev_info(&func->dev, "lynx firmware download: Transferred FW: %s, size: %ld\n",
         hdev->fw_name, (unsigned long) hdev->firmware->size);
    /*
     * wait for firmware complete
     */
    msleep(3000);
    
    return 0;

err_fw:
    if (buf)
        kfree(buf);
    return err;
}

/* FIXME: update firmware feature */
#ifdef LYNX_WAIT_DOWNLOAD_FIRMWARE
static void lynx_sdio_firmware_fail(struct lynx_mpif_device *hdev)
{
    struct if_sdio_card *card = mpdev_priv(hdev);
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
    struct lynx_mpif_device *hdev = context;
    struct if_sdio_card *card = mpdev_priv(hdev);
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
#if 0
static int lynx_sdio_switch_on(struct lynx_mpif_device *hdev)
{
    /* init usb device */
    if(mpif_device_init(hdev))
        return -ENODEV;

    if(mpif_device_start(hdev))
	{
		mpif_device_deinit(hdev);
        return -ENODEV;
	}
	return 0;
}
static int lynx_sdio_switch_off(struct lynx_mpif_device *hdev)
{
    if (hdev) {
        hif_device_stop(hdev);
        hif_device_deinit(hdev);
    }
    return 0;
}
#endif
static int lynx_sdio_device_detached(struct lynx_mpif_device *hdev)
{
    struct if_sdio_card *card = mpdev_priv(hdev);
    struct sk_buff *skb;
    unsigned long flags;

    //lynx_dbg(LYNX_DBG_WARN, "%s(): hdev=0x%x\n", __FUNCTION__, (uintptr_t)hdev);
    printk(KERN_INFO "%s(): hdev=0x%x\n", __FUNCTION__, (uintptr_t)hdev);

    if (hdev) {
        set_bit(SDIO_REMOVE, &card->flag);
        clear_bit(SDIO_ACTIVE, &card->flag);
#if 0
        lynx_sdio_switch_off(hdev);
#endif
#if 0
        if (hdev->lynx) {
            lynx_core_cleanup(hdev->lynx);
			os_dep_deinit(hdev->lynx);
        }
#endif
        cancel_work_sync(&card->rx_worker);
        flush_workqueue(card->rx_workqueue);
        destroy_workqueue(card->rx_workqueue);
        cancel_work_sync(&card->tx_worker);
        flush_workqueue(card->tx_workqueue);
        destroy_workqueue(card->tx_workqueue);

        spin_lock_irqsave(&hdev->rx_lock, flags);
        while (1) {
            if (!(skb = __skb_dequeue(&hdev->rx_skb_queue)))
		    	break;
            kfree_skb(skb);
        }
        spin_unlock_irqrestore(&hdev->rx_lock, flags);
        spin_lock_irqsave(&hdev->tx_lock, flags);
        while (1) {
            if (!(skb = __skb_dequeue(&hdev->tx_skb_queue)))
		    	break;
            kfree_skb(skb);
            hdev->tx_skb_queue_cnt--;
        }
        spin_unlock_irqrestore(&hdev->tx_lock, flags);
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
    }
    return 0;
}

static int if_sdio_prog_firmware(struct if_sdio_card *card)
{
    struct sdio_func *func = card->func;
    struct lynx_mpif_device *hdev = sdio_get_drvdata(func);
    int ret = 0;

    if ((ret = if_sdio_get_fw_info(card)))
        return ret;


    card->rom_ver = if_sdio_is_boot_rom(card);
    if (card->rom_ver) {

        //dev_info(&func->dev, "Lynx [boot mode] now attached\n");
		printk(KERN_CRIT "Lynx [boot mode] now attached\n");

        if_sdio_get_fw_verion(card);

        init_completion(&hdev->fw_done);

        /* Assign which firmware to load */

        if(card->rom_ver == SDIO_TUPLE_BOOT_ROM3)
            hdev->fw_name = FIRMWARE_LYNX_3_0_0;
        else
            hdev->fw_name = FIRMWARE_LYNX_2_0_0;

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
#if 0//def LYNX_WAIT_DOWNLOAD_FIRMWARE
		printk(KERN_INFO "%s:%d\n", __func__, __LINE__);
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
		printk(KERN_CRIT "Lynx Download Firmware START.\n");
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
#endif

		printk(KERN_CRIT "Lynx Download Firmware DONE.\n");
        dev_info(&func->dev, "%s: Firmware %s download OK\n", __func__, hdev->fw_name);
        dev_info(&func->dev, "%s: Re-initialize!!\n", __func__);
        ret = if_sdio_get_fw_info(card);
        if (ret) {
            dev_err(&func->dev, "%s: Re-initialize for firmware %s failed\n", __func__, hdev->fw_name);
            goto err_fw;
        }

        dev_info(&func->dev, "%s: Firmware %s requested\n", __func__, hdev->fw_name);
    }
    else {
        dev_info(&func->dev, "Lynx [app mode] now attached\n");
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
#if 0//for disable SDIO rx irq
    ret = sdio_claim_irq(func, if_sdio_interrupt);
    if (ret) {
        pr_err("%s() %d error\n", __func__, __LINE__);
        goto release_func;
    }
#endif

    //lynx_dbg(LYNX_DBG_SDIO, "register irq OK\n");
    //printk(KERN_INFO "register irq OK\n");

    sdio_release_host(func);

    ret = if_sdio_prog_firmware(card);
    if (ret)
        goto release_irq;

    return 0;

release_irq:
    sdio_claim_host(func);
    //pr_err("%s() %d going to release irq\n", __func__, __LINE__);
//  sdio_release_irq(func);
//release_func:
    sdio_disable_func(func);
release:
    sdio_release_host(func);
    return ret;
}

static int if_sdio_power_off(struct if_sdio_card *card)
{
    struct sdio_func *func = card->func;

    sdio_claim_host(func);
    pr_info("%s() %d going to release irq\n", __func__, __LINE__);
    sdio_release_irq(func);
    sdio_disable_func(func);
    sdio_release_host(func);

    return 0;
}


/*******************************************************************/
/* SDIO callbacks                                                  */
/*******************************************************************/
static void if_sdio_interrupt(struct sdio_func *func)
{
    //struct lynx_hif_device *hdev = sdio_get_drvdata(func);
    struct lynx_mpif_device *hdev = sdio_get_drvdata(func);
    struct if_sdio_card *card;
    int err;
    struct sk_buff *skb;
    u8* buf;
    u32 seg1, seg2, size1, size2, transfer_len, bytes, pkt_cnt = 0;
 
    pr_err("%s()\n", __func__);
 
    if (!hdev)
        return;
 
    card = mpdev_priv(hdev);
    //card = hdev_priv(hdev);
    if (!card)
        return;
 
    if (!test_bit(SDIO_REMOVE, &card->flag)) {
        pr_err("%s() queue rx worker\n", __func__);
#if 0
        queue_work(card->rx_workqueue, &card->rx_worker);
#else
 
        while (1) {
            //pr_err("%s() start to collect compressed skb\n", __func__);
            get_rx_producer_ptr(card);
            bytes = rx_buffer_len_remain(card);
            if (bytes == 0) {
                //pr_err("%s() rx is empty\n", __func__);
                break;
            }
            //rx_buffer_dump();
            skb = dev_alloc_skb(bytes);
            buf = skb_put(skb, bytes);
 
            transfer_len = 0;
            if ((rx_buf_reader_ptr + bytes) > rx_buf_end) {
                seg1 = rx_buf_reader_ptr;
                size1 = rx_buf_end - rx_buf_reader_ptr;
                seg2 = rx_buf_start;
                size2 = (rx_buf_reader_ptr + bytes) - rx_buf_end;
 
                //pr_err("%s() transfer data (seg1)! rx_buf_reader_ptr 0x%x transfer_len 0x%x\n", __func__, seg1, size1);
                err = sdio_transfer(card, buf, seg1 - sdio_dma_base_addr, size1, 1);
                if (!err){
                    //pr_err("%s() transfer data (seg2)! rx_buf_reader_ptr 0x%x transfer_len 0x%x\n", __func__, seg2, size2);
                    err = sdio_transfer(card, (buf + size1), seg2 - sdio_dma_base_addr, size2, 1);
                    if (!err) {
                        transfer_len = size1 + size2;
                        pkt_cnt++;
                    } else {
                        pr_err("%s() %d err %d\n", __func__, __LINE__, err);
                    }
 
                } else {
                    pr_err("%s() %d err %d\n", __func__, __LINE__, err);
                }
            } else {
                //pr_err("%s() transfer data! rx_buf_reader_ptr 0x%x transfer_len 0x%x\n", __func__, rx_buf_reader_ptr, bytes);
                err = sdio_transfer(card, buf, rx_buf_reader_ptr- sdio_dma_base_addr, bytes, 1);
                if (!err){
                    transfer_len = bytes;
                    pkt_cnt++;
                } else {
                    pr_err("%s() %d err %d\n", __func__, __LINE__, err);
                }
            }
 
            /* update local pointer for TX buffer producer */
            if (transfer_len) {
                rx_buffer_update(card, transfer_len);
                err = set_rx_consumer_ptr(card, rx_buf_reader_ptr);
                if (err)
                    pr_err("%s() %d err %d\n", __func__, __LINE__, err);
                err = rx_commit(card);
                if (err)
                    pr_err("%s() %d err %d\n", __func__, __LINE__, err);
            }
            //rx_buffer_dump();
 
            /* queue data */
            if (!err) {
                spin_lock(&hdev->rx_compress_lock);
                __skb_queue_tail(&hdev->rx_compress_queue, skb);
                spin_unlock(&hdev->rx_compress_lock);
                queue_work(card->rx_decompress_workqueue, &card->rx_decompress_worker);
            } else {
                pr_err("%s() recevice error (err : %d)\n", __func__, err);
                dev_kfree_skb(skb);
                break;
            }
        }
 
        if (!pkt_cnt)
            pr_err("%s() there is a unnecessary interrupt that inform us with no data\n", __func__);
 
#endif
    }
}


static void  sdio_rx_decompress_workqueue(struct work_struct *work)
{
    struct if_sdio_card *card = container_of(work, struct if_sdio_card, rx_decompress_worker);
    struct sdio_func *func;
    struct lynx_mpif_device *hdev;
    unsigned long flags;
    struct sk_buff *compressed_skb;
    u8 *data, *ptr;
    sdio_header* hdr;
    struct sk_buff *skb;
    int debug_cnt = 0, i;
    u8 cksum;

    if (!card)
        return;

    func = card->func;
    hdev = sdio_get_drvdata(card->func);

    while (1) {
        spin_lock_irqsave(&hdev->rx_compress_lock, flags);
        compressed_skb = __skb_dequeue(&hdev->rx_compress_queue);
        spin_unlock_irqrestore(&hdev->rx_compress_lock, flags);
        if (!compressed_skb) {
            pr_err("%s() no compressed skb\n", __func__);
            break;
        } else {
            pr_err("%s() get a compressed skb\n", __func__);
        }

        data = compressed_skb->data;
        debug_cnt = 0;
        while (data < skb_tail_pointer(compressed_skb)) {
            u32 len;
            if (debug_cnt > 200) {
                pr_err("%s() compressed_skb contains abnormal number of skb\n", __func__);
                break;
            }
            pr_err("%s() loop of processing compressed_skb, data %p skb_tail %p\n",
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

            pr_err("%s() payload id 0x%x, seq_no 0x%x, len %d\n", __func__, hdr->cksum, hdr->seq_no, len);
            if (card->info.sdio_cksum_enable) {
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

            /* check sequential number */
            if ((((int)hdr->seq_no - (int)last_rx_seq_no) != 1) && (((int)hdr->seq_no - (int)last_rx_seq_no) != -255) ){
                pr_err("%s() dis-continuous seq number: seq_no 0x%x last_seq_no 0x%x\n",
                    __func__, hdr->seq_no, last_rx_seq_no);
                rx_buffer_dump();
            }

            /* queue skb for precessing */
            if (test_bit(SDIO_RX_WAIT, &card->flag)) {
                pr_err("%s() copy firmware data\n", __func__);
                card->rx_buflen = len - SDIO_HDR_SIZE;
                memcpy(card->rx_buffer, data + SDIO_HDR_SIZE , len - SDIO_HDR_SIZE);
                clear_bit(SDIO_RX_WAIT, &card->flag);
                wake_up(&card->rx_wq);
            } else if (test_bit(SDIO_ACTIVE, &card->flag)) {
                pr_err("%s() copy to hif\n", __func__);
                skb = dev_alloc_skb(len - SDIO_HDR_SIZE);
                if (!skb) {
                    hdev->rx_skb_alloc_fail_cnt++;
                    pr_err("%s:%d Error(NOMEM) RX alloc fail count (%d)\n", __func__, __LINE__, hdev->rx_skb_alloc_fail_cnt);
                } else {
					int cc;
                    memcpy(skb_put(skb, len - SDIO_HDR_SIZE), data + SDIO_HDR_SIZE, len - SDIO_HDR_SIZE);
#if 1
                    /*
                     * process your data here then free it because you don't
                     * have hif anymore to process it
                     */
					printk(KERN_INFO "skb data[len: %d]:", len - SDIO_HDR_SIZE);
					for(cc = 0; cc < len - SDIO_HDR_SIZE; cc++)
						printk(KERN_INFO " %x", *(((unsigned char *)skb->data + cc)));
					printk(KERN_INFO "\n");
                    /* To do : process your data here */

                    /* To do : free your data */
                    dev_kfree_skb(skb);
#else
                    spin_lock(&hdev->rx_lock);
                    __skb_queue_tail(&hdev->rx_skb_queue, skb);
                    spin_unlock(&hdev->rx_lock);
#endif
                }
            } else {
                pr_err("%s() copy to rx_dummy\n", __func__);
                memcpy(card->rx_dummy, data + SDIO_HDR_SIZE, len - SDIO_HDR_SIZE);
            }
            last_rx_seq_no = hdr->seq_no;
            
            /* forward to next skb */
            data += SDIO_SIZE_ALIGN(len, card);
            pr_err("%s() forward to next skb (after updated : data 0x%p, skb_tail 0x%p)\n",
                __func__, data, skb_tail_pointer(compressed_skb));

            /* for debugging */
            debug_cnt ++;
        }

        dev_kfree_skb(compressed_skb);

#if 0
        if (skb_queue_len(&hdev->rx_skb_queue) > 0)
            hif_device_recv(hdev, 0);
#endif
    }

}

static void sdio_tx_recycle_workqueue(struct work_struct *work)
{
    struct if_sdio_card *card = container_of(work, struct if_sdio_card, tx_recycle_worker);
    struct sdio_func *func;
    struct lynx_mpif_device *hdev;
    unsigned long flags;
    struct sk_buff *skb;

    if (!card)
        return;

    func = card->func;
    hdev = sdio_get_drvdata(card->func);

    while (1) {
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
            hdev->sdio_tx_callback(hdev, skb, 0, true);
#else
            dev_kfree_skb(skb);
#endif
        }

    }
}

static void sdio_rx_workqueue(struct work_struct *work)
{
    struct if_sdio_card *card = container_of(work, struct if_sdio_card, rx_worker);
    int retry = 5;
    int delay_ms = 10;

    if (!card)
        return;

    while(if_sdio_card_to_host(card) && (retry)) {
        pr_err("%s() error retry of sdio read (cnt %d delay_ms %d)\n", __func__, retry, delay_ms);
        retry--;
        mdelay(delay_ms);
    }
}

#ifdef SKB_RING
static void sdio_tx_workqueue(struct work_struct *work)
{
    struct if_sdio_card *card = container_of(work, struct if_sdio_card, tx_worker);
    struct sdio_func *func;
    struct lynx_mpif_device *hdev;
    int err = 0;
    u32 buf_remain = 0;
    u32 trans_remain = 0;

    if (!card)
        return;
    func = card->func;
    hdev = sdio_get_drvdata(card->func);

    /* foresee the remain len of lynx if appropriate number skb can be dequeue */
    //get_tx_reader_ptr(card);
    buf_remain = tx_buffer_len_remain(card);

GET_SKB_RING:
    trans_remain = t_buf_len_remain_cons(card);
    if (!trans_remain)
        return;

    /* polling lynx if tx buffer is not enough */
    if (!buf_remain) {
        do {
            get_tx_reader_ptr(card);
            buf_remain = tx_buffer_len_remain(card);
        } while (!buf_remain);
    }

    if (trans_remain > buf_remain) {
        trans_remain = buf_remain;
        buf_remain = 0;
    } else {
        buf_remain -= trans_remain;
    }

    sdio_claim_host(card->func);
    if ((card->cons + trans_remain) <= card->t_buf_end) {
        err = if_sdio_host_to_card(card, card->cons, trans_remain, false);
        if (err) {
            dev_err(&func->dev, "%s: sdio_tx fail and drop pkt!\n", __func__);
        }
    } else {
        u8* seg1 = card->cons;
        u8* seg2 = card->t_buf_start;
        u32 len1 = card->t_buf_end - card->cons;
        u32 len2 = trans_remain - len1;
        if (len1 & (card->info.size_align-1))
            pr_err("%s() len1 not aligned\n", __func__);

        if (len2 & (card->info.size_align-1))
            pr_err("%s() len2 not aligned\n", __func__);

        err = if_sdio_host_to_card(card, seg1, len1, false);
        if (err) {
            dev_err(&func->dev, "%s: sdio_tx fail and drop pkt!\n", __func__);
        }
        err = if_sdio_host_to_card(card, seg2, len2, false);
        if (err) {
            dev_err(&func->dev, "%s: sdio_tx fail and drop pkt!\n", __func__);
        }
    }
    /* sync TX buffer producer to lynx */
    set_tx_producer_ptr(card, tx_buf_ptr);
    tx_commit(card);
    sdio_release_host(card->func);

    /* update skb ring */
    t_buf_update_cons(card, trans_remain);
    goto GET_SKB_RING;

}
#endif

struct lynx_mpif_device *lynx_alloc_mpdev(size_t priv_size)
{
    struct lynx_mpif_device *hdev;
    
    hdev = (struct lynx_mpif_device *)os_api_alloc(sizeof(struct lynx_mpif_device) + priv_size, GFP_KERNEL);

    return (hdev) ? hdev : NULL;
}

void lynx_free_mpdev(struct lynx_mpif_device *hdev)
{
    if (hdev)
        kfree(hdev);
}

static int if_sdio_probe(struct sdio_func *func,
        const struct sdio_device_id *id)
{
    struct lynx_mpif_device *hdev;
    struct if_sdio_card *card;
    struct lynx *lynx = NULL;
    int vendor_id, product_id;
    int ret = 0;
    int i;

    last_rx_seq_no = 0xff;
    tx_seq_no = 0;

    vendor_id = func->vendor;
    product_id = func->device;

    //lynx_dbg(LYNX_DBG_SDIO, "sdio new func %d vendor 0x%x device 0x%x block 0x%x/0x%x\n",
    printk(KERN_INFO "sdio new func %d vendor 0x%x device 0x%x block 0x%x/0x%x\n",
            func->num, func->vendor, func->device,
            func->max_blksize, func->cur_blksize);

	printk(KERN_INFO "%s:%d\n", __func__, __LINE__);
    pr_info("\nfunc info start:\n");
    pr_info("card supported clock %d\n", func->card->cis.max_dtr);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,13,0)
	printk(KERN_INFO "%s:%d\n", __func__, __LINE__);
    //pr_info("class 0x%x vendor 0x%x, device 0x%x, max_blk_size 0x%x cur_blksize 0x%x\n"
    printk(KERN_INFO "class 0x%x vendor 0x%x, device 0x%x, max_blk_size 0x%x cur_blksize 0x%x\n"
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
        //lynx_dbg(LYNX_DBG_SDIO, "info[%d]=%s\n", i, func->card->info[i]);
        printk(KERN_INFO "info[%d]=%s\n", i, func->card->info[i]);
    }

    if (func->card->num_info == 0) {
        pr_err("unable to identify card model\n");
        return -ENODEV;
    }

    func->card->quirks |= MMC_QUIRK_LENIENT_FN0;

    hdev = lynx_alloc_mpdev(sizeof(struct if_sdio_card));
    if(hdev == NULL) {
        ret = -ENOMEM;
        goto err_alloc_hdev;
    }
    hdev->vendor_id = vendor_id;
    hdev->device_id = product_id;
    hdev->hif_ops = &hif_sdio;
    hdev->hif_type = LYNX_MPIF_TYPE_SDIO;

    card = mpdev_priv(hdev);

#ifdef CONFIG_PROC_FS
    sdio_card = card;
#endif

    card->func = func;

    card->rx_buffer = kmalloc(MAX_RX_BUF_SIZE, GFP_KERNEL);
    if(!card->rx_buffer) {
        pr_err("%s() failed to allocate rx buffer\n", __func__);
        ret = -ENOMEM;
        goto err_alloc_hdev;
    }

	//printk(KERN_INFO "%s:%d\n", __func__, __LINE__);
    /* t_buf data structure */
    //card->t_buf_start = kmalloc(1*1024*1024, GFP_KERNEL);
    card->t_buf_start = kmalloc(32*1024, GFP_KERNEL);
    //card->t_buf_size = 1*1024*1024;
    card->t_buf_size = 32*1024;
    if(!card->t_buf_start) {
        pr_err("%s() failed to allocate rx buffer\n", __func__);
        ret = -ENOMEM;
        goto err_alloc_hdev;
    }
    card->prod = card->t_buf_start;
    card->cons = card->t_buf_start;
    card->t_buf_end = card->t_buf_start + card->t_buf_size;
    card->t_buf_full_flag = 0;
    card->t_buf_empty_flag = 1;
    spin_lock_init(&card->t_buf_lock);

	printk(KERN_INFO "%s:%d\n", __func__, __LINE__);

    INIT_LIST_HEAD(&card->packets);
    init_waitqueue_head(&card->rx_wq);

    card->rx_workqueue = create_workqueue("lynx_sdio_rx");
    INIT_WORK(&card->rx_worker, sdio_rx_workqueue);
    card->tx_workqueue = create_workqueue("lynx_sdio_tx");
    INIT_WORK(&card->tx_worker, sdio_tx_workqueue);
    card->tx_recycle_workqueue = create_workqueue("lynx_sdio_tx_recycle");
    INIT_WORK(&card->tx_recycle_worker, sdio_tx_recycle_workqueue);
    card->rx_decompress_workqueue = create_workqueue("lynx_sdio_rx_decompress");
    INIT_WORK(&card->rx_decompress_worker, sdio_rx_decompress_workqueue);

    /* pre-init member of hdev for hdev not ready */
    spin_lock_init(&hdev->rx_lock);
    spin_lock_init(&hdev->tx_lock);
    spin_lock_init(&hdev->tx_recycle_lock);
    spin_lock_init(&hdev->sender_lock);
    spin_lock_init(&hdev->rx_compress_lock);
    mutex_init(&hdev->tx_mutex);
    __skb_queue_head_init(&hdev->rx_skb_queue);
    __skb_queue_head_init(&hdev->tx_skb_queue);
    __skb_queue_head_init(&hdev->sender_queue);
    __skb_queue_head_init(&hdev->tx_recycle_queue);
    __skb_queue_head_init(&hdev->rx_compress_queue);
    hdev->tx_skb_queue_cnt = 0;

    sdio_set_drvdata(func, hdev);

    ret = if_sdio_power_on(card);//Lynx download firmware
    if (ret)
        goto err_activate_card;

    set_bit(SDIO_ACTIVE, &card->flag);

#if 0//def SDIO_LOOPBACK_ENABLE
    hif_sdio_init(hdev);
    hif_sdio_start(hdev);
    dev_info(&func->dev, "%s Ready for sdio loopback test\n", __func__);
    return ret;
#endif
//goto err_activate_card;

#if 0
    /* init wlan interface */
    lynx = lynx_core_create();
    if (lynx == NULL) {
        lynx_dbg(LYNX_DBG_ERR, "Failed to alloc lynx core\n");
        ret = -ENOMEM;
        goto err_hdev_destroy;
    }
#endif
	//printk(KERN_INFO "%s:%d\n", __func__, __LINE__);
    //hdev->lynx = lynx;
    //lynx->hif_priv = hdev;

#if 0
    ret = lynx_sdio_switch_on(hdev);
    if(ret) {
        dev_err(&func->dev, "%s: lynx sdio switch on failed\n", __func__);
        goto err_lynx_core;
    }

    ret = lynx_core_init(lynx);
    if (ret) {
        dev_err(&func->dev, "%s: lynx core init failed\n", __func__);
        /* FIXME: Does it work? Are there other resource needed to be free? */
        goto err_lynx_core;
    }
#endif
out:
    //lynx_dbg(LYNX_DBG_SDIO, "%s ret=%d\n", __func__, ret);
    printk(KERN_INFO "Lynx SDIO Host driver probe done. (%s:%d)\n", __func__,  __LINE__);

    return ret;

err_activate_card:

err_lynx_core:
    lynx_sdio_device_detached(hdev);
err_hdev_destroy:
    if (hdev->firmware) {
        release_firmware(hdev->firmware);
        hdev->firmware = NULL;
    }
    if_sdio_power_off(card);
    lynx_free_mpdev(hdev);
    hdev = NULL;
#ifdef CONFIG_PROC_FS
    sdio_card = NULL;
#endif
	printk(KERN_INFO "%s:%d\n", __func__, __LINE__);
    sdio_set_drvdata(func, NULL);
err_alloc_hdev:

	printk(KERN_INFO "%s:%d\n", __func__, __LINE__);
    goto out;
}

static void if_sdio_remove(struct sdio_func *func)
{
    struct lynx_mpif_device *hdev = sdio_get_drvdata(func);
    struct if_sdio_card *card = mpdev_priv(hdev);

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

    lynx_sdio_device_detached(hdev);
    if_sdio_power_off(card);
    if(card->rx_buffer) {
        kfree(card->rx_buffer);
    }
    if(card->t_buf_start) {
        kfree(card->t_buf_start);
    }
    lynx_free_mpdev(hdev);
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
//    struct lynx_mpif_device *hdev = sdio_get_drvdata(func);
//    struct if_sdio_card *card = mpdev_priv(hdev);
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
//    struct lynx_mpif_device *hdev = sdio_get_drvdata(func);
//    struct if_sdio_card *card = mpdev_priv(hdev);
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
#if 0
static int strtoul(char *str, void *v)
{   
    return(sscanf(str,"%d", (unsigned *)v)==1);
}

static int hextoul(char *str, void *v)
{   
    return(sscanf(str,"%x", (unsigned *)v)==1);
}
#endif

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

static ssize_t proc_sdio_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	if(sdio_card == NULL)
		return 0;

	int ret = 0, rev = 0, err = 0, argc, tlen;
	char* argv[MAX_ARGV];
	unsigned char *start, *p;
	unsigned char *sdio_data = os_api_alloc(128, GFP_KERNEL);
	unsigned char cmd_buf[128] = "";
	struct lynx_wci_hdr lynx_cmd = {
		.cmd_id = WCI_H2D_MP_TEST_CMD,
		.seq_no = 0,
		.payload_len = strlen(wt_cmd) + 1,//string end '\0'
	};

	if(strlen(wt_cmd) > 3)
	{
		memcpy(sdio_data, &lynx_cmd, sizeof(struct lynx_wci_hdr));
		memcpy(sdio_data + sizeof(struct lynx_wci_hdr), wt_cmd, strlen(wt_cmd));

		tlen = lynx_cmd.payload_len + sizeof(struct lynx_wci_hdr);
		//printk(KERN_INFO "Lynx SDIO bulk out msg(len: %d) %s\n", tlen, sdio_data + sizeof(struct lynx_wci_hdr));

		err = sdio_bulk_msg(sdio_card, SDIO_DIR_OUT_SYNC, sdio_data, tlen, 0, 500);
		if(err)
		{
			printk(KERN_CRIT "Lynx SDIO send wt command Fail! %d\n", err);
			goto err_bulk;
		}
	}
	//strcpy(cmd_buf, wt_cmd);
	//printk(KERN_INFO "%s:%d\n", __func__, __LINE__);
	argc = get_args((const char *)wt_cmd, argv);

	if((argc == 2) && ((!strcmp("stat", argv[1])) || (!strcmp("bbcnt", argv[1]))))
	{
#if 1
		//printk(KERN_INFO "Lynx SDIO bulk in msg\n");
		//memset(cmd_buf, 0, 128);
		err = sdio_bulk_msg(sdio_card, SDIO_DIR_IN, cmd_buf, sizeof(cmd_buf), 0, 3000);//500
		if(err)
		{
			printk(KERN_CRIT "Lynx SDIO receive wt command result Fail! %d\n", err);
			goto err_bulk;
		}
		printk(KERN_INFO "Lynx SDIO receive:\n%s\n", cmd_buf + sizeof(struct lynx_wci_hdr));
		rev = 1;
#else
		printk(KERN_INFO "Lynx SDIO receive:\n");
#endif
	}

	if(!(start = os_api_alloc(128, GFP_KERNEL)))
		goto err_bulk;
	p = start;
	if(rev)
		p += sprintf(p, "%s\n", cmd_buf + sizeof(struct lynx_wci_hdr));

	ret = simple_read_from_buffer(buf, count, ppos, start, strlen(start));

err_bulk:
	kfree(sdio_data);
	kfree(start);
	return ret;
}

static ssize_t proc_sdio_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
	if(sdio_card == NULL)
		return count;
	memset(wt_cmd, 0, 256);

	if(count > 0 && count < 255)
	{
		if(copy_from_user(wt_cmd, buffer, count))
			return -EFAULT;
		wt_cmd[count-1] = '\0';
		printk(KERN_CRIT "%s", wt_cmd);
	}
	return count;
}

static const struct file_operations sdio_proc_fops = {
	.read		= proc_sdio_read,
	.write		= proc_sdio_write,
	.owner		= THIS_MODULE,
};
#endif

static int __init if_sdio_init_module(void)
{
    int ret = 0;

#ifdef CONFIG_PROC_FS
    struct proc_dir_entry *res;

    if(!(res = proc_create("lynx_dut_sdio", S_IWUSR | S_IRUGO, NULL, &sdio_proc_fops)))
        return -ENOMEM;
#endif

    printk(KERN_CRIT "Montage SDIO: Lynx DUT SDIO driver load!!\n");

    ret = sdio_register_driver(&if_sdio_driver);

    (void) sdio_hexdump;
    (void) sdio_test_rate;

    return ret;
}

static void __exit if_sdio_exit_module(void)
{
#ifdef CONFIG_PROC_FS
    remove_proc_entry("lynx_dut_sdio", NULL);
#endif
    sdio_unregister_driver(&if_sdio_driver);

    printk(KERN_CRIT "Montage SDIO: Lynx DUT SDIO driver unload!!\n");
}

module_init(if_sdio_init_module);
module_exit(if_sdio_exit_module);

module_param(user_mac_addr, charp, 0644);

MODULE_AUTHOR("Montage");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Lynx SDIO driver for DUT");
MODULE_FIRMWARE(FIRMWARE_LYNX_2_0_0);
MODULE_FIRMWARE(FIRMWARE_LYNX_3_0_0);
MODULE_DEVICE_TABLE(sdio, if_sdio_ids);
