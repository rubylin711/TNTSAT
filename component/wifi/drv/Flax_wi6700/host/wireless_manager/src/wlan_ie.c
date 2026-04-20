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
#include "ieee80211.h"
#include "mac_ctrl.h"
#include "lynx_debug.h"
#include <wm_vif.h>
#include <wm_sta.h>
#include <wm_mlme.h>
#include <wm_msg.h>
#include <wlan_ie.h>
#ifdef CONFIG_WPA
#include <wpa.h>
#endif

/*=============================================================================+
| Define
+=============================================================================*/
u16 rates_in_100kbps[] = {
	10, 
	20,
	55,
	110, 
	60,
	90,
	120,
	180,
	240,
	360,
	480,
	540,
};

/*=============================================================================+
| Functions
+=============================================================================*/
static int cipher_suite_to_bit(const u8 *s)
{
	u32 suite;
	suite = READ_BE32((unsigned char *)s);

	if(suite == RSN_CIPHER_SUITE_WEP40)
		return AUTH_CAP_CIPHER_WEP40;
	if(suite== RSN_CIPHER_SUITE_TKIP)
		return AUTH_CAP_CIPHER_TKIP;
	if(suite == RSN_CIPHER_SUITE_CCMP)
		return AUTH_CAP_CIPHER_CCMP;
	if(suite== RSN_CIPHER_SUITE_WEP104)
		return AUTH_CAP_CIPHER_WEP104;
	if(suite == RSN_CIPHER_SUITE_AES_128_CMAC)
		return AUTH_CAP_CIPHER_AES_128_CMAC;

	if(suite == WPA_CIPHER_SUITE_WEP40)
		return AUTH_CAP_CIPHER_WEP40;
	if(suite == WPA_CIPHER_SUITE_TKIP)
		return AUTH_CAP_CIPHER_TKIP;
	if(suite == WPA_CIPHER_SUITE_CCMP)
		return AUTH_CAP_CIPHER_CCMP;
	if(suite == WPA_CIPHER_SUITE_WEP104)
		return AUTH_CAP_CIPHER_WEP104;
#ifdef CONFIG_WAPI
	if(suite == WPI_CIPHER_SUITE_SMS4)
		return AUTH_CAP_CIPHER_SMS4;
#endif

	return 0;
}

static int akm_suite_to_bit(const u8 *s)
{
	u32 suite;
	suite = READ_BE32((unsigned char *)s);

	if(suite == RSN_AKM_SUITE_1X)
		return AUTH_CAP_KEY_MGT_1X;
	if(suite == RSN_AKM_SUITE_PSK)
		return AUTH_CAP_KEY_MGT_PSK;
	if(suite == RSN_AKM_SUITE_FT_1X)
		return AUTH_CAP_KEY_MGT_FT_1X;
	if(suite == RSN_AKM_SUITE_FT_PSK)
		return AUTH_CAP_KEY_MGT_FT_PSK;
	if(suite == RSN_AKM_SUITE_1X_SHA256)
		return AUTH_CAP_KEY_MGT_1X_SHA256;
	if(suite == RSN_AKM_SUITE_PSK_SHA256)
		return AUTH_CAP_KEY_MGT_FT_PSK;

	if(suite == WPA_AKM_SUITE_1X)
		return AUTH_CAP_KEY_MGT_1X;
	if(suite == WPA_AKM_SUITE_PSK)
		return AUTH_CAP_KEY_MGT_PSK;
#ifdef CONFIG_WAPI
	if(suite == WAI_CIPHER_SUITE_1X)
		return AUTH_CAP_KEY_MGT_1X_SHA256;
	if(suite == WAI_CIPHER_SUITE_PSK)
		return AUTH_CAP_KEY_MGT_PSK_SHA256;
#endif
	return 0;
}


