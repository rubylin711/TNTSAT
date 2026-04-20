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

#ifndef _WLAN_SCAN_H_
#define _WLAN_SCAN_H_

#include <wm_vif.h>

struct scan_cb {
	int (*func) (struct wm_vif *bss, struct sta_context *sta, unsigned long data);
	struct wm_vif *bss;
	struct sta_context *sta;
	unsigned long data;
	struct scan_cb *next;
};

void reset_nbss_entry(struct neighbor_bss *nbss);
void reset_nbss_list(void);
struct neighbor_bss *find_nbss_by_ssid(unsigned char *ssid);
int scan_cb_handler(void);
void flush_scan_cb_list(void);
void add_to_scan_cb_list(int (*cb)(struct wm_vif *, struct sta_context *, unsigned long), struct wm_vif *bss, struct sta_context *sta, unsigned long data);
void ap_handle_beacon_prob_resp(struct wlan_hdr *fm, u32 len, u8 rssi);
#endif // _WLAN_SCAN_H_
