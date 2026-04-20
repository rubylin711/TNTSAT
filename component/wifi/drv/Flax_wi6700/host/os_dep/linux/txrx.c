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
#include <linux/etherdevice.h>

#if defined(CONFIG_LYNX_OS_LINUX) && !defined(CONFIG_LYNX_WM_MANAGER)
#include <net/mac80211.h>
#endif

#include "init.h"
#include "wlan_def.h"
#include "core.h"
#include "vfc.h"
#include "wci.h"
#include "txrx.h"
#include "hif.h"
#include "lynx_debug.h"

#if defined(CONFIG_LYNX_WM_MANAGER)
#include "wm_msg.h"
#endif

#ifdef CONFIG_SUPPORT_MONITOR_MODE
#include <net/ieee80211_radiotap.h>
#endif /*CONFIG_SUPPORT_MONITOR_MODE */

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#define MAX_MSDU_SUBFRAME_PAYLOAD_LEN   1508
#define MIN_MSDU_SUBFRAME_PAYLOAD_LEN   46 

#define HT_RX_REORDER_BUF_TIMEOUT (HZ / 10)

#ifndef IFF_UP
#define	IFF_UP		0x1		/* interface is up		*/
#endif


#ifdef CONFIG_SUPPORT_MONITOR_MODE
struct lynx_rx_radiotap_hdr {
	struct ieee80211_radiotap_header hdr;
	/* channel,mhz */
	__le16 chnl_freq __aligned(2);
	__le16 chnl_flags;

	s8 dbmsignal; /*dbM*/
} __packed;

#define RX_RADIOTAP_PRESENT (			\
	(1 << IEEE80211_RADIOTAP_CHANNEL) |\
	(1 << IEEE80211_RADIOTAP_DBM_ANTSIGNAL) |\
	0)
#endif /*CONFIG_SUPPORT_MONITOR_MODE */

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

/************************/
/*    common function   */
/************************/
static void lynx_deliver_frames_to_nw_stack(struct net_device *dev, struct sk_buff *skb)
{
	struct lynx_vif *vif = netdev_priv(dev);

	if(!skb)
		return;

	if(!(dev->flags & IFF_UP)) 
	{
		dev_kfree_skb(skb);
		return;
	}

	skb->dev = dev;      
	skb->protocol = eth_type_trans(skb, skb->dev); 
	memset(skb->cb, 0, sizeof(skb->cb));

	vif->net_stats.rx_packets++;
	vif->net_stats.rx_bytes += skb->len;

	lynx_dbg_dump(LYNX_DBG_USB_RX, "lynx_deliver_frames_to_nw_stack", "rx ", skb->data, skb->len);

	netif_rx_ni(skb);    
}

struct sk_buff *lynx_forward_check(struct lynx_vif *vif, struct sk_buff *skb)
{
    struct lynx *lnx = vif->lnx;
	struct lynx_sta *sta;
    struct sk_buff *skb_multicast = NULL;
	struct ethhdr *ehdr = (struct ethhdr *)skb->data;
	int i;

	if(ntohs(ehdr->h_proto) == ETHERTYPE_EAPOL)
		return 0;

	if(is_multicast_ether_addr(ehdr->h_dest))
	{
		skb_multicast = skb_copy(skb, GFP_ATOMIC);
		return skb_multicast;
	}

	for(i=0; i<LYNX_STA_MAX_NUM; i++)
	{
		if(vif->sta_idx_map & (1 << i))
		{
			sta = &lnx->sta_list[i];
			if(sta->flags & LYNX_STA_VALID)
			{
				if(memcmp(sta->addr, ehdr->h_dest, ETH_ALEN) == 0)
					return skb;
			}
		}
	}

	return NULL;
}

/************************/
/*     rx function      */
/************************/

