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
#if defined(CONFIG_LYNX_OS_LINUX)
#if !defined(CONFIG_LYNX_WM_MANAGER)
#include <net/mac80211.h>
#endif	// CONFIG_LYNX_WM_MANAGER
#include <linux/random.h>
#include <linux/moduleparam.h>
#include "utility.h"	// for ioctl function
#endif	// CONFIG_LYNX_OS_LINUX

#include "wlan_def.h"
#include "init.h"
#include "core.h"
#include "vfc.h"
#include "wci.h"
#include "txrx.h"
#include "hif.h"
#include "lynx_debug.h"
#include "mlme_api.h"


#if !defined(CONFIG_ANDROID) && defined(CONFIG_LYNX_OS_LINUX)
#include "lynx_calibration.h"
#endif

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#if defined(CONFIG_ANDROID)
/* Android doesn't use Makefie to auto-generate the lynx_calibration.h */
#define LYNX_MAC_PATH "/system/etc/wifi/"
#define LYNX_RF_PATH "/data/"
#endif

/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/

/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/

/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/
#if defined(CONFIG_LYNX_OS_LINUX)
unsigned char *mp_resp=NULL;
static char *no_k_file_path="";

module_param(no_k_file_path,charp,0);
#endif

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/

int sta_map2idx(struct lynx_vif *vif, const u8 *addr)
{
    struct lynx *lnx = vif->lnx;
	struct lynx_sta *sta;
    int sta_idx;

    for(sta_idx=0; sta_idx < LYNX_STA_MAX_NUM; sta_idx++)
    {
        if(vif->sta_idx_map & (1 << sta_idx))
		{
			sta = &lnx->sta_list[sta_idx];
			if((sta->flags & LYNX_STA_VALID) && (!addr || !memcmp(addr, sta->addr, ETH_ALEN)))
				break;
		}
    }

    if(sta_idx >= LYNX_STA_MAX_NUM)
        sta_idx = -1;

    return sta_idx;
}

void lynx_sync_link_status(struct lynx_vif *vif, unsigned int sta_idx_map, unsigned char *rate_idx)
{
	unsigned int diff_map=0, count=0;
	int i;
#ifdef CONFIG_LYNX_STATICS
	int idx;
	struct lynx *lnx = vif->lnx;
	struct lynx_sta *sta;
	struct timespec64 now;
#endif	// CONFIG_LYNX_STATICS

	if(test_bit(CONNECTED, &vif->flags) == 0)
		return;

#ifdef CONFIG_LYNX_STATICS
	for(i=0; i<LYNX_STA_MAX_NUM; i++) 
	{
		idx = 1 << i;
		if((vif->sta_idx_map & idx))
		{
			lnx->sta_tx_rate[i] = rate_idx[i];

			/* The hostapd uses the inactive_time to check the STA alived, or not.
			   Update the last_active_time to avoid that the alived STA has no traffic with 
			   long term  */
			if(idx & sta_idx_map)
			{
				sta = &lnx->sta_list[i];
				ktime_get_real_ts64(&now);
				sta->last_active_time = now.tv_sec * 1000;
			}
		}
	}
#endif	// CONFIG_LYNX_STATICS

	diff_map = vif->sta_idx_map ^ sta_idx_map;

	if(diff_map)	// handle the link on status mismatch between the host & the device
	{
		for(i=0; i<LYNX_STA_MAX_NUM; i++)
		{
			count = 1<<i;
			if(count & diff_map)
			{
				if(sta_idx_map & count)		// link on in the device
				{
					if(vif->nw_type == VIF_AP_MODE)
					{
						lynx_wci_sta_remove_cmd(vif, i, WLAN_REASON_DEAUTH_LEAVING);
					}
				}
				else	// link on in the host
				{
					mlme_disconnect(vif, i);
				}
			}
		}
	}
}

static int lynx_wci_scan_complete_evt(struct lynx_vif *vif, u8 *datap, u32 len)
{
    struct wci_evt_ret_scan_state *ev;

    ev = (struct wci_evt_ret_scan_state *)datap;
   
    mlme_scan_result(vif, ev->isok);

    return 0;
}

static void lynx_wci_connect_evt(struct lynx_vif *vif, u8 *datap, u32 len)
{
    struct lynx *lnx = vif->lnx;
	struct device_configs *fw = &lnx->fw_config;
    struct wci_evt_ret_sta_connected *ev = (struct wci_evt_ret_sta_connected *)datap;
	int sta_idx;
	int i;
	struct lynx_sta *sta;
#ifdef CONFIG_LYNX_STATICS
	struct timespec64 now;
#endif

	sta_idx = ev->sta_idx;

	/* the sta_idx is over the valid range */
	if(sta_idx >= LYNX_STA_MAX_NUM)
		return;
   
    if(mlme_connect(vif, ev->channel, ev->bssid, 
				ev->assoc_req_ie_len, ev->assoc_resp_ie_len, 
				(unsigned char *)(&ev[1])))
    {
        /* FIXME: can't info cfg80211 * Should disconnect it? */
    }

	if(vif->nw_type != VIF_IBSS_MODE)
	{
		vif->sta_idx_map |=  (1 << sta_idx);
		sta = &lnx->sta_list[sta_idx];
		sta->flags = LYNX_STA_VALID;
		sta->vif = vif;
		memcpy(sta->addr, ev->bssid, ETH_ALEN);

#ifdef CONFIG_LYNX_STATICS
		sta->rx_bytes = 0;
		sta->rx_packets = 0;
		sta->tx_bytes = 0;
		sta->tx_packets = 0;
		sta->signal_last = 0;
		sta->signal_avg = 0;
		ktime_get_real_ts64(&now);
		sta->last_active_time = (now.tv_sec * 1000);
#endif	// CONFIG_LYNX_STATICS
		sta->bandwidth = ev->bandwidth;
		sta->support_rates = be32_to_cpu(ev->supp_rates);
	}

	if(vif->nw_type == VIF_AP_MODE)
	{
		if(ev->is_wds)
			sta->flags |= LYNX_STA_WDS_PEER;
	}
	else // if((vif->nw_type == VIF_STA_MODE)  || (vif->nw_type == VIF_IBSS_MODE))
	{
		/* FIXME: handle channel switch in sta mode */
		lnx->channel = ev->channel;

		/* FIXME: not sure what to do with IBSS mode */
		/* avoid re-schedule sync_work */
		mlme_schedule_sync_link_st(vif, SYNC_LINK_STOP, 0);
		/* delay 10 sec to start sync work */
		mlme_schedule_sync_link_st(vif, SYNC_LINK_START, 1);

		if((lnx->num_vif == 1) && (vif->nw_type == VIF_STA_MODE))
		{
			for(i=0; i<4; i++)
			{
				fw->sta_ac_parms[i].aifs = ev->aifs[i];
			}

			lynx_wmm_queue_prioty_setup(lnx);
		}
	}

	os_api_dsr_lock(&vif->if_lock);

	set_bit(CONNECTED, &vif->flags);
	clear_bit(CONNECT_PEND, &vif->flags);

	if(vif->auth_mode == NONE_AUTH)
    	set_bit(DATAPATH_EN, &vif->flags);

	mlme_data_path_ctrl(vif, (DATAPATH_QUEUE_WAKUP | DATAPATH_CARRIER_ON));

	os_api_dsr_unlock(&vif->if_lock);
}

static void lynx_wci_disconnect_evt(struct lynx_vif *vif, u8 *datap, u32 len)
{
	struct wci_evt_ret_sta_disconnected *ev = (struct wci_evt_ret_sta_disconnected *)datap;
	unsigned int sta_idx = ev->sta_idx;
	
	mlme_disconnect(vif, sta_idx);
}

static void lynx_wci_link_err_evt(struct lynx_vif *vif, u8 *datap, u32 len)
{
//	struct wci_evt_ret_sta_link_err *ev = (struct wci_evt_ret_sta_link_err *)datap;

	return;
}

static void lynx_wci_get_tx_pwr_evt(struct lynx_vif *vif, u8 *datap, u32 len)
{
	struct lynx *lnx = vif->lnx;
	struct wci_evt_ret_tx_power *ev = (struct wci_evt_ret_tx_power *)datap;

	lnx->tx_pwr = be32_to_cpu(ev->tx_pwr);
	/* wakeup the wait queue to handle the event */
	os_wakeup_wait_queue(&lnx->event_wq);
}

static void lynx_wci_vif_attach_evt(struct lynx_vif *vif, u8 *datap, u32 len)
{
    struct wci_evt_ret_vif_attach *ev = (struct wci_evt_ret_vif_attach *)datap;

	if(len != sizeof(struct wci_evt_ret_vif_attach))
	{
		lynx_dbg(LYNX_DBG_WARN, "%s(): event size mismatch(%d)\n", __FUNCTION__, len);
	}
	else 
	{
		mlme_info_vif_status(vif, ev->isok, ev->macaddr);

		if(!ev->isok)
			lynx_dbg(LYNX_DBG_WARN, "%s(): device info failure\n", __FUNCTION__);
		else	
			lynx_dbg(LYNX_DBG_WARN, "%s(): device attach success\n", __FUNCTION__);
	}
}

static void lynx_wci_get_vif_addr_evt(struct lynx_vif *vif, u8 *datap, u32 len)
{
    struct wci_evt_ret_vif_addr *ev = (struct wci_evt_ret_vif_addr *)datap;

	if(len < sizeof(struct wci_evt_ret_vif_addr))
	{
		lynx_dbg(LYNX_DBG_WCI_EVENT, "%s(): wrong evt length(%d)\n", __FUNCTION__, len);
		return; 
	}
   
	lynx_dbg(LYNX_DBG_WCI_EVENT, "%s(): bss_idx=%d, addr=%02x:%02x:%02x:%02x:%02x:%02x\n", 
			__FUNCTION__, vif->fw_vif_idx, ev->addr[0], ev->addr[1], ev->addr[2],
			ev->addr[3], ev->addr[4], ev->addr[5]); 

    memcpy(vif->vif_addr, ev->addr, WLAN_ADDR_LEN);
}

static void lynx_wci_mic_err_evt(struct lynx_vif *vif, u8 *datap, u32 len)
{
    struct wci_evt_rx_mic_err *ev = (struct wci_evt_rx_mic_err *)datap;

	mlme_mic_err(vif, ev->sta_idx, ev->key_id, ev->is_broadcast);
}

