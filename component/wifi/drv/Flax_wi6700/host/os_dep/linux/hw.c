/*=============================================================================+
|                                                                              |
| Copyright 2013                                                               |
| Acrospeed Inc. All right reserved.                                           |
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
#if !defined(CONFIG_LYNX_WM_MANAGER)
#include <net/mac80211.h>
#endif

#include "wlan_def.h"
#include "vfc.h"
#include "init.h"
#include "core.h"
#include "hif.h"
#include "hw.h"
#include "usb.h"
#include "sdio.h"
#include "lynx_debug.h"
#include "wci.h"

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/

/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/

/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/

/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/

#if 0
static int hex2num(char c)
{               
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

int hwaddr_aton(const char *txt, u8 *addr)
{
    int i;

    for (i = 0; i < 6; i++) {
        int a, b;

        a = hex2num(*txt++);
        if (a < 0)
            return -1;
        b = hex2num(*txt++);
        if (b < 0)
            return -1;
        *addr++ = (a << 4) | b;
        if (i < 5 && *txt++ != ':')
            return -1;
    }

    return 0;
}
#endif

/****************************************************/
#ifdef CONFIG_LYNX_W2USB
static u8 lynx_hw_iw7000_gettx_epid(u8 id)
{
    enum lynx_ep_id rid = END_POINT_UNUSED;

    switch(id)
    {
        case VFC_TX_AC0:
            rid = END_POINT_1;
            break;
        case VFC_TX_AC1:
            rid = END_POINT_2;
            break;
        case VFC_TX_AC2:
            rid = END_POINT_3;
            break;
        case VFC_TX_AC3:
        case VFC_TX_CMD:
            rid = END_POINT_4;
            break;
        case VFC_RX_MCU:
            rid = END_POINT_1;
            break;
        default:
            printk(KERN_ERR "%s unknown lynx ep id = %d\n", __func__, id);
            break;
    }

    return rid;
}
static u8 lynx_hw_iw7000_getrx_epid(u8 id)
{
    return lynx_hw_iw7000_gettx_epid(VFC_RX_MCU);
}
static u8 lynx_hw_iw7000_valid_epid(u8 id)
{
    if((id < END_POINT_1) || (id > END_POINT_4))
        return 1; // error
    else
        return 0;
}
#endif /*CONFIG_LYNX_W2USB*/

#ifdef CONFIG_LYNX_W2SDIO
static void lynx_sdio_tx_callback(void *target, struct sk_buff *skb, enum lynx_ep_id id, bool txok)
{
	struct lynx_wci_hdr *wci_hdr;
	struct net_device *dev;
	struct lynx_vif *vif;
	struct lynx *lynx = NULL;
	lynx = ((struct lynx_hif_device *)target)->lynx;
	
	wci_hdr = (struct lynx_wci_hdr *)skb->data;
	if((txok == true) && (wci_hdr->cmd_id >= WCI_H2D_PACKET) && 
		(wci_hdr->cmd_id <= WCI_H2D_PACKET_END)){
		
		if(skb->dev){
			dev = skb->dev;
			dev->stats.tx_packets++;
			dev->stats.tx_bytes += skb->len;
			/* lynx_get_stats use the vif->net_stats info */ 
			vif = netdev_priv(dev);
			vif->net_stats.tx_packets++;
			vif->net_stats.tx_bytes += skb->len;
		}else{
			vif = lynx_vif_first(lynx);
			vif->net_stats.tx_packets++;
			vif->net_stats.tx_bytes += skb->len;
			vif->ndev->stats.tx_packets++;
			vif->ndev->stats.tx_bytes += skb->len;
		}
	}

	/* FIXME: add any tx callback feature here */
	if (skb->data[0] >= 128){
		lynx_dbg(LYNX_DBG_USB_TX, "%s:%d tx completion data[1]=%04x\n", __func__, __LINE__, skb->data[1]);
	}

	dev_kfree_skb_any(skb);
	/* check & dequeue the queued skb */
	if(txok == true)
		lynx_data_dequeue(lynx);

	lynx_dbg(LYNX_DBG_USB_TX, "%s:%d tx completion\n", __func__, __LINE__);
}
#endif /*CONFIG_LYNX_W2SDIO*/

