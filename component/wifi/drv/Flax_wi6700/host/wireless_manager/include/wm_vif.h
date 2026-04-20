/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file wlan_def.h
*   \brief  WLAN protocol definitions.
*   \author Montage
*/

#ifndef _WM_BSS_H_
#define _WM_BSS_H_

#ifdef CONFIG_WPA
#include <wpa.h>
#endif
#ifdef CONFIG_WAPI
#include <wapi.h>
#endif
#ifdef CONFIG_WPS
#include <eap.h>
#include <wps.h>
#endif

#define VIF_FLG_ENABLE					BIT(0)
#define VIF_FLG_HIDDEN_SSID				BIT(1)
#define VIF_FLG_WMM						BIT(2)
#define VIF_FLG_NO_PROBE_RESP			BIT(3)
#define VIF_FLG_TKIP_COUNTERMEASURE		BIT(4)
#define VIF_FLG_DISABLE_BEACON			BIT(5)

#define MAX_CHANNEL_NUM				14
#define MAX_NEIGHBOR_BSS_NUM			64

#define WLAN_CHAN_DISABLED		0x1

struct wlan_channel_info {
	unsigned char max_txpower; /* maximum transmit power in dBm */
	unsigned char flag;
	unsigned int cca_busy;	
	unsigned char rssi;
	unsigned char bss_num;
	
	/* for auto channel selection */
	unsigned char noise_floor;
	unsigned int active_time;
	unsigned int busy_time;
};

struct wm_vif {
	unsigned char	bss_desc;			/* indicate Driver's VIF number */
	unsigned char	bssid[WLAN_ADDR_LEN];
	unsigned char	myaddr[WLAN_ADDR_LEN];
	unsigned char	ssid[33];
	unsigned char	ssid_len;
	unsigned int	flag;
	unsigned int	auth_capability;
	unsigned short	sta_num; 			/* current number of sta */
	unsigned short	role;				/* ex: WIF_AP_ROLE ... */
	struct sta_context *sta_list; /* STA info list head */

	unsigned char	peer_phy_cap;	/* for VIF_STA_MODE */
	
	/* for TSF sync */
	unsigned short	beacon_interval;
	unsigned char	dtim_period;

	/* WMM */
	unsigned short	slottime; 			/* 0: auto, 1: short(9us), 2: long(20us) */ 
	/* HT Capability */
	unsigned short	ht_capability;
	unsigned char	ampdu_params;
	
#ifdef CONFIG_WPA
	unsigned char	psk[PMK_LEN];
	unsigned char	*wpa_ie;
	unsigned char	wpa_ie_len;
	unsigned char	*rsn_ie;
	unsigned char	rsn_ie_len;
	unsigned char 	wpa_counter[WPA_NONCE_LEN];
#endif	// CONFIG_WPA
	unsigned char	key_idx;
	unsigned char	wep[WLAN_WEP_NKID][WLAN_WEP_MAX_KEYLEN];
#ifdef CONFIG_WPS
	struct 	wps_data *wps;	//one per bss
	unsigned char 	*wps_ie;
	unsigned char 	wps_ie_len;
	unsigned char	*wps_ie2;
	unsigned char	wps_ie2_len;
#endif

#if defined(CONFIG_WPA) || defined(CONFIG_WAPI)
	unsigned int	gtk_rekey_time;
#endif
	
#ifdef CONFIG_WPA
	struct wpa_group_key *wpa_group;
#endif
#ifdef CONFIG_WAPI
	struct wapi_group_key *wapi_group;
#endif


	unsigned int last_mic_failure;
	unsigned short mic_failure_count;

#ifdef CONFIG_LYNX_IBSS
	unsigned char ibss_bssid[WLAN_ADDR_LEN];
#endif

	void *drv_vif;
};


#define NBSS_ENABLE				BIT(0)
#define NBSS_NON_ERP			BIT(1)
#define NBSS_NON_HT				BIT(2)
#define NBSS_NO_40_PERMITTED	BIT(3)
#define NBSS_SUPPORT_WMM		BIT(4)
#define NBSS_P2P_INFO			BIT(5)
#define NBSS_P2P_NONGO_RESP		BIT(6)

#define NBSS_NORMAL_EXPIRE_TIME	5*2
#define NBSS_P2P_EXPIRE_TIME	180

#define NBSS_WPS_ST_SELECTED_REGISTRAR	BIT(0)
#define NBSS_WPS_ST_PUSHBUTTON			BIT(1)

