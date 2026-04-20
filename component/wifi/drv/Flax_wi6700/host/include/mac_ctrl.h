/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Acrospeed Inc. All right reserved.                                           |
|                                                                              |
+=============================================================================*/
/*! 
*   \file 
*   \brief
*   \author
*/

#ifndef __MAC_CTRL_H__
#define __MAC_CTRL_H__

#define PMK_LEN 32
#define WPA_NONCE_LEN   32
#define WLAN_WEP_NKID   4
#define WLAN_WEP_MAX_KEYLEN 16
#define PMKID_LEN           16

enum {
	WLAN_PS_NON = 0,
	WLAN_PS_LEGACY,
	WLAN_PS_UAPSD,
};

enum {
    VIF_STA_MODE = 1,
    VIF_AP_MODE = 2,
    VIF_IBSS_MODE = 3,
    VIF_WDS_MODE = 4,
    VIF_P2P_CLIENT_MODE = 9,
    VIF_P2P_GO_MODE = 0xa,
};

#define BSS_FLG_ENABLE                  BIT(0)
#define BSS_FLG_HIDDEN_SSID             BIT(1)
#define BSS_FLG_WMM                     BIT(2)
#define BSS_FLG_NO_PROBE_RESP           BIT(3)
#define BSS_FLG_TKIP_COUNTERMEASURE     BIT(4)
#define BSS_FLG_DISABLE_BEACON          BIT(5)
#define BSS_FLG_MAT_CLIENT_MODE			BIT(6)
#define BSS_FLG_MATCH_BSSID				BIT(7)
#define BSS_FLG_RUN						BIT(8)
#define BSS_FLG_STATIC_IP				BIT(9)
#define BSS_FLG_DISABLE_DHCP			BIT(10)
#define BSS_FLG_LINKUP					BIT(11)
#define BSS_FLG_RECONNECT				BIT(12)
#define BSS_FLG_FAST_CONN				BIT(13)
#define BSS_FLG_P2P_DEVICE_MODE			BIT(14)
#define BSS_FLG_SCAN					BIT(15)
#define BSS_FLG_SPECIFY_SSID			BIT(16)
#define BSS_FLG_BCAST_SSID				BIT(17)
#define BSS_FLG_P2P_CLIENT_MODE			BIT(18)
#define BSS_FLG_P2P_GO_MODE				BIT(19)
#define BSS_FLG_BEACON_REPORT			BIT(20)

#define WMBSS_START_SECTOR  tsf_idx
#define WMBSS_END_SECTOR    sta_list

enum {
	AUTH_CIPHER_NONE = 0,
	AUTH_CIPHER_WEP40,
	AUTH_CIPHER_WEP104,
	AUTH_CIPHER_TKIP,
	AUTH_CIPHER_CCMP,
	AUTH_CIPHER_SMS4,
	AUTH_CIPHER_AES_128_CMAC,
};

#define AUTH_CAP_OPEN                   BIT(0)
#define AUTH_CAP_AUTO_AUTH_ALG          BIT(1)
#define AUTH_CAP_WEP                    BIT(2)
#define AUTH_CAP_WPA                    BIT(3)
#define AUTH_CAP_WPA2                   BIT(4)
#define AUTH_CAP_WPS                    BIT(5)
#define AUTH_CAP_LEAP                   BIT(6)
#define AUTH_CAP_WAPI                   BIT(7)

#define AUTH_CAP_CIPHER_MASK			0xff00
#define AUTH_CAP_CIPHER_SHIFT			8

#define AUTH_CAP_CIPHER(_a)		((1 << _a) << AUTH_CAP_CIPHER_SHIFT)
#define AUTH_CAP_CIPHER_NONE			AUTH_CAP_CIPHER(AUTH_CIPHER_NONE)
#define AUTH_CAP_CIPHER_WEP40			AUTH_CAP_CIPHER(AUTH_CIPHER_WEP40)
#define AUTH_CAP_CIPHER_WEP104			AUTH_CAP_CIPHER(AUTH_CIPHER_WEP104)
#define AUTH_CAP_CIPHER_TKIP			AUTH_CAP_CIPHER(AUTH_CIPHER_TKIP)
#define AUTH_CAP_CIPHER_CCMP			AUTH_CAP_CIPHER(AUTH_CIPHER_CCMP)
#define AUTH_CAP_CIPHER_SMS4			AUTH_CAP_CIPHER(AUTH_CIPHER_SMS4)
#define AUTH_CAP_CIPHER_AES_128_CMAC	AUTH_CAP_CIPHER(AUTH_CIPHER_AES_128_CMAC)

