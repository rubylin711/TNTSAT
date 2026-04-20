/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file wm_sta.c
*   \brief  
*   \author Montage
*/

/*=============================================================================+
| Included Files
+=============================================================================*/
#include "wlan_def.h"
#include "init.h"
#include "mac_ctrl.h"
#include "core.h"
#include "wci.h"
#include <wm_sta.h>
#include <wm_msg.h>
#include <wm_mlme.h>
#include <wlan_ie.h>
#include "lynx_debug.h"
#ifdef CONFIG_WPA
#include <wpa.h>
#include <wm_wpa.h>
#include <sta_wpa.h>
#endif
#ifdef CONFIG_WAPI
#include <wm_wapi.h>
#endif
#include "mlme_api.h"

/*=============================================================================+
| Functions
+=============================================================================*/
struct sta_context *ap_alloc_sta(void)
{
	struct sta_context *sta;

	sta = (struct sta_context *)os_api_alloc(sizeof(struct sta_context), GFP_KERNEL);	

	return sta;
}

struct sta_context *ap_find_sta(struct wm_vif *bss, unsigned char *addr)
{
	struct sta_context *sta;

	for(sta = bss->sta_list; sta; sta = sta->next)
	{
		if(!(sta->flags & WLAN_STA_VALID))
			continue;
		if(!memcmp(sta->addr, addr, WLAN_ADDR_LEN))
			break;
	}

	return sta;
}

void ap_free_sta(struct sta_context *sta)
{
	struct sta_context *one, *prev;
	struct wm_vif *bss;

	bss = sta->vif;
	one = bss->sta_list;
	prev = 0;
	while(one)
	{
		if(one == sta)
		{
			if(prev)
				prev->next = sta->next;	
			else
				bss->sta_list = sta->next;

			os_api_free(sta);
			break;
		}
		prev = one;
		one = one->next;
	}

	bss->sta_num--;
}

void ap_free_sta_all(struct wm_vif *bss)
{
	struct sta_context *sta, *prev;

	sta = bss->sta_list;
	while(sta)
	{
		prev = sta;	
		sta = sta->next;
		
		os_api_free(prev);
	}

	bss->sta_list = NULL;
	bss->sta_num = 0;
}

void ap_release_sta(struct sta_context *sta, char release)
{
	/****
		NOTE: The wif mac addr is saved in bss->sta_list (created in wm_reload_config), 
		so that sta can't be free, and the release code STA_RESET_FLAGS is used.
	 ***/

	/* checking WLAN_STA_VALID is for one case:
	   arthur release sta & sta send deauth at the same time (casuse double free sta) */
	if(!(sta->flags & WLAN_STA_VALID))
		return;

	wm_del_timer((u32)sta, STA_ALL_TIMEOUT);

#ifdef CONFIG_WPA
	if(sta->wpa_ctx)
	{
		wpa_release(sta);
	}
#endif
#ifdef CONFIG_WAPI
	if(sta->wapi_ctx)
	{
		wapi_release(sta);
	}
#endif

#ifdef CONFIG_WPS
	if (sta->eap_ctx) {
		eap_deinit(sta->eap_ctx);
		sta->eap_ctx = 0;
	}
#endif

	sta->flags = 0;

	/* TODO: wci del_sta or disconnect */

	if(release != STA_RESET_FLAGS)
		ap_free_sta(sta);

	/* TODO: remove sta */
	// int lynx_wci_sta_remove_cmd(struct lynx_vif *vif, unsigned int sta_idx, u32 reason);
}

void ap_sta_deauth(struct sta_context *sta, int reason, char release)
{
	int sta_idx;

	if((sta_idx = sta_map2idx(sta->vif->drv_vif, sta->addr)) >= 0)
		lynx_wci_disconnect_cmd(sta->vif->drv_vif, sta_idx, reason);
	
	ap_release_sta(sta, release);
}

void ap_release_all_sta(struct wm_vif *bss, int policy)
{
	struct sta_context *sta, *prev;

	sta = bss->sta_list;
	while(sta)
	{
		prev = sta;	
		sta = sta->next;

		/* TODO: trigger wci sta disconnect */
		ap_release_sta(prev, policy);
	}
}

