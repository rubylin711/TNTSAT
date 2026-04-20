/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file wm_wpa.c
*   \brief  
*   \author Montage
*/
#ifdef CONFIG_LYNX_OS_UCOS
/* ucos */
#include <mt_type.h>
#include <os/mtos_mem.h>
#include <os/mtos_misc.h>
#include <os/mtos_task.h>
#include <os/mtos_printk.h>
#include <porting.h>
#include <util/list.h>
#include <util/lib_bitops.h>
#include <stdio.h>
#include <string.h>
#include "lynx_define.h"
#endif

#include "init.h"
#include "core.h"
#include <wlan_def.h>
#include <wm_msg.h>
#include <wpa.h>
#include <wm_sta.h>
#include <wm_wpa.h>
#include <wm_mlme.h>
#include <eapol.h>
#include <wlan_ie.h>
#include <wlan_util.h>
#include <md5.h>
#include <hmac-md5.h>
#include <sha1.h>
#include <aes_wrap.h>
#include <rc4.h>
#include <sta_wpa.h>
#include "lynx_debug.h"
#include "ieee80211.h"
#include "txrx.h"
#include "wci.h"
#include "mlme_api.h"
#include "ctrl_intf.h"

#ifdef CONFIG_WPA

void ap_group_enter_wpa_state(struct wm_vif *bss, u8 new_state);

void wpa_clean(struct wpa_ctx *ctx)
{
	os_api_free(ctx);
}

void wpa_release(struct sta_context *sta)
{
	if(sta->vif->role == WIF_STA_ROLE)
		sta_enter_wpa_state(sta, AS_DISCONNECTED, NULL, 0);
	else
		ap_sta_enter_wpa_state(sta, AS_DISCONNECTED, NULL, 0);

	wpa_clean(sta->wpa_ctx);
	sta->wpa_ctx = 0;
}

struct wpa_ctx *wpa_alloc(struct wm_vif *bss)
{
	struct wpa_ctx *ctx;

	ctx = (struct wpa_ctx *)os_api_alloc(sizeof(*ctx), GFP_KERNEL);

	return ctx; 
}

int wpa_setup(struct wm_vif *bss, struct wpa_ctx *ctx, const u8 *wpa_ie, size_t wpa_ie_len,
			u8 is_rsn)
{
	struct wpa_ie_data info;
	int res;
	u8 gcipher;
	//const u8 *pmkid = NULL;
	
	res = wlan_parse_wpa_rsn_ie(wpa_ie, wpa_ie_len, &info, is_rsn);
	
	if(res < 0) 
	{
		return WLAN_STATUS_INVALID_IE;
	}

	if((bss->auth_capability & info.key_mgmt) == 0)
	{
		lynx_dbg(LYNX_DBG_WM, "No matched AKM\n");
		return WLAN_STATUS_INVALID_AKMP;
	}
	ctx->key_mgt = high_prioity_bit(AUTH_CAP_TO_MGT_TYPE(bss->auth_capability & info.key_mgmt), 16);

	if((bss->auth_capability & info.pairwise_cipher) == 0)
	{
		lynx_dbg(LYNX_DBG_WM, "No matched pairewise cipher, bss->auth_capability=0x%x, info.pairwise_cipher=0x%x\n", bss->auth_capability, info.pairwise_cipher);
		return WLAN_STATUS_INVALID_PAIRWISE_CIPHER;
	}
	ctx->pairwise = high_prioity_bit(AUTH_CAP_TO_CIPHER_TYPE(bss->auth_capability & info.pairwise_cipher), 8);
	
	/* setup group cipher */
	gcipher = low_prioity_bit(AUTH_CAP_TO_CIPHER_TYPE(bss->auth_capability & info.group_cipher), AUTH_CIPHER_TKIP);
	if(gcipher == 0xff)
	{
		lynx_dbg(LYNX_DBG_WM, "No matched group cipher\n");
		return WLAN_STATUS_INVALID_GROUP_CIPHER;
	}
	else
	{
		bss->wpa_group->cipher = gcipher;
	}

	/* TODO: clear WPA/WPA2 state if STA changes from one to another */
	if(is_rsn)
		ctx->flags |= DESC_TYPE_RSN;

	return WLAN_STATUS_SUCCESS;
}

u8 wpa_group_key_get_seqnum(struct wm_vif *bss, u8 key_idx, char *rsc)
{
	return 0;
}

u8 wpa_set_key_to_hw(struct sta_context *sta, u32 type, char *key)
{
	return 0;
}

void ap_wpa_rekey_ptk(struct sta_context *sta)
{
	sta->wpa_ctx->flags &= ~WPA_PTK_VALID;
#if 1
	/* XXX: check this entry is right? */
	ap_sta_enter_wpa_state(sta, AS_PTKSTART, NULL, 0);
#else
	ap_sta_enter_wpa_state(sta, AS_AUTHENTICATION2, NULL, 0);
#endif
}

int wpa_eapol_key_mic(const u8 *key, int ver, const u8 *buf, u32 len, u8 *mic)
{
	u8 hash[SHA1_MAC_LEN];

	switch (ver) 
	{
		case KEY_INFO_TYPE_HMAC_MD5_RC4:
			hmac_md5(buf, len, key, 16, mic);
			break;
		case KEY_INFO_TYPE_HMAC_SHA1_AES:
			hmac_sha1(buf, len, key, 16, hash);
			memcpy(mic, hash, MD5_MAC_LEN);
			break;
		default:
			return -1;
	}

	return 0;
}

void wpa_prf(const u8 *pmk, size_t pmk_len, const char *label,
		    const u8 *addr1, const u8 *addr2,
		    const u8 *nonce1, const u8 *nonce2,
		    u8 *ptk, size_t ptk_len, int use_sha256)
{
	u8 data[2 * WLAN_ADDR_LEN + 2 * WPA_NONCE_LEN];

	/* for GTK */
	if(!addr2)
	{
		memcpy(data, addr1, WLAN_ADDR_LEN);
	}
	/* for PTK */
	else if(memcmp(addr1, addr2, WLAN_ADDR_LEN) < 0) 
	{
		memcpy(data, addr1, WLAN_ADDR_LEN);
		memcpy(data + WLAN_ADDR_LEN, addr2, WLAN_ADDR_LEN);
	} 
	else 
	{
		memcpy(data, addr2, WLAN_ADDR_LEN);
		memcpy(data + WLAN_ADDR_LEN, addr1, WLAN_ADDR_LEN);
	}

	if(!nonce2)
	{
		memcpy(data + 1 * WLAN_ADDR_LEN, nonce1, WPA_NONCE_LEN);
	}
	else if(memcmp(nonce1, nonce2, WPA_NONCE_LEN) < 0) 
	{
		memcpy(data + 2 * WLAN_ADDR_LEN, nonce1, WPA_NONCE_LEN);
		memcpy(data + 2 * WLAN_ADDR_LEN + WPA_NONCE_LEN, nonce2,
			  WPA_NONCE_LEN);
	} 
	else 
	{
		memcpy(data + 2 * WLAN_ADDR_LEN, nonce2, WPA_NONCE_LEN);
		memcpy(data + 2 * WLAN_ADDR_LEN + WPA_NONCE_LEN, nonce1,
			  WPA_NONCE_LEN);
	}

	sha1_prf(pmk, pmk_len, label, data, (addr2 ? sizeof(data):(sizeof(data)/2)), ptk,
			 ptk_len);
}

