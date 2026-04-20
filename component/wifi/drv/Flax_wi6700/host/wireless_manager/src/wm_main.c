/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file wm_main.c
*   \brief  configure and bring up wireless manager daemon
*   \author Montage
*/

/*=============================================================================+
| Included Files
+=============================================================================*/
#if defined(CONFIG_LYNX_OS_LINUX)
#include <linux/etherdevice.h>
#endif

#include "init.h"
#include "core.h"
#include <wlan_def.h>
#include <wm_vif.h>
#include <wm_msg.h>
#include <wm_mlme.h>
#include <wm_sta.h>
#include <wm_util.h>
#include <wm_scan.h>
#include <wm_ctrl.h>
#include <wlan_ie.h>
#include "mac_ctrl.h"
#include "wci.h"
#include "lynx_debug.h"
#include <wlan_util.h>

#ifdef CONFIG_WPA
#include <wm_wpa.h>
#include <wpa.h>
#include <sta_wpa.h>
#endif
#ifdef CONFIG_WAPI
#include <wm_wapi.h>
#endif
#ifdef CONFIG_WPS
#include <wlan_util.h>
#include <wps_i.h>
#endif

/*=============================================================================+
| Define
+=============================================================================*/
#define IFNAME "lynx-%d"
#define WLAN_TIMEOUT_MAX_NUM		(LYNX_STA_MAX_NUM * 3)
#define PSQ_AGING_TIME				2000 /* msec */

struct wlan_timeout{
	struct wlan_timeout *next;
	u32 event;
	u32 target;
	u32 expire;
};

#define DYNAMIC_ALLOC_MSGQ

struct wlan_timeout_data {
	struct timer_list timer;
	struct wlan_timeout *free_wlan_timeout;
	struct wlan_timeout *wait_wlan_timeout;
#if defined(DYNAMIC_ALLOC_MSGQ)
	struct wlan_timeout *all_wlan_timeout;
#else
	struct wlan_timeout all_wlan_timeout[WLAN_TIMEOUT_MAX_NUM];
#endif
};

struct wlan_timeout_data wm_timer_data, *wm_timer = &wm_timer_data;

struct wlan_msgq {
	unsigned char msgq_is_inited;
	struct sk_buff_head msgq;
	OS_COMPLETION msgq_complete;
	spinlock_t msgq_lock;
};
struct wlan_msgq msgq_data;

struct wlan_dev my_wlan_dev_obj; 
struct wlan_dev *my_wlan_dev = &my_wlan_dev_obj;

int wm_msgq_init(void);
int wm_msgq_deinit(int is_deinit);

/*=============================================================================+
| Functions
+=============================================================================*/

/*!-----------------------------------------------------------------------------
 * function: wm_timer_trigger()
 *
 *      \brief	system timer invoke this function once time up
 *		\param 	target: target's pointer 
 *				event:	event type		
 *      \return	void
 +----------------------------------------------------------------------------*/
void wm_timer_trigger(unsigned long priv)
{
	struct sk_buff *skb;
	struct wm_msg *msg;

	skb = wm_msgq_alloc(0);

	if(skb != NULL)
	{
		msg = (struct wm_msg *)skb->cb;
		msg->type = WM_MSG_TIMER;

		wm_msgq_write(skb);
	}
}

/*!-----------------------------------------------------------------------------
 * function: wm_add_timer()
 *
 *      \brief	add a target's timer
 *		\param 	target: target's pointer 
 *				event:	event type	
 *				next_time:	duration : (secs * WLAN_TIME_UNIT)
 *      \return	true or false
 +----------------------------------------------------------------------------*/
