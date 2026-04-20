/*=============================================================================+
|                                                                              |
| Copyright 2013                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file 
*   \brief
*   \author Montage
*/

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <linux/types.h>
#include <linux/ieee80211.h>
#include <linux/etherdevice.h>
#include <linux/err.h>
#include <linux/spinlock.h>
#include <net/cfg80211.h>
#include "wlan_def.h"
#include "init.h"
#include "core.h"
#include "wci.h"
#include "lynx_debug.h"
#include "cfg80211.h"
#include "mlme_api.h"
#include <linux/version.h> /*Get LINUX_VERSION_CODE*/

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#define DEFAULT_BG_SCAN_PERIOD 60

#define lynx_g_htcap    IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40 | \
                        IEEE80211_HT_CAP_GRN_FLD | IEEE80211_HT_CAP_DSSSCCK40

#define CHAN2G(_channel, _freq, _flags) {       \
    .band               = NL80211_BAND_2GHZ,  \
    .hw_value           = (_channel),           \
    .center_freq        = (_freq),              \
    .flags              = (_flags),             \
    .max_antenna_gain   = 0,                    \
    .max_power          = 20,                   \
}

#define RATETAB_ENT(_rate, _rateid, _flags) {   \
    .bitrate    = (_rate),                      \
    .flags      = (_flags),                     \
    .hw_value   = (_rateid),                    \
}

#define lynx_g_rates_size    12


extern unsigned int mp_test_firm;
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
static const u32 cipher_suites[] = {
    WLAN_CIPHER_SUITE_WEP40,
    WLAN_CIPHER_SUITE_WEP104,
    WLAN_CIPHER_SUITE_TKIP,
    WLAN_CIPHER_SUITE_CCMP,
    WLAN_CIPHER_SUITE_SMS4,
};

static const struct ieee80211_txrx_stypes 
lynx_mgmt_stypes[NUM_NL80211_IFTYPES] = {
        [NL80211_IFTYPE_STATION] = {
        },
        [NL80211_IFTYPE_AP] = {
            .tx = BIT(IEEE80211_STYPE_ACTION >> 4) | BIT(IEEE80211_STYPE_PROBE_RESP >> 4),
            .rx = BIT(IEEE80211_STYPE_ACTION >> 4) | BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
        },
#ifdef CONFIG_HOST_P2P
        [NL80211_IFTYPE_P2P_CLIENT] = {
        },
        [NL80211_IFTYPE_P2P_GO] = {
            .tx = BIT(IEEE80211_STYPE_ACTION >> 4) | BIT(IEEE80211_STYPE_PROBE_RESP >> 4),
            .rx = BIT(IEEE80211_STYPE_ACTION >> 4) | BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
        },
        [NL80211_IFTYPE_P2P_DEVICE] = {
            .tx = BIT(IEEE80211_STYPE_ACTION >> 4) | BIT(IEEE80211_STYPE_PROBE_RESP >> 4),
            .rx = BIT(IEEE80211_STYPE_ACTION >> 4) | BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
        },
#endif /*CONFIG_HOST_P2P*/
};

static struct ieee80211_channel lynx_2ghz_channels[] = {
    CHAN2G(1, 2412, 0),  /* Channel 1  */
    CHAN2G(2, 2417, 0),  /* Channel 2  */
    CHAN2G(3, 2422, 0),  /* Channel 3  */
    CHAN2G(4, 2427, 0),  /* Channel 4  */
    CHAN2G(5, 2432, 0),  /* Channel 5  */
    CHAN2G(6, 2437, 0),  /* Channel 6  */
    CHAN2G(7, 2442, 0),  /* Channel 7  */
    CHAN2G(8, 2447, 0),  /* Channel 8  */
    CHAN2G(9, 2452, 0),  /* Channel 9  */
    CHAN2G(10, 2457, 0), /* Channel 10 */
    CHAN2G(11, 2462, 0), /* Channel 11 */
    CHAN2G(12, 2467, 0), /* Channel 12 */
    CHAN2G(13, 2472, 0), /* Channel 13 */
    CHAN2G(14, 2484, 0), /* Channel 14 */
#ifdef CONFIG_CH_15
    CHAN2G(15, 2504, 0), /* Channel 15 */
#endif
#ifdef CONFIG_LOW_FREQ_CH
    CHAN2G(29, 2382, 0), /* Channel 29 */
    CHAN2G(30, 2387, 0), /* Channel 30 */
    CHAN2G(31, 2392, 0), /* Channel 31 */
    CHAN2G(32, 2402, 0), /* Channel 32 */
#endif
};

struct ieee80211_rate lynx_g_rates[] = {
    RATETAB_ENT(10, 0x1, 0),
    RATETAB_ENT(20, 0x2, 0),
    RATETAB_ENT(55, 0x4, 0),
    RATETAB_ENT(110, 0x8, 0),
    RATETAB_ENT(60, 0x10, 0),
    RATETAB_ENT(90, 0x20, 0),
    RATETAB_ENT(120, 0x40, 0),
    RATETAB_ENT(180, 0x80, 0),
    RATETAB_ENT(240, 0x100, 0),
    RATETAB_ENT(360, 0x200, 0),
    RATETAB_ENT(480, 0x400, 0),
    RATETAB_ENT(540, 0x800, 0),
};

static struct ieee80211_supported_band lynx_band_2ghz = {
    .n_channels             = ARRAY_SIZE(lynx_2ghz_channels),
    .channels               = lynx_2ghz_channels,
    .n_bitrates             = lynx_g_rates_size,
    .bitrates               = lynx_g_rates,
    .ht_cap.cap             = lynx_g_htcap,
    .ht_cap.ht_supported    = true,
};


static const struct ieee80211_iface_limit lynx_iface_limits[] = {
//Allow #P2P_Device <= 1, #{P2P-Client,P2P-GO} <= 1,#{STA,AP} <= 1 on only a channels, 3 total.
#ifdef CONFIG_HOST_P2P
    {
        .max = 1,//  (num of ifconfig up-able )
        .types = BIT(NL80211_IFTYPE_STATION) | BIT(NL80211_IFTYPE_AP),
    },
    {
        .max = 1,//  (num of ifconfig up-able )
        .types =BIT(NL80211_IFTYPE_P2P_GO) |BIT(NL80211_IFTYPE_P2P_CLIENT),
    },
    {
        .max = 1,//  (num of ifconfig up-able )
        .types = BIT(NL80211_IFTYPE_P2P_DEVICE),
    },
#else
//Allow #{STA,AP} <= 2 on only a channels, 2 total.
    {
        .max = 2,//  (num of ifconfig up-able )
      .types = BIT(NL80211_IFTYPE_STATION) | BIT(NL80211_IFTYPE_AP),
    },
#endif /*CONFIG_HOST_P2P*/
    
};

static const struct ieee80211_iface_combination lynx_iface_combinations[] = {
    {
#ifdef CONFIG_HOST_P2P
        .max_interfaces = 3, //(num of iw add-able)
#else
        .max_interfaces = 2, //(num of iw add-able)
#endif /*CONFIG_HOST_P2P*/
        .limits = lynx_iface_limits,
        .n_limits = ARRAY_SIZE(lynx_iface_limits),
        .num_different_channels = 1,
    },
};

/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
static int lynx_cfg80211_disconnect(struct wiphy *wiphy, struct net_device *dev, u16 reason_code);

/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/

static inline struct lynx_vif *lynx_vif_from_wdev(struct wireless_dev *wdev)
{
    return container_of(wdev, struct lynx_vif, wdev);
}

/*======================================================================================+
 |                                                                                      |
 |                                  Utils                                               |
 |                                                                                      |
 +=====================================================================================*/
#if 1
u32 support_rate_to_bits(u8 *data, u8 len)
{
	u8 *pos, *end;
	u32 rates;

	rates = 0;
	pos = data;
	end = data + len;
	while(pos < end)
	{
		switch(*pos & 0x7f)
		{
			case 2:	
				rates |= R_BIT(CCK_1M);
				break;
			case 4:	
				rates |= R_BIT(CCK_2M);
				break;
			case 11:	
				rates |= R_BIT(CCK_5_5M);
				break;
			case 22:	
				rates |= R_BIT(CCK_11M);
				break;
			case 12:	
				rates |= R_BIT(OFDM_6M);
				break;
			case 18:	
				rates |= R_BIT(OFDM_9M);
				break;
			case 24:	
				rates |= R_BIT(OFDM_12M);
				break;
			case 36:	
				rates |= R_BIT(OFDM_18M);
				break;
			case 48:	
				rates |= R_BIT(OFDM_24M);
				break;
			case 72:	
				rates |= R_BIT(OFDM_36M);
				break;
			case 96:	
				rates |= R_BIT(OFDM_48M);
				break;
			case 108:	
				rates |= R_BIT(OFDM_54M);
				break;
			default:
				break;
		}
		pos++;
	}
	return rates;
}

u32 ht_rate_to_bits(char *supported_mcs_set)
{
	u32 i, j, rates = 0;
	
	/* only support 1x1 stream currently */
	for(j = 0; j < 1; j++)
	{
		for(i = 0; i < 8; i++)
		{
			/* MCS_0 start at BIT(12) */
			if(supported_mcs_set[j] & R_BIT(i))
				rates |= R_BIT(i + j + MCS_0);
		}
	}

	return rates;	
}

int lynx_parse_wme_ie(struct wlan_ie_wme_param *wme_param)
{
	int i;
	int wme_acm=0;
	u8 acm=0, aci=0;
	u8 *ac_parm;

	if((wme_param->len >= 8) && (wme_param->version == 1))
	{
		for(i=0; i<WME_NUM_AC; i++)
		{
			ac_parm = (unsigned char *)&wme_param->ac_params[i];
			aci = (ac_parm[0] & 0x60) >> 5;
			acm = (ac_parm[0] & 0x10) >> 4;
	
			switch(aci)
			{
				case 1:	/* AC_BK */
					if(acm)
						wme_acm |= LYNX_WME_ACM_BK;
					break;
				case 2: /* AC_VI */
					if(acm)
						wme_acm |= LYNX_WME_ACM_VI;
					break;
				case 3: /* AC_VO */
					if(acm)
						wme_acm |= LYNX_WME_ACM_VO;
					break;
				case 0: /* AC_BE */
				default:
					if(acm)
						wme_acm |= LYNX_WME_ACM_BE;
					break;
			}
		}
	}
	
	return wme_acm;
}

/* refer the enum ieee80211_eid in <linux/ieee80211.h> */
u32 lynx_parse_ie(const u8 *pos, u32 left, struct wlan_ie_info *info)
{
	while(left >= 2) 
	{
		struct wlan_ie_generic *one = (struct wlan_ie_generic *)pos;

		if(left < one->len)
			return -1;

		switch (one->id) 
		{
			case WLAN_EID_SUPP_RATES:
				if(one->len > 32)
					return -1;
				info->supp_rates |= support_rate_to_bits(one->data, one->len);

				if(info->supp_rates & R_BIT(CCK_5_5M))
					info->phy_cap |= AP_CAP_11B;
				if(info->supp_rates & OFDM_RATES)
					info->phy_cap |= AP_CAP_11G;
				break;
			case WLAN_EID_EXT_SUPP_RATES:
				if(one->len > 32)
					return -1;
				info->supp_rates |= support_rate_to_bits(one->data, one->len);
				break;
			case WLAN_EID_ERP_INFO:
				info->phy_cap |= AP_CAP_11G;
				break;
			case WLAN_EID_HT_CAPABILITY:
				if(one->len < (sizeof(struct ieee80211_ht_cap)))
					goto skip;
				info->phy_cap |= AP_CAP_11N;
				info->ht_cap = ((struct ieee80211_ht_cap*)one->data)->cap_info;
				info->ampdu_params = ((struct ieee80211_ht_cap*)one->data)->ampdu_params_info;
				info->supp_rates |= ht_rate_to_bits((u8 *)&((struct ieee80211_ht_cap*)one->data)->mcs);
				break;
			case WLAN_EID_RSN:
				info->rsn_ie = (u8 *)one;
				break;
			case WLAN_EID_VENDOR_SPECIFIC:
				{
					unsigned int oui;

					/* OUI(3 bytes) + type(1 byte) */
					if(one->len < 4)
						goto skip;
					oui = (one->data[0] << 16) |  (one->data[1] << 8) |  one->data[2];
					if(oui == MICROSOFT_OUI) 
					{
						/* Microsoft/Wi-Fi information elements are further typed and subtyped */
						if(one->data[3] == WPA_OUI_TYPE)
							info->wpa_ie = (u8 *)one;
						else if(one->data[3] == WME_OUI_TYPE)
							info->wme_ie = (u8 *)one;
					}
				}
				break;
		}
skip:
		left -= (one->len + 2);
		pos += (one->len + 2);
	}

	if(left)
		return -1;

	return 0;
}

#endif

/*======================================================================================+
 |                                                                                      |
 |                                  END                                                 |
 |                                                                                      |
 +=====================================================================================*/

static struct cfg80211_scan_info info;

void lynx_cfg80211_scan_complete(struct lynx_vif *vif, u8 isok)
{   
    bool aborted = false;
        
    lynx_dbg(LYNX_DBG_WLAN_CFG, "scan complete: isok=%d, vif->scan_req=0x%x, SCHED_SCANNING=%d\n", 
                        isok, (void *)vif->scan_req, test_bit(SCHED_SCANNING, &vif->flags));
    
    if(vif->scan_req)
    {
        if (isok != 1)
            aborted = true;

	info.aborted = aborted;
        cfg80211_scan_done(vif->scan_req, &info);
        vif->scan_req = NULL;
    }
    else if(test_bit(SCHED_SCANNING, &vif->flags))
    {
        cfg80211_sched_scan_results(vif->lnx->wiphy, 0);
    }
}

static int lynx_clear_cipher(struct lynx_vif *vif)
{
	struct wm_bss *bss = &vif->bss;

	vif->prwise_crypto = 0;
 	vif->grp_crypto = 0;

	bss->auth_capability = 0;

	return 0;
}

static unsigned int lynx_set_cipher(struct lynx_vif *vif, unsigned int cipher, bool ucast)
{
	struct wm_bss *bss = &vif->bss;
	unsigned int auth_capability = be32_to_cpu(bss->auth_capability);
	unsigned char *lynx_cipher = ucast ? &vif->prwise_crypto : &vif->grp_crypto;
	unsigned char cipher_type=0;

    lynx_dbg(LYNX_DBG_WLAN_CFG, "%s: cipher 0x%x, ucast %u\n", __FUNCTION__, cipher, ucast);

	switch (cipher) {
		case 0:
			/* value 0 as no crypto used */
			*lynx_cipher |= BIT(CIPHER_TYPE_NONE);
			cipher_type = CIPHER_TYPE_NONE;
			auth_capability |= AUTH_CAP_CIPHER_NONE;
			break;
		case WLAN_CIPHER_SUITE_WEP40:
			*lynx_cipher = BIT(CIPHER_TYPE_WEP40);
			cipher_type = CIPHER_TYPE_WEP40;
			auth_capability |= (AUTH_CAP_CIPHER_WEP40 | AUTH_CAP_WEP);
			break;
		case WLAN_CIPHER_SUITE_WEP104:
			*lynx_cipher = BIT(CIPHER_TYPE_WEP104);
			cipher_type = CIPHER_TYPE_WEP104;
			auth_capability |= (AUTH_CAP_CIPHER_WEP104 | AUTH_CAP_WEP);
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			*lynx_cipher |= BIT(CIPHER_TYPE_TKIP);
			cipher_type = CIPHER_TYPE_TKIP;
			auth_capability |= AUTH_CAP_CIPHER_TKIP;
			break;
		case WLAN_CIPHER_SUITE_CCMP:
			*lynx_cipher |= BIT(CIPHER_TYPE_CCMP);
			cipher_type = CIPHER_TYPE_CCMP;
			auth_capability |= AUTH_CAP_CIPHER_CCMP;
			break;
		case WLAN_CIPHER_SUITE_SMS4:
			*lynx_cipher |= BIT(CIPHER_TYPE_SMS4);
			cipher_type = CIPHER_TYPE_SMS4;
			auth_capability |= AUTH_CAP_CIPHER_SMS4;
			break;
		default:
			lynx_dbg(LYNX_DBG_WLAN_CFG, "cipher 0x%x not supported\n", cipher);
			return -ENOTSUPP;
	}

	bss->auth_capability = cpu_to_be32(auth_capability);

	return cipher_type;
}