struct neighbor_bss {
	unsigned int flag;
	unsigned char channel;
	unsigned char secondary_ch;
	unsigned char addr[WLAN_ADDR_LEN];
	signed char rssi;
	unsigned char band;
	unsigned char dtim_period;
	unsigned char qinfo;
	unsigned short sec;
	unsigned short capability;
	unsigned short ht_capability;
	unsigned short beacon_interval;
	unsigned int expire;
	unsigned char ssid[33];

#ifdef CONFIG_LYNX_IBSS
	/* for IBSS */
	unsigned char	ampdu_params;
	unsigned char	bssid[WLAN_ADDR_LEN];
	unsigned short	atim_window;
	unsigned int	supp_rates;
	unsigned int	timestamp[2];
#endif // CONFIG_LYNX_IBSS

#ifdef CONFIG_WPS
	unsigned int wps_info;
#endif

	struct wpa_ie_data *wpa_info;
	struct wpa_ie_data *rsn_info;
	struct wme_ie_data *wme_info;
};

enum {
	NBSS_BAND_B = 1,
	NBSS_BAND_G = 2,
	NBSS_BAND_N = 4,
};

enum {
	NBSS_SEC_NON = 0,
	NBSS_SEC_WEP = 1,
	NBSS_SEC_WPA = 2,
	NBSS_SEC_WPA2 = 4,
	NBSS_SEC_WPS = 8,
	NBSS_SEC_WAPI = 16,
};
#define NBSS_SEC_BASIC_MASK	0x7	/* WEP, WPA, WPA2 */

struct wlan_dev {
	unsigned short	phy_cap;
	unsigned short	capability;
	unsigned short	sta_max_num;
	unsigned short	listen_interval_max; /* listen interval maximum value */
	unsigned char	state;

	unsigned char	channel;
	unsigned char	secondary_channel;	/* enum BW_MODE, exclude BW40MHZ_AUTO */
	unsigned char	secondary_channel_def;	/* enum BW_MODE */

	unsigned char	power_saving_mode; /* 0 => disable power saving, 1 => legacy power saving, 2 => UPASD */
	/* WMM */
	char parameter_set_count;
	struct wlan_wme_ac_params	wme_ac_params[4];
	/* HT capabilities */
	unsigned short	ht_capability;
	unsigned char	disable_ampdu;
	unsigned char	ampdu_params;
	unsigned char	ampdu_tx_mask;
	unsigned char	ba_win_size;
	unsigned int  ampdu_tx_policy;
	/* HT information */
	unsigned short	ht_op_mode;	// FIXME: obss scan
	
	/* security */
	unsigned int	ptk_rekey_time;
	
	struct wm_vif bss[VIF_MAX_NUM];
	
	struct wlan_channel_info channels[MAX_CHANNEL_NUM];
	unsigned char	channel_num;
	unsigned char	country[4];			/* country string */

	unsigned char	wds_mode;			/* WDS operation indictor */

	enum {
		CTS_PROTECTION_AUTOMATIC = 0,
		CTS_PROTECTION_FORCE_ENABLED = 1,
		CTS_PROTECTION_FORCE_DISABLED = 2,
		CTS_PROTECTION_AUTOMATIC_NO_OLBC = 3,
	} cts_protection_type;

	char nbss_num;
	struct neighbor_bss site_survey[MAX_NEIGHBOR_BSS_NUM];

	struct scan_cb *scan_cb_list;

#ifdef CONFIG_LYNX_IBSS
	unsigned char	ibss_exist;
#endif
	unsigned char	ap_exist;

	/* driver layer lynx pointer */
	void *driver_priv;	// struct lynx *lnx
};

/*  AP timeout */
enum {
	STA_ALL_TIMEOUT = 0,
	STA_WAPI_TX_TIMEOUT,
	STA_WAPI_PTK_REKEY_TIMEOUT,
	STA_WPA_TX_TIMEOUT,
	STA_WPA_PTK_REKEY_TIMEOUT,
	STA_CONNECT_TIMEOUT,

	BSS_WPA_GROUP_REKEY_TIMEOUT,
	BSS_WAPI_GROUP_REKEY_TIMEOUT,
	BSS_4WAY_HANDSHAKE_TIMEOUT,
	BSS_TKIP_COUNTERMEASURE,
	BSS_GET_LINK_STATUS,

	DEV_OBSS_SCAN_TIMEOUT,

	WPS_PBC_TIMEOUT,
	WPS_PIN_TIMEOUT,
	WPS_M2D_TIMEOUT,
	WPS_SET_SELECTED_REGISTRAR_TIMEOUT,
	WPS_AP_SETUP_LOCKED_TIMEOUT,

	EAPOL_REXMIT_TIMEOUT,
#ifdef CONFIG_LYNX_IBSS
	IBSS_MAINTAIN_TIMEOUT,
#endif
	SYSTEM_MAINTAIN_DHCP,
};

extern struct wlan_dev *my_wlan_dev;

#endif /* _WM_BSS_H_ */
