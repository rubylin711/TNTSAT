#ifndef CORE_H_
#define CORE_H_

#if defined(CONFIG_LYNX_OS_LINUX)
#include <linux/version.h>
#include <linux/rtnetlink.h>
#include <linux/skbuff.h>
#else	// defined(CONFIG_LYNX_OS_LINUX)
#include "byteorder.h"
#include "skbuff.h"
#endif	// defined(CONFIG_LYNX_OS_LINUX)

#if defined(CONFIG_LYNX_WM_MANAGER)
#include "ieee80211.h"
#elif defined(CONFIG_LYNX_OS_LINUX)
#include <linux/ieee80211.h>
#else
#define WLAN_CAPABILITY_ESS 			BIT(0)
#define WLAN_CAPABILITY_SHORT_PREAMBLE	BIT(5)
#define WLAN_CAPABILITY_SHORT_SLOT_TIME	BIT(10)
#endif	// defined(CONFIG_LYNX_WM_MANAGER)

#include "wlan_def.h"
#include "mac_ctrl.h"

#ifndef CONFIG_LYNX_OS_LINUX
struct ethhdr {
	unsigned char	h_dest[ETH_ALEN];	/* destination eth addr	*/
	unsigned char	h_source[ETH_ALEN];	/* source ether addr	*/
	__be16		h_proto;		/* packet type ID field	*/
} __attribute__((packed));

struct udphdr {
	__be16	source;
	__be16	dest;
	__be16	len;
	__sum16	check;
};

#endif	// CONFIG_LYNX_OS_LINUX

#ifndef ETH_P_PAE
#define ETH_P_PAE 0x888E
#endif /*ETH_P_PAE*/

#ifndef ETH_ALEN
#define ETH_ALEN	6
#endif

#ifndef MAX_ADDR_LEN
#define MAX_ADDR_LEN	32		/* Largest hardware address length */
#endif

#define VIF_MAX_NUM	3
#define RSSI_OFFSET	18

#if defined(CONFIG_LYNX_OS_LINUX)
#define TIME_UNIT	HZ
#elif defined(CONFIG_LYNX_OS_UCOS)
#define TIME_UNIT	1
#endif

#ifndef CONFIG_LYNX_W2USB_NO_RESET
#define WCI_TIMEOUT (3 * TIME_UNIT)
#else
#define WCI_TIMEOUT (1 * TIME_UNIT)
#endif
#define WCI_MAX_KEY_INDEX   3
#define LYNX_STA_MAX_NUM	8

#define ALL_SUPPORTED_RATES (B_RATES|OFDM_RATES|MCS_RATES)
#define ALL_BASIC_RATES (R_BIT(CCK_1M)|R_BIT(CCK_2M)|R_BIT(CCK_5_5M)|R_BIT(CCK_11M)| \
                        R_BIT(OFDM_6M)|R_BIT(OFDM_12M)|R_BIT(OFDM_24M))
#define STA_IDX_MAX 8   /* This is sta entry num in firmware */

#define AP_CAP_11B					BIT(0)
#define AP_CAP_11G					BIT(1)
#define AP_CAP_11N					BIT(2)
#define AP_CAP_11A					BIT(3)

enum {
	SLOTTIME_9US = 9,
	SLOTTIME_20US = 20,
};

enum BW_MODE {
	BW40MHZ_SCN = 0,	/* no secondary channel is present */
	BW40MHZ_SCA = 1,	/* secondary channel is above the primary channel */
	BW40MHZ_SCB = 3,	/* secondary channel is below the primary channel */
	BW40MHZ_AUTO = 4,	/* auto select secondary channel */
};

#if 0
struct wlan_ie_generic {
    u8 id;
    u8 len;
    u8 data[0];
} __attribute__ ((packed));

/*
 * WME/802.11e information element.
 */
struct wlan_ie_wme_info {
	u8		id;			
	u8		len;	
	u8		oui[3];	/* 0x00, 0x50, 0xf2 */
	u8		type;
	u8		subtype;
	u8		version;	
	u8		qosinfo;	
} __attribute__ ((packed));