static int wpa_verify_key_mic(u8 *mic_key, struct eapol_hdr *data, u32 data_len)
{
	struct eapol_key *key;
	unsigned short key_info;
	int ret = 0;
	u8 mic[16];

	key = (struct eapol_key *)data->data;
	memcpy(mic, key->key_mic, 16);
	memset(key->key_mic, 0, 16);

	key_info = READ_BE16((char *)&key->key_info);

	if((wpa_eapol_key_mic(mic_key, (key_info & KEY_INFO_TYPE_MASK), (u8 *)data, data_len, key->key_mic)) || (memcmp(mic, key->key_mic, 16) != 0))
	{
		lynx_dbg_dump(LYNX_DBG_WM, "cal-mac:", "", key->key_mic, 16);
		lynx_dbg_dump(LYNX_DBG_WM, "------------------\n", "", mic, 16);
		ret = -1;
	}
	memcpy(key->key_mic, mic, 16); /* ?? */
	return ret;
}

char wpa_replay_counter_valid(struct wpa_ctx *ctx, char *counter)
{

	return 1;
}

void ap_proc_eapol_key(struct wm_vif *bss, struct sta_context *sta, struct eapol_hdr *data, unsigned int data_len)
{
	unsigned short key_data_len, key_info;
	struct eapol_key *key;
	struct wpa_ctx *ctx = sta->wpa_ctx;

	key = (struct eapol_key *)data->data;
	if(ctx->state < AS_PTKSTART)
	{
		lynx_dbg(LYNX_DBG_WM, "Fail to process eapol-key, ctx->state=%d < AS_PTKSTART\n", ctx->state);
		return;
	}

	key_info = READ_BE16((char *)&key->key_info);
	key_data_len = READ_BE16(key->key_data_len);
	MIN_LENGTH_VERIFY(data_len, key_data_len+sizeof(*key), return);

	/* All eapol-key frame authenticator received should have MIC */ 
	if(!(key_info & KEY_INFO_MIC)) 
	{
		lynx_dbg(LYNX_DBG_WM, "Received invalid EAPOL-Key: Key MIC not set");
		return;
	}

	if((key_info & KEY_INFO_SECURE) && (ctx->flags & WPA_PTK_VALID))
	{
		if (wpa_verify_key_mic(ctx->ptk.kck, data, data_len)) 
		{
			lynx_dbg(LYNX_DBG_WM, "Received EAPOL-Key with invalid MIC");
			return;
		}
		/* cancel timer */
	}
	wm_del_timer((u32)sta, STA_WPA_TX_TIMEOUT);

	if(key_info & KEY_INFO_SMK_MESSAGE)
	{
		/* SMK handshake */
		lynx_dbg(LYNX_DBG_WM, "SMK ??\n");
		return;
	} 
	else if(key_info & KEY_INFO_REQUEST) 
	{
		if((ctx->flags & WPA_REPLAY_COUNTER) && memcmp(key->replay_counter, ctx->req_replay_counter,
			      WPA_REPLAY_COUNTER_LEN) <= 0) 
		{
			lynx_dbg(LYNX_DBG_WM, "Received EAPOL-Key request with replayed counter\n");
			return;
		}

		ctx->flags |= WPA_REPLAY_COUNTER;
		memcpy(ctx->req_replay_counter, key->replay_counter, WPA_REPLAY_COUNTER_LEN);
 
		if(key_info & KEY_INFO_ERROR) 
		{
			/* Supplicant reported a Michael MIC error */
			/*lynx_dbg(LYNX_DBG_WM, "STA report MIC error?\n");
			ctx->flags &= ~WPA_PTK_VALID;
			ap_sta_enter_wpa_state(sta, AS_PTKSTART, NULL, 0);*/
			lynx_dbg(LYNX_DBG_WM, "WBUF receive MIC ERROR Report\n");
			if(++(bss->mic_failure_count) == 2)
			{
				lynx_dbg(LYNX_DBG_WM, "TKIP COUNTERMEASURE Start, time = %d\n", os_current_time());
				bss->flag |= BSS_FLG_TKIP_COUNTERMEASURE;
				ap_release_all_sta(bss, STA_FREE_MEM);
				wpa_group_init(bss);
			}
			wm_del_timer((u32)bss, BSS_TKIP_COUNTERMEASURE);
			wm_add_timer((u32)bss, BSS_TKIP_COUNTERMEASURE, 60*WLAN_TIME_UNIT);
			return;
		}
		else if(key_info & KEY_INFO_KEY_TYPE) 
		{
			/* PTK rekey */
			lynx_dbg(LYNX_DBG_WM, "STA ask for rekey.\n");
			ctx->flags &= ~WPA_PTK_VALID;
			ap_sta_enter_wpa_state(sta, AS_PTKSTART, NULL, 0);
		}
		else
		{
			/* GTK rekey */
			lynx_dbg(LYNX_DBG_WM, "STA ask for rekey??\n");
			return;
		}

	} 
	else
	{
		if(!wpa_replay_counter_valid(ctx, key->replay_counter))
			return;

		if (!(key_info & KEY_INFO_KEY_TYPE)) 
		{
			if((ctx->state != AS_GTK_REKEYNEGOTIATING) || ((ctx->flags & WPA_PTK_VALID) == 0))
				return;

			/* receive group key handshake message 2 */
			ap_sta_enter_wpa_state(sta, AS_GTK_REKEYESTABLISHED, NULL, 0);
		} 
		else if (key_data_len == 0) 
		{
			if((ctx->state != AS_PTKINITNEGOTIATING) || ((ctx->flags & WPA_PTK_VALID) == 0))
				return;
			/* receive 4-way handshake message 4 */
			ap_sta_enter_wpa_state(sta, AS_PTKINITDONE, NULL, 0);
		} 
		else 
		{
			/* receive 4-way handshake message 2 */
			memcpy(ctx->snonce, key->key_nonce, WPA_NONCE_LEN);
			
			ap_sta_enter_wpa_state(sta, AS_PTKCALCNEGOTIATING, (unsigned char *)data, data_len);
		}
	}
}

void wpa_sta_proc_eapol_key(struct sta_context *sta, struct eapol_hdr *data, u32 data_len)
{
	u16 key_info, key_data_len;
	struct eapol_key *key;
	struct wpa_ctx *ctx = sta->wpa_ctx;

	if(!ctx)
	{
		lynx_dbg(LYNX_DBG_WM, "%s(): sta->wpa_ctx is not existing\n", __FUNCTION__);
		return;
	}

	key = (struct eapol_key *)data->data;
	key_info = READ_BE16((char *)&key->key_info);
	key_data_len = READ_BE16(key->key_data_len);
	MIN_LENGTH_VERIFY(data_len, key_data_len+sizeof(*key), return);
	
	if(key_info & KEY_INFO_SMK_MESSAGE || key_info & KEY_INFO_REQUEST)
	{
		lynx_dbg(LYNX_DBG_WM, "STA: KEY_INFO_SMK_MESSAGE || KEY_INFO_REQUEST\n");
		return;
	}
	else
	{
		if(!wpa_replay_counter_valid(ctx, key->replay_counter))
		{
			lynx_dbg(LYNX_DBG_WM, "STA: replay counter invalid\n");
			return;
		}

   		if(key_info & KEY_INFO_MIC) 
		{
			if(wpa_verify_key_mic(ctx->ptk.kck, data, data_len) != 0)
			{
				lynx_dbg(LYNX_DBG_WM, "STA mic verified fail!\n");
				return;
			}
		}
		/* Only handle FSM after station Mode start (AS_AUTGENTICATION) */	
		if((sta->wpa_ctx) && (sta->wpa_ctx->state >= AS_AUTHENTICATION))
		{
    		lynx_dbg_dump(LYNX_DBG_WLAN_TX, "rx eapol", " ", (unsigned char *)data, data_len);
			memcpy(ctx->req_replay_counter, key->replay_counter, WPA_REPLAY_COUNTER_LEN);
			sta_enter_wpa_state(sta, AS_STAKEY_START, (unsigned char *)data, data_len);
		}
	}
}

