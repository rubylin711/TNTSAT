/*=============================================================================+
|                                                                              |
| Copyright 2013                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file wm_scan.c
*   \brief 
*   \author Montage
*/

/*=============================================================================+
| Included Files
+=============================================================================*/
#include "wlan_def.h"
#include "mac_ctrl.h"
#include "init.h"
#include "core.h"
#include "lynx_debug.h"
#include <wm_scan.h>
#include <wm_vif.h>
#include <wm_sta.h>
#include <wm_mlme.h>
#include <wm_msg.h>
#include <wlan_ie.h>
#ifdef CONFIG_WPS
#include <wps.h>
#endif

static void free_nbss_dynamic_resource(struct neighbor_bss *nbss)
{
	if(nbss->wpa_info)
		os_api_free(nbss->wpa_info);
	if(nbss->rsn_info)
		os_api_free(nbss->rsn_info);
	if(nbss->wme_info)
		os_api_free(nbss->wme_info);
}

void reset_nbss_entry(struct neighbor_bss *nbss)
{
	free_nbss_dynamic_resource(nbss);
	memset(nbss, 0, sizeof(struct neighbor_bss));
	my_wlan_dev->nbss_num -= 1;
}

void reset_nbss_list(void)
{
	struct neighbor_bss *nbss=NULL;
	int i;

	for(i = 0; i < MAX_NEIGHBOR_BSS_NUM; i++)
	{
		nbss = &my_wlan_dev->site_survey[i];
		
		free_nbss_dynamic_resource(nbss);
	}

	my_wlan_dev->nbss_num = 0;
	memset(my_wlan_dev->site_survey, 0, sizeof(struct neighbor_bss)*MAX_NEIGHBOR_BSS_NUM);
}

struct neighbor_bss *find_nbss_by_ssid(unsigned char *ssid)
{
	struct neighbor_bss *nbss=NULL;
	int i;

	for(i = 0; i < MAX_NEIGHBOR_BSS_NUM; i++)
	{
		nbss = &my_wlan_dev->site_survey[i];
		
		if(nbss->flag & NBSS_ENABLE)
		{
			if(strcmp(nbss->ssid, ssid) == 0)
				break;
		}
	}

	if(i >= MAX_NEIGHBOR_BSS_NUM)
		nbss = NULL;

	return nbss;
}

inline int affected_frequency(int channel)
{
	int ch_start=0, ch_end=0, ch_center=0, ch_hit=1;

	if(my_wlan_dev->secondary_channel_def == BW40MHZ_SCN)
	{
		ch_start = ch_end = my_wlan_dev->channel;
	}
	else
	{
		if(my_wlan_dev->secondary_channel_def == BW40MHZ_SCA)
			ch_center = my_wlan_dev->channel+2;
		else if(my_wlan_dev->secondary_channel_def == BW40MHZ_SCB)
			ch_center = my_wlan_dev->channel-2;

		ch_start = ch_center - 5;
		ch_end = ch_center + 5; 
		
		if(ch_start < 1)
			ch_start = 1;
		if(ch_end > 13)
			ch_end = 13;
	}

	if((channel < ch_start) || (channel > ch_end))
		ch_hit = 0;
	else
		ch_hit = 1;

	return ch_hit;
}