static int lynx_set_auth_type(struct lynx_vif *vif, 
                    enum nl80211_auth_type auth_type)
{
	struct wm_bss *bss = &vif->bss;
	u32 auth_capability = be32_to_cpu(bss->auth_capability);
    lynx_dbg(LYNX_DBG_WLAN_CFG, "%s: 0x%x\n", __FUNCTION__, auth_type);

    switch(auth_type) {
        case NL80211_AUTHTYPE_OPEN_SYSTEM:
            vif->dot11_auth_mode = OPEN_AUTH;
			auth_capability |= AUTH_CAP_OPEN;
            break;
        case NL80211_AUTHTYPE_SHARED_KEY:
            vif->dot11_auth_mode = SHARED_AUTH;
			auth_capability &= ~AUTH_CAP_OPEN;
            break;
        case NL80211_AUTHTYPE_NETWORK_EAP:
            vif->dot11_auth_mode = LEAP_AUTH;
			auth_capability |= AUTH_CAP_LEAP;
            break;
        case NL80211_AUTHTYPE_AUTOMATIC:
            vif->dot11_auth_mode = OPEN_AUTH | SHARED_AUTH;
			auth_capability |= (AUTH_CAP_OPEN | AUTH_CAP_AUTO_AUTH_ALG);
            break;
        default:
            lynx_dbg(LYNX_DBG_WLAN_CFG, "%s: 0x%x not supported\n", __FUNCTION__, auth_type);
            return -ENOTSUPP;
        }

		bss->auth_capability = cpu_to_be32(auth_capability);

        return 0;
}

static int lynx_set_wpa_version(struct lynx_vif *vif, 
                    enum nl80211_wpa_versions wpa_version)
{
	struct wm_bss *bss = &vif->bss;
	u32 auth_capability = be32_to_cpu(bss->auth_capability);
    lynx_dbg(LYNX_DBG_WLAN_CFG, "%s: %u\n", __FUNCTION__, wpa_version);

    if (!wpa_version) {
        vif->auth_mode = NONE_AUTH;
    } else if (wpa_version & NL80211_WPA_VERSION_2) {
        vif->auth_mode = WPA2_AUTH;
		auth_capability |= AUTH_CAP_WPA2;
    } else if (wpa_version * NL80211_WPA_VERSION_1) {
        vif->auth_mode = WPA_AUTH;
		auth_capability |= AUTH_CAP_WPA;
    } else {
        lynx_dbg(LYNX_DBG_WLAN_CFG, "%s: %u not supported\n", __FUNCTION__, wpa_version);
        return -ENOTSUPP;
    }
    
	bss->auth_capability = cpu_to_be32(auth_capability);

    return 0;
}

#ifdef CONFIG_HOST_WPS
static int lynx_set_wps_enable(struct lynx_vif *vif,bool wps_enable)
{
    struct wm_bss *bss = &vif->bss;
    u32 auth_capability = be32_to_cpu(bss->auth_capability);
    lynx_dbg(LYNX_DBG_WLAN_CFG, "%s:WPS enable or not :%d\n", __FUNCTION__, wps_enable);

    if (wps_enable) {
        auth_capability |= AUTH_CAP_WPS;    
    } else {
        auth_capability &= ~AUTH_CAP_WPS;
    }
    
    bss->auth_capability = cpu_to_be32(auth_capability);
    return 0;
}
#endif /*CONFIG_HOST_WPS*/


static void lynx_set_key_mgmt(struct lynx_vif *vif, u32 key_mgmt)
{
	struct wm_bss *bss = &vif->bss;
	u32 auth_capability = be32_to_cpu(bss->auth_capability);

    lynx_dbg(LYNX_DBG_WLAN_CFG, "%s: 0x%x\n", __func__, key_mgmt);

	/* FIXME: May vif need to record mgmt? */

	if (key_mgmt == WLAN_AKM_SUITE_PSK)
		auth_capability |= AUTH_CAP_KEY_MGT_PSK;
	else if (key_mgmt == WLAN_AKM_SUITE_8021X)
		auth_capability |= AUTH_CAP_KEY_MGT_1X;
	else
    	lynx_dbg(LYNX_DBG_WLAN_CFG, "%s: not supported mgmt(0x%x)\n", __func__, key_mgmt);

	/* may need to implement */
#if 0 
	else if (key_mgmt == 0x00409600) {
		if (vif->auth_mode == WPA_AUTH)
			vif->auth_mode = WPA_AUTH_CCKM;
		else if (vif->auth_mode == WPA2_AUTH)
			vif->auth_mode = WPA2_AUTH_CCKM;
	} else if (key_mgmt != WLAN_AKM_SUITE_8021X) {
		vif->auth_mode = NONE_AUTH;
	}
#endif

	bss->auth_capability = cpu_to_be32(auth_capability);
}


#ifdef CONFIG_ANDROID
	static int sta_1st_scan =1 ;
#endif


static int lynx_cfg80211_setup_scan(struct lynx_vif *vif, struct cfg80211_scan_request *request, u16 interval)
{
    struct cfg80211_sched_scan_request *sscan_request=(struct cfg80211_sched_scan_request *)request;
    struct ieee80211_channel **channels;
    const u8 *ie;
    u8 *ssid=NULL;
    u32 ie_len, ssid_len=0, n_channels;
    u32 channel_map=0;
    u8 i;

    if(request == NULL) /* stop scan */
    {
        goto begin_scan;
    }
	/* FIXME: wci_evt_timer cause kernel exception */
#if 0
	else
	{
		/* don't scan in the connect progress */
		if(test_bit(CONNECT_PEND, &vif->flags))
			return -EBUSY;
	}
#endif

    if(interval)    /* schedule scan */
    {
        ie = sscan_request->ie;
        ie_len = sscan_request->ie_len;
		if((request->n_ssids) && (request->ssids))
		{
			ssid = sscan_request->ssids->ssid;
			ssid_len = sscan_request->ssids->ssid_len;
		}
        channels = sscan_request->channels;
        n_channels = sscan_request->n_channels;
    }
    else    /* normal scan */
    {
        ie = request->ie;
        ie_len = request->ie_len;
		if((request->n_ssids) && (request->ssids))
		{
			ssid = request->ssids->ssid;
			ssid_len = request->ssids->ssid_len;
		}
        channels = request->channels;
        n_channels = request->n_channels;
    }
    if((n_channels > 0) && (n_channels <= WCI_MAX_CHANNELS)) 
    {
        for(i=0; i < n_channels; i++)
            channel_map |= lynx_ch_map(channels[i]->center_freq);
    }

#ifdef CONFIG_ANDROID
	if(sta_1st_scan && ( vif->wdev.iftype  == NL80211_IFTYPE_STATION )){
		sta_1st_scan =0;
		channel_map =0;
		channel_map |= lynx_ch_map(2412);
		channel_map |= lynx_ch_map(2437);
		channel_map |= lynx_ch_map(2462);
	}
#endif

begin_scan:
    /* channel_maps == 0 : stop scan */ 

#ifdef CONFIG_HOST_P2P
    if( vif->wdev.iftype  == NL80211_IFTYPE_P2P_CLIENT )
    {
	ssid = NULL;
	ssid_len = 0;
    }
#endif /*CONFIG_HOST_P2P*/

    return lynx_wci_beginscan_cmd(vif, channel_map, interval, ssid, ssid_len);
}

static int lynx_cfg80211_sscan_disable(struct lynx_vif *vif)
{
    /* FIXME: not implement schedule scan now */

    struct lynx *lnx = vif->lnx;
    int ret = 0;

    if(test_and_clear_bit(SCHED_SCANNING, &vif->flags))
    {
        cfg80211_sched_scan_stopped(lnx->wiphy, 0);

        ret = lynx_cfg80211_setup_scan(vif, 0, 0);
    }

    return ret;
}

static bool lynx_cfg80211_ready(struct lynx_vif *vif)
{
    if(!test_bit(WLAN_ENABLED, &vif->flags)) {
        lynx_dbg(LYNX_DBG_WLAN_CFG, "wlan disable\n");
        return false;
    }

    return true;
}

void lynx_disconnect(struct lynx_vif *vif, u16 reason_code)
{
    int sta_idx=0;

	if((sta_idx = sta_map2idx(vif, NULL)) < 0)
	{
		lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): can't find the sta idx\n", __FUNCTION__);
		return; 
	}

	if((vif->nw_type == VIF_STA_MODE) || (vif->nw_type == VIF_P2P_CLIENT_MODE))
		mlme_schedule_sync_link_st(vif, SYNC_LINK_STOP, 0);

	lynx_wci_disconnect_cmd(vif, sta_idx, reason_code);
}

void lynx_cfg80211_disconnect_event(struct lynx_vif *vif, u8 *bssid, 
                                    u8 reason_code, u16 status_code)
{
    /* reason_code, ex: WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY  */
    /* status_code, ex: WLAN_STATUS_UNSPECIFIED_FAILURE         */
    lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): reason=%d, status=%d\n", __FUNCTION__, reason_code, status_code);
    
    if(vif->scan_req)
    {
	info.aborted = true;
        cfg80211_scan_done(vif->scan_req, &info);
        vif->scan_req = NULL;
    }

    if(test_bit(CONNECT_PEND, &vif->flags)) 
    {
        cfg80211_connect_result(vif->ndev, bssid, NULL, 0, NULL, 0,
                                WLAN_STATUS_UNSPECIFIED_FAILURE,
                                CFG80211_GFP);
    } 
    else if(test_bit(CONNECTED, &vif->flags)) 
    {
		if(vif->nw_type == VIF_STA_MODE)
		{
			cancel_delayed_work(&vif->sync_work);
		}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
        cfg80211_disconnected(vif->ndev, status_code, NULL, 0, true, CFG80211_GFP);
#else
        cfg80211_disconnected(vif->ndev, status_code, NULL, 0, CFG80211_GFP);
#endif
    }
}

void cleanup_sta_list(struct lynx_vif *vif)
{
    struct lynx *lnx = vif->lnx;
	struct lynx_sta *sta=NULL;
	int i=0;
	
	for(i=0; i<LYNX_STA_MAX_NUM; i++)
	{
		sta = &lnx->sta_list[i];
		if((sta->flags & LYNX_STA_VALID) && (sta->vif == vif))
		{
			cfg80211_del_sta(vif->ndev, sta->addr, CFG80211_GFP);
			memset(sta, 0, sizeof(struct lynx_sta));
		}
	}
	
	vif->sta_idx_map = 0;
}

void lynx_disconnect_event(struct lynx_vif *vif, u8 *bssid, u8 reason_code, u16 status_code)
{
    if(vif->nw_type == VIF_AP_MODE)
    {
#if 0
        if (!is_broadcast_ether_addr(bssid)) 
        {
            /* send event to application */
            cfg80211_del_sta(vif->ndev, bssid, CFG80211_GFP);
        }
#endif
		/* Note: we expect the VIF_AP_MODE will flush all sta */
		cleanup_sta_list(vif);

        goto out;
    }

    lynx_cfg80211_disconnect_event(vif, bssid, reason_code, status_code);

    memset(vif->req_bssid, 0, ETH_ALEN);

out:
    /* FIXME: stop all sta function in driver (ex: keep alive) */

    return;
}

void lynx_cfg80211_vif_stop(struct lynx_vif *vif)
{
    static u8 bcast_mac[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): vif=0x%x, flags=0x%x\n", __FUNCTION__, (void *) vif, vif->flags);

    if(!test_bit(DEV_ATTACHED, &vif->flags))
    {
        lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): DEV_ATTACHED has cleared !!!!\n", __FUNCTION__);
        return;
    }
    
    clear_bit(DEV_ATTACHED, &vif->flags);

#ifdef CONFIG_HOST_P2P
	if( vif->wdev.iftype  != NL80211_IFTYPE_P2P_DEVICE )
#endif /*CONFIG_HOST_P2P*/
	{
		netif_stop_queue(vif->ndev);
		netif_carrier_off(vif->ndev);

		if(test_bit(CONNECTED, &vif->flags) || test_bit(CONNECT_PEND, &vif->flags))
		{
			lynx_disconnect_event(vif, (vif->nw_type == VIF_AP_MODE) ? bcast_mac : vif->vif_addr, 
			WLAN_REASON_UNSPECIFIED, WLAN_STATUS_UNSPECIFIED_FAILURE);

			/* Inform DUT to remove bss */
			lynx_disconnect(vif, WLAN_REASON_UNSPECIFIED);
			clear_bit(CONNECT_PEND, &vif->flags);
			clear_bit(CONNECTED, &vif->flags);
		}
	}

	/*Stop scan action*/
	if(vif->scan_req)
	{
		info.aborted = true;
		cfg80211_scan_done(vif->scan_req, &info);
		vif->scan_req = NULL;
	}

	cancel_delayed_work(&vif->sync_work);
}

static int lynx_nliftype_to_drv_iftype(enum nl80211_iftype type)
{
	int nw_type = -ENOTSUPP;

    /*Some iftype not implement*/
    switch (type) 
    {
#ifdef CONFIG_SUPPORT_MONITOR_MODE
       case NL80211_IFTYPE_MONITOR:
#endif /*CONFIG_SUPPORT_MONITOR_MODE */
        case NL80211_IFTYPE_STATION:
            nw_type = VIF_STA_MODE;
            break;
        case NL80211_IFTYPE_ADHOC:
            nw_type = VIF_IBSS_MODE;
            break;
        case NL80211_IFTYPE_AP:
            nw_type = VIF_AP_MODE;
            break;
        case NL80211_IFTYPE_WDS:
            nw_type = VIF_WDS_MODE;
            break;
#ifdef CONFIG_HOST_P2P
        case NL80211_IFTYPE_P2P_DEVICE:
        case NL80211_IFTYPE_P2P_CLIENT:
            nw_type = VIF_STA_MODE;
            break;
        case NL80211_IFTYPE_P2P_GO:
            nw_type = VIF_AP_MODE;
            break;
#endif /*CONFIG_HOST_P2P*/
        default:
            lynx_dbg(LYNX_DBG_WLAN_CFG, "invalid interface type %u\n", type);
    }

    return nw_type;
}

static int lynx_is_valid_iftype(struct lynx *lnx, enum nl80211_iftype type, int *nw_type)
{
    int i;

    if((*nw_type = lynx_nliftype_to_drv_iftype(type)) < 0)
	{
		i = -1;
	}
	else if(lnx->ibss_existing || ((type == NL80211_IFTYPE_ADHOC) && lnx->num_vif))
	{
    	/* IBSS is allowed in the only one vif case. */
        i = -1;
	}
	else
	{
		/* FIXME: Does any type has special bss_idx request? */
		for (i = 0; i < lnx->vif_max; i++) 
		{
			if ((lnx->avail_idx_map) & BIT(i)) 
			{
				break;;
			}
		}

		if(i >= lnx->vif_max)
			i = -1;
	}

    return i;
}

void lynx_cfg80211_vif_cleanup(struct lynx_vif *vif)
{
    struct lynx *lnx = vif->lnx;


    lnx->avail_idx_map |= BIT(vif->fw_vif_idx);
    
    if(vif->nw_type == VIF_IBSS_MODE)
        lnx->ibss_existing = 0;

#ifdef CONFIG_HOST_P2P
    if( vif->wdev.iftype  != NL80211_IFTYPE_P2P_DEVICE )
#endif /*CONFIG_HOST_P2P */
    {
        lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): vif=%p, ndev=%p(%s)\n", __FUNCTION__, (void *)vif, 
            (void *)vif->ndev, vif->ndev->name);    
        unregister_netdevice(vif->ndev);
    }

#ifdef CONFIG_HOST_P2P
	if( vif->wdev.iftype  == NL80211_IFTYPE_P2P_DEVICE )
	{
		lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): vif=%p\n", __FUNCTION__, (void *)vif);    
		cfg80211_unregister_wdev(&(vif->wdev));
		free_netdev(vif->ndev);
	}
#endif /*CONFIG_HOST_P2P */

    lnx->num_vif--;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
static struct wireless_dev *lynx_cfg80211_add_iface(struct wiphy *wiphy, const char *name,
    unsigned char name_assign_type, enum nl80211_iftype type, struct vif_params *params)
