/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file wm_mlme.h
*   \brief  MLME of access point
*   \author Montage
*/

/*=============================================================================+
| Included Files
+=============================================================================*/
#include "wlan_def.h"
#include "init.h"
#include "core.h"
#include <wm_sta.h>
#include <wm_mlme.h>
#include <wm_msg.h>
#include <wlan_ie.h>
#include <wm_scan.h>
#ifdef CONFIG_WPA
#include <wm_wpa.h>
#include <sta_wpa.h>
#endif
#ifdef CONFIG_WAPI
#include <wm_wapi.h>
#endif
#ifdef CONFIG_WPS
#include <wlan_util.h>
#include <wps.h>
#include <eap.h>
#include <eapol.h>
#endif	// CONFIG_WPS

/*=============================================================================+
| Define
+=============================================================================*/
unsigned char broadcast_addr[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
#define BC_ADDR		broadcast_addr	
#define MAC2STR		ether_ntoa	

/*=============================================================================+
| Functions
+=============================================================================*/

struct sk_buff *alloc_data_frame(struct wm_vif *bss, unsigned short ether_type, unsigned char *daddr, unsigned int data_len, unsigned char **data_ptr)
{
	struct sk_buff *skb = NULL;
	ehdr *ef;

	if((skb = dev_alloc_skb(data_len + SKB_DATA_OFFSET)) != NULL)
	{
		/* SKB_DATA_OFFSET : alloc wrb space */
		/* reference from the lynx_wci_get_new_buf() */
		skb_reserve(skb, SKB_DATA_OFFSET);
		skb_put(skb, data_len);

		ef = (ehdr *)skb->data;
		memcpy(ef->da, daddr, 6);
		memcpy(ef->sa, bss->myaddr, 6);
		//ef->type = ether_type;
		WRITE_BE16((char *)&ef->type, ether_type);
		if(data_ptr)
			*data_ptr = ef->data;
	}

	return skb;
}

void ap_handle_channel_switch(unsigned int channel, unsigned int secondary_ch)
{
	unsigned int bss_map=0;
	int bss_idx;
	struct wm_vif *bss=NULL;

	if((channel != my_wlan_dev->channel) || (secondary_ch != my_wlan_dev->secondary_channel_def))
	{
		/* deassoc all ap's stations */
		for(bss_idx=0; bss_idx < VIF_MAX_NUM; bss_idx++)
		{
			bss = &my_wlan_dev->bss[bss_idx];

			/* FIXME: should consider P2P case */
			if(!(bss->flag & VIF_FLG_ENABLE) || (bss->role != WIF_AP_ROLE))
				continue;
			bss_map |= 1 << bss_idx;
			ap_release_all_sta(bss, STA_FREE_MEM);
		}

		/* setup channel & secondary channel */
		my_wlan_dev->channel = channel;
		my_wlan_dev->secondary_channel_def = my_wlan_dev->secondary_channel = secondary_ch;

		/* re-prepare beacon */
		for(bss_idx=0; bss_idx < VIF_MAX_NUM; bss_idx++)
		{
			if(bss_map & (1<<bss_idx))
			{
				bss = &my_wlan_dev->bss[bss_idx];
				//ap_prepare_beacon(bss);
			}
		}
	}

	lynx_vif_cfg(my_wlan_dev->driver_priv, NULL, VIF_CFG_CHANNEL, channel);
}

/**************************************************************************************/

void data_proc(struct wm_vif *bss, struct sta_context *sta, struct wlan_hdr *fm, int len)
{
#ifdef CONFIG_WPA
	ehdr *ef = (ehdr*) fm;
	
	/* Note: We expect the data frame is transferred to ethernet header in arthur data path */
	len -= 14; // 14 = DA + SA + type

	/* only handle EAPoL packets */
	if(READ_BE16((u8 *)&ef->type) == ETHERTYPE_EAPOL)
	{
		if(bss->role == WIF_STA_ROLE)
			sta_receive_eapol(sta, (struct eapol_hdr *)ef->data, len);
		else
			ap_proc_eapol(bss, sta, (struct eapol_hdr *)ef->data, len);
	}
#endif
#ifdef CONFIG_WAPI
	if(ef->type == ETHERTYPE_WAPI)
	{
		if(bss->role == WIF_STA_ROLE)
			sta_proc_wai(sta, ef->data, len);
		else
			ap_proc_wai(bss, sta, ef->data, len);
	}
#endif
}

/*!-----------------------------------------------------------------------------
 * function: wm_frame_handler()
 *
 *      \brief	handle all frames from driver
 *		\param 	wb: wbuf pointer of frame
 *				sta: which station		
 *      \return	void
 +----------------------------------------------------------------------------*/
void wm_frame_handler(struct sk_buff *skb, int vif_idx, int sta_idx, int type, int rssi)
{
	struct wm_vif *mybss=NULL;
	struct sta_context *sta=NULL;
	
	if(type == WM_WF_TYPE_MGMT)
	{
		/* FIXME: host driver only receive beacon/probe resp */
		ap_handle_beacon_prob_resp((struct wlan_hdr *)skb->data, skb->len, rssi);
	}
	else
	{
		mybss=&my_wlan_dev->bss[vif_idx];
		sta = &mybss->sta_list[sta_idx];
		
		if(sta && (sta->flags & WLAN_STA_VALID))
		{
			data_proc(mybss, sta, (struct wlan_hdr *)skb->data, skb->len);
		}
		/* FIXME: else : may lookup the address to find the sta */
	}

	return;
}

/* TODO: not complete */
#if 0
int mic_err_handler(void)
{
	if((wb->secst & WBUF_RX_MIC_ERR) && (sta != NULL))
	{
		/* handle the tkip countermeasure function */
		lynx_dbg(LYNX_DBG_WM, "WBUF receive MIC ERROR\n");
		if(client_mode)
		{
			struct wpa_ctx *ctx = sta->wpa_ctx;
			unsigned short info;

			info = KEY_INFO_REQUEST | KEY_INFO_ERROR;
			if(ctx->flags & WPA_PTK_VALID)
				info |= KEY_INFO_MIC;
			if(!wb->bcmc)
				info |= KEY_INFO_KEY_TYPE;
			inc_byte_array(ctx->req_replay_counter, WPA_REPLAY_COUNTER_LEN);
			sta_wpa_send_eapol(mybss,
							   info,
							   NULL, NULL, 
							   NULL, 0, 0, 0, ctx->req_replay_counter);
			if(++(mybss->mic_failure_count) == 2)
			{
				lynx_dbg(LYNX_DBG_WM, "TKIP COUNTERMEASURE Start, time = %d\n", time(0));
				ap_sta_deauth(sta, WLAN_REASON_MIC_FAILURE, STA_RESET_FLAGS);
				/* do reconnect process */
				wm_del_timer((unsigned int) sta->bss, STA_MODE_KEEP_ALIVE);
				wm_add_timer((unsigned int) sta->bss, STA_MODE_KEEP_ALIVE, STA_RSNA_TIMEOUT);
			}
		}
		else
		{
			if(++(mybss->mic_failure_count) == 2)
			{
				lynx_dbg(LYNX_DBG_WM, "TKIP COUNTERMEASURE Start, time = %d\n", time(0));
				mybss->flag |= VIF_FLG_TKIP_COUNTERMEASURE;
				ap_release_all_sta(mybss, STA_FREE_MEM);
				wpa_group_init(mybss);
			}
		}
		wm_del_timer((unsigned int)mybss, BSS_TKIP_COUNTERMEASURE);
		wm_add_timer((unsigned int)mybss, BSS_TKIP_COUNTERMEASURE, 60*WLAN_TIME_UNIT);
	}
}
#endif