/*
 * WME Parameter Element
 */
struct wme_acparams {
	u8 		reserved:1;
	u8		aci:2;
	u8 		acm:1;
	u8 		aifsn:4;
	u8 		ecwmax:4;
	u8 		ecwmin:4;
	u16		txop;
} __attribute__ ((packed));

struct wme_sta_qosinfo {
	u8 		:1;
	u8 		max_sp_len:2;
	u8 		:1;
	u8 		uapsd_ac_be:1;
	u8 		uapsd_ac_bk:1;
	u8 		uapsd_ac_vi:1;
	u8 		uapsd_ac_vo:1;
}__attribute__ ((packed));

struct wlan_ie_wme_param {
	u8		id;
	u8		len;
	u8		oui[3];
	u8		type;
	u8		subtype;
	u8		version;
	u8		qosinfo;
#define	WME_QOSINFO_COUNT	0x0f	/* Mask for param count field */
#define WME_QOSINFO_APSD	0x80
	u8		reserved;
#define WME_NUM_AC		4	/* 4 AC categories */
	struct wme_acparams	ac_params[WME_NUM_AC];
} __attribute__ ((packed));
#endif

/*FIXME: put this in which header*/
enum htc_endpoint_id {
    ENDPOINT_UNUSED = -1,
    ENDPOINT_0 = 0,
    ENDPOINT_1 = 1,
    ENDPOINT_2 = 2,
    ENDPOINT_3,
    ENDPOINT_4,
    ENDPOINT_5,
    ENDPOINT_6,
    ENDPOINT_7,
    ENDPOINT_8,
    ENDPOINT_MAX,
};

#if 0
/* Encode Bit */
#define IE_SSID_BIT             BIT(0)
#define IE_SUPP_RATES_BIT       BIT(1)
#define IE_DS_PARMS_BIT         BIT(2)
#define IE_TIM_BIT              BIT(3)
#define IE_COUNTRY_BIT          BIT(4)
#define IE_ERP_INFO_BIT         BIT(5)
#define IE_RSN_BIT              BIT(6)
#define IE_HT_CAP_BIT           BIT(7)
#define IE_EXT_SUPP_RATES_BIT   BIT(8)
#define IE_HT_INFO_BIT          BIT(9)
#define IE_EXT_CAP_BIT          BIT(10)
#define IE_WPA_BIT              BIT(11)
#define IE_WAPI_BIT             BIT(12)
#define IE_WMM_BIT              BIT(13)
#define IE_WPS_BIT2             BIT(14)
#define IE_WPS_BIT              BIT(15)
#define IE_IBSS_BIT             BIT(16)
#define IE_P2P_BIT              BIT(17)
#define IE_END_BIT              BIT(18)     /* last bit */
#endif

/* struct lynx->flag info 
 * FIXME:Not all flag use.
 */
enum lynx_dev_state {
    WCI_READY,
    DESTROY_IN_PROGRESS,

	/* FIXME: check that these items are needed */
    WCI_CTRL_EP_FULL,
    FIRST_BOOT,
    RECOVERY_CLEANUP,
	WCI_LINK_ST_RET,
};

/* vif flags info */
/* ((CONNECTED | CONNECT_PEND) == 0) : disconnect */
enum lynx_vif_state {
	DEV_ATTACHED,
	CONNECTED,
	CONNECT_PEND,
	WLAN_ENABLED,
	SCHED_SCANNING,
	DATAPATH_EN,

	/* FIXME: check that these items are needed */
    WMM_ENABLED,
    NETQ_STOPPED,
    DTIM_EXPIRED,
    CLEAR_BSSFILTER_ON_BEACON,
    DTIM_PERIOD_AVAIL,
    STATS_UPDATE_PEND,
    HOST_SLEEP_MODE_CMD_PROCESSED,
    NETDEV_MCAST_ALL_ON,
    NETDEV_MCAST_ALL_OFF,
};

#define  KEY_SEQ_LEN 8

