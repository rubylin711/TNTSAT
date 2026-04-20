/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Acrospeed Inc. All right reserved.                                           |
|                                                                              |
+=============================================================================*/
/*! 
*   \file 
*   \brief
*   \author Acrospeed
*/

#ifndef __WCI_H__
#define __WCI_H__

#define as_u8  u8
#define as_u16 u16
#define as_u32 u32


#ifdef CONFIG_LYNX_OS_LINUX
/* is_valid_ether_addr() */
#include <linux/etherdevice.h>

/* IEEE80211_AC_VO... */
#include "mont_ioctl.h"
#endif

#include "wlan_def.h"
#include "mac_ctrl.h"

#define PMKID_LEN 16    /* hostapd config */
enum WCI_SET_PMKSA_ACT {
    PMKSA_ADD,
    PMKSA_DEL,
    PMKSA_FLUSH,
};

/***************************** Host to Device *****************************/
/*
struct wm_bss_test
{
    u8  bssid[WLAN_ADDR_LEN];
    u16 role;
    u8  test[500];
}__attribute__ ((packed));
*/
/* WCI_H2D_VIF_ATTACH */
struct wci_cmd_dev_start {
	u8 rule_num;
	u8 fofs;
	u8 txvga[14];
	u8 mac[6];
	u8 en40ht;
	u8 en_txoplimit_ap;
}__attribute__ ((packed));
struct wci_cmd_vif_attach {
    u8  bss_idx;
    u8  padding;
    u16 wm_bss_len;
    /* the following is struct wm_bss */
} __attribute__ ((packed));

/* WCI_H2D_VIF_DETACH */
struct wci_cmd_vif_detach {
    u8 bss_idx;
} __attribute__ ((packed));

/* WCI_H2D_VIF_UPDATE */
struct wci_cmd_vif_update {
    u8  bss_idx;
    u8  padding;
    u16 wm_bss_len;
    /* the following is struct wm_bss */
} __attribute__ ((packed));

/* WCI_H2D_SET_IE_POOL */
struct wci_cmd_set_ie_pool {
    u8 overwrite;
    u8 bss_idx;
    u16 ie_data_len;
} __attribute__ ((packed));

/* WCI_H2D_SET_KEY */
struct wci_cmd_set_key {
    u8 bss_idx;
    u8 sta_idx;
    u8 cipher;
    u8 key_type;
    u8 is_txkey;
    u8 key_idx;
    u8 key[0];
} __attribute__ ((packed));

/* WCI_H2D_SET_DEF_KEY */
struct wci_cmd_set_def_key {
    u8 bss_idx;
    u8 sta_idx;
    u8 key_type_map;
    u8 key_idx;
} __attribute__ ((packed));

/* WCI_H2D_DEV_SCAN */
struct wci_cmd_dev_scan {
    u8 bss_idx;
    u8 ssid_len;
    u16 interval;
    u32 ch_map;
	/* u8 ssid[0] */
} __attribute__ ((packed));

/* WCI_H2D_START_AP */
struct wci_cmd_start_ap {
    u8 bss_idx;
	u8 padding[3];
	u32 current_rate;
} __attribute__ ((packed));

/* WCI_H2D_STA_REMOVE */
struct wci_cmd_remove_sta {
	u8 bss_idx;
	u8 sta_idx;
	u8 reason;
	u8 padding;
} __attribute__ ((packed));

/* WCI_H2D_STA_CONNECT */
struct wci_cmd_sta_connect {
    u8 bss_idx;
    u8 ssid_len;
	u8 rsn_ie_len;
	u8 wpa_ie_len;
    u8 channel;
	u8 pending;
    u8 bssid[6];
	u32 supp_rates;
    /* u8 ssid[0] */
	/* u8 rsn_ie[0] */
	/* u8 wpa_ie[0] */
} __attribute__ ((packed));

/* WCI_H2D_STA_DISCONNECT */
struct wci_cmd_sta_disconnect {
    u8 bss_idx;
    u8 sta_idx;
    u16 reason_code;
} __attribute__ ((packed));