struct sta_context *ap_add_sta(struct wm_vif *bss, unsigned char *addr)
{
	struct sta_context *sta;

	if(addr)
	{
		sta = ap_find_sta(bss, addr);

		if(sta)
			return sta;
	}

	if(bss->sta_num >= my_wlan_dev->sta_max_num)
		return 0;

	sta = (struct sta_context *)os_api_alloc(sizeof(struct sta_context), GFP_KERNEL);	

	if(sta == 0) 
		return 0;

	/* initialize STA info data */
	if(addr)
		memcpy(sta->addr, addr, WLAN_ADDR_LEN);

	sta->flags |= WLAN_STA_VALID; 
	sta->next = bss->sta_list;
	sta->vif = bss;
	bss->sta_list = sta;

	bss->sta_num++;
	
	return sta;
}

int sta_connect(struct wm_vif *sta_bss, struct sta_context *sta, unsigned long data)
{
	struct neighbor_bss *nbss=NULL;
	int ret=-1, i;
	int reconnect_timeout = (5 * WLAN_TIME_UNIT);
	struct wlan_ie_info ie_info;
#ifdef CONFIG_WPA
	int wpa_ie_len=0, is_rsn=0;
	unsigned char *wpa_ie = NULL, ie_context[256];
	struct wlan_ie_generic *ptr;
	int size=0, change=0, cipher=0;
	unsigned char tmp[256];
#endif
	
	if((sta_bss == NULL) || (sta == NULL))
		goto out;

	if(!(sta_bss->flag & VIF_FLG_ENABLE) || 
		   !((sta_bss->role == WIF_STA_ROLE) || (sta_bss->role == WIF_P2P_CLIENT_ROLE)))
		goto out;
	
	for(i = 0; i < MAX_NEIGHBOR_BSS_NUM; i++)
	{
		nbss = &my_wlan_dev->site_survey[i];
		if(!(nbss->flag & NBSS_ENABLE))
			continue;

#ifdef CONFIG_WPS
		if((sta_bss->wps) && (sta_bss->wps->sta_status == WPS_STA_SCANING))
		{
			if(!(sta_bss->wps->registrar->pbc) || 
				!(nbss->wps_info & NBSS_WPS_ST_PUSHBUTTON))
			{
				continue;
			}
			else
			{
				sta_bss->ssid_len = strlen(nbss->ssid);
				memcpy(sta_bss->ssid, nbss->ssid, sta_bss->ssid_len);
				sta_bss->ssid[sta_bss->ssid_len] = '\0';
				WLAN_DBG("%s(): find the bss (%s) contained the PUSH BUTTON\n", __FUNCTION__, sta_bss->ssid);
			}
		}
		else
#endif
		{
			if(memcmp(nbss->ssid, sta_bss->ssid, sta_bss->ssid_len))
				continue;
		}
	
		memset(&ie_info, 0, sizeof(struct wlan_ie_info));

		/* replace the old ap bssid info */
		memcpy(sta_bss->bssid, nbss->addr, WLAN_ADDR_LEN);
		memcpy(sta->addr, nbss->addr, WLAN_ADDR_LEN);

		/* set tsf related parameters */
		sta_bss->beacon_interval = nbss->beacon_interval;
		if(nbss->dtim_period)
			sta_bss->dtim_period = nbss->dtim_period;

		sta_bss->peer_phy_cap = 0;

		if(nbss->band & NBSS_BAND_B)
			sta_bss->peer_phy_cap |= AP_CAP_11B;
		if(nbss->band & NBSS_BAND_G)
			sta_bss->peer_phy_cap |= AP_CAP_11G;
		if(nbss->band & NBSS_BAND_N)
		{
			sta_bss->flag |= VIF_FLG_WMM;
			sta_bss->peer_phy_cap |= AP_CAP_11N;
			sta_bss->ht_capability = my_wlan_dev->ht_capability & nbss->ht_capability;
		}

		if(nbss->flag & NBSS_SUPPORT_WMM)
		{
			sta_bss->flag |= VIF_FLG_WMM;
		}

		lynx_dbg(LYNX_DBG_WM, "%s(): auth_cap=%x, AUTH_CAP_WPA2=%x\n", __FUNCTION__, sta_bss->auth_capability, AUTH_CAP_WPA2);
#ifdef CONFIG_WAPI
		if(sta_bss->auth_capability & AUTH_CAP_WAPI)
		{
			if(!(nbss->rsn_info) || !(nbss->sec & NBSS_SEC_WAPI))
			{
				/* expect rsn_info existing */
				break;
			}
		}
		else
#endif
#ifdef CONFIG_WPA
		if(nbss->sec & (NBSS_SEC_WPA | NBSS_SEC_WPA2))
		{
			sta_bss->auth_capability &= (AUTH_CAP_OPEN | AUTH_CAP_WPA | AUTH_CAP_WPA2 | AUTH_CAP_CIPHER_TKIP | AUTH_CAP_CIPHER_CCMP | AUTH_CAP_KEY_MGT_PSK);
			cipher = (sta_bss->auth_capability & ALL_CIPHER);

			if(!!(nbss->sec & NBSS_SEC_WPA) != !!(sta_bss->auth_capability & AUTH_CAP_WPA))
			{
				if(nbss->sec & NBSS_SEC_WPA)
					sta_bss->auth_capability |= AUTH_CAP_WPA;
				else
					sta_bss->auth_capability &= ~AUTH_CAP_WPA;

				change = 1;
			}

			if(!!(nbss->sec & NBSS_SEC_WPA2) != !!(sta_bss->auth_capability & AUTH_CAP_WPA2))
			{
				if(nbss->sec & NBSS_SEC_WPA2)
					sta_bss->auth_capability |= AUTH_CAP_WPA2;
				else
					sta_bss->auth_capability &= ~AUTH_CAP_WPA2;

				change = 1;
			}

			if(nbss->wpa_info)
			{
				if((nbss->wpa_info->pairwise_cipher & sta_bss->auth_capability) != nbss->wpa_info->pairwise_cipher)
				{
					cipher = nbss->wpa_info->pairwise_cipher;
					change = 1;
				}
			}

			if(nbss->rsn_info)
			{
				if((nbss->rsn_info->pairwise_cipher & sta_bss->auth_capability) != nbss->rsn_info->pairwise_cipher)
				{
					cipher = nbss->rsn_info->pairwise_cipher;
					change = 1;
				}
			}

			if(change)
			{
				sta_bss->auth_capability &= ~(AUTH_CAP_CIPHER_TKIP | AUTH_CAP_CIPHER_CCMP);
				sta_bss->auth_capability |= (cipher | AUTH_CAP_KEY_MGT_PSK);

				lynx_dbg(LYNX_DBG_WM, "%s(change): auth_cap=%x, AUTH_CAP_WPA2=%x\n", __FUNCTION__, sta_bss->auth_capability, AUTH_CAP_WPA2);

				if(sta_bss->wpa_ie)
				{
					os_api_free(sta_bss->wpa_ie);
					sta_bss->wpa_ie = NULL;
					sta_bss->wpa_ie_len = 0;
				}

				if(sta_bss->rsn_ie)
				{
					os_api_free(sta_bss->rsn_ie);
					sta_bss->rsn_ie = NULL;
					sta_bss->rsn_ie_len = 0;
				}

				if((sta_bss->auth_capability & AUTH_CAP_WPA) && ((size = ap_encode_wpa_ie(sta_bss, tmp, 0)) > 0))
				{
					if((sta_bss->wpa_ie = os_api_alloc(size, GFP_KERNEL)) != NULL)
					{
						memcpy(sta_bss->wpa_ie, tmp, size);
						sta_bss->wpa_ie_len = size;
					}
				}

				if((sta_bss->auth_capability & AUTH_CAP_WPA2) && ((size = ap_encode_wpa_ie(sta_bss, tmp, 1)) > 0))
				{
					if((sta_bss->rsn_ie = os_api_alloc(size, GFP_KERNEL)) != NULL)
					{
						memcpy(sta_bss->rsn_ie, tmp, size);
						sta_bss->rsn_ie_len = size;
					}
				}
			}

			if(sta_bss->rsn_ie) 
			{
				wpa_ie = sta_bss->rsn_ie;
				wpa_ie_len = sta_bss->rsn_ie_len;
				is_rsn = 1;
			} 
			else if(sta_bss->wpa_ie) 
			{
				wpa_ie = sta_bss->wpa_ie + 4; /* skip OUI */
				wpa_ie_len = sta_bss->wpa_ie_len - 4;
				is_rsn = 0;
			}
	
			if(wpa_ie)
			{
				if(sta->wpa_ctx == NULL)
					sta->wpa_ctx = (struct wpa_ctx *) wpa_alloc(sta_bss);
				if(sta->wpa_ctx == NULL)
					goto out;

				if(sta_bss->wpa_group == NULL)
				{
					sta_bss->wpa_group = (struct wpa_group_key *)os_api_alloc(sizeof(struct wpa_group_key), GFP_KERNEL);
					if(sta_bss->wpa_group == NULL)
					{
						goto out;
					}
				}

				if(wpa_setup(sta_bss, sta->wpa_ctx, wpa_ie, wpa_ie_len, is_rsn))
				{
					lynx_dbg(LYNX_DBG_WM, "wpa_setup fail\n");
					wpa_release(sta);
					goto out;
				}
			}
		}
		else
#endif // CONFIG_WPA
		if(nbss->sec & NBSS_SEC_WEP) {
			sta_bss->auth_capability &= (AUTH_CAP_OPEN | AUTH_CAP_AUTO_AUTH_ALG | AUTH_CAP_WEP | AUTH_CAP_CIPHER_WEP40 | AUTH_CAP_CIPHER_WEP104);

			if(sta_bss->auth_capability & AUTH_CAP_WEP) {
				if(sta_bss->auth_capability & AUTH_CAP_CIPHER_WEP40)
					cipher_type = CIPHER_TYPE_WEP40;
				else
					cipher_type = CIPHER_TYPE_WEP104;

				lynx_dbg(LYNX_DBG_WM, "%s: WEP cipher=%x key_idx=%d key=[%s]\n", 
					__FUNCTION__, cipher_type, sta_bss->key_idx, &sta_bss->wep[sta_bss->key_idx][0]);

				/* clear group key */
				sta_set_key((struct sta_context *)sta_bss, CIPHER_TYPE_NONE, 
							KEY_TYPE_GLOBAL_KEY, NULL, sta_bss->key_idx, 1);
				/* setup group key */
				sta_set_key((struct sta_context *)sta_bss, cipher_type, KEY_TYPE_GLOBAL_KEY, 
							&sta_bss->wep[sta_bss->key_idx][0], sta_bss->key_idx, 1);
			}
		}
		
		ap_handle_channel_switch(nbss->channel, nbss->secondary_ch);
		
		ret = i;

		lynx_vif_cfg(my_wlan_dev->driver_priv, sta_bss->drv_vif, VIF_CFG_AUTH_TYPE, 
						sta_bss->auth_capability);
		lynx_vif_cfg(my_wlan_dev->driver_priv, sta_bss->drv_vif, VIF_CFG_SSID, 
						(unsigned long)sta_bss->ssid);
		lynx_vif_cfg(my_wlan_dev->driver_priv, sta_bss->drv_vif, VIF_CFG_HT_CAP, 
						sta_bss->ht_capability);
		lynx_wci_update_vif_cmd(sta_bss->drv_vif);

		ie_info.phy_cap = sta_bss->peer_phy_cap & my_wlan_dev->phy_cap;
		ie_info.ht_cap = sta_bss->ht_capability &  my_wlan_dev->ht_capability;
		if(ie_info.phy_cap & AP_CAP_11B)
			ie_info.supp_rates |= B_RATES;
		if(ie_info.phy_cap & AP_CAP_11G)
			ie_info.supp_rates |= OFDM_RATES;
		if(ie_info.phy_cap & AP_CAP_11N)
			ie_info.supp_rates |= MCS_RATES;

#ifdef CONFIG_WPA
		if(wpa_ie)
		{
			ptr = (struct wlan_ie_generic *)ie_context;
			if(is_rsn)
			{
				ptr->id = WLAN_ELEMID_RSN;
				memcpy(ptr->data, sta_bss->rsn_ie, sta_bss->rsn_ie_len);
				ptr->len = sta_bss->rsn_ie_len;

				ie_info.rsn_ie = ie_context;
			}
			else
			{
				ptr->id = WLAN_ELEMID_VENDOR_SPEC;
				memcpy(ptr->data, sta_bss->wpa_ie, sta_bss->wpa_ie_len);
				ptr->len = sta_bss->wpa_ie_len;
				
				ie_info.wpa_ie = ie_context;
			}
		}
#endif	// CONFIG_WPA

		/* FIXME: how to setup the device status with the AP ROLE? */
		/* TODO: how to setup the device status WIFI_CONNECT_FAILED? */
#if defined(CONFIG_LYNX_OS_UCOS)
		set_device_status(WIFI_CONNECTING);
#endif
		lynx_wci_connect_cmd(sta_bss->drv_vif, sta_bss->bssid, sta_bss->ssid, sta_bss->ssid_len, &ie_info);
		
		break;
	}
	
out:

	if(ret < 0)
		reconnect_timeout = (1*WLAN_TIME_UNIT);

	wm_del_timer((unsigned int)sta_bss, STA_CONNECT_TIMEOUT);
	wm_add_timer((unsigned int)sta_bss, STA_CONNECT_TIMEOUT, reconnect_timeout);

	return ret;
}