struct lynx_key {
    u8 key[WLAN_MAX_KEY_LEN];
    u8 key_len;
    u8 is_txkey;
    u8 key_type;
    u8 cipher;
};

/* refer the device firmware ldev struct */
struct device_configs {
	u8	phy_cap;
	u8	slottime;
	u16	capability;

	u8	channel_num;
	u8	disable_ampdu;
	u8	ampdu_params;
	u8	ba_win_size;

	u8	channel;
	u8	bandwidth;
	u16 rts_threshold;

	u8	power_saving_mode;
	u8	bw40mhz_intolerant;
	u16 listen_interval_max; /* listen interval maximum value */

	struct wlan_wme_ac_params sta_ac_parms[4];
	struct wlan_wme_ac_params ap_ac_parms[4];
	u32	ampdu_tx_mask;
	u32	wrx_filter_mask;
	u32 current_rates;
	u8	country[4];			/* country string */
	u8 	obss_scan;			/* FIXME: obss_scan is needed? */
	u8	cts_protection_type;
	u8	min_rate_idx;
	u8	padding;
};

#define LYNX_WMM_QUEUE_SIZE	 256
struct lynx_queue_arr {
	int free_queue_counter;
	struct sk_buff_head tx_queue;
};

#define LYNX_QUEUE_IN_Q_VO		BIT(0)
#define LYNX_QUEUE_IN_Q_VI		BIT(1)
#define LYNX_QUEUE_IN_Q_BE		BIT(2)
#define LYNX_QUEUE_IN_Q_BK		BIT(3)
#define LYNX_QUEUE_IN_URGENT	BIT(4)
#define LYNX_QUEUE_DEQUEING		BIT(5)
#define LYNX_QUEUE_ENABLE		BIT(6)

#define LYNX_QUEUE_NUMS		5
#define LYNX_QUEUE_MASK		((1 << LYNX_QUEUE_NUMS) - 1)

#define LYNX_URGENT_QUEUE_NO (LYNX_QUEUE_NUMS - 1)

/*
	wmm_queue:
		QUEUE 0: IEEE80211_AC_VO
		QUEUE 1: IEEE80211_AC_VI
		QUEUE 2: IEEE80211_AC_BE
		QUEUE 3: IEEE80211_AC_BK
		QUEUE 4: Urgent Queue
*/

/* WMM Queue Handle */
struct lynx_queue {
	unsigned int flags;
	unsigned char priority[LYNX_QUEUE_NUMS];
	struct lynx_queue_arr wmm_queue[LYNX_QUEUE_NUMS];
};


#define LYNX_STA_VALID			0x00000001
#define LYNX_STA_WDS_PEER		0x00000002

struct lynx_sta {
	struct lynx_vif *vif;	/* direct to the vif in the lnx */
	u32 flags;
	u8 addr[ETH_ALEN];
	u32 support_rates;
	u8 bandwidth;

	/* FIXME: may add other info, ex: security */

	/* statics */
	int signal_last;
	int	signal_avg;
	unsigned int last_active_time;
	unsigned long rx_bytes;
	unsigned long rx_packets;
	unsigned long tx_bytes;
	unsigned long tx_packets;
};

#if defined(CONFIG_LYNX_OS_LINUX)
#define OS_WAIT_QUEUE_T wait_queue_head_t
#elif defined(CONFIG_LYNX_OS_UCOS)
#define OS_WAIT_QUEUE_T os_sem_t
#endif

struct lynx {
#if defined(CONFIG_LYNX_OS_LINUX)
    struct device   *dev;
    struct wiphy    *wiphy;
#endif

    u8 num_vif;
    unsigned int vif_max;
    u8 avail_idx_map;
    
    u8 mac_addr[ETH_ALEN];

    /* channel info */
    u8 channel;

    OS_WAIT_QUEUE_T event_wq;
#if defined(CONFIG_LYNX_OS_LINUX)
    struct semaphore sem;
#endif

    /* VIF list to check which vif should use by fw_vif_idx*/
    struct list_head vif_list;

