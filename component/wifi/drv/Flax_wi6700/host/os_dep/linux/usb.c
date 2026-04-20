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
#include <linux/types.h>
#if !defined(CONFIG_LYNX_WM_MANAGER)
#include <net/mac80211.h>
#endif

#ifdef CONFIG_LYNX_W2USB_FIRMWARE_ARRAY
#ifdef CONFIG_LYNX_ROM3
#include "firmware_array_3.h"
#endif /*CONFIG_LYNX_ROM3*/

#ifdef CONFIG_LYNX_ROM2
#include "firmware_array_2.h"
#endif /*CONFIG_LYNX_ROM2*/

#ifdef CONFIG_MP_FW

#ifdef CONFIG_LYNX_ROM3
#include "mp_firmware_array_3.h"
#endif /*CONFIG_LYNX_ROM3*/

#ifdef CONFIG_LYNX_ROM2
#include "mp_firmware_array_2.h"
#endif /*CONFIG_LYNX_ROM2*/

#endif /*CONFIG_MP_FW*/
#endif /*CONFIG_LYNX_W2USB_FIRMWARE_ARRAY*/

#include "lynx_rev.h"
#include "wlan_def.h"
#include "init.h"
#include "core.h"
#include "wci.h"
#include "hif.h"
#include "hw.h"
#include "usb.h"
#include "lynx_debug.h"
#include "vfc.h"

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
/* identify firmware images */
#define FIRMWARE_LYNX_2_0_0     "lynx/app_2.img"
#define FIRMWARE_LYNX_3_0_0     "lynx/app_3.img"
//#define LYNX_WAIT_DOWNLOAD_FIRMWARE

/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
static struct usb_device_id lynx_usb_ids[] = {
	//{ USB_DEVICE(0x1234, 0x3282) }, /* lynx */
#ifdef CONFIG_LYNX_ROM2
	{ USB_DEVICE(USB_VID_MONTAGE, USB_PID_LYNX_ROM2) }, /* lynx */
#endif /*CONFIG_LYNX_ROM2*/
#ifdef CONFIG_LYNX_ROM3
	{ USB_DEVICE(USB_VID_MONTAGE, USB_PID_LYNX_ROM3) }, /* lynx */
#endif/*CONFIG_LYNX_ROM3*/
	{ USB_DEVICE(USB_VID_MONTAGE, USB_PID_LYNX_FPGA) }, /* lynx */
    { },
};

#ifdef CONFIG_SUPPORT_MONITOR_MODE
static unsigned int monitor_level = 0;
module_param(monitor_level, uint, 0644);
MODULE_PARM_DESC(monitor_level, " Monitor level 0-5 : (default:0)\n" \
				"                (0: Monitor data frame ; 1: Monitor beacon/probe_req frame ;\n" \
				"                (2: Monitor mgmt frame  ; 3: Monitor all ; 4 : OMNICFG ; 5 : OMNICFG (with beacon)\n");
#endif /*CONFIG_SUPPORT_MONITOR_MODE */

extern unsigned int mp_test_firm;

/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/

/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/
extern void lynx_rx_tasklet(unsigned long data);

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/

/******************* hif function ***********************/

void hif_txcompletion_cb(struct lynx_hif_device *hdev, struct sk_buff *skb, bool txok)
{
    struct lynx_usb_device *usb = hdev_priv(hdev);
    struct lynx_hw *hw = &usb->hw;
    enum lynx_ep_id epid = hw->usb.gettx_epid(VFC_TX_AC0); // lynx only use END_POINT_1

    if(hw->usb.ep[epid].ep_callbacks.tx)
        hw->usb.ep[epid].ep_callbacks.tx(hdev, skb, epid, txok);
}

static inline void hif_skb_queue_purge(struct lynx_hif_device *hdev, struct sk_buff_head *queue)
{
    struct sk_buff *skb;

    while ((skb = __skb_dequeue(queue)) != NULL) {
        dev_kfree_skb_any(skb);
    }
}

static inline void hif_skb_queue_complete(struct lynx_hif_device *hdev,
                        struct sk_buff_head *queue, bool txok)
{
    struct sk_buff *skb;

    while ((skb = __skb_dequeue(queue)) != NULL) {
        hif_txcompletion_cb(hdev, skb, txok);
    }
}

