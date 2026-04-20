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
#ifdef CONFIG_LYNX_OS_LINUX
#include <linux/udp.h>
#if !defined(CONFIG_LYNX_WM_MANAGER)
#include <net/mac80211.h>
#endif	// CONFIG_LYNX_WM_MANAGER
#endif	// CONFIG_LYNX_OS_LINUX
#include "init.h"
#include "core.h"
#include "vfc.h"
#include "wci.h"
#include "txrx.h"
#include "hif.h"
#include "lynx_debug.h"
#include "cfg80211.h"

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

struct lynx_vif *lynx_vif_first(struct lynx *lnx)
{
	struct lynx_vif *vif=NULL;
	os_api_dsr_lock(&lnx->list_lock);
	if (!list_empty(&lnx->vif_list)) {
		vif = list_first_entry(&lnx->vif_list, struct lynx_vif, list);
	}
	os_api_dsr_unlock(&lnx->list_lock);
	return vif;
}

struct lynx_vif *lynx_remove_vif_first(struct lynx *lnx)
{
	struct lynx_vif *vif=NULL;
	os_api_dsr_lock(&lnx->list_lock);
	if (!list_empty(&lnx->vif_list)) {
		vif = list_first_entry(&lnx->vif_list, struct lynx_vif, list);
		list_del(&vif->list);
	}
	os_api_dsr_unlock(&lnx->list_lock);
	return vif;
}

bool lynx_remove_vif(struct lynx *lnx ,struct lynx_vif * search_vif)
{
	struct lynx_vif *vif      = NULL;
	bool found = false;
	os_api_dsr_lock(&lnx->list_lock);
	list_for_each_entry(vif, &lnx->vif_list, list) {
		if(vif == search_vif ) {
			list_del(&vif->list);
			found = true;
			break;
		}
	}
	
	os_api_dsr_unlock(&lnx->list_lock);
	return found;
}

struct lynx_vif *lynx_get_vif_by_index(struct lynx *lnx, u8 if_idx)
{
	/* FIXME: Must sure that lynx_get_vif_by_index() called in lynx_rx_tasklet.
		  The lynx_rx_tasklet() disables the hardware interrupt, 
		  so only use spin_lock() here, instead of spin_lock_bh().				*/
    struct lynx_vif *vif, *found = NULL;

	/* FIXME: mark this for temperal, the device should fill the right ra_index in rx_desc */


    os_api_lock(&lnx->list_lock);

    list_for_each_entry(vif, &lnx->vif_list, list) {
		/* FIXME: only for debug */
        if(vif->fw_vif_idx == if_idx) {
            found = vif;
            break;
        }
    }

    os_api_unlock(&lnx->list_lock);

    return found;
}


#ifdef CONFIG_HOST_P2P
struct lynx_vif *lynx_get_p2p_vif(struct lynx *lnx,bool at_lynx_rx_tasklet)
{
	struct lynx_vif *vif      = NULL;
	struct lynx_vif *found = NULL;
	
	if (at_lynx_rx_tasklet)
		os_api_lock(&lnx->list_lock);
	else
		os_api_dsr_lock(&lnx->list_lock);
	
	list_for_each_entry(vif, &lnx->vif_list, list) {
		if(vif->wdev.iftype  == NL80211_IFTYPE_P2P_DEVICE ) {
			found = vif;
			break;
		}
	}
	
	if (at_lynx_rx_tasklet)
		os_api_unlock(&lnx->list_lock);
	else
		os_api_dsr_unlock(&lnx->list_lock);
	
	return found;
}

struct lynx_vif *lynx_get_p2p_data_vif(struct lynx *lnx,bool at_lynx_rx_tasklet)
{
	struct lynx_vif *vif      = NULL;
	struct lynx_vif *found = NULL;
	
	if (at_lynx_rx_tasklet)
		spin_lock(&lnx->list_lock);
	else
		spin_lock_bh(&lnx->list_lock);
	