int wm_add_timer(unsigned int target, unsigned short event, unsigned int next_time)
{
	struct wlan_timeout *one=NULL, *new=NULL, *prev=NULL;
	u32 now, expire;
	
	if(wm_timer->free_wlan_timeout == NULL)
	{
		lynx_dbg(LYNX_DBG_WM, "wlan timer is not enough!\n");
		return -1;
	}

	new = wm_timer->free_wlan_timeout;
	wm_timer->free_wlan_timeout = wm_timer->free_wlan_timeout->next;

	now = os_current_time();
	expire = now + next_time;
	
	new->event = event;
	new->target = target;
	new->expire = expire;

	if(wm_timer->wait_wlan_timeout)
	{
		/* insert new one by expire */
		for(one=wm_timer->wait_wlan_timeout; one; one = one->next)
		{
			if(one->expire > expire)
			{
				break;
			}
			prev = one;
		}
	}

	if(prev)
	{
		new->next = prev->next;
		prev->next = new;
	}
	else
	{
		new->next = wm_timer->wait_wlan_timeout;
		wm_timer->wait_wlan_timeout = new;
	}

#ifdef CONFIG_USE_SYS_TIMER
	/* expire record unit = 10 msec, os_add_timer() interval unit = 1 msec */	
	os_add_timer(&wm_timer->timer, (wm_timer->wait_wlan_timeout->expire - now)*10);
#endif

	return 0;
}

/*!-----------------------------------------------------------------------------
 * function: wm_del_timer()
 *
 *      \brief	delete target's timer
 *		\param 	target: target's pointer 
 *				event:	event type		
 *      \return	true or false
 +----------------------------------------------------------------------------*/
int wm_del_timer(unsigned int target, unsigned short event)
{
	struct wlan_timeout *one, *prev;

	if(wm_timer->wait_wlan_timeout == 0)
	{
		lynx_dbg(LYNX_DBG_WM, "No timer has registered!\n");
		return -1;
	}
	
	prev = NULL;
	one = wm_timer->wait_wlan_timeout;
	while(one)
	{
		if((one->target == target) && (!event || (one->event == event)))
		{
			if(prev)
				prev->next = one->next;
			else
			{
#ifdef CONFIG_USE_SYS_TIMER
				os_del_timer(&wm_timer->timer);
#endif
				wm_timer->wait_wlan_timeout = one->next;
#ifdef CONFIG_USE_SYS_TIMER
				if(wm_timer->wait_wlan_timeout)
					os_add_timer(&wm_timer->timer, wm_timer->wait_wlan_timeout->expire);
#endif
			}

			one->next = wm_timer->free_wlan_timeout;
			wm_timer->free_wlan_timeout = one;	
			
			/* found only one at once */
			if(event)
				break;
			else
			{
				/* compare every one if no event specified */ 
				if(prev)
					one = prev->next;
				else
					one = wm_timer->wait_wlan_timeout;
				continue; 
			}
		}
		prev = one;
		one = one->next;
	}

	return 0;
}

void wm_process_timer(void)
{
	struct wlan_timeout *one;
	u32 target, now;
	u8 event=255;
	now = os_current_time();

	/* check timeout event */
	while(wm_timer->wait_wlan_timeout)
	{
		one = wm_timer->wait_wlan_timeout;
		if(one->expire > now)
			break;
					
		wm_timer->wait_wlan_timeout = one->next;	
		event = one->event;
		target = one->target;
		one->next = wm_timer->free_wlan_timeout;
		wm_timer->free_wlan_timeout = one;	
		wm_handle_timeout((struct sta_context *)target, event);
	}

#ifdef CONFIG_USE_SYS_TIMER
	if(wm_timer->wait_wlan_timeout)
		os_add_timer(&wm_timer->timer, (wm_timer->wait_wlan_timeout->expire - now)*10);
#endif
}

/*!-----------------------------------------------------------------------------
 * function: wm_reset_timer()
 *
 *      \brief	reset all timer
 *		\param 	void 
 *      \return	void
 +----------------------------------------------------------------------------*/
void wm_reset_timer(void)
{
	struct wlan_timeout *one;
	int i;

	wm_timer->wait_wlan_timeout = 0;
	memset(wm_timer->all_wlan_timeout, 0, sizeof(struct wlan_timeout)*WLAN_TIMEOUT_MAX_NUM);

	one = wm_timer->all_wlan_timeout; 
	for(i = 0; i<(WLAN_TIMEOUT_MAX_NUM-1); i++)
	{
		one[i].next = &one[i+1];
	}
	one[i].next = 0;
	wm_timer->free_wlan_timeout = wm_timer->all_wlan_timeout;

#ifdef CONFIG_USE_SYS_TIMER
	os_del_timer(&wm_timer->timer);
#endif
}