static void hif_usb_tx_cb(struct urb *urb)
{
    struct tx_buf *tx_buf = (struct tx_buf *) urb->context;
    struct lynx_hif_device *hdev;
    bool txok = true;

    if (!tx_buf || !tx_buf->hdev)
        return;

    hdev = tx_buf->hdev;
    lynx_dbg(LYNX_DBG_USB_TX, "!!!! status:%d\n",urb->status);

    switch (urb->status) {
    case 0:
        break;
    case -ENOENT:
    case -ECONNRESET:
    case -ENODEV:
    case -ESHUTDOWN:
        txok = false;

        /*
         * If the URBs are being flushed, no need to add this
         * URB to the free list.
         */

        os_api_lock(&hdev->tx_lock);
        if (hdev->flags & HIF_USB_TX_FLUSH) {
            os_api_unlock(&hdev->tx_lock);
            hif_skb_queue_purge(hdev, &tx_buf->skb_queue);
            return;
        }
        os_api_unlock(&hdev->tx_lock);
        break;
    default:
        txok = false;
        break;
    }

	if (txok == false)
		lynx_dbg((LYNX_DBG_USB_TX | LYNX_DBG_ERR), "!!!! %s: status:%d\n",__func__, urb->status);

    hif_skb_queue_complete(hdev, &tx_buf->skb_queue, txok);

    /* Re-initialize the SKB queue */
    tx_buf->len = tx_buf->offset = 0;
    __skb_queue_head_init(&tx_buf->skb_queue);

    /* Add this TX buffer to the free list */
    os_api_lock(&hdev->tx_lock);
    list_move_tail(&tx_buf->list, &hdev->tx_buf);
    hdev->tx_buf_cnt++;
    os_api_unlock(&hdev->tx_lock);
}

static void hif_usb_rx_cb(struct urb *urb)
{
	struct sk_buff *skb = (struct sk_buff *) urb->context;
	struct lynx_hif_device *hdev = usb_get_intfdata(usb_ifnum_to_if(urb->dev, 0));
	struct lynx_usb_device *usb = hdev_priv(hdev);
	struct sk_buff *new = NULL;
	int ret=0;

	lynx_dbg(LYNX_DBG_USB_RX, "%s(1), skb=%p, hdev=%p, urb->status=%d, urb->actual_length=%d\n",
			__FUNCTION__, (void *)skb, (void *)hdev, urb->status, urb->actual_length);

	if (!skb)
		return;

	if (!hdev)
		goto free;

	switch (urb->status) {
		case 0:
			break;
		case -ENOENT:
		case -ECONNRESET:
		case -ENODEV:
		case -ESHUTDOWN:
			goto free;
		default:
			goto resubmit;
    }

	lynx_dbg_dump(LYNX_DBG_USB_RX, "hif_usb_rx_cb", "rx ", skb->data, skb->len);

	if(likely(urb->actual_length != 0)) 
	{
		skb_put(skb, urb->actual_length);

		new = alloc_skb(MAX_RX_BUF_SIZE, GFP_ATOMIC);
		if(!new)
		{
			hdev->rx_skb_alloc_fail_cnt++;
			lynx_dbg((LYNX_DBG_USB_RX | LYNX_DBG_ERR), "%s:%d Error(NOMEM) RX alloc fail count (%d)\n", __func__, __LINE__, hdev->rx_skb_alloc_fail_cnt);
		}
		else
		{
			hif_device_recv(hdev, skb);
			skb = new;
			urb->transfer_buffer = skb->data;
			urb->context = skb;
		}
    }

resubmit:
	skb_reset_tail_pointer(skb);
	skb_trim(skb, 0);

	usb_anchor_urb(urb, &usb->rx_submitted);
	ret = usb_submit_urb(urb, GFP_ATOMIC);
	if (ret) {
		usb_unanchor_urb(urb);
		goto free;
	}

	return;
free:
	kfree_skb(skb);
}

static void hif_usb_dealloc_tx_urbs(struct lynx_hif_device *hdev)
{
    struct tx_buf *tx_buf = NULL, *tx_buf_tmp = NULL;
    unsigned long flags;

    os_api_isr_lock(&hdev->tx_lock, &flags);
    hdev->flags |= HIF_USB_TX_FLUSH;
    os_api_isr_unlock(&hdev->tx_lock, &flags);

    list_for_each_entry_safe(tx_buf, tx_buf_tmp,
                 &hdev->tx_buf, list) {
        usb_kill_urb(tx_buf->urb);
        list_del(&tx_buf->list);
        usb_free_urb(tx_buf->urb);
        os_api_free(tx_buf->buf);
        os_api_free(tx_buf);
    }

    list_for_each_entry_safe(tx_buf, tx_buf_tmp,
                 &hdev->tx_pending, list) {
        usb_kill_urb(tx_buf->urb);
        list_del(&tx_buf->list);
        usb_free_urb(tx_buf->urb);
        os_api_free(tx_buf->buf);
        os_api_free(tx_buf);
    }
}

static int hif_usb_alloc_tx_urbs(struct lynx_hif_device *hdev)
{
	struct tx_buf *tx_buf;
	int i;

	INIT_LIST_HEAD(&hdev->tx_buf);
	INIT_LIST_HEAD(&hdev->tx_pending);
	os_api_lock_init(&hdev->tx_lock);
	__skb_queue_head_init(&hdev->tx_skb_queue);

	for (i = 0; i < MAX_TX_URB_NUM; i++) 
	{
		tx_buf = (struct tx_buf *)os_api_alloc(sizeof(struct tx_buf), GFP_KERNEL);
		if (!tx_buf)
			goto err;

		tx_buf->buf = os_api_alloc(MAX_TX_BUF_SIZE, GFP_KERNEL);
		if (!tx_buf->buf)
			goto err;

		tx_buf->urb = usb_alloc_urb(0, GFP_KERNEL);
		if (!tx_buf->urb)
			goto err;

		tx_buf->hdev = hdev;
		__skb_queue_head_init(&tx_buf->skb_queue);

		list_add_tail(&tx_buf->list, &hdev->tx_buf);
	}

	hdev->tx_buf_cnt = MAX_TX_URB_NUM;

    return 0;
err:
	if(tx_buf) 
	{
		os_api_free(tx_buf->buf);
		os_api_free(tx_buf);
	}
	hif_usb_dealloc_tx_urbs(hdev);
	return -ENOMEM;
}