	list_for_each_entry(vif, &lnx->vif_list, list) {
		if( ( vif->wdev.iftype  == NL80211_IFTYPE_P2P_CLIENT ) || 
			( vif->wdev.iftype  == NL80211_IFTYPE_P2P_GO )){
			found = vif;
			break;
		}
	}
	
	if (at_lynx_rx_tasklet)
		spin_unlock(&lnx->list_lock);
	else
		spin_unlock_bh(&lnx->list_lock);
	
	return found;
}
#endif /*CONFIG_HOST_P2P*/


struct lynx_vif *lynx_get_sta_vif(struct lynx *lnx,bool at_lynx_rx_tasklet)
{
	struct lynx_vif *vif      = NULL;
	struct lynx_vif *found = NULL;
	
	if (at_lynx_rx_tasklet)
		os_api_lock(&lnx->list_lock);
	else
		os_api_dsr_lock(&lnx->list_lock);
	
	list_for_each_entry(vif, &lnx->vif_list, list) {
		if(vif->wdev.iftype == NL80211_IFTYPE_STATION ) {
			found = vif;
			break;
		}
	}
	
	if (at_lynx_rx_tasklet)
		os_api_unlock(&lnx->list_lock);
	else
		os_api_dsr_unlock(&lnx->list_lock);
	
	return found;
}



struct lynx_vif *lynx_get_scanning_vif(struct lynx *lnx,bool at_lynx_rx_tasklet)
{
	struct lynx_vif *vif      = NULL;
	struct lynx_vif *found = NULL;
	
	if (at_lynx_rx_tasklet)
		spin_lock(&lnx->list_lock);
	else
		spin_lock_bh(&lnx->list_lock);
	
	list_for_each_entry(vif, &lnx->vif_list, list) {
		 if(vif->scan_req){
			found = vif;
			break;
		}
	}
	
	if (at_lynx_rx_tasklet)
		spin_unlock(&lnx->list_lock);
	else
		spin_unlock_bh(&lnx->list_lock);
	
	return found;
}

int lynx_configs_setup(struct lynx *lnx)
{
	struct device_configs *fw = &lnx->fw_config;

	/* setting is from device firmware */
	fw->phy_cap = AP_CAP_11B|AP_CAP_11G|AP_CAP_11N;
	fw->channel = 1;					/* default channel */
	fw->bandwidth = BW40MHZ_AUTO;		/* 40MHz */
	fw->rts_threshold = (2346 - 24);
	fw->slottime = SLOTTIME_9US;
	fw->capability = (WLAN_CAPABILITY_ESS|WLAN_CAPABILITY_SHORT_SLOT_TIME|WLAN_CAPABILITY_SHORT_PREAMBLE);	/* WLAN_CAPABILITY_APSD */
	fw->current_rates = ALL_SUPPORTED_RATES;
	fw->ampdu_params = 0x02; /* Maximum AMPDU RX length is 32K */
	fw->ba_win_size = 32;
	fw->channel_num = 11;
	fw->country[0]='U'; fw->country[1]='S'; fw->country[2]=' '; fw->country[3]=0x0;
    fw->listen_interval_max = 100;
	/* AC_BE */
	fw->sta_ac_parms[0].qidx = 1;
	fw->sta_ac_parms[0].cwmin = 15;
	fw->sta_ac_parms[0].cwmax = 1023;
	fw->sta_ac_parms[0].aifs = 3;
	fw->sta_ac_parms[0].txoplimit = 0;
	/* AC_BK */
	fw->sta_ac_parms[1].qidx = 0;
	fw->sta_ac_parms[1].cwmin = 15;
	fw->sta_ac_parms[1].cwmax = 1023;
	fw->sta_ac_parms[1].aifs = 7;
	fw->sta_ac_parms[1].txoplimit = 0;
	/* AC_VI */
	fw->sta_ac_parms[2].qidx = 2;
	fw->sta_ac_parms[2].cwmin = 7;
	fw->sta_ac_parms[2].cwmax = 15;
	fw->sta_ac_parms[2].aifs = 2;
	fw->sta_ac_parms[2].txoplimit = 94;
	/* AC_VO */
	fw->sta_ac_parms[3].qidx = 3;
	fw->sta_ac_parms[3].cwmin = 3;
	fw->sta_ac_parms[3].cwmax = 7;
	fw->sta_ac_parms[3].aifs = 2;
	fw->sta_ac_parms[3].txoplimit = 47;

	return 0;
}