int wlan_parse_wpa_rsn_ie(const u8 *start, u32 len, struct wpa_ie_data *info, u8 is_rsn)
{
	u8 *pos;
	u32 i, count, left;
	u16 version;

	memset(info, 0, sizeof(*info));
	info->pairwise_cipher = AUTH_CAP_CIPHER_CCMP;
	info->group_cipher = AUTH_CAP_CIPHER_CCMP;
	info->key_mgmt = AUTH_CAP_KEY_MGT_1X;
	info->capabilities = 0;
	info->pmkid = NULL;
	info->num_pmkid = 0;
#ifdef CONFIG_IEEE80211W
	info->mgmt_group_cipher = WPA_CIPHER_AES_128_CMAC;
#else /* CONFIG_IEEE80211W */
	info->mgmt_group_cipher = 0;
#endif /* CONFIG_IEEE80211W */

	pos = (u8 *) start;
	left = len;
	/* check version */
	if(is_rsn)
	{
		info->proto = AUTH_CAP_WPA2; 
		version = RSN_VERSION;
	}
	else
	{
		info->proto = AUTH_CAP_WPA;
		version = WPA_VERSION;
	}
 
	if(READ_LE16(pos) != version)
	{
		lynx_dbg(LYNX_DBG_WM, "%s(): unknown RSN version=%x\n", __func__, READ_LE16(pos));
		return -2;
	}
	pos += 2;
	left -= 2;

	MIN_LENGTH_VERIFY(left, SELECTOR_LEN, return WLAN_RET_ERROR);

	/* get group cipher suite */
	info->group_cipher = cipher_suite_to_bit(pos);

	pos += SELECTOR_LEN;
	left -= SELECTOR_LEN;

	/* get pairwise cipher suite list */
	info->pairwise_cipher = 0;
	count = READ_LE16(pos);
	pos += 2;
	left -= 2;
	if (count == 0 || left < count * SELECTOR_LEN) 
	{
		lynx_dbg(LYNX_DBG_WM, "%s: ie count botch (pairwise), count %u left %u", __func__, count, left);
		return WLAN_RET_ERROR;
	}
	for (i = 0; i < count; i++) 
	{
		info->pairwise_cipher |= cipher_suite_to_bit(pos);
	
		pos += SELECTOR_LEN;
		left -= SELECTOR_LEN;
	}
#ifdef CONFIG_IEEE80211W
	if((is_rsn) && (info->pairwise_cipher & WPA_CIPHER_AES_128_CMAC)) 
	{
		lynx_dbg(LYNX_DBG_WM, "%s: AES-128-CMAC used as pairwise cipher", __func__);
		return WLAN_RET_ERROR;	
	}
#endif /* CONFIG_IEEE80211W */

	/* get AKM suite list */
	info->key_mgmt = 0;
	count = READ_LE16(pos);
	pos += 2;
	left -= 2;
	if(count == 0 || left < count * SELECTOR_LEN)
	{
		lynx_dbg(LYNX_DBG_WM, "%s: ie count botch (key mgmt), count %u left %u", __func__, count, left);
		return WLAN_RET_ERROR;
	}
	for(i = 0; i < count; i++) 
	{
		info->key_mgmt |= akm_suite_to_bit(pos);

		pos += SELECTOR_LEN;
		left -= SELECTOR_LEN;
	}

	/* get RSN capability */	
	info->capabilities = READ_LE16(pos);
	pos += 2;
	left -= 2;

	if((is_rsn) && (left > 2)) /* 2 == pmk_num info bytes */
	{
		/* get PMKID list */
		info->num_pmkid = READ_LE16(pos);
		pos += 2;
		left -= 2;
		if (left < (int) info->num_pmkid * WLAN_PMKID_LEN) 
		{
			lynx_dbg(LYNX_DBG_WM, "%s: PMKID underflow (num_pmkid=%lu left=%d)\n", __func__, (unsigned long) info->num_pmkid, left);
			info->num_pmkid = 0;
			return WLAN_RET_ERROR;
		} 
		info->pmkid = pos;
		pos += info->num_pmkid * WLAN_PMKID_LEN;
		left -= info->num_pmkid * WLAN_PMKID_LEN;

	}
	return 0;
}

#ifdef CONFIG_WAPI
int wlan_parse_wapi_ie(const u8 *start, u32 len, void *buf)
{
	struct wpa_ie_data *info;
	u8 *pos;
	u32 i, count, left;

	memset(buf, 0, sizeof(struct wpa_ie_data));
	info = (struct wpa_ie_data *)buf;

	pos = (u8 *) start;
	left = len;
	/* check version */
	info->proto = AUTH_CAP_WAPI;
 
	if(READ_LE16(pos) != WAPI_VERSION)
	{
		lynx_dbg(LYNX_DBG_WM, "%s(): unknown WAPI version=%x\n", __func__, READ_LE16(pos));
		return -2;
	}
	pos += 2;
	left -= 2;

	MIN_LENGTH_VERIFY(left, SELECTOR_LEN, return WLAN_RET_ERROR);

	/* get AKM suite list */
	count = READ_LE16(pos);
	pos += 2;
	left -= 2;
	if(count == 0 || left < count * SELECTOR_LEN)
	{
		lynx_dbg(LYNX_DBG_WM, "%s: ie count botch (key mgmt), count %u left %u", __func__, count, left);
		return WLAN_RET_ERROR;
	}
	for(i = 0; i < count; i++) 
	{
		info->key_mgmt |= akm_suite_to_bit(pos);

		pos += SELECTOR_LEN;
		left -= SELECTOR_LEN;
	}

	/* get pairwise cipher suite list */
	count = READ_LE16(pos);
	pos += 2;
	left -= 2;
	if (count == 0 || left < count * SELECTOR_LEN) 
	{
		lynx_dbg(LYNX_DBG_WM, "%s: ie count botch (pairwise), count %u left %u", __func__, count, left);
		return WLAN_RET_ERROR;
	}
	for (i = 0; i < count; i++) 
	{
		info->pairwise_cipher |= cipher_suite_to_bit(pos);
	
		pos += SELECTOR_LEN;
		left -= SELECTOR_LEN;
	}

	/* get group cipher suite */
	info->group_cipher = cipher_suite_to_bit(pos);

	pos += SELECTOR_LEN;
	left -= SELECTOR_LEN;

	/* get WAPI capability */	
	info->capabilities = READ_LE16(pos);
	pos += 2;
	left -= 2;

	/* get BKID list */
	info->num_pmkid = READ_LE16(pos);
	pos += 2;
	left -= 2;
	if (left < (int) info->num_pmkid * WAI_BKID_LEN) 
	{
		lynx_dbg(LYNX_DBG_WM, "%s: BKID underflow (num_pmkid=%lu left=%d)\n", __func__, (unsigned long) info->num_pmkid, left);
		info->num_pmkid = 0;
		return WLAN_RET_ERROR;
	} 
	info->pmkid = pos;
	pos += info->num_pmkid * WAI_BKID_LEN;
	left -= info->num_pmkid * WAI_BKID_LEN;
	return 0;
}
#endif