static void hif_usb_dealloc_rx_urbs(struct lynx_hif_device *hdev)
{
    struct lynx_usb_device *usb = hdev_priv(hdev);
    usb_kill_anchored_urbs(&usb->rx_submitted);
}

static int hif_usb_alloc_rx_urbs(struct lynx_hif_device *hdev)
{
    struct lynx_usb_device *usb = hdev_priv(hdev);
    struct lynx_hw *hw = &usb->hw;
    struct urb *urb = NULL;
    struct sk_buff *skb = NULL;
    enum lynx_ep_id epid;
    int i, ret;

    init_usb_anchor(&usb->rx_submitted);
    os_api_lock_init(&hdev->rx_lock);
    __skb_queue_head_init(&hdev->rx_skb_queue);

	for (i = 0; i < MAX_RX_URB_NUM; i++) {

		/* Allocate URB */
		urb = usb_alloc_urb(0, GFP_KERNEL);
		if (urb == NULL) {
			ret = -ENOMEM;
			goto err_urb;
		}

        /* Allocate buffer */
        skb = alloc_skb(MAX_RX_BUF_SIZE, GFP_KERNEL);
		if (!skb) {
			ret = -ENOMEM;
			goto err_skb;
		}

		epid = hw->usb.getrx_epid(VFC_RX_MCU);

		usb_fill_bulk_urb(urb, usb->udev,
                  usb_rcvbulkpipe(usb->udev, epid),
                  skb->data, MAX_RX_BUF_SIZE,
                  hif_usb_rx_cb, skb);

        /* Anchor URB */
		usb_anchor_urb(urb, &usb->rx_submitted);

        /* Submit URB */
		ret = usb_submit_urb(urb, GFP_KERNEL);
		if (ret) 
		{
			usb_unanchor_urb(urb);
			goto err_submit;
		}

        /*
         * Drop reference count.
         * This ensures that the URB is freed when killing them.
         */
        usb_free_urb(urb);
    }

    return 0;

err_submit:
	kfree_skb(skb);
err_skb:
	usb_free_urb(urb);
err_urb:
	hif_usb_dealloc_rx_urbs(hdev);
	return ret;
}

static int hif_usb_alloc_urbs(struct lynx_hif_device *hdev)
{
    /* TX */
    if (hif_usb_alloc_tx_urbs(hdev) < 0)
        goto err;

    /* RX */
    if (hif_usb_alloc_rx_urbs(hdev) < 0)
        goto err_rx;

    return 0;
err_rx:
    hif_usb_dealloc_tx_urbs(hdev);
err:
    return -ENOMEM;
}

static void hif_usb_dealloc_urbs(struct lynx_hif_device *hdev)
{
    hif_usb_dealloc_tx_urbs(hdev);
    hif_usb_dealloc_rx_urbs(hdev);
}

static int hif_usb_init(void *hif)
{
    struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif;
    struct lynx_usb_device *usb = hdev_priv(hdev);
    int ret;

	tasklet_init(&hdev->rx_tasklet, lynx_rx_tasklet, (unsigned long)hdev);

	ret = lynx_hw_init(hdev);
	if (ret) {
		dev_err(usb->dev,
            "hif_usb_init: Unable to match lynx hw\n");
		goto err;
	}

    /* Alloc URBs */
    ret = hif_usb_alloc_urbs(hdev);
    if (ret) {
        dev_err(usb->dev,
            "hif_usb_init: Unable to allocate URBs\n");
		goto err;
    }


    hdev->init = 1;

    return 0;

err:
    tasklet_kill(&hdev->rx_tasklet);
	
	return ret;
}

static void hif_usb_deinit(void *hif)
{
    struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif;

    if (!hdev->init)
        return;

    hif_usb_dealloc_urbs(hdev);

    tasklet_kill(&hdev->rx_tasklet);

    hdev->init = 0;
}

static void hif_usb_start(void *hif)
{
    struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif;
    unsigned long flags;

    os_api_isr_lock(&hdev->tx_lock, &flags);
    hdev->flags &= ~HIF_USB_TX_STOP;
    os_api_isr_unlock(&hdev->tx_lock, &flags);
}

static void hif_usb_stop(void *hif)
{
    struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif;
    struct tx_buf *tx_buf = NULL, *tx_buf_tmp = NULL;
    unsigned long flags;

    if (!hdev->init)
        return;

    os_api_isr_lock(&hdev->tx_lock, &flags);
    hdev->flags |= HIF_USB_TX_STOP;
    os_api_isr_unlock(&hdev->tx_lock, &flags);

	hif_skb_queue_complete(hdev, &hdev->tx_skb_queue, false);

    /* The pending URBs have to be canceled. */
    list_for_each_entry_safe(tx_buf, tx_buf_tmp,
        &hdev->tx_pending, list) {
        usb_kill_urb(tx_buf->urb);
    }
}