int lynx_wmm_queue_prioty_setup(struct lynx *lnx)
{
	struct lynx_queue *lnx_q = &lnx->queue;
	struct device_configs *fw = &lnx->fw_config;
	int i, j, biggest, tmp;
	int priority[4]={IEEE80211_AC_BK, IEEE80211_AC_BE, IEEE80211_AC_VI, IEEE80211_AC_VO};
	int aifs[4];

	/* sequence in the fw->sta_ac_parms : BE, BK, VI, VO */
	/* normal priority : VO > VI > BE > BK */
	
	aifs[0] = fw->sta_ac_parms[1].aifs;
	aifs[1] = fw->sta_ac_parms[0].aifs;
	aifs[2] = fw->sta_ac_parms[2].aifs;
	aifs[3] = fw->sta_ac_parms[3].aifs;

	for(i=0; i < 3; i++)
	{
		biggest = i;
		for(j=i+1; j < 4; j++)
		{
			if(aifs[biggest] < aifs[j])
				biggest = j;
		}

		if(biggest != i)
		{
			tmp = priority[i];
			priority[i] = priority[biggest];
			priority[biggest] = tmp;

			tmp = aifs[i];
			aifs[i] = aifs[biggest];
			aifs[biggest] = tmp;
		}
	}

	for(i=0; i<4; i++)
	{
		lnx_q->priority[i] = priority[i];
	}

	/* urgent queue */
	lnx_q->priority[LYNX_URGENT_QUEUE_NO] = LYNX_URGENT_QUEUE_NO;

	return 0;
}

int lynx_wmm_queue_init(struct lynx *lnx)
{
	struct lynx_queue *lnx_q = &lnx->queue;
	struct lynx_queue_arr *wmm_queue;
	int i;

	wmm_queue = &lnx_q->wmm_queue[0];

	for(i=0; i<LYNX_QUEUE_NUMS; i++)
	{
		__skb_queue_head_init(&(wmm_queue[i].tx_queue));
		wmm_queue[i].free_queue_counter = LYNX_WMM_QUEUE_SIZE;
	}
	
	lynx_wmm_queue_prioty_setup(lnx);
	lnx_q->flags = LYNX_QUEUE_ENABLE;

	return 0;
}

int lynx_wmm_queue_deinit(struct lynx *lnx)
{
	struct lynx_queue *lnx_q = &lnx->queue;
	struct lynx_queue_arr *wmm_queue=&lnx_q->wmm_queue[0];
    struct lynx_hif_device *lnx_hif = (struct lynx_hif_device *)lnx->hif_priv;
	int i;
	unsigned long flags=0;
	
	if(!(lnx_q->flags & LYNX_QUEUE_ENABLE))
	{
        return -ENODEV;
	}
	
    hif_tx_lock(lnx_hif, &flags);
	
	lnx_q->flags = 0;

	for(i=0; i<LYNX_QUEUE_NUMS; i++)
	{
		if(wmm_queue->free_queue_counter != LYNX_WMM_QUEUE_SIZE)
		{
			skb_queue_purge(&(wmm_queue[i].tx_queue));
			wmm_queue->free_queue_counter = LYNX_WMM_QUEUE_SIZE;
		}
	}
	
    hif_tx_unlock(lnx_hif, &flags);
			
	return 0;
}

