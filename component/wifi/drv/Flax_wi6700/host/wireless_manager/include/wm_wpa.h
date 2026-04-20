/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file wm_wpa.h
*   \brief  AP WPA utility.
*   \author Montage
*/

#ifndef _WM_WPA_H_
#define _WM_WPA_H_


#include <eapol.h>
#include <wm_vif.h>
#include <wpa.h>

#define WPA_4WAY_HANDSHAKE_TIMEOUT	5*WLAN_TIME_UNIT

void ap_proc_eapol_key(struct wm_vif *bss, struct sta_context *sta, struct eapol_hdr *data, unsigned int data_len);
void wpa_timeout(struct sta_context *sta);
void ap_wpa_rekey_ptk(struct sta_context *sta);
void ap_sta_enter_wpa_state(struct sta_context *sta, int new_state, unsigned char *data, int data_len);

char wpa_group_init(struct wm_vif *bss);
void wpa_group_rekey(struct wm_vif *bss);
void wpa_release(struct sta_context *sta);
struct wpa_ctx *wpa_alloc(struct wm_vif *bss);
int wpa_setup(struct wm_vif *bss, struct wpa_ctx *ctx, const u8 *wpa_ie, size_t wpa_ie_len,
			u8 is_rsn);
void pmk_to_pmkid(const u8 *pmk, size_t pmk_len, const u8 *aa,
		      const u8 *spa, u8 *pmkid, int use_sha256);
void sta_wpa_send_eapol(struct wm_vif *bss, u16 key_info, const u8 *key_rsc, const u8 *nonce,
		      const u8 *kde, u32 kde_len, u32 keyidx, u8 encr, u8 *rpl_cnter);
#endif // _WM_WPA_H_