static void lynx_wci_get_vif_link_st_evt(struct lynx_vif *vif, u8 *datap, u32 len)
{
	struct wci_evt_ret_vif_link_st *ev = (struct wci_evt_ret_vif_link_st *)datap;

	if(len < sizeof(struct wci_evt_ret_vif_link_st))
	{
		lynx_dbg(LYNX_DBG_WCI_EVENT, "%s(): wrong evt length(%d)\n", __FUNCTION__, len);
		return; 
	}
   
	lynx_dbg(LYNX_DBG_WCI_EVENT, "%s(): bss_idx=%d, sta_idx_map=0x%02x"
			", rate_idx=(%u, %u, %u, %u, %u, %u, %u, %u)\n", 
			__FUNCTION__, vif->fw_vif_idx, ev->sta_idx_map,
			ev->rate_idx[0], ev->rate_idx[1], ev->rate_idx[2], ev->rate_idx[3],
			ev->rate_idx[4], ev->rate_idx[5], ev->rate_idx[6], ev->rate_idx[7]); 
	
	memcpy(vif->lnx->fw_count,ev->count,sizeof(u32)*3);
	vif->lnx->fw_count[0]=be32_to_cpu(vif->lnx->fw_count[0]);
	vif->lnx->fw_count[1]=be32_to_cpu(vif->lnx->fw_count[1]);
	vif->lnx->fw_count[2]=be32_to_cpu(vif->lnx->fw_count[2]);
	lynx_sync_link_status(vif, ev->sta_idx_map, ev->rate_idx);
	set_bit(WCI_LINK_ST_RET,&vif->lnx->flag);
	os_wakeup_wait_queue(&vif->lnx->event_wq);
}

#ifdef CONFIG_SUPPORT_MONITOR_MODE
static void lynx_wci_get_monitor_evt(struct lynx_vif *vif, u8 *datap, u32 len)
{
	struct wci_evt_ret_mon *ev = (struct wci_evt_ret_mon *)datap;
	u32 mon_mask = 0 ;
	mon_mask = be32_to_cpu( ev->mon_mask);
	printk("Get Original FW Monitor Mask=0x%08x\n",  mon_mask); 

	vif->lnx->mon_mask = mon_mask ;
}
#endif /*CONFIG_SUPPORT_MONITOR_MODE */

#ifdef CONFIG_LYNX_DEBUG
#ifdef CONFIG_HOST_DEBUG_DEVICE
static int lynx_wci_ret_dbg_info(struct lynx_vif *vif, u8 *datap, u32 len)
{
	struct wci_evt_dbg_info *evt = NULL;
	char *data=NULL;
	int ret = -1;

    if(vif)
	{
		evt = (struct wci_evt_dbg_info *)datap;
		if(evt->payload_len)
			data = (char *)&evt[1];
		lynx_handle_dbg_info(vif, be16_to_cpu(evt->item_id), be16_to_cpu(evt->payload_len), data);
	}

    return ret;
}
#endif

const char *wci_cmd_to_name(enum WCI_CMD_ID wci_cmd)
{   
    switch (wci_cmd) {
        case WCI_H2D_MGT_TX:
            return "WCI_H2D_MGT_TX";
        case WCI_H2D_MGT_TX_ACK:
            return "WCI_H2D_MGT_TX_ACK";
        case WCI_H2D_DEV_START:
            return "WCI_H2D_DEV_START";
        case WCI_H2D_DEV_STOP:
            return "WCI_H2D_DEV_STOP";
        case WCI_H2D_ACCESS_MEMORY:
            return "WCI_H2D_ACCESS_MEMORY";
        case WCI_H2D_GET_FW_VERSION:
            return "WCI_H2D_GET_FW_VERSION";
        case WCI_H2D_VIF_ATTACH:
            return "WCI_H2D_VIF_ATTACH";
        case WCI_H2D_VIF_DETACH:
            return "WCI_H2D_VIF_DETACH";
        case WCI_H2D_STA_ADD:
            return "WCI_H2D_STA_ADD";
        case WCI_H2D_STA_REMOVE:
            return "WCI_H2D_STA_REMOVE";
        case WCI_H2D_STA_CONNECT:
            return "WCI_H2D_STA_CONNECT";
        case WCI_H2D_STA_DISCONNECT:
            return "WCI_H2D_STA_DISCONNECT";
        case WCI_H2D_ALIVE:
            return "WCI_H2D_ALIVE";
        case WCI_H2D_SET_MAC:
            return "WCI_H2D_SET_MAC";
        case WCI_H2D_SET_CHANNEL:
            return "WCI_H2D_SET_CHANNEL";
        case WCI_H2D_SET_TX_POWER:
            return "WCI_H2D_SET_TX_POWER";
        case WCI_H2D_SET_TXQ:
            return "WCI_H2D_SET_TXQ";
        case WCI_H2D_SET_ERP:
            return "WCI_H2D_SET_ERP";
        case WCI_H2D_SET_HT:
            return "WCI_H2D_SET_HT";
        case WCI_H2D_SET_KEY:
            return "WCI_H2D_SET_KEY";
        case WCI_H2D_SET_DEF_KEY:
            return "WCI_H2D_SET_DEF_KEY";
        case WCI_H2D_SET_TSF:
            return "WCI_H2D_SET_TSF";
        case WCI_H2D_SET_BITRATE_MASK:
            return "WCI_H2D_SET_BITRATE_MASK";
        case WCI_H2D_SET_BEACON:
            return "WCI_H2D_SET_BEACON";
        case WCI_H2D_SET_SLOTTIME:
            return "WCI_H2D_SET_SLOTTIME";
        case WCI_H2D_SET_MON:
            return "WCI_H2D_SET_MON";
        case WCI_H2D_GET_VIF:
            return "WCI_H2D_GET_VIF";
        case WCI_H2D_GET_STA:
            return "WCI_H2D_GET_STA";
#ifdef CONFIG_HOST_WPS
        case WCI_H2D_SET_MGMT_IE:
           return "WCI_H2D_SET_MGMT_IE";
#endif /*CONFIG_HOST_WPS*/
#ifdef CONFIG_HOST_P2P
        case WCI_H2D_SET_MGMT_TXPKT:
           return "WCI_H2D_SET_MGMT_TXPKT";
#endif /*CONFIG_HOST_P2P*/
        case WCI_H2D_GET_MON:
            return "WCI_H2D_GET_MON";
        default:
            return "Others";
    }
    return "UNKNOWN";
}

const char *wci_event_to_name(enum WCI_RETURN_EVENT wci_evt)
{   
    switch (wci_evt) {
        case WCI_D2H_TGT_RDY:
            return "WCI_D2H_TGT_RDY";
        case WCI_D2H_CONFIG_DONE:
            return "WCI_D2H_CONFIG_DONE";
        case WCI_D2H_MGT_RX:
            return "WCI_D2H_MGT_RX";
        case WCI_D2H_FATAL:
            return "WCI_D2H_FATAL";
        case WCI_D2H_ADD_STA:
            return "WCI_D2H_ADD_STA";
        case WCI_D2H_DEL_STA:
            return "WCI_D2H_DEL_STA";
        case WCI_D2H_ADDBA:
            return "WCI_D2H_ADDBA";
        case WCI_D2H_DELBA:
            return "WCI_D2H_DELBA";
        case WCI_D2H_BAR:
            return "WCI_D2H_BAR";
		case WCI_D2H_RX_MIC_ERR:
			return "WCI_D2H_RX_MIC_ERR";
        case WCI_D2H_RET_VIF:
            return "WCI_D2H_RET_VIF";
        case WCI_D2H_RET_STA:
            return "WCI_D2H_RET_STA";
        case WCI_D2H_RET_TSF:
            return "WCI_D2H_RET_TSF";
        case WCI_D2H_RET_IES:
            return "WCI_D2H_RET_IES";
        case WCI_D2H_RET_TXPOWER:
            return "WCI_D2H_RET_TXPOWER";
        case WCI_D2H_RET_VIF_ATTACH:
            return "WCI_D2H_RET_VIF_ATTACH";
        case WCI_D2H_RET_SCAN_STATE:
            return "WCI_D2H_RET_SCAN_STATE";
        case WCI_D2H_RET_STA_CONNECTED:
            return "WCI_D2H_RET_STA_CONNECTED";
        case WCI_D2H_RET_STA_DISCONNECTED:
            return "WCI_D2H_RET_STA_DISCONNECTED";
        case WCI_D2H_RET_VIF_ADDR:
            return "WCI_D2H_RET_VIF_ADDR";
        case WCI_D2H_RET_VIF_LINK_ST:
            return "WCI_D2H_RET_VIF_LINK_ST";
		case WCI_D2H_RET_MON:
            return "WCI_D2H_RET_MON";
        case WCI_D2H_RET_DBG_INFO:
            return "WCI_D2H_RET_DBG_INFO";
        case WCI_D2H_RET_STA_LINK_ERR:
            return "WCI_D2H_RET_STA_LINK_ERR";

		case WCI_D2H_RET_IOCTL_FW_DEBUG:
			return "WCI_D2H_RET_IOCTL_FW_DEBUG";
        case WCI_D2H_RET_MP_TEST_CMD:
            return "WCI_D2H_RET_MP_TEST_CMD";

        case WCI_D2H_FW_INFO:
            return "WCI_D2H_FW_INFO";
        default:
            break;
    }
    return "UNKNOWN";
}
#endif


/**********************/
/* wci basic function */
/**********************/

static unsigned char lynx_wci_get_seq_no(struct lynx_vif *vif)
{
    u8 seq_no = 0;

    os_api_dsr_lock(&vif->if_lock);
    seq_no = vif->seq_no++;
    os_api_dsr_unlock(&vif->if_lock);

    return seq_no;
}

int lynx_freq2ch(unsigned int freq)
{
    int ch=0;
    switch (freq)
    {
        case 2412:
            ch = 1;
            break;
        case 2417:
            ch = 2;
            break;
        case 2422:
            ch = 3;
            break;
        case 2427:
            ch = 4;
            break;
        case 2432:
            ch = 5;
            break;
        case 2437:
            ch = 6;
            break;
        case 2442:
            ch = 7;
            break;
        case 2447:
            ch = 8;
            break;
        case 2452:
            ch = 9;
            break;
        case 2457:
            ch = 10;
            break;
        case 2462:
            ch = 11;
            break;
        case 2467:
            ch = 12;
            break;
        case 2472:
            ch = 13;
            break;
        case 2484:
            ch = 14;
           break;
#ifdef CONFIG_CH_15
		case 2504:
			ch = 15;
		   break;
#endif
#ifdef CONFIG_LOW_FREQ_CH
		case 2382:
			ch = 29;
		   break;
		case 2387:
			ch = 30;
		   break;
		case 2392:
			ch = 31;
		   break;
		case 2402:
			ch = 32;
		   break;
#endif
        default:
            lynx_dbg(LYNX_DBG_ERR, "unknow channel freq = %d\n", freq);
            ch = -1;
            break;
    }

    return ch;
}