int lynx_wmm_classify(struct sk_buff *skb)
{
	u8 dscp=0;
	u8 priority=IEEE80211_AC_BE;
	
	if(skb->protocol == htons(ETHERTYPE_IP))
	{
		dscp = (((ip_hdr(skb)->tos) >> 5) & 0x7);

		/*
			dscp == 6,7 => VI
			dscp == 4,5 => VO
			dscp == 0,3 => BE
			dscp == 1,2 => BK
		*/

		if(dscp >= 0x6)
			priority = IEEE80211_AC_VO;	// 0
		else if(dscp >= 0x4)
			priority = IEEE80211_AC_VI;	// 1
		else if((dscp == 0x0) || (dscp == 0x3))
			priority = IEEE80211_AC_BE;	// 2
		else	
			priority = IEEE80211_AC_BK;	// 3
	}
	else if((skb->protocol == htons(ETHERTYPE_EAPOL))
#ifdef CONFIG_WAPI
			|| (skb->protocol == htons(ETHERTYPE_WAPI))
#endif	// CONFIG_WAPI
			)
	{
		priority = LYNX_URGENT_QUEUE_NO;
	}

	return priority;
}

void lynx_data_dequeue(struct lynx *lnx)
{
	struct lynx_queue *lnx_q = &lnx->queue;
	struct lynx_queue_arr *wmm_queue=NULL;
	struct sk_buff *skb=NULL;
	struct lynx_hif_device *lnx_hif = (struct lynx_hif_device *)lnx->hif_priv;
	unsigned long flags=0;
	int dscp=0;
	int ret=0;
	int i;

	if(!(lnx_q->flags & LYNX_QUEUE_ENABLE))
		return;

	hif_tx_lock(lnx_hif, &flags);
	
	if((lnx_q->flags & LYNX_QUEUE_MASK) != 0)	
	{
#if 0
		if(lnx_q->flags & LYNX_QUEUE_IN_Q_VO)
			dscp = IEEE80211_AC_VO;
		else if(lnx_q->flags & LYNX_QUEUE_IN_Q_VI)
			dscp = IEEE80211_AC_VI;
		else if(lnx_q->flags & LYNX_QUEUE_IN_Q_BE)
			dscp = IEEE80211_AC_BE;
		else if(lnx_q->flags & LYNX_QUEUE_IN_Q_BK)
			dscp = IEEE80211_AC_BK;
#else
		for(i=LYNX_URGENT_QUEUE_NO; i>=0; i--)
		{
			if(lnx_q->flags & (1 << lnx_q->priority[i]))
			{
				dscp = lnx_q->priority[i];
				break;
			}
		}
#endif

		wmm_queue = &lnx_q->wmm_queue[dscp];

		skb = __skb_dequeue(&wmm_queue->tx_queue);
		
		if(skb)
		{
			hif_tx_unlock(lnx_hif, &flags);
			ret = lynx_tx_send(lnx, skb);
			hif_tx_lock(lnx_hif, &flags);

			if(ret == 0)
			{
				wmm_queue->free_queue_counter++;
				if(wmm_queue->free_queue_counter == LYNX_WMM_QUEUE_SIZE)
					lnx_q->flags &= ~(1 << dscp);
			}
			else
			{
				__skb_queue_head(&wmm_queue->tx_queue, skb);
			}
		}
	}
	
	hif_tx_unlock(lnx_hif, &flags);
}