void lynx_rx_tasklet(unsigned long data)
{
	struct lynx_hif_device *hdev = (struct lynx_hif_device *)data;
	struct lynx *lynx = (struct lynx *)hdev->lynx;
	struct lynx_rx_hdr *rx_hdr=NULL;
	struct wrb *wrb=NULL;
	struct lynx_vif *vif = NULL;
	unsigned long flags;
	struct sk_buff *skb=NULL, *skb_forward=NULL;
	unsigned char host, wh, ra_idx, data_off;
	unsigned short pkt_len;
	struct ieee80211_mgmt *mgmt;
#ifdef CONFIG_LYNX_WM_MANAGER
	struct wm_msg *msg;
#else
	s32 notify_signal;
	struct ieee80211_channel *channel;
	struct cfg80211_bss *bss;
	unsigned short freq;
#endif

#ifdef CONFIG_ANDROID
	/* Android needs the timestamp fields as the frame recevied time */
	struct timespec64 ts;
#endif
	u16 mgmt_type;

    while (1) 
    {
    	os_api_isr_lock(&hdev->rx_lock, &flags);
    	skb = __skb_dequeue(&hdev->rx_skb_queue);
    	os_api_isr_unlock(&hdev->rx_lock, &flags);

		if(skb == NULL)
			break;

        rx_hdr = (struct lynx_rx_hdr *) skb->data;
		wrb = &rx_hdr->rx;
		host = WRB_BIT_HOST(wrb);
		wh = WRB_BIT_WH(wrb); 
		ra_idx = WRB_RAIDX(wrb);
		data_off = WRB_DATAOFF(wrb);
		pkt_len = WRB_PKTLEN(wrb);

        lynx_dbg(LYNX_DBG_WLAN_RX, "%s:%d (host, wh, ra_index, dataoff, pkt_len)=(%d:%d:%d:%d:%d)"
				"\n", __func__, __LINE__, host , wh, ra_idx, data_off, pkt_len);

        lynx_dbg_dump(LYNX_DBG_WLAN_RX, "lynx_rx_tasklet input", "rx ", skb->data, skb->len);

        /* data or mgmt*/
        if(host) 
        {
			if(!test_bit(WCI_READY, &lynx->flag))
			{
				lynx_dbg(LYNX_DBG_WLAN_RX, "%s(): wci is not ready\n", __FUNCTION__);
                goto skb_free;
			}

			if((pkt_len + data_off) != skb->len)
			{
				lynx_dbg(LYNX_DBG_WLAN_RX, "%s(): error skb, skb->len=%d, wrb->pkt_len=%d, "
						"wrb->data_off=%d\n", __FUNCTION__, skb->len, pkt_len, data_off);
				goto skb_free;
			}

            /*FIXME: device should return correct ra_index to let host found the valid vif */
            vif = lynx_get_vif_by_index(lynx, ra_idx);
                
            if(vif == NULL)
			{
				/* FIXME: lynx_vif_first() & only for debug. 
						  the frame with invalid ra_index should be dropped */
				if (!list_empty(&lynx->vif_list)) 
    				vif = list_first_entry(&lynx->vif_list, struct lynx_vif, list);
				else
				{
					lynx_dbg(LYNX_DBG_WLAN_RX, "%s(): can't get the vif\n", __FUNCTION__);
                	goto skb_free;
				}
			}
            
            /* Temporarily use WRB_B0_WH to determine data or mgmt frame.
             * 0:data ; 1:mgmt                                              */

            /* to the real frame head
             * mgmt: no LLC, head is start at wifi header
             * data: have LLc, head start at ether header                   */  
            skb_pull(skb, data_off);

#ifdef CONFIG_SUPPORT_MONITOR_MODE
			if (vif->lnx->fw_monitor_en == 1)
			{
				//ly_dbg(LYNX_DBG_WLAN_RX, "%s():monitor mode get  pkgt \n", __FUNCTION__);
				// create the  radio header 
				struct lynx_rx_radiotap_hdr radiotap_hdr;
				struct lynx_rx_radiotap_hdr *pradiotap_hdr;
				memset(&radiotap_hdr, 0, sizeof(radiotap_hdr));
				radiotap_hdr.hdr.it_version = PKTHDR_RADIOTAP_VERSION;
				radiotap_hdr.hdr.it_pad = 0;
				// radiotap header uses little endian
				radiotap_hdr.hdr.it_len = cpu_to_le16 (sizeof(struct lynx_rx_radiotap_hdr)); 
				radiotap_hdr.hdr.it_present = cpu_to_le32 (RX_RADIOTAP_PRESENT);

				if (lynx->channel == 0)
					lynx->channel=lynx->fw_config.channel;

				radiotap_hdr.chnl_freq = 
				cpu_to_le16(ieee80211_channel_to_frequency( lynx->channel, NL80211_BAND_2GHZ));//mhz

				radiotap_hdr.chnl_flags = cpu_to_le16(0);
				radiotap_hdr.dbmsignal = (s8) bb_rssi_decode(WRB_RSSI(wrb), RSSI_OFFSET);

				//lynx_dbg(LYNX_DBG_WLAN_RX, "%s():  lynx->channel  %d \n",__FUNCTION__, lynx->channel );

				// add space for the new radio header
				if ((skb_headroom(skb) < sizeof(struct lynx_rx_radiotap_hdr)) &&
					pskb_expand_head(skb, sizeof(struct lynx_rx_radiotap_hdr), 0, GFP_ATOMIC)){
					lynx_dbg(LYNX_DBG_WLAN_RX, "%s():Couldn't expand skb for Radio header\n", __FUNCTION__);
					goto skb_free;
				}
				pradiotap_hdr = (void *)skb_push(skb, sizeof(struct lynx_rx_radiotap_hdr));
				memcpy(pradiotap_hdr, &radiotap_hdr, sizeof(struct lynx_rx_radiotap_hdr));

				//forward monitored data frame to host
				lynx_deliver_frames_to_nw_stack(vif->ndev, skb);
				continue;
			}
#endif /*CONFIG_SUPPORT_MONITOR_MODE */

			if(wh) 
			{
#if !defined(CONFIG_LYNX_WM_MANAGER)
				/* To avoid warn msg */
				if(skb->len < offsetof(struct ieee80211_mgmt, u.probe_resp.variable))
					goto skb_free;

				mgmt = (struct ieee80211_mgmt *)skb->data;
				if(ieee80211_is_beacon(mgmt->frame_control) ||
					ieee80211_is_probe_resp(mgmt->frame_control)){

					//if ( ieee80211_is_probe_resp(mgmt->frame_control)){
					//	lynx_dbg(LYNX_DBG_WLAN_CFG, "%s():Receive ProbeRsp mgmt pkgt\n", __FUNCTION__);
					//}

					freq = ieee80211_channel_to_frequency(rx_hdr->rx.channel, NL80211_BAND_2GHZ); 
					channel = ieee80211_get_channel(lynx->wiphy, freq);
					/*To avoid warn msg*/
					if(!channel)
						goto skb_free;

#ifdef CONFIG_ANDROID
					/* Android needs the timestamp fields as the frame recevied time */
					ktime_get_boottime_ts64(&ts);
					mgmt->u.probe_resp.timestamp = ((u64)ts.tv_sec*1000000) + (ts.tv_nsec / 1000);
#endif
					notify_signal = bb_rssi_decode(WRB_RSSI(wrb), RSSI_OFFSET); 
																
					//CFG80211_SIGNAL_TYPE_MBM: signal strength in mBm (100*dBm)
					bss = cfg80211_inform_bss_frame(lynx->wiphy, channel, mgmt, skb->len,
							notify_signal*100, GFP_ATOMIC);
					if (bss == NULL)
						goto skb_free;

					cfg80211_put_bss(lynx->wiphy, bss);

#ifdef CONFIG_HOST_ROAMING
					if((be32_to_cpu(&vif->bss.flag) & BSS_FLG_BEACON_REPORT) &&
						(test_bit(CONNECTED, &vif->flags)) &&
						(!memcmp(vif->req_bssid, bss->bssid, ETH_ALEN) ) &&
						(notify_signal < 0)) {
							lynx_vif_cqm_rssi_notify(vif, (s32)notify_signal,1);
					}
#endif /*CONFIG_HOST_ROAMING*/

				}
#ifdef CONFIG_HOST_P2P
				else if ( ieee80211_is_probe_req(mgmt->frame_control) ){
					//lynx_dbg(LYNX_DBG_WLAN_CFG, "%s():Receive ProbeReq mgmt pkgt\n", __FUNCTION__);

					if( vif->wdev.iftype  != NL80211_IFTYPE_P2P_DEVICE ) {
						//lynx_dbg(LYNX_DBG_WLAN_CFG, "%s():VIF not P2P dev !,pass to p2p vif\n", __FUNCTION__);
						vif = lynx_get_p2p_vif(lynx,1) ;
					}

					/* Check if wpa_supplicant has registered for ProbeReq frame */
					mgmt_type = (IEEE80211_STYPE_PROBE_REQ & IEEE80211_FCTL_STYPE) >> 4;
					if ((vif->mgmt_rx_reg & BIT(mgmt_type)) == 0){
						//lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): Supplicant NOT registered for probe_req.\n", __FUNCTION__);
						goto skb_free;  
					}
					
					if (NULL != vif){
						freq = ieee80211_channel_to_frequency(rx_hdr->rx.channel, NL80211_BAND_2GHZ);
						
#if  (LINUX_VERSION_CODE < KERNEL_VERSION(3,12,0)) && !defined(CONFIG_BACKPORT_PWD)
						cfg80211_rx_mgmt(&vif->wdev, freq, 0,(const u8 *) mgmt, skb->len, GFP_ATOMIC);
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0))
						cfg80211_rx_mgmt(&vif->wdev, freq, 0,(const u8 *) mgmt, skb->len, 0, GFP_ATOMIC);
#else // kernel >= 3.18
						cfg80211_rx_mgmt(&vif->wdev, freq, 0,(const u8 *) mgmt, skb->len, 0);
#endif
					}
					else{
						lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): p2p vif not found!\n", __FUNCTION__);
						goto skb_free;  
					}
				}
				else if ( ieee80211_is_action(mgmt->frame_control) ){
					//lynx_dbg(LYNX_DBG_WLAN_CFG, "%s():Receive Action mgmt pkgt\n", __FUNCTION__);
					
					if( vif->wdev.iftype  != NL80211_IFTYPE_P2P_DEVICE ) {
						//lynx_dbg(LYNX_DBG_WLAN_CFG, "%s():VIF not P2P dev,pass to p2p vif !\n", __FUNCTION__);
						vif = lynx_get_p2p_vif(lynx,1) ;
					}

					if (NULL != vif){
						freq = ieee80211_channel_to_frequency(rx_hdr->rx.channel, NL80211_BAND_2GHZ);
						
#if  (LINUX_VERSION_CODE < KERNEL_VERSION(3,12,0)) && !defined(CONFIG_BACKPORT_PWD)
						cfg80211_rx_mgmt(&vif->wdev, freq, 0,(const u8 *) mgmt, skb->len, GFP_ATOMIC);
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0))
						cfg80211_rx_mgmt(&vif->wdev, freq, 0,(const u8 *) mgmt, skb->len, 0, GFP_ATOMIC);
#else // kernel >= 3.18
						cfg80211_rx_mgmt(&vif->wdev, freq, 0,(const u8 *) mgmt, skb->len, 0);
#endif

						lynx_dbg(LYNX_DBG_WLAN_CFG, "%s():Receive Action mgmt pkgt at channel %d\n", __FUNCTION__,rx_hdr->rx.channel);
						
					}
					else{
						lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): p2p vif not found!\n", __FUNCTION__);
						goto skb_free;  
					}
						
				}