unsigned int lynx_ch_map(unsigned int freq)
{
    unsigned int map;
    int ch = lynx_freq2ch(freq);
    if(ch < 0)
        return 0;

    map =  (1 << (ch - 1));

    return map;
}

int lynx_data_wci_hdr_add(struct sk_buff *skb, struct net_device *dev, enum WCI_CMD_ID cmd_id)
{
    struct lynx_wci_hdr *wci_hdr;
	static unsigned char seq_no=0;
    unsigned int len;
	int ret = 0;
	u8 *ptr=NULL;
    
	if(skb)
	{
		len = skb->len;

		/* FIXME: must sure that "SKB_DATA_OFFSET > sizeof( struct lynx_wci_hdr)" */
		wci_hdr = (struct lynx_wci_hdr *)skb_push(skb, SKB_DATA_OFFSET);

		wci_hdr->cmd_id = cmd_id;
		if (cmd_id >= 128)
			wci_hdr->seq_no = seq_no++;
		wci_hdr->payload_len = len; 
		cpu_to_be16s(&(wci_hdr->payload_len));

		ptr = (u8 *)(wci_hdr + 1);
		memset(ptr, 0, (SKB_DATA_OFFSET - sizeof(struct lynx_wci_hdr)));

		lynx_dbg_dump(LYNX_DBG_WLAN_TX, "data buf after add wci header", "", 
									skb->data, len + sizeof(struct lynx_wci_hdr));
	}
	else
	{
#ifdef CONFIG_LYNX_DEBUG
		lynx_dbg(LYNX_DBG_WLAN_TX, "WCI failure for: %s\n", wci_cmd_to_name(cmd_id));
#endif
		ret = -EINVAL;
	}

    return ret;
}

int wci_progress_check(int operation, int val)
{
	/* check for this case: usb/driver deinited, but wm_manager want send wci command */
	static int working=0; 

	if(operation == 1)	// set val
		working = val;

	return working;
}

#define LYNX_OFFSET 16
#ifdef CONFIG_LYNX_OS_UCOS
#define L1_CACHE_BYTES	0
#endif
struct sk_buff *lynx_wci_get_new_buf(u32 size)
{
    struct sk_buff *skb=NULL;
    u16 reserved;

	if(wci_progress_check(0, 0))
	{
#ifdef CONFIG_LYNX_OS_LINUX
		reserved = roundup((2 * L1_CACHE_BYTES) + LYNX_OFFSET, 4);
#else
		reserved = LYNX_OFFSET;
#endif

		skb = dev_alloc_skb(size + reserved); 
	
		if(skb)
		{
			skb_reserve(skb, reserved - L1_CACHE_BYTES);
			skb_put(skb, size);

			if(size)
				memset(skb->data, 0, size);
		}
	}

    return skb;
}


int lynx_cmd_wci_send(struct lynx *lnx, struct lynx_vif *vif, enum WCI_CMD_ID cmd_id,
                        struct sk_buff *skb, u32 cmd_len, void *rsp_buf, u32 rsp_len)
{
    struct lynx_wci_hdr *wci_hdr;
	int ret = -EINVAL;
#ifdef CONFIG_LYNX_DEBUG
    u16 headroom = sizeof(struct lynx_wci_hdr);
#endif

	if(skb)
	{
		if(!(test_bit(DESTROY_IN_PROGRESS, &lnx->flag)))
		{
			skb_push(skb, sizeof(struct lynx_wci_hdr));
			wci_hdr = (struct lynx_wci_hdr *)skb->data;
			wci_hdr->cmd_id = cmd_id;

			if(vif)
				wci_hdr->seq_no = lynx_wci_get_seq_no(vif);

			wci_hdr->payload_len = cmd_len; 
			cpu_to_be16s(&(wci_hdr->payload_len));
			lynx_dbg_dump(LYNX_DBG_WCI, "cmd buf", "", skb->data, headroom + cmd_len);

			if(lynx_data_queue_is_empty(lnx) != 0)
				ret = lynx_tx_send(lnx, skb);
		
			if(ret)
				ret = lynx_data_enqueue(skb, vif, 1);
		}

		if(ret)
		{
    		kfree_skb(skb);
		}
	}

#ifdef CONFIG_LYNX_DEBUG
	if(ret)
		lynx_dbg(LYNX_DBG_WCI, "WCI send failure for: %s\n", wci_cmd_to_name(cmd_id));
#endif

	return ret;
}

/*======================================================================================+
 |                                                                                      |
 |                                  WCI EVENT TO HOST                                   |
 |                                                                                      |
 +=====================================================================================*/
static void lynx_wci_get_device_resp(struct lynx *lnx, struct lynx_vif *vif, u8 *datap, u32 len)
{
#if defined(CONFIG_LYNX_OS_LINUX)
	/* datap[0] is bss_idx */
	unsigned char *str = datap + 1;
	if(mp_resp)
	{
		/* get response from device schedule error */
		printk(KERN_CRIT "%s schedule error\n", __func__);
		kfree(mp_resp);
		mp_resp = NULL;
	}

	mp_resp = (u8 *)kmalloc(len, GFP_KERNEL);
	memset(mp_resp, 0, len);
	//printk(KERN_CRIT "%s(%d):\n%s", __func__, len, str);

	memcpy(mp_resp, str, len);

	os_wakeup_wait_queue(&lnx->event_wq);
#endif
}

/* FIXME: handle the device ready info */
static int lynx_wci_ready_evt(struct lynx *lnx, u8 *datap, u32 len)
{
	struct wci_evt_tgt_rdy *evt = NULL;
	int ret = -1;

	if(lnx)
	{
		if(len == sizeof(struct wci_evt_tgt_rdy))
		{
			evt = (struct wci_evt_tgt_rdy *)datap;

			lynx_dbg(LYNX_DBG_WCI_EVENT, "%s(): base_mac = %02x:%02x:%02x:%02x:%02x:%02x\n", 
			__FUNCTION__, evt->base_mac[0], evt->base_mac[1], evt->base_mac[2], 
			evt->base_mac[3], evt->base_mac[4], evt->base_mac[5]);
			
			//printk(KERN_CRIT "%s no_k_rule_status:%d\n", __func__,evt->no_k_rule_status);
			printk("MAC Rule:%d\n",(evt->no_k_rule_status & 0x3)+1);
			printk("FOFS Rule:%d\n",(evt->no_k_rule_status >> 2 & 0x3)+1);
			printk("TXVGA Rule:%d\n",(evt->no_k_rule_status >> 4 & 0x3)+1);

			if(is_valid_ether_addr(evt->base_mac))
				memcpy(lnx->mac_addr, evt->base_mac, ETH_ALEN);
			else
			{
				/* FIXME: for error case */
				lnx->mac_addr[0] = 0x0;
				lnx->mac_addr[3] = ((unsigned long)lnx & 0xFF);
				lnx->mac_addr[5] = ((unsigned long)datap & 0xFF);
			}

			set_bit(WCI_READY, &lnx->flag);
			os_wakeup_wait_queue(&lnx->event_wq);
			ret = 0;
		}
	}

    return ret;
}

/* WCI EVENT END */

void lynx_wci_callback(struct lynx *lnx, struct lynx_wci_hdr *cmd, u32 len)
{
    struct lynx_vif *vif=NULL;
    u8 *datap = (u8 *)(cmd + 1);
	len -= sizeof(struct lynx_wci_hdr);
    
	if(cmd->cmd_id > WCI_D2H_WITHOUT_BSSIDX)
	{
        vif = lynx_get_vif_by_index(lnx, datap[0]);

		if(vif == NULL)
		{
            lynx_dbg(LYNX_DBG_WARN,"%s(): cmd_id=%d, bssid=%d, vif mismatch, %s\n", __FUNCTION__, cmd->cmd_id, datap[0],datap);
			return;
		}
	}

    switch(cmd->cmd_id)
    {
        case WCI_D2H_TGT_RDY:
            lynx_dbg(LYNX_DBG_WCI_EVENT,"WCI_D2H_READY_EVENTID\n");
            lynx_wci_ready_evt(lnx, datap, len);
            break;
        case WCI_D2H_RET_SCAN_STATE:
            lynx_dbg(LYNX_DBG_WCI_EVENT,"WCI_D2H_RET_SCAN_STATE\n");
            lynx_wci_scan_complete_evt(vif, datap, len);
            break;
        case WCI_D2H_RET_STA_CONNECTED:
            lynx_dbg(LYNX_DBG_WCI_EVENT,"WCI_D2H_RET_STA_CONNECTED\n");
            lynx_wci_connect_evt(vif, datap, len);
            break;
        case WCI_D2H_RET_STA_DISCONNECTED:
            lynx_dbg(LYNX_DBG_WCI_EVENT,"WCI_D2H_RET_STA_DISCONNECTED\n");
            lynx_wci_disconnect_evt(vif, datap, len);
            break;
        case WCI_D2H_RET_STA_LINK_ERR:
            lynx_dbg(LYNX_DBG_WCI_EVENT,"WCI_D2H_RET_STA_LINK_ERR\n");
            lynx_wci_link_err_evt(vif, datap, len);
            break;
        case WCI_D2H_RET_TXPOWER:
            lynx_dbg(LYNX_DBG_WCI_EVENT,"WCI_D2H_RET_TXPOWER\n");
            lynx_wci_get_tx_pwr_evt(vif, datap, len);
            break;
        case WCI_D2H_RET_VIF_ATTACH:
            lynx_dbg(LYNX_DBG_WCI_EVENT,"WCI_D2H_RET_VIF_ATTACH\n");
            lynx_wci_vif_attach_evt(vif, datap, len);
			break;
        case WCI_D2H_RET_VIF_ADDR:
            lynx_dbg(LYNX_DBG_WCI_EVENT,"WCI_D2H_RET_VIF_ADDR\n");
            lynx_wci_get_vif_addr_evt(vif, datap, len);
			break;
        case WCI_D2H_RET_VIF_LINK_ST:
            lynx_dbg(LYNX_DBG_WCI_EVENT,"WCI_D2H_RET_VIF_LINK_ST\n");
            lynx_wci_get_vif_link_st_evt(vif, datap, len);
			break;
        case WCI_D2H_RX_MIC_ERR:
            lynx_dbg(LYNX_DBG_WCI_EVENT,"WCI_D2H_RX_MIC_ERR\n");
			lynx_wci_mic_err_evt(vif, datap, len);
			break;
		case WCI_D2H_RET_MP_TEST_CMD:
			lynx_dbg(LYNX_DBG_WCI_EVENT,"WCI_D2H_RET_MP_TEST_CMD\n");
			lynx_wci_get_device_resp(lnx, vif, datap, len);
			break;
		case WCI_D2H_RET_IOCTL_FW_DEBUG:
			lynx_dbg(LYNX_DBG_WCI_EVENT,"WCI_D2H_RET_IOCTL_FW_DEBUG\n");
			lynx_wci_get_device_resp(lnx, vif, datap, len);
			break;
#ifdef CONFIG_HOST_DEBUG_DEVICE
		case WCI_D2H_RET_DBG_INFO:
			lynx_dbg(LYNX_DBG_WCI_EVENT,"WCI_D2H_RET_DBG_INFO\n");
			lynx_wci_ret_dbg_info(vif, datap, len);
			break;
#endif
#ifdef CONFIG_SUPPORT_MONITOR_MODE
        case WCI_D2H_RET_MON:
            lynx_dbg(LYNX_DBG_WCI_EVENT,"WCI_D2H_RET_MON\n");
            lynx_wci_get_monitor_evt(vif, datap, len);
            break;
#endif /*CONFIG_SUPPORT_MONITOR_MODE */

			
#if 0
        case WCI_D2H_CONFIG_DONE:
        case WCI_D2H_FATAL:
        case WCI_D2H_ADDED_STA:
            break;
        case WCI_D2H_ADDBA:
            /*FIXME: Not implement*/
            if(cmd->payload_len == sizeof(struct wci_evt_addba))
                lynx_wci_recv_addba_req(priv, (struct wci_evt_addba *)(cmd + 1));
            break;
        case WCI_D2H_DELBA:
            /*FIXME: Not implement*/
            if(cmd->payload_len == sizeof(struct wci_evt_delba))
                lynx_wci_recv_delba_req(priv, (struct wci_evt_delba *)(cmd + 1));
            break;
        case WCI_D2H_BAR:
            /*FIXME: Not implement*/
            if(cmd->payload_len == sizeof(struct wci_evt_bar))
                lynx_wci_recv_bar(priv, (struct wci_evt_bar *)(cmd + 1));
            break;
#endif          
        default:
#ifdef CONFIG_LYNX_DEBUG
            lynx_dbg(LYNX_DBG_WCI_EVENT, "%s:%d Not support event id=%d(%s), seq=%d\n", 
				__func__, __LINE__, cmd->cmd_id, wci_event_to_name(cmd->cmd_id), cmd->seq_no);
#endif
            break;
    }
}