int lynx_data_urgentq_check(struct sk_buff *skb)
{
	struct ethhdr *ethhdr;
	struct iphdr *iph;
	struct udphdr *udph;
	int is_urgent = 0;

	/* FIXME: make sure that the skb has been installed the wci header */
	ethhdr = (struct ethhdr *)(skb->data + SKB_DATA_OFFSET);

	if(ethhdr->h_proto == htons(ETHERTYPE_EAPOL))
	{
		is_urgent = 1;
	}
	else if(ethhdr->h_proto == htons(ETHERTYPE_ARP))
	{
		/* Android uses the arp req/resp to do router check */
		is_urgent = 1;
	}
	else if (ethhdr->h_proto == htons(ETHERTYPE_IP)) 
	{
		iph = (struct iphdr *) &ethhdr[1];

		if(iph->protocol == IPPROTO_UDP)
		{
			udph = (struct udphdr *) ((unsigned long)iph + iph->ihl*4);
			
			if((udph->source == htons(68)) || (udph->source == htons(67)))
			{
				/* DHCP */
				is_urgent = 1;
			}
		}
	}

	return is_urgent;
}

int lynx_data_enqueue(struct sk_buff *skb, struct lynx_vif *vif, int urgent)
{
	struct lynx *lnx = vif->lnx;
	struct lynx_queue *lnx_q = &lnx->queue;
	struct lynx_queue_arr *wmm_queue;
	struct lynx_hif_device *lnx_hif = (struct lynx_hif_device *)lnx->hif_priv;
	unsigned long flags;
	u8 priority=0;
	int ret = 0;
	
	if(!(lnx_q->flags & LYNX_QUEUE_ENABLE))
	{
        return -ENODEV;
	}
	
	if(urgent)
	{
		priority = LYNX_URGENT_QUEUE_NO;
	}
	else
	{
		priority = lynx_wmm_classify(skb);

		if((BIT(priority) & vif->acm) && (priority != IEEE80211_AC_BK))
			priority += 1;

		/* for error handle */
		if(priority > LYNX_URGENT_QUEUE_NO)
			priority = LYNX_URGENT_QUEUE_NO;
	}

	wmm_queue = &lnx_q->wmm_queue[priority];
	
	hif_tx_lock(lnx_hif, &flags);

	if(wmm_queue->free_queue_counter > 0)
	{
		wmm_queue->free_queue_counter--;
		
		__skb_queue_tail(&wmm_queue->tx_queue, skb);
		lnx_q->flags |= (1 << priority);
	}
	else
	{
		ret = -1;
	}
		
	hif_tx_unlock(lnx_hif, &flags);

    return ret;
}

int lynx_data_queue_is_empty(struct lynx *lnx)
{
	struct lynx_queue *lnx_q = &lnx->queue;
	int ret=0;

	ret = !(lnx_q->flags & LYNX_QUEUE_MASK);

	return ret;
}

int lynx_core_init(struct lynx *lnx)
{
	//struct lynx_vif *vif;
	int ret = -ENOMEM;

	/* FIXME: How to set up current_rates */
	lnx->current_rates = ALL_SUPPORTED_RATES;
	lnx->basic_rates = ALL_BASIC_RATES;
	lynx_configs_setup(lnx);
	lynx_wmm_queue_init(lnx);

	ret = os_dep_init(lnx);

	return ret;
}

/* put this func in the usb/sdio probe */
struct lynx *lynx_core_create(void)
{
    struct lynx *lnx;

    lnx = os_dep_create();
    if(!lnx)
        return NULL;

    /*FIXME: maybe add a config to change vif max value*/
    lnx->vif_max = VIF_MAX_NUM;

    os_api_lock_init(&lnx->list_lock);

#if defined(CONFIG_LYNX_OS_LINUX)
    sema_init(&lnx->sem, 1);
#endif // CONFIG_LYNX_OS_LINUX

    INIT_LIST_HEAD(&lnx->vif_list);

    clear_bit(WCI_READY, &lnx->flag);
    clear_bit(DESTROY_IN_PROGRESS, &lnx->flag);

    return lnx;
    
}