int sta_connected(struct wm_vif *bss, unsigned char *addr, int data_len, unsigned char *data)
{
	struct sta_context *sta;
	int i;
	int cipher_type=0;
	int key_type=0;

	if(bss->flag & VIF_FLG_ENABLE)
	{
		if(bss->role != VIF_STA_MODE)
			sta = ap_add_sta(bss, addr);
		else
			sta = bss->sta_list;

		if(sta != NULL)
		{
			sta->flags |= WLAN_STA_CONNECTED;

			/* TODO: parse the ie to fill the related fields */
			lynx_dbg(LYNX_DBG_WM, "%s(%d): sta=%x, auth_capability=%x, "
					"addr=%02x:%02x:%02x:%02x:%02x:%02x\n", 
					__FUNCTION__, __LINE__, sta, bss->auth_capability,
					addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
#ifdef CONFIG_WPA
			if(sta->wpa_ctx)
			{
				/* AuthenticationRequest = true */
				sta_enter_wpa_state(sta, AS_AUTHENTICATION, NULL, 0);
			}
			else
#endif	// CONFIG_WPA
			{
				if(bss->auth_capability & AUTH_CAP_WEP)
				{
					if(bss->auth_capability & AUTH_CAP_CIPHER_WEP40)
						cipher_type = CIPHER_TYPE_WEP40;
					else
						cipher_type = CIPHER_TYPE_WEP104;

					if(bss->role == WIF_STA_ROLE)
						key_type = KEY_TYPE_STA_PAIRWISE_KEY;
					else
						key_type = KEY_TYPE_PAIRWISE_KEY;
    			
					lynx_dbg(LYNX_DBG_WM, "%s(): cipher=%x, key_type=%x\n", __FUNCTION__, cipher_type, key_type);
					/* clear pairwise key */
					sta_set_key(sta, CIPHER_TYPE_NONE, key_type, NULL, 0, 0);

					/* setup pairwise key */
					for(i=0; i<WLAN_WEP_NKID; i++)
					{
						lynx_dbg_dump(LYNX_DBG_WM, "wep ", "", &bss->wep[i][0], WLAN_WEP_MAX_KEYLEN);
						if(bss->wep[i][0] == 0)
							continue;
						sta_set_key(sta, cipher_type, key_type, 
									&bss->wep[i][0], i, (bss->key_idx == i));
					}

					if(bss->role == WIF_STA_ROLE)
					{
						/* clea group key */
						sta_set_key((struct sta_context *)bss, CIPHER_TYPE_NONE, 
									KEY_TYPE_GLOBAL_KEY, NULL, bss->key_idx, 1);
						/* setup group key */
						sta_set_key((struct sta_context *)bss, cipher_type, KEY_TYPE_GLOBAL_KEY, 
									&bss->wep[bss->key_idx][0], bss->key_idx, 1);
					}
				}

				/* FIXME: wpa/wpa2 should set WLAN_STA_AUTHORIZED after 4-way handshake done */
				sta->flags |= WLAN_STA_AUTHORIZED;
				mlme_data_path_ctrl(sta->vif->drv_vif, DATAPATH_CARRIER_ON);
				wm_add_timer((unsigned int)sta->vif, SYSTEM_MAINTAIN_DHCP, 5 * WLAN_TIME_UNIT);

#if defined(CONFIG_LYNX_OS_UCOS)
				/* FIXME: how to setup the device status with the AP ROLE? */
				set_device_status(WIFI_CONNECTED);
#endif
			}
			
			/* sync the sta connect status for the reconnect mechanism */
			wm_del_timer((unsigned int)bss, BSS_GET_LINK_STATUS);
			wm_add_timer((unsigned int)bss, BSS_GET_LINK_STATUS, 1 * WLAN_TIME_UNIT);
		}
	}

	return 0;
}

int sta_disconnect(struct wm_vif *bss, unsigned char *addr, int reconnect, int reason)
{
	struct sta_context *sta;
	int flag = STA_RESET_FLAGS;
	int sta_idx=0;

	if(bss->flag & VIF_FLG_ENABLE)
	{
		if((sta = ap_find_sta(bss, addr)) != NULL)
		{
			/* inform device to disconnect */
			if((sta_idx = sta_map2idx(sta->vif->drv_vif, sta->addr)) >= 0)
				lynx_wci_disconnect_cmd(sta->vif->drv_vif, sta_idx, reason);

			if((bss->role == WIF_AP_ROLE) || (reconnect == 0))
				flag = STA_FREE_MEM;
			ap_release_sta(sta, flag);
			mlme_data_path_ctrl(bss->drv_vif, 0);
			wm_del_timer((unsigned int)sta->vif, SYSTEM_MAINTAIN_DHCP);

			if(bss->role == WIF_STA_ROLE)
			{
				wm_del_timer((unsigned int)bss, STA_CONNECT_TIMEOUT);
				if(reconnect)
				{
					sta->flags |= WLAN_STA_VALID;
					wm_add_timer((unsigned int)bss, STA_CONNECT_TIMEOUT, 1 * WLAN_TIME_UNIT);
				}
			}

#if defined(CONFIG_LYNX_OS_UCOS)
			/* FIXME: how to setup the device status with the AP ROLE? */
			set_device_status(WIFI_DIS_CONNECT);
#endif
		}

		if(!(bss->sta_list) || (bss->role == WIF_STA_ROLE))
		{
			wm_del_timer((unsigned int)bss, BSS_GET_LINK_STATUS);
		}
	}

	return 0;
}

void sta_set_key(struct sta_context *sta, unsigned char cipher_type, unsigned char key_type,
						char *key, char keyidx, char is_txkey)
{
	struct lynx_key key_data, *key_ptr=NULL;
	int key_len=WLAN_KEY_LEN_TKIP;
	void *drv_vif=NULL;
	unsigned char *addr=NULL;

	if(key_type == KEY_TYPE_GLOBAL_KEY)
		drv_vif = ((struct wm_vif *)sta)->drv_vif;
	else
	{
		drv_vif = sta->vif->drv_vif;
		addr = sta->addr;
	}

	if(key)
	{	
		if(cipher_type == CIPHER_TYPE_WEP40)
			key_len = WLAN_KEY_LEN_WEP40;
		else if(cipher_type == CIPHER_TYPE_WEP104)
			key_len = WLAN_KEY_LEN_WEP104;
		else if(cipher_type == CIPHER_TYPE_TKIP)
			key_len = WLAN_KEY_LEN_TKIP;
		else if(cipher_type == CIPHER_TYPE_CCMP)
			key_len = WLAN_KEY_LEN_CCMP;

		key_data.cipher = cipher_type;
		key_data.key_type = key_type;
		key_data.is_txkey = is_txkey;
		key_data.key_len = key_len;
		
		memcpy(key_data.key, key, key_len);
		
		key_ptr = &key_data;
	}
		
	lynx_wci_addkey_cmd(drv_vif, addr, key_ptr, keyidx, key_type);
}