/*======================================================================================+
 |                                                                                      |
 |                                  UTILS                                               |
 |                                                                                      |
 +=====================================================================================*/
#ifdef CONFIG_LYNX_OS_LINUX
void lynx_wci_sscan_timer(unsigned long ptr)
{
#if !defined(CONFIG_LYNX_WM_MANAGER)
    struct lynx_vif *vif = (struct lynx_vif *) ptr;

	if(test_bit(WLAN_ENABLED, &vif->flags))
 	   cfg80211_sched_scan_results(vif->lnx->wiphy, 0);
#endif
}

void create_default_mac(u8 *mac_addr)
{
	int num,i;
	
	get_random_bytes(&i,sizeof(i));
	num= i% 10000 +1;
	mac_addr[0]= 0x80;
	mac_addr[1]= 0x05;
	mac_addr[2]= 0xdf;
	mac_addr[3]= 0x11;
	mac_addr[4]= num & 0xf0;
	mac_addr[5]= num & 0x0f;
	printk("Set default mac(random):%02x %02x %02x %02x %02x %02x\n",mac_addr[0],mac_addr[1],mac_addr[2],mac_addr[3],mac_addr[4],mac_addr[5]);
}

int read_file(const u8 *file_name,u8 *data)
{
	struct file *f;
	char buf[512]={0};
	int ret=0;
	
	if(strlen(no_k_file_path))
	{
		if(no_k_file_path[strlen(no_k_file_path)-1]!='/')
			sprintf(buf,"%s/%s",no_k_file_path,file_name);
		else	
			sprintf(buf,"%s%s",no_k_file_path,file_name);
	}
	else
	{
		if(!strcmp(file_name,"MAC.txt"))
		{
			sprintf(buf,"%s/%s",LYNX_MAC_PATH,file_name);
		}
		else if(!strcmp(file_name,"RF.txt"))
		{
			sprintf(buf,"%s/%s",LYNX_RF_PATH,file_name);
		}
		else
			strcpy(buf,file_name);
	}
	

	f=filp_open(buf,O_RDONLY,0);
	
	if(IS_ERR(f))
	{
		sprintf(data,"%s,%s",buf,"file open error. ");
		return -1;
	}
	else
	{
		memset((char*)buf,0,512);
		
		if(f->f_op->read(f,buf,256,&f->f_pos)>0)
		{
			if(strchr(buf,'\r'))
				buf[strchr(buf,'\r')-buf]=0;
			if(strchr(buf,'\n'))
				buf[strchr(buf,'\n')-buf]=0;
			strcpy(data,buf);
		}
		else
		{
			
			strcpy(data,"read error.");
			ret = -1;
		}
		filp_close(f,NULL);
		return ret;
	}
}
enum no_k_file_list
{
	MAC,
	TXVGA,
};

int store_mac(u8 *mac,u8 *data)
{
	u8 tmp[3]={0},tmp2[13]={0};
	int i=0;
	long num;

	//if(strlen(data)!=12)
	//	return -1;
	//else
	//{
		strcpy(tmp2,data);
		for(i=0;i<6;i++)
		{
			if(strlen(tmp2)>2)
			{
				strncpy(tmp,tmp2,2);
				tmp[2]=0;
				strcpy(tmp2,tmp2+2);
			}
			else
				strcpy(tmp,tmp2);

			if(kstrtol(tmp,16,&num) != 0)
				return -1;
			mac[i]=num;
		}
	//}
	return 0;
}

int store_rf_info(u8 *fofs,u8 *txvga,u8* data)
{
	long val,num;

	u8 tmp[512]={0};
	u8 buf[50]={0};
	u8 msg[256]={0};
	int ret=0;
	remove_spaces(data);
	
	//printk("%s\n",data);
	//get fofs
	if(strstr(data,"freq.="))
	{
		strcpy(tmp,strstr(data,"freq.=")+6);
		if(strchr(tmp,';'))
		{
			tmp[strchr(tmp,';')-(char*)tmp]=0;
		}
		else
			goto freq_no_found;
		if(kstrtol(tmp,10,&val) != 0)
			return -1;
		*fofs=val;
		ret=1;
	}
	else
	{
		freq_no_found:
		sprintf(msg+strlen(msg),"%s\n","freq. not found.");
//		return -1;
	}

	//get txvga string
	if(strstr(data,"txvga="))
	{
		strcpy(tmp,strstr(data,"txvga=")+6);
		if(strchr(tmp,';'))
			tmp[strchr(tmp,';')-(char*)tmp]=0;
		else
			goto txvga_not_found;

		
		while(strchr(tmp,'='))
		{
			//get index
			strncpy(buf,tmp,strchr(tmp,'=')-(char*)tmp);
			buf[strchr(tmp,'=')-(char*)tmp]=0;
			if(kstrtol(buf,10,&num) != 0)
				return -1;
			//get val
			if(!strchr(tmp,','))
			{
				strcpy(buf,strchr(tmp,'=')+1);
			}
			else
			{
				strncpy(buf,strchr(tmp,'=')+1,strchr(tmp,',')-(strchr(tmp,'=')+1));
				buf[strchr(tmp,',')-(strchr(tmp,'=')+1)]=0;
			}

			if(kstrtol(buf,10,&val) != 0)
				return -1;
			txvga[num-1]=val;
			
			if(!strchr(tmp,','))
				break;
			else
				strcpy(tmp,strchr(tmp,',')+1);
		}
		ret|=0x2;
	}
	else
	{
		txvga_not_found:
			sprintf(msg+strlen(msg),"%s\n","txvga. not found.");
		//return -1;
	}
	if(strlen(msg))
	{
		strcpy(data,msg);
	}
	return ret;
}

void get_no_k_data(struct wci_cmd_dev_start *start_data)
{
	s8 data[512]={0};
	u8 *file_name[]={"MAC.txt","RF.txt"};
	int num=0,i=0,j=0;
	u8 fofs=0;
	u8 txvga[15]={0};
	u8 mac[6]={0};
	int txvga_def=50;
	int ret=0;
	start_data->rule_num=0; // bit 0:MAC 1:fofs 2:TXVGA
	num=sizeof(file_name)/sizeof(char*);
	for(i=0; i<num; i++)
	{
		if(read_file(file_name[i],data))
		{
			printk("%s\n",data);
			
			if(i==MAC)  //MAC
			{
				printk("%s","set random mac\n");
				create_default_mac(start_data->mac);
				//memcpy(start_data->mac,mac,6);
			}
			continue;
			//goto solution3;
		}
		else
		{
			switch(i)
			{
				case MAC:
					if(strlen(data)!=12)
					{
						printk("%s\n","MAC file contecnt error.(length less 12)");
						printk("%s","set random mac\n");
						create_default_mac(start_data->mac);
						//memcpy(start_data->mac,mac,6);
						continue;
						//goto solution3;
					}
					else
					{
						store_mac(mac,data);
						memcpy(start_data->mac,mac,6);
						start_data->rule_num=1;
					}
					break;
				case TXVGA:
					ret=store_rf_info(&fofs,txvga,data);
					if(ret)
					{
						if((ret & 0x1)==0x1)//fofs
						{
							start_data->fofs=fofs;
						}

						if((ret & 0x2)==0x2)//txvga
						{
							for(j=0;j<sizeof(txvga);j++)
							{
								if(txvga[j]==txvga_def)
									txvga[j]=0;
								else if(txvga[j]<txvga_def)
									txvga[j]= 16- (txvga_def- txvga[j]);
								else
									txvga[j]= txvga[j]-txvga_def;
							}
							memcpy(start_data->txvga,txvga,14);
						}
						start_data->rule_num |= (ret << 1);
						if(ret !=3)
							printk("%s\n",data);
					}
					else
					{
							printk("%s\n",data);
					}

					break;
			}
		}
	}

	return;

}

