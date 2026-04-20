
#ifdef CONFIG_LYNX_OS_UCOS
/* ucos */
#include <mt_type.h>
#include <os/mtos_misc.h>
#include <util/list.h>
#include <porting.h>
#include <string.h>
#endif

#include "init.h"
#include "core.h"
#include "wlan_def.h"
#include "wm_msg.h"
#include "wm_vif.h"
#include "wm_ctrl.h"
#include "wm_sta.h"
#include "wci.h"
#include "lynx_debug.h"

/* ================= Control API Message ================= */

struct sk_buff * alloc_ctrl_msg(int type, int vif_idx, int size)
{
	struct sk_buff *skb=NULL;
	struct wm_msg *msg;

	if((skb = wm_msgq_alloc(size)) != NULL)
	{
		msg = (struct wm_msg *)skb->cb;
		msg->type = WM_MSG_CTRL;
		msg->u.ctrl.type = type;
		msg->u.ctrl.vif_idx = vif_idx;
	}

	return skb;
}

struct sk_buff *wm_ctrl_set_config(int vif_idx, unsigned char *addr, unsigned char *ssid, 
								unsigned int security, unsigned int cipher, unsigned char *key, 
								int key_len)
{
	struct sk_buff *skb = alloc_ctrl_msg(CTRL_CMD_CONFIG, vif_idx, sizeof(struct wm_ctrl_conf));
	struct wm_ctrl_conf *conf;

	if(skb)
	{
		conf = (struct wm_ctrl_conf *)skb->data;
		
		if(addr)
			memcpy(conf->mac_addr, addr, 6);
		
		if(ssid)
		{
			memcpy(conf->ssid, ssid, 32);
			conf->ssid[32] = 0;
		}
		
		conf->security = security;
		conf->cipher = cipher;
		
		/* FIXME: set the key idx in the WEP setting */
		conf->key_idx = 0;

		if(key && key_len)
		{
			memcpy(conf->key, key, key_len);
			conf->key[key_len] = 0;
			conf->key_len = key_len;
		}
	
		if(wm_msgq_write(skb) < 0)
			skb = NULL;
	}

	return skb;
}

struct sk_buff *wm_ctrl_connect(int vif_idx)
{
	struct sk_buff *skb = alloc_ctrl_msg(CTRL_CMD_CONNECT, vif_idx, 0);

	if(skb)
	{
		if(wm_msgq_write(skb) < 0)
			skb = NULL;
	}

	return skb;
}

struct sk_buff *wm_ctrl_disconnect(int vif_idx)
{
	struct sk_buff *skb = alloc_ctrl_msg(CTRL_CMD_DISCONNECT, vif_idx, 0);

	if(skb)
	{
		if(wm_msgq_write(skb) < 0)
			skb = NULL;
	}

	return skb;
}

struct sk_buff *wm_ctrl_scan(int vif_idx)
{
	struct sk_buff *skb = alloc_ctrl_msg(CTRL_CMD_SCAN, vif_idx, 0);

	if(skb)
	{
		if(wm_msgq_write(skb) < 0)
			skb = NULL;
	}

	return skb;
}

struct neighbor_bss *wm_ctrl_scan_results(void)
{
	return my_wlan_dev->site_survey;
}

/* ================= WM_MANAGER Handle the CTRL Message ================= */

void wm_ctrl_config_vif(struct wm_vif *vif, struct wm_ctrl_conf *conf)
{
	struct sta_context *sta;

	/* FIXME: the sta in STA mode has been alloced at wm_reload_config */
	if((vif->role == VIF_STA_MODE) && ((sta = vif->sta_list) == NULL))
		return;

	vif->ssid_len = strlen(conf->ssid);
	memcpy(vif->ssid, conf->ssid, vif->ssid_len);

	wm_setup_security(vif, conf->cipher, conf->security, conf->key, conf->key_idx, 0, 1);
}

void wm_ctrl_handler(struct wm_msg *msg, void *data)
{
	int vif_idx = msg->u.ctrl.vif_idx;
	int ctrl_type = msg->u.ctrl.type;
	int len;
	struct wm_vif *vif = &my_wlan_dev->bss[vif_idx];
	struct sta_context *sta;
	struct wm_ctrl_conf *conf;
	unsigned char *key=NULL;

	if(vif_idx >= VIF_MAX_NUM)
		return;

	switch(ctrl_type)
	{
		case CTRL_CMD_CONFIG:
			conf = (struct wm_ctrl_conf *)data;

			/* FIXME: is_valid_ether_addr(conf->mac_addr) => mac_addr has meaning? */
			if((len = strlen(conf->ssid)) != 0)
			{
				strncpy(vif->ssid, conf->ssid, len);
				vif->ssid_len = len;
				printk("%s(): ssid=%s\n", __FUNCTION__, vif->ssid);
			}

			/* FIXME: WEP with 4 sets ? */
			if(conf->key_len)
			{
				key = conf->key;
				printk("%s(): key=%s\n", __FUNCTION__, key);
			}
			wm_setup_security(vif, conf->cipher, conf->security, key, conf->key_idx, 1, 1);
			break;
		case CTRL_CMD_CONNECT:
			/* FIXME: only process STA role now */
			sta = vif->sta_list;
			if(sta)
			{
				/* free sta for the case : connect to the different AP */
				if(sta->flags & WLAN_STA_CONNECTED)
					ap_sta_deauth(sta, WLAN_REASON_DEAUTH_LEAVING, STA_FREE_MEM);
				else
					ap_release_sta(sta, STA_FREE_MEM);
			}

			sta = ap_add_sta(vif, vif->bssid);

			if(sta)
			{
				sta->flags = WLAN_STA_VALID;
				sta_connect(vif, sta, 0);
			}
			break;
		case CTRL_CMD_DISCONNECT:
			/* FIXME: STA_RESET_FLAGS or STA_FREE_MEM ? */
			if((sta = vif->sta_list) != NULL)
			{
				sta_disconnect(vif, sta->addr, 0, WLAN_REASON_DEAUTH_LEAVING);
			}
			break;
		case CTRL_CMD_SCAN:
			lynx_wci_beginscan_cmd(vif->drv_vif, 0x3FFF, 0, vif->ssid, vif->ssid_len);
			//add_to_scan_cb_list(sta_connect, sta_vif, sta_vif->sta_list, 0);
			break;
		default:
			break;
	}
}