/* WCI_H2D_WDS_PEER_ADD */
struct wci_cmd_wds_peer_add {
	u8 bss_idx;
	u8 wds_mode;
	u8 peer_addr[WLAN_ADDR_LEN];
} __attribute__ ((packed));

/* WCI_H2D_SET_MAC */
struct wci_cmd_set_mac {
	u8 bss_idx;
	u8 addr[WLAN_ADDR_LEN];
} __attribute__ ((packed));

/* WCI_H2D_SET_CHANNEL */
struct wci_cmd_set_channel {
	u8 bss_idx;
	u8 channel;
	u8 bandwidth;	/* ex: BW40MHZ_SCN */
	u8 padding;
} __attribute__ ((packed));

/* WCI_H2D_SET_TX_POWER */
struct wci_cmd_set_tx_power {
    u32 tx_power;
} __attribute__ ((packed));

/* WCI_H2D_GET_TX_POWER */
struct wci_cmd_get_tx_power {
    u8  bss_idx;
} __attribute__ ((packed));

/* WCI_H2D_SET_PMKSA */
struct wci_cmd_set_pmksa {
    u8 bss_idx;
    u8 bssid[ETH_ALEN];
    u8 pmkid[PMKID_LEN];
    u8 action;      /* enum WCI_SET_PMKSA_ACT */
} __attribute__ ((packed));

/* WCI_H2D_SET_RTS_THRESHOLD */
struct wci_cmd_set_rts {
	u8 bss_idx;
	u8 padding;	/* avoid memory access fail */
	u16 rts_threshold;
} __attribute__ ((packed));

/* WCI_H2D_SET_2040_COEX */
struct wci_cmd_set_2040_coex {
	u8 bss_idx;
	u8 en_2040_coex;
} __attribute__ ((packed));

/* WCI_H2D_GET_VIF_ADDR */
struct wci_cmd_get_vif_addr {
	u8	bss_idx;
} __attribute__ ((packed));

/* WCI_H2D_GET_VIF_LINK_ST */
struct wci_cmd_get_vif_link_st {
	u8	bss_idx;
} __attribute__ ((packed));

/* WCI_H2D_SET_BEACON */
struct wci_cmd_set_beacon {
    u8 bss_idx;
} __attribute__ ((packed));

/* WCI_H2D_SEND_ADDBA */
struct wci_cmd_send_addba {
	u8 bss_idx;
	u8 sta_idx;
	u8 tid;
	u8 padding;
} __attribute__ ((packed));


#if defined(CONFIG_HOST_WPS) || defined(CONFIG_CUST1_PRIV_BEACON_IE)
/* WCI_H2D_SET_MGMT_IE, */
struct wci_cmd_set_mgmt_ie {
	u8 bss_idx;        /* multi  interface */
	u8 mgmt_frm_type; /* wci_mgmt_frame_type*/
	u8 overwrite;    /* overwrite or append*/
	u8 padding;
	u16 ie_data_len;
} __attribute__ ((packed));
#endif /*CONFIG_HOST_WPS*/

#ifdef CONFIG_HOST_P2P
/* WCI_H2D_SET_MGMT_TXPKT, */
struct wci_cmd_set_mgmt_txpkt {
    u8 bss_idx;       /* multi interface */
    u8 mgmt_frm_type; /* wci_mgmt_frame_type*/
    u8	 padding[2];
    u16 pkt_data_len;
} __attribute__ ((packed));
#endif /*CONFIG_HOST_P2P*/

/* WCI_H2D_SET_MON */
struct wci_cmd_set_mon {
    u32 mon_mask;
} __attribute__ ((packed));

/* WCI_H2D_SET_MIN_RATEIDX */
struct wci_cmd_set_min_rateidx {
    u8 min_rate_idx;
} __attribute__ ((packed));

/* WCI_H2D_SET_PS */
struct wci_cmd_set_ps {
    u8 power_saving_mode;
} __attribute__ ((packed));

/* WCI_H2D_DBG_INFO_CMD */
struct wci_cmd_dbg_info {
	u8 bss_idx;        /* multi  interface */
    u8 is_get;	   	   /* 1: get, 0: set */
    u8 padding[2];		/* avoid memory access fail */
	u16 item_id;
	u16 payload_len;
	/* following is the payload */
} __attribute__ ((packed));