#endif /*CONFIG_HOST_P2P*/
				else{
					lynx_dbg(LYNX_DBG_WLAN_CFG, "%s():Receive other mgmt pkgt\n", __FUNCTION__);
					goto skb_free;  /* Only update BSS table and Action Frame for now */
				}
#if 0
                /*FIXME:maybe need more condition */
                if(!timer_pending(&vif->sched_scan_timer)) 
                {
                    mod_timer(&vif->sched_scan_timer, jiffies + msecs_to_jiffies(5000));
                }
#endif
#else // CONFIG_LYNX_WM_MANAGER
                /* To avoid warn msg */
				if(skb->len < sizeof(struct wlan_probe_resp_frame))
				{
					lynx_dbg(LYNX_DBG_WLAN_RX, "%s(): skb len too small (%d < %d)\n", 
						__FUNCTION__, skb->len, sizeof(struct wlan_probe_resp_frame));
					goto skb_free;
				}

				mgmt = (struct ieee80211_mgmt *)skb->data;

				if(ieee80211_is_beacon(mgmt->frame_control) || 
					ieee80211_is_probe_resp(mgmt->frame_control))
				{
					msg = (struct wm_msg *)skb->cb;
					msg->type = WM_MSG_WLAN_FRAME;
					msg->u.wf.rssi = bb_rssi_decode(WRB_RSSI(wrb), RSSI_OFFSET);
					msg->u.wf.vif_idx = ra_idx;
					msg->u.wf.sta_idx = WRB_SAIDX(wrb);
					msg->u.wf.type = WM_WF_TYPE_MGMT;
					wm_msgq_write(skb);
					continue;	// free skb in the wm_manager
				}