u8 cipher_key_length(u8 alg)
{
	switch (alg) 
	{
		case AUTH_CIPHER_CCMP:
			return 16;
		case AUTH_CIPHER_TKIP:
			return 32;
		case AUTH_CIPHER_WEP40:
			return 5;
		case AUTH_CIPHER_WEP104:
			return 13;
	}
	return 0;
}

void wpa_send_eapol(struct sta_context *sta, u16 key_info, const u8 *key_rsc, const u8 *nonce,
		      const u8 *kde, u32 kde_len, u32 keyidx, u8 encr)
{
	struct sk_buff *skb;
	struct eapol_hdr *ehdr;
	struct eapol_key *key;
	u32 len;
	int alg;
	int key_data_len, pad_len = 0;
	u8 *buf, *pos;
	int version;
	struct wpa_ctx *ctx = sta->wpa_ctx;

	len = sizeof(struct eapol_hdr) + sizeof(struct eapol_key);
	ctx = sta->wpa_ctx;
	if(ctx->pairwise == AUTH_CIPHER_CCMP)
		version = KEY_INFO_TYPE_HMAC_SHA1_AES;
	else
		version = KEY_INFO_TYPE_HMAC_MD5_RC4;

	key_data_len = kde_len;

	if ((version == KEY_INFO_TYPE_HMAC_SHA1_AES ||
	     version == KEY_INFO_TYPE_AES_128_CMAC) && encr) 
	{
		pad_len = key_data_len % 8;
		if (pad_len)
			pad_len = 8 - pad_len;
		key_data_len += pad_len + 8;
	}

	len += key_data_len;

	skb = alloc_data_frame(sta->vif, ETHERTYPE_EAPOL, sta->addr, len, (u8 **)(int)&ehdr);
	if(skb == 0)
		return;

	ehdr->ver = EAPOL_VERSION;
	ehdr->type = EAPOL_KEY;
	ehdr->len = len  - sizeof(*ehdr);

	key = (struct eapol_key *)(ehdr->data);

	key->type = (ctx->flags & DESC_TYPE_RSN) ? 
					EAPOL_KEY_DESC_TYPE_RSN : EAPOL_KEY_DESC_TYPE_WPA;
	key_info |= version;
	if(encr && (ctx->flags & DESC_TYPE_RSN))
		key_info |= KEY_INFO_ENCR_KEY_DATA;
	if((ctx->flags & DESC_TYPE_RSN) == 0)
		key_info |= keyidx << KEY_INFO_KEY_INDEX_SHIFT;

	WRITE_BE16((char *)&key->key_info, key_info);
	
	alg = (key_info & KEY_INFO_KEY_TYPE) ? ctx->pairwise : sta->vif->wpa_group->cipher;
	WRITE_BE16(key->key_len, cipher_key_length(alg));

	if(key_info & KEY_INFO_SMK_MESSAGE)
	{
		WRITE_BE16(key->key_len, 0);
	}

	inc_byte_array(ctx->key_replay_counter, WPA_REPLAY_COUNTER_LEN);
	memcpy(key->replay_counter, ctx->key_replay_counter, WPA_REPLAY_COUNTER_LEN);

	if(nonce)
		memcpy(key->key_nonce, nonce, WPA_NONCE_LEN);

	if(key_rsc)
		memcpy(key->key_rsc, key_rsc, WPA_KEY_RSC_LEN);

	if(kde && !encr) 
	{
		memcpy(key + 1, kde, kde_len);
		WRITE_BE16(key->key_data_len, kde_len);
	} 
	else if (encr && kde) 
	{
		buf = os_api_alloc(key_data_len, GFP_KERNEL);
		if(buf == NULL) 
			goto fail;

		pos = buf;
		memcpy(pos, kde, kde_len);
		pos += kde_len;

		if (pad_len)
			*pos++ = 0xdd;
		if (version == KEY_INFO_TYPE_HMAC_SHA1_AES ||
		    version == KEY_INFO_TYPE_AES_128_CMAC) 
		{
			if (aes_wrap(ctx->ptk.kek, (key_data_len - 8) / 8, buf,
				     (u8 *) (key->key_data))) 
			{
				os_api_free(buf);
				goto fail;
			}
			
			WRITE_BE16(key->key_data_len, key_data_len);
		} 
		else 
		{
			u8 ek[32];
			memcpy(key->key_iv,
				  sta->vif->wpa_counter + WPA_NONCE_LEN - 16, 16);
			inc_byte_array(sta->vif->wpa_counter, WPA_NONCE_LEN);
			memcpy(ek, key->key_iv, 16);
			memcpy(ek + 16, ctx->ptk.kek, 16);
			memcpy(key->key_data, buf, key_data_len);
			rc4_skip(ek, 32, 256, key->key_data, key_data_len, key->key_data);
			WRITE_BE16(key->key_data_len, key_data_len);
		}
		os_api_free(buf);
	}

	if(key_info & KEY_INFO_MIC) 
	{
		if ((ctx->flags & WPA_PTK_VALID) == 0)
		{
			lynx_dbg(LYNX_DBG_WM, "PTK not valid when sending EAPOL-Key frame");
			goto fail;
		}
		wpa_eapol_key_mic(ctx->ptk.kck, version, (u8 *) ehdr, len,
				  key->key_mic);
	}

	lynx_dbg(LYNX_DBG_WM, ">>>TX eapol\n");
	//lynx_dbg_dump(LYNX_DBG_WCI, "eapol : ", "", (u8 *)hdr, (u8 *)ehdr + len - (u8 *)hdr);

	wm_add_timer((u32)sta, STA_WPA_TX_TIMEOUT, WPA_STA_TX_RETRY_TIME);

	/* FIXME: may use the lynx_data_tx */
	if(lynx_tx_send(my_wlan_dev->driver_priv, skb) == 0)
		return;

fail:
	kfree_skb(skb);
}