static int hif_usb_send(void *hif, struct sk_buff *skb)
{
	struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif;
	struct lynx_usb_device *usb = hdev_priv(hdev);
	struct lynx_hw *hw = &usb->hw;
	struct tx_buf *tx_buf = NULL;
#if 0
	enum lynx_ep_id epid = hw->usb.gettx_epid(SKB_TID(hdev, skb));
#else
	enum lynx_ep_id epid = 1;
#endif
	u8 *buf;
	unsigned long flags;
	int ret = 0;

    os_api_isr_lock(&hdev->tx_lock, &flags);

    if (hdev->flags & HIF_USB_TX_STOP) {
        lynx_dbg((LYNX_DBG_USB_TX | LYNX_DBG_ERR), "%s:%d Error(NODEV)\n", __func__, __LINE__);
        ret = -ENODEV;
		goto exit;
    }

    /* Check if a free TX buffer is available */
    if (list_empty(&hdev->tx_buf)) {
        lynx_dbg((LYNX_DBG_USB_TX | LYNX_DBG_ERR), "%s:%d Error(NOMEM)\n", __func__, __LINE__);
        ret = -ENOMEM;
		goto exit;
    }

    /* Check if a valid endpoint id is available */
    if (hw->usb.valid_epid(epid)) {
        lynx_dbg((LYNX_DBG_USB_TX | LYNX_DBG_ERR), "%s:%d Error(NXIO) epid=%d\n", __func__, __LINE__, epid);
        ret = -ENXIO;
		goto exit;
    }

    lynx_dbg_dump(LYNX_DBG_USB_TX, "hif_usb_send", "tx ", skb->data, skb->len);

    /* FIXME: we do not use lynx_cmd_header and lynx_cmd in lynx-2.0 */
    lynx_dbg_dump(LYNX_DBG_USB_TX, "AFTER FIX ENDAIN", "tx ", skb->data, skb->len);

    tx_buf = list_first_entry(&hdev->tx_buf, struct tx_buf, list);
    list_move_tail(&tx_buf->list, &hdev->tx_pending);
    hdev->tx_buf_cnt--;

    buf = tx_buf->buf;

    tx_buf->len = skb->len;
    memcpy(buf, skb->data, skb->len);

    __skb_queue_tail(&tx_buf->skb_queue, skb);

#ifdef CONFIG_LYNX_DEBUG
	if (epid)
        lynx_dbg(LYNX_DBG_USB_TX, "!!!! (%d) usb_submit_urb epid(%d)\n",__LINE__, epid);
#endif

    usb_fill_bulk_urb(tx_buf->urb, usb->udev,
              usb_sndbulkpipe(usb->udev, epid),
              tx_buf->buf, tx_buf->len,
              hif_usb_tx_cb, tx_buf);

    if ((tx_buf->len % usb->hw.usb.ep[epid].max_packet_size) == 0) {
        /* hit a max packet boundary on this pipe */
        tx_buf->urb->transfer_flags |= URB_ZERO_PACKET;
    }
    else
    {
        tx_buf->urb->transfer_flags = 0;
    }

    ret = usb_submit_urb(tx_buf->urb, GFP_ATOMIC);
    if (ret) {
        lynx_dbg(LYNX_DBG_USB_TX, "!!!! usb_submit_urb fail %d:%d\n",__LINE__, ret);

		os_api_isr_unlock(&hdev->tx_lock, &flags);
        tx_buf->len = tx_buf->offset = 0;
        hif_skb_queue_complete(hdev, &tx_buf->skb_queue, false);
        __skb_queue_head_init(&tx_buf->skb_queue);
		os_api_isr_lock(&hdev->tx_lock, &flags);

        list_move_tail(&tx_buf->list, &hdev->tx_buf);
        hdev->tx_buf_cnt++;
    }

exit:
    os_api_isr_unlock(&hdev->tx_lock, &flags);

    return ret;
}

static int hif_usb_recv(void *hif, struct sk_buff *skb)
{
    struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif;
    struct lynx_usb_device *usb = hdev_priv(hdev);
    struct lynx_hw *hw = &usb->hw;
    enum lynx_ep_id epid = hw->usb.gettx_epid(VFC_RX_MCU); // lynx only use END_POINT_1

    lynx_dbg(LYNX_DBG_USB_RX,"usb epid:%d rx-hook:%s\n", epid, hw->usb.ep[epid].ep_callbacks.rx ? "YES" : "NULL");

    if(hw->usb.ep[epid].ep_callbacks.rx)
    {
        hw->usb.ep[epid].ep_callbacks.rx(hdev, skb, epid);
    }
    else
    {
        kfree_skb(skb);
    }
    return 0;
}

static int hif_usb_tx_lock(void *hif, unsigned long *flags)
{
	struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif;

	if(flags)
		os_api_isr_lock(&hdev->tx_lock, flags);
	else
		os_api_lock(&hdev->tx_lock);

    return 0;
}