/* WCI_H2D_SET_SPECIAL_PARAM */
struct wci_cmd_set_special_param {
	u8 bss_idx;			/* multi  interface */
    u8 padding[3];		/* avoid memory access fail */
	u16 item_id;
	u16 payload_len;
	/* following is the payload */
} __attribute__ ((packed));

/*-----------------------------------------------------*/

/***************************** Device to HOST *****************************/
/* FIXME: must let bss_idx in the 1st field */

/* WCI_D2H_TGT_RDY */
struct wci_evt_tgt_rdy {
	u8 base_mac[ETH_ALEN];
	u8 no_k_rule_status;
} __attribute__ ((packed));

/* WCI_D2H_RET_VIF_ATTACH */
struct wci_evt_ret_vif_attach {
    u8 bss_idx;
    u8 isok;
    u8 macaddr[ETH_ALEN];   /* bss mac */
} __attribute__ ((packed));

/* WCI_D2H_RET_SCAN_STATE */
struct wci_evt_ret_scan_state {
    u8 bss_idx;
    u8 isok;
} __attribute__ ((packed));

/* WCI_D2H_RET_STA_CONNECTED */
struct wci_evt_ret_sta_connected {
    u8 bss_idx;
    u8 sta_idx;
    u8 channel;
	u8 bandwidth;
    u8 assoc_req_ie_len;
    u8 assoc_resp_ie_len;
    u8 bssid[ETH_ALEN];
	u8 is_wds;
	u8 padding[3];
	u16 aifs[4];
	u32	supp_rates;
	/* following is the assoc req/resp frame ies */
} __attribute__ ((packed));

/* WCI_D2H_RET_STA_DISCONNECTED */
struct wci_evt_ret_sta_disconnected {
	u8 bss_idx;
	u8 sta_idx;
} __attribute__ ((packed));

/* WCI_D2H_RET_STA_LINK_ERR */
struct wci_evt_ret_sta_link_err {
	u8 bss_idx;
	u8 addr[WLAN_ADDR_LEN];
	int sta_err;
} __attribute__ ((packed));

/* WCI_D2H_RET_TXPOWER */
struct wci_evt_ret_tx_power {
    u8 bss_idx;
    u8 padding[3];  /* avoid memory access fail */
    int tx_pwr;
    /* the following data is the ies */
} __attribute__ ((packed));

/* WCI_D2H_RET_VIF_ADDR */
struct wci_evt_ret_vif_addr {
	u8 bss_idx;
	u8 addr[WLAN_ADDR_LEN];
} __attribute__ ((packed));

/* WCI_D2H_RET_VIF_LINK_ST */
struct wci_evt_ret_vif_link_st {
	u8 bss_idx;
	u8 sta_idx_map;
	u8 res[2];	// padding
	u8 rate_idx[8];	// 8 = LYNX_STA_MAX_NUM
	u32 count[3];
} __attribute__ ((packed));

/* WCI_D2H_RX_MIC_ERR */
struct wci_evt_rx_mic_err {
	u8 bss_idx;
	u8 sta_idx;
	u8 is_broadcast;
	u8 key_id;
} __attribute__ ((packed));

/* WCI_D2H_RET_MON */
struct wci_evt_ret_mon {
    u32 mon_mask;
} __attribute__ ((packed));

/* WCI_D2H_RET_DBG_INFO */
struct wci_evt_dbg_info {
	u8 bss_idx;
    u8 padding[3];		/* avoid memory access fail */
	u16 item_id;
	u16 payload_len;
	/* following is the dbg info */
} __attribute__ ((packed));

enum {
    CIPHER_TYPE_NONE, 
    CIPHER_TYPE_WEP40,
    CIPHER_TYPE_WEP104,
    CIPHER_TYPE_TKIP,
    CIPHER_TYPE_CCMP,
    CIPHER_TYPE_SMS4,
};

enum {
    KEY_TYPE_PAIRWISE_KEY = 0,
    KEY_TYPE_STA_PAIRWISE_KEY,
    KEY_TYPE_GLOBAL_KEY,
    // KEY_TYPE_STA_GLOBAL_KEY, /* FIXME: removed by device firmware? */
};