void sta_wpa_send_eapol(struct wm_vif *bss, u16 key_info, const u8 *key_rsc, const u8 *nonce,
		      const u8 *kde, u32 kde_len, u32 keyidx, u8 encr, u8 *rpl_cnter)
{
	struct sk_buff *skb;
	struct eapol_hdr *ehdr;
	struct eapol_key *key;
	u32 len;
	//int alg;
	int key_data_len = 0, pad_len = 0;
	//u8 *buf, *pos;
	int version;
	struct sta_context *sta = bss->sta_list;
	struct wpa_ctx *ctx = sta->wpa_ctx;
	unsigned int tmp;

	len = sizeof(struct eapol_hdr) + sizeof(struct eapol_key);

	if(ctx->pairwise == AUTH_CIPHER_CCMP)
		version = KEY_INFO_TYPE_HMAC_SHA1_AES;
	else
		version = KEY_INFO_TYPE_HMAC_MD5_RC4;

	key_data_len = kde_len;
	
	if ((version == KEY_INFO_TYPE_HMAC_SHA1_AES ||
	     version == KEY_INFO_TYPE_AES_128_CMAC) && encr) 
	{
		pad_len = key_data_len % 8;
		if (pad_len)
			pad_len = 8 - pad_len;
		key_data_len += pad_len + 8;
	}
	
	len += key_data_len;

	tmp = len + 14;	// 14 = ethernet header length
	if((skb = alloc_data_frame(bss, ETHERTYPE_EAPOL, bss->bssid, tmp, (u8 **)(int)&ehdr)) == NULL)
	{
		lynx_dbg(LYNX_DBG_WM, "%s(%d): no resource \n", __FUNCTION__, __LINE__);
		return;
	}

	skb->len = tmp;

	ehdr->ver = EAPOL_VERSION;
	ehdr->type = EAPOL_KEY;
	tmp = len  - sizeof(*ehdr);
	WRITE_BE16((char *)&ehdr->len, tmp);

	key = (struct eapol_key *)(ehdr->data);

	key->type = (ctx->flags & DESC_TYPE_RSN) ? 
					EAPOL_KEY_DESC_TYPE_RSN : EAPOL_KEY_DESC_TYPE_WPA;

	key_info |= version;
#if 0
	if(encr && (ctx->flags & DESC_TYPE_RSN))
		key_info |= KEY_INFO_ENCR_KEY_DATA;
	if((ctx->flags & DESC_TYPE_RSN) == 0)
		key_info |= keyidx << KEY_INFO_KEY_INDEX_SHIFT;
#endif
	WRITE_BE16((char *)&key->key_info, key_info);

#if 0
	alg = ctx->pairwise;
	WRITE_BE16(key->key_len, cipher_key_length(alg));
#else
	WRITE_BE16(key->key_len, 0);
#endif

#if 0
	if(key_info & KEY_INFO_SMK_MESSAGE)
		WRITE_BE16(key->key_len, 0);
#endif

	memcpy(key->replay_counter, rpl_cnter, WPA_REPLAY_COUNTER_LEN);
	if(nonce)
		memcpy(key->key_nonce, nonce, WPA_NONCE_LEN);

#if 0
	if(key_rsc)
		memcpy(key->key_rsc, key_rsc, WPA_KEY_RSC_LEN);
#endif

	//if(kde && !encr) 
	if(kde)
	{
		memcpy(key + 1, kde, kde_len);
		WRITE_BE16(key->key_data_len, kde_len);
	}
#if 0
	else if (encr && kde) 
	{
		buf = os_api_alloc(key_data_len, GFP_KERNEL);
		if(buf == NULL) 
			goto fail;

		pos = buf;
		memcpy(pos, kde, kde_len);
		pos += kde_len;

		if (pad_len)
			*pos++ = 0xdd;
		if (version == KEY_INFO_TYPE_HMAC_SHA1_AES ||
		    version == KEY_INFO_TYPE_AES_128_CMAC) 
		{
			if (aes_wrap(ctx->ptk.kek, (key_data_len - 8) / 8, buf,
				     (u8 *) (key->key_data))) 
			{
				os_api_free(buf);
				goto fail;
			}
			
			WRITE_BE16(key->key_data_len, key_data_len);
		} 
		else 
		{
			u8 ek[32];
			memcpy(key->key_iv,
				  sta->vif->wpa_counter + WPA_NONCE_LEN - 16, 16);
			inc_byte_array(sta->vif->wpa_counter, WPA_NONCE_LEN);
			memcpy(ek, key->key_iv, 16);
			memcpy(ek + 16, ctx->ptk.kek, 16);
			memcpy(key->key_data, buf, key_data_len);
			rc4_skip(ek, 32, 256, key->key_data, key_data_len, key->key_data);
			WRITE_BE16(key->key_data_len, key_data_len);
		}
		os_api_free(buf);
	}
#endif

	if(key_info & KEY_INFO_MIC) 
	{
		if ((ctx->flags & WPA_PTK_VALID) == 0)
		{
			lynx_dbg(LYNX_DBG_WM, "PTK not valid when sending EAPOL-Key frame");
			goto fail;
		}
#if 1
		lynx_dbg(LYNX_DBG_WM, "STA: gen MIC:\n");
		lynx_dbg_dump(LYNX_DBG_WCI, "STA: ptk.kck", "", ctx->ptk.kck, 16);
		lynx_dbg(LYNX_DBG_WM, "STA: version %x\n", version);
		lynx_dbg_dump(LYNX_DBG_WCI, "STA: ehdr len", "", (u8 *)ehdr, len);
#endif
		wpa_eapol_key_mic(ctx->ptk.kck, version, (u8 *) ehdr, len,
				  key->key_mic);
#if 1
		lynx_dbg_dump(LYNX_DBG_WCI, "STA: key_mic", "", key->key_mic, 16);
#endif
	}
	

#if defined(CONFIG_LYNX_OS_LINUX)
	lynx_data_tx(skb, ((struct lynx_vif *)bss->drv_vif)->ndev);
#else
	lynx_data_tx(skb, bss->drv_vif);
#endif
	
	/* FIXME: if failed at lynx_data_tx, lynx_data_tx will free the skb. */
	return;

fail:
	lynx_dbg(LYNX_DBG_WM, "%s(%d): tx frame fail \n", __FUNCTION__, __LINE__);
	kfree_skb(skb);
}

void pmk_to_pmkid(const u8 *pmk, size_t pmk_len, const u8 *aa,
		      const u8 *spa, u8 *pmkid, int use_sha256)
{
	char *title = "PMK Name";
	const u8 *addr[3];
	const u32 len[3] = { 8, WLAN_ADDR_LEN, WLAN_ADDR_LEN };
	unsigned char hash[32];

	addr[0] = (u8 *) title;
	addr[1] = aa;
	addr[2] = spa;

	hmac_sha1_vector(pmk, pmk_len, 3, addr, len, hash);
	memcpy(pmkid, hash, WLAN_PMKID_LEN);
}

u8 * wpa_add_kde(u8 *pos, u32 kde, const u8 *data, size_t data_len,
		 const u8 *data2, size_t data2_len)
{
	*pos++ = WLAN_ELEMID_VENDOR_SPEC;
	*pos++ = SELECTOR_LEN + data_len + data2_len;
	WRITE_BE32(pos, kde);
	pos += SELECTOR_LEN;
	if(data) 
	{
		memcpy(pos, data, data_len);
		pos += data_len;
	}
	if(data2) 
	{
		memcpy(pos, data2, data2_len);
		pos += data2_len;
	}
	return pos;
}

char key_mgt_type(u8 val)
{
	switch(val)
	{
		case AUTH_KEY_MGT_1X:
		case AUTH_KEY_MGT_FT_1X:
		case AUTH_KEY_MGT_1X_SHA256:
			return AUTH_CAP_KEY_MGT_ALL_1X; 
		case AUTH_KEY_MGT_PSK:
		case AUTH_KEY_MGT_PSK_SHA256:
		case AUTH_KEY_MGT_FT_PSK:
			return AUTH_CAP_KEY_MGT_ALL_PSK;
		default:
			return 0; 
	}
}

char key_mgt_is_psk(u8 val)
{
	if((val == AUTH_KEY_MGT_1X) || (val == AUTH_KEY_MGT_1X_SHA256) ||
		(val == AUTH_KEY_MGT_FT_1X))
		return 1;
	return 0;
}