u8 high_prioity_bit(u32 flag, u8 upbound)
{
	u8 i, max;
	for(i = max = 0; i < upbound; i++)
	{
		if(flag & (1 << i))
			max = i;
	}
	return max;
}

u8 low_prioity_bit(u32 flag, u8 lowbound)
{
	u8 i, min=0xff;
	for(i = lowbound; i < 8; i++)
	{
		if(flag & (1 << i))
		{
			min = i;
			break;
		}
	}
	return min;
}

u32 wlan_parse_ie(u8 *pos, u32 left, struct wlan_ie *ie)
{
	
	while(left >= 2) 
	{
		struct wlan_ie_generic *one = (struct wlan_ie_generic *)pos;
		
		MIN_LENGTH_VERIFY(left, one->len, return WLAN_RET_ERROR);

		switch (one->id) 
		{
			case WLAN_ELEMID_SSID:
				ie->ssid = one->data;
				break;
			case WLAN_ELEMID_SUPP_RATES:
				MAX_LENGTH_VERIFY(one->len, 32, return WLAN_RET_ERROR);
				ie->supp_rates = one->data;
				break;
			case WLAN_ELEMID_DS_PARMS:
				ie->ds_params = one->data;
				break;
			case WLAN_ELEMID_TIM:
				ie->tim = one->data;
				break;
			case WLAN_ELEMID_CHALLENGE:
				ie->challenge = one->data;
				break;
			case WLAN_ELEMID_ERP_INFO:
				ie->erp_info = one->data;
				break;
			case WLAN_ELEMID_EXT_SUPP_RATES:
				MAX_LENGTH_VERIFY(one->len, 32, return WLAN_RET_ERROR);
				ie->ext_supp_rates = one->data;
				break;
			case WLAN_ELEMID_RSN:
				ie->rsn_ie = one->data;
				break;
			case WLAN_ELEMID_PWR_CAP:
				ie->power_cap = one->data;
				break;
			case WLAN_ELEMID_SUPP_CHAN:
				ie->supp_channels = one->data;
				break;
			case WLAN_ELEMID_MOBILITY_DOMAIN:
				ie->mdie = one->data;
				break;
			case WLAN_ELEMID_FAST_BSS_TRANS:
				ie->ftie = one->data;
				break;
			case WLAN_ELEMID_TIMEOUT_INTVAL:
				ie->timeout_int = one->data;
				break;
			case WLAN_ELEMID_HT_CAP:
				MIN_LENGTH_VERIFY(one->len, (sizeof(struct wlan_ie_ht_capability) - 2), goto skip);
				ie->ht_capabilities = one->data;
				break;
			case WLAN_ELEMID_HT_INFO:
				ie->ht_operation = one->data;
				break;
#ifdef CONFIG_WAPI
			case WLAN_ELEMID_WAPI:
				ie->wapi_ie = one->data;
				break;
#endif
			case WLAN_ELEMID_VENDOR_SPEC:
				{
					unsigned int oui;

					/* OUI(3 bytes) + type(1 byte) */
					MIN_LENGTH_VERIFY(one->len, 4, goto skip);
					oui = READ_BE24(one->data);
					switch(oui) 
					{
						case MICROSOFT_OUI:
						/* Microsoft/Wi-Fi information elements are further typed and subtyped */
						switch(one->data[3])
						{
							case WPA_OUI_TYPE: /* WPA IE */
								ie->wpa_ie = one->data;
								break;
							case WME_OUI_TYPE: /* this is a Wi-Fi WME info. element */
								MIN_LENGTH_VERIFY(one->len, 5, return WLAN_RET_ERROR);
								switch(one->data[4])
								{
									case WME_INFO_OUI_SUBTYPE:
									case WME_PARAM_OUI_SUBTYPE:
										ie->wme = one->data;
										break;
									case WME_TSPEC_OUI_SUBTYPE:
										//ie->wme_tspec = one->data;
										break;
									default:
										lynx_dbg(LYNX_DBG_WM, "Unknown WME IE(subtype=%d)\n", one->data[4]);
										goto skip;
								}
								break;
							case WPS_OUI_TYPE: /* Wi-Fi Protected Setup (WPS) IE */
								ie->wps_ie = one->data;
								break;
							default:
								//lynx_dbg(LYNX_DBG_WM, "Unknown MS IE(type=%d)\n", one->data[3]);
								goto skip;
						}
						break;
#ifdef CONFIG_WLAN_P2P_MODE
						case WFA_SPECIFIC_OUI:
						{
							switch(one->data[3])
							{
								case WFA_P2P_OUI_SUBTYPE:
									ie->p2p_ie = (u8 *)one;
									break;
							}
							break;
						}
#endif // CONFIG_WLAN_P2P_MODE
						default:
							//lynx_dbg(LYNX_DBG_WM, "Unknown vendor IE(OUI=%02x:%02x:%02x)\n", one->data[0], one->data[1], one->data[2]);
							goto skip;
					}
					break;
				}
#ifdef CONFIG_LYNX_IBSS
			case WLAN_ELEMID_IBSS_PARMS:
				ie->ibss_ie = one->data;
				break;
#endif
			default:
				//lynx_dbg(LYNX_DBG_WM,"Unknown IE(id=%d len=%d)\n", one->id, one->len);
				break;
		}
skip:
		left -= one->len + 2;
		pos += one->len + 2;
	}

	if(left)
		return WLAN_RET_ERROR;

	return WLAN_RET_OK;
}