#define ALL_CIPHER (AUTH_CAP_CIPHER_WEP40|AUTH_CAP_CIPHER_WEP104|AUTH_CAP_CIPHER_TKIP| \
					AUTH_CAP_CIPHER_CCMP|AUTH_CAP_CIPHER_SMS4|AUTH_CAP_CIPHER_AES_128_CMAC)

#define AUTH_CAP_TO_WEP_TYPE(_a)		(_a & AUTH_CAP_CIPHER_WEP40 ? AUTH_CIPHER_WEP40 : AUTH_CIPHER_WEP104)
#define AUTH_CAP_TO_CIPHER_TYPE(_a)		((AUTH_CAP_CIPHER_MASK & _a) >> AUTH_CAP_CIPHER_SHIFT)

enum {
	AUTH_KEY_MGT_PSK = 0,
	AUTH_KEY_MGT_1X,
	AUTH_KEY_MGT_PSK_SHA256,
	AUTH_KEY_MGT_1X_SHA256,
	AUTH_KEY_MGT_FT_PSK,
	AUTH_KEY_MGT_FT_1X,	
};

#define AUTH_CAP_KEY_MGT_MASK			0xffff0000
#define AUTH_CAP_KEY_MGT_SHIFT			16

#define AUTH_CAP_KEY_MGT(_a)	((1 << _a) << AUTH_CAP_KEY_MGT_SHIFT)
#define AUTH_CAP_KEY_MGT_PSK			AUTH_CAP_KEY_MGT(AUTH_KEY_MGT_PSK)
#define AUTH_CAP_KEY_MGT_1X				AUTH_CAP_KEY_MGT(AUTH_KEY_MGT_1X)
#define AUTH_CAP_KEY_MGT_PSK_SHA256 	AUTH_CAP_KEY_MGT(AUTH_KEY_MGT_PSK_SHA256) /* 802.11w */
#define AUTH_CAP_KEY_MGT_1X_SHA256 		AUTH_CAP_KEY_MGT(AUTH_KEY_MGT_1X_SHA256)
#define AUTH_CAP_KEY_MGT_FT_PSK 		AUTH_CAP_KEY_MGT(AUTH_KEY_MGT_FT_PSK)
#define AUTH_CAP_KEY_MGT_FT_1X 			AUTH_CAP_KEY_MGT(AUTH_KEY_MGT_FT_1X)

#define AUTH_CAP_TO_MGT_TYPE(_a)		((AUTH_CAP_KEY_MGT_MASK & _a) >> AUTH_CAP_KEY_MGT_SHIFT)

#define AUTH_CAP_KEY_MGT_ALL_1X			1
#define AUTH_CAP_KEY_MGT_ALL_PSK		2


#define WMBSS_START_SECTOR	tsf_idx
#define WMBSS_END_SECTOR	sta_list

#define MAX_SSID_LEN		33

struct wm_bss {
	/*--- Host Setup Section : Start ---*/
	u8	tsf_idx;
	u8	bss_desc;				/* indicate Driver's VIF number */

	/* for TSF sync */
	u16	beacon_interval;
	u8	dtim_period;

	u8	phy_cap;	/* (lynx dev cap) & (parent AP's cap) */
	u16 tx_rate_code;

	/* WMM */
	unsigned int wme_acm;
	/* FIXME: bss->slottime records the connection's ability & to effect the ldev->slottime */
	u16	slottime; 			/* 0: auto, 1: short(9us), 2: long(20us) */ 
	/* HT Capability */
	u16	ht_capability;
	u8	support_mcs_set[16];	/* FIXME: No Need ? */

	u8  myaddr[WLAN_ADDR_LEN]; 	/* My MAC address */
	u8	bssid[WLAN_ADDR_LEN];	/* BSSID in the bss group (ex: ibss group BSSID) */

	u8	ssid[MAX_SSID_LEN];
	u8	ssid_len;

	u16	role;				/* ex: WIF_AP_ROLE ... */
	u32	flag;
	u32	auth_capability;
	u8	ampdu_params;

	/* IBSS */
	u8 	ibss_state;
	u16 atim_window;

	/*--- Host Setup Section : End ---*/
	/***** Below is that can't been overwrite when host update bss data *****/
	/* sta_list must in the head of which can't been overwrited. */
	struct sta_ctx *sta_list; /* STA info list head */
#if 0
	/* ie pool pointer */
	struct wlan_ie ie_pool;

	u32 wb1_field;
	u32 wb2_field;
	u32 tsc[4];
	u32 rsc[4];

	struct tx_q *bcq;
	struct beacon_desc *beacon_desc;
	struct beacon_desc *new_beacon_desc;