void gtk_rekey_sta_num_change(struct sta_context *sta)
{
	struct wpa_ctx *ctx = sta->wpa_ctx;
	struct wpa_group_key *gctx = sta->vif->wpa_group;

	if(ctx->flags & WPA_GTK_UPDATING)
	{
		gctx->key_done_sta--;
		ctx->flags &= ~WPA_GTK_UPDATING;

		if(gctx->key_done_sta == 0)
		{
			ap_group_enter_wpa_state(sta->vif, AS_GTK_SETKEYSDONE);
		} 
	}
}

void ap_sta_enter_wpa_state(struct sta_context *sta, int new_state, unsigned char *data, int data_len)
{
	struct wpa_ctx *ctx = sta->wpa_ctx;
	if(ctx == 0)
		return;
	lynx_dbg(LYNX_DBG_WM, "WPA old state=%d, new state=%d\n", ctx->state, new_state);

	/* change to new state */
	ctx->state = new_state; 

	switch(new_state)
	{
		case AS_INITIALIZE:
			/* delete key */
			sta_set_key(sta, CIPHER_TYPE_NONE, KEY_TYPE_PAIRWISE_KEY, NULL, 0, 0);
			break;
		case AS_DISCONNECT:
			lynx_dbg(LYNX_DBG_WM, "AS_DISCONNECT: time %d\n", os_current_time()/100);
			ap_sta_deauth(sta, WLAN_REASON_PREV_AUTH_NOT_VALID, STA_FREE_MEM);
			break;
		case AS_DISCONNECTED:
			gtk_rekey_sta_num_change(sta);
			break;
		case AS_AUTHENTICATION:
		{
			/* remove old ptk */
			memset(&ctx->ptk, 0, sizeof(ctx->ptk));
			ctx->flags &= ~WPA_PTK_VALID;

			/* FIXME: Disable port authorized state */

			ap_sta_enter_wpa_state(sta, AS_AUTHENTICATION2, NULL, 0);	

			wm_add_timer((u32)sta, BSS_4WAY_HANDSHAKE_TIMEOUT, WPA_4WAY_HANDSHAKE_TIMEOUT);
			break;
		}
		case AS_AUTHENTICATION2:
		{
			memcpy(ctx->anonce, sta->vif->wpa_counter, WPA_NONCE_LEN);
			inc_byte_array(sta->vif->wpa_counter, WPA_NONCE_LEN);
			ctx->timeout_counter = 0;

			/* XXX: We should also check 802.1X::keyRun */
			{
				if (key_mgt_type(ctx->key_mgt) == AUTH_CAP_KEY_MGT_ALL_PSK)
					ap_sta_enter_wpa_state(sta, AS_INITPSK, NULL, 0);
			}
			break;
		}
		case AS_INITPMK:
		{
			break;
		}
		case AS_INITPSK:
		{
			ctx->pmk = sta->vif->psk;
			ctx->flags &= ~WPA_REPLAY_COUNTER;
			ap_sta_enter_wpa_state(sta, AS_PTKSTART, NULL, 0);
			break;
		}
		case AS_PTKSTART:
		{
			u8 kde[2 + SELECTOR_LEN + PMKID_LEN], *pmkid = NULL;
			u32 pmkid_len = 0;
	
			/* invalid GTK */	
			ctx->flags &= ~WPA_GTK_VALID;
			/* don't allow GTK rekey during the ptk rekey/exchange */
			ctx->flags |= WPA_PTK_NEGOTIATING;
	
			/* send 4-way handshake message 1 */
			if(++ctx->timeout_counter > EAPOL_KEY_MAX_REPLAY)
			{
				ap_sta_enter_wpa_state(sta, AS_DISCONNECT, NULL, 0);
				return;
			}

			if((ctx->flags & DESC_TYPE_RSN) && (key_mgt_type(ctx->key_mgt) == AUTH_CAP_KEY_MGT_ALL_1X)) 
			{
				wpa_add_kde(kde, RSN_KDE_PMDID, 0, 0, 0, 0);

				/*
				* Calculate PMKID since no PMKSA cache entry was
				* available with pre-calculated PMKID.
				*/
				pmk_to_pmkid(ctx->pmk, PMK_LEN, sta->vif->bssid,
					sta->addr, &kde[2 + SELECTOR_LEN], 0);

				pmkid = kde;
				pmkid_len = 2 + SELECTOR_LEN + PMKID_LEN; 
			}

			wpa_send_eapol(sta, KEY_INFO_ACK | KEY_INFO_KEY_TYPE, NULL,
		       ctx->anonce, pmkid, pmkid_len, 0, 0);

			break;
		}
		case AS_PTKCALCNEGOTIATING:
		{
			if(data == NULL)
				break;

			/* derive PTK */
			wpa_prf(ctx->pmk, PMK_LEN, "Pairwise key expansion", 
				sta->vif->bssid, sta->addr, ctx->anonce, ctx->snonce, 
				(u8 *)&ctx->ptk, sizeof(ctx->ptk), 0);

			if(wpa_verify_key_mic(ctx->ptk.kck, (struct eapol_hdr *) data, data_len) != 0)
			{
				lynx_dbg(LYNX_DBG_WM, "mic verified fail!\n");
				break;
			}
	
			/* TODO: cancel timer of waiting eapol-key */

			ctx->flags |= WPA_PTK_VALID;

			ap_sta_enter_wpa_state(sta, AS_PTKCALCNEGOTIATING2, NULL, 0);
			break;
		}
		case AS_PTKCALCNEGOTIATING2:
		{
			ctx->timeout_counter = 0;
			ap_sta_enter_wpa_state(sta, AS_PTKINITNEGOTIATING, NULL, 0);
			break;
		}
		case AS_PTKINITNEGOTIATING:
		{
			u8 rsc[WPA_KEY_RSC_LEN], kde[MAX_KDE_LEN], *_rsc, *pos;
			struct wpa_group_key *gctx = sta->vif->wpa_group;
			u8 secure, keyidx, encr;

			if(++ctx->timeout_counter > EAPOL_KEY_MAX_REPLAY)
			{
				ap_sta_enter_wpa_state(sta, AS_DISCONNECT, NULL, 0);
				return;
			}

			/* send 4-way handshake message 3 */
			/* Send EAPOL(1, 1, 1, Pair, P, RSC, ANonce, MIC(PTK), RSNIE, GTK[GN]) */
			_rsc = 0;
			secure = 0;
			keyidx = 0;
			encr = 0;
			pos = kde;

			if(!(ctx->flags & DESC_TYPE_RSN))
			{
				if(sta->vif->wpa_ie_len)
				{
					pos += wlan_ap_encode_ie(sta->vif, pos, IE_WPA_BIT);
				}
			}
			else
			{
				u8 hdr[2];

				memset(rsc, 0, WPA_KEY_RSC_LEN);
				wpa_group_key_get_seqnum(sta->vif, gctx->key_idx, rsc);
	
				/* RSN IE */
				if(sta->vif->rsn_ie_len)
				{
					pos += wlan_ap_encode_ie(sta->vif, pos, IE_RSN_BIT);
				}

				if(sta->vif->wpa_ie_len)
				{
					pos += wlan_ap_encode_ie(sta->vif, pos, IE_WPA_BIT);
				}

				/* WPA2 send GTK in the 4-way handshake */
				secure = 1;
				_rsc = rsc;
				encr = 1;
				keyidx = gctx->key_idx+1;

				hdr[0] = keyidx;
				hdr[1] = 0;
				pos = wpa_add_kde(pos, RSN_KDE_GTK, hdr, 2, 
					gctx->gtk[gctx->key_idx], cipher_key_length(gctx->cipher));

			}

			wpa_send_eapol(sta, (secure ? KEY_INFO_SECURE : 0) | KEY_INFO_MIC |
				KEY_INFO_ACK | KEY_INFO_INSTALL | KEY_INFO_KEY_TYPE,
		       _rsc, ctx->anonce, kde, pos - kde, keyidx, encr);

			break;
		}
		case AS_PTKINITDONE:
		{
			/* allow GTK rekey now */
			ctx->flags &= ~WPA_PTK_NEGOTIATING;

			sta_set_key(sta, ctx->pairwise, KEY_TYPE_PAIRWISE_KEY, ctx->ptk.tk1, 0, 0);

			if(ctx->flags & DESC_TYPE_RSN)
				ctx->flags |= WPA_GTK_VALID;
			else
				ap_sta_enter_wpa_state(sta, AS_GTK_IDLE, NULL, 0);

			/* hook next PTK rekey time */
			if(my_wlan_dev->ptk_rekey_time)
				wm_add_timer((u32)sta, STA_WPA_PTK_REKEY_TIMEOUT, my_wlan_dev->ptk_rekey_time*WLAN_TIME_UNIT);

			/* FIXME: Enable port authorized state */
			//wla_forward(DATA_CAN_FORWARD, sta->wcb);
			sta->flags |= WLAN_STA_AUTHORIZED;
			mlme_data_path_ctrl(sta->vif->drv_vif, DATAPATH_CARRIER_ON);
			wm_add_timer((unsigned int)sta->vif, SYSTEM_MAINTAIN_DHCP, 5 * WLAN_TIME_UNIT);
			/* FIXME: how to setup the device status with the AP ROLE? */
			//set_device_status(WIFI_CONNECTED);
			
			break;
		}
		case AS_GTK_IDLE:
		{
			ctx->timeout_counter = 0;
			ap_sta_enter_wpa_state(sta, AS_GTK_REKEYNEGOTIATING, NULL, 0);
			break;
		}
		case AS_GTK_REKEYNEGOTIATING:
		{
			u8 rsc[WPA_KEY_RSC_LEN], kde[MAX_KDE_LEN];
			struct wpa_group_key *gctx = sta->vif->wpa_group;
			u8 *_kde, *pos, hdr[2];

			if(++ctx->timeout_counter > EAPOL_KEY_MAX_REPLAY)
			{
				ap_sta_enter_wpa_state(sta, AS_GTK_ERROR, NULL, 0);
				return;
			}

			/* Send EAPOL(1, 1, 1, !Pair, G, RSC, GNonce, MIC(PTK), GTK[GN]) */
			memset(rsc, 0, WPA_KEY_RSC_LEN);
			if(gctx->state == AS_GTK_SETKEYSDONE)
				wpa_group_key_get_seqnum(sta->vif, gctx->key_idx, rsc);

			if(ctx->flags & DESC_TYPE_RSN)
			{
				pos = _kde = kde;
				hdr[0] = gctx->key_idx+1;
				hdr[1] = 0;
				pos = wpa_add_kde(pos, RSN_KDE_GTK, hdr, 2,
				  gctx->gtk[gctx->key_idx], cipher_key_length(gctx->cipher));
			} 
			else 
			{
				_kde = (u8 *) &(gctx->gtk[gctx->key_idx]);
				pos = _kde + cipher_key_length(gctx->cipher);
			}

			wpa_send_eapol(sta, KEY_INFO_SECURE | KEY_INFO_MIC | KEY_INFO_ACK,
		       rsc, gctx->gnonce, _kde, pos - _kde, gctx->key_idx+1, 1);

			break;
		}
		case AS_GTK_REKEYESTABLISHED:
		{
			ctx->timeout_counter = 0;
			ctx->flags |= WPA_GTK_VALID;

			gtk_rekey_sta_num_change(sta);
		
			break;
		}
		case AS_GTK_ERROR:
		{
			gtk_rekey_sta_num_change(sta);
			ap_sta_enter_wpa_state(sta, AS_DISCONNECT, NULL, 0);

			break;
		}
	}

}