void lynx_core_cleanup(struct lynx *lnx)
{
	struct lynx_vif *vif;

	/* FIXME: may do power off action for usb */
	if (lnx) {

	set_bit(DESTROY_IN_PROGRESS, &lnx->flag);

#if !defined(CONFIG_LYNX_WM_MANAGER)
#if defined(CONFIG_LYNX_OS_LINUX)
	while((vif = lynx_remove_vif_first(lnx))) {
		lynx_cfg80211_vif_stop(vif);
		//lynx_wci_detach_cmd(vif);
		rtnl_lock();
		lynx_interface_del(lnx, vif);
		rtnl_unlock();
	}
#endif
#endif

	lynx_wmm_queue_deinit(lnx);
	os_api_lock_deinit(&lnx->list_lock);
    }
}

int lynx_vif_cfg(struct lynx *lnx, struct lynx_vif *vif, int item, unsigned long value)
{
	/* FIXME: param "int value" should take in the 64 bits CPU */
	struct device_configs *fw = &lnx->fw_config;
	struct wm_bss *bss=NULL;
	int ret = 0;

	if(item > VIF_CFG_BANDWIDTH)
	{
		if(vif == NULL)
			return -1;
		bss = &vif->bss;
	}

	switch(item)
	{
		case VIF_CFG_MIN_RATEIDX:
			fw->min_rate_idx = value;
			lynx_wci_set_min_rateidx(lnx);
			break;
		case VIF_CFG_PS:
			fw->power_saving_mode = value;
			lynx_wci_set_power_saving(lnx);
			break;
		case VIF_CFG_CHANNEL:
			fw->channel = lnx->channel = value;
			lynx_wci_set_channel_cmd(lnx);
			break;
		case VIF_CFG_BANDWIDTH:
			fw->bandwidth = value;
			lynx_wci_set_channel_cmd(lnx);
			break;
		case VIF_GET_TXPWR:
			lynx_wci_get_tx_pwr_cmd(vif);
			break;
		case VIF_CFG_PHY_CAP:
			bss->phy_cap = value;
			break;
		case VIF_CFG_BEACON_INTERVAL:
			bss->beacon_interval = value;
			cpu_to_be16s(&(bss->beacon_interval));
			break;
		case VIF_CFG_DTIM:
			bss->dtim_period = value;
			break;
		case VIF_CFG_BSSID:
			memcpy(bss->bssid, (char *)value, WLAN_ADDR_LEN);
			break;
		case VIF_CFG_SSID:
			strcpy(bss->ssid, (char *)value);
			bss->ssid_len = strlen(bss->ssid);
			//lynx_wci_update_vif_cmd(vif);
			break;
		case VIF_CFG_AUTH_TYPE:
			/* TODO: vif->dot11_auth_mode */
			bss->auth_capability = cpu_to_be32((unsigned int)value);
			if(value & AUTH_CAP_WPA2)
				vif->auth_mode = WPA2_AUTH;
			else if(value & AUTH_CAP_WPA)
				vif->auth_mode = WPA_AUTH;
			else
				vif->auth_mode = NONE_AUTH;
			//lynx_wci_update_vif_cmd(vif);
			break;
		case VIF_CFG_HT_CAP:
			bss->ht_capability = value;
			cpu_to_be16s(&(bss->ht_capability));
			//lynx_wci_update_vif_cmd(vif);
			break;
		default:
			ret = -1;
			break;
	}

#ifdef CONFIG_UNIT_TEST
	if((ret == 0) && (item >= VIF_CFG_PHY_CAP))
	{
		unittest_bss_backup(vif, __FUNCTION__);
	}
#endif	// CONFIG_UNIT_TEST

	return ret;
}


#define IEEE80211_SIGNAL_AVE_WEIGHT	3
#define IEEE80211_SIGNAL_AVE_MIN_COUNT	4