#ifdef CONFIG_LYNX_W2USB
static void lynx_usb_tx_callback(void *target, struct sk_buff *skb, enum lynx_ep_id id, bool txok)
{
	struct lynx_wci_hdr *wci_hdr;
	struct net_device *dev;
	struct lynx_vif *vif;

	wci_hdr = (struct lynx_wci_hdr *)skb->data;

	if((txok == true) && (wci_hdr->cmd_id >= WCI_H2D_PACKET) && 
		(wci_hdr->cmd_id <= WCI_H2D_PACKET_END))
    {
        if(skb->dev)
		{
			dev = skb->dev;
			dev->stats.tx_packets++;
			dev->stats.tx_bytes += skb->len;
   
			/* lynx_get_stats use the vif->net_stats info */ 
			vif = netdev_priv(dev);
			vif->net_stats.tx_packets++;
			vif->net_stats.tx_bytes += skb->len;
		}
    }

    /* FIXME: add any tx callback feature here */

    if (skb->data[0] >= 128)
	{
		lynx_dbg(LYNX_DBG_USB_TX, "%s:%d tx completion data[1]=%04x\n", __func__, __LINE__, skb->data[1]);
	}

    dev_kfree_skb_any(skb);

	/* check & dequeue the queued skb */
	if(txok == true)
		lynx_data_dequeue(((struct lynx_hif_device *)target)->lynx);

    lynx_dbg(LYNX_DBG_USB_TX, "%s:%d tx completion\n", __func__, __LINE__);
}

static void lynx_usb_rx_callback(void *target, struct sk_buff *skb, enum lynx_ep_id id)
{
    struct lynx_hif_device *hdev = (struct lynx_hif_device *)target;

    lynx_dbg(LYNX_DBG_USB_RX, "%s:%d rx callback\n", __func__, __LINE__);

    os_api_lock(&hdev->rx_lock);
    __skb_queue_tail(&hdev->rx_skb_queue, skb);
    os_api_unlock(&hdev->rx_lock);
    tasklet_schedule(&hdev->rx_tasklet);
}

static void lynx_hw_iw7000_ep_init(struct lynx_hif_device *hdev, struct lynx_endpoint *ep, enum lynx_ep_id id)
{
    struct lynx_usb_device *usb = (struct lynx_usb_device *)hdev_priv(hdev);
    struct usb_interface *interface = usb->interface;
    struct usb_host_interface *iface_desc = interface->cur_altsetting;
    struct usb_endpoint_descriptor *endpoint;
    int i;

    ep->epid = id;
    ep->max_packet_size = 512; // default size
    memset(&ep->ep_callbacks, 0, sizeof(struct lynx_ep_callbacks));

    for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i) 
	{
        endpoint = &iface_desc->endpoint[i].desc;

        if ((usb_endpoint_is_bulk_out(endpoint)) && (id == usb_endpoint_num(endpoint)))
        {
            /* max_packet_size is only used in out-direction */
            ep->max_packet_size = le16_to_cpu(endpoint->wMaxPacketSize);
            break;
        }
    }

    switch(id)
    {
        case END_POINT_1:
            ep->ep_callbacks.rx = lynx_usb_rx_callback;
        case END_POINT_2:
        case END_POINT_3:
        case END_POINT_4:
            ep->ep_callbacks.tx = lynx_usb_tx_callback;
            break;
        default:
            break;  
    }
}

void lynx_hw_usb_iw7000_init(struct lynx_hif_device *hdev)
{
    struct lynx_usb_device *usb = hdev_priv(hdev);
    struct lynx_hw *hw = hdev_priv(hdev);
    struct usb_interface *interface = usb->interface;
    struct usb_host_interface *iface_desc = interface->cur_altsetting;
    struct usb_endpoint_descriptor *endpoint;
    struct lynx_hw_usb *hwusb;
    int i;

    hwusb = &hw->usb;

    for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i) 
	{
        endpoint = &iface_desc->endpoint[i].desc;

        printk(KERN_DEBUG "bNumEndpoints=%d[%d]\n", iface_desc->desc.bNumEndpoints, i);
        if (usb_endpoint_xfer_bulk(endpoint)) {
            printk(KERN_DEBUG "%s Bulk Ep:0x%2.2X maxpktsz:%d\n",
                   usb_endpoint_dir_in(endpoint) ?
                   "RX" : "TX", endpoint->bEndpointAddress,
                   le16_to_cpu(endpoint->wMaxPacketSize));
        } else if (usb_endpoint_xfer_int(endpoint)) {
            printk(KERN_DEBUG "%s Int Ep:0x%2.2X maxpktsz:%d interval:%d\n",
                   usb_endpoint_dir_in(endpoint) ?
                   "RX" : "TX", endpoint->bEndpointAddress,
                   le16_to_cpu(endpoint->wMaxPacketSize),
                   endpoint->bInterval);
        } else if (usb_endpoint_xfer_isoc(endpoint)) {
            printk(KERN_DEBUG "%s ISOC Ep:0x%2.2X maxpktsz:%d interval:%d\n",
                   usb_endpoint_dir_in(endpoint) ?
                   "RX" : "TX", endpoint->bEndpointAddress,
                   le16_to_cpu(endpoint->wMaxPacketSize),
                   endpoint->bInterval);
        }
    }

    for(i=END_POINT_0; i<END_POINT_MAX; i++)
        lynx_hw_iw7000_ep_init(hdev, &hwusb->ep[i], i);
}