void wt_get_no_k_data(u8 *cmd_data)
{
	s8 data[512]={0};
	u8 fofs=0;
	u8 txvga[15]={0};
	u8 new_data[256]={0};
	u8 *file_name[]={"MAC.txt","RF.txt"};
	int num=0,i=0;
	int ret=0;

	num=sizeof(file_name)/sizeof(char*);
	for(i=0; i<num; i++)
	{
		//MAC does not load
		if(i == MAC)
		{
			sprintf(new_data+strlen(new_data)," %s","none");
			continue;
		}
		if(read_file(file_name[i],data))
		{
			printk("%s",data);
			if(i == MAC)
				sprintf(new_data+strlen(new_data)," %s","none");
			else
				sprintf(new_data+strlen(new_data)," %s","none none");
			continue;
			//goto solution3;
		}
		else
		{
			switch(i)
			{
				case MAC:
					if(strlen(data)!=12)
					{
						printk("%s\n","MAC file contecnt error.(length less 12)");
						sprintf(new_data+strlen(new_data)," %s","none");
						continue;
					}
					else
					{
						sprintf(new_data+strlen(new_data)," %s",data);
						//store_mac(mac,data);
						//memcpy(start_data->mac,mac,6);
					}
					break;
				case TXVGA:
					ret=store_rf_info(&fofs,txvga,data);
					if(ret)
					{
						if((ret & 0x1)==0x1)//fofs
							sprintf(new_data+strlen(new_data)," %d",fofs);
						else
							sprintf(new_data+strlen(new_data)," %s","none");

						if((ret & 0x2)==0x2)//txvga
							sprintf(new_data+strlen(new_data)," %s",txvga);
						else
							sprintf(new_data+strlen(new_data)," %s","none");

						if(ret !=3)
							printk("%s\n",data);
					}
					else
					{
							printk("%s\n",data);
					}

					break;
			}
		}

	}
	
	sprintf(cmd_data+strlen(cmd_data),"%s",new_data);

	return;
}
#endif // CONFIG_LYNX_OS_LINUX

/*UTILS END*/

/*======================================================================================+
 |                                                                                      |
 |                                  WCI CMD TO DEVICE                                   |
 |                                                                                      |
 +=====================================================================================*/
void lynx_wci_start_cmd(struct lynx *lnx)
{
    struct sk_buff *skb;
#if defined(CONFIG_LYNX_OS_LINUX)
	struct wci_cmd_dev_start *cmdq;
#endif
    
    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_dev_start));
    if(!skb)
        return ;
#if defined(CONFIG_LYNX_OS_LINUX)
    else
	{
		cmdq= (struct wci_cmd_dev_start *)skb->data;
		get_no_k_data(cmdq);
	}
#endif

#if defined(CONFIG_DISABLE_HT40)
	cmdq->en40ht = 0;
#else
	cmdq->en40ht = 1;
#endif

#if defined(CONFIG_ENABLE_TXOPLIMIT_AP)
	/* update acp params from AP assignned */
	cmdq->en_txoplimit_ap = 1;
#else
	/* FW:JH have to fix TX suspend issue, decrease CWmax is a workaround */
	cmdq->en_txoplimit_ap = 0;
#endif

    lynx_dbg(LYNX_DBG_WCI, "WCI START CMD skb=0x%x, skb->len=%d", skb, skb->len);

    lynx_cmd_wci_send(lnx, NULL ,WCI_H2D_DEV_START, skb, skb->len, NULL, 0);
}

void lynx_wci_stop_cmd(struct lynx *lnx)
{
    struct sk_buff *skb;
    
    skb = lynx_wci_get_new_buf(1);
    if(!skb)
        return ;
    
    lynx_dbg(LYNX_DBG_WCI, "WCI STOP CMD skb=0x%x, skb->len=%d", skb, skb->len);
    lynx_cmd_wci_send(lnx, NULL ,WCI_H2D_DEV_STOP, skb, 0, NULL, 0);

}

void lynx_wci_attach_cmd(struct lynx_vif *vif)
{
    struct wci_cmd_vif_attach *cmdq;
    struct wm_bss *bss = 0;
    struct sk_buff *skb;
	int wm_bss_len=0;
    
	wm_bss_len = (int)((void *)&(bss->WMBSS_END_SECTOR) - (void *)&(bss->WMBSS_START_SECTOR));
    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_vif_attach) + wm_bss_len);
    
    if(skb)
	{
		cmdq = (struct wci_cmd_vif_attach *)skb->data;
		bss = (struct wm_bss *)(cmdq + 1);
		
		lynx_dbg(LYNX_DBG_ERR, "%s(): role=%d, skb->len=%d, wm_bss data len=%d(%p : %p)\n",
				__FUNCTION__, vif->nw_type, skb->len, wm_bss_len, 
				(void *)&(bss->WMBSS_END_SECTOR), (void *)&(bss->WMBSS_START_SECTOR));

		memcpy(bss, &vif->bss, wm_bss_len);

		cmdq->bss_idx = vif->fw_vif_idx;
		cmdq->wm_bss_len = wm_bss_len;
		cpu_to_be16s(&(cmdq->wm_bss_len));

		lynx_dbg_dump(LYNX_DBG_WCI, "WCI ATTACH CMD", "", skb->data, skb->len);

		lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_VIF_ATTACH, skb, skb->len, NULL, 0);
	}
}

void lynx_wci_detach_cmd(struct lynx_vif *vif)
{
    struct wci_cmd_vif_detach *cmdq;
    struct sk_buff *skb;
    
    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_vif_detach));
    if(skb)
	{
		cmdq = (struct wci_cmd_vif_detach *)skb->data;
		cmdq->bss_idx = vif->fw_vif_idx;
		
		lynx_dbg_dump(LYNX_DBG_WCI, "WCI DETACH CMD", "", skb->data, skb->len);
			
		lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_VIF_DETACH, skb, 
							sizeof(struct wci_cmd_vif_detach), NULL, 0);
	}
}

#if defined(CONFIG_LYNX_OS_LINUX)
int lynx_wci_dynamic_access_cmd(struct lynx_vif *vif, struct mont_ioctl_data *cmd)
{
	struct sk_buff *skb;
	unsigned char cmd_name[5] = "", nlen = 0;
	unsigned char bss_idx = vif->fw_vif_idx;
	unsigned char err_str[] = "ERROR: receive fail";
	unsigned char ok_str[] = "OK";
	int timeout = HZ >> 3;//125ms
	int i, resp = 0, ret = -ENOMEM;
	int normal_fw = 0;
	unsigned char wt_valid_resp_cmd[][32] = {
	/* warning: write otp command */
	"mac_addr",
	"dut_pass",
	/* get device status or config*/
	"stat",
	"bbcnt",
	"txvga",
	"fofs",
	"mp_dump",
	"txvga_dump",
	"otp_space",
	"no_k_load",
	"no_k_load_and_write",
	"per",
	"txp_diff",
	};
	unsigned char cmd_name_list[][16]={
	"wt ",
	"mp ",
	"bb ",
	"rf ",
	"wd ",
	".otp ",
	};

	switch(cmd->cmd_id)
	{
		case MONT_IOCTL_WT:
			strcpy(cmd_name,cmd_name_list[0]);
			if(!strcmp((u8 *)cmd->data,"no_k_load") || !strcmp((u8 *)cmd->data,"no_k_load_and_write"))
			{
				wt_get_no_k_data((u8 *)cmd->data);
				if(!strcmp((u8 *)cmd->data,"no_k_load_and_write"))
				{
					sprintf((u8 *)cmd->data+strlen((u8 *)cmd->data)," %d",1);
				}
				cmd->used_length=strlen((u8 *)cmd->data);
				resp=1;
			}
			break;
		case MONT_IOCTL_MP:
			strcpy(cmd_name,cmd_name_list[1]);
			break;
		case MONT_IOCTL_BB:
			strcpy(cmd_name,cmd_name_list[2]);
			break;
		case MONT_IOCTL_RF:
			strcpy(cmd_name,cmd_name_list[3]);
			break;
		case MONT_IOCTL_WD:
			strcpy(cmd_name,cmd_name_list[4]);
			normal_fw = 1;
			break;		
		case MONT_IOCTL_OTP:
			strcpy(cmd_name,cmd_name_list[5]);
			normal_fw = 1;
			break;
		
	}
	//if(cmd->cmd_id == MONT_IOCTL_WT)
	//	strcpy(cmd_name, "wt ");
	//else//MONT_IOCTL_MP
	//	strcpy(cmd_name, "mp ");
	nlen = strlen(cmd_name);

	/* +1 place the '\0' at the end of the string */
		/* +1 place the bss_idx at the start of the data */
	skb = lynx_wci_get_new_buf(cmd->used_length + nlen + 1 + 1);
	if(skb)
	{
		*skb->data = bss_idx;
		memcpy(skb->data + 1, cmd_name, nlen);
		if(cmd->cmd_id >= MONT_IOCTL_WD && cmd->cmd_id <= MONT_IOCTL_OTP)
		{
			memcpy(skb->data + nlen, (char *)cmd->data, cmd->used_length);
		}
		else
			memcpy(skb->data + 1 + nlen, (char *)cmd->data, cmd->used_length + 1);
		*(skb->data + skb->len - 1) = '\0';

		if(cmd->cmd_id == MONT_IOCTL_WT)
		{
			for(i = 1; i < (sizeof(wt_valid_resp_cmd) >> 4); i++)
			{
				if(!strcmp((char *)cmd->data, wt_valid_resp_cmd[i]))
				{
					resp = 1;
					break;
				}
			}
			if(!strncmp((char *)cmd->data, wt_valid_resp_cmd[0], strlen(wt_valid_resp_cmd[0])))
				resp = 1;
		}
		else//MONT_IOCTL_MP
			resp = 1;
		
		//printk(KERN_CRIT "%s(%d), bss_idx %d: %s\n", __func__, skb->len, bss_idx, skb->data + 1);
		//lynx_dbg_dump(LYNX_DBG_WCI, "WCI WT CMD", "", skb->data, skb->len);
		lynx_dbg_dump(LYNX_DBG_WCI, "WCI CMD", "", skb->data, skb->len);
		if(normal_fw)
		{
			lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_IOCTL_FW_DEBUG, skb, skb->len, NULL, 0);
			//printk("Ryan2\n");
		}
		/* after wci send, skb->data and len will add wci header(len + 4)*/
		else
			lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_MP_TEST_CMD, skb, skb->len, NULL, 0);

		ret = 0;
		/* waiting for response string data from device */
		if(resp)
		{
			if((ret = wait_event_interruptible_timeout(((struct lynx *)(vif->lnx))->event_wq, mp_resp, timeout)) <= 0)
			{
				printk(KERN_CRIT "%s wait event condition false, ret=%d\n", __func__, ret);
				if(copy_to_user((char *)cmd->data, err_str, strlen(err_str) + 1) != 0)
					ret = -1;
				cmd->used_length=strlen(err_str);
			}
			else
			{
				//printk(KERN_CRIT "%s return string: %s\n", __func__, mp_resp);
				lynx_dbg_dump(LYNX_DBG_WCI, "return string:", "", mp_resp, strlen(mp_resp));
				/* return string data to host */
				if(copy_to_user((char *)cmd->data, mp_resp, strlen(mp_resp) + 1) != 0)
					ret = -1;
				else
					cmd->used_length=strlen(mp_resp);
			}
			if(mp_resp)
			{
				kfree(mp_resp);
				mp_resp = NULL;
			}
		}
		else 
		{
			if(copy_to_user((char *)cmd->data, ok_str, strlen(ok_str) + 1)!=0)
				ret=-1;
			else
				cmd->used_length=strlen(ok_str);
		}
	}

	return ret;
}
#endif // defined(CONFIG_LYNX_OS_LINUX)