void ap_handle_beacon_prob_resp(struct wlan_hdr *fm, u32 len, u8 rssi)
{
	struct wlan_probe_resp_frame *resp;
	struct neighbor_bss *nbss=NULL, *null_nbss=NULL;
	char *pos;
	int left, i, erp=0, non_ht=0;
	struct wlan_ie ie;
	unsigned int supported_rates;
	struct wlan_ie_ht_capability *cap=NULL;
	u32 current_time = os_current_time()/100;
#ifdef CONFIG_WPS
	struct wps_attribute attr={0};
#endif

	resp = (struct wlan_probe_resp_frame *)fm;
	pos = (char *)&resp[1];
	left = len - sizeof(struct wlan_probe_resp_frame);

	memset(&ie, 0, sizeof(ie));
	if(wlan_parse_ie(pos, left, &ie) < 0)
	{ 
		return;	
	}

	for(i = 0; i < MAX_NEIGHBOR_BSS_NUM; i++)
	{
		nbss = &my_wlan_dev->site_survey[i];
		if(nbss->flag & NBSS_ENABLE) 
		{
			if(memcmp(nbss->addr, fm->addr2, WLAN_ADDR_LEN) == 0)
			{
				/* overwrite same bss */
				break;
			}
			else
			{
				if(nbss->expire < current_time)
				{
					reset_nbss_entry(nbss);

					if(null_nbss == NULL)
						null_nbss = nbss;
				}
			}
		}
		else
		{
			if(null_nbss == NULL)
				null_nbss = nbss;
		}
	}

	if(i >= MAX_NEIGHBOR_BSS_NUM)
	{
		if(null_nbss == NULL)
			return;
		else
		{
			nbss = null_nbss;
			my_wlan_dev->nbss_num++;
			memcpy(nbss->addr, fm->addr2, WLAN_ADDR_LEN);
		}
	}

	nbss->flag = NBSS_ENABLE;  /* nbss->flag reset to NBSS_ENABLE */
	nbss->rssi = rssi;

	nbss->expire = current_time + NBSS_NORMAL_EXPIRE_TIME;
	/* FIXME: endian issue? */
	nbss->beacon_interval = resp->beacon_interval;

	if(ie.tim)
	{
		struct wlan_ie_tim *tim = (struct wlan_ie_tim *)(ie.tim-2);
		nbss->dtim_period = tim->period;
	}
	else
		nbss->dtim_period = 0xff;

	/* reset band information & check N mode cap */
	if(ie.ht_capabilities)
	{
		nbss->band = NBSS_BAND_N;

		cap = (struct wlan_ie_ht_capability *)(ie.ht_capabilities-2);
		/* FIXME: endian issue? */
		nbss->ht_capability = cap->ht_cap.capabilities_info;
		
		if(ie.ht_operation)
		{
			nbss->secondary_ch = ie.ht_operation[1] & WLAN_HTINFO_SECCHAN;
		}
	}
	else
	{
		nbss->band = 0;
		non_ht = 1;
	}

	if(ie.ssid)
	{
		int ssid_len = WLAN_IE_CONTEXT_TO_LEN(ie.ssid);

		if(ssid_len > 0)	/* avoid that the hidden ssid AP's beacon */
		{
			if(ssid_len > 32)
				ssid_len = 32;
			memcpy(nbss->ssid, ie.ssid, ssid_len);
			nbss->ssid[ssid_len] = '\0';
		}
	}

	/* 1. (ie.supported_rates < R_BIT(OFDM_6M) == 1 : only has 1, 2, 5.5, 11 Mbps rates 
	   2. (ie.erp_info && (ie.erp_info & 0x1)) == 1 : NonERP_Present bit is set to 1 in the beacon */
	if(ie.erp_info)
	{
		nbss->band |= NBSS_BAND_G;

		if((*ie.erp_info & WLAN_ERP_NON_ERP_PRESENT))
			erp = 1;
	}

	if(ie.supp_rates)
		supported_rates = support_rate_to_bits(ie.supp_rates, WLAN_IE_CONTEXT_TO_LEN(ie.supp_rates));
	else
	{
		lynx_dbg(LYNX_DBG_WM, "%s beacon has no support rate IE\n", nbss->ssid);
		supported_rates = 1;
	}

	if(supported_rates & R_BIT(CCK_5_5M))
		nbss->band |= NBSS_BAND_B;

	if(ie.ext_supp_rates) 
	{
		supported_rates |= support_rate_to_bits(ie.ext_supp_rates, WLAN_IE_CONTEXT_TO_LEN(ie.ext_supp_rates));
	}

	/* OBSS */
	if(supported_rates < R_BIT(OFDM_6M))
		erp = 1;
	if(erp)
		nbss->flag |= NBSS_NON_ERP;
	if(ie.ht_operation && ((ie.ht_operation[2] & WLAN_HTINFO_NONHT_PRESENT) || (ie.ht_operation[2] & WLAN_HTINFO_OPMODE_MIXED)))
		non_ht = 1;
	if(non_ht)
		nbss->flag |= NBSS_NON_HT;

	/* FIXME: endian issue? */
	nbss->capability = resp->capability;

	if(ie.ds_params)
		nbss->channel = *ie.ds_params;

	if(ie.rsn_ie)
	{
		nbss->sec = NBSS_SEC_WPA2;
		if(!(nbss->rsn_info))
			nbss->rsn_info = (struct wpa_ie_data *)os_api_alloc(sizeof(struct wpa_ie_data), GFP_KERNEL);
		if(wlan_parse_wpa_rsn_ie(ie.rsn_ie, WLAN_IE_CONTEXT_TO_LEN(ie.rsn_ie), nbss->rsn_info, 1) < 0)
		{
			os_api_free(nbss->rsn_info);
			nbss->rsn_info = NULL;
		}
	}
#ifdef CONFIG_WAPI
	else if(ie.wapi_ie)
	{
		nbss->sec = NBSS_SEC_WAPI;
		if(!(nbss->rsn_info))
			nbss->rsn_info = (struct wpa_ie_data *)os_api_alloc(sizeof(struct wpa_ie_data), GFP_KERNEL);
		if(wlan_parse_wapi_ie(ie.wapi_ie, WLAN_IE_CONTEXT_TO_LEN(ie.wapi_ie), nbss->rsn_info) < 0)
		{
			os_api_free(nbss->rsn_info);
			nbss->rsn_info = NULL;
		}
	}
#endif
	else
	{
		nbss->sec = NBSS_SEC_NON;
		if(nbss->rsn_info)
		{
			os_api_free(nbss->rsn_info);
			nbss->rsn_info = NULL;
		}
	}
	
	if(ie.wpa_ie)
	{
		nbss->sec |= NBSS_SEC_WPA;
		if(!(nbss->wpa_info))
			nbss->wpa_info = (struct wpa_ie_data *)os_api_alloc(sizeof(struct wpa_ie_data), GFP_KERNEL);
		if(wlan_parse_wpa_rsn_ie(ie.wpa_ie + 4, WLAN_IE_CONTEXT_TO_LEN(ie.wpa_ie) - 4, nbss->wpa_info, 0) < 0)
		{
			os_api_free(nbss->wpa_info);
			nbss->wpa_info = NULL;
		}
	}
	else
	{
		if(nbss->wpa_info)
		{
			os_api_free(nbss->wpa_info);
			nbss->wpa_info = NULL;
		}
	}
	
	if(ie.wps_ie)
	{
		nbss->sec |= NBSS_SEC_WPS;
#ifdef CONFIG_WPS
		nbss->wps_info = 0;
		if(ie.wps_ie && (wps_msg_parse((struct wps_msg *)((u8*)ie.wps_ie + 4), 
				(((struct wlan_ie_generic *)(ie.wps_ie - 2))->len - 4), &attr) == 0))
		{
			if((attr.selected_registrar) && (*attr.selected_registrar == 1))
				nbss->wps_info |= NBSS_WPS_ST_SELECTED_REGISTRAR;
			if((attr.dev_password_id))
			{
				u32 value=0;
				/* attr.dev_password_id may not alignment 4 */
				value = attr.dev_password_id[0] << 8 | attr.dev_password_id[1];
				if(value == DEV_PW_PUSHBUTTON)
					nbss->wps_info |= NBSS_WPS_ST_PUSHBUTTON;
			}
		}
#endif
	}

	if(((nbss->sec & (NBSS_SEC_WEP | NBSS_SEC_WPA | NBSS_SEC_WPA2 | NBSS_SEC_WAPI)) == 0) &&
		(nbss->capability & WLAN_CAPABILITY_PRIVACY)) {
		nbss->sec = NBSS_SEC_WEP;
	}

	if(ie.wme)
	{
		struct wlan_ie_wme_param *wme = (struct wlan_ie_wme_param *)(ie.wme - 2);
		nbss->qinfo = wme->qosinfo;
		nbss->flag |= NBSS_SUPPORT_WMM;

		if(!(nbss->wme_info))
			nbss->wme_info = (struct wme_ie_data *)os_api_alloc(sizeof(struct wme_ie_data), GFP_KERNEL);
		if(wlan_parse_wme_ie((struct wm_vif *)nbss, ie.wme, WLAN_IE_CONTEXT_TO_LEN(ie.wme), 1) < 0)
		{
			os_api_free(nbss->wme_info);
			nbss->wme_info = NULL;
		}
	}
	else
	{
		if(nbss->wme_info)
		{
			os_api_free(nbss->wme_info);
			nbss->wme_info = NULL;
		}
	}
#ifdef CONFIG_LYNX_IBSS
	nbss->supp_rates = supported_rates;
	if(cap)
	{
		nbss->ampdu_params = cap->ht_cap.ampdu_params;
		nbss->supp_rates |= ht_rate_to_bits(&cap->ht_cap.supported_mcs_set[0]);
	}

	memcpy(nbss->bssid, fm->addr3, WLAN_ADDR_LEN);
	
	nbss->timestamp[0] = resp->timestamp[0];
	nbss->timestamp[1] = resp->timestamp[1];

	if(ie.ibss_ie)
	{
		nbss->atim_window = wtohs(((struct wlan_ie_ibss *)(ie.ibss_ie-2))->atim_window);
	}
#endif // CONFIG_LYNX_IBSS

	return;
}