void ap_group_enter_wpa_state(struct wm_vif *bss, u8 new_state)
{
	struct wpa_group_key *gctx = bss->wpa_group;

	/* change to new state */
	gctx->state = new_state; 

	switch(new_state)
	{
		case AS_GTK_SETKEYS:
		{
			struct wpa_ctx *ctx;
			struct sta_context *sta;

			/* swap key index */
			gctx->key_idx = 1 - gctx->key_idx;
			memcpy(gctx->gnonce, bss->wpa_counter, WPA_NONCE_LEN);
			inc_byte_array(bss->wpa_counter, WPA_NONCE_LEN);
			/* generate new GTK */
			wpa_prf(gctx->gmk, GMK_LEN, "Group key expansion", 
				bss->bssid, 0, gctx->gnonce, 0, (u8 *) &(gctx->gtk[gctx->key_idx]),
				WPA_GTK_MAX_LEN, 0);

			for(sta = bss->sta_list; sta; sta = sta->next)
			{
				ctx = sta->wpa_ctx;
				if(!ctx || (ctx->state < AS_PTKINITDONE))
					continue;
				
				/* avoid GTK rekey handshake during PTK rekey handshake */
				if(ctx->flags & WPA_PTK_NEGOTIATING)
					continue;

				ctx->flags &= ~WPA_GTK_VALID;
				gctx->key_done_sta++; 

				ctx->flags |= WPA_GTK_UPDATING;
				ap_sta_enter_wpa_state(sta, AS_GTK_IDLE, NULL, 0);
			}
			/* waiting for all stas rekey done */
			if(!gctx->key_done_sta)
				ap_group_enter_wpa_state(bss, AS_GTK_SETKEYSDONE);
	
			break;
		}
		case AS_GTK_SETKEYSDONE:
		{
			/* We assume all stas should finish rekey */ 
			gctx->key_done_sta = 0;
			lynx_dbg(LYNX_DBG_WM, "before gkey, bss->bss_desc = %d\n", bss->bss_desc);
			sta_set_key((struct sta_context *)bss, gctx->cipher, KEY_TYPE_GLOBAL_KEY, 
					(char *) &(gctx->gtk[gctx->key_idx]), gctx->key_idx+1, gctx->key_idx+1);
			break;
		}
	}
}

/* register GTK rekey */
void wpa_group_rekey(struct wm_vif *bss)
{
	ap_group_enter_wpa_state(bss, AS_GTK_SETKEYS);

	/* register BSS's GTK rekey timer */
	if(bss->gtk_rekey_time)
		wm_add_timer((u32)bss, BSS_WPA_GROUP_REKEY_TIMEOUT, (bss->gtk_rekey_time * WLAN_TIME_UNIT));
}