enum dot11_auth_mode {
    OPEN_AUTH = 0x01,
    SHARED_AUTH = 0x02,

    /* different from IEEE_AUTH_MODE definitions */
    LEAP_AUTH = 0x04,
};

enum auth_mode {
    NONE_AUTH = 0x01,
    WPA_AUTH = 0x02,
    WPA2_AUTH = 0x04,
    WPA_PSK_AUTH = 0x08,
    WPA2_PSK_AUTH = 0x10,
    WPA_AUTH_CCKM = 0x20,
    WPA2_AUTH_CCKM = 0x40,
};


#define MAX_PROBED_SSIDS    16
#define WCI_MAX_CHANNELS    32
#define WCI_MAX_KEY_LEN     32

/* Note: host to device frame should be
         struct lynx_cmd_header + 
         struct lynx_cmd        + 
         struct wci_cmd_xxx (if necessary & may be variate) */

enum WCI_CMD_ID {
	/* "1" reserved */

    /* managment cmd */
    WCI_H2D_MGT_TX = 2,
    WCI_H2D_MGT_TX_ACK,
    WCI_H2D_DEV_START,
    WCI_H2D_DEV_STOP = 5,
    WCI_H2D_DEV_SCAN = 6,
    WCI_H2D_ACCESS_MEMORY,
    WCI_H2D_GET_FW_VERSION,
    WCI_H2D_VIF_ATTACH,
    WCI_H2D_VIF_DETACH = 10,
    WCI_H2D_VIF_UPDATE,
	WCI_H2D_START_AP,
    WCI_H2D_STA_ADD,
    WCI_H2D_STA_REMOVE,
    WCI_H2D_STA_CONNECT = 15,
    WCI_H2D_STA_DISCONNECT,
    WCI_H2D_ALIVE,
	WCI_H2D_WDS_PEER_ADD,
    /* config cmd */
    WCI_H2D_SET_MAC,
    WCI_H2D_SET_CHANNEL = 20,
    WCI_H2D_SET_TX_POWER,
    WCI_H2D_SET_TXQ,
    WCI_H2D_SET_ERP,
    WCI_H2D_SET_HT,
    WCI_H2D_SET_KEY = 25,
    WCI_H2D_SET_DEF_KEY,
    WCI_H2D_SET_BSS_WEP,
    WCI_H2D_SET_BSS_PSK,
    WCI_H2D_SET_TSF,
    WCI_H2D_SET_BITRATE_MASK = 30,
    WCI_H2D_SET_BEACON,
    WCI_H2D_SET_SLOTTIME,
	WCI_H2D_SET_MON,
    WCI_H2D_SET_IE_POOL,
    WCI_H2D_SET_BSS_UPDATE = 35,
    WCI_H2D_SET_PMKSA, 
    WCI_H2D_SET_RTS_THRESHOLD,
    WCI_H2D_SET_2040_COEX,
    /* get info cmd */
    WCI_H2D_GET_VIF,
    WCI_H2D_GET_STA = 40,
    WCI_H2D_GET_TSF,
    WCI_H2D_GET_IES,
    WCI_H2D_GET_TX_POWER,
    WCI_H2D_GET_VIF_ADDR,
    WCI_H2D_GET_VIF_LINK_ST = 45,
	WCI_H2D_GET_MON,
	/* special control cmd */
    WCI_H2D_SEND_ADDBA,
    WCI_H2D_SET_MIN_RATEIDX,
    WCI_H2D_SET_PS,
	WCI_H2D_DBG_INFO_CMD = 50,
	WCI_H2D_SET_SPECIAL_PARAM,

	WCI_H2D_IOCTL_FW_DEBUG = 123,
	WCI_H2D_SYNC_PID = 124,
	WCI_H2D_SET_MGMT_TXPKT = 125,
    /* host WPS P2P cmd */
    WCI_H2D_SET_MGMT_IE = 126,
    WCI_H2D_MP_TEST_CMD = 127,
    /* data packet */
    WCI_H2D_PACKET = 128,
	/* reserve 128 ~ 135 for data packet per BSS */
	WCI_H2D_PACKET_END = 135,	
	WCI_H2D_FW_INFO = 250,
	WCI_H2D_FW_DOWNLOAD = 254,
	WCI_H2D_FW_COMP = 255,
};

