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

#ifndef _WM_MSG_H_
#define _WM_MSG_H_

#ifdef CONFIG_WPS
#include <wps_subr.h>
#endif

/* message type */
enum{
	WM_MSG_WLAN_FRAME = 1,
	WM_MSG_TIMER,
	WM_MSG_CMD,		/* from device & wm_manager */
	WM_MSG_WPS,
	WM_MSG_CTRL,	/* from Application layer */
};

/* AP's command */
enum{
	AP_CMD_RELOAD,
	AP_CMD_SHUTDOWN,
	AP_CMD_SCAN_DONE,
	AP_CMD_ADD_STA,
	AP_CMD_DEL_STA,
	AP_CMD_INFO_INTERFACE_STATUS,
};

enum {
	WM_WF_TYPE_DATA,
	WM_WF_TYPE_MGMT,
};

/* ap message struct */
struct wm_msg {
	u8 type;
	union{
		struct ap_wframe_msg{
			int rssi;
			unsigned char vif_idx;
			unsigned char sta_idx;
			unsigned char type;
		}wf;
		struct ap_timer_msg{
			u8 type;
			u32 data;
		}timer;
		struct ap_cmd_msg{
			u8 cmd;
			u8 vif_idx;
			u32 arg[4];	// skb->cb = 48 bytes
		}cmd;
		struct ctrl_cmd_msg{
			u8 type;
			u8 vif_idx;
		}ctrl;
#ifdef CONFIG_WPS
		struct shm_unit wps;
#endif
	}u;
};


#ifndef WIF_NONE
#define	WIF_NONE				0
#define WIF_STA_ROLE			0x01
#define WIF_AP_ROLE				0x02
#define WIF_IBSS_ROLE			0x03
#define WIF_P2P_CLIENT_ROLE		0x9
#define WIF_P2P_GO_ROLE			0xa
#endif

#ifdef CONFIG_WPS
int upnp_msgq_write(struct wm_msg *, int);
#endif
int wm_add_timer(unsigned int target, unsigned short event, unsigned int next_time);
int wm_del_timer(unsigned int target, unsigned short event);

struct sk_buff *wm_msgq_alloc(int size);
int wm_msgq_write(struct sk_buff *skb);
struct sk_buff *wm_msgq_read(void);

#endif /* _WM_MSG_H_ */