/*!-----------------------------------------------------------------------------
 * function: wm_init_timer()
 *
 *      \brief	initialize all timer
 *		\param 	void 
 *      \return	void
 +----------------------------------------------------------------------------*/
int wm_init_timer(void)
{
	int ret = 0;

#if defined(DYNAMIC_ALLOC_MSGQ)
	wm_timer->all_wlan_timeout = (struct wlan_timeout *)os_api_alloc(sizeof(struct wlan_timeout)*WLAN_TIMEOUT_MAX_NUM, GFP_KERNEL);
#endif

#if defined(DYNAMIC_ALLOC_MSGQ)
	if(wm_timer->all_wlan_timeout == NULL)
		ret = -1;
	else
#endif
	{
#ifdef CONFIG_USE_SYS_TIMER
		os_init_timer(&wm_timer->timer, wm_timer_trigger, 0);
#endif
		wm_reset_timer();
	}

	return ret;
}

/*!-----------------------------------------------------------------------------
 * function: wm_deinit_timer()
 *
 *      \brief	deinit all timer
 *		\param 	void 
 *      \return	void
 +----------------------------------------------------------------------------*/
void wm_deinit_timer(void)
{
	if(wm_timer->all_wlan_timeout)
	{
#if defined(DYNAMIC_ALLOC_MSGQ)
		os_api_free(wm_timer->all_wlan_timeout);
		wm_timer->all_wlan_timeout = NULL;
#endif
#ifdef CONFIG_USE_SYS_TIMER
		os_deinit_timer(&wm_timer->timer);
#endif
	}
}

/*!-----------------------------------------------------------------------------
 * function: wm_shutdown()
 *
 *      \brief	free all resource of AP
 *		\param 
 *      \return
 +----------------------------------------------------------------------------*/
void wm_shutdown(int is_deinit)
{
	struct wm_vif *bss;
	u32 i;

	/* flush msgq */
	wm_msgq_deinit(is_deinit);

	/* FIXME: if implement upnp + wps, need inform WPS FSM. refer the Cheetah/ECoS */

	for(i=0; i<VIF_MAX_NUM; i++)
	{
		bss = &my_wlan_dev->bss[i];
		
		if((bss->flag & VIF_FLG_ENABLE) == 0)
			continue;

		ap_release_all_sta(bss, STA_FREE_MEM);

#ifdef CONFIG_WPA
		if(bss->wpa_ie)
			os_api_free(bss->wpa_ie);
		if(bss->rsn_ie)
			os_api_free(bss->rsn_ie);
		bss->wpa_ie = NULL;
		bss->rsn_ie = NULL;
		bss->wpa_ie_len=0;
		bss->rsn_ie_len=0;
#endif

		/* TODO: del KEY_TYPE_GLOBAL_KEY */

#ifdef CONFIG_WPS
		if((bss->auth_capability & AUTH_CAP_WPS) && bss->wps){
			bss_deinit_wps(bss);
		}
#endif
		bss->flag = 0;
		bss->auth_capability = 0;
	}
	
	reset_nbss_list();
	flush_scan_cb_list();

	if(is_deinit)
		wm_deinit_timer();

	/* TODO: info lower level stop */

	os_thread_sleep(100); /* wait lower layer finish */
}

