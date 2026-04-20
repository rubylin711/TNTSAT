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

#ifndef _WM_STA_H_
#define _WM_STA_H_

#include <wm_vif.h>
#ifdef CONFIG_WPA	
#include <wpa.h>
#endif
#ifdef CONFIG_WPS
#include <eap.h>
#endif

#ifndef BIT
#define BIT(s) (1 << (s))
#endif

#define WLAN_STA_VALID				BIT(0)
#define WLAN_STA_CONNECTED			BIT(1)
#define WLAN_STA_AUTHORIZED			BIT(2)
#define WLAN_STA_QOS				BIT(3)
#define WLAN_STA_HT					BIT(4)
#define WLAN_STA_ERP				BIT(5)
#define WLAN_STA_SHORT_SLOT			BIT(6)
#define WLAN_STA_SHORT_PREAMBLE		BIT(7)
#define WLAN_STA_HT_40M				BIT(8)
#define WLAN_STA_HT_GF				BIT(9)
#define WLAN_STA_WPS				BIT(10)
#define WLAN_STA_MAYBE_WPS			BIT(11)
#define WLAN_STA_AUTH_SHARED		BIT(12)
#define WLAN_STA_FORTY_INTOLERANT	BIT(13)		// dis-allow 20/40Mhz or not
#define WLAN_STA_TRY_CONN_FAIL		BIT(14)

#if defined(CONFIG_LYNX_OS_LINUX)
#define WLAN_TIME_UNIT				HZ
#elif defined(CONFIG_LYNX_OS_UCOS)
#define WLAN_TIME_UNIT				100	//
#endif

#define STA_TRY_ASSOC_TIME			(5*WLAN_TIME_UNIT)

#define STA_RESET_FLAGS		0	//STA_RELEASE_WITHOUT_FREE_MEM
#define STA_FREE_MEM		1	//STA_RELEASE_WITH_FREE_MEM


/* station context */
struct sta_context {
	struct sta_context *next;
	struct wm_vif *vif;

	unsigned char	addr[WLAN_ADDR_LEN];
	unsigned int	flags;

#ifdef CONFIG_WPA	
	struct wpa_ctx *wpa_ctx;
#endif
#ifdef CONFIG_WAPI
	struct wapi_ctx *wapi_ctx;
#endif
#ifdef CONFIG_WPS
	struct eap_ctx *eap_ctx;
#endif
};

struct sta_context *ap_add_sta(struct wm_vif *bss, unsigned char *addr);
void ap_free_sta(struct sta_context *sta);
void ap_free_sta_all(struct wm_vif *bss);
struct sta_context *ap_find_sta(struct wm_vif *bss, unsigned char *addr);
void ap_release_all_sta(struct wm_vif *bss, int policy);
void ap_release_sta(struct sta_context *sta, char release);
void ap_sta_deauth(struct sta_context *sta, int reason, char release);

int sta_connect(struct wm_vif *sta_bss, struct sta_context *sta, unsigned long data);
int sta_connected(struct wm_vif *bss, unsigned char *addr, int data_len, unsigned char *data);
int sta_disconnect(struct wm_vif *bss, unsigned char *addr, int reconnect, int reason);

void sta_set_key(struct sta_context *sta, unsigned char cipher_type, unsigned char key_type, char *key, char keyidx, char is_txkey);
#endif /* _WM_STA_H_ */