#endif  // CONFIG_LYNX_WM_MANAGER
			} 
			else 
			{   /* Data frame */
#ifdef CONFIG_HOST_P2P
				if( vif->wdev.iftype  == NL80211_IFTYPE_P2P_DEVICE ){
					lynx_dbg(LYNX_DBG_WARN, "%s():P2P control interface not handle DATA,drop it. \n", __FUNCTION__);
					goto skb_free;
				}
#endif /*CONFIG_HOST_P2P*/
#ifdef CONFIG_LYNX_WM_MANAGER
				ehdr *ef = (ehdr *)skb->data;
				unsigned short type;
				type = be16_to_cpu(ef->type);

				if((type == ETHERTYPE_EAPOL)
#ifdef CONFIG_WAPI
					|| (type == ETHERTYPE_WAPI)
#endif	// CONFIG_WAPI
				  )
				{
					msg = (struct wm_msg *)skb->cb;
					msg->type = WM_MSG_WLAN_FRAME;
					msg->u.wf.vif_idx = ra_idx;
					msg->u.wf.sta_idx = WRB_SAIDX(wrb);
					msg->u.wf.type = WM_WF_TYPE_DATA;
					wm_msgq_write(skb);
					continue;	// free skb in the wm_manager
				}
#endif // CONFIG_LYNX_WM_MANAGER
#ifdef CONFIG_LYNX_STATICS
				update_sta_static(vif, WRB_SAIDX(wrb), 0, WRB_RSSI(wrb), pkt_len);
#endif	// CONFIG_LYNX_STATICS

				if(vif->nw_type == VIF_AP_MODE)
				{
					if((skb_forward = lynx_forward_check(vif, skb)) != NULL)
					{
						lynx_data_tx(skb_forward, vif->ndev);

						if(skb == skb_forward)
							continue;
						/* skb != skb_forward : multicast frame */
					}
				}

				lynx_deliver_frames_to_nw_stack(vif->ndev, skb);
				continue;
			}
		}
		else 
		{
			lynx_wci_callback(lynx, (struct lynx_wci_hdr *)skb->data, skb->len);
		}