#else
static struct wireless_dev *lynx_cfg80211_add_iface(struct wiphy *wiphy, const char *name,
                            enum nl80211_iftype type, u32 *flags, struct vif_params *params)
#endif
{
    struct lynx *lnx = wiphy_priv(wiphy);
    struct wireless_dev *wdev;
    int if_idx, nw_type;
	//lynx_dbg(LYNX_DBG_WLAN_CFG, "lynx_cfg80211_add_iface()\n");
	
	if(mp_test_firm)
		return ERR_PTR(-EINVAL);

    if(lnx->num_vif == lnx->vif_max)
    {
        lynx_dbg(LYNX_DBG_WLAN_CFG, "Reached maximum number of supported vif\n");
        return ERR_PTR(-EINVAL);
    }

#ifdef CONFIG_SUPPORT_MONITOR_MODE
	if((type == NL80211_IFTYPE_MONITOR) && (lnx->num_vif != 0)	)
	{
		/* Monitor mode  is allowed in the only one vif case. */
		lynx_dbg(LYNX_DBG_ERR, "%s(): Support  monitor mode when only a interface \n",__FUNCTION__);
		return ERR_PTR(-ENFILE);
	}
#endif /*CONFIG_SUPPORT_MONITOR_MODE */

#ifdef CONFIG_HOST_P2P
	if(( type == NL80211_IFTYPE_P2P_DEVICE ) &&
		( NULL != lynx_get_p2p_vif(lnx,0) ))
	{
		/* P2P dev interface num is allowed as 1*/
		lynx_dbg(LYNX_DBG_ERR, "%s(): p2p control interface already exist ! \n",__FUNCTION__);
		return ERR_PTR(-ENFILE);
	}
#endif /*CONFIG_HOST_P2P*/

    if((if_idx = lynx_is_valid_iftype(lnx, type, &nw_type)) < 0)
    {
        lynx_dbg(LYNX_DBG_WLAN_CFG, "Not a supported interface type\n");
        return ERR_PTR(-EINVAL);
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
    wdev = (struct wireless_dev *)lynx_interface_add(lnx, name, name_assign_type, type, if_idx, nw_type);
#else
	/* name_assign_type = 0 : NET_NAME_UNKNOWN */
    wdev = (struct wireless_dev *)lynx_interface_add(lnx, name, 0, type, if_idx, nw_type);
#endif

    if(!wdev)
        return ERR_PTR(-ENOMEM);

#ifdef CONFIG_HOST_P2P
    if( type  != NL80211_IFTYPE_P2P_DEVICE )
#endif /*CONFIG_HOST_P2P */
    {
    lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): wdev=%p, ndev=%p\n", __FUNCTION__, 
                (void *)wdev, (void *) wdev->netdev);
    }
    return wdev;
}

static int lynx_cfg80211_del_iface(struct wiphy *wiphy, struct wireless_dev *wdev)
{
	struct lynx *lnx = wiphy_priv(wiphy);
	struct lynx_vif *vif = lynx_vif_from_wdev(wdev);
	int ret = 0;
	//lynx_dbg(LYNX_DBG_WLAN_CFG, "lynx_cfg80211_del_iface()\n");
	
	if(mp_test_firm)
		return -EINVAL;


	if (lnx) {

		if (test_bit(DESTROY_IN_PROGRESS, &lnx->flag)) {
			lynx_dbg(LYNX_DBG_WLAN_CFG, "DESTROY_IN_PROGRESS\n");
			return 0;
		}

#if !defined(CONFIG_LYNX_WM_MANAGER)
#if defined(CONFIG_LYNX_OS_LINUX)
		if(true == lynx_remove_vif(lnx ,vif)){
			lynx_cfg80211_vif_stop(vif);
			//if(!test_bit(DEV_ATTACHED, &vif->flags))
			//{
				lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): DEV_ATTACHED has cleared !!!!\n", __FUNCTION__);
			//}
			//else
			//{
				lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): wci_detach_cmd !!!!\n", __FUNCTION__);
				lynx_wci_detach_cmd(vif);
				lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): interface_del !!!!\n", __FUNCTION__);
				ret =lynx_interface_del(lnx, vif);
				lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): interface_del done !!!!\n", __FUNCTION__);
			//}
		}
#endif
#endif
	}
	return ret;
}

static int lynx_cfg80211_change_iface(struct wiphy *wiphy, struct net_device *ndev,
                  enum nl80211_iftype type, struct vif_params *params)
{
    struct lynx_vif *vif = netdev_priv(ndev);
	struct wm_bss *bss = &vif->bss;
    int new_mode = 0;
    enum nl80211_iftype old_type;
#ifdef CONFIG_SUPPORT_MONITOR_MODE
    struct lynx *lnx = vif->lnx;
#endif

#ifdef CONFIG_HOST_P2P
    u32 new_flag;
#endif /*CONFIG_HOST_P2P*/
	if(mp_test_firm)
		return -EINVAL;

    lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): ndev=%p, original type=%d, new type=%d\n", __FUNCTION__, 
                (void *) ndev, vif->wdev.iftype, type);

#ifdef CONFIG_SUPPORT_MONITOR_MODE
    if((type == NL80211_IFTYPE_MONITOR) && (lnx->num_vif >1)) 
    {
		lynx_dbg(LYNX_DBG_ERR, "%s(): Support monitor mode when only a interface \n",__FUNCTION__);
		return -ENFILE;
    }
#endif /*CONFIG_SUPPORT_MONITOR_MODE */

#ifdef CONFIG_HOST_P2P
	if( type == NL80211_IFTYPE_P2P_DEVICE ){
		lynx_dbg(LYNX_DBG_ERR, "%s(): Not support to switch a normal interface to p2p control interface \n",__FUNCTION__);
		return -EPERM  ;
	}

#ifdef CONFIG_UNIT_TEST
	unittest_bss_compare(vif, __FUNCTION__);
#endif

	if( type == NL80211_IFTYPE_P2P_CLIENT ){
		new_flag = be32_to_cpu(bss->flag);
		new_flag &= ~BSS_FLG_P2P_GO_MODE;
		new_flag |= BSS_FLG_P2P_CLIENT_MODE;
		bss->flag = cpu_to_be32(new_flag);
	}else if( type == NL80211_IFTYPE_P2P_GO ){
		new_flag = be32_to_cpu(bss->flag);
		new_flag &= ~BSS_FLG_P2P_CLIENT_MODE;
		new_flag |= BSS_FLG_P2P_GO_MODE;
		bss->flag = cpu_to_be32(new_flag);
	}
#endif /*CONFIG_HOST_P2P*/

#ifdef CONFIG_HOST_ROAMING
	if( type == NL80211_IFTYPE_STATION ){
		new_flag = be32_to_cpu(bss->flag);
		new_flag |= BSS_FLG_BEACON_REPORT;
		bss->flag = cpu_to_be32(new_flag);
	}
#endif /*CONFIG_HOST_ROAMING*/

    if((new_mode = lynx_nliftype_to_drv_iftype(type)) < 0)
        return new_mode;
    
    if(vif->nw_type != new_mode)
    {
        /* FIXME: trigger disconnect. reason_code=4? */
	if(( type != NL80211_IFTYPE_P2P_CLIENT ) && ( type != NL80211_IFTYPE_P2P_GO ))
        	lynx_cfg80211_disconnect(wiphy, ndev, WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY);

        vif->nw_type = new_mode;
		bss->role = new_mode;	
   		cpu_to_be16s(&(bss->role));
	
		if(new_mode == VIF_STA_MODE)
			bss->tsf_idx = 1;
		else
			bss->tsf_idx = 0;
    }
    old_type = vif->wdev.iftype ;
    vif->wdev.iftype = type;

	if(is_valid_ether_addr(params->macaddr))
	{
		memcpy(vif->vif_addr, params->macaddr, ETH_ALEN);
		memcpy(bss->myaddr, params->macaddr, ETH_ALEN);
	}

    if(vif->nw_type == VIF_IBSS_MODE)
	{
		bss->phy_cap = AP_CAP_11B;
		bss->slottime = SLOTTIME_20US;
   		cpu_to_be16s(&(bss->slottime));
		//bss->atim_window = ?
	}
    
	bss->bss_desc = vif->fw_vif_idx;

	lynx_wci_update_vif_cmd(vif);

#ifdef CONFIG_UNIT_TEST
	unittest_bss_backup(vif, __FUNCTION__);
#endif
	
#ifdef CONFIG_SUPPORT_MONITOR_MODE
	if( vif->wdev.iftype == NL80211_IFTYPE_MONITOR ){
		ndev->type = ARPHRD_IEEE80211_RADIOTAP;
		wlan_init_monitor(vif);
		vif->lnx->monitor_level = 0;
	}else{
		if( old_type == NL80211_IFTYPE_MONITOR ){
			wlan_stop_monitor(vif);
			lynx_dbg(LYNX_DBG_ERR, "%s():Back to  nomal mode: \n", __FUNCTION__);
			ndev->type =  ARPHRD_ETHER; ;
		}
	}
#endif /*CONFIG_SUPPORT_MONITOR_MODE */
    return 0;
}

static int lynx_cfg80211_scan(struct wiphy *wiphy, struct cfg80211_scan_request *request)
{
    struct lynx_vif *vif = lynx_vif_from_wdev(request->wdev);
    struct lynx *lnx = wiphy_priv(wiphy);
    int ret = 0;
#ifdef CONFIG_HOST_WPS
    bool p2p_exist = 0 ; 
#endif /*CONFIG_HOST_WPS*/
	
#ifdef CONFIG_HOST_P2P
	int i;
	struct lynx_sta *sta=NULL;
	struct lynx_vif *p2p_data_vif = NULL;
	struct lynx_vif *sta_vif = NULL;
	
	if(mp_test_firm)
		return -EINVAL;

	p2p_data_vif = lynx_get_p2p_data_vif(lnx,0);
	/*  when p2p data interface connected,not allow other interfaces to scan for better p2p throuput*/
	if (( NULL != p2p_data_vif ) &&
		( vif != p2p_data_vif )){

		if( p2p_data_vif->wdev.iftype == NL80211_IFTYPE_P2P_CLIENT ) {
			if(test_bit(CONNECTED, &p2p_data_vif->flags) || test_bit(CONNECT_PEND, &p2p_data_vif->flags)){
				//lynx_dbg(LYNX_DBG_WLAN_CFG, "Not allow scan when p2p GC interface connected!\n");
				return -EBUSY;
			}
		}else if( p2p_data_vif->wdev.iftype == NL80211_IFTYPE_P2P_GO ) {
			for(i=0; i<LYNX_STA_MAX_NUM; i++)
			{
				sta = &lnx->sta_list[i];
				if(sta->vif == p2p_data_vif)
				{
					//lynx_dbg(LYNX_DBG_WLAN_CFG, "Not allow scan when p2p GO interface linked with peer!\n");
					return -EBUSY;
				}
			}	
			//lynx_dbg(LYNX_DBG_WLAN_CFG, "Not allow scan when p2p GO interface created !\n");
			return -EBUSY;
		}
	}

#endif /*CONFIG_HOST_P2P*/
	
#ifdef CONFIG_HOST_P2P
	if( vif->wdev.iftype	!= NL80211_IFTYPE_P2P_DEVICE )
#endif /*CONFIG_HOST_P2P*/
	{
		if (!lynx_cfg80211_ready(vif))
			return -EIO;
	}

	/*If other interface is scanning .*/
	if(NULL != lynx_get_scanning_vif(lnx,0)){
		lynx_dbg(LYNX_DBG_WLAN_CFG, "Not allow scan when other interface scanning!\n");
		return -EBUSY;
	}

    /*Disable schedule scan*/
    lynx_cfg80211_sscan_disable(vif);

#ifdef CONFIG_HOST_WPS
#ifdef CONFIG_HOST_P2P
		if( ( vif->wdev.iftype  == NL80211_IFTYPE_P2P_DEVICE ) ||
		     ( vif->wdev.iftype  == NL80211_IFTYPE_P2P_CLIENT ) ){
			p2p_exist = 1 ;
		}
#endif /*CONFIG_HOST_P2P*/
    // if ((NULL != request->ie)&&( 0 <  request->ie_len)){
    if (1){
            /*Add WPS/P2p extra information element(s) to add into Probe Request frame */
            ret = lynx_cfg80211_set_wps_p2p_ies( vif,(u8) WCI_FRAME_PROBE_REQ,
                                                            request->ie,request->ie_len,p2p_exist);
            //lynx_dbg(LYNX_DBG_WLAN_CFG, "Set Probe Request Extra IEs\n");
            if (0 != ret){//when error
                    lynx_dbg(LYNX_DBG_WLAN_CFG, "Error:Set Probe Reuest Extra IE Error!\n");
                    return ret;
            }
    }
    else{
        lynx_dbg(LYNX_DBG_WLAN_CFG, "Error:Receive AP set NULL Probe Reuest Extra IE\n");
    }

#ifdef CONFIG_HOST_P2P
	/* when station interface connected,not allow p2p dev interface to scan ; (block after set ies)*/
	if( vif->wdev.iftype == NL80211_IFTYPE_P2P_DEVICE ){
		sta_vif = lynx_get_sta_vif(lnx,0);
		if( NULL != sta_vif ){
			if(test_bit(CONNECTED, &sta_vif->flags) || test_bit(CONNECT_PEND, &sta_vif->flags)){
				//lynx_dbg(LYNX_DBG_WLAN_CFG, "Block p2pdev scan when sta connected!\n");
				return -EBUSY;
			}
		}
	}
#endif /*CONFIG_HOST_P2P*/
#endif /*CONFIG_HOST_WPS*/

    /* FIXME: implement the BBS filter? (allow the specific BSS beacon/info) */

    ret = lynx_cfg80211_setup_scan(vif, request, 0);

	if(!ret)
	{
        vif->scan_req = request;
#ifdef CONFIG_TIMER_HANDLER
		vif->scan_start_time = jiffies;
#endif
	}
    
    return ret;
}

static int lynx_cfg80211_sscan_start(struct wiphy *wiphy, struct net_device *dev,
            					struct cfg80211_sched_scan_request *request)
{
    struct lynx *lnx = wiphy_priv(wiphy);
    struct lynx_vif *vif = netdev_priv(dev);
    u16 interval;
    int ret=0;

	if(mp_test_firm)
		return -EINVAL;

    if(!(test_bit(WCI_READY, &lnx->flag)))
        return -EIO;

    if(test_bit(CONNECTED, &vif->flags) || test_bit(CONNECT_PEND, &vif->flags))
        return -EBUSY;

    if(vif->scan_req)
    {
	info.aborted = true;
        cfg80211_scan_done(vif->scan_req, &info);
        vif->scan_req = NULL;
    }
    
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
	if(request->n_scan_plans >= 1)
    	interval = max_t(u16, 1, request->scan_plans[0].interval);
	else
		return -EINVAL;
#else
    interval = max_t(u16, 1, request->interval / 1000); /* unit = sec */
#endif

    ret = lynx_cfg80211_setup_scan(vif, (struct cfg80211_scan_request *)request, interval);

    if (!ret)
        set_bit(SCHED_SCANNING, &vif->flags);

    return ret;
}

static int lynx_cfg80211_sscan_stop(struct wiphy *wiphy,
                      struct net_device *dev, u64 reqid)
{
    struct lynx_vif *vif = netdev_priv(dev);
	if(mp_test_firm)
		return -EINVAL;

    return  lynx_cfg80211_sscan_disable(vif);
}

static int lynx_cfg80211_connect(struct wiphy *wiphy, struct net_device *dev,
                				struct cfg80211_connect_params *sme)
{
    struct lynx *lnx = lynx_priv(dev);
    struct lynx_vif *vif = netdev_priv(dev);
    struct wm_bss *bss = &vif->bss;
	struct cfg80211_bss *cfg_bss=NULL;
	const struct cfg80211_bss_ies *beacon_ies=NULL;	
	struct wlan_ie_info info_ap, info_wpa;
    int status=-EINVAL;
#ifdef CONFIG_HOST_WPS
    bool p2p_exist = 0 ; 
#endif /*CONFIG_HOST_WPS*/
	unsigned int p_cipher=0, g_cipher=0;

