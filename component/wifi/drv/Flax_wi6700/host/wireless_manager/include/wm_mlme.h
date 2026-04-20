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

#ifndef _WLAN_MLME_H_
#define _WLAN_MLME_H_

#include <wm_vif.h>

void wm_frame_handler(struct sk_buff *skb, int vif_idx, int sta_idx, int type, int rssi);
void ap_handle_channel_switch(unsigned int channel, unsigned int secondary_ch);
struct sk_buff *alloc_data_frame(struct wm_vif *bss, unsigned short ether_type, unsigned char *daddr, unsigned int data_len, unsigned char **data_ptr);

#endif  // _WLAN_MLME_H_