int wm_setup_security(struct wm_vif *vif, int cipher, int sec_type, char *key, int key_idx, int is_default_key, int clean_old)
{
	int key_len = 0;
	int ret=0;
#ifdef CONFIG_WPA
	int size=0;
	unsigned char tmp[256];
#endif

	if(clean_old)
	{
		vif->auth_capability = 0;
		memset(vif->wep, 0, (WLAN_WEP_NKID * WLAN_WEP_MAX_KEYLEN));

#ifdef CONFIG_WPA
		if(vif->rsn_ie)
		{
			os_api_free(vif->rsn_ie);
			vif->rsn_ie = NULL;
		}

		if(vif->wpa_ie)
		{
			os_api_free(vif->wpa_ie);
			vif->wpa_ie = NULL;
			vif->wpa_ie_len=0;
		}

		if(vif->wpa_group)
		{
			os_api_free(vif->wpa_group);
			vif->wpa_group = NULL;
			vif->rsn_ie_len=0;
		}
#endif	// CONFIG_WPA
	}

	/*	cipher : AUTH_CAP_CIPHER_TKIP
		sec_type : AUTH_CAP_WPA			*/
	vif->auth_capability = cipher | sec_type;

	vif->auth_capability |= AUTH_CAP_OPEN;
	if(!(sec_type & AUTH_CAP_WEP))
		vif->auth_capability |= AUTH_CAP_AUTO_AUTH_ALG;

	if(key)
		key_len = strlen(key);

	if(is_default_key)
		vif->key_idx = key_idx;

	if(cipher & AUTH_CAP_CIPHER_WEP40)
	{
		if(key_len == WLAN_WEP_40BIT_KEYLEN)
			memcpy(&vif->wep[key_idx][0], key, WLAN_WEP_40BIT_KEYLEN); 
		else if(key_len == (WLAN_WEP_40BIT_KEYLEN * 2))
			hexstr2bin(key, (u8 *)&vif->wep[key_idx][0], WLAN_WEP_40BIT_KEYLEN);
		else
		{
			ret = -1;
			lynx_dbg(LYNX_DBG_WM, "%s(): wrong wep40 key len(%d)\n", __FUNCTION__, key_len);
		}
	}
	else if(cipher & AUTH_CAP_CIPHER_WEP104)
	{
		if(key_len == WLAN_WEP_104BIT_KEYLEN)
			memcpy(&vif->wep[key_idx][0], key, WLAN_WEP_104BIT_KEYLEN); 
		else if(key_len == (WLAN_WEP_104BIT_KEYLEN * 2))
			hexstr2bin(key, (u8 *)&vif->wep[key_idx][0], WLAN_WEP_104BIT_KEYLEN);
		else
		{
			ret = -1;
			lynx_dbg(LYNX_DBG_WM, "%s(): wrong wep104 key len(%d)\n", __FUNCTION__, key_len);
		}
	}
#ifdef CONFIG_WPA
	else if(cipher & (AUTH_CAP_CIPHER_TKIP | AUTH_CAP_CIPHER_CCMP))
	{
		if(key_len == PMK_LEN*2)
			hexstr2bin(key, vif->psk, PMK_LEN);
		else	/* 8 ~ 63 is ASCII */
			pbkdf2(key, vif->ssid, vif->ssid_len, 4096, vif->psk, PMK_LEN);
		
		vif->auth_capability |= AUTH_CAP_KEY_MGT_PSK;	/* FIXME: only support PSK now */

		if(sec_type & AUTH_CAP_WPA)
		{
			if((size = ap_encode_wpa_ie(vif, tmp, 0)) > 0)
			{
				if(vif->wpa_ie)
					os_api_free(vif->wpa_ie);
				if((vif->wpa_ie = os_api_alloc(size, GFP_KERNEL)) != NULL)
				{
					memcpy(vif->wpa_ie, tmp, size);
					vif->wpa_ie_len = size;
				}
			}
		}
		if(sec_type & AUTH_CAP_WPA2)
		{
			if((size = ap_encode_wpa_ie(vif, tmp, 1)) > 0)
			{
				if(vif->rsn_ie)
					os_api_free(vif->rsn_ie);
				if((vif->rsn_ie = os_api_alloc(size, GFP_KERNEL)) != NULL)
				{
					memcpy(vif->rsn_ie, tmp, size);
					vif->rsn_ie_len = size;
				}
			}
		}

		if(vif->role == VIF_STA_MODE)
		{
			if(vif->wpa_group == NULL)
				vif->wpa_group = (struct wpa_group_key *)os_api_alloc(sizeof(struct wpa_group_key), GFP_KERNEL);
			if(vif->wpa_group == NULL)
				ret = -1;
		}
		else //if((vif->role == VIF_AP_ROLE))
		{
			wpa_group_init(vif);
		}
	}
#endif	// CONFIG_WPA
	else	/* the security setting is not correct, but store the key for connect progress */
	{
#ifdef CONFIG_WPA
		/* FIXME: only work for WPA/WPA2 */
		if(key_len == PMK_LEN*2)
			hexstr2bin(key, vif->psk, PMK_LEN);
		else	/* 8 ~ 63 is ASCII */
			pbkdf2(key, vif->ssid, vif->ssid_len, 4096, vif->psk, PMK_LEN);

		vif->auth_capability |= (AUTH_CAP_WPA2 | AUTH_CAP_WPA | AUTH_CAP_CIPHER_TKIP | AUTH_CAP_CIPHER_CCMP | AUTH_CAP_KEY_MGT_PSK);
#else
		ret = -1;
#endif
	}

	return ret;
}