    /*FIXME:Why sme->ie_len is zero*/
	if(mp_test_firm)
		return -EINVAL;

    lynx_dbg(LYNX_DBG_WLAN_CFG, "Enter CONNECT\n");

    lynx_cfg80211_sscan_disable(vif);

    if (!lynx_cfg80211_ready(vif))
        return -EIO;

    if (test_bit(DESTROY_IN_PROGRESS, &lnx->flag)) {
        lynx_dbg(LYNX_DBG_WLAN_CFG, "DESTROY_IN_PROGRESS\n");
        return -EBUSY;
    }

    /*FIXME:Use semaphores*/
    if(down_interruptible(&lnx->sem)) {
        lynx_dbg(LYNX_DBG_WLAN_CFG, "Busy, destroy in progress\n");
        return -ERESTARTSYS;
    }

#ifdef CONFIG_UNIT_TEST
	unittest_bss_compare(vif, __FUNCTION__);
#endif

#ifdef CONFIG_HOST_WPS
#ifdef CONFIG_HOST_P2P
    if( vif->wdev.iftype  == NL80211_IFTYPE_P2P_CLIENT ){
        p2p_exist = 1 ;
    }
#endif /*CONFIG_HOST_P2P*/
    /*Add WPS/P2p extra information element(s) to add into Assoc Request frame */
    lynx_cfg80211_set_wps_p2p_ies( vif,(u8) WCI_FRAME_ASSOC_REQ,sme->ie,sme->ie_len,p2p_exist);
#endif /*CONFIG_HOST_WPS*/

	if(sme->ssid)
	{
		/* FIXME: should return error, if ssid_len > 32? */
		if(sme->ssid_len > 32)
		{
			lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): ssid is too long(%d)\n", __FUNCTION__, sme->ssid_len);
			goto out;
		}

		bss->ssid_len = sme->ssid_len;
		memcpy(bss->ssid, sme->ssid, bss->ssid_len);
    
		/* FIXME: remove the reduncy fields in the vif */
		vif->ssid_len = sme->ssid_len;
    	memcpy(vif->ssid, sme->ssid, sme->ssid_len);
	}
    
	if (sme->bssid && !is_broadcast_ether_addr(sme->bssid))
	{
		memcpy(bss->bssid, sme->bssid, WLAN_ADDR_LEN);
        memcpy(vif->req_bssid, sme->bssid, WLAN_ADDR_LEN);
	}

    if(sme->channel)
    {
        /* FIXME: vif->ch_hint should been removed & where is the bandwidth info? */
        vif->ch_hint = sme->channel->center_freq;
        /* FIXME: other vif trigger disconnect? and change channel? */
        lnx->channel = lynx_freq2ch(sme->channel->center_freq);
		lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): lnx->channel=%d, sme->channel->center_freq=%d\n", 
							__FUNCTION__, lnx->channel, sme->channel->center_freq);
    }
	else
	{
		lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): sme without channel info\n", __FUNCTION__);
	}

	lynx_clear_cipher(vif);

    if((status = lynx_set_auth_type(vif, sme->auth_type)))
        goto out;

    lynx_set_wpa_version(vif, sme->crypto.wpa_versions);

    if(sme->crypto.n_ciphers_pairwise)
        p_cipher = lynx_set_cipher(vif, sme->crypto.ciphers_pairwise[0], true);
    else
        p_cipher = lynx_set_cipher(vif, 0, true);

    g_cipher = lynx_set_cipher(vif, sme->crypto.cipher_group, false);

	printk("%s(): vif->prwise_crypto=0x%x, vif->grp_crypto=%x\n", __FUNCTION__, vif->prwise_crypto, vif->grp_crypto);	

	if (sme->crypto.n_akm_suites)
		lynx_set_key_mgmt(vif, sme->crypto.akm_suites[0]);	

    if ((sme->key_len) &&
        (vif->auth_mode == NONE_AUTH) &&
        (vif->prwise_crypto & (BIT(CIPHER_TYPE_WEP40) | BIT(CIPHER_TYPE_WEP104))))
	{
		struct lynx_key *key = NULL;

		if(sme->key_idx > WCI_MAX_KEY_INDEX) 
		{
			lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(ERROR): key index %d out of bounds\n", __FUNCTION__, sme->key_idx);
			status = -ENOENT;
			goto out;
		}

        key = &vif->keys[sme->key_idx];
		if(sme->key_len <= WLAN_MAX_KEY_LEN)
		{
			key->key_len = sme->key_len;
			memcpy(key->key, sme->key, key->key_len);
			key->cipher = p_cipher;
			key->is_txkey = 1;
			/* FIXME: WEP key type */
			key->key_type = KEY_TYPE_GLOBAL_KEY;
			vif->def_txkey_index = sme->key_idx;

			lynx_wci_addkey_cmd(vif, NULL, key, sme->key_idx, key->key_type);
		}
		else
		{
			lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(ERROR): key len is too long %d\n", sme->key_len);
		}
        /* FIXME: is need to trigger unicast key setting? */
    }


	if(vif->nw_type == VIF_STA_MODE)
	{
		cfg_bss = cfg80211_get_bss(lnx->wiphy, NULL, vif->req_bssid,
					   vif->ssid, vif->ssid_len, 
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
					   IEEE80211_BSS_TYPE_ESS, IEEE80211_PRIVACY_ANY
#else
					   WLAN_CAPABILITY_ESS, WLAN_CAPABILITY_ESS
#endif
						);

		if(cfg_bss == NULL)
		{
			lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(ERROR): no matched bss in the cfg80211\n");
			goto out;
		}

		if(cfg_bss->beacon_ies)
			beacon_ies = rcu_dereference(cfg_bss->beacon_ies);
		else if(cfg_bss->proberesp_ies)
			beacon_ies = rcu_dereference(cfg_bss->proberesp_ies);
		
		if(beacon_ies)
		{
			memset(&info_ap, 0, sizeof(struct wlan_ie_info));
			lynx_parse_ie(beacon_ies->data, beacon_ies->len, &info_ap);
			vif->bss.phy_cap = info_ap.phy_cap;
		}
	}

	/* avoid that WEP/TKIP disallowed in the 11N, maybe should parse the phy_cap from IEs */
	if((vif->prwise_crypto & (BIT(CIPHER_TYPE_WEP40)|BIT(CIPHER_TYPE_WEP104)|BIT(CIPHER_TYPE_TKIP))))
	{
		vif->bss.phy_cap &= ~AP_CAP_11N;
	}

	vif->bss.bss_desc = vif->fw_vif_idx;
	lynx_wci_update_vif_cmd(vif);

#ifdef CONFIG_UNIT_TEST
	unittest_bss_backup(vif, __FUNCTION__);
#endif	// CONFIG_UNIT_TEST

	if(sme->ie_len && sme->ie)
	{
		info_wpa.rsn_ie = NULL;
		info_wpa.wpa_ie = NULL;

		lynx_parse_ie(sme->ie, sme->ie_len, &info_wpa);
		info_ap.rsn_ie = info_wpa.rsn_ie;
		info_ap.wpa_ie = info_wpa.wpa_ie;
	}

    /* FIXME:   
        if(test_bit(CONNECTED, &vif->flags) && (ssid is differnet to the connected BSS))
            should trigger disconnect event (not detach vif)
    */

    status = lynx_wci_connect_cmd(vif, (char *)sme->bssid, (char *)sme->ssid, sme->ssid_len, &info_ap);
    if(!status)
    {
        /* FIXME: lynx-2.0 supports back ground period scan ? */
        if(sme->bg_scan_period == 0) {
            /* disable background scan if period is 0 */
            sme->bg_scan_period = 0xffff;
        } else if (sme->bg_scan_period == -1) {
            /* configure default value if not specified */
            sme->bg_scan_period = DEFAULT_BG_SCAN_PERIOD;
        }
    }

out:
    /* FIXME: Is there WPA PSK information? */
    up(&lnx->sem);

    if (status == -EINVAL) {
        memset(vif->ssid, 0, sizeof(vif->ssid));
        vif->ssid_len = 0;
        lynx_dbg(LYNX_DBG_WLAN_CFG, "invalid request\n");
        return -ENOENT;
    } else if (status) {
        lynx_dbg(LYNX_DBG_WLAN_CFG, "wci connect cmd failed\n");
        return -EIO;
    }

    /* FIXME: check the connect return evt to set lnx->ibss_existing */

    set_bit(CONNECT_PEND, &vif->flags);
    clear_bit(DATAPATH_EN, &vif->flags);

    return 0;
}

static int lynx_cfg80211_set_wiphy_params(struct wiphy *wiphy, u32 changed)
{
    struct lynx *lnx = wiphy_priv(wiphy);
    struct lynx_vif *vif = lynx_vif_first(lnx);
	if(mp_test_firm)
		return -EINVAL;

	lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): changed 0x%x\n", __FUNCTION__, changed);

	if((!vif) || (!lynx_cfg80211_ready(vif)))
		return -EIO;

	if(changed & WIPHY_PARAM_RTS_THRESHOLD) 
	{
		if(lynx_wci_set_rts_cmd(vif, wiphy->rts_threshold))
			return -EIO;
	}

	return 0;
}

#ifdef CONFIG_HOST_WPS
static bool  lynx_cfg80211_is_p2p_ie(const u8 *pos)
{
	return pos[0] == WLAN_EID_VENDOR_SPECIFIC && pos[1] >= 4 &&
           pos[2] == 0x50 && pos[3] == 0x6f &&
           pos[4] == 0x9a && pos[5] == 0x09;
}

static bool  lynx_cfg80211_is_wfd_ie(const u8 *pos)
{
	return pos[0] == WLAN_EID_VENDOR_SPECIFIC && pos[1] >= 4 &&
           pos[2] == 0x50 && pos[3] == 0x6f &&
           pos[4] == 0x9a && pos[5] == 0x0a;
}

static bool  lynx_cfg80211_is_wps_ie(const u8 *pos)
{
	return pos[0] == WLAN_EID_VENDOR_SPECIFIC && pos[1] >= 4 &&
           pos[2] == 0x00 && pos[3] == 0x50&&
           pos[4] == 0xf2 && pos[5] == 0x04;
}

int lynx_cfg80211_set_wps_p2p_ies(struct lynx_vif *vif,u8 mgmt_type,
                                        const u8 *ies, size_t ies_len,bool p2p_exist)
{
	const u8 *pos;
	u8 *buf = NULL;
	size_t len = 0;
	int ret = -ENOMEM;

	/* Filter out other IE(s) Only send WPS IE from hostapd/wpa_supplicant.*/
	/* if p2p_exist (at p2p mode) is true, send P2P IE at the same time */
	/* if no ies be set,it will clears IE in fw */
	if (ies && ies_len) {
		buf = kmalloc(ies_len, GFP_KERNEL);
		if (buf == NULL)
			return -ENOMEM;
		
		pos = ies;
		while (pos + 1 < ies + ies_len) {
			
				if (pos + 2 + pos[1] > ies + ies_len)
					break;
				
				if ((lynx_cfg80211_is_wps_ie(pos)) ||
					(p2p_exist&&(lynx_cfg80211_is_p2p_ie(pos)) ) ||
					(p2p_exist&&(lynx_cfg80211_is_wfd_ie(pos)) )){
					memcpy(buf + len, pos, 2 + pos[1]);
					len += 2 + pos[1];
				}

				pos += 2 + pos[1];
			}
	}
	ret = lynx_wci_set_mgmt_ie_cmd(vif, buf, (u16) len,mgmt_type);

	if (buf != NULL)
		kfree(buf);
	return ret;
}

static int lynx_cfg80211_set_ies(struct lynx_vif *vif,
                          struct cfg80211_beacon_data *info)
{

	int res =   -ENOMEM;
	bool p2p_exist = 0 ; 

#ifdef CONFIG_HOST_P2P
	if( vif->wdev.iftype  == NL80211_IFTYPE_P2P_GO ){
		p2p_exist = 1 ;
	}
#endif /*CONFIG_HOST_P2P*/

	//if ((NULL != info->beacon_ies)&&( 0 < info->beacon_ies_len)){
	if (1){
		/*Add WPS/P2P extra information element(s) to add into Beacon frames */
		res = lynx_cfg80211_set_wps_p2p_ies(vif, (u8)WCI_FRAME_BEACON,
							info->beacon_ies,info->beacon_ies_len,p2p_exist);
		//lynx_dbg(LYNX_DBG_WLAN_CFG, "Set beacon_ies\n");

		if (res)//when error
			return res;
	}

	/*Add extra information element(s) to add into Probe Response frames */
	/*Filter P2P frame first,add WPS frame only*/
	//if ((NULL != info->proberesp_ies)&&( 0 < info->proberesp_ies_len)){
	if (1){
		res = lynx_cfg80211_set_wps_p2p_ies(vif, (u8)WCI_FRAME_PROBE_RESP,
							info->proberesp_ies,info->proberesp_ies_len,p2p_exist);
		//lynx_dbg(LYNX_DBG_WLAN_CFG, "Set proberesp_ies\n");
		if (res)//when error
			return res;
	}
		    
	/*Add WPS/P2P extra information element(s) to add into (Re)Association Response frames*/
	//if ((NULL != info->assocresp_ies)&&( 0 < info->assocresp_ies_len)){
	if (1){
		res = lynx_cfg80211_set_wps_p2p_ies(vif, (u8)WCI_FRAME_ASSOC_RESP,
							info->assocresp_ies,info->assocresp_ies_len,p2p_exist);
		//lynx_dbg(LYNX_DBG_WLAN_CFG, "Set assocresp_ies\n");
		if (res)//when error
			return res;
	}
	 return 0;//success
}
#endif /*CONFIG_HOST_WPS*/

#ifdef CONFIG_CUST1_PRIV_BEACON_IE
static int  lynx_cfg80211_set_priv_beacon(struct lynx_vif *vif,
                                        const u8 *ies, size_t ies_len)
{
	const u8 *pos;
	u8 *buf = NULL;
	int ret = -ENOMEM;

	if (ies && ies_len) {
		buf = kmalloc(ies_len, GFP_KERNEL);
		if (buf == NULL)
			return -ENOMEM;
		
		pos = ies;
		if (pos + 1 < ies + ies_len) {
				memcpy(buf, pos, ies_len);
		}
	}

	ret = lynx_wci_set_mgmt_ie_cmd(vif, buf,(u16)ies_len,WCI_FRAME_BEACON);
	
	ret = lynx_wci_set_mgmt_ie_cmd(vif, buf,(u16)ies_len,WCI_FRAME_PROBE_RESP);

	if (buf != NULL)
		kfree(buf);
	return ret;
}
#endif
static int lynx_cfg80211_stop_ap(struct wiphy *wiphy, struct net_device *dev)
{

    	struct lynx_vif *vif = netdev_priv(dev);
	
	if((vif->nw_type != VIF_AP_MODE) && (vif->nw_type != VIF_P2P_GO_MODE))
		return -EOPNOTSUPP;
	if (!test_bit(CONNECTED, &vif->flags))
		return -ENOTCONN;
	
	lynx_cfg80211_vif_stop(vif);
	return 0;	
}

#ifdef CONFIG_CUST1_PRIV_BEACON_IE
static s32 find_priv_beacon_ie_data(struct lynx_vif *vif,const u8 *pos,u32 len)
{
	u32 i=0;
	s32 location=-1;
	for( i=0; i<len; i++)
	{
		if(pos[i] == 0xdd) //type
		{
			// skip check len avoid len change
			if(pos[i+2] == 0x30 &&  // OUI[0]
			   pos[i+3] == 0xff &&  // OUI[1]
			   pos[i+4] == 0x83 &&  // OUI[2]
			   pos[i+5] == 0x68)    // OUI[3]
			{
				location = i;
				break;
			}
		}
	}
	return location;
}
#endif
static int lynx_cfg80211_start_ap(struct wiphy *wiphy, struct net_device *dev,
			   struct cfg80211_ap_settings *info)
{
    struct lynx *lnx = lynx_priv(dev);
    struct lynx_vif *vif = netdev_priv(dev);
	struct wm_bss *bss = &vif->bss;
	struct device_configs *fw = &lnx->fw_config;
	struct wlan_ie_info info_ap;
	struct cfg80211_beacon_data *beacon = &info->beacon;
	struct ieee80211_mgmt *mgmt;
	unsigned int beacon_ie_len=0;
	int ret=0, i=0;
	int ssid_len=0;

