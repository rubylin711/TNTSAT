
#if !defined(CONFIG_LYNX_WM_MANAGER)
#include <net/mac80211.h>
#endif
#include "wlan_def.h"
#include "init.h"
#include "core.h"
#include "mlme_api.h"
#include "cfg80211.h"
#include "lynx_debug.h"

void linux_mlme_disconnect(struct lynx_vif *vif, unsigned int sta_idx)
{
    struct lynx *lnx = vif->lnx;
	struct lynx_sta *sta;
	unsigned char null_addr[6] = {0, 0, 0, 0, 0, 0};
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
	int freq=0;
	struct ieee80211_channel *chan;
#endif
  
    if(vif->nw_type == VIF_AP_MODE) 
	{
		sta = &lnx->sta_list[sta_idx];
		
		if((sta->flags & LYNX_STA_VALID) && (sta->vif == vif))
		{
			vif->sta_idx_map &= ~(1 << sta_idx);
			cfg80211_del_sta(vif->ndev, sta->addr, CFG80211_GFP);
			memset(sta, 0, sizeof(struct lynx_sta));
		}
	}
	else if(vif->nw_type == VIF_IBSS_MODE) 
	{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
		if(lnx->channel < 14)
			freq = 2412 + (lnx->channel - 1) * 5;
		else if(lnx->channel == 14)
			freq = 2484;
		chan = ieee80211_get_channel(lnx->wiphy, freq);
		cfg80211_ibss_joined(vif->ndev, null_addr, chan, CFG80211_GFP);
#else
		cfg80211_ibss_joined(vif->ndev, null_addr, CFG80211_GFP);
#endif
	}
	else if(vif->nw_type == VIF_STA_MODE) 
	{
		if(test_bit(CONNECTED, &vif->flags)) 
		{
			/* FIXME: status code may be set by firmweare */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
			cfg80211_disconnected(vif->ndev, WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY, 
									NULL, 0, false, CFG80211_GFP);
#else
			cfg80211_disconnected(vif->ndev, WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY, 
									NULL, 0, CFG80211_GFP);
#endif
			spin_lock_bh(&vif->if_lock);
			clear_bit(CONNECTED, &vif->flags);
			if(test_and_clear_bit(DATAPATH_EN, &vif->flags))
				netif_carrier_off(vif->ndev);
			spin_unlock_bh(&vif->if_lock);

			cancel_delayed_work(&vif->sync_work);
		}
		else if(test_bit(CONNECT_PEND, &vif->flags)) 
		{
			cfg80211_connect_result(vif->ndev,
						vif->req_bssid, NULL, 0,
						NULL, 0,
						WLAN_STATUS_UNSPECIFIED_FAILURE,
						CFG80211_GFP);
			spin_lock_bh(&vif->if_lock);
			clear_bit(CONNECT_PEND, &vif->flags);
			spin_unlock_bh(&vif->if_lock);
		}
	}
}

int linux_mlme_connect(struct lynx_vif *vif, int channel, unsigned char *bssid, 
					int assoc_req_ie_len, int assoc_resp_ie_len, unsigned char *ie_data)
{
	int ret = 0;

    ret = lynx_cfg80211_connect_event(vif, channel, bssid, assoc_req_ie_len, assoc_resp_ie_len,
				ie_data);

	return ret;
}

void linux_mlme_scan_result(struct lynx_vif *vif, int is_ok)
{
    lynx_cfg80211_scan_complete(vif, is_ok);
}

int linux_mlme_schedule_sync_link_st(struct lynx_vif *vif, unsigned int op, int sec)
{
	if(op == SYNC_LINK_STOP)
		cancel_delayed_work(&vif->sync_work);
	else if(op == SYNC_LINK_START)
		schedule_delayed_work(&vif->sync_work, HZ*sec);

	return 0;
}

void linux_mlme_data_path_ctrl(struct lynx_vif *vif, unsigned int flags)
{
	if(flags & DATAPATH_QUEUE_WAKUP)
    	netif_wake_queue(vif->ndev);
	if(flags & DATAPATH_CARRIER_ON)
    	netif_carrier_on(vif->ndev);
}
	
void linux_mlme_mic_err(struct lynx_vif *vif, int sta_idx, int key_id, int is_broadcast)
{
    struct lynx *lnx = vif->lnx;
	struct lynx_sta *sta;
	unsigned char *addr=NULL;
	unsigned int key_type=0;

	if(vif->nw_type == VIF_AP_MODE) 
	{
		sta = &lnx->sta_list[sta_idx];
		addr = sta->addr;
		key_type = NL80211_KEYTYPE_PAIRWISE;
	}
	else
	{
		addr = vif->req_bssid;
		if(is_broadcast)
			key_type = NL80211_KEYTYPE_GROUP;
		else
			key_type = NL80211_KEYTYPE_PAIRWISE;
	}

	lynx_dbg(LYNX_DBG_WCI_EVENT, "%s(): addr=%02x:%02x:%02x:%02x:%02x:%02x, "
		"key_id=%d, key_type=%d. sat_idx=%d\n",
		__FUNCTION__, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5],
		key_id, key_type, sta_idx);

	lynx_cfg80211_mic_err_event(vif, addr, key_id, key_type);
}

static struct lynx_mlme_ops linux_mlme_ops = {
	.disconnect 			=	linux_mlme_disconnect,
	.connect 				=	linux_mlme_connect,
	.scan_result 			=	linux_mlme_scan_result,
	.schedule_sync_link_st  =	linux_mlme_schedule_sync_link_st,
	.data_path_ctrl			=	linux_mlme_data_path_ctrl,
	.mic_err				=	linux_mlme_mic_err,
};

int linux_mlme_init(struct lynx *lnx)
{
	lnx->mlme_ops = &linux_mlme_ops;
	
	return 0;
}
