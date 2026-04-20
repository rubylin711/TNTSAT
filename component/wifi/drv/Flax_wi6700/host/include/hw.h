/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Acrospeed Inc. All right reserved.                                           |
|                                                                              |
+=============================================================================*/
/*! 
*   \file 
*   \brief
*   \author Acrospeed
*/

#ifndef __HW_H__
#define __HW_H__

enum lynx_ep_id {
	END_POINT_UNUSED = -1,
	END_POINT_0 = 0,
	END_POINT_1 = 1,
	END_POINT_2 = 2,
	END_POINT_3 = 3,
	END_POINT_4 = 4,
	END_POINT_5 = 5,
	END_POINT_6 = 6,
	END_POINT_7 = 7,
	END_POINT_8 = 8,
	END_POINT_9 = 9,
	END_POINT_MAX = 16
};

struct lynx_ep_callbacks {
	void (*tx) (void *, struct sk_buff *, enum lynx_ep_id, bool txok);
	void (*rx) (void *, struct sk_buff *, enum lynx_ep_id);
};  

struct lynx_endpoint {
	u8 epid;
	u16 max_packet_size;                     /* max_packet_size is only used in out-direction */
	struct lynx_ep_callbacks ep_callbacks;
};

struct lynx_hw_usb {
	struct lynx_endpoint ep[END_POINT_MAX];
	u8 (*gettx_epid) (u8 id);
	u8 (*getrx_epid) (u8 id);
	u8 (*valid_epid) (u8 id);
};

struct lynx_hw_sdio {
    int test;
};

struct lynx_hw {
	u16 vendor_id;
	u16 device_id;
	u16 version;
	u16 subversion;
	enum lynx_hif_type type;
	const char name[10];
	void (*init) (struct lynx_hif_device *hdev);
  	struct lynx_hw_usb usb;
   	struct lynx_hw_sdio sdio;
};

int lynx_hw_init(struct lynx_hif_device *hdev);

#endif