/*!-----------------------------------------------------------------------------
 * function: wm_reload_config()
 *
 *      \brief	reconfigure all AP's parameter
 *		\param 
 *      \return
 +----------------------------------------------------------------------------*/
void wm_reload_config(void)
{
	struct wm_vif *vif;
	u16 beacon_interval=100;
	u32 vif_idx;
#ifdef CONFIG_WPS
	char wps_key[128]={0};
	int wps_state = 0;
#endif
	char ap_is_exist = 0;

	void *driver_priv = my_wlan_dev->driver_priv;

	memset(my_wlan_dev, 0, sizeof(struct wlan_dev));
	my_wlan_dev->driver_priv = driver_priv;

	my_wlan_dev->phy_cap = AP_CAP_11B | AP_CAP_11G; /* TODO: AP_CAP_11N */

	my_wlan_dev->capability = WLAN_CAPABILITY_ESS;
	my_wlan_dev->capability |= WLAN_CAPABILITY_SHORT_SLOT_TIME;
	my_wlan_dev->capability |= WLAN_CAPABILITY_SHORT_PREAMBLE;

	my_wlan_dev->power_saving_mode = WLAN_PS_LEGACY;
	if(my_wlan_dev->power_saving_mode >= WLAN_PS_UAPSD)
		my_wlan_dev->capability |= WLAN_CAPABILITY_APSD;

	my_wlan_dev->sta_max_num = LYNX_STA_MAX_NUM;

	strcpy(my_wlan_dev->country, "US");
	my_wlan_dev->country[2] = ' ';
	my_wlan_dev->country[3] = '\0';

	if(memcmp(my_wlan_dev->country, "JP", 2) == 0)
		my_wlan_dev->channel_num = 13;
	else
		my_wlan_dev->channel_num = 11;
	
	/* setup work channel */
	my_wlan_dev->channel = 1;
	my_wlan_dev->secondary_channel_def = BW40MHZ_SCN;
	my_wlan_dev->secondary_channel = BW40MHZ_SCN;  /* for 20/40 coex */

	my_wlan_dev->ptk_rekey_time = 0; /* disable PTK rekey */

	/* FIXME: wm_manager may not need config AMPDU */
	my_wlan_dev->disable_ampdu = 0;
	my_wlan_dev->ampdu_tx_mask = 0;
	my_wlan_dev->ba_win_size = 32;
	my_wlan_dev->ampdu_tx_policy = 0;

	my_wlan_dev->listen_interval_max = PSQ_AGING_TIME/beacon_interval;

	my_wlan_dev->cts_protection_type = CTS_PROTECTION_AUTOMATIC;

	if(my_wlan_dev->phy_cap & AP_CAP_11N)
	{
		my_wlan_dev->ht_capability = WLAN_HTCAP_DSSSCCK40;
		my_wlan_dev->ht_capability |= WLAN_HTCAP_GREENFIELD;
		my_wlan_dev->ht_capability |= (WLAN_HTCAP_SHORTGI20|WLAN_HTCAP_SHORTGI40);

		my_wlan_dev->ampdu_params = 0x03; /* Maxiumum Rx AMPDU size = 64K */
	}

	//for(bss_idx=0; bss_idx < MBSS_MAX_NUM; bss_idx++)
	for(vif_idx=0; vif_idx < 1; vif_idx++)
	{
		vif = &my_wlan_dev->bss[vif_idx];
		vif->role = VIF_STA_MODE;

		if((my_wlan_dev->phy_cap & (AP_CAP_11N|AP_CAP_11G)) == 0)
			vif->slottime = SLOTTIME_20US;
		else
			vif->slottime = SLOTTIME_9US;

		/* FIXME: only handle sta & ap role */
		if((vif->role == WIF_STA_ROLE))
		{
#if 0
			/* add sta at connect cmd */
			struct sta_context *sta;
			sta = ap_add_sta(vif, NULL);
#endif
		}
		else
#ifdef CONFIG_LYNX_IBSS
		if(vif->role == WIF_IBSS_ROLE)
		{
			vif->ibss_state = IBSS_STATE_INIT;
			my_wlan_dev->ibss_exist = 1;
			wm_add_timer((u32)vif, IBSS_MAINTAIN_TIMEOUT, IBSS_MAINTAIN_TIMER);
			vif->dtim_period = 1; // for ATIM function
			vif->slottime = SLOTTIME_20US;
		}
		else
#endif
		{	/* ap role */
			ap_is_exist = 1;
			my_wlan_dev->ap_exist = ap_is_exist;
		}

		vif->bss_desc = vif_idx;
	
		vif->ssid_len = strlen(vif->ssid);
		vif->flag |= VIF_FLG_ENABLE;

		/* set tsf related parameters */
		vif->dtim_period = 3;
		vif->beacon_interval = beacon_interval;

		if((my_wlan_dev->phy_cap & AP_CAP_11N))
		{
			vif->flag |= VIF_FLG_WMM;
			//my_wlan_dev->capability |= WLAN_CAPABILITY_QOS; /* Intel will check this bit */ 
		}

		vif->drv_vif = lynx_interface_add((struct lynx *) my_wlan_dev->driver_priv, IFNAME, 0, NL80211_IFTYPE_STATION, vif_idx, vif->role);

		if(vif->role == VIF_STA_MODE)
		{
#if 0
			/* TODO: trigger scan here */
			wm_add_timer((unsigned int)vif, STA_CONNECT_TIMEOUT, (1*WLAN_TIME_UNIT));
			memcpy(vif->ssid, "tp-lynx", 7);
			vif->ssid[7] = '0';
			vif->ssid_len = 7;
#endif
			/* FIXME: read security setting & need the ssid info to generate psk */
		}
		else if(vif->role == VIF_AP_MODE)
		{
			/* TODO: trigger start ap */
		}
	}
}

