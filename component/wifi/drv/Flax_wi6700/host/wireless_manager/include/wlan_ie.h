
/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file wlan_ie.h
*   \brief  WLAN Information Element definitions.
*   \author Montage
*/

#ifndef _WLAN_IE_H_
#define _WLAN_IE_H_

#define WLAN_IE_CONTEXT_TO_LEN(_context) \
		(*(char *)(_context - 1))	

u8 high_prioity_bit(u32 flag, u8 upbound);
u8 low_prioity_bit(u32 flag, u8 lowbound);
u32 ht_rate_to_bits(char *supported_mcs_set);	
u32 support_rate_to_bits(u8 *data, u8 len);

int wlan_parse_wpa_rsn_ie(const u8 *start, u32 len, struct wpa_ie_data *info, u8 is_rsn);
int wlan_parse_wapi_ie(const u8 *start, u32 len, void *info);
int wlan_parse_wme_ie(struct wm_vif *bss, const u8 *start, u32 len, u32 survey);
char cw2ecw(short cw);
short ecw2cw(u8 ecw);
u32 wlan_ap_encode_ie(struct wm_vif *bss, u8 *start, u32 encode_bits);
u32 wlan_parse_ie(u8 *pos, u32 left, struct wlan_ie *ie);
u16 ap_encode_wpa_ie(struct wm_vif *bss, u8 *start, u8 is_rsn);
u16 ap_encode_wapi_ie(u8 *start, u8 *bkid);
void wpa_ie_change_group_cipher(u8 *start, u32 len, u32 group_cipher, u8 is_rsn);

#endif /* _WLAN_IE_H_ */
