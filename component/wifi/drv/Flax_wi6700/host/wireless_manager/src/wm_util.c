/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file wm_util.c
*   \brief  
*   \author Montage
*/

/*=============================================================================+
| Included Files
+=============================================================================*/
#include "init.h"
#include "core.h"
#include "wlan_def.h"
#include <wm_sta.h>
#include <wm_msg.h>
#include <wm_mlme.h>
#include <wm_util.h>
#include <wm_scan.h>
#include <wlan_ie.h>
#include "lynx_debug.h"
#include "wci.h"
#ifdef CONFIG_WPA
#include <wm_wpa.h>
#include <sta_wpa.h>
#endif
#ifdef CONFIG_WAPI
#include <wm_wapi.h>
#endif
#ifdef CONFIG_WPS
#include <wps.h>
#include <eap.h>
#endif

/*=============================================================================+
| Functions
+=============================================================================*/

void wm_handle_timeout(struct sta_context *sta, unsigned char event)
{
	/* TODO: recheck it */
#if 1
	if((event<BSS_WPA_GROUP_REKEY_TIMEOUT) && (sta->flags == 0))	
	{
		lynx_dbg(LYNX_DBG_WM, "%s(), wrong STA, sta=%x, event=%d\n", __func__, (unsigned int)sta, event);
		return;
	}
#else
	lynx_dbg(LYNX_DBG_WM, "%s(), sta=%x, event=%d\n", __func__, (unsigned int)sta, event);
#endif

	switch(event)
	{
#ifdef CONFIG_WAPI
		case STA_WAPI_TX_TIMEOUT:
		{
			wapi_timeout(sta);
			break;
		}
		case STA_WAPI_PTK_REKEY_TIMEOUT:
		{
			wapi_ptk_rekey(sta);
			break;
		}
		case BSS_WAPI_GROUP_REKEY_TIMEOUT:
		{
			wapi_gtk_rekey((struct wm_vif *)sta);	
			break;
		}
#endif
#ifdef CONFIG_WPA
		case STA_WPA_TX_TIMEOUT:
		{
			wpa_timeout(sta);
			break;
		}
		case STA_WPA_PTK_REKEY_TIMEOUT:
		{
			ap_wpa_rekey_ptk(sta);
			break;
		}
#endif
		case STA_CONNECT_TIMEOUT:
		{
			struct wm_vif *sta_vif = (struct wm_vif *) sta;
			unsigned int next_time = 0;
			unsigned short channel_map = 0x3FFF;

			if((!sta_vif) || (!sta_vif->sta_list))
			{
				break;
			}

			if(!(sta_vif->sta_list->flags & WLAN_STA_CONNECTED))
			{
				next_time = 5 * WLAN_TIME_UNIT;
				lynx_wci_beginscan_cmd(sta_vif->drv_vif, channel_map, 0, sta_vif->ssid, sta_vif->ssid_len);
				add_to_scan_cb_list(sta_connect, sta_vif, sta_vif->sta_list, 0);
			}

			if(next_time)
			{
				wm_add_timer((unsigned int)sta, STA_CONNECT_TIMEOUT, next_time);
			}

			//mtos_printk("%s(%d): STA_CONNECT_TIMEOUT add next timeout=%d\n", __FUNCTION__, __LINE__, next_time);
			/* Do keep alive in the firmware */
			break;
		}
#ifdef CONFIG_WPA
		case BSS_WPA_GROUP_REKEY_TIMEOUT:
		{
			wpa_group_rekey((struct wm_vif *)sta);	
			break;
		}
		case BSS_4WAY_HANDSHAKE_TIMEOUT:
		{
			int reconnect=0;

			if((sta->wpa_ctx) && ((sta->wpa_ctx->flags & 
					(WPA_PTK_VALID|WPA_GTK_VALID)) != (WPA_PTK_VALID|WPA_GTK_VALID)))
			{
				if((sta->vif->role == WIF_STA_ROLE) || (sta->vif->role == WIF_P2P_CLIENT_ROLE))
					reconnect = 1;
				/* 4 Way Handshake is not completed */
				sta_disconnect(sta->vif, sta->addr, reconnect, WLAN_REASON_4WAY_HANDSHAKE_TIMEOUT);
			}
			break;
		}
		case BSS_TKIP_COUNTERMEASURE:
		{
			if((struct wm_vif *)sta)
			{
				((struct wm_vif *)sta)->flag &= ~VIF_FLG_TKIP_COUNTERMEASURE;
				((struct wm_vif *)sta)->mic_failure_count = 0;
				lynx_dbg(LYNX_DBG_WM, "TKIP COUNTERMEASURE Stop, time = %d\n", os_current_time()/100);
			}
			break;
		}
#endif
		case BSS_GET_LINK_STATUS:
		{
			struct wm_vif *vif = (struct wm_vif *)sta;

			if(vif && (vif->flag & VIF_FLG_ENABLE))
			{
				lynx_wci_get_vif_link_st_cmd(vif->drv_vif);
				
				wm_add_timer((unsigned int)vif, BSS_GET_LINK_STATUS, 1 * WLAN_TIME_UNIT);
			}
			break;
		}
#ifdef CONFIG_WPS
		case WPS_PBC_TIMEOUT:
		case WPS_PIN_TIMEOUT:
		case WPS_M2D_TIMEOUT:
		case WPS_SET_SELECTED_REGISTRAR_TIMEOUT:
		case WPS_AP_SETUP_LOCKED_TIMEOUT:
		{
			wps_timeout_handler((unsigned int)sta, event);
			break;
		}
		case EAPOL_REXMIT_TIMEOUT:
   		{
			ap_eapol_rexmit(sta->eap_ctx);
			break;
   		}
#endif	// CONFIG_WPS
		case SYSTEM_MAINTAIN_DHCP:
		{
#ifdef CONFIG_LYNX_OS_UCOS
			struct wm_vif *vif = (struct wm_vif *)sta;
			ethernet_device_t *eth_dev = get_ethernet_dev();
			u8 ipaddr[4] = {0};
			u8 netmask[4]= {0};
			u8 gw[4]= {0};
			u8 primarydns[4]= {0};
			u8 alternatedns[4]= {0};

			if(!vif || !(vif->flag & VIF_FLG_ENABLE) || (vif->role != WIF_STA_ROLE))
			{
				mtos_printk("SYSTEM_MAINTAIN_DHCP : vif is not valid\n");
				break;
			}
			
			if(!vif->sta_list)
			{
				mtos_printk("SYSTEM_MAINTAIN_DHCP : sta is not valid\n");
				break;
			}
				
			if((vif->sta_list->flags & (WLAN_STA_AUTHORIZED | WLAN_STA_CONNECTED | WLAN_STA_VALID)) != (WLAN_STA_AUTHORIZED | WLAN_STA_CONNECTED | WLAN_STA_VALID))
			{
				mtos_printk("SYSTEM_MAINTAIN_DHCP : sta flags is not valid (0x%x)\n", vif->sta_list->flags);
				break;
			}

			if(eth_dev)
			{
				get_net_device_addr_info(eth_dev, ipaddr, netmask, gw, primarydns, alternatedns);

				if((0 == ipaddr[0]) && (0 == ipaddr[1]) && (0 == ipaddr[2]) && (0 == ipaddr[3]))
				{
					mtos_printk("re-trigger the DHCP setup\n");
					if(wm_manager_dhcp() < 0)
						wm_add_timer((unsigned int)vif, SYSTEM_MAINTAIN_DHCP, 5 * WLAN_TIME_UNIT);
				}
			}
#endif	// CONFIG_LYNX_OS_UCOS
			break;
		}
		default:
			break;
	}
}