static int hif_usb_tx_unlock(void *hif, unsigned long *flags)
{
	struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif;

	if(flags)
		os_api_isr_unlock(&hdev->tx_lock, flags);
	else
		os_api_unlock(&hdev->tx_lock);

	return 0;
}

static struct lynx_hif_ops hif_usb = {
    .name   = "lynx_hif_usb",

    .init   = hif_usb_init,
    .deinit = hif_usb_deinit,
    .start  = hif_usb_start,
    .stop   = hif_usb_stop,
    .send   = hif_usb_send,
    .recv   = hif_usb_recv,
    .tx_lock   = hif_usb_tx_lock,
    .tx_unlock   = hif_usb_tx_unlock,
};

/******************* hif function ***********************/

static int lynx_usb_get_fw_verion(struct lynx_hif_device *hdev)
{
    struct lynx_usb_device *usb = hdev_priv(hdev);
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
    err = usb_bulk_msg(usb->udev, usb_sndbulkpipe(usb->udev, 0x01),
                  buf, transfer, 0, 3000);
    if (err) {
        err = -EIO;
        goto err_fw;
    }

    err = usb_bulk_msg(usb->udev,
                       usb_rcvbulkpipe(usb->udev, 0x01),
                       buf, 512, &actual, 3000);
    if (err < 0 || actual == 0) {
        err = -EIO;
        goto err_fw;
    }
    if (actual < sizeof(struct lynx_wci_hdr) + 4) {
        dev_err(usb->dev, "%s:%d info not enough!\n", __func__, __LINE__);
        err = -EIO;
        goto err_fw;
    }

    hdev->fw_version = be32_to_cpu(*(u32 *)(buf+sizeof(struct lynx_wci_hdr)));

    os_api_free(buf);

    dev_info(usb->dev, "Lynx ROM version: 0x%x\n", hdev->fw_version);

    return 0;

err_fw:
    if (buf)
        os_api_free(buf);
    dev_err(usb->dev, "%s:%d Can't get Lynx ROM version !\n", __func__, __LINE__);
    return err;
}