	if(mp_test_firm)
		return -EINVAL;

    lynx_dbg(LYNX_DBG_WLAN_CFG, "%s():\n", __FUNCTION__);

#ifdef CONFIG_HOST_P2P
	if(( vif->wdev.iftype  == NL80211_IFTYPE_P2P_DEVICE )||
		( vif->wdev.iftype  == NL80211_IFTYPE_P2P_CLIENT ))
		return -EIO;
#endif /*CONFIG_HOST_P2P*/

	if (!lynx_cfg80211_ready(vif))
		return -EIO;

#ifdef CONFIG_UNIT_TEST
	unittest_bss_compare(vif, __FUNCTION__);
#endif

	vif->nw_type = VIF_AP_MODE;
	vif->sta_idx_map = 0;	/* reset the sta_idx_map */

	/* TODO: parser the info from ies to setup the interface capability */
	//res = ath6kl_set_ies(vif, &info->beacon);

	/* reset the security setting */
	lynx_clear_cipher(vif);
	memset(vif->keys, 0, sizeof(struct lynx_key) * (WCI_MAX_KEY_INDEX + 1));
	
#ifdef CONFIG_HOST_WPS
	lynx_set_wps_enable(vif,1);
	ret = lynx_cfg80211_set_ies(vif, &info->beacon);
	if (ret){
		printk(KERN_ALERT "Error: %s  %d \n",__FUNCTION__,__LINE__);   
		return ret;
	}
#endif /*CONFIG_HOST_WPS*/

	bss->beacon_interval = info->beacon_interval;
    cpu_to_be16s(&(bss->beacon_interval));
	bss->dtim_period = info->dtim_period;
	memcpy(bss->bssid, bss->myaddr, WLAN_ADDR_LEN);

	if (info->ssid == NULL)
		return -EINVAL;
	if(info->ssid_len <= 32)
		ssid_len = info->ssid_len;
	else
		ssid_len = 32;

	memcpy(vif->ssid, info->ssid, ssid_len);
	vif->ssid_len = ssid_len;
	vif->ssid[ssid_len] = 0;
	memcpy(bss->ssid, info->ssid, ssid_len);
	bss->ssid_len = ssid_len;
	bss->ssid[ssid_len] = 0;

	ret = lynx_set_auth_type(vif, info->auth_type);
	if (ret)
		return ret;

	for (i = 0; i < info->crypto.n_akm_suites; i++) {
		switch (info->crypto.akm_suites[i]) {
		case WLAN_AKM_SUITE_8021X:
		case WLAN_AKM_SUITE_PSK:
    		lynx_set_wpa_version(vif, info->crypto.wpa_versions);
			lynx_set_key_mgmt(vif, info->crypto.akm_suites[i]);
			break;
		default:
    		lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): not supported mgmt(0x%x)\n", __FUNCTION__, info->crypto.akm_suites[i]);
			break;
		}
	}

    lynx_set_wpa_version(vif, info->crypto.wpa_versions);
	for (i = 0; i < info->crypto.n_ciphers_pairwise; i++) {
		lynx_set_cipher(vif, info->crypto.ciphers_pairwise[i], 1);
	}
	lynx_set_cipher(vif, info->crypto.cipher_group, 0);
	
	printk("%s(): vif->prwise_crypto=0x%x, vif->grp_crypto=%x\n", __FUNCTION__, vif->prwise_crypto, vif->grp_crypto);	

	fw->channel = lnx->channel = lynx_freq2ch(info->chandef.chan->center_freq);	

	/* TODO: hidden ssid */
#if 0
	if (info->hidden_ssid != NL80211_HIDDEN_SSID_NOT_IN_USE)
		hidden = true;
#endif

#ifdef CONFIG_CUST1_PRIV_BEACON_IE
	if(beacon->tail)
	{
		s32 location=0;
		
		location = find_priv_beacon_ie_data(vif,beacon->tail,beacon->tail_len);
		if(location != -1)
		{
			print_hex_dump(KERN_INFO, "start_ap_priv_beacon_ie", DUMP_PREFIX_OFFSET,
					16, 1,
				beacon->tail+location,beacon->tail[location+1]+2, false);
			lynx_cfg80211_set_priv_beacon(vif,beacon->tail+location, beacon->tail[location+1]+2); //+2 add type and length
		}
	}
#endif

	if(beacon->head)
	{
		mgmt = (struct ieee80211_mgmt *)beacon->head;
		beacon_ie_len = ((unsigned long)beacon->tail - (unsigned long)mgmt->u.beacon.variable);
		memset(&info_ap, 0, sizeof(struct wlan_ie_info));
		lynx_parse_ie(mgmt->u.beacon.variable, beacon_ie_len, &info_ap);
		if(beacon->beacon_ies)
			lynx_parse_ie(beacon->beacon_ies, beacon->beacon_ies_len, &info_ap);
		bss->phy_cap = info_ap.phy_cap;
	}

	/* FIXME: time in seconds to determine station's inactivity */
	/* info->inactivity_timeout */

	switch(cfg80211_get_chandef_type(&info->chandef))
	{
		case NL80211_CHAN_NO_HT:
			bss->phy_cap &= ~AP_CAP_11N;
			fw->bandwidth = BW40MHZ_SCN;
			break;
		case NL80211_CHAN_HT20:
			bss->phy_cap |= AP_CAP_11N;
			fw->bandwidth = BW40MHZ_SCN;
			break;
		case NL80211_CHAN_HT40MINUS:
			bss->phy_cap |= AP_CAP_11N;
			fw->bandwidth = BW40MHZ_SCB;
			break;
		case NL80211_CHAN_HT40PLUS:
			bss->phy_cap |= AP_CAP_11N;
			fw->bandwidth = BW40MHZ_SCA;
			break;
	}

	/* update bss->ht_capability */
	bss->ht_capability = 0;
	if(bss->phy_cap & AP_CAP_11N)
	{
		bss->ht_capability = IEEE80211_HT_CAP_SGI_20;

	 	if(fw->bandwidth != BW40MHZ_SCN)
			bss->ht_capability |= (IEEE80211_HT_CAP_DSSSCCK40|IEEE80211_HT_CAP_SUP_WIDTH_20_40|IEEE80211_HT_CAP_GRN_FLD|IEEE80211_HT_CAP_SGI_40);

    	cpu_to_be16s(&(bss->ht_capability));
	}

	bss->bss_desc = vif->fw_vif_idx;
	/* wci update bss */
	lynx_wci_update_vif_cmd(vif);

#ifdef CONFIG_UNIT_TEST
	unittest_bss_backup(vif, __FUNCTION__);
#endif

	/* wci set channel */
	lynx_dbg(LYNX_DBG_WLAN_CFG, "%s():Set AP/GO channel as %d\n", __FUNCTION__,fw->channel);
	lynx_wci_set_channel_cmd(lnx);
	/* start ap */
	lynx_wci_start_ap_cmd(vif);
	/* enable the datapath */
    spin_lock_bh(&vif->if_lock);
    netif_carrier_on(vif->ndev);
    netif_wake_queue(vif->ndev);

	/* FIXME: only for debug */
    set_bit(CONNECTED, &vif->flags);
    set_bit(DATAPATH_EN, &vif->flags);

    spin_unlock_bh(&vif->if_lock);

	schedule_delayed_work(&vif->sync_work, HZ*30);

	return 0;
}

#ifdef CONFIG_SUPPORT_MONITOR_MODE

static int lynx_cfg80211_set_monitor_channel(struct wiphy *wiphy,
				       struct cfg80211_chan_def *chandef)
{
	struct lynx *lnx = wiphy_priv(wiphy);
	struct lynx_vif *vif = lynx_vif_first(lnx);
	
	if(mp_test_firm)
		return -EINVAL;

	lynx_dbg(LYNX_DBG_WLAN_CFG, "lynx_cfg80211_set_monitor_channel() start\n");

	if (!chandef->chan)
		return -EINVAL;
	
	if (chandef->chan->band != NL80211_BAND_2GHZ)
		return -EINVAL;

	/*
	if((!vif) || (!lynx_cfg80211_ready(vif))){
		lynx_dbg(LYNX_DBG_WLAN_CFG, "lynx_cfg80211_set_monitor_channel() failure\n");
		return -EINVAL;
	}
	*/
	lynx_wci_update_chan_band(vif,cfg80211_get_chandef_type(chandef),lynx_freq2ch(chandef->chan->center_freq)   ,0);

	lynx_dbg(LYNX_DBG_WLAN_CFG, "lynx_cfg80211_set_monitor_channel() OK\n");

	return 0;
}
#endif /*CONFIG_SUPPORT_MONITOR_MODE */

#if defined(CONFIG_HOST_WPS) || defined(CONFIG_CUST1_PRIV_BEACON_IE)
static int lynx_cfg80211_change_beacon(struct wiphy *wiphy, struct net_device *dev,
                      struct cfg80211_beacon_data *beacon)
{
	int res = 0;
	struct lynx_vif *vif = netdev_priv(dev);
	if(mp_test_firm)
		return -EINVAL;

	lynx_dbg(LYNX_DBG_WLAN_CFG, "%s():\n", __FUNCTION__);
	
	if (!lynx_cfg80211_ready(vif))
		return -EIO;

	if((vif->nw_type != VIF_AP_MODE) && (vif->nw_type != VIF_P2P_GO_MODE)){
		return -EOPNOTSUPP;
	}
	else{
#ifdef CONFIG_CUST1_PRIV_BEACON_IE
		s32 location = 0;
		location = find_priv_beacon_ie_data(vif,beacon->tail,beacon->tail_len);
		if(location != -1)
		{
			print_hex_dump(KERN_INFO, "change_beacon_priv_beacon_ie", DUMP_PREFIX_OFFSET,
					16, 1,
				beacon->tail+location,beacon->tail[location+1]+2, false);
			res = lynx_cfg80211_set_priv_beacon(vif,beacon->tail+location, beacon->tail[location+1]+2); //+2 add type and length
		}
#endif
#ifdef CONFIG_HOST_WPS
		res = lynx_cfg80211_set_ies(vif, beacon);
#endif
		/*FIXME: Get event back of FW*/
		msleep(300);

		return res;
	}
}
#endif /*CONFIG_HOST_WPS*/

#ifdef CONFIG_HOST_P2P

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
static int lynx_cfg80211_mgmt_tx(struct wiphy *wiphy, struct wireless_dev *wdev,
			struct cfg80211_mgmt_tx_params *params, 
			u64 *cookie)
#else
static int lynx_cfg80211_mgmt_tx(struct wiphy *wiphy, struct wireless_dev *wdev,
			struct ieee80211_channel *chan, bool offchan,
			unsigned int wait, const u8 *buf, size_t len,
			bool no_cck, bool dont_wait_for_ack,
			u64 *cookie)
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0) */
{
	int ret = -EINVAL;
	struct ieee80211_mgmt *mgmt;
	struct lynx *lnx = wiphy_priv(wiphy);
	struct device_configs *fw = &lnx->fw_config;
	struct lynx_vif *vif = lynx_vif_from_wdev(wdev);

	u8 mgmt_type = WCI_NUM_MGMT_FRAME;
	
	
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
		struct ieee80211_channel *chan= params->chan;
		const u8 *buf = params->buf;
		size_t len = params->len;
		bool dont_wait_for_ack = params->dont_wait_for_ack;
#endif
		if(mp_test_firm)
		return ret;
	
	if (!dont_wait_for_ack){
		vif->action_cookie++;
		*cookie = vif->action_cookie;
	}
	else{
		*cookie = 0;
	}
	
	//lynx_dbg(LYNX_DBG_WLAN_CFG,  "Lynx:cfg80211: %s , cookie: %d\n",__FUNCTION__,vif->action_cookie);

	if( vif->wdev.iftype  != NL80211_IFTYPE_P2P_DEVICE ){
		lynx_dbg(LYNX_DBG_WLAN_CFG, "Only support send mgmt frame for P2P control interface\n");
		return ret;
	}
	
/*
	if( vif->scan_req){
		lynx_dbg(LYNX_DBG_WLAN_CFG, "Can not send Action when scan or p2p find  not end!\n");
		return ret;
	}
*/
	if ( len < offsetof(struct ieee80211_mgmt, u)){
		lynx_dbg(LYNX_DBG_WLAN_CFG,  "%s : mgmt packet length error!\n",__FUNCTION__);
		return ret;
	}

	mgmt = (struct ieee80211_mgmt *)buf;

	if (( ieee80211_is_probe_resp(mgmt->frame_control))
		&&(len >= offsetof(struct ieee80211_mgmt, u.probe_resp.variable)))
	{
		mgmt_type = WCI_FRAME_PROBE_RESP ;
		lynx_dbg(LYNX_DBG_WLAN_CFG,  "Send PROBE_RESP mgmt packet\n");
	}
	else if (( ieee80211_is_action(mgmt->frame_control))
		&&(len >= offsetof(struct ieee80211_mgmt, u.action.u.self_prot.variable)))

	{
		mgmt_type = WCI_FRAME_ACTION ;
		lynx_dbg(LYNX_DBG_WLAN_CFG,  "Send ACTION mgmt packet\n");
	}
	else
	{
		lynx_dbg(LYNX_DBG_WLAN_CFG,
			"%s : Not support for this mgmt packet type (fc:0x%02X) or  length error (%d)!\n",
			__FUNCTION__,mgmt->frame_control,len);
		return ret;
	}

	if( fw->channel !=lynx_freq2ch (chan->center_freq)){
		fw->channel = lnx->channel =lynx_freq2ch (chan->center_freq);
		lynx_dbg(LYNX_DBG_WLAN_CFG,  "Set to channel %d,freq %d for send mgmt packet!\n",lnx->channel ,chan->center_freq);
		/* wci set channel */
		lynx_wci_set_channel_cmd(lnx);
	}

	ret= lynx_wci_set_mgmt_txpkt_cmd(vif, buf, (u16) len,mgmt_type);

	if((ret==0)&&(!dont_wait_for_ack)){
		cfg80211_mgmt_tx_status(&(vif->wdev), *cookie, buf,len, true,GFP_KERNEL);
	}

	//lynx_dbg(LYNX_DBG_WLAN_CFG,  "%s : completed! RET %d \n",__FUNCTION__,ret);
	return ret;
}

static void lynx_cfg80211_mgmt_frame_register(struct wiphy *wiphy,
				       struct wireless_dev *wdev,
				       u16 frame_type, bool reg)
{
		struct lynx_vif *vif = lynx_vif_from_wdev(wdev);
		u16 mgmt_type;
		
		if(mp_test_firm)
			return ;

		lynx_dbg(LYNX_DBG_WLAN_CFG,"Lynx:cfg80211: %s \n",__FUNCTION__);
		lynx_dbg(LYNX_DBG_WLAN_CFG,"frame_type %04x, reg=%d\n", frame_type, reg);

		if(( vif->wdev.iftype  != NL80211_IFTYPE_P2P_DEVICE ) &&
			( vif->wdev.iftype  != NL80211_IFTYPE_P2P_GO )) 
		{
			lynx_dbg(LYNX_DBG_WLAN_CFG, "Only support register mgmt frame for P2P control/data interface\n");
			return;
		}

		mgmt_type = (frame_type & IEEE80211_FCTL_STYPE) >> 4;

		if (reg){
			vif->mgmt_rx_reg |= BIT(mgmt_type);
		}
		else{
			vif->mgmt_rx_reg &= ~BIT(mgmt_type);
		}
		return;
}

static int lynx_cfg80211_start_p2p_device(struct wiphy *wiphy, struct wireless_dev *wdev)
{
	int ret = 0;
	struct lynx *lnx = NULL;
	struct lynx_vif *vif = NULL;
	struct wm_bss *bss = NULL;
	u32 new_flag;

	if(mp_test_firm)
		return -EINVAL;

	lnx = wiphy_priv(wiphy);
	if(!lnx)
		return -EINVAL;

	vif = lynx_get_p2p_vif(lnx,0) ;
	if(!vif)
		return -EINVAL;

	bss = &vif->bss;
	if(!bss)
		return -EINVAL;

	lynx_dbg(LYNX_DBG_WLAN_CFG,  "Lynx:cfg80211: %s \n",__FUNCTION__);
	
	new_flag = be32_to_cpu(bss->flag);
	new_flag |= (BSS_FLG_ENABLE );

	bss->flag = cpu_to_be32(new_flag);
	lynx_wci_update_vif_cmd(vif);
	
	return ret;
}

