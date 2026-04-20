#include "wlan_def.h"
#include "init.h"
#include "core.h"
#include "mlme_api.h"
#include "wm_vif.h"
#include "wm_msg.h"
#include "driver_ops.h"

int wm_mlme_connect(struct lynx_vif *vif, int channel, unsigned char *bssid, 
					int assoc_req_ie_len, int assoc_resp_ie_len, unsigned char *ie_data)
{
	struct sk_buff *skb;
	struct wm_msg *msg;
	int len=0, ret=0;

	len = assoc_req_ie_len + assoc_resp_ie_len;
	if((skb = wm_msgq_alloc(len + 6)) != NULL)	// 6 = address 
	{
		msg = (struct wm_msg *)skb->cb;
		msg->type = WM_MSG_CMD;
		msg->u.cmd.cmd = AP_CMD_ADD_STA;
		msg->u.cmd.vif_idx = vif->fw_vif_idx;
		msg->u.cmd.arg[0] = len;

		memcpy(skb->data, bssid, 6);
		if(len && ie_data)
			memcpy(skb->data + 6, ie_data, len);

		wm_msgq_write(skb);
	}
	
	/* TODO: switch channel? */

#if 0
    if(vif->nw_type == VIF_STA_MODE)
	{
		/* parse the WME ie information & setup the acm info */
		memset(&info_wme, 0, sizeof(info_wme));
		lynx_parse_ie(assoc_resp_ie, assoc_resp_len, &info_wme);
		if(info_wme.wme_ie != NULL)
			vif->acm = lynx_parse_wme_ie((struct wlan_ie_wme_param *)(info_wme.wme_ie));
		else 
			vif->acm = 0;
    }
#endif
#ifdef CONFIG_LYNX_IBSS
	if(vif->nw_type == VIF_IBSS_MODE) 
	{
		/* setup device beacon */
		lynx_wci_set_beacon_cmd(vif);
        return 0;
    }
#endif

	return ret;
}

void wm_mlme_disconnect(struct lynx_vif *vif, unsigned int sta_idx)
{
    struct lynx *lnx = vif->lnx;
	struct sk_buff *skb;
	struct lynx_sta *sta;
	struct wm_msg *msg;

	sta = &lnx->sta_list[sta_idx];

	if(sta->flags & LYNX_STA_VALID)
	{
		if((skb = wm_msgq_alloc(6)) != NULL)	// 6 = address 
		{
			msg = (struct wm_msg *)skb->cb;
			msg->type = WM_MSG_CMD;
			msg->u.cmd.cmd = AP_CMD_DEL_STA;
			msg->u.cmd.vif_idx = vif->fw_vif_idx;
			memcpy(skb->data, sta->addr, 6);

			wm_msgq_write(skb);
		}
	}
}

void wm_mlme_scan_result(struct lynx_vif *vif, int is_ok)
{
	struct sk_buff *skb;
	struct wm_msg *msg;

	if((skb = wm_msgq_alloc(0)) != NULL)
	{
		msg = (struct wm_msg *)skb->cb;
		msg->type = WM_MSG_CMD;
		msg->u.cmd.cmd = AP_CMD_SCAN_DONE;

		wm_msgq_write(skb);
	}
}

void wm_mlme_data_path_ctrl(struct lynx_vif *vif, unsigned int flags)
{
#if defined(CONFIG_LYNX_OS_LINUX)
	if(flags & DATAPATH_QUEUE_WAKUP)
    	netif_wake_queue(vif->ndev);
#endif

	if(flags & DATAPATH_CARRIER_ON)
	{
		set_bit(DATAPATH_EN, &vif->flags);
#if defined(CONFIG_LYNX_OS_LINUX)
		netif_carrier_on(vif->ndev);
#elif defined(CONFIG_LYNX_OS_UCOS)
		lynx_set_link_change(1);
		wm_manager_dhcp();
#endif
	}
	else
	{
		clear_bit(DATAPATH_EN, &vif->flags);
#if defined(CONFIG_LYNX_OS_LINUX)
		netif_carrier_off(vif->ndev);
#elif defined(CONFIG_LYNX_OS_UCOS)
		lynx_set_link_change(0);
#endif
	}
}

void wm_mlme_info_vif_status(struct lynx_vif *vif, int is_attached, unsigned char *addr)
{
	struct sk_buff *skb;
	struct wm_msg *msg;

	if((skb = wm_msgq_alloc(0)) != NULL)
	{
		msg = (struct wm_msg *)skb->cb;
		msg->type = WM_MSG_CMD;
		msg->u.cmd.cmd = AP_CMD_INFO_INTERFACE_STATUS;
		msg->u.cmd.vif_idx = vif->fw_vif_idx;
		msg->u.cmd.arg[0] = is_attached;
		memcpy((unsigned char *)&msg->u.cmd.arg[1], addr, 6);

		wm_msgq_write(skb);
	}
}

void wm_mlme_trigger_manager_start(void)
{
	struct sk_buff *skb;
	struct wm_msg *msg;

	if((skb = wm_msgq_alloc(0)))
	{
		msg = (struct wm_msg *)skb->cb;
		msg->type = WM_MSG_CMD;
		msg->u.cmd.cmd = AP_CMD_RELOAD;
		wm_msgq_write(skb);
	}
}

static struct lynx_mlme_ops wm_mlme_ops = {
#if 0
	.schedule_sync_link_st  =	wm_mlme_schedule_sync_link_st,
#endif
	.disconnect 			=	wm_mlme_disconnect,
	.connect 				=	wm_mlme_connect,
	.scan_result 			=	wm_mlme_scan_result,
	.data_path_ctrl			=	wm_mlme_data_path_ctrl,
	.info_vif_status		=	wm_mlme_info_vif_status,
	.trigger_manager_start	=	wm_mlme_trigger_manager_start,
//	.mic_err				=	wm_mlme_mic_err,
};

int wm_mlme_init(struct lynx *lynx)
{
	lynx->mlme_ops = &wm_mlme_ops;
	my_wlan_dev->driver_priv = (void *)lynx;

	return 0;
}