void lynx_wci_update_vif_cmd(struct lynx_vif *vif)
{
    struct wci_cmd_vif_update *cmdq;
    struct wm_bss *bss = 0;
    struct sk_buff *skb;
	int wm_bss_len=0;
    
	wm_bss_len = (int)((void *)&(bss->WMBSS_END_SECTOR) - (void *)&(bss->WMBSS_START_SECTOR));
    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_vif_update) + wm_bss_len);
    
    if(skb)
	{
		cmdq = (struct wci_cmd_vif_update *)skb->data;
		bss = (struct wm_bss *)(cmdq + 1);
		
		lynx_dbg(LYNX_DBG_ERR, "%s(): role=%d, skb->len=%d, wm_bss data len=%d(%p : %p)\n",
				__FUNCTION__, vif->nw_type, skb->len, wm_bss_len, 
				(void *)&(bss->WMBSS_END_SECTOR), (void *)&(bss->WMBSS_START_SECTOR));

		memcpy(bss, &vif->bss, wm_bss_len);

		cmdq->bss_idx = vif->fw_vif_idx;
		cmdq->wm_bss_len = wm_bss_len;
		cpu_to_be16s(&(cmdq->wm_bss_len));

		lynx_dbg_dump(LYNX_DBG_WCI, "WCI VIF UPDATE CMD", "", skb->data, skb->len);

		lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_VIF_UPDATE, skb, skb->len, NULL, 0);
	}
}

int lynx_wci_set_appie_cmd(struct lynx_vif *vif, const u8 *ie, u8 ie_len, enum wci_mgmt_frame_type mgmt_type)
{
    struct wci_cmd_set_ie_pool *cmdq;
    struct sk_buff *skb;
	int ret = -ENOMEM;
    
    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_set_ie_pool) + ie_len);
    if(skb)
	{
		cmdq = (struct wci_cmd_set_ie_pool *)skb->data;
		cmdq->overwrite = 1;
		/* FIXME: not sure fw_vif_idx is the bssid */
		cmdq->bss_idx = vif->fw_vif_idx;
		cmdq->ie_data_len = ie_len;
		cpu_to_be16s(&cmdq->ie_data_len);

		if((ie != NULL) && (ie_len > 0))
			memcpy((char *)cmdq + sizeof(struct wci_cmd_set_ie_pool), ie, ie_len);

		lynx_dbg_dump(LYNX_DBG_WCI, "WCI SET IE CMD", "", skb->data, skb->len);
		lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_SET_IE_POOL, skb, skb->len, NULL, 0);
		ret = 0;
	}

    return ret;
}

int lynx_wci_beginscan_cmd(struct lynx_vif *vif, u32 channel_maps, u32 interval, u8 *ssid, u32 ssid_len)
{
    struct wci_cmd_dev_scan *cmdq;
    u32 size;
    struct sk_buff *skb;
	int ret = -ENOMEM;

    size = sizeof(struct wci_cmd_dev_scan);
	
	if(ssid_len)
		size += (ssid_len + 1);
    
	if((skb = lynx_wci_get_new_buf(size)))
	{
		cmdq = (struct wci_cmd_dev_scan *)skb->data;
		cmdq->bss_idx = vif->fw_vif_idx;
		cmdq->ch_map |= channel_maps;
		cmdq->interval = interval;
		cpu_to_be32s(&cmdq->ch_map);

		if(ssid && ssid_len)
		{
			cmdq->ssid_len = ssid_len;
			memcpy((unsigned char *)&cmdq[1], ssid, ssid_len);
			((unsigned char *)&cmdq[1])[ssid_len] = 0;
		}

		lynx_dbg_dump(LYNX_DBG_WCI, "WCI BEGIN SCAN CMD", "", skb->data, skb->len);
		lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_DEV_SCAN, skb, size, NULL, 0);
		ret = 0;
	}

    return ret;
}

void lynx_wci_addkey_cmd(struct lynx_vif *vif, const u8 *addr, struct lynx_key *key, u32 key_idx, u32 key_type)
{
    struct sk_buff *skb;
    struct wci_cmd_set_key *cmdq;
    u32 size;
    int sta_idx=0;

	size = sizeof(struct wci_cmd_set_key);

	if(key)
		size += key->key_len;

	if(key_type != KEY_TYPE_GLOBAL_KEY)
	{
		/* FIXME: how to find the AP role's sta idx? */
		if((sta_idx = sta_map2idx(vif, addr)) < 0)
		{
			lynx_dbg(LYNX_DBG_ERR, "%s(): can't find the sta idx\n", __FUNCTION__);
			return; 
		}
	}

    skb = lynx_wci_get_new_buf(size);
    if(!skb)
        return ;

    cmdq = (struct wci_cmd_set_key *)skb->data;
    cmdq->bss_idx = vif->fw_vif_idx;
    cmdq->sta_idx = sta_idx;
    cmdq->key_type = key_type;
    cmdq->key_idx = key_idx;

    if(key)
    {
        /*FIXME:Make sure it is a tx key*/
        cmdq->is_txkey = (vif->def_txkey_index == key_idx);
        cmdq->cipher = key->cipher;
        memcpy(cmdq->key, key->key, key->key_len);
    }
    /* key == NULL : del key command */
        
    lynx_dbg_dump(LYNX_DBG_WCI, "WCI SET KEY", "", skb->data, skb->len);

    lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_SET_KEY, skb, size, NULL, 0);
}

int lynx_wci_set_defkey_cmd(struct lynx_vif *vif, int unicast, int multicast, int key_idx)
{
    struct sk_buff *skb;
    struct wci_cmd_set_def_key *cmdq;
    u32 size;
    int sta_idx=0;
    u8 key_type_map=0;

    size = sizeof(struct wci_cmd_set_def_key);

    if(unicast)
    {
        /* FIXME: how to find the AP role's sta idx? */
        if((sta_idx = sta_map2idx(vif, NULL)) >= 0)
			key_type_map |= (1 << KEY_TYPE_PAIRWISE_KEY);
    }

    if(multicast) {
        key_type_map |= (1 << KEY_TYPE_GLOBAL_KEY);
    }
	
	if(key_type_map == 0)
	{
		lynx_dbg(LYNX_DBG_WCI, "%s(): set def key idx err, unicast=%d, multicast=%d, sta_idx=%d\n"
						, __FUNCTION__, unicast, multicast, sta_idx);
		return -EINVAL;
	}

    skb = lynx_wci_get_new_buf(size);
    if(!skb)
        return -ENOMEM;

    cmdq = (struct wci_cmd_set_def_key *)skb->data;
    cmdq->bss_idx = vif->fw_vif_idx;
    cmdq->sta_idx = sta_idx;
    cmdq->key_type_map = key_type_map;
    cmdq->key_idx = key_idx;
        
    lynx_dbg_dump(LYNX_DBG_WCI, "WCI SET DEFAULT KEY", "", skb->data, skb->len);

    lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_SET_DEF_KEY, skb, size, NULL, 0);

    return 0;
}

int lynx_wci_sta_remove_cmd(struct lynx_vif *vif, unsigned int sta_idx, u32 reason)
{
    struct sk_buff *skb;
    struct wci_cmd_remove_sta *cmdq;
    
    lynx_dbg(LYNX_DBG_WCI, "%s(): vif=0x%x, sta_idx=0x%x, reason=%d\n", 
                __FUNCTION__, (unsigned long)vif, sta_idx, reason);

    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_remove_sta));
    if(!skb)
        return -ENOMEM;

    cmdq = (struct wci_cmd_remove_sta *)skb->data;

    cmdq->bss_idx = vif->fw_vif_idx;
	cmdq->sta_idx = sta_idx;
    cmdq->reason = reason;

    lynx_dbg_dump(LYNX_DBG_WCI, "WCI STA REMOVE CMD", "", skb->data, skb->len);

    return lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_STA_REMOVE, skb, skb->len, NULL, 0);
}

int lynx_wci_connect_cmd(struct lynx_vif *vif, char *bssid, char *ssid, u32 ssid_len, 
						struct wlan_ie_info *info)
{
    struct lynx *lnx = vif->lnx;
    struct sk_buff *skb;
    struct wci_cmd_sta_connect *cmdq;
	int rsn_ie_len=0, wpa_ie_len=0;
	unsigned char *pos;
    
    lynx_dbg(LYNX_DBG_WCI, "wci connect bssid %pM freq %d ssid_len %d "
                "dot11_auth %d auth %d\n", vif->req_bssid, vif->ch_hint, vif->ssid_len,
                vif->dot11_auth_mode, vif->auth_mode);

	if(info)
	{
		if(info->rsn_ie)
			rsn_ie_len = ((struct wlan_ie_generic *)info->rsn_ie)->len + 2;
		if(info->wpa_ie)
			wpa_ie_len = ((struct wlan_ie_generic *)info->wpa_ie)->len + 2;
	}

    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_sta_connect) + ssid_len + 
								rsn_ie_len + wpa_ie_len);
    if(!skb)
        return -ENOMEM;

    cmdq = (struct wci_cmd_sta_connect *)skb->data;
    cmdq->bss_idx = vif->fw_vif_idx;
    cmdq->ssid_len = ssid_len;
    cmdq->rsn_ie_len = rsn_ie_len;
    cmdq->wpa_ie_len = wpa_ie_len;
    cmdq->channel = lnx->channel;

	if(info)
	{
		cmdq->supp_rates = info->supp_rates;
		cmdq->supp_rates = cpu_to_be32(info->supp_rates);
	}

    memcpy(cmdq->bssid, bssid, ETH_ALEN);
	pos = skb->data + sizeof(struct wci_cmd_sta_connect);

	if(ssid)
	{
    	memcpy(pos, ssid, ssid_len);
		pos += ssid_len;
	}
	if(rsn_ie_len)
	{
		memcpy(pos, info->rsn_ie, rsn_ie_len);
		pos += rsn_ie_len;
	}
	if(wpa_ie_len)
	{
		memcpy(pos, info->wpa_ie, wpa_ie_len);
		pos += wpa_ie_len;
	}

    lynx_dbg_dump(LYNX_DBG_WCI, "WCI CONNECT CMD", "", skb->data, skb->len);

    return lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_STA_CONNECT, skb, skb->len, NULL, 0);
}