skb_free:
		kfree_skb(skb);
	}
}

int lynx_tx_send(struct lynx *lnx, struct sk_buff *skb)
{
	struct lynx_hif_device *lnx_usb = (struct lynx_hif_device *)lnx->hif_priv;
	int ret=-ENODEV;
#ifdef CONFIG_LYNX_STATICS
	int bytes;
	int sta_idx;
	struct lynx_vif *vif = NULL;
	unsigned long *cb_ptr=NULL;
#endif	// CONFIG_LYNX_STATICS

	lynx_dbg(LYNX_DBG_WARN, "%s(): (len=%d)\n", __FUNCTION__, skb->len);
	lynx_dbg_dump(LYNX_DBG_WLAN_TX, "lynx_tx_send", "tx ", skb->data, skb->len);

#ifdef CONFIG_LYNX_STATICS
	bytes = skb->len - SKB_DATA_OFFSET;
	cb_ptr = (unsigned long *)&skb->cb[0];
	vif = (struct lynx_vif *)(cb_ptr[0]);
	sta_idx = cb_ptr[1];
#endif	// CONFIG_LYNX_STATICS

	if(lnx_usb)
		ret = hif_device_send(lnx_usb, skb);
#ifdef CONFIG_LYNX_STATICS
	if((ret == 0) && (vif != NULL))
	{
		update_sta_static(vif, sta_idx, 1, 0, bytes);
	}
#endif	// CONFIG_LYNX_STATICS

	return ret;
}