short reverse_ecw(char i)
{
	short cw=0;
	
	cw = (1 << i) - 1;

	return cw;
}

int wlan_parse_wme_ie(struct wm_vif *bss, const u8 *start, u32 len, u32 survey)
{
	struct wlan_ie_wme_param *wme_param = (struct wlan_ie_wme_param *)(start - 2);
	struct neighbor_bss *nbss;
	struct wme_ie_data *wme_info;
	int count, i, wme_acm;

	if(!bss)
		return WLAN_RET_ERROR;
	if((len < 8) || (wme_param->version != 1))
		return WLAN_RET_ERROR;

	if(!my_wlan_dev->ap_exist && !survey)
	{
		count = wme_param->qosinfo & 0x0f;
		if(count == my_wlan_dev->parameter_set_count)
			return WLAN_RET_ERROR;
		my_wlan_dev->parameter_set_count = count;
		lynx_dbg(LYNX_DBG_WM, "wme acp cnt update(%d)\n", my_wlan_dev->parameter_set_count);
	}

	nbss = (struct neighbor_bss *)bss;
	wme_info = nbss->wme_info;
	wme_acm = 0;
	for(i=0; i<WME_NUM_AC; i++)
	{
		struct wme_acparams	*ac_params = &wme_param->ac_params[i];
		struct wlan_wme_ac_params acp;
		u8 acm = ac_params->acm;
		u16 qidx = 0;

		switch(ac_params->aci)
		{
		case 1:	/* AC_BK */
			qidx = 0;
			if(acm)
				wme_acm |= BIT(1) | BIT(2); /* BK/- */
			break;
		case 2: /* AC_VI */
			qidx = 2;
			if(acm)
				wme_acm |= BIT(4) | BIT(5); /* CL/VI */
			break;
		case 3: /* AC_VO */
			qidx = 3;
			if(acm)
				wme_acm |= BIT(6) | BIT(7); /* VO/NC */
			break;
		case 0: /* AC_BE */
		default:
			qidx = 1;
			if(acm)
				wme_acm |= BIT(0) | BIT(3); /* BE/EE */
			break;
		}

		acp.qidx = qidx;
		acp.cwmin = ecw2cw(ac_params->ecwmin);
		acp.cwmax = ecw2cw(ac_params->ecwmax);
		acp.aifs = ac_params->aifsn;
		acp.txoplimit = ac_params->txop;

		if(survey)
		{
			memcpy(&wme_info->wme_ac_params[i], &acp, sizeof(struct wlan_wme_ac_params));
		}
		else
		{
			/* STA do not update AC parameters if AP exist */
			if(my_wlan_dev->ap_exist)
				continue;
			/* TODO: set ac param */
#if 0
			wla_ac_paramters(acp.qidx, acp.cwmax, acp.cwmin, acp.aifs, acp.txoplimit);
#endif
		}
	}

	if(survey)
		wme_info->wme_acm = wme_acm;
#if 0
	else
		wla_cfg(SET_WME_ACM, bss->bss_desc, wme_acm);
#endif

	return WLAN_RET_OK;
}