int scan_cb_handler(void)
{
	struct scan_cb *now = my_wlan_dev->scan_cb_list, *next=NULL;
#if 0
	int i;
	struct neighbor_bss *nbss=NULL;

	printk("%s(): nbss_num = %d\n", __FUNCTION__, my_wlan_dev->nbss_num);

	for(i = 0; i < MAX_NEIGHBOR_BSS_NUM; i++)
	{
		nbss = &my_wlan_dev->site_survey[i];
		if(!(nbss->flag & NBSS_ENABLE))
			continue;
		printk("nbss(%d): ch=%d, ssid=%s\n", i, nbss->channel, nbss->ssid);
	}		
#endif

	/* trigger call back function */
	while(now)
	{
		now->func(now->bss, now->sta, now->data);
		next = now->next;
		os_api_free(now);
		now = next;
	}

	my_wlan_dev->scan_cb_list = NULL;

	return 0;
}

void add_to_scan_cb_list(int (*cb)(struct wm_vif *, struct sta_context *, unsigned long), 
						struct wm_vif *bss, struct sta_context *sta, unsigned long data)
{
	struct scan_cb *entry = (struct scan_cb *)os_api_alloc(sizeof(struct scan_cb), GFP_KERNEL);
	if(entry)
	{
		entry->func = cb;
		entry->bss = bss;
		entry->sta = sta;
		entry->data = data;
		entry->next = my_wlan_dev->scan_cb_list;
		my_wlan_dev->scan_cb_list = entry;
	}
}

void flush_scan_cb_list(void)
{
	struct scan_cb *now = my_wlan_dev->scan_cb_list, *next=NULL;

	while(now)
	{
		next = now->next;
		os_api_free(now);
		now = next;
	}

	my_wlan_dev->scan_cb_list = NULL;
}