    /* Lock to avoid race in vif_list entries among add/del/traverse */
    OS_LOCK_TYPE list_lock;

    u32 current_rates;
    u32 basic_rates;
    int tx_pwr;

    /*FIXME:to memorize current lnx state*/
    unsigned long flag;
    
    /* USB/SDIO */
    void *hif_priv;

    /* IBSS feature */
    u8 ibss_existing;

	struct device_configs fw_config;

	/* sta list, may dynamic alloc? */
	struct lynx_sta sta_list[LYNX_STA_MAX_NUM];
	unsigned char sta_tx_rate[LYNX_STA_MAX_NUM];
	
	struct lynx_queue queue;

	struct proc_dir_entry *proc_dir;

	/* mlme ops, os dep */
	struct lynx_mlme_ops *mlme_ops;

#ifdef CONFIG_SUPPORT_MONITOR_MODE
	u32 mon_mask;
	u8 fw_monitor_en;
	u32 monitor_level;
#endif /*CONFIG_SUPPORT_MONITOR_MODE */

#ifdef CONFIG_HOST_P2P
	u32 remain_on_channel_cookie;
	struct ieee80211_channel remain_on_channel;
#endif /*CONFIG_HOST_P2P*/

	/* debug info */
#ifdef CONFIG_TIMER_HANDLER
	unsigned int scan_not_done_count;
#endif

	int en_40mhz;	/* enable HT 40Mhz support */

	s32 cqm_rssi_thold;
	u32 cqm_rssi_hyst;
	int last_cqm_event_rssi;
	int ave_beacon_signal;
	unsigned int count_beacon_signal;
	u32 fw_count[3]; //0:tx,1:rx,2:beacon
};

#define WCI_EVT_TIMER_FLAGS_CONNECT_CHECK	BIT(0)

#define LYNX_WME_ACM_VO		BIT(0)
#define LYNX_WME_ACM_VI		BIT(1)
#define LYNX_WME_ACM_BE		BIT(2)
#define LYNX_WME_ACM_BK		BIT(3)

struct lynx_vif {
    struct list_head list;
#if defined(CONFIG_LYNX_OS_LINUX) && !defined(CONFIG_LYNX_WM_MANAGER)
    struct wireless_dev wdev;
#endif
    struct net_device *ndev;
    struct lynx *lnx;
    
    /* Lock to protect vif specific net_stats and flags */
    OS_LOCK_TYPE if_lock;
    u8 fw_vif_idx;
    unsigned long flags;
    
    u8 nw_type;

    /* sta info */
    u8 sta_idx_map; /* now device firmware only support 8 sta entries */

    //struct lynx_htcap htcap[IEEE80211_NUM_BANDS]; // ?

    u8 seq_no;  /* FIXME: may not need anymore */

    u8 vif_addr[ETH_ALEN];
    u8 req_bssid[ETH_ALEN];
    u16 ch_hint;
    
    int ssid_len;
    u8 ssid[IEEE80211_MAX_SSID_LEN];
    u8 dot11_auth_mode;
    u8 auth_mode;
    u8 prwise_crypto;
    u8 grp_crypto;

    u8 def_txkey_index;
    struct lynx_key keys[WCI_MAX_KEY_INDEX + 1];

	struct wm_bss bss;	/* FIXME: move the duplicated items to the bss */
#ifdef CONFIG_UNIT_TEST
	struct wm_bss bss_copy;
#endif

#if 0
    struct timer_list sched_scan_timer;
#endif

#if defined(CONFIG_LYNX_OS_LINUX)
	/* work_queue for sync device & host driver */
	struct delayed_work sync_work;

#ifdef CONFIG_TIMER_HANDLER
	/* work_queue for host drvier do some timeout handle */
	struct delayed_work timer_work;
#endif

	struct cfg80211_scan_request *scan_req;
#ifdef CONFIG_TIMER_HANDLER
	unsigned long scan_start_time;
#endif	// CONFIG_TIMER_HANDLER
#endif	// CONFIG_LYNX_OS_LINUX