u16 ap_encode_wpa_ie(struct wm_vif *bss, u8 *start, u8 is_rsn)
{
	u32 num_suites, val;
	u8 *pos, *count;
	int bypass=0;

	pos = start;
	if(!is_rsn)
	{
		WRITE_BE32(pos, WPA_AKM_SUITE_1X);
		pos += 4;
	}

	/* rsn version is same as wpa version */
	//printk("%s(%d): WPA_VERSION : %x, (u16)=%x, (((u16) (v)) >> 8)=%x, (((u16) (v)) & 0xff)=%x\n", __FUNCTION__, __LINE__, WPA_VERSION, (u16)WPA_VERSION, (((u16) (WPA_VERSION)) >> 8), (((u16) (WPA_VERSION)) & 0xff));
	WRITE_LE16(pos, WPA_VERSION);
	pos += 2;

	/* fill in group cipher */
	if(bss->auth_capability & AUTH_CAP_CIPHER_TKIP) 
	{
		if(!is_rsn)
			val = WPA_CIPHER_SUITE_TKIP;
		else
			val = RSN_CIPHER_SUITE_TKIP;
		WRITE_BE32(pos, val);
	} 
	else if(bss->auth_capability & AUTH_CAP_CIPHER_CCMP) 
	{
		if(!is_rsn)
			val = WPA_CIPHER_SUITE_CCMP;
		else
			val = RSN_CIPHER_SUITE_CCMP;
		WRITE_BE32(pos, val);
	} 
	else if(bss->auth_capability & AUTH_CAP_CIPHER_WEP104) 
	{
		if(!is_rsn)
			val = WPA_CIPHER_SUITE_WEP104;
		else
			val = RSN_CIPHER_SUITE_WEP104;
		WRITE_BE32(pos, val);
	} 
	else if(bss->auth_capability & AUTH_CAP_CIPHER_WEP40) 
	{
		if(!is_rsn)
			val = WPA_CIPHER_SUITE_WEP40;
		else
			val = RSN_CIPHER_SUITE_WEP40;
		WRITE_BE32(pos, val);
	} 
	else 
			return 0;
	pos += SELECTOR_LEN;

	num_suites = 0;
	count = pos;
	pos += 2;

	/* fill in pairwise cipher */
	if(bss->auth_capability & AUTH_CAP_CIPHER_CCMP) 
	{
		if(!is_rsn)
			val = WPA_CIPHER_SUITE_CCMP;
		else
			val = RSN_CIPHER_SUITE_CCMP; 
		WRITE_BE32(pos, val);

		pos += SELECTOR_LEN;
		num_suites++;

		if(bss->role == WIF_STA_ROLE)
			bypass = 1; /* only set one pairwise in STA role */
	}
	if((bss->auth_capability & AUTH_CAP_CIPHER_TKIP) && !bypass)
	{
		if(!is_rsn)
			val = WPA_CIPHER_SUITE_TKIP;
		else
			val = RSN_CIPHER_SUITE_TKIP;
		WRITE_BE32(pos, val);

		pos += SELECTOR_LEN;
		num_suites++;
	}

	/* ??? */
	if((bss->auth_capability & ALL_CIPHER) == 0) 
	{
		if(!is_rsn)
			val = WPA_CIPHER_SUITE_NONE;
		else
			val = RSN_CIPHER_SUITE_NONE;
		WRITE_BE32(pos, val);

		pos += SELECTOR_LEN;
		num_suites++;
	}

	if(num_suites == 0) 
		return 0;
	WRITE_LE16(count, num_suites);

	num_suites = 0;
	count = pos;
	pos += 2;

	/* fill in key management */
	if(bss->auth_capability & AUTH_CAP_KEY_MGT_1X) 
	{
		if(!is_rsn)
			val = WPA_AKM_SUITE_1X;
		else
			val = RSN_AKM_SUITE_1X;
		WRITE_BE32(pos, val);

		pos += SELECTOR_LEN;
		num_suites++;
	}
	if(bss->auth_capability & AUTH_CAP_KEY_MGT_PSK)
	{
		if(!is_rsn)
			val = WPA_AKM_SUITE_PSK;
		else
			val = RSN_AKM_SUITE_PSK;
		WRITE_BE32(pos, val);

		pos += SELECTOR_LEN;
		num_suites++;
	}

	if(num_suites == 0)
		return 0;
	WRITE_LE16(count, num_suites);

	if(is_rsn)
	{
		/* RSN capability */
		WRITE_LE16(pos, 0);
		pos += 2;

		/* PMKID */


	}

	return (pos - start); 
}

#ifdef CONFIG_WAPI
u16 ap_encode_wapi_ie(u8 *start, u8 *bkid)
{
	u8 *pos;

	pos = start;
	WRITE_LE16(pos, WAPI_VERSION);
	pos += 2;

	WRITE_LE16(pos, 1);
	pos += 2;

	WRITE_BE32(pos, WAI_CIPHER_SUITE_PSK);
	pos += SELECTOR_LEN;

	WRITE_LE16(pos, 1);
	pos += 2;

	WRITE_BE32(pos, WPI_CIPHER_SUITE_SMS4);
	pos += SELECTOR_LEN;

	WRITE_BE32(pos, WPI_CIPHER_SUITE_SMS4);
	pos += SELECTOR_LEN;

	/* WAPI capabilities */
	WRITE_LE16(pos, 0);
	pos += 2;

	if(bkid)
	{
		if(pos + 2 + WAI_BKID_LEN > start + (255 - 2))
			return -1;
		/* BKID Count */
		WRITE_LE16(pos, 1);
		pos += 2;
		memcpy(pos, bkid, WAI_BKID_LEN);
		pos += WAI_BKID_LEN;
	}
	else
	{
		WRITE_LE16(pos, 0);
		pos += 2;
	}
	return pos - start;
}
#endif