#endif /*CONFIG_LYNX_W2USB*/


#ifdef CONFIG_LYNX_W2SDIO
void lynx_hw_sdio_iw7000_init(struct lynx_hif_device *hdev)
{
    hdev->sdio_tx_callback = (tx_cb)lynx_sdio_tx_callback;
}
#endif /*CONFIG_LYNX_W2SDIO*/


#ifdef CONFIG_LYNX_W2USB
static struct lynx_hw usb_fpga_iw7000_hw = {
    .vendor_id  = 0xF000,
    .device_id  = 0x3281, /*FPGA*/
    .version    = 0,
    .subversion = 0,
    .type       = LYNX_HIF_TYPE_USB,
    .name       = "iw7000_f",
    .init       = lynx_hw_usb_iw7000_init,
    .usb.gettx_epid  = lynx_hw_iw7000_gettx_epid,
    .usb.getrx_epid  = lynx_hw_iw7000_getrx_epid,
    .usb.valid_epid  = lynx_hw_iw7000_valid_epid,
};

#ifdef CONFIG_LYNX_ROM2
static struct lynx_hw usb_iw7000_hw= {
    .vendor_id  = 0xF000,
    .device_id  = 0x7000,
    .version    = 0,
    .subversion = 0,
    .type       = LYNX_HIF_TYPE_USB,
    .name       = "iw7000",
    .init       = lynx_hw_usb_iw7000_init,
    .usb.gettx_epid  = lynx_hw_iw7000_gettx_epid,
    .usb.getrx_epid  = lynx_hw_iw7000_getrx_epid,
    .usb.valid_epid  = lynx_hw_iw7000_valid_epid,
};
#endif /*CONFIG_LYNX_ROM2*/

#ifdef CONFIG_LYNX_ROM3
static struct lynx_hw usb_iw6700_hw = {
    .vendor_id  = 0xF000,
    .device_id  = 0x6700,
    .version    = 0,
    .subversion = 0,
    .type       = LYNX_HIF_TYPE_USB,
    .name       = "iw6700",
    .init       = lynx_hw_usb_iw7000_init,
    .usb.gettx_epid  = lynx_hw_iw7000_gettx_epid,
    .usb.getrx_epid  = lynx_hw_iw7000_getrx_epid,
    .usb.valid_epid  = lynx_hw_iw7000_valid_epid,
};
#endif /*CONFIG_LYNX_ROM3*/
#endif /*CONFIG_LYNX_W2USB*/


#ifdef CONFIG_LYNX_W2SDIO
static struct lynx_hw sdio_fpga_iw7000_hw = {
    .vendor_id  = SDIO_VENDOR_ID_MONTAGE,
    .device_id  = SDIO_DEVICE_ID_LYNX_FPGA,
    .version    = 0,
    .subversion = 0,
    .type       = LYNX_HIF_TYPE_SDIO,
    .name       = "iw7000_f",
    .init       = lynx_hw_sdio_iw7000_init,
};

#ifdef CONFIG_LYNX_ROM2
static struct lynx_hw sdio_iw7000_hw = {
    .vendor_id  = SDIO_VENDOR_ID_MONTAGE,
    .device_id  = SDIO_DEVICE_ID_LYNX_ROM2,
    .version    = 0,
    .subversion = 0,
    .type       = LYNX_HIF_TYPE_SDIO,
    .name       = "iw7000",
    .init       = lynx_hw_sdio_iw7000_init,
};
#endif /*CONFIG_LYNX_ROM2*/