static void lynx_cfg80211_stop_p2p_device(struct wiphy *wiphy, struct wireless_dev *wdev)
{
	struct lynx *lnx = NULL;
	struct lynx_vif *vif = NULL;
	struct wm_bss *bss = NULL;
	u32 new_flag;

	if(mp_test_firm)
		return ;
	
	lnx = wiphy_priv(wiphy);
	if(!lnx)
		return ;
	
	vif = lynx_get_p2p_vif(lnx,0) ;
	if(!vif)
		return ;
	
	bss = &vif->bss;
	if(!bss)
		return ;

	lynx_dbg(LYNX_DBG_WLAN_CFG,  "Lynx:cfg80211: %s \n",__FUNCTION__);

	if((NULL != vif)&&(vif->scan_req)){
		info.aborted = true;
		cfg80211_scan_done(vif->scan_req, &info);
		vif->scan_req = NULL;
	}

	new_flag = be32_to_cpu(bss->flag);
	new_flag &= ~(BSS_FLG_ENABLE );
	bss->flag = cpu_to_be32(new_flag);
	lynx_wci_update_vif_cmd(vif);
}

static int lynx_cfg80211_remain_on_channel(struct wiphy *wiphy,
				 struct wireless_dev *wdev,
				 struct ieee80211_channel *chan,
				 unsigned int duration,
				 u64 *cookie)
{
	int rc = 0;
	struct lynx *lnx = wiphy_priv(wiphy);

	u32 new_flag;
	struct lynx_vif *vif = lynx_get_p2p_vif(lnx,0) ;
	struct wm_bss *bss = &vif->bss;

	if(mp_test_firm)
		return -EINVAL;

	/* TODO: handle duration */
	lynx_dbg(LYNX_DBG_WLAN_CFG,  "Lynx:cfg80211: %s \n",__FUNCTION__);

	if (!chan){
		lynx_dbg(LYNX_DBG_WLAN_CFG,  "Error :cfg80211: %s : chan error! \n",__FUNCTION__);
		return -EINVAL;
	}
	if (chan->center_freq == 0){
		lynx_dbg(LYNX_DBG_WLAN_CFG,  "Error :cfg80211: %s : center_freq error! \n",__FUNCTION__);
		return -EINVAL;
	}
	if (chan->band != NL80211_BAND_2GHZ){
		lynx_dbg(LYNX_DBG_WLAN_CFG,  "Error :cfg80211: %s : not 2Ghz ! \n",__FUNCTION__);
		return -EINVAL;
	}

	new_flag = be32_to_cpu(bss->flag);
	new_flag &= ~(BSS_FLG_NO_PROBE_RESP );
	bss->flag = cpu_to_be32(new_flag);

	/* The function will be called at  lynx_wci_update_chan_band(), too.*/
	//lynx_wci_update_vif_cmd(vif); 

	/* Update Channel and Bandwidth*/
	lynx_wci_update_chan_band(vif,(enum nl80211_channel_type)NL80211_CHAN_NO_HT,lynx_freq2ch(chan->center_freq)	,0);
	lynx_dbg(LYNX_DBG_WLAN_CFG, "Lynx:switch to chan %d,freq %d,NL band %d \n",lnx->channel ,chan->center_freq,NL80211_CHAN_NO_HT);


	lnx->remain_on_channel_cookie++;
	memcpy(&lnx->remain_on_channel, chan, sizeof(*chan));
	*cookie = lnx->remain_on_channel_cookie;
	cfg80211_ready_on_channel(wdev, *cookie, chan, duration, GFP_ATOMIC);

	return rc;
}

static int lynx_cfg80211_cancel_remain_on_channel(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					u64 cookie)
{
	int rc = 0;
	u32 new_flag;
	struct lynx *lnx = wiphy_priv(wiphy);
	struct lynx_vif *vif = lynx_get_p2p_vif(lnx,0) ;
	struct wm_bss *bss = &vif->bss;

	if(mp_test_firm)
		return rc;

	lynx_dbg(LYNX_DBG_WLAN_CFG,  "Lynx:cfg80211: %s \n",__FUNCTION__);

	new_flag = be32_to_cpu(bss->flag);
	new_flag |= (BSS_FLG_NO_PROBE_RESP );
	bss->flag = cpu_to_be32(new_flag);
	lynx_wci_update_vif_cmd(vif);

	cfg80211_remain_on_channel_expired(wdev, lnx->remain_on_channel_cookie,
					&lnx->remain_on_channel,GFP_ATOMIC);
	return rc;
}

#endif /*CONFIG_HOST_P2P*/

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
static int lynx_cfg80211_del_station(struct wiphy *wiphy, struct net_device *dev, struct station_del_parameters *params)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
static int lynx_cfg80211_del_station(struct wiphy *wiphy, struct net_device *dev, const u8 *mac)
#else
static int lynx_cfg80211_del_station(struct wiphy *wiphy, struct net_device *dev, u8 *mac)
#endif
{
	struct lynx_vif *vif = netdev_priv(dev);
	struct lynx *lnx = vif->lnx;
	struct lynx_sta *sta;
	int sta_idx=0, i;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
	/* FIXME: may cfg80211 use the bcast_addr ? */
	const unsigned char bcast_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	const unsigned char *mac = params->mac ? params->mac : bcast_addr;
#endif

	if(mp_test_firm)
		return -EINVAL;

	/* TODO: find sta_idx by mac */
	if((mac) && (is_valid_ether_addr(mac)))
	{
		sta_idx = sta_map2idx(vif, mac);
		if(sta_idx < 0)
			return -1;
	}
	else
		sta_idx = 0xFF;	// max 8 stations

#ifdef CONFIG_HOST_P2P
	if((vif->wdev.iftype == NL80211_IFTYPE_P2P_GO) &&
		( sta_idx != 0xFF) &&
		(lnx->sta_list[sta_idx].flags & LYNX_STA_VALID)){
		lynx_dbg(LYNX_DBG_WLAN_CFG,"%s: Avoid send deauth after p2p client connected to lynx(Go).\n", __FUNCTION__);
		return 0;
	}
#endif /*CONFIG_HOST_P2P*/

	/* del the sta in the driver */
	for(i=0; i<LYNX_STA_MAX_NUM; i++)
	{
		if((vif->sta_idx_map & (1 << i)) && ((sta_idx == i) || (sta_idx == 0xff)))
		{
			vif->sta_idx_map &= ~(1 << i);
			sta = &lnx->sta_list[i];
			memset(sta, 0, sizeof(struct lynx_sta));
		}
	}

	return lynx_wci_sta_remove_cmd(vif, sta_idx, WLAN_REASON_PREV_AUTH_NOT_VALID);
}

static struct cfg80211_bss *lynx_add_bss_if_needed(struct lynx_vif *vif, const u8 *bssid,
             			struct ieee80211_channel *chan, const u8 *beacon_ie, size_t beacon_ie_len)
{
    struct lynx *lnx = vif->lnx;
    struct cfg80211_bss *bss=NULL;
    u16 cap_mask, cap_val;
    u8 *ie;

    if (vif->nw_type == VIF_IBSS_MODE) 
	{
        cap_mask = WLAN_CAPABILITY_IBSS;
        cap_val = WLAN_CAPABILITY_IBSS;
    } 
	else 
	{
        cap_mask = WLAN_CAPABILITY_ESS;
        cap_val = WLAN_CAPABILITY_ESS;
    }
    
	/* FIXME: use chan=NULL to avoid that chan (addr) is not the same as that in the cfg80211 bss list */
    bss = cfg80211_get_bss(lnx->wiphy, NULL, bssid, vif->ssid, vif->ssid_len, cap_mask, cap_val);

    if(bss == NULL) 
	{
        /*
         * Since cfg80211 may not yet know about the BSS,
         * generate a partial entry until the first BSS info
         * event becomes available.
         *
         * Prepend SSID element since it is not included in the Beacon
         * IEs from the target.
         */
        ie = kmalloc(2 + vif->ssid_len + beacon_ie_len, CFG80211_GFP);
        if(ie)
		{
			ie[0] = WLAN_EID_SSID;
			ie[1] = vif->ssid_len;
			memcpy(ie + 2, vif->ssid, vif->ssid_len);
			memcpy(ie + 2 + vif->ssid_len, beacon_ie, beacon_ie_len);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
			bss = cfg80211_inform_bss(lnx->wiphy, chan, CFG80211_BSS_FTYPE_UNKNOWN, bssid, 
						0, cap_val, 100, ie, 2 + vif->ssid_len + beacon_ie_len, 0, CFG80211_GFP);
#else
			bss = cfg80211_inform_bss(lnx->wiphy, chan, bssid, 0, cap_val, 100,
			  			ie, 2 + vif->ssid_len + beacon_ie_len, 0, CFG80211_GFP);
#endif
			if(bss)
				lynx_dbg(LYNX_DBG_WLAN_CFG, "added bss %pM to cfg80211\n", bssid);
			kfree(ie);
		}
    } 
	else 
	{
        lynx_dbg(LYNX_DBG_WLAN_CFG, "cfg80211 already has a bss\n");
	}

    return bss;
}

int lynx_cfg80211_connect_event(struct lynx_vif *vif, u8 channel,
                   u8 *bssid, u8 assoc_req_len, u8 assoc_resp_len, u8 *assoc_info)
{
    struct ieee80211_channel *chan;
    struct lynx *lnx = vif->lnx;
    struct cfg80211_bss *bss=NULL;
    int beacon_ie_len = 0;	/* FIXME: may bring the beacon ie */
    u16 freq = lynx_2ghz_channels[channel-1].center_freq;
    struct station_info sinfo;	
    struct wlan_ie_info info_wme;
    /* FIXME: How to get the beacon ie, if bss is not in cfg80211 now? */
    u8 *assoc_resp_ie = NULL;

#ifdef CONFIG_HOST_P2P
	struct wm_bss *vif_bss = &vif->bss;
	u32 new_flag;
	if( vif->wdev.iftype  == NL80211_IFTYPE_P2P_DEVICE ){
		return -1;
	}else if( vif->wdev.iftype  == NL80211_IFTYPE_P2P_CLIENT ){
		new_flag = be32_to_cpu(vif_bss->flag);
		if(new_flag & BSS_FLG_RECONNECT){
			lynx_dbg(LYNX_DBG_WLAN_CFG, "%s: Clear BSS_FLG_RECONNECT for P2P GC\n", __FUNCTION__);
			new_flag &= ~BSS_FLG_RECONNECT;
		}else{
			lynx_dbg(LYNX_DBG_WLAN_CFG, "%s: Set BSS_FLG_RECONNECT for P2P GC\n", __FUNCTION__);
			new_flag |= BSS_FLG_RECONNECT;
		}
		vif_bss->flag = cpu_to_be32(new_flag);
		lynx_wci_update_vif_cmd(vif);
	}

#endif /*CONFIG_HOST_P2P*/

	lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): vif->nw_type=%d, vif->fw_vif_idx=%d, vif->wdev.iftype=%d,"
								" assoc_req_len=%d, assoc_resp_len=%d, channel=%d, freq=%d\n",
		 						__FUNCTION__, vif->nw_type, vif->fw_vif_idx, vif->wdev.iftype, 
								assoc_req_len, assoc_resp_len, channel, freq);

	if((vif->nw_type == VIF_AP_MODE) || (vif->nw_type == VIF_P2P_GO_MODE))
	{
		/* send event to application */
		memset(&sinfo, 0, sizeof(sinfo));

		/* TODO: sinfo.generation */

		sinfo.assoc_req_ies = assoc_info + beacon_ie_len;
		sinfo.assoc_req_ies_len = assoc_req_len;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0))
		sinfo.filled |= STATION_INFO_ASSOC_REQ_IES;
#endif

		/* bssid should be the sta's mac addr */
		cfg80211_new_sta(vif->ndev, bssid, &sinfo, CFG80211_GFP);

		return 0;
	}

	/* assoc_info may include the beacon ie */
	assoc_resp_ie = assoc_info + beacon_ie_len;

    /*
     * Store Beacon interval here; DTIM period will be available only once
     * a Beacon frame from the AP is seen.
     */
    clear_bit(DTIM_PERIOD_AVAIL, &vif->flags);

    if (vif->nw_type == VIF_IBSS_MODE) {
        if (vif->wdev.iftype != NL80211_IFTYPE_ADHOC) {
            lynx_dbg(LYNX_DBG_WLAN_CFG, "%s: lynx not in ibss mode\n", __FUNCTION__);
            return -1;
        }
    }
    else if (vif->nw_type == VIF_STA_MODE) {
        if (vif->wdev.iftype != NL80211_IFTYPE_STATION &&
            vif->wdev.iftype != NL80211_IFTYPE_P2P_CLIENT) {
            lynx_dbg(LYNX_DBG_WLAN_CFG, "%s: lynx not in station mode\n", __FUNCTION__);
            return -1;
        }

		/* parse the WME ie information & setup the acm info */
		memset(&info_wme, 0, sizeof(info_wme));
		lynx_parse_ie(assoc_resp_ie, assoc_resp_len, &info_wme);
		if(info_wme.wme_ie != NULL)
			vif->acm = lynx_parse_wme_ie((struct wlan_ie_wme_param *)(info_wme.wme_ie));
		else 
			vif->acm = 0;
    }

    chan = ieee80211_get_channel(lnx->wiphy, (int) freq);

	/* FIXME: update the bss info, if it's not in cfg80211 list */
    if(!(bss = lynx_add_bss_if_needed(vif, bssid, chan, assoc_info, beacon_ie_len)))
    {
        lynx_dbg(LYNX_DBG_WLAN_CFG, "could not add cfg80211 bss entry\n");
        return -1;
    }

	/* TODO: check the chan & bss->channel info & do something  */

    if (vif->nw_type == VIF_IBSS_MODE) {
        lynx_dbg(LYNX_DBG_WLAN_CFG, "connect with IBSS mode\n");
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
		cfg80211_ibss_joined(vif->ndev, bssid, chan, CFG80211_GFP);
#else
		cfg80211_ibss_joined(vif->ndev, bssid, CFG80211_GFP);
#endif
        cfg80211_put_bss(lnx->wiphy, bss);

		/* setup device beacon */
		lynx_wci_set_beacon_cmd(vif);
        return -1;
    }

    if (test_bit(CONNECT_PEND, &vif->flags)) {
        /* inform connect result to cfg80211 */
        cfg80211_connect_result(vif->ndev, bssid,
                    NULL, 0,
                    assoc_resp_ie, assoc_resp_len,
                    WLAN_STATUS_SUCCESS, CFG80211_GFP);
        cfg80211_put_bss(lnx->wiphy, bss);
        lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): info connect result\n", __FUNCTION__);
    } else if (test_bit(CONNECTED, &vif->flags)) {
       /* inform FW autonomous roam event to cfg80211 ; Not roaming by wpa_supplicant  */
       /* FW report BSSID is not the same with wpa_supplicant request*/
        if(memcmp(vif->req_bssid, bssid, ETH_ALEN) ) {
             lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): info roam result\n", __FUNCTION__);
	     struct cfg80211_roam_info info = {0};
	     info.bss = bss;
	     info.channel = chan;
	     info.resp_ie = assoc_resp_ie;
	     info.resp_ie_len = assoc_resp_len;
             cfg80211_roamed(vif->ndev, &info, CFG80211_GFP);
        }
    }
    return 0;
}

static int lynx_cfg80211_disconnect(struct wiphy *wiphy,
                struct net_device *dev,
                u16 reason_code)
{
    struct lynx *lnx = lynx_priv(dev);
    struct lynx_vif *vif = netdev_priv(dev);
	if(mp_test_firm)
		return -EINVAL;

    lynx_dbg(LYNX_DBG_WLAN_CFG, "%s: reason=%u\n", __FUNCTION__, reason_code);

    lynx_cfg80211_sscan_disable(vif);

    if(!lynx_cfg80211_ready(vif))
        return -EIO;

    if (test_bit(DESTROY_IN_PROGRESS, &lnx->flag)) {
        lynx_dbg(LYNX_DBG_WLAN_CFG, "DESTROY_IN_PROGRESS\n");
        return -EBUSY;
    }

    if(down_interruptible(&lnx->sem)) {
        lynx_dbg(LYNX_DBG_WLAN_CFG, "busy, couldn't get access\n");
        return -ERESTARTSYS;
    }

	if(test_bit(CONNECTED, &vif->flags) || test_bit(CONNECT_PEND, &vif->flags))
	{
    	lynx_disconnect(vif, reason_code);
	}

    memset(vif->ssid, 0, sizeof(vif->ssid));
    vif->ssid_len = 0;

    up(&lnx->sem);

    return 0;

}