void wm_handle_interface(struct wm_vif *vif, int is_attached, unsigned char *addr)
{
	if(is_attached)
	{
		if(vif->role == VIF_STA_MODE)
		{
			/* FIXME: for concerto debug */
			wm_del_timer((unsigned int)vif, STA_CONNECT_TIMEOUT);

			if(addr && is_valid_ether_addr(addr))
				memcpy(vif->myaddr, addr, 6);

#if 0
			/* FIXME: need check the setting first, enable this to do connect after power on */
			wm_add_timer((unsigned int)vif, STA_CONNECT_TIMEOUT, (1*WLAN_TIME_UNIT));
#endif
		}
	}
	else
	{
		/* TODO: error handle */
	}
}

int wm_ap_command(struct wm_msg *msg, unsigned char *data)
{
	int ret = 0;

	switch(msg->u.cmd.cmd)
	{
		case AP_CMD_SHUTDOWN:
			lynx_dbg(LYNX_DBG_WM, "AP_CMD_SHUTDOWN !!!!!!!!!!!!!!!\n");
			ret = -1;
			break;
		case AP_CMD_RELOAD: 
			wm_reset_timer();
			wm_shutdown(0);
			wm_reload_config();	
			break;
		case AP_CMD_SCAN_DONE:
			scan_cb_handler();
			break;
		case AP_CMD_ADD_STA:
			lynx_dbg(LYNX_DBG_WM, "AP_CMD_ADD_STA !!!!!!!!!!!!!!!\n");
			sta_connected(&my_wlan_dev->bss[msg->u.cmd.vif_idx], data, 
						msg->u.cmd.arg[0], (data+6));
			break;
		case AP_CMD_DEL_STA:
			lynx_dbg(LYNX_DBG_WM, "AP_CMD_DEL_STA !!!!!!!!!!!!!!!\n");
			/* FIXME: check the reconnect is worked */
			sta_disconnect(&my_wlan_dev->bss[msg->u.cmd.vif_idx], data, 1, WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY);
			break;
		case AP_CMD_INFO_INTERFACE_STATUS:
			wm_handle_interface(&my_wlan_dev->bss[msg->u.cmd.vif_idx], msg->u.cmd.arg[0], (unsigned char *) &msg->u.cmd.arg[1]);
			break;
		default:
			lynx_dbg(LYNX_DBG_WM, "%s(): wrong cmd\n", __FUNCTION__);
			break;
	}

	return ret;
}