	u8	sta_num; 			/* current number of sta */
	u8	ps_sta_num; 		/* current number of power saving sta */
	u16	last_aid;
	u32 last_mic_failure;
	u16 mic_failure_count;
	u16 tx_seq_num;			/* global sequence number */
	
	u8	dialogtoken; /* global dialogtoken for ADDBA use */

	u8	key_idx;
	u8	wep_type;
	u8	auth_mode;
	u8	psk[PMK_LEN];
	u8 	wpa_counter[WPA_NONCE_LEN];
	u8	wep[WLAN_WEP_NKID][WLAN_WEP_MAX_KEYLEN];

#ifdef CONFIG_WPS
	struct 	wps_data *wps;	//one per bss
	u8 	*wps_ie;
	u8	*wps_ie2;
	u16	wps_ie_len;
	u16	wps_ie2_len;
#endif

	struct pmksa_cache_entry pmksa; /* pmksa cache entry */

	u32	ptk_rekey_time;
	u32	gtk_rekey_time;

	union {
#ifdef CONFIG_WPA
		struct wpa_group_key wpa_group;
		struct wapi_group_key wapi_group;
#endif
	} group_key;

	unsigned char ps_state;
	unsigned char ps_tx_null;
	unsigned char ps_pending;
	unsigned char ps_policy;
	unsigned int busy_timestamp;		/* unit: 10ms, os_current_time() */

	u8 *rsn_ie;
	u8 *wpa_ie;

#ifdef CONFIG_IBSS_ATIM
	struct beacon_desc *atim_bc_desc;
#endif
	/* iot */
	u32 ipaddr;
	u32 netmask;
	u32 gateway;
	u8 op_flag;
	u8 op_state;
	u8 bss_match;
	u8 res;

	unsigned char password[65];
#ifdef CONFIG_HOST_WPS
	u8  beacon_ie_len;
	u8  probe_req_ie_len;
	u8  probe_resp_ie_len;
	u8  assoc_req_ie_len;
	u8  assoc_resp_ie_len;
	u8	padding[2];	// FIXME: for alignment
	/* HOST WPS P2P IE */
	u8  *beacon_ie;     /*26~51 bytes for WPS Host mode */
	u8  *probe_req_ie;  /*135 bytes for WPS Client mode */
	u8  *probe_resp_ie; /*94~119 bytes for WPS Host mode */
	u8  *assoc_req_ie;  /*not use cause host not implement.TODO: check p2p if need */
	u8  *assoc_resp_ie; /*26 bytes for WPS Host mode*/
#endif /*CONFIG_HOST_WPS*/
#endif
} __attribute__ ((packed));		/* FIXME: is "packed" needed for attach cmd to directy copy? */

struct sta_info_t   {
    u16 flags;
    u8  addr[WLAN_ADDR_LEN];
    u16 aid; /* STA's unique AID (1 .. 2007) or 0 if not yet assigned */
    u16 listen_interval; /* or beacon_int for APs */
    
    u32 auth_capability;
    
    u32 challenge; /* Shared Key Authentication Challenge */

    /* WMM */
    u8  qosinfo;    /* power_saving_mode + parameter_set_count */
    u16 slottime;           /* 0: auto, 1: short(9us), 2: long(20us) */ 
    struct wlan_wme_ac_params   wme_ac_params[4];

    u8  min_mpdu_start_spacing;
    u8  max_sp_len;
    u8  apsd_trigger_deliver;
} __packed;

struct tx_ba_session {
    u8      state;
    u8      req_num;    /* record how many times to send ADDBA or fail to send AMPDU */
    u8      dialogtoken;
    u8      dummy;
    u16     start_seq;
    u16     win_start;
} __packed;

struct sta_ctx {
    u16 flags;
    u8  mode;
    u8  vif_idx;
    u8  addr_idx;

    struct sta_info_t   si;

    struct {
        u8  success;
        u8  recovery;
        u8  success_threshold;
        u16 total_cnt;
        u16 rate0_ok_cnt;
    }amrr __packed;

    u8      rate_flags;
    u8      tx_rate_idx;
    u8      max_tx_rate;
    u8      min_tx_rate;    
    u32     supported_rates;

    u32     rx_bytes;
    u32     rx_cnt;

    u32     tx_bytes;
    u16     tx_ok_cnt;
    u16     tx_fail_cnt;
    u16     tx_ok_rate[4];
    
    u8      tx_ampdu_bitmap;
    u8      chip_vendor;

    struct tx_ba_session ba_tx[8];
    u32 ba_rx[8];
    u16     last_seq_num[9]; /* per TID + global */
} __packed;

#define SKB_DATA_OFFSET		56		/* for headroom, must sync with firmware */

#ifdef CONFIG_LYNX_OS_UCOS
#define L1_CACHE_BYTES	0
#endif


#endif
