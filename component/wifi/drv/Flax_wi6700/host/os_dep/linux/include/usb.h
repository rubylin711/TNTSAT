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

#ifndef __USB_H__
#define __USB_H__

#include <linux/module.h>
#include <linux/usb.h>
#include <linux/firmware.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/leds.h>
#include <linux/slab.h>

#define FIRMWARE_DOWNLOAD       0xFE 
#define FIRMWARE_DOWNLOAD_COMP  0xFF

//#define MAX_TX_URB_NUM  32		// enlarge from 8
#define MAX_TX_URB_NUM  96		// To avoid hif_usb_send: Error(NOMEM)

#define MAX_TX_BUF_NUM  256
#define MAX_TX_BUF_SIZE 2*1024
#define TX_RESERVE      3       // reserve for wci only
#define TX_THRESHOLD    (MAX_TX_URB_NUM - TX_RESERVE)

#define MAX_RX_URB_NUM  64      // enlarge from 8
#define MAX_RX_BUF_SIZE 4*1024


#define USB_VID_MONTAGE              0xF000
#define USB_PID_LYNX_ROM3             0x6700
#define USB_PID_LYNX_ROM2             0x7000
#define USB_PID_LYNX_FPGA              0x3281



struct tx_buf {
    u8 *buf;
    u16 len;
    u16 offset;
    struct urb *urb;
    struct sk_buff_head skb_queue;
    struct lynx_hif_device *hdev;
    struct list_head list;
};

#define HIF_USB_TX_STOP  BIT(0)
#define HIF_USB_TX_FLUSH BIT(1)

struct lynx_usb_device {
    struct lynx_hw             hw; // it must be at first
    struct usb_interface       *interface;
    struct usb_device          *udev;
#if defined(CONFIG_LYNX_OS_LINUX)
    const struct usb_device_id *id;
    struct device              *dev;
    struct usb_anchor          rx_submitted;
#endif
};

int lynx_usb_init(void);
void lynx_usb_exit(void);
#endif