char wpa_group_init(struct wm_vif *bss)
{
	struct wpa_group_key *gctx;
	u8 buf[WLAN_ADDR_LEN + 10];
	u8 rkey[32];
	u32 i;
	int current_time;

	if(bss->wpa_group)
		os_api_free(bss->wpa_group);

	if((gctx = (struct wpa_group_key *)os_api_alloc(sizeof(struct wpa_group_key), GFP_KERNEL)) == 0)
		return 0;

	current_time = os_current_time();
	bss->wpa_group = gctx;
	/* Counter = PRF-256(Random number, "Init Counter",
	 *                   Local MAC Address || Time)
	 */
	memcpy(buf, bss->bssid, WLAN_ADDR_LEN);
	sprintf(buf+WLAN_ADDR_LEN, "%08d", current_time);

	for(i=0; i<8; i++)
	{
		*(int *)&rkey[i*4] = os_api_rand(current_time);
		*(int *)&gctx->gmk[i*4] = os_api_rand(current_time + 555);
	}

	sha1_prf(rkey, WPA_NONCE_LEN, "Init Counter", buf, sizeof(buf), bss->wpa_counter, WPA_NONCE_LEN);

	gctx->cipher = low_prioity_bit(AUTH_CAP_TO_CIPHER_TYPE(bss->auth_capability), AUTH_CIPHER_TKIP);
	gctx->key_idx = 1;

	wpa_group_rekey(bss);

	return 1;
}

void wpa_timeout(struct sta_context *sta)
{
	if(sta->wpa_ctx)
	{
		if((sta->wpa_ctx->state == AS_PTKSTART) ||
			(sta->wpa_ctx->state == AS_PTKINITNEGOTIATING) || 
			(sta->wpa_ctx->state == AS_GTK_REKEYNEGOTIATING))
		{
			ap_sta_enter_wpa_state(sta, sta->wpa_ctx->state, NULL, 0);
		}
		else
		{
			lynx_dbg(LYNX_DBG_WM, "????%s(), wrong state:%d\n", __func__, sta->wpa_ctx->state); 
		}
	}
}

#define SWAP_MIC_KEY(tx_mic, rx_mic)		\
	do{										\
		u8 tmp[8];							\
		memcpy(tmp, tx_mic, 8);				\
		memcpy(tx_mic, rx_mic, 8);			\
		memcpy(rx_mic, tmp, 8);				\
	} while(0)
inline void decode_key_data(u8 *kek, u8 cipher, u8 *key_iv, u8 *key_data, u32 key_len, u8 *dest)
{
	if(cipher == AUTH_CIPHER_TKIP)
	{
		u8 ek[32];
		
		memcpy(ek, key_iv, 16);
		memcpy(ek + 16, kek, 16);
		rc4_skip(ek, 32, 256, key_data, key_len, dest);
	}
	else if(cipher == AUTH_CIPHER_CCMP)
	{
		if(aes_unwrap(kek, ((key_len - 8) / 8), key_data, dest))
		{
			lynx_dbg(LYNX_DBG_WM, "CCMP GTK unwrap fail!!\n");
		}
	}
}

inline int rsn_get_gtk(u8 cipher, u8 *kde, u16 kde_len, u8 **gtk)
{
	u8 gtk_len;
	u8 *tmp;
	int i;
	int idx=-1;

	tmp = kde;

	for(i=0; i<kde_len; i++)
	{
		if((tmp[0] == 0xDD) && ((tmp[2] << 24 | tmp[3] << 16 | tmp[4] << 8 | tmp[5]) == RSN_KDE_GTK))
		{	
			/* only handle gtk kde */
			gtk_len = tmp[1] - 6;
			*gtk = &tmp[8];

			if(gtk_len == cipher_key_length(cipher)) // avoid wrong key len
				idx = tmp[6] & 0x3;
			break;
		}
		else
		{
			tmp = tmp + 2 + tmp[1]; // tmp[1] : length field , 2 : tmp[0] & tmp[1]
			i += (tmp[1] + 2);
		}
	}
	
	return idx;
}

#if !defined(CONFIG_LYNX_OS_LINUX)
int arc4random(void)
{
    unsigned int res;
    static unsigned long seed = 0xDEADB00B;

	res = os_current_time();
    seed = ((seed & 0x007F00FF) << 7) ^
        ((seed & 0x0F80FF00) >> 8) ^ // be sure to stir those low bits
        (res << 13) ^ (res >> 9);    // using the clock too!
    return (int)seed;
}

void get_random_bytes(void *buf, size_t len)
{
    unsigned long ranbuf, *lp;
    lp = (unsigned long *)buf;
    while (len > 0) {
        ranbuf = arc4random();
        *lp++ = ranbuf;
        len -= sizeof(ranbuf);
    }
}
#endif