#ifdef CONFIG_WPS
void ap_set_wps_ie(struct wm_vif *bss, u8 *start1, u32 len1, u8 *start2, u32 len2)
{
	if (bss->wps_ie)
		free(bss->wps_ie);
	if (bss->wps_ie2)
		free(bss->wps_ie2);
	bss->wps_ie = 0;
	bss->wps_ie2 = 0;

	if (start1 && len1) {
		bss->wps_ie = malloc(len1);
		if (bss->wps_ie) {
			memcpy(bss->wps_ie, start1, len1);
			bss->wps_ie_len = len1;
		}
		else
			printd("%s(%d)alloc wps_ie failed\n", __func__, __LINE__);
	}
	
	if (start2 && len2) {
		bss->wps_ie2 = malloc(len2);
		if (bss->wps_ie2) {
			memcpy(bss->wps_ie2, start2, len2);
			bss->wps_ie2_len = len2;
		}
		else
			printd("%s(%d)alloc wps_ie2 failed\n", __func__, __LINE__);
	}

	//ap_prepare_beacon(bss);
}
#endif

char cw2ecw(short cw)
{
	char i;
	for(i = 0; i < 10; i++)
	{
		if(cw < (1 << i))
			break;
	}
	return i;
}

short ecw2cw(u8 ecw)
{
	return (1 << ecw) - 1;
}