static int lynx_cfg80211_add_key(struct wiphy *wiphy,
                struct net_device *ndev,
                u8 key_index,
                bool pairwise,
                const u8 *mac_addr,
                struct key_params *params)
{
	struct lynx_vif *vif = netdev_priv(ndev);
	struct lynx_key *key;
	unsigned int cipher=0;

    /* FIXME: ath6kl store the seq & seq_len
              broadcom uses the seq/seq_len for rx_iv ("if IW_ENCODE_EXT_RX_SEQ_VALID set") */
#if 0
    /* handle seq & seq_len */
    int seq_len;
    seq_len = params->seq_len;
    if((params->cipher == WLAN_CIPHER_SUITE_SMS4) && (seq_len > KEY_SEQ_LEN)) 
    {
        /* Only first half of the WPI PN is configured */
        seq_len = KEY_SEQ_LEN;
    }

    /* ieee80211.h : #define WLAN_MAX_KEY_LEN 32 */
    if (params->key_len > WLAN_MAX_KEY_LEN || seq_len > sizeof(key->seq))
        return -EINVAL;

    key->seq_len = seq_len;
    memcpy(key->seq, params->seq, key->seq_len);
#endif
	if(mp_test_firm)
		return -EINVAL;

	if(params->key_len > WLAN_MAX_KEY_LEN)
	{
		lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): WRONG key len (%d)\n", __FUNCTION__, params->key_len);
		return -EINVAL;
	}
    
	if(key_index > WCI_MAX_KEY_INDEX) 
	{
		lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): WRONG key idx (%d)\n", __FUNCTION__, key_index);
		return -EINVAL;
	}

    cipher = lynx_set_cipher(vif, params->cipher, pairwise);

    key = &vif->keys[key_index];
    key->key_len = params->key_len;
    memcpy(key->key, params->key, key->key_len);

    if(!pairwise)
    {
        /* FIXME: not sure this setting is right */
        key->cipher = cipher;
        key->key_type = KEY_TYPE_GLOBAL_KEY;
    }
    else
    {
        key->cipher = cipher;
        /* FIXME: IBSS use ap mode setting? */
        if((vif->nw_type == VIF_STA_MODE) || (vif->nw_type == VIF_P2P_CLIENT_MODE))
            key->key_type = KEY_TYPE_STA_PAIRWISE_KEY;
        else
            key->key_type = KEY_TYPE_PAIRWISE_KEY;
    }

    lynx_wci_addkey_cmd(vif, mac_addr, key, key_index, key->key_type);

	if(be32_to_cpu(vif->bss.auth_capability) & (AUTH_CAP_CIPHER_WEP40 | AUTH_CAP_CIPHER_WEP104))
	{
		/* FIXME: wpa_supplicant seems only send group key with wep */
        if((vif->nw_type == VIF_STA_MODE) || (vif->nw_type == VIF_P2P_CLIENT_MODE) || (vif->nw_type == VIF_IBSS_MODE))
    		lynx_wci_addkey_cmd(vif, mac_addr, key, key_index, KEY_TYPE_STA_PAIRWISE_KEY);
	}

	/* authorize the data path */
    spin_lock_bh(&vif->if_lock);
    set_bit(DATAPATH_EN, &vif->flags);
    spin_unlock_bh(&vif->if_lock);
	

    return 0;
}

static int lynx_cfg80211_del_key(struct wiphy *wiphy,
                struct net_device *ndev,
                u8 key_index,
                bool pairwise,
                const u8 *mac_addr)
{
    struct lynx_vif *vif = netdev_priv(ndev);
    struct lynx_key *key;
    u32 key_type;
	if(mp_test_firm)
		return -EINVAL;

	if(!vif || (key_index > WCI_MAX_KEY_INDEX))
	{
		lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): error, vif=0x%x, key_index=%d\n", 
					__FUNCTION__, (u32)(uintptr_t)vif, key_index);
		return -1;
	}

    key = &vif->keys[key_index];
    key_type = key->key_type;
    memset(key, 0, sizeof(struct lynx_key));

    /* key == NULL : delete key */
    lynx_wci_addkey_cmd(vif, mac_addr, NULL, key_index, key_type);

    return 0;
}

static int lynx_cfg80211_get_key(struct wiphy *wiphy, struct net_device *ndev,
                   u8 key_index, bool pairwise, const u8 *mac_addr, void *cookie,
                   void (*callback) (void *cookie, struct key_params *))
{
    struct lynx_vif *vif = netdev_priv(ndev);
    struct lynx_key *key = NULL;
    struct key_params params;
	if(mp_test_firm)
		return -EINVAL;

    lynx_dbg(LYNX_DBG_WLAN_CFG, "%s: index %d\n", __FUNCTION__, key_index);

    if(!lynx_cfg80211_ready(vif))
        return -EIO;

    if(key_index > WCI_MAX_KEY_INDEX) 
    {
        lynx_dbg(LYNX_DBG_WLAN_CFG, "%s: illegal key index %d\n", __FUNCTION__, key_index);
        return -ENOENT;
    }

    key = &vif->keys[key_index];
    memset(&params, 0, sizeof(params));
    params.cipher = key->cipher;
    params.key_len = key->key_len;
    params.key = key->key;

    /* FIXME: 1) Does lynx need to store seq & seq_length? 
              2) how to get the correct seq/seq_len, if 4 way handshake is done in the device? */
#if 0
    params.seq_len = key->seq_len;
    params.seq = key->seq;
#endif

    callback(cookie, &params);

    return key->key_len ? 0 : -ENOENT;
}

static int lynx_cfg80211_set_default_key(struct wiphy *wiphy,
                struct net_device *ndev,
                u8 key_index,
                bool unicast,
                bool multicast)
{
	struct lynx_vif *vif = netdev_priv(ndev);
	int ret = 0;
	
	if(mp_test_firm)
	{
		ret = -EINVAL;
	}
	else if(key_index > WCI_MAX_KEY_INDEX) 
	{
		lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): WRONG key idx (%d)\n", __FUNCTION__, key_index);
		ret = -EINVAL;
	}
	else
	{
		vif->def_txkey_index = key_index;
		vif->keys[key_index].is_txkey = 1;

		if(test_bit(WLAN_ENABLED, &vif->flags))
			ret = lynx_wci_set_defkey_cmd(vif, unicast, multicast, key_index);
	}

	return ret;
}

static int lynx_cfg80211_join_ibss(struct wiphy *wiphy,
                     struct net_device *dev,
                     struct cfg80211_ibss_params *ibss_param)
{
    struct lynx *lnx = lynx_priv(dev);
    struct lynx_vif *vif = netdev_priv(dev);
    int status;
	if(mp_test_firm)
		return -EINVAL;

    if(!lynx_cfg80211_ready(vif)) {
        return -EIO;
    }

	if(ibss_param->ssid_len)
	{
		vif->ssid_len = ibss_param->ssid_len;
		memcpy(vif->ssid, ibss_param->ssid, vif->ssid_len);
	}

    if (ibss_param->chandef.chan)
    {
        vif->ch_hint = ibss_param->chandef.chan->center_freq;
        /* FIXME: if ibss group is in another channel, how to fix it? */
        lnx->channel = lynx_freq2ch(ibss_param->chandef.chan->center_freq);
    }

    if (ibss_param->channel_fixed) {
        /*
         * TODO: channel_fixed: The channel should be fixed, do not
         * search for IBSSs to join on other channels. Target
         * firmware does not support this feature, needs to be
         * updated.
         */
        return -EOPNOTSUPP;
    }

    memset(vif->req_bssid, 0, sizeof(vif->req_bssid));
    if (ibss_param->bssid && !is_broadcast_ether_addr(ibss_param->bssid)) {
        memcpy(vif->req_bssid, ibss_param->bssid, sizeof(vif->req_bssid));
    }

	lynx_clear_cipher(vif);
    lynx_set_wpa_version(vif, 0);

    if((status = lynx_set_auth_type(vif, NL80211_AUTHTYPE_OPEN_SYSTEM))) {
        return status;
    }

    if (ibss_param->privacy) {
        lynx_set_cipher(vif, WLAN_CIPHER_SUITE_WEP40, true);
        lynx_set_cipher(vif, WLAN_CIPHER_SUITE_WEP40, false);
    } else {
        lynx_set_cipher(vif, 0, true);
        lynx_set_cipher(vif, 0, false);
    }

    lynx_dbg(LYNX_DBG_WLAN_CFG,
           "%s: connect called with authmode %d dot11 auth %d"
           " PW crypto %d GRP crypto %d channel hint %u\n",
           __FUNCTION__, vif->auth_mode, vif->dot11_auth_mode, 
		   vif->prwise_crypto, vif->grp_crypto, vif->ch_hint);

    if (!(status = lynx_wci_connect_cmd(vif, vif->req_bssid, vif->ssid, vif->ssid_len, NULL)))
    {
    	set_bit(CONNECT_PEND, &vif->flags);
    }

    return status;
}

static int lynx_cfg80211_leave_ibss(struct wiphy *wiphy,
                      struct net_device *dev)
{
    struct lynx_vif *vif = netdev_priv(dev);
	
	if (mp_test_firm) {
		return -EINVAL;
	}

    if (!lynx_cfg80211_ready(vif)) {
        return -EIO;
    }

	if(test_bit(CONNECTED, &vif->flags) || test_bit(CONNECT_PEND, &vif->flags))
	{
    	lynx_disconnect(vif, WLAN_REASON_DEAUTH_LEAVING);
#if 0
		clear_bit(CONNECT_PEND, &vif->flags);
        clear_bit(CONNECTED, &vif->flags);
#endif
	}
    memset(vif->ssid, 0, sizeof(vif->ssid));
    vif->ssid_len = 0;

	lynx_wci_disconnect_cmd(vif, 0, WLAN_REASON_DEAUTH_LEAVING);

    return 0;
}

static int lynx_cfg80211_set_txpower(struct wiphy *wiphy,
                       struct wireless_dev *wdev,
                       enum nl80211_tx_power_setting type,
                       int mbm)
{
    struct lynx *lnx = (struct lynx *)wiphy_priv(wiphy);
    int dbm = MBM_TO_DBM(mbm);
	
	if (mp_test_firm) {
		return -EINVAL;
	}

    lynx_dbg(LYNX_DBG_WLAN_CFG, "%s: type 0x%x, dbm %d\n", __FUNCTION__, type, dbm);

    switch (type) {
    case NL80211_TX_POWER_AUTOMATIC:
        lnx->tx_pwr = 0;
        break;
    case NL80211_TX_POWER_LIMITED:
        lnx->tx_pwr = dbm;
        break;
    default:
        return -EOPNOTSUPP;
    }

    lynx_wci_set_tx_pwr_cmd(lnx, dbm);

    return 0;
}

static int lynx_cfg80211_get_txpower(struct wiphy *wiphy,
                       struct wireless_dev *wdev, int *dbm)
{
    struct lynx *lnx = (struct lynx *)wiphy_priv(wiphy);
    struct lynx_vif *vif = lynx_vif_first(lnx);
	
	if (mp_test_firm) {
		return -EINVAL;
	}

    if (!vif) {
        return -EIO;
    }

    lnx->tx_pwr = 0;

    if(lynx_wci_get_tx_pwr_cmd(vif) != 0) 
    {
        lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): can't trigger wci cmd\n", __FUNCTION__);
        return -EIO;
    }

    wait_event_interruptible_timeout(lnx->event_wq, lnx->tx_pwr != 0, 2 * HZ);

	if(lnx->tx_pwr == 0)
    {
        lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): target did not respond\n", __FUNCTION__);
        return -EINTR;
    }
	else
	{
		*dbm = lnx->tx_pwr;
		return 0;
	}
}

int lynx_cfg80211_mic_err_event(struct lynx_vif *vif, u8 *addr, u32 key_id, u32 key_type)
{
	/* tsc == NULL ? */
	cfg80211_michael_mic_failure(vif->ndev, addr, key_type, key_id, NULL, CFG80211_GFP);
	
    return 0;
}

static int lynx_cfg80211_set_wds_peer(struct wiphy *wiphy, struct net_device *dev,
				  const u8 *addr)
{
    struct lynx_vif *vif = netdev_priv(dev);

	if(mp_test_firm)
		return -EINVAL;

#ifdef CONFIG_HOST_P2P
	if( vif->wdev.iftype  == NL80211_IFTYPE_P2P_DEVICE )
		return 0;
#endif /*CONFIG_HOST_P2P*/

	lynx_wci_wds_peer_add_cmd(vif, 0, (u8 *) addr);

    spin_lock_bh(&vif->if_lock);
	
	/* enable the datapath */
    netif_carrier_on(vif->ndev);
    netif_wake_queue(vif->ndev);

	/* FIXME: only for debug */
    set_bit(CONNECTED, &vif->flags);
    set_bit(DATAPATH_EN, &vif->flags);

    spin_unlock_bh(&vif->if_lock);

	return 0;
}

static void set_sinfo(struct lynx *lnx, int sta_idx, struct station_info *sinfo)
{
	struct lynx_sta *sta = &lnx->sta_list[sta_idx];
	struct timespec64 now;
	int is_64bits = (sizeof(unsigned long) == 8);
	/* rate_bg[] = {CCK_1M, CCK_2M, CCK_5_5M, CCK_11M, OFDM_6M, OFDM_9M, OFDM_12M, OFDM_18M, OFDM_24M, OFDM_36M, OFDM_48M, OFDM_54M} */
	int rate_bg[] = {10, 20, 55, 110, 60, 90, 120, 180, 240, 360, 480, 540};
	
	/* rate_n[] = {MCS_0, MCS_1, MCS_2, MCS_3, MCS_4, MCS_5, MCS_6, MCS_7} */
	int rate_n20[] = {65, 130, 195, 260, 390, 520, 585, 650};/*11n20 800ns GI*/
	int rate_n40[] = {135, 270, 405, 540, 810, 1080, 1215, 1350};/*11n40 800ns GI*/
	int rate_bg_tbl_size = 12;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
	if(is_64bits)
	{
		sinfo->filled |= BIT(NL80211_STA_INFO_RX_BYTES64);
		sinfo->filled |= BIT(NL80211_STA_INFO_TX_BYTES64);
	}
	else
	{
		sinfo->filled |= BIT(NL80211_STA_INFO_RX_BYTES);
		sinfo->filled |= BIT(NL80211_STA_INFO_TX_BYTES);
	}
	sinfo->filled |= (BIT(NL80211_STA_INFO_RX_PACKETS) | BIT(NL80211_STA_INFO_TX_PACKETS)
				 	| BIT(NL80211_STA_INFO_SIGNAL) | BIT(NL80211_STA_INFO_SIGNAL_AVG)
					| BIT(NL80211_STA_INFO_INACTIVE_TIME) | BIT(NL80211_STA_INFO_TX_BITRATE));
#else
	if(is_64bits)
	{
		sinfo->filled |= STATION_INFO_RX_BYTES64;
		sinfo->filled |= STATION_INFO_TX_BYTES64;
	}
	else
	{
		sinfo->filled |= STATION_INFO_RX_BYTES;
		sinfo->filled |= STATION_INFO_TX_BYTES;
	}
	sinfo->filled |= (STATION_INFO_RX_PACKETS | STATION_INFO_TX_PACKETS | 
	 				STATION_INFO_SIGNAL | STATION_INFO_SIGNAL_AVG |
	 				STATION_INFO_INACTIVE_TIME | STATION_INFO_TX_BITRATE);
#endif

	sinfo->rx_bytes = sta->rx_bytes;
	sinfo->rx_packets = sta->rx_packets;
	sinfo->tx_bytes = sta->tx_bytes;
	sinfo->tx_packets = sta->tx_packets;

	if(lnx->sta_tx_rate[sta_idx] > rate_bg_tbl_size)
	{
		sinfo->txrate.mcs = (lnx->sta_tx_rate[sta_idx] - rate_bg_tbl_size-1);
		sinfo->txrate.flags |= RATE_INFO_FLAGS_MCS;
		if(sta->bandwidth == BW40MHZ_SCN)
		{
			sinfo->txrate.legacy = rate_n20[sinfo->txrate.mcs];
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
			sinfo->txrate.bw = RATE_INFO_BW_20;
#endif
		}
		else
		{
			sinfo->txrate.legacy = rate_n40[sinfo->txrate.mcs];
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
			sinfo->txrate.bw = RATE_INFO_BW_40;
#endif
		}
	}
	else
		sinfo->txrate.legacy = rate_bg[lnx->sta_tx_rate[sta_idx] - 1];

	sinfo->signal = bb_rssi_decode(sta->signal_last, RSSI_OFFSET);
	sinfo->signal_avg = bb_rssi_decode(sta->signal_avg, RSSI_OFFSET);

	ktime_get_real_ts64(&now);
	sinfo->inactive_time = (now.tv_sec * 1000) - sta->last_active_time;
}