int lynx_wci_disconnect_cmd(struct lynx_vif *vif, u8 sta_idx, u16 reason_code)
{
    struct sk_buff *skb;
    struct wci_cmd_sta_disconnect *cmdq;
    int ret = -ENOMEM;
#if 0   
    lynx_dbg(LYNX_DBG_WCI, "%s(): bss_idx=%d, sta_idx=%d, reason_code=%d\n",
                __FUNCTION__, bss_idx, sta_idx, reason_code);
#endif
    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_sta_disconnect));
    if(skb)
	{
		/* FIXME: What is the default reason code? */
		/*  8: Disassociated because sending STA is leaving (or has left) BSS */
		if(reason_code == 0)
			reason_code = 8;

		cmdq = (struct wci_cmd_sta_disconnect *)skb->data;
		cmdq->bss_idx = vif->fw_vif_idx;
		cmdq->sta_idx = sta_idx;
		cmdq->reason_code = cpu_to_be16(reason_code);

		lynx_dbg_dump(LYNX_DBG_WCI, "WCI DISCONNECT CMD", "", skb->data, skb->len);

		ret = lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_STA_DISCONNECT, skb, skb->len, NULL, 0);
	}

    return ret;
}

int lynx_wci_set_tx_pwr_cmd(struct lynx *lnx, unsigned int tx_pwr)
{
    struct sk_buff *skb;
    struct wci_cmd_set_tx_power *cmdq;
    
    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_set_tx_power));
    if(!skb)
        return -ENOMEM;

    cmdq = (struct wci_cmd_set_tx_power *)skb->data;
    cmdq->tx_power = tx_pwr;
    
    lynx_dbg_dump(LYNX_DBG_WCI, "WCI SET TX PWR CMD", "", skb->data, skb->len);

    return lynx_cmd_wci_send(lnx, NULL, WCI_H2D_SET_TX_POWER, skb, skb->len, NULL, 0);
}

int lynx_wci_get_tx_pwr_cmd(struct lynx_vif *vif)
{
    struct sk_buff *skb;
    struct wci_cmd_get_tx_power *cmdq;
    
    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_get_tx_power));
    if(!skb)
        return -ENOMEM;

    cmdq = (struct wci_cmd_get_tx_power *)skb->data;
    cmdq->bss_idx = vif->fw_vif_idx;
    
    lynx_dbg_dump(LYNX_DBG_WCI, "WCI GET TX PWR CMD", "", skb->data, skb->len);

    return lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_GET_TX_POWER, skb, skb->len, NULL, 0);
}

int lynx_wci_set_pmksa_cmd(struct lynx_vif *vif, u8 *bssid, u8 *pmkid, u8 is_add)
{
    struct sk_buff *skb;
    struct wci_cmd_set_pmksa *cmdq;
    
    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_set_pmksa));
    if(!skb)
        return -ENOMEM;

    cmdq = (struct wci_cmd_set_pmksa *)skb->data;
    cmdq->bss_idx = vif->fw_vif_idx;
    if(bssid)
        memcpy(cmdq->bssid, bssid, ETH_ALEN);
    if(pmkid)
        memcpy(cmdq->pmkid, bssid, PMKID_LEN);
    
    if(is_add)
        cmdq->action = PMKSA_ADD;
    else if(pmkid)
        cmdq->action = PMKSA_DEL;
    else
        cmdq->action = PMKSA_FLUSH;
    
    lynx_dbg_dump(LYNX_DBG_WCI, "WCI SET PMKSA CMD", "", skb->data, skb->len);


    return lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_SET_PMKSA, skb, skb->len, NULL, 0);
}

int lynx_wci_get_vif_addr_cmd(struct lynx_vif *vif)
{
    struct sk_buff *skb;
    struct wci_cmd_get_vif_addr *cmdq;
    
    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_get_vif_addr));
    if(!skb)
        return -ENOMEM;

    cmdq = (struct wci_cmd_get_vif_addr *)skb->data;
    cmdq->bss_idx = vif->fw_vif_idx;
    
    lynx_dbg_dump(LYNX_DBG_WCI, "WCI GET VIF ADDR CMD", "", skb->data, skb->len);

    return lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_GET_VIF_ADDR, skb, skb->len, NULL, 0);
}

int lynx_wci_get_vif_link_st_cmd(struct lynx_vif *vif)
{
    struct sk_buff *skb;
    struct wci_cmd_get_vif_link_st *cmdq;
    
    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_get_vif_link_st));
    if(!skb)
        return -ENOMEM;

    cmdq = (struct wci_cmd_get_vif_link_st *)skb->data;
    cmdq->bss_idx = vif->fw_vif_idx;
   
    lynx_dbg_dump(LYNX_DBG_WCI, "WCI GET VIF LINK ST CMD", "", skb->data, skb->len);

    return lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_GET_VIF_LINK_ST, skb, skb->len, NULL, 0);
}

int lynx_wci_set_beacon_cmd(struct lynx_vif *vif)
{
    struct sk_buff *skb;
    struct wci_cmd_set_beacon *cmdq;
    
    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_set_beacon));
    if(!skb)
        return -ENOMEM;

    cmdq = (struct wci_cmd_set_beacon *)skb->data;
    cmdq->bss_idx = vif->fw_vif_idx;
    
    lynx_dbg_dump(LYNX_DBG_WCI, "WCI SET BEACON CMD", "", skb->data, skb->len);

    return lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_SET_BEACON, skb, skb->len, NULL, 0);
}

int lynx_wci_set_rts_cmd(struct lynx_vif *vif, u32 rts_threshold)
{
    struct sk_buff *skb;
    struct wci_cmd_set_rts *cmdq;
    
    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_set_rts));
    if(!skb)
        return -ENOMEM;

    cmdq = (struct wci_cmd_set_rts *)skb->data;
    cmdq->bss_idx = vif->fw_vif_idx;
	cmdq->rts_threshold = rts_threshold;
    
    lynx_dbg_dump(LYNX_DBG_WCI, "WCI SET RTS THRESHOLD CMD", "", skb->data, skb->len);

    return lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_SET_RTS_THRESHOLD, skb, skb->len, NULL, 0);
}

int lynx_wci_set_2040_coex_cmd(struct lynx_vif *vif, int enable)
{
	struct sk_buff *skb;
	struct wci_cmd_set_2040_coex *cmdq;
    
	skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_set_2040_coex));
	if(!skb)
		return -ENOMEM;

	cmdq = (struct wci_cmd_set_2040_coex *)skb->data;
	cmdq->bss_idx = vif->fw_vif_idx;
	cmdq->en_2040_coex = enable;
    
	lynx_dbg_dump(LYNX_DBG_WCI, "WCI SET 20/40 COEX CMD", "", skb->data, skb->len);

	return lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_SET_2040_COEX, skb, skb->len, NULL, 0);
}

int lynx_wci_start_ap_cmd(struct lynx_vif *vif)
{
	struct sk_buff *skb;
	struct wm_bss *bss = &vif->bss;
	struct wci_cmd_start_ap *cmdq;
	unsigned int supp_rates=0;
    
    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_start_ap));
    if(!skb)
        return -ENOMEM;

    cmdq = (struct wci_cmd_start_ap *)skb->data;
    cmdq->bss_idx = vif->fw_vif_idx;

	if(bss->phy_cap & AP_CAP_11B)
		supp_rates |= B_RATES;
	if(bss->phy_cap & AP_CAP_11G)
		supp_rates |= OFDM_RATES;
	if(bss->phy_cap & AP_CAP_11N)
		supp_rates |= MCS_RATES;
	cmdq->current_rate = cpu_to_be32(supp_rates);
    
    lynx_dbg_dump(LYNX_DBG_WCI, "WCI START AP CMD", "", skb->data, skb->len);

    return lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_START_AP, skb, skb->len, NULL, 0);
}

int lynx_wci_set_channel_cmd(struct lynx *lnx)
{
    struct sk_buff *skb;
	struct device_configs *fw = &lnx->fw_config;
    struct wci_cmd_set_channel *cmdq;
    struct lynx_vif *vif=NULL;
	int bss_idx = 0;

    vif = lynx_vif_first(lnx);
	if(vif)
		bss_idx = vif->fw_vif_idx;

    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_set_channel));
    if(!skb)
        return -ENOMEM;

    cmdq = (struct wci_cmd_set_channel *)skb->data;
    cmdq->bss_idx = bss_idx;
	cmdq->bandwidth = fw->bandwidth;
	cmdq->channel = fw->channel;
    
    lynx_dbg_dump(LYNX_DBG_WCI, "WCI SET CHANNEL CMD", "", skb->data, skb->len);

    return lynx_cmd_wci_send(lnx, vif, WCI_H2D_SET_CHANNEL, skb, skb->len, NULL, 0);
}

int lynx_wci_set_min_rateidx(struct lynx *lnx)
{
	struct sk_buff *skb;
	struct device_configs *fw = &lnx->fw_config;
	struct wci_cmd_set_min_rateidx *cmdq;
	struct lynx_vif *vif=NULL;

	vif = lynx_vif_first(lnx);

	skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_set_min_rateidx));
	if (!skb)
		return -ENOMEM;

	cmdq = (struct wci_cmd_set_min_rateidx *)skb->data;
	cmdq->min_rate_idx = fw->min_rate_idx;

	lynx_dbg_dump(LYNX_DBG_WCI, "WCI SET MIN RATEIDX CMD", "", skb->data, skb->len);

	return lynx_cmd_wci_send(lnx, vif, WCI_H2D_SET_MIN_RATEIDX, skb, skb->len, NULL, 0);
}

int lynx_wci_set_power_saving(struct lynx *lnx)
{
	struct sk_buff *skb;
	struct device_configs *fw = &lnx->fw_config;
	struct wci_cmd_set_ps *cmdq;
	struct lynx_vif *vif=NULL;

	vif = lynx_vif_first(lnx);

	skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_set_ps));
	if (!skb)
		return -ENOMEM;

	cmdq = (struct wci_cmd_set_ps *)skb->data;
	cmdq->power_saving_mode = fw->power_saving_mode;

	lynx_dbg_dump(LYNX_DBG_WCI, "WCI SET POWER SAVING CMD", "", skb->data, skb->len);

	return lynx_cmd_wci_send(lnx, vif, WCI_H2D_SET_PS, skb, skb->len, NULL, 0);
}

int lynx_wci_wds_peer_add_cmd(struct lynx_vif *vif, int wds_mode, u8 *peer_addr)
{
    struct sk_buff *skb;
    struct wci_cmd_wds_peer_add *cmdq;
    
    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_wds_peer_add));
    if(!skb)
        return -ENOMEM;

    cmdq = (struct wci_cmd_wds_peer_add *)skb->data;
    cmdq->bss_idx = vif->fw_vif_idx;
	cmdq->wds_mode = wds_mode;
	memcpy(cmdq->peer_addr, peer_addr, WLAN_ADDR_LEN);
    
    lynx_dbg_dump(LYNX_DBG_WCI, "WCI WDS PEER ADD CMD", "", skb->data, skb->len);

    return lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_WDS_PEER_ADD, skb, skb->len, NULL, 0);
}