u32 wlan_ap_encode_ie(struct wm_vif *bss, u8 *start, u32 encode_bits)
{
	u32 total_len, ie_len, mask;
	u32 which;
	u8 *pos = start;

	total_len = 0;
	mask = 1;
	while(1)
	{
		ie_len = 0;
		if(mask == IE_END_BIT)
			break;
		which = encode_bits & mask;
		switch(which)
		{
#if 0
			case IE_SSID_BIT:
			{
				((struct wlan_ie_generic *)pos)->id = WLAN_ELEMID_SSID;
				memcpy(((struct wlan_ie_generic *)pos)->data, bss->ssid, bss->ssid_len);
				ie_len = 2 + bss->ssid_len;
				break; 
			}
			case IE_SUPP_RATES_BIT:
			case IE_EXT_SUPP_RATES_BIT:
			{
				u32 val, num = 0;
	
				((struct wlan_ie_generic *)pos)->id = WLAN_ELEMID_SUPP_RATES;
				ie_len = 2;

				for(i=LOWEST_BITRATE; i<HT_LOWEST_BITRATE; i++)
				{
					val = R_BIT(i);
					if((my_wlan_dev->current_rates & val) == 0)
						continue;
					num++;
					if(num > 8)
					{
						if(which == IE_SUPP_RATES_BIT)
							break;
					}
					else
					{
						if(which == IE_EXT_SUPP_RATES_BIT)
							continue;
					}
						
					pos[ie_len] = rates_in_100kbps[i-1]/5;
					if(my_wlan_dev->basic_rates & val)
						pos[ie_len]  |= 0x80;
					ie_len++;
				}

				if(which == IE_EXT_SUPP_RATES_BIT)
				{
					if(num <= 8)
						ie_len = 0;	
					else
						((struct wlan_ie_generic *)pos)->id = WLAN_ELEMID_EXT_SUPP_RATES;
				}

				break;
			}
			case IE_DS_PARMS_BIT:
			{
				((struct wlan_ie_generic *)pos)->id = WLAN_ELEMID_DS_PARMS;
				((struct wlan_ie_generic *)pos)->data[0] =  my_wlan_dev->channel;
				ie_len = 3;
				break;
			}
			case IE_COUNTRY_BIT:
			{
				u32 num;
				struct wlan_channel_info *curr, *prev;
				struct wlan_country_str *cstr;

				((struct wlan_ie_generic *)pos)->id = WLAN_ELEMID_COUNTRY;
				/* country string length always have 3 bytes */
				memcpy(((struct wlan_ie_generic *)pos)->data, my_wlan_dev->country,3);
				ie_len = 5;

				cstr = (struct wlan_country_str *)(pos+ie_len);
				num = 0;
				prev = NULL;
				for(i=0, curr = &my_wlan_dev->channels[i]; i<my_wlan_dev->channel_num; 
					curr = &my_wlan_dev->channels[++i])
				{
					if(!prev)
					{
						cstr->first = i+1; 
					}
					else if(prev && ((curr->flag & WLAN_CHAN_DISABLED) ||
								(curr->max_txpower != prev->max_txpower)))
					{
						cstr->num = num;
						cstr->max_txpower = prev->max_txpower;
						ie_len += 3;
					
						/* next country string */	
						cstr = (struct wlan_country_str *)(pos+ie_len);
						cstr->first = i+1;
						num = 0;
					}
					prev = curr;
					num++;
				}
			
				if(prev)
				{
					cstr->num = num;
					cstr->max_txpower = prev->max_txpower;
					ie_len += 3;
				}

				break;
			}
			case IE_ERP_INFO_BIT:
			{
				u8 erp = 0;
				if((my_wlan_dev->phy_cap & (AP_CAP_11N|AP_CAP_11G)) == 0)
					break;

				((struct wlan_ie_generic *)pos)->id = WLAN_ELEMID_ERP_INFO;

				switch(my_wlan_dev->cts_protection_type) 
				{
					case CTS_PROTECTION_FORCE_ENABLED:
						erp |= WLAN_ERP_NON_ERP_PRESENT | WLAN_ERP_USE_PROTECTION;
						break;
					case CTS_PROTECTION_FORCE_DISABLED:
						erp = 0;
						break;
					case CTS_PROTECTION_AUTOMATIC:
						if (my_wlan_dev->obss_non_erp[0])
							erp |= WLAN_ERP_USE_PROTECTION;
						if(my_wlan_dev->num_sta_non_erp[0] > 0) 
						{
							erp |= WLAN_ERP_NON_ERP_PRESENT |
							WLAN_ERP_USE_PROTECTION;
						}
						break;
					default :
						break;
				}
				if(my_wlan_dev->num_sta_no_short_preamble[0] > 0)
					erp |= WLAN_ERP_BARKER_PREAMBLE;

				((struct wlan_ie_generic *)pos)->data[0] = erp;
				ie_len = 3;

				break;
			}
#endif
#ifdef CONFIG_WPA
			case IE_WPA_BIT:
			{
				if(!(bss->auth_capability & AUTH_CAP_WPA))
					break;

				((struct wlan_ie_generic *)pos)->id = WLAN_ELEMID_VENDOR_SPEC;
				memcpy(((struct wlan_ie_generic *)pos)->data, bss->wpa_ie, bss->wpa_ie_len);
				ie_len = bss->wpa_ie_len + 2;
				break;
			}
#endif
#ifdef CONFIG_WAPI
			case IE_WAPI_BIT:
			{
				if(!(bss->auth_capability & AUTH_CAP_WAPI))
					break;
				((struct wlan_ie_generic *)pos)->id = WLAN_ELEMID_WAPI;
				memcpy(((struct wlan_ie_generic *)pos)->data, bss->wpa_ie, bss->wpa_ie_len);
				ie_len = bss->wpa_ie_len + 2;
				break;
			}
#endif
#ifdef CONFIG_WPA
			case IE_RSN_BIT:
			{
				if(!(bss->auth_capability & AUTH_CAP_WPA2))
					break;

				((struct wlan_ie_generic *)pos)->id = WLAN_ELEMID_RSN;
				memcpy(((struct wlan_ie_generic *)pos)->data, bss->rsn_ie, bss->rsn_ie_len);
				ie_len = bss->rsn_ie_len + 2;
				break;
			}
#endif
#if 0
			case IE_HT_CAP_BIT:
			{
				struct wlan_ie_ht_capability *cap = (struct wlan_ie_ht_capability *)pos;
				
				if(!(my_wlan_dev->phy_cap & AP_CAP_11N))
					break;

				cap->id = WLAN_ELEMID_HT_CAP;
				/* for station mode*/
				if(bss->role == WIF_STA_ROLE)
				{
					cap->ht_cap.capabilities_info = htows(bss->ht_capability);
				}
				else
				{
					cap->ht_cap.capabilities_info = htows(my_wlan_dev->ht_capability);
				}
				cap->ht_cap.ampdu_params = my_wlan_dev->ampdu_params;
				cap->ht_cap.supported_mcs_set[0] = 0xff & (my_wlan_dev->current_rates >> 12);
				cap->ht_cap.supported_mcs_set[4] = 0x1; /* support MCS 32 */

				ie_len = sizeof(*cap);
				break;
			}
			case IE_HT_INFO_BIT:
			{
				struct wlan_ie_ht_info *info = (struct wlan_ie_ht_info *)pos;

				if(!(my_wlan_dev->phy_cap & AP_CAP_11N))
					break;
				info->id = WLAN_ELEMID_HT_INFO;

				info->primary_channel = my_wlan_dev->channel;
				info->operation_mode = htows(my_wlan_dev->ht_op_mode);
				info->param = WLAN_HTINFO_RIFSMODE_PERM;
				if(my_wlan_dev->secondary_channel)
					info->param |= (my_wlan_dev->secondary_channel|WLAN_HTINFO_TXWIDTH); 
				ie_len = sizeof(*info);

				break;
			}
			case IE_WMM_BIT:
			{
				struct wlan_ie_wme_param  *wme = (struct wlan_ie_wme_param *)pos;
				struct wme_acparams *ac;
				struct wlan_wme_ac_params *wme_ac_params=NULL;
				struct wlan_wme_ac_params *acp;
				u32 i;

				if(!(bss->flag & VIF_FLG_WMM))
					break;
				/* for station mode*/
				if(bss->role == WIF_STA_ROLE)
				{
#ifdef CONFIG_WLAN_CLIENT_PS
					u8 ps_policy = wla_get_ps_policy(bss->bss_desc);
					wme->qosinfo = (ps_policy & 0xf); // max sp length = 0
#endif //CONFIG_WLAN_CLIENT_PS
					wme->qosinfo |= 0x80;			  // more data ack = 1
					wme->subtype = WME_INFO_OUI_SUBTYPE;
					ie_len  = sizeof(struct wlan_ie_wme_info);
				}
				else
				{
					wme_ac_params = &my_wlan_dev->wme_ac_params[0];
					wme->subtype = WME_PARAM_OUI_SUBTYPE;
					wme->qosinfo = (my_wlan_dev->parameter_set_count & 0xf) | 
								((my_wlan_dev->power_saving_mode >= 2)?WME_QOSINFO_APSD:0);
					
					/* fill in a parameter set record for each AC */
					for(i = 0; i < 4; i++) 
					{
						ac = &wme->ac_params[i];
						acp = &wme_ac_params[i];

						ac->aifsn = acp->aifs;
						ac->acm = acp->admission_control_mandatory;
						ac->aci = i;
						ac->reserved = 0;
						ac->ecwmin = cw2ecw(acp->cwmin);
						ac->ecwmax = cw2ecw(acp->cwmax);
						ac->txop = htows(acp->txoplimit);
					}

					ie_len  = sizeof(struct wlan_ie_wme_param);
				}

				wme->id = WLAN_ELEMID_VENDOR_SPEC;
				wme->oui[0] = 0x00;
				wme->oui[1] = 0x50;
				wme->oui[2] = 0xf2;
				wme->type = WME_OUI_TYPE;
				wme->version = WME_VERSION;

				break;
			}
#ifdef CONFIG_WPS
			case IE_WPS_BIT:
			{
				if(!(bss->auth_capability & AUTH_CAP_WPS) || (bss->wps_ie==NULL))
					break;

				((struct wlan_ie_generic *)pos)->id = WLAN_ELEMID_VENDOR_SPEC;
				memcpy(((struct wlan_ie_generic *)pos)->data, bss->wps_ie, bss->wps_ie_len);
				ie_len = bss->wps_ie_len + 2;
				break;
			}
			case IE_WPS_BIT2:
			{
				if(!(bss->auth_capability & AUTH_CAP_WPS) || (bss->wps_ie2==NULL))
					break;

				((struct wlan_ie_generic *)pos)->id = WLAN_ELEMID_VENDOR_SPEC;
				memcpy(((struct wlan_ie_generic *)pos)->data, bss->wps_ie2, bss->wps_ie2_len);
				ie_len = bss->wps_ie2_len + 2;
				break;
			}
#endif
			case IE_EXT_CAP_BIT:
			{
				((struct wlan_ie_generic *)pos)->id = WLAN_ELEMID_EXT_CAP;
				/* 20/40 BSS coexistence management support*/
				if((my_wlan_dev->secondary_channel_def != BW40MHZ_SCN))
					((struct wlan_ie_generic *)pos)->data[0] |= 0x1;
				ie_len = 3;
				break;
			}
#ifdef CONFIG_LYNX_IBSS
			case IE_IBSS_BIT:
			{
				if(bss->role == WIF_IBSS_ROLE)
				{
					struct wlan_ie_ibss *ibss_ie = (struct wlan_ie_ibss *) pos;
					ibss_ie->id = WLAN_ELEMID_IBSS_PARMS;
					ibss_ie->atim_window = htows(bss->atim_window);
					ie_len = 4;
				}
				break;
			}
#endif
#ifdef CONFIG_WLAN_P2P_MODE
			case IE_P2P_BIT:
			{
				struct wlan_ie_p2p *p2p = (struct wlan_ie_p2p *) pos;
				u8* ptr;

				p2p->id = WLAN_ELEMID_VENDOR_SPEC;
				WRITE_BE24(p2p->oui, WFA_SPECIFIC_OUI);
				p2p->oui_type = WFA_P2P_OUI_SUBTYPE;

				ptr = (u8 *)&p2p[1];
				ie_len = p2p_put_attr(bss->p2p_ins, ptr, P2P_ATTR_CAPABILITY);
				ptr += ie_len;
				ie_len += p2p_put_attr(bss->p2p_ins, ptr, P2P_ATTR_LISTEN_CHANNEL);

				ie_len += sizeof(struct wlan_ie_p2p);
				break;
			}
#endif // CONFIG_WLAN_P2P_MODE
#endif
			default:
				break;
		}

		if(ie_len)
		{
			((struct wlan_ie_generic *)pos)->len = ie_len - 2;
			pos += ie_len;
			total_len += ie_len;
		}
		mask <<= 1;
	}

	return total_len;
}

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

