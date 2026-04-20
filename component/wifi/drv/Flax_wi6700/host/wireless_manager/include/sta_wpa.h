/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file sta_wpa.h
*   \brief  station mode configuration
*   \author Montage
*/

#ifndef _STA_WPA_H_
#define _STA_WPA_H_

unsigned char *sta_alloc_data_frame(const unsigned char *bssid, unsigned short ether_type, 
		unsigned char *saddr, unsigned int data_len, unsigned char qos, unsigned char **data_ptr);
void sta_enter_wpa_state(struct sta_context *sta, u8 new_state, unsigned char *data, int data_len);
void sta_receive_eapol(struct sta_context *sta, struct eapol_hdr *eapol, u32 data_len);
void sta_mode_release(int active);
void sta_new_assoc(struct wm_bss *bss);
void wpa_sta_proc_eapol_key(struct sta_context *sta, struct eapol_hdr *data, u32 data_len);

#ifdef CONFIG_WPS
int wps_send_eapol_start(struct wm_bss *bss);
#endif
#endif //_STA_WPA_H_