static int lynx_usb_download_fw(struct lynx_hif_device *hdev)
{
	struct lynx_usb_device *usb = hdev_priv(hdev);
	struct lynx_wci_hdr lynx_cmd = {
         .cmd_id = WCI_H2D_FW_DOWNLOAD, .seq_no = 0, .payload_len = 0
    };
	int transfer = 0, err = 0;
	u8 *buf = NULL;

	const void *data ;
	size_t len ;

#ifndef CONFIG_LYNX_W2USB_FIRMWARE_ARRAY
	data = hdev->firmware->data;
	len = hdev->firmware->size;
#else

#ifdef CONFIG_LYNX_ROM3
	if(hdev->fw_version == 3){
		data = lynx_static_firmware_3;
		len = FIRMWARE_ARRAY_SIZE_3;
	}
#endif /*CONFIG_LYNX_ROM3*/
	
#ifdef CONFIG_LYNX_ROM2
	if(hdev->fw_version == 2){
		data = lynx_static_firmware_2;
		len = FIRMWARE_ARRAY_SIZE_2;
	}
#endif /*CONFIG_LYNX_ROM2*/


	if(mp_test_firm)
	{
#ifdef CONFIG_MP_FW

#ifdef CONFIG_LYNX_ROM3
	if(hdev->fw_version == 3){
		data = lynx_static_mp_firmware_3;
		len = MP_FIRMWARE_ARRAY_SIZE_3;
	}
#endif /*CONFIG_LYNX_ROM3*/

#ifdef CONFIG_LYNX_ROM2
	if(hdev->fw_version == 2){
		data = lynx_static_mp_firmware_2;
		len = MP_FIRMWARE_ARRAY_SIZE_2;
	}
#endif /*CONFIG_LYNX_ROM2*/
	
#else
        err = -EIO;
        return err;
#endif /*!CONFIG_MP_FW*/
	}
#endif /*CONFIG_LYNX_W2USB_FIRMWARE_ARRAY*/

	if((buf = os_api_alloc(512, GFP_KERNEL)) == NULL) 
	{
		err = -ENOMEM;
		goto err_fw;
	}

    lynx_cmd.payload_len = cpu_to_be16((len / 512) + ((len % 512) ? 1 : 0));

    memcpy(buf, &lynx_cmd, sizeof(struct lynx_wci_hdr));
    transfer = sizeof(struct lynx_wci_hdr);
    err = usb_bulk_msg(usb->udev, usb_sndbulkpipe(usb->udev, 0x01), 
                  buf, transfer, 0, 3000);
    if (err) {
        err = -EIO;
        goto err_fw;
    }

	while(len > 0)
	{
		if(len >= 512)
			transfer = 512;
		else
			transfer = len;

        memcpy(buf, data, transfer);

        err = usb_bulk_msg(usb->udev, usb_sndbulkpipe(usb->udev, 0x01), 
                      buf, transfer, 0, 3000);
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
    err = usb_bulk_msg(usb->udev, usb_sndbulkpipe(usb->udev, 0x01), 
                  buf, transfer, 0, 3000);
    if (err) {
        err = -EIO;
        goto err_fw;
    }

	os_api_free(buf);

#ifndef CONFIG_LYNX_W2USB_FIRMWARE_ARRAY
    dev_info(usb->dev, "lynx firmware download: Transferred FW: %s, size: %ld\n",
        hdev->fw_name, (unsigned long) hdev->firmware->size);
#else

	if(mp_test_firm){
#ifdef CONFIG_MP_FW
#ifdef CONFIG_LYNX_ROM2
			if(hdev->fw_version == 2){
				dev_info(usb->dev, "7000u mp	firmware download: Transferred FW: %s, rev: %d, size: %ld\n",
					"static array", LYNX_MP_FW_REV_2, (unsigned long)MP_FIRMWARE_ARRAY_SIZE_2);
			}
#endif /*CONFIG_LYNX_ROM2*/
#ifdef CONFIG_LYNX_ROM3
			if(hdev->fw_version == 3){
				dev_info(usb->dev, "6700u mp	firmware download: Transferred FW: %s, rev: %d, size: %ld\n",
					"static array", LYNX_MP_FW_REV_3, (unsigned long)MP_FIRMWARE_ARRAY_SIZE_3);
			}
#endif /*CONFIG_LYNX_ROM3*/
	}else{
#endif /*CONFIG_MP_FW*/


#ifdef CONFIG_LYNX_ROM2
			if(hdev->fw_version == 2){
				dev_info(usb->dev, "7000u firmware download: Transferred FW: %s, rev: %d, size: %ld\n",
					"static array", LYNX_FW_REV_2, (unsigned long)FIRMWARE_ARRAY_SIZE_2);
			}
#endif /*CONFIG_LYNX_ROM2*/
#ifdef CONFIG_LYNX_ROM3
				if(hdev->fw_version == 3){
					dev_info(usb->dev, "6700u firmware download: Transferred FW: %s, rev: %d, size: %ld\n",
						"static array", LYNX_FW_REV_3, (unsigned long)FIRMWARE_ARRAY_SIZE_3);
				}
#endif /*CONFIG_LYNX_ROM3*/
	}
#endif /*CONFIG_LYNX_W2USB_FIRMWARE_ARRAY*/
	return 0;

err_fw:
	if(buf)
		os_api_free(buf);
	return err;
}

/* FIXME: update firmware feature */
#ifdef LYNX_WAIT_DOWNLOAD_FIRMWARE
static void lynx_usb_firmware_fail(struct lynx_hif_device *hdev)
{
    struct lynx_usb_device *usb = hdev_priv(hdev);
    struct device *parent = usb->dev->parent;

    if (parent)
        device_lock(parent);

    device_release_driver(usb->dev);

    if (parent)
        device_unlock(parent);

    dev_err(usb->dev,
            "lynx: lynx_usb_firmware_cb failed\n");
}

static void lynx_usb_firmware_cb(const struct firmware *fw, void *context)
{
    struct lynx_hif_device *hdev = context;
    struct lynx_usb_device *usb = hdev_priv(hdev);
    int ret;

    if (!fw) {
        dev_err(usb->dev, "lynx: Failed to get firmware %s\n", hdev->fw_name);
        goto err_fw;
    }

    hdev->firmware = fw;
    ret = lynx_usb_download_fw(hdev);
    if (ret) {
        dev_err(usb->dev, "lynx: Firmware - %s download failed\n", hdev->fw_name);
        goto err_fw;
    }

err_no:
    if (hdev->firmware)
        release_firmware(hdev->firmware);
    hdev->firmware = NULL;

    complete(&hdev->fw_done);

    return;

err_fw:
    lynx_usb_firmware_fail(hdev);
    goto err_no;
}
#endif

static int lynx_usb_switch_on(struct lynx_hif_device *hdev)
{
    /* init usb device */
    if(hif_device_init(hdev))
        return -ENODEV;

    if(hif_device_start(hdev))
	{
		hif_device_deinit(hdev);
        return -ENODEV;
	}

	return 0;
}

static int lynx_usb_switch_off(struct lynx_hif_device *hdev)
{
    if (hdev) {
        hif_device_stop(hdev);
        hif_device_deinit(hdev);
    }
    return 0;
}

static int lynx_usb_device_detached(struct lynx_hif_device *hdev)
{
    unsigned long flags;

	/* FIXME: set HIF_USB_TX_STOP to avoid that USB tx is requested */
    os_api_isr_lock(&hdev->tx_lock, &flags);
    hdev->flags |= HIF_USB_TX_STOP;
    os_api_isr_unlock(&hdev->tx_lock, &flags);

    lynx_dbg(LYNX_DBG_WARN, "%s(): hdev=0x%x\n", __FUNCTION__, (unsigned long)hdev);

	if(hdev) 
	{
		if(hdev->lynx) 
		{
			lynx_core_cleanup(hdev->lynx);
			wci_progress_check(1, 0);
			os_dep_deinit(hdev->lynx);
		}

		lynx_usb_switch_off(hdev);
	}

    return 0;
}

/*======================================================================================+
 |                                                                                      |
 |                                  USB OPS                                             |
 |                                                                                      |
 +=====================================================================================*/
static int firmware_loaded = 0;

static int lynx_usb_probe(struct usb_interface *interface,
                        const struct usb_device_id *id)
{
    struct usb_device *udev = interface_to_usbdev(interface);
    struct lynx_hif_device *hdev;
    struct lynx_usb_device *usb;
    struct lynx *lynx = NULL;
    int vendor_id, product_id;
    int ret = 0;
    
    usb_get_dev(udev);
    
    vendor_id = le16_to_cpu(udev->descriptor.idVendor);
    product_id = le16_to_cpu(udev->descriptor.idProduct);

    lynx_dbg(LYNX_DBG_USB_PROBE, "vendor id = %04x product id = %04x\n", vendor_id, product_id);
    
    /* FIXME:create new func */
    hdev = lynx_alloc_hdev(sizeof(struct lynx_usb_device));
    if(hdev == NULL) {
        ret = -ENOMEM;
        goto err_usb_put;
    }
    usb_set_intfdata(interface, hdev);
    hdev->vendor_id = vendor_id;
    hdev->device_id = product_id;
    hdev->hif_ops = &hif_usb;
    hdev->hif_type = LYNX_HIF_TYPE_USB;

    usb = hdev_priv(hdev);
    usb->interface = interface;
    usb->id = id;
    usb->udev = udev;
    usb->dev = &udev->dev;

#ifdef CONFIG_PM
    udev->reset_resume = 1;
#endif

	/* let wci can work */
	wci_progress_check(1, 1);

    if (le16_to_cpu(udev->descriptor.bcdDevice) & 0xFF) {
        dev_info(usb->dev, "Lynx [boot mode] now attached\n");

        lynx_usb_get_fw_verion(hdev);

        init_completion(&hdev->fw_done);

        /* Assign which firmware to load */

        if(hdev->fw_version == 3)
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
#ifdef LYNX_WAIT_DOWNLOAD_FIRMWARE
        ret = request_firmware_nowait(THIS_MODULE, true, hdev->fw_name,
            usb->dev, GFP_KERNEL,
            hdev, lynx_usb_firmware_cb);
        if (ret) {
            dev_err(usb->dev,
                "lynx_usb_probe: Async request for firmware %s failed\n",
                hdev->fw_name);
            goto err_hdev_destroy;
        }
        wait_for_completion(&hdev->fw_done);
        if (hdev->firmware) {
            release_firmware(hdev->firmware);
            hdev->firmware = NULL;
        }
#else
#ifndef CONFIG_LYNX_W2USB_FIRMWARE_ARRAY
        ret = request_firmware(&hdev->firmware, hdev->fw_name, usb->dev);
        if (ret) {
            dev_err(usb->dev,
                "lynx_usb_probe: request for firmware %s failed\n",
                hdev->fw_name);
            goto err_hdev_destroy;
        }
        lynx_usb_download_fw(hdev);
        release_firmware(hdev->firmware);
        hdev->firmware = NULL;
#else
        lynx_usb_download_fw(hdev);
        hdev->firmware = NULL;
#endif
#endif

        firmware_loaded = 1 ;
        dev_info(usb->dev, "lynx_usb_probe: Firmware %s requested\n",
            hdev->fw_name);

		/* FIXME: 20151118: mark this for the exception when device reboot */
        //mdelay(3000); // after firmware download, wait for device ready to service
#ifdef CONFIG_LYNX_W2USB_NO_RESET
		/* Lynx would not do USB port reset after firmware downloading */
		dev_info(usb->dev, "Lynx new [app mode] now attached\n");
		udev->descriptor.bcdDevice = cpu_to_le16(0x00);

		lynx = lynx_core_create();
		if (lynx == NULL) {
			lynx_dbg(LYNX_DBG_ERR, "Failed to alloc lynx core\n");
			ret = -ENOMEM;
			goto err_hdev_destroy;
		}

		hdev->lynx = lynx;
		lynx->hif_priv = hdev;

		ret = lynx_usb_switch_on(hdev);
		if(ret) {
			dev_err(usb->dev, "lynx_usb_probe: lynx usb switch on failed\n");
			goto err_lynx_core;
		}

		ret = lynx_core_init(lynx);
		if (ret) {
			dev_err(usb->dev, "lynx_usb_probe: lynx core init failed\n");
			/* FIXME: Does it work? Are there other resource needed to be free? */
			goto err_lynx_core;
		}

#ifdef CONFIG_SUPPORT_MONITOR_MODE
		lynx->monitor_level = monitor_level;
#endif /*CONFIG_SUPPORT_MONITOR_MODE */
#endif
    }
    else
    {
        dev_info(usb->dev, "Lynx [app mode] now attached\n");

        lynx = lynx_core_create();
        if (lynx == NULL) {
            lynx_dbg(LYNX_DBG_ERR, "Failed to alloc lynx core\n");
            ret = -ENOMEM;
            goto err_hdev_destroy;
        }

        hdev->lynx = lynx;
        lynx->hif_priv = hdev;

        ret = lynx_usb_switch_on(hdev);
        if(ret) {
            dev_err(usb->dev, "lynx_usb_probe: lynx usb switch on failed\n");
            goto err_lynx_core;
        }

#ifndef CONFIG_ANDROID
	/*if(firmware_loaded != 1)
	{
	   dev_err(usb->dev, "lynx_usb_probe: FW state not match ,reboot FW\n");
	   lynx_wci_stop_cmd(hdev->lynx);
	   msleep(300);
	   lynx_usb_switch_off(hdev);
	   goto err_hdev_destroy;
	}*/
#endif

        ret = lynx_core_init(lynx);
        if (ret) {
            dev_err(usb->dev, "lynx_usb_probe: lynx core init failed\n");
			/* FIXME: Does it work? Are there other resource needed to be free? */
            goto err_lynx_core;
        }

#ifdef CONFIG_SUPPORT_MONITOR_MODE
        lynx->monitor_level=monitor_level ;
#endif /*CONFIG_SUPPORT_MONITOR_MODE */
		
    }

    return 0;

err_lynx_core:
    lynx_core_cleanup(lynx);
	os_dep_deinit(lynx);
    lynx_usb_switch_off(hdev);
err_hdev_destroy:
    if (hdev->firmware) {
        release_firmware(hdev->firmware);
        hdev->firmware = NULL;
    }
    lynx_free_hdev(hdev);
    hdev = NULL;
    usb_set_intfdata(interface, NULL);
err_usb_put:
    usb_put_dev(udev);

    return ret;
}

static void lynx_usb_disconnect(struct usb_interface *interface)
{
    struct usb_device *udev = interface_to_usbdev(interface);
    struct lynx_hif_device *hdev = usb_get_intfdata(interface);

    if (!hdev)
        return;

    if (le16_to_cpu(udev->descriptor.bcdDevice) & 0xFF) {
        dev_info(&udev->dev, "Lynx [boot mode] detached\n");
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
        dev_info(&udev->dev, "Lynx [app mode] detached\n");
        lynx_usb_device_detached(hdev);
    }

    lynx_free_hdev(hdev);
    hdev = NULL;
    usb_set_intfdata(interface, NULL);
    dev_info(&udev->dev, "lynx: USB layer deinitialized\n");
    usb_put_dev(udev);

}
#ifdef CONFIG_PM
static int lynx_usb_suspend(struct usb_interface *interface,
                        pm_message_t message)
{
    struct lynx_hif_device *hdev = usb_get_intfdata(interface);

    /*
     * The device has to be set to FULLSLEEP mode in case no
     * interface is up.
     */
// wait for implement later by nady
//  if (!(hif_dev->flags & HIF_USB_START))
//      ath9k_htc_suspend(hif_dev->htc_handle);

    hif_usb_dealloc_urbs(hdev);

    return 0;
}
static int lynx_usb_resume(struct usb_interface *interface)
{
    struct lynx_hif_device *hdev = usb_get_intfdata(interface);
    int ret;

    ret = hif_usb_alloc_urbs(hdev);
    if (ret)
        return ret;

    if (hdev->firmware) {
        ret = lynx_usb_download_fw(hdev);
        if (ret)
            goto fail_resume;
    } else {
        hif_usb_dealloc_urbs(hdev);
        return -EIO;
    }

    mdelay(100);

// wait for implement later by nady
//  ret = ath9k_htc_resume(htc_handle);
//  if (ret)
//      goto fail_resume;

    return 0;

fail_resume:
    hif_usb_dealloc_urbs(hdev);

    return ret;
}

#endif

static struct usb_driver lynx_usb_driver = {
    .name = KBUILD_MODNAME,
    .probe = lynx_usb_probe,
    .disconnect = lynx_usb_disconnect,
#ifdef CONFIG_PM
    .suspend = lynx_usb_suspend,
    .resume = lynx_usb_resume,
    .reset_resume = lynx_usb_resume,
#endif
    .id_table = lynx_usb_ids,
//#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
//  .soft_unbind = 1,
//#endif
};

int lynx_usb_init(void)
{
    if(usb_register(&lynx_usb_driver) < 0)
    {
        pr_err("No device found, lynx driver not installed\n");
        return -ENODEV;
    }
#ifdef CONFIG_LYNX_DEBUG
	if (lynx_proc_init_fs())
	{
		printk("lynx proc init error.\n");
	}
#endif
    //lynx_proc_init_fs();  

	printk("lynx driver rev. = %s\n", LYNX_DRIVER_REV);
    return 0;
}

void lynx_usb_exit(void)
{
#ifdef CONFIG_LYNX_DEBUG
	lynx_proc_exit_fs();
#endif
    usb_deregister(&lynx_usb_driver);
    //remove_proc_entry(LYNX_PROCFS_NAME, NULL);    
    pr_info("lynx driver unloaded\n");
}

module_init(lynx_usb_init);
module_exit(lynx_usb_exit);

//module_param(user_mac_addr, charp, 0644);

MODULE_AUTHOR("Montage");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Lynx driver for 802.11n wireless devices");
MODULE_VERSION(LYNX_DRIVER_REV);
MODULE_FIRMWARE(FIRMWARE_LYNX_2_0_0);
MODULE_FIRMWARE(FIRMWARE_LYNX_3_0_0);
MODULE_DEVICE_TABLE(usb, lynx_usb_ids);