void wpa_ie_change_group_cipher(u8 *start, u32 len, u32 group_cipher, u8 is_rsn)
{
	u8 *pos;
	u32 val=0;

	pos = start + 2; // skip version field
	len -= 2;

	if(is_rsn)
	{
		switch(group_cipher)
		{
			case AUTH_CAP_CIPHER_TKIP:
				val = RSN_CIPHER_SUITE_TKIP;
				break;
			case AUTH_CAP_CIPHER_CCMP:
				val = RSN_CIPHER_SUITE_CCMP;
				break;
			default:
				lynx_dbg(LYNX_DBG_WM, "%s(): wrong group_cipher(0x%x)\n", __FUNCTION__, group_cipher);
				break;
		}
	}
	else
	{
		switch(group_cipher)
		{
			case AUTH_CAP_CIPHER_TKIP:
				val = WPA_CIPHER_SUITE_TKIP;
				break;
			case AUTH_CAP_CIPHER_CCMP:
				val = WPA_CIPHER_SUITE_CCMP;
				break;
			default:
				lynx_dbg(LYNX_DBG_WM, "%s(): wrong group_cipher(0x%x)\n", __FUNCTION__, group_cipher);
				break;
		}
	}
	
	if((val) && (len >= 4))
	{
		WRITE_BE32(pos, val);
	}
}