void lynx_vif_cqm_rssi_notify(struct lynx_vif *vif, s32 rssi,int beacon)
{
	struct lynx *lnx = vif->lnx;
	enum nl80211_cqm_rssi_threshold_event event;
	int thold, hyst, last_event;
	s32 rssi_avg = 0;

	if(vif != lynx_vif_first(lnx)){
		lynx_dbg(LYNX_DBG_ERR, "Only support first vif for cqm_rssi_notify \n");
		return ;
	}

	if ((lnx->cqm_rssi_thold >= 0) || ( rssi >= 0)){
		//lynx_dbg(LYNX_DBG_ERR, "rssi throld / rssi not negative \n");
		return;
	}

	if(lnx->ave_beacon_signal){
		lnx->ave_beacon_signal= ( IEEE80211_SIGNAL_AVE_WEIGHT * rssi* 16 + 
								(16 - IEEE80211_SIGNAL_AVE_WEIGHT) * lnx->ave_beacon_signal) / 16;

		lnx->count_beacon_signal++;
	}else{
		lnx->ave_beacon_signal = rssi*16;
		lnx->count_beacon_signal =1;
	}

	rssi_avg = lnx->ave_beacon_signal /16;

	if (lnx->count_beacon_signal < IEEE80211_SIGNAL_AVE_MIN_COUNT ){
		//lynx_dbg(LYNX_DBG_ERR, "count_beacon_signal (%d)< IEEE80211_SIGNAL_AVE_MIN_COUNT \n",lnx->count_beacon_signal);
		return;
	}

	last_event = lnx->last_cqm_event_rssi;
	thold = lnx->cqm_rssi_thold;
	hyst = lnx->cqm_rssi_hyst;

	if ( rssi_avg < thold && (last_event == 0 || rssi_avg < (last_event - hyst))){
		event = NL80211_CQM_RSSI_THRESHOLD_EVENT_LOW;
		lynx_dbg(LYNX_DBG_ERR, "NL80211_CQM_RSSI_THRESHOLD_EVENT_LOW! %d \n", rssi_avg);
	}else if ( rssi_avg > thold && (last_event == 0 || rssi_avg > (last_event + hyst))){
		event = NL80211_CQM_RSSI_THRESHOLD_EVENT_HIGH;
		lynx_dbg(LYNX_DBG_ERR, " NL80211_CQM_RSSI_THRESHOLD_EVENT_HIGH!%d \n", rssi_avg);
	}else{
		//lynx_dbg(LYNX_DBG_ERR, "AVG RSSI %d (RSSI %d) at range \n",rssi_avg,rssi);
		return;
	}

	/*Notify event*/
	lnx->last_cqm_event_rssi = rssi_avg;
	cfg80211_cqm_rssi_notify(vif->ndev, event, rssi_avg, GFP_KERNEL);
}

#ifdef CONFIG_LYNX_STATICS
void update_sta_static(struct lynx_vif *vif, int sta_idx, int is_tx, int rssi, int bytes)
{
    struct lynx *lnx;
	struct lynx_sta *sta;
	struct timespec64 now;

	if(sta_idx >= LYNX_STA_MAX_NUM)
		return;

    lnx = vif->lnx;
	sta = &lnx->sta_list[sta_idx];

	if((sta->flags & LYNX_STA_VALID))
	{
		if(!is_tx)
		{
			sta->rx_bytes += bytes;
			sta->rx_packets += 1;
			/* rssi is not encoded to dBm at here */
			sta->signal_last = rssi;
			if(sta->signal_avg)
				sta->signal_avg = (rssi + sta->signal_avg)/2;
			else
				sta->signal_avg = rssi;
		}
		else
		{
			sta->tx_bytes += bytes;
			sta->tx_packets += 1;
		}

		ktime_get_real_ts64(&now);
		sta->last_active_time = (now.tv_sec * 1000);
	}
}
#endif // CONFIG_LYNX_STATICS

/*Reference : lynx-2.0/lib/wla/bb.c*/
int bb_rssi_decode(unsigned char val, int rssi_offset)
{
	int cal = 0;

	if((cal = rssi_offset - val) > 0)
		cal = 0;

	return cal;
}


