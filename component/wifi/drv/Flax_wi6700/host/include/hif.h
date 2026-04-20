/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Acrospeed Inc. All right reserved.                                           |
|                                                                              |
+=============================================================================*/
/*! 
*   \file 
*   \brief
*   \author Acrospeed
*/

#ifndef __HIF_H__
#define __HIF_H__

#ifdef CONFIG_LYNX_OS_LINUX
#include <linux/interrupt.h>
#include <linux/ip.h>
#endif

enum lynx_hif_type {
    LYNX_HIF_TYPE_SDIO,
    LYNX_HIF_TYPE_USB,
};

struct lynx_hif_ops {
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

#ifndef CONFIG_LYNX_OS_LINUX
struct firmware {
	int size;
	unsigned char *data;
};
#endif

typedef void (*tx_cb)(void *target, struct sk_buff *skb, u8 id, int txok);

struct lynx_hif_device {
//    struct lynx_device *own;
    u16 vendor_id;
    u16 device_id;

    u32 fw_version;
    const char *fw_name;
    const struct firmware *firmware;
    struct completion fw_done;
    struct lynx_hif_ops *hif_ops;
    enum lynx_hif_type hif_type;
    struct lynx *lynx;

#if defined(CONFIG_LYNX_W2SDIO) 
    struct mutex tx_mutex;
    OS_LOCK_TYPE tx_sdio_lock;
#endif
    OS_LOCK_TYPE tx_lock;
    OS_LOCK_TYPE rx_lock;
    OS_LOCK_TYPE tx_recycle_lock;
    OS_LOCK_TYPE sender_lock;

    u8 init;
    u8 flags;
    u16 tx_buf_cnt;
    u32 rx_skb_alloc_fail_cnt;
    struct sk_buff_head tx_skb_queue;
    struct sk_buff_head sender_queue;
    struct sk_buff_head tx_recycle_queue;

#define MAX_TX_QUEUE_NUM    512
    tx_cb sdio_tx_callback;

    struct list_head tx_buf;
    struct list_head tx_pending;
    struct sk_buff_head rx_skb_queue;
    struct tasklet_struct rx_tasklet;

#if defined(CONFIG_LYNX_W2SDIO) 
    struct sk_buff_head rx_compress_queue;
    spinlock_t rx_compress_lock;
#endif

    unsigned long       private[0] ____cacheline_aligned;
};

int hif_device_init(void *hif_handle);
void hif_device_deinit(void *hif_handle);
int hif_device_start(void *hif_handle);
void hif_device_stop(void *hif_handle);
int hif_device_send(void *hif_handle, struct sk_buff *buf);
int hif_device_recv(void *hif_handle, struct sk_buff *buf);
int hif_tx_lock(void *hif_handle, unsigned long *flags);
int hif_tx_unlock(void *hif_handle, unsigned long *flags);

void lynx_free_hdev(struct lynx_hif_device *hdev);

struct lynx_hif_device *lynx_alloc_hdev(int priv_size);
static inline void *hdev_priv(struct lynx_hif_device *hdev)
{
    if (hdev)
        return (void *)hdev->private;
    else
        return 0;
}

#endif