void sta_enter_wpa_state(struct sta_context *sta, unsigned char new_state, unsigned char *data, int data_len)
{
	struct wm_vif *bss = sta->vif;
	struct wpa_ctx *ctx = sta->wpa_ctx;
	
	lynx_dbg(LYNX_DBG_WM, "STA Mode WPA old state=%d, new state=%d\n", ctx->state, new_state);
	ctx->state = new_state; 

	switch(new_state)
	{
		case AS_INITIALIZE:
			/* delete key */
			sta_set_key(sta, AUTH_CIPHER_NONE, KEY_TYPE_STA_PAIRWISE_KEY, NULL, 0, 0);
			break;
		case AS_DISCONNECTED:
			sta_enter_wpa_state(sta, AS_INITIALIZE, NULL, 0);
			break;
		case AS_AUTHENTICATION:
		{
			memset(&ctx->ptk, 0, sizeof(ctx->ptk));
			ctx->flags &= ~WPA_PTK_VALID;

			/* FIXME: disable port */
			//wla_forward(DATA_NO_FORWARD, sta->wcb);

			ctx->pmk = bss->psk;
			lynx_dbg_dump(LYNX_DBG_WCI, "AS_AUTHENTICATION: pmk", "", ctx->pmk, PMK_LEN);

			wm_add_timer((u32)sta, BSS_4WAY_HANDSHAKE_TIMEOUT, WPA_4WAY_HANDSHAKE_TIMEOUT);
			break;
		}
		case AS_STAKEY_START:
		{
			//u8 cipher;
			u8 *gtk;
			u8 *pos;
			u8 kde[MAX_KDE_LEN];
   			u16 key_info;
			u16 key_len;
			u16 info;
			u32 wpa_bit=0;
			char idx;
			struct eapol_key *key;
   
			if(data == NULL)
			{
				lynx_dbg(LYNX_DBG_WM, "%s(%d): no eapol info\n", __FUNCTION__, __LINE__);
				break;
			}

			key = (struct eapol_key *)((struct eapol_hdr *)data)->data;
			key_info = READ_BE16((char *)&key->key_info);
			key_len = (key->key_data_len[0] << 8) | key->key_data_len[1];
			//cipher = high_prioity_bit(AUTH_CAP_TO_CIPHER_TYPE(bss->auth_capability), 8);

   			if(key_info & KEY_INFO_KEY_TYPE) // pairwise key
			{
				if(key_info & KEY_INFO_MIC) 
				{
					lynx_dbg(LYNX_DBG_WM, "%s(%d): rx M3\n", __FUNCTION__, __LINE__);
					/* receive 4-way handshake message 3 */
					lynx_dbg(LYNX_DBG_WM, "=== Handle 4-way handshake M3 ===\n");

					/* send 4-way handshake message 4 */
					if(ctx->flags & DESC_TYPE_RSN)
						info = KEY_INFO_MIC | KEY_INFO_KEY_TYPE | KEY_INFO_SECURE;
					else
						info = KEY_INFO_MIC | KEY_INFO_KEY_TYPE;
					sta_wpa_send_eapol(bss, info, NULL, NULL, 
									   NULL, 0, 0, 0, key->replay_counter);
					
					/* set key into HW */
					sta_set_key(sta, ctx->pairwise, KEY_TYPE_STA_PAIRWISE_KEY, ctx->ptk.tk1, 0, 0);
#if 0
					/* FIXME: Enable port authorized state */
					//wla_forward(DATA_CAN_FORWARD, sta->wcb);
					sta->flags |= WLAN_STA_AUTHORIZED;
					mlme_data_path_ctrl(sta->vif->drv_vif, DATAPATH_CARRIER_ON);
#endif
					
					/* WPA2's M3 contains the GTK */
					if(ctx->flags & DESC_TYPE_RSN)
					{
						decode_key_data(ctx->ptk.kek, ctx->pairwise, key->key_iv, key->key_data, key_len, kde);
						lynx_dbg_dump(LYNX_DBG_WCI, "decoded kde :", "", kde, key_len);
						if((idx = rsn_get_gtk(bss->wpa_group->cipher, kde, key_len, &gtk)) > 0)
						{
							/* swap gtk tx & rx mic key */
							if(bss->wpa_group->cipher == AUTH_CIPHER_TKIP)
								SWAP_MIC_KEY(gtk+16, gtk+24);

							sta_set_key((struct sta_context *)bss, bss->wpa_group->cipher, 
										KEY_TYPE_GLOBAL_KEY, gtk, idx, idx);

							ctx->flags |= WPA_GTK_VALID;

							if(!(sta->flags & WLAN_STA_AUTHORIZED))
							{
								/* open datapath here to avoid the broadcast frame rx 
									before gkey is ok 									*/
								sta->flags |= WLAN_STA_AUTHORIZED;
								mlme_data_path_ctrl(sta->vif->drv_vif, DATAPATH_CARRIER_ON);
								wm_add_timer((unsigned int)sta->vif, SYSTEM_MAINTAIN_DHCP, 5 * WLAN_TIME_UNIT);

#if defined(CONFIG_LYNX_OS_UCOS)
								/* FIXME: for montage ucos */
								/* FIXME: how to setup the device status with the AP ROLE? */
								set_device_status(WIFI_CONNECTED);
#endif
							}

						}
					}
				}
				else
				{
					lynx_dbg(LYNX_DBG_WM, "%s(%d): rx M1\n", __FUNCTION__, __LINE__);
					/* receive 4-way handshake message 1 */
					lynx_dbg(LYNX_DBG_WM, "=== Handle 4-way handshake M1 ===\n");

					memcpy(ctx->anonce, key->key_nonce, WPA_NONCE_LEN);
					get_random_bytes(ctx->snonce, WPA_NONCE_LEN);

					/* derive PTK */
					wpa_prf(ctx->pmk, PMK_LEN, "Pairwise key expansion", 
						bss->bssid, bss->myaddr, ctx->anonce, ctx->snonce, 
						(u8 *)&ctx->ptk, sizeof(ctx->ptk), 0);
#if 1
					lynx_dbg_dump(LYNX_DBG_WM, "bssid:", "", bss->bssid, 6);
					lynx_dbg_dump(LYNX_DBG_WM, "my addr:", "", bss->myaddr, 6);
					lynx_dbg_dump(LYNX_DBG_WM, "anonce:", "", ctx->anonce, WPA_NONCE_LEN);
					lynx_dbg_dump(LYNX_DBG_WM, "snonce:", "", ctx->snonce, WPA_NONCE_LEN);
					lynx_dbg_dump(LYNX_DBG_WM, "pmk:", "", ctx->pmk, PMK_LEN);
					lynx_dbg_dump(LYNX_DBG_WM, "kck:", "", ctx->ptk.kck, 16);
					lynx_dbg_dump(LYNX_DBG_WM, "kek:", "", ctx->ptk.kek, 16);
					lynx_dbg_dump(LYNX_DBG_WM, "tk1:", "", ctx->ptk.tk1, 16);
					lynx_dbg_dump(LYNX_DBG_WM, "tk2(tx:rx):", "", ctx->ptk.u.tk2, 16);
#endif
					
					/* swap the tx_mic_key & rx_mic_key */
					if(ctx->pairwise == AUTH_CIPHER_TKIP)
						SWAP_MIC_KEY(ctx->ptk.u.auth.tx_mic_key, ctx->ptk.u.auth.rx_mic_key);

					ctx->flags |= WPA_PTK_VALID;

					pos = kde;
					
					if((bss->wpa_ie_len) && (key->type == 254))
						wpa_bit = IE_WPA_BIT;
					else if((bss->rsn_ie_len) && (key->type == 2))
						wpa_bit = IE_RSN_BIT;
					
					pos += wlan_ap_encode_ie(bss, pos, wpa_bit);
					
					/* send 4-way handshake message 2 */
					sta_wpa_send_eapol(bss, KEY_INFO_MIC | KEY_INFO_KEY_TYPE, NULL, ctx->snonce,
									   kde, pos - kde, 0, 0, key->replay_counter);
				}
			}
			else	// group key
			{
				lynx_dbg(LYNX_DBG_WM, "receive group key message\n");
				
				if(key_info & KEY_INFO_SECURE)
				{
					decode_key_data(ctx->ptk.kek, ctx->pairwise, key->key_iv, key->key_data, key_len, kde);
					
					if(ctx->flags & DESC_TYPE_RSN)
					{
						if((idx = rsn_get_gtk(bss->wpa_group->cipher, kde, key_len, &gtk)) < 0)
						{
							lynx_dbg_dump(LYNX_DBG_WCI, "can't get the gtk info in RSN type, the original kde :", "", kde, key_len);
							break;
						}
					}
					else
					{
						idx = ((key_info & KEY_INFO_KEY_INDEX_MASK) >> KEY_INFO_KEY_INDEX_SHIFT);
						gtk = kde;
					}
						
					/* swap gtk tx & rx mic key */
					if(bss->wpa_group->cipher == AUTH_CIPHER_TKIP)
						SWAP_MIC_KEY(gtk+16, gtk+24);

					sta_set_key((struct sta_context *)bss, bss->wpa_group->cipher, 
								KEY_TYPE_GLOBAL_KEY, gtk, idx, idx);
					
					/* send group key handshake message 2 */
					sta_wpa_send_eapol(bss, KEY_INFO_MIC | KEY_INFO_SECURE, NULL, NULL,
									   NULL, 0, 0, 0, key->replay_counter);
					ctx->flags |= WPA_GTK_VALID;

					if(!(sta->flags & WLAN_STA_AUTHORIZED))
					{
						/* open datapath here to avoid the broadcast frame rx before gkey is ok */
						sta->flags |= WLAN_STA_AUTHORIZED;
						mlme_data_path_ctrl(sta->vif->drv_vif, DATAPATH_CARRIER_ON);
						wm_add_timer((unsigned int)sta->vif, SYSTEM_MAINTAIN_DHCP, 5 * WLAN_TIME_UNIT);

#ifdef CONFIG_LYNX_OS_UCOS
						/* FIXME: only for montage ucos */
						/* FIXME: how to setup the device status with the AP ROLE? */
						set_device_status(WIFI_CONNECTED);
#endif
					}
				}
				else
				{
					lynx_dbg(LYNX_DBG_WM, "Wrong key info = 0x%x\n", key_info);
				}
			}
			break;
		}
	}
}

#endif	// CONFIG_WPA