/* FIXME: define the last EVT without bss_idx as the WCI_D2H_WITHOUT_BSSIDX */
#define WCI_D2H_WITHOUT_BSSIDX WCI_D2H_CONFIG_DONE

enum WCI_RETURN_EVENT {
	WCI_D2H_TGT_RDY = 1,
	WCI_D2H_CONFIG_DONE,
	WCI_D2H_MGT_RX,
	WCI_D2H_FATAL,
	WCI_D2H_ADD_STA = 5,		/* return sta index */
	WCI_D2H_DEL_STA,			/* return sta index */
	WCI_D2H_ADDBA,
	WCI_D2H_DELBA,
	WCI_D2H_BAR,
	WCI_D2H_RX_MIC_ERR = 10,
	/* return 'get' data */
	WCI_D2H_RET_VIF,
	WCI_D2H_RET_STA,
	WCI_D2H_RET_TSF,
	WCI_D2H_RET_IES,
	WCI_D2H_RET_TXPOWER = 15,
	/* return cmd result */
	WCI_D2H_RET_VIF_ATTACH,
	WCI_D2H_RET_SCAN_STATE,
	WCI_D2H_RET_STA_CONNECTED,
	WCI_D2H_RET_STA_DISCONNECTED,	
	WCI_D2H_RET_VIF_ADDR = 20,
	WCI_D2H_RET_VIF_LINK_ST,
	WCI_D2H_RET_MON,
	WCI_D2H_RET_DBG_INFO,
	WCI_D2H_RET_STA_LINK_ERR,

	WCI_D2H_RET_IOCTL_FW_DEBUG = 123,
	WCI_D2H_RET_MP_TEST_CMD = 127,

	WCI_D2H_FW_INFO = 250,
};

enum wci_mgmt_frame_type {
    WCI_FRAME_BEACON = 0,
    WCI_FRAME_PROBE_REQ,
    WCI_FRAME_PROBE_RESP,
    WCI_FRAME_ASSOC_REQ,
    WCI_FRAME_ASSOC_RESP,
    WCI_FRAME_ACTION,
    WCI_NUM_MGMT_FRAME
};

enum wci_speial_param_list {
	WCI_SET_EN40MHZ = 0,
};

enum wci_ssid_flag {
    /* disables entry */
    DISABLE_SSID_FLAG = 0,

    /* probes specified ssid */
    SPECIFIC_SSID_FLAG = 0x01,

    /* probes for any ssid */
    ANY_SSID_FLAG = 0x02,

    /* match for ssid */
    MATCH_SSID_FLAG = 0x08,
};

struct lynx_wci_hdr {
    u8  cmd_id;
    u8  seq_no;
    u16 payload_len;
};


/***************************** function definition *****************************/
#ifdef CONFIG_LYNX_DEBUG
const char *wci_cmd_to_name(enum WCI_CMD_ID wci_cmd);
const char *wci_event_to_name(enum WCI_RETURN_EVENT wci_evt);
#endif

/*
 *  WCI CMD API
 */
void lynx_wci_start_cmd(struct lynx *lnx);
void lynx_wci_stop_cmd(struct lynx *lnx);