int lynx_wci_set_mac_cmd(struct lynx_vif *vif, u8 *addr)
{
    struct sk_buff *skb;
    struct wci_cmd_set_mac *cmdq;
    
    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_set_mac));
    if(!skb)
        return -ENOMEM;

    cmdq = (struct wci_cmd_set_mac *)skb->data;
    cmdq->bss_idx = vif->fw_vif_idx;
	memcpy(cmdq->addr, addr, WLAN_ADDR_LEN);
    
    lynx_dbg_dump(LYNX_DBG_WCI, "WCI SET MAC CMD", "", skb->data, skb->len);

    return lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_SET_MAC, skb, skb->len, NULL, 0);
}

int lynx_wci_send_addba_cmd(struct lynx_vif *vif, int sta_idx, int tid)
{
    struct sk_buff *skb;
    struct wci_cmd_send_addba *cmdq;
    
    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_send_addba));
    if(!skb)
        return -ENOMEM;

    cmdq = (struct wci_cmd_send_addba *)skb->data;
    cmdq->bss_idx = vif->fw_vif_idx;
    cmdq->sta_idx = sta_idx;
    cmdq->tid = tid;
    
    lynx_dbg_dump(LYNX_DBG_WCI, "WCI SEND ADDBA CMD", "", skb->data, skb->len);

    return lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_SEND_ADDBA, skb, skb->len, NULL, 0);
}

#ifdef CONFIG_HOST_P2P
int lynx_wci_set_mgmt_txpkt_cmd(struct lynx_vif *vif, const u8 *pkt, u16 pkt_len,u8 mgmt_type)
{
    struct wci_cmd_set_mgmt_txpkt *cmdq;
    struct sk_buff *skb;
    int ret = -ENOMEM;

    if ( pkt && pkt_len) {
            skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_set_mgmt_txpkt) + pkt_len);
            if(skb){
                        cmdq = (struct wci_cmd_set_mgmt_txpkt *)skb->data;
                        cmdq->bss_idx                     = vif->fw_vif_idx;
                        cmdq->mgmt_frm_type       = mgmt_type;

                        cmdq->pkt_data_len            = pkt_len;
                        cpu_to_be16s(&cmdq->pkt_data_len);

                        if((pkt != NULL) && (pkt_len > 0))
                                memcpy((char *)cmdq + sizeof(struct wci_cmd_set_mgmt_txpkt), pkt, pkt_len);

                        lynx_dbg_dump(LYNX_DBG_WCI, "WCI SET MGMT TXPKT CMD", "", skb->data, skb->len);
                        lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_SET_MGMT_TXPKT, skb, skb->len, NULL, 0);
                        ret = 0;
             }
     }
    return ret;
}
#endif /*CONFIG_HOST_P2P*/

#if defined(CONFIG_HOST_WPS) || defined(CONFIG_CUST1_PRIV_BEACON_IE)
int lynx_wci_set_mgmt_ie_cmd(struct lynx_vif *vif, const u8 *ie, u16 ie_len,u8 mgmt_type)
{
    struct wci_cmd_set_mgmt_ie *cmdq;
    struct sk_buff *skb;
    int ret = 0;
    u8 overwrite = 1;
    //if (ie&& ie_len) {
    if (1) {
            skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_set_mgmt_ie) + ie_len);
            if(skb){
                        cmdq = (struct wci_cmd_set_mgmt_ie *)skb->data;
                        cmdq->bss_idx             = vif->fw_vif_idx;
                        cmdq->mgmt_frm_type       = mgmt_type;
                        cmdq->overwrite           = overwrite;              
                        cmdq->ie_data_len         = ie_len;
                        cpu_to_be16s(&cmdq->ie_data_len);

                        if((ie != NULL) && (ie_len > 0))
                                memcpy((char *)cmdq + sizeof(struct wci_cmd_set_mgmt_ie), ie, ie_len);

                        //lynx_dbg_dump(LYNX_DBG_WCI, "WCI SET MGMT IE CMD", "", skb->data, skb->len);
                        lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_SET_MGMT_IE, skb, skb->len, NULL, 0);
                        ret = 0;
             }
     }
    return ret;
}
#endif /*CONFIG_HOST_WPS*/


#ifdef CONFIG_SUPPORT_MONITOR_MODE
int lynx_wci_set_monitor_cmd(struct lynx_vif *vif,u32 mon_mask)
{
    struct wci_cmd_set_mon *cmdq;
    struct sk_buff *skb;
    int ret = -ENOMEM;
    printk("Set FW Monitor Mask=0x%08x\n", mon_mask); 

    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_set_mon) );
    if(skb){
           cmdq = (struct wci_cmd_set_mon *)skb->data;
           cmdq->mon_mask = cpu_to_be32(mon_mask);
           lynx_dbg_dump(LYNX_DBG_WCI, "WCI SET MON CMD", "", skb->data, skb->len);

           lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_SET_MON, skb, skb->len, NULL, 0);
           ret = 0;
    }

    return ret;
}


int lynx_wci_get_monitor_cmd(struct lynx_vif *vif)
{
    struct sk_buff *skb;
    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_set_mon) ); /*NO payload ,set just for pass skb opertioan*/
    if(!skb)
        return -ENOMEM;
   
    lynx_dbg_dump(LYNX_DBG_WCI, "WCI GET MON CMD", "", skb->data, skb->len);
    return lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_GET_MON, skb, skb->len, NULL, 0);
}
#endif /*CONFIG_SUPPORT_MONITOR_MODE */

void lynx_wci_update_chan_band(struct lynx_vif *vif,enum nl80211_channel_type superchan ,unsigned int channel,int skip_band)
{
	struct lynx *lnx = vif->lnx;
	struct device_configs *fw = &lnx->fw_config;
	struct wm_bss *bss = NULL;

	lynx_dbg(LYNX_DBG_WLAN_CFG, "lynx_update_chan_band() start\n");
	fw->channel = lnx->channel = channel;

	if(!skip_band){
#ifdef CONFIG_UNIT_TEST
		unittest_bss_compare(vif, __FUNCTION__);
#endif
		bss = &vif->bss;
		switch(superchan){
			case NL80211_CHAN_NO_HT:
				bss->phy_cap &= ~AP_CAP_11N;
				fw->bandwidth = BW40MHZ_SCN;
				break;
			case NL80211_CHAN_HT20:
				bss->phy_cap |= AP_CAP_11N;
				fw->bandwidth = BW40MHZ_SCN;
				break;
			case NL80211_CHAN_HT40MINUS:
				bss->phy_cap |= AP_CAP_11N;
				fw->bandwidth = BW40MHZ_SCB;
				break;
			case NL80211_CHAN_HT40PLUS:
				bss->phy_cap |= AP_CAP_11N;
				fw->bandwidth = BW40MHZ_SCA;
				break;
		}

		/* update bss->ht_capability */
		bss->ht_capability = 0;
		if(bss->phy_cap & AP_CAP_11N){
			bss->ht_capability = IEEE80211_HT_CAP_SGI_20;
			if(fw->bandwidth != BW40MHZ_SCN){
				bss->ht_capability |= 
					(IEEE80211_HT_CAP_DSSSCCK40|IEEE80211_HT_CAP_SUP_WIDTH_20_40|
					 IEEE80211_HT_CAP_GRN_FLD|IEEE80211_HT_CAP_SGI_40);
			}
			cpu_to_be16s(&(bss->ht_capability));
		}
#ifdef CONFIG_UNIT_TEST
		unittest_bss_backup(vif, __FUNCTION__);
#endif
		/* wci update bss */
		lynx_dbg(LYNX_DBG_WLAN_CFG, "update vif for bandbwidth\n");
		lynx_wci_update_vif_cmd(vif);
	}
	
	/* wci set channel */
	lynx_wci_set_channel_cmd(lnx);
	lynx_dbg(LYNX_DBG_WLAN_CFG, "lynx_update_chan_band() OK\n");
}

#ifdef CONFIG_HOST_DEBUG_DEVICE
int lynx_wci_dbg_info_cmd(struct lynx_vif *vif, int is_get, int item_id, int len, char *data)
{
    struct sk_buff *skb;
    struct wci_cmd_dbg_info *cmdq;
    
    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_dbg_info) + len);
    if(!skb)
        return -ENOMEM;

    cmdq = (struct wci_cmd_dbg_info *)skb->data;
    cmdq->bss_idx = vif->fw_vif_idx;
    cmdq->is_get = is_get;
    cmdq->item_id = cpu_to_be16(item_id);
	if(len && data)
	{
    	cmdq->payload_len = cpu_to_be16(len);
		memcpy(&cmdq[1], data, len);
	}
	else
    	cmdq->payload_len = 0;
    
    lynx_dbg_dump(LYNX_DBG_WCI, "WCI H2D DBG INFO CMD", "", skb->data, skb->len);

    return lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_DBG_INFO_CMD, skb, skb->len, NULL, 0);
}
#endif	// CONFIG_HOST_DEBUG_DEVICE

int lynx_wci_set_special_param_cmd(struct lynx_vif *vif, int item_id, int len, char *data)
{
    struct sk_buff *skb;
    struct wci_cmd_set_special_param *cmdq;
    
    skb = lynx_wci_get_new_buf(sizeof(struct wci_cmd_set_special_param) + len);
    if(!skb)
        return -ENOMEM;

    cmdq = (struct wci_cmd_set_special_param *)skb->data;
    cmdq->bss_idx = vif->fw_vif_idx;
    cmdq->item_id = cpu_to_be16(item_id);
	if(len && data)
	{
    	cmdq->payload_len = cpu_to_be16(len);
		memcpy(&cmdq[1], data, len);
	}
	else
    	cmdq->payload_len = 0;
    
    lynx_dbg_dump(LYNX_DBG_WCI, "WCI H2D SET SPECIAL PARAM CMD", "", skb->data, skb->len);

    return lynx_cmd_wci_send(vif->lnx, vif, WCI_H2D_SET_SPECIAL_PARAM, skb, skb->len, NULL, 0);
}

void lynx_wci_bulk_out_sync_pid(struct lynx *lnx)
{
	struct sk_buff *skb;
	skb = lynx_wci_get_new_buf(1);

	if(!skb)
		return;

	lynx_cmd_wci_send(lnx, NULL, WCI_H2D_SYNC_PID, skb, skb->len, NULL, 0);
}

/*WCI CMD END*/

