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
#if !defined(CONFIG_LYNX_WM_MANAGER) && defined(CONFIG_LYNX_OS_LINUX)
#include <net/mac80211.h>
#endif

#include "init.h"
#include "wlan_def.h"
#include "core.h"
#include "vfc.h"
#include "hif.h"
#include "hw.h"
#include "usb.h"
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

int hif_device_init(void *hif_handle)
{
    struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif_handle;
    return (hdev) ? hdev->hif_ops->init(hdev) : -1;
}

void hif_device_deinit(void *hif_handle)
{
    struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif_handle;
    if(hdev)
        hdev->hif_ops->deinit(hif_handle);
}

int hif_device_start(void *hif_handle)
{
	struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif_handle;
	struct lynx *lnx = hdev->lynx;
	int ret = -ENODEV;
#ifdef CONFIG_LYNX_W2USB_NO_RESET
	int retry = 0;
#endif

#ifndef CONFIG_LYNX_W2USB_NO_RESET
	if(hdev && lnx) 
	{
		hdev->hif_ops->start(hif_handle);
		lynx_wci_start_cmd(lnx);
		ret = os_wait_init_complete(lnx);

		/* FIXME: if os doesn't support wait method, direct do
			  set_bit(WCI_READY, &lynx->flag);	*/

		if(ret)
		{
			lynx_dbg(LYNX_DBG_ERR, "Device is not ready!! \n");
			hif_device_stop(hif_handle);
		}
	}
#else
	if(hdev && lnx)
	{
		hdev->hif_ops->start(hif_handle);
		for(retry = 0; retry < 5; retry++)
		{
			//printk(KERN_CRIT "send wci start cmd %d\n", retry);
			if(retry)
			{
				lynx_wci_bulk_out_sync_pid(lnx);
				lynx_wci_start_cmd(lnx);
			}
			ret = os_wait_init_complete(lnx);

			/* FIXME: if os doesn't support wait method, direct do
				  set_bit(WCI_READY, &lynx->flag);	*/

			if(ret)
			{
				//printk(KERN_CRIT "Device is not ready!! %d\n", retry);
				lynx_dbg(LYNX_DBG_ERR, "Device is not ready!! %d\n", retry);
			}
			else
				break;
		}
		if(ret)
			hif_device_stop(hif_handle);
	}
#endif
	return ret;
}

void hif_device_stop(void *hif_handle)
{
    struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif_handle;
    if(hdev)
        hdev->hif_ops->stop(hif_handle);
}

int hif_device_send(void *hif_handle, struct sk_buff *buf)
{
    struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif_handle;
    return (hdev) ? hdev->hif_ops->send(hif_handle, buf) : 0;
}
int hif_device_recv(void *hif_handle, struct sk_buff *buf)
{
    struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif_handle;
    return (hdev) ? hdev->hif_ops->recv(hif_handle, buf) : 0;
}

int hif_tx_lock(void *hif_handle, unsigned long *flags)
{
	struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif_handle;
	int ret = 0;

	if((hdev) && (hdev->hif_ops->tx_lock))
	{	
		ret = hdev->hif_ops->tx_lock(hif_handle, flags);
	}
	else
		ret = -1;

    return ret;
}

int hif_tx_unlock(void *hif_handle, unsigned long *flags)
{
	struct lynx_hif_device *hdev = (struct lynx_hif_device *)hif_handle;
	int ret = 0;

	if((hdev) && (hdev->hif_ops->tx_unlock))
	{	
		ret = hdev->hif_ops->tx_unlock(hif_handle, flags);
	}
	else
		ret = -1;

    return ret;
}

struct lynx_hif_device *lynx_alloc_hdev(int priv_size)
{
	struct lynx_hif_device *hdev;
    
	hdev = (struct lynx_hif_device *) os_api_alloc(sizeof(struct lynx_hif_device) + priv_size, GFP_KERNEL);

	return (hdev) ? hdev : NULL;
}

void lynx_free_hdev(struct lynx_hif_device *hdev)
{
	if(hdev)
		os_api_free(hdev);
}