int lynx_wci_set_appie_cmd(struct lynx_vif *vif, const u8 *ie, u8 ie_len, enum wci_mgmt_frame_type mgmt_type);
int lynx_wci_beginscan_cmd(struct lynx_vif *vif, u32 channel_maps, u32 interval, u8 *ssid, u32 ssid_len);
int lynx_wci_reconnect_cmd(struct lynx_vif *vif);
void lynx_wci_addkey_cmd(struct lynx_vif *vif, const u8 *addr, struct lynx_key *key, u32 key_idx, u32 key_type);
int lynx_wci_set_defkey_cmd(struct lynx_vif *vif, int unicast, int multicast, int key_idx);
int lynx_wci_connect_cmd(struct lynx_vif *vif, char *bssid, char *ssid, u32 ssid_len, struct wlan_ie_info *info);
int lynx_wci_disconnect_cmd(struct lynx_vif *vif, u8 sta_idx, u16 reason_code);
void lynx_wci_detach_cmd(struct lynx_vif *vif);
void lynx_wci_attach_cmd(struct lynx_vif *vif);
void lynx_wci_update_vif_cmd(struct lynx_vif *vif);
int lynx_data_wci_hdr_add(struct sk_buff *skb, struct net_device *dev, enum WCI_CMD_ID cmd_id);
int lynx_wci_set_tx_pwr_cmd(struct lynx *lnx, unsigned int tx_pwr);
int lynx_wci_get_tx_pwr_cmd(struct lynx_vif *vif);
int lynx_wci_set_pmksa_cmd(struct lynx_vif *vif, u8 *bssid, u8 *pmkid, u8 is_add);
int lynx_wci_get_vif_addr_cmd(struct lynx_vif *vif);
int lynx_wci_set_beacon_cmd(struct lynx_vif *vif);
int lynx_wci_set_rts_cmd(struct lynx_vif *vif, u32 rts_threshold);
int lynx_wci_start_ap_cmd(struct lynx_vif *vif);
int lynx_wci_set_channel_cmd(struct lynx *lnx);
int lynx_wci_set_min_rateidx(struct lynx *lnx);
int lynx_wci_set_power_saving(struct lynx *lnx);
void lynx_wci_bulk_out_sync_pid(struct lynx *lnx);
int lynx_wci_sta_remove_cmd(struct lynx_vif *vif, unsigned int sta_idx, u32 reason);
int lynx_wci_wds_peer_add_cmd(struct lynx_vif *vif, int wds_mode, u8 *peer_addr);
int lynx_wci_set_mac_cmd(struct lynx_vif *vif, u8 *addr);
int lynx_wci_get_vif_link_st_cmd(struct lynx_vif *vif);
int lynx_wci_send_addba_cmd(struct lynx_vif *vif, int sta_idx, int tid);
int lynx_wci_set_2040_coex_cmd(struct lynx_vif *vif, int enable);
#if defined(CONFIG_HOST_WPS) || defined(CONFIG_CUST1_PRIV_BEACON_IE)
int lynx_wci_set_mgmt_ie_cmd(struct lynx_vif *vif, const u8 *ie, u16 ie_len, u8 mgmt_type);
#endif /*CONFIG_HOST_WPS*/
#ifdef CONFIG_HOST_P2P
int lynx_wci_set_mgmt_txpkt_cmd(struct lynx_vif *vif, const u8 *pkt, u16 pkt_len, u8 mgmt_type);
#endif /*CONFIG_HOST_P2P*/
#ifdef CONFIG_SUPPORT_MONITOR_MODE
int lynx_wci_set_monitor_cmd(struct lynx_vif *vif,u32 mon_mask);
int lynx_wci_get_monitor_cmd(struct lynx_vif *vif);
#endif /*CONFIG_SUPPORT_MONITOR_MODE */
void lynx_wci_update_chan_band(struct lynx_vif *vif,enum nl80211_channel_type superchan ,unsigned int channel,int skip_band);
#ifdef CONFIG_LYNX_DEBUG
int lynx_wci_dbg_info_cmd(struct lynx_vif *vif, int is_get, int item_id, int len, char *data);
#endif
#if defined(CONFIG_LYNX_OS_LINUX)
int lynx_wci_dynamic_access_cmd(struct lynx_vif *vif, struct mont_ioctl_data *cmd);
#endif	// CONFIG_LYNX_OS_LINUX
int lynx_wci_set_special_param_cmd(struct lynx_vif *vif, int item_id, int len, char *data);

/*WCI CMD END*/

void lynx_wci_callback(struct lynx *lnx, struct lynx_wci_hdr *cmd, u32 len);
void lynx_wci_sscan_timer(unsigned long ptr);
int wci_progress_check(int operation, int val);
struct lynx_vif *lynx_get_vif_by_index(struct lynx *lnx, u8 if_idx);
int lynx_freq2ch(unsigned int freq);
unsigned int lynx_ch_map(unsigned int freq);
int sta_map2idx(struct lynx_vif *vif, const u8 *addr);

#endif