/*!-----------------------------------------------------------------------------
 * function: wm_manager()
 *
 *      \brief	AP's main loop
 *		\param 	void 
 *      \return	void
 +----------------------------------------------------------------------------*/
int wm_manager(void *priv)
{
	struct wm_msg *msg;
	struct sk_buff *skb;
	int ret=0;

	wm_msgq_init();
	
	if(priv == NULL)
		goto fail;
	my_wlan_dev->driver_priv = priv;

	if((wm_init_timer()) != 0)
		goto fail;

	lynx_dbg(LYNX_DBG_WM, "%s(): init success & start\n", __FUNCTION__);

	while(1)
	{
		if(os_check_thread_stop())
		{
			/* TODO: do shutdown */
			break;
		}
		
		ret = os_wait_for_completion_interruptible(&(msgq_data.msgq_complete), 1);

		/* FIXME: can't trigger timer in UCOS, patch it */
		wm_process_timer();

		if(ret < 0)
			break;
		else if(ret == 0)
			continue;
		
		if((skb = wm_msgq_read()) == NULL)
			continue;

		msg = (struct wm_msg *)skb->cb;
		
		switch(msg->type)
		{
			case WM_MSG_CMD:
			{
				if(wm_ap_command(msg, skb->data) != 0)
					goto fail;
				break;
			}
			case WM_MSG_WLAN_FRAME:
			{
				wm_frame_handler(skb, msg->u.wf.vif_idx, msg->u.wf.sta_idx, msg->u.wf.type, msg->u.wf.rssi);
				break;
			}	
			case WM_MSG_TIMER:
			{
				wm_process_timer();
				break;
			}
			case WM_MSG_WPS:
			{
#ifdef CONFIG_WPS
				extern void wps_msg_handler(struct shm_unit *shm);
				struct shm_unit *m = &msg->u.wps;
				wps_msg_handler(m);
#endif // CONFIG_WPS
				break;
			}
			case WM_MSG_CTRL:
			{
				wm_ctrl_handler((struct wm_msg *)skb->cb, skb->data);
				break;
			}
			default:
				lynx_dbg(LYNX_DBG_WM, "error: %s(%d), msg->type=%x\n", __func__, __LINE__, msg->type);
				break;	
		}
        
		kfree_skb(skb);
	}

fail:

	wm_shutdown(1);
	lynx_dbg(LYNX_DBG_WM, "%s(): exit\n", __FUNCTION__);
	os_thread_exit();

	return 0;
}

struct sk_buff *wm_msgq_alloc(int size)
{
	struct sk_buff *skb;
	skb = dev_alloc_skb(size);

	return skb;
}

int wm_msgq_write(struct sk_buff *skb)
{
	int ret=0;

	if(msgq_data.msgq_is_inited)
	{
		/* skb queue list handler do lock */
		__skb_queue_tail(&(msgq_data.msgq), skb);
		ret = os_complete(&msgq_data.msgq_complete);
	}
	else
	{
		/* FIXME: may free skb in the other place */
		kfree_skb(skb);
		ret = -1;
	}

	return ret;
}

struct sk_buff *wm_msgq_read(void)
{
	struct sk_buff *skb = NULL;

	/* skb queue list handler do lock */
	if(msgq_data.msgq_is_inited)
		skb = skb_dequeue(&(msgq_data.msgq));

	return skb;
}

int wm_msgq_init(void)
{
	memset(&msgq_data, 0, sizeof(struct wlan_msgq));
	os_init_completion(&msgq_data.msgq_complete);
	/* FIXME: Linux should use skb_queue_head_init() not __skb_queue_head_init() */
	skb_queue_head_init(&(msgq_data.msgq));

	msgq_data.msgq_is_inited = 1;

	return 0;
}

int wm_msgq_deinit(int is_deinit)
{
	/* FIXME: must sure that wm_msgq_deinit/wm_msgq_init only one is process */
	if(is_deinit)
		msgq_data.msgq_is_inited = 0;

	/* skb_queue_purge will do spin lock irqsave */
	skb_queue_purge(&(msgq_data.msgq));

	return 0;
}