#ifdef CONFIG_LYNX_ROM3
static struct lynx_hw sdio_iw6700_hw = {
    .vendor_id  = SDIO_VENDOR_ID_MONTAGE,
    .device_id  = SDIO_DEVICE_ID_LYNX_ROM3,
    .version    = 0,
    .subversion = 0,
    .type       = LYNX_HIF_TYPE_SDIO,
    .name       = "iw6700",
    .init       = lynx_hw_sdio_iw7000_init,
};
#endif /*CONFIG_LYNX_ROM3*/

#endif /*CONFIG_LYNX_W2SDIO*/

struct lynx_hw *hws[] = {
//------ USB -----------------
#ifdef CONFIG_LYNX_W2USB
	&usb_fpga_iw7000_hw,
	
#ifdef CONFIG_LYNX_ROM2
	&usb_iw7000_hw,
#endif /*CONFIG_LYNX_ROM2*/

#ifdef CONFIG_LYNX_ROM3
	&usb_iw6700_hw,
#endif /*CONFIG_LYNX_ROM3*/

#endif /*CONFIG_LYNX_W2USB*/

//------ SDIO -----------------
#ifdef CONFIG_LYNX_W2SDIO
	&sdio_fpga_iw7000_hw,

#ifdef CONFIG_LYNX_ROM2
	&sdio_iw7000_hw,
#endif /*CONFIG_LYNX_ROM2*/

#ifdef CONFIG_LYNX_ROM3
	&sdio_iw6700_hw,
#endif /*CONFIG_LYNX_ROM3*/

#endif /*CONFIG_LYNX_W2SDIO*/
};

/*************** export function *****************/
int lynx_hw_init(struct lynx_hif_device *hdev)
{
    struct lynx_hw *hw = hdev_priv(hdev);
    struct lynx_hw_usb *hwusb;
#ifdef CONFIG_LYNX_W2SDIO
    struct lynx_hw_sdio *hwsdio;
#endif
    int found = 0;
    int i;

    for(i=0;i<sizeof(hws)/sizeof(struct lynx_hw *);i++) 
	{
        if((hws[i]->type == hdev->hif_type) && 
           (hws[i]->vendor_id == hdev->vendor_id) && 
           (hws[i]->device_id == hdev->device_id)) 
		{
            memcpy(hw, hws[i], sizeof(struct lynx_hw));
            if(hw->init)
                hw->init(hdev);
            found = 1;
            break;
        }
    }

	if(!found) {
		printk(KERN_ERR "hw init failed!!\n");
		printk(KERN_DEBUG "    type = %x\n", hdev->hif_type);
		printk(KERN_DEBUG "    vendor id = %x\n", hdev->vendor_id);
		printk(KERN_DEBUG "    device id = %x\n", hdev->device_id);
		return -1;
	}

    if(hdev->hif_type == LYNX_HIF_TYPE_USB)
        printk(KERN_DEBUG "USB: ");
    else if(hdev->hif_type == LYNX_HIF_TYPE_SDIO)
        printk(KERN_DEBUG "SDIO: ");
    else 
	{
        // impossible code here
        printk(KERN_ERR "unknownn hif_type = %d\n", hdev->hif_type);
        return -1;
    }
    printk(KERN_DEBUG "%s hw init ready\n", hw->name);
    printk(KERN_DEBUG "    vendor id=%x\n", hw->vendor_id);
    printk(KERN_DEBUG "    device id=%x\n", hw->device_id);
    if(hdev->hif_type == LYNX_HIF_TYPE_USB) 
	{
        hwusb = &hw->usb;
        printk(KERN_DEBUG "endpoint mapping\n");
        printk(KERN_DEBUG "    VFC_TX_AC0=%d\n", hwusb->gettx_epid(VFC_TX_AC0));
        printk(KERN_DEBUG "    VFC_TX_AC1=%d\n", hwusb->gettx_epid(VFC_TX_AC1));
        printk(KERN_DEBUG "    VFC_TX_AC3=%d\n", hwusb->gettx_epid(VFC_TX_AC2));
        printk(KERN_DEBUG "    VFC_TX_AC4=%d\n", hwusb->gettx_epid(VFC_TX_AC3));
        printk(KERN_DEBUG "    VFC_RX_MCU=%d\n", hwusb->getrx_epid(VFC_RX_MCU));
    }
#ifdef CONFIG_LYNX_W2SDIO
    else if(hdev->hif_type == LYNX_HIF_TYPE_SDIO) 
	{
        hwsdio = &hw->sdio;
        printk(KERN_DEBUG "SDIO init not implement\n");
    }
#endif

    // always return successful
    return 0;
}