static int lynx_cfg80211_get_station(struct wiphy *wiphy, struct net_device *dev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
			      const u8 *mac, struct station_info *sinfo)
#else
			      u8 *mac, struct station_info *sinfo)
#endif
{
    struct lynx_vif *vif = netdev_priv(dev);
    struct lynx *lnx = vif->lnx;
	struct lynx_sta *sta=NULL;
	int i;
	int ret = -ENOENT;
	if(mp_test_firm)
		return -EINVAL;

	for(i=0; i<LYNX_STA_MAX_NUM; i++)
	{
		sta = &lnx->sta_list[i];
		if((sta->flags & LYNX_STA_VALID) && (sta->vif == vif))
		{
			if(vif->nw_type == VIF_STA_MODE)
				break;
			else if(mac)
			{
				if(memcmp(mac, sta->addr, 6) == 0)
					break;
			}
		}
	}

	if(i < LYNX_STA_MAX_NUM)
	{
		set_sinfo(lnx, i, sinfo);
		ret = 0;
	}

	return ret;
}

static int lynx_cfg80211_dump_station(struct wiphy *wiphy, struct net_device *dev,
				 int idx, u8 *mac, struct station_info *sinfo)
{
    struct lynx_vif *vif = netdev_priv(dev);
    struct lynx *lnx = vif->lnx;
	struct lynx_sta *sta=NULL;
	int ret = -ENOENT;
	int i=0, count=0;
	if(mp_test_firm)
		return -EINVAL;

    spin_lock_bh(&vif->if_lock);

	for(i=0; i<LYNX_STA_MAX_NUM; i++)
	{
		if(vif->sta_idx_map & (1 << i))
		{
			sta = &lnx->sta_list[i];
			
			if(sta->flags & LYNX_STA_VALID)
			{
				if(count < idx)
					count++;
				else
					break;
			}
		}
	}

	if(i < LYNX_STA_MAX_NUM)
	{
		set_sinfo(lnx, i, sinfo);
		if(mac)
			memcpy(mac, sta->addr, ETH_ALEN);
		ret = 0;
	}

    spin_unlock_bh(&vif->if_lock);

	return ret;
}

#if 0
static int lynx_set_pmksa(struct wiphy *wiphy, struct net_device *netdev,
                struct cfg80211_pmksa *pmksa)
{
    struct lynx_vif *vif = netdev_priv(netdev);

    return lynx_wci_set_pmksa_cmd(vif, pmksa->bssid, pmksa->pmkid, 1);
}

static int lynx_del_pmksa(struct wiphy *wiphy, struct net_device *netdev,
                struct cfg80211_pmksa *pmksa)
{
    struct lynx_vif *vif = netdev_priv(netdev);
    
    return lynx_wci_set_pmksa_cmd(vif, pmksa->bssid, pmksa->pmkid, 0);
}

static int lynx_flush_pmksa(struct wiphy *wiphy, struct net_device *netdev)
{
    struct lynx_vif *vif = netdev_priv(netdev);

    return lynx_wci_set_pmksa_cmd(vif, NULL, NULL, 0);
}
#endif


/* cfg80211 will call the function when interface up (station/p2p gc) base wdev->ps */
/* iw also could sent cmd to set/get wdev->ps .*/
static int lynx_cfg80211_set_power_mgmt(struct wiphy *wiphy,
									struct net_device *dev,
									bool enabled, int timeout)
{
	struct lynx *lnx = wiphy_priv(wiphy);
	struct lynx_vif *vif = lynx_vif_first(lnx);
	if ((vif->wdev.iftype != NL80211_IFTYPE_STATION) && 
		(vif->wdev.iftype != NL80211_IFTYPE_P2P_CLIENT)){
		return -EOPNOTSUPP;
	}
#ifdef CONFIG_ANDROID
	/*Disable Power Saving Support for Android*/
	return 0;
#else
	if (enabled) {
		pr_info("\nEnable Power-Saving\n");
		lynx_vif_cfg(lnx, vif, VIF_CFG_PS, 1);
	} else {
		pr_info("\nDisable Power-Saving\n");
		lynx_vif_cfg(lnx, vif, VIF_CFG_PS, 0);
	}
#endif
	return 0;
}

static int lynx_cfg80211_set_cqm_rssi_config(struct wiphy *wiphy,
					struct net_device *dev,
					s32 rssi_thold, u32 rssi_hyst)
{
	struct lynx *lnx = wiphy_priv(wiphy);
	struct lynx_vif *vif = lynx_vif_first(lnx);

	if(mp_test_firm)
		return 0;

	if(vif->ndev != dev){
		pr_err("Only support first vif for cqm_rssi \n");
		return 0;
	}

	lnx->cqm_rssi_thold = rssi_thold;
	lnx->cqm_rssi_hyst = rssi_hyst;
	lnx->last_cqm_event_rssi = 0;
	lnx->ave_beacon_signal = 0;
	lnx->count_beacon_signal= 0;
	
	pr_err("Set rssi_thold %d ; rssi_hyst %d\n",rssi_thold,rssi_hyst);

	return 0;
}

static struct cfg80211_ops lynx_cfg80211_ops = {
        .add_virtual_intf       = lynx_cfg80211_add_iface,
        .del_virtual_intf       = lynx_cfg80211_del_iface,
        .change_virtual_intf    = lynx_cfg80211_change_iface,
        .scan                   = lynx_cfg80211_scan,
        .connect                = lynx_cfg80211_connect,
        .disconnect             = lynx_cfg80211_disconnect,
        .add_key                = lynx_cfg80211_add_key,
        .get_key                = lynx_cfg80211_get_key,
        .del_key                = lynx_cfg80211_del_key,
        .set_default_key        = lynx_cfg80211_set_default_key,
        .set_tx_power           = lynx_cfg80211_set_txpower,
        .get_tx_power           = lynx_cfg80211_get_txpower,
        .join_ibss              = lynx_cfg80211_join_ibss,
        .leave_ibss             = lynx_cfg80211_leave_ibss,
		/* FIXME: not support pmksa in the device now */
#if 0
        .set_pmksa              = lynx_set_pmksa,
        .del_pmksa              = lynx_del_pmksa,
        .flush_pmksa            = lynx_flush_pmksa,
#endif
        .sched_scan_start       = lynx_cfg80211_sscan_start,
        .sched_scan_stop        = lynx_cfg80211_sscan_stop,
		.set_wiphy_params 		= lynx_cfg80211_set_wiphy_params,
		.start_ap 				= lynx_cfg80211_start_ap,
		.stop_ap 				= lynx_cfg80211_stop_ap,
#if defined(CONFIG_HOST_WPS) || defined(CONFIG_CUST1_PRIV_BEACON_IE)
		.change_beacon			= lynx_cfg80211_change_beacon,
#endif /*CONFIG_HOST_WPS*/
		.del_station 			= lynx_cfg80211_del_station,
		.set_wds_peer 			= lynx_cfg80211_set_wds_peer,
		.get_station 			= lynx_cfg80211_get_station,
		.dump_station 			= lynx_cfg80211_dump_station,

#ifdef CONFIG_SUPPORT_MONITOR_MODE
		.set_monitor_channel = lynx_cfg80211_set_monitor_channel,
#endif /*CONFIG_SUPPORT_MONITOR_MODE */

#ifdef CONFIG_HOST_P2P
		.mgmt_tx					= lynx_cfg80211_mgmt_tx,
		.mgmt_frame_register		= lynx_cfg80211_mgmt_frame_register,
		.start_p2p_device			= lynx_cfg80211_start_p2p_device,
		.stop_p2p_device 			= lynx_cfg80211_stop_p2p_device,
		.remain_on_channel		= lynx_cfg80211_remain_on_channel,
		.cancel_remain_on_channel	= lynx_cfg80211_cancel_remain_on_channel,
#endif /*CONFIG_HOST_P2P*/

		.set_power_mgmt			= lynx_cfg80211_set_power_mgmt,
		.set_cqm_rssi_config		= lynx_cfg80211_set_cqm_rssi_config,
};


void lynx_cfg80211_scan_complete_event(struct lynx_vif *vif, bool aborted)
{
    lynx_dbg(LYNX_DBG_WLAN_CFG, "%s: status%s\n", __FUNCTION__, aborted ? " aborted" : "");

	if( vif->wdev.iftype  == NL80211_IFTYPE_P2P_DEVICE ){
		lynx_dbg(LYNX_DBG_WLAN_CFG,  "Set to orig channel %d after p2p_find !\n",vif->lnx->channel );
		/* wci set channel */
		lynx_wci_set_channel_cmd(vif->lnx);
	}

    if(vif->scan_req)
	{
		info.aborted = aborted;
		cfg80211_scan_done(vif->scan_req, &info);
		vif->scan_req = NULL;
	}
}

void lynx_cfg80211_stop(struct lynx_vif *vif)
{
    lynx_cfg80211_sscan_disable(vif);

    if(test_bit(CONNECTED, &vif->flags))
    {
		if((vif->nw_type == VIF_AP_MODE) || (vif->nw_type == VIF_P2P_GO_MODE))
			cleanup_sta_list(vif);
		else	// VIF_STA_MODE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
			cfg80211_disconnected(vif->ndev, 0, NULL, 0, true, CFG80211_GFP);
#else
			cfg80211_disconnected(vif->ndev, 0, NULL, 0, CFG80211_GFP);
#endif
    }
    else if(test_bit(CONNECT_PEND, &vif->flags))
    {
        cfg80211_connect_result(vif->ndev, vif->req_bssid, NULL, 0, NULL, 0,
                    WLAN_STATUS_UNSPECIFIED_FAILURE, CFG80211_GFP);
    }

    /* FIXME: inform device do disconnect */

    clear_bit(CONNECTED, &vif->flags);
    clear_bit(CONNECT_PEND, &vif->flags);

    lynx_cfg80211_scan_complete_event(vif, true);
}

int lynx_cfg80211_init(struct lynx *lnx)
{
    struct wiphy *wiphy = lnx->wiphy;
    int ret = 0;
    
    /*FIXME:should be know more of them
     * for receiving certain mgmt frame for processing in userspace
     */
    wiphy->mgmt_stypes = lynx_mgmt_stypes;

    wiphy->max_remain_on_channel_duration = 5000; 
    //wiphy->max_remain_on_channel_duration = 0xffff;//5000; 5sec -> 65sec

    /* set device pointer for wiphy */
    set_wiphy_dev(wiphy, lnx->dev);
    
    wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION) | BIT(NL80211_IFTYPE_AP) |
#ifdef CONFIG_HOST_P2P
                             BIT(NL80211_IFTYPE_P2P_DEVICE) |
                             BIT(NL80211_IFTYPE_P2P_CLIENT) |
                             BIT(NL80211_IFTYPE_P2P_GO) |
#endif /*CONFIG_HOST_P2P*/
#ifdef CONFIG_SUPPORT_MONITOR_MODE
                             BIT(NL80211_IFTYPE_MONITOR) |
#endif /*CONFIG_SUPPORT_MONITOR_MODE */
                             BIT(NL80211_IFTYPE_WDS) | BIT(NL80211_IFTYPE_ADHOC);

#ifdef CONFIG_SUPPORT_MONITOR_MODE
    lynx_dbg(LYNX_DBG_ERR, "%s: Support Monitor mode\n", __FUNCTION__);
#else /*CONFIG_SUPPORT_MONITOR_MODE */
lynx_dbg(LYNX_DBG_ERR, "%s: NOT support Monitor mode\n", __FUNCTION__);
#endif /*CONFIG_SUPPORT_MONITOR_MODE */

    /*FIXME:what is it for*/
    /* max num of ssids that can be probed during scanning */
    wiphy->max_scan_ssids = MAX_PROBED_SSIDS;

    /*FIXME: what is correct limit?*/
    wiphy->max_scan_ie_len = 1000;
    
    /*FIXME:
     * maybe we should set wiphy->bands by the above switch case by cap. 
     * ath6k is doing as switch case.
     * currently we fixed wiphy->bands
     */
    wiphy->bands[NL80211_BAND_2GHZ] = &lynx_band_2ghz;


    wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;
    wiphy->cipher_suites = cipher_suites;
    wiphy->n_cipher_suites = ARRAY_SIZE(cipher_suites);
    
    /*
     * WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD: When operating as an AP, the device responds to probe-requests in hardware.
     */
    wiphy->flags |= WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD | 
#ifdef  CONFIG_ANDROID
                    WIPHY_FLAG_HAVE_AP_SME | 
#endif /*CONFIG_ANDROID */
#ifdef CONFIG_SUPPORT_MONITOR_MODE 
                    WIPHY_FLAG_HAVE_AP_SME | 
#endif /*CONFIG_SUPPORT_MONITOR_MODE */
                    WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL;
    
    /*FIXME:what is it for*/
    wiphy->probe_resp_offload =
#ifdef CONFIG_ANDROID
        /*set offload ie by lynx_android_set_wps_p2p_ie()*/
        NL80211_PROBE_RESP_OFFLOAD_SUPPORT_P2P |
#endif
        NL80211_PROBE_RESP_OFFLOAD_SUPPORT_WPS |
        NL80211_PROBE_RESP_OFFLOAD_SUPPORT_WPS2;

    /* allowed interface combinations */
    wiphy->iface_combinations     = lynx_iface_combinations;
    wiphy->n_iface_combinations   = ARRAY_SIZE(lynx_iface_combinations);

    ret = wiphy_register(wiphy);

    if(ret < 0) 
        lynx_dbg(LYNX_DBG_WLAN_CFG, "couldn't register wiphy device\n");

    return ret;
}

void lynx_cfg80211_cleanup(struct lynx *lnx)
{
    if(lnx->wiphy && lnx->wiphy->registered) 
	{
        wiphy_unregister(lnx->wiphy);
    }
}

struct lynx *lynx_cfg80211_create(void)
{
    struct lynx *lnx=NULL;
    struct wiphy *wiphy=NULL;

    /* create a new wiphy for use with cfg80211 */
    wiphy = wiphy_new(&lynx_cfg80211_ops, sizeof(struct lynx));

	if(wiphy) {
		lnx = wiphy_priv(wiphy);
		lnx->wiphy = wiphy;
/*Overwrite CPTCFG_CFG80211_DEFAULT_PS*/
#ifdef CONFIG_LYNX_POWER_SAVING 
		/* Trigger cfg80211 set wdev->ps as true;*/
		/* cfg80211 will call lynx_cfg80211_set_power_mgmt() when interface up*/
		wiphy->flags |= WIPHY_FLAG_PS_ON_BY_DEFAULT;
#else
		wiphy->flags &= ~WIPHY_FLAG_PS_ON_BY_DEFAULT;
#endif /*CONFIG_LYNX_POWER_SAVING */
	}else{
		lynx_dbg(LYNX_DBG_WLAN_CFG, "couldn't allocate wiphy device\n");
	}

    return lnx;
}

void lynx_cfg80211_destroy(struct lynx *lnx)
{
	struct wiphy *wiphy=NULL;
	if(lnx && lnx->wiphy)
	{
		wiphy=lnx->wiphy;
		lnx->wiphy = NULL;
		wiphy_free(wiphy);
	}
}