	/* For WMM ACM policy */
	unsigned int acm;

#if defined(CONFIG_LYNX_OS_LINUX)
    struct net_device_stats net_stats;
#endif

#ifdef CONFIG_HOST_P2P
	u16 mgmt_rx_reg;
	u32 action_cookie;
#endif /*CONFIG_HOST_P2P*/
};

#if !defined(CONFIG_LYNX_OS_LINUX)
struct net_device
{
      struct lynx_vif     vif;
      unsigned char       dev_addr[MAX_ADDR_LEN];
      unsigned short      needed_headroom;
};
#endif

enum {
	/* whole device */
	VIF_CFG_MIN_RATEIDX,
	VIF_CFG_PS,
	VIF_CFG_CHANNEL,
	VIF_CFG_BANDWIDTH,
	VIF_GET_TXPWR,
	/* per vif */
	VIF_CFG_PHY_CAP,
	VIF_CFG_BEACON_INTERVAL,
	VIF_CFG_DTIM,
	VIF_CFG_BSSID,
	VIF_CFG_SSID,
	VIF_CFG_AUTH_TYPE,
	VIF_CFG_HT_CAP,
};

struct lynx *lynx_core_create(void);
int lynx_core_init(struct lynx *lnx);
void lynx_core_cleanup(struct lynx *lnx);

int lynx_wmm_queue_prioty_setup(struct lynx *lnx);
int lynx_wmm_classify(struct sk_buff *skb);
int lynx_data_queue_is_empty(struct lynx *lnx);
int lynx_data_enqueue(struct sk_buff *skb, struct lynx_vif *vif, int urgent);
int lynx_data_urgentq_check(struct sk_buff *skb);
void lynx_data_dequeue(struct lynx *lnx);
int lynx_data_tx(struct sk_buff *skb, struct net_device *dev);

void *lynx_interface_add(struct lynx *lnx, const char *name, unsigned char name_type,
                    enum nl80211_iftype type, u8 fw_vif_idx, u8 nw_type);
int lynx_interface_del(struct lynx *lnx, struct lynx_vif *vif);

struct lynx *lynx_priv(struct net_device *dev);
struct lynx_vif *lynx_vif_first(struct lynx *lnx);

struct lynx_vif *lynx_remove_vif_first(struct lynx *lnx);
bool lynx_remove_vif(struct lynx *lnx ,struct lynx_vif * search_vif);

struct lynx_vif *lynx_get_vif_by_index(struct lynx *lnx, u8 if_idx);
int lynx_vif_cfg(struct lynx *lnx, struct lynx_vif *vif, int item, unsigned long value);

int bb_rssi_decode(unsigned char val, int rssi_offset);
#ifdef CONFIG_LYNX_STATICS
void update_sta_static(struct lynx_vif *vif, int sta_idx, int is_tx, int rssi, int bytes);
#endif

void lynx_vif_cqm_rssi_notify(struct lynx_vif *vif, s32 rssi,int beacon);

#ifdef CONFIG_HOST_P2P
struct lynx_vif *lynx_get_p2p_vif(struct lynx *lnx,bool at_lynx_rx_tasklet);
struct lynx_vif *lynx_get_p2p_data_vif(struct lynx *lnx,bool at_lynx_rx_tasklet);
#endif /*CONFIG_HOST_P2P*/

struct lynx_vif *lynx_get_sta_vif(struct lynx *lnx,bool at_lynx_rx_tasklet);

struct lynx_vif *lynx_get_scanning_vif(struct lynx *lnx,bool at_lynx_rx_tasklet);

#ifdef CONFIG_SUPPORT_MONITOR_MODE
int wlan_stop_monitor(struct lynx_vif *vif);
int wlan_init_monitor(struct lynx_vif *vif);
int lynx_monitor_switch(struct lynx_vif *vif,u32 new_monitor_level);
#endif /*CONFIG_SUPPORT_MONITOR_MODE*/


#endif /*CORE_H_*/
