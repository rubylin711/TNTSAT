/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file eapol.h
*   \brief 
*   \author Montage
*/

#ifndef _EAPOL_H_
#define _EAPOL_H_

struct eapol_hdr {
	u8 ver;
	u8 type;
	u16 len;
	u8 data[0];
} __attribute__ ((packed));


#define EAPOL_VERSION 2

enum {
	EAP_PACKET = 0,
	EAPOL_START = 1,
	EAPOL_LOGOFF = 2,
	EAPOL_KEY = 3,
	EAPOL_ENCAPSULATED_ASF_ALERT = 4
};

enum { 
	EAPOL_KEY_DESC_TYPE_RC4 = 1, 
	EAPOL_KEY_DESC_TYPE_RSN = 2,
	EAPOL_KEY_DESC_TYPE_WPA = 254 
};

struct eapol_key_info {
	u16  :2;
	u16 smk:1;
	u16 encrypted_key_data:1;
	u16 request:1;
	u16 error:1;
	u16 secure:1;
	u16 mic:1;
	u16 ack:1;
	u16 install:1;
	u16 key_id:2;	/* bit4..5 is used in WPA, but is reserved in IEEE 802.11i/RSN */
	u16 type:1;	/* 1 = Pairwise, 0 = Group key */
	u16 ver:3;
} __attribute__ ((packed));

struct eapol_key {
	u8 type;
	struct eapol_key_info key_info;
	u8 key_len[2];
	u8 replay_counter[WPA_REPLAY_COUNTER_LEN];
	u8 key_nonce[WPA_NONCE_LEN];
	u8 key_iv[16];
	u8 key_rsc[WPA_KEY_RSC_LEN];
	u8 key_id[8]; /* Reserved in IEEE 802.11i/RSN */
	u8 key_mic[16];
	u8 key_data_len[2];
	/* followed by key_data_length bytes of key_data */
	u8 key_data[0];
} __attribute__ ((packed));


/* IEEE 802.11, 8.5.2 EAPOL-Key frames */
#define KEY_INFO_TYPE_MASK ((u16) (BIT(0) | BIT(1) | BIT(2)))
#define KEY_INFO_TYPE_HMAC_MD5_RC4 BIT(0)
#define KEY_INFO_TYPE_HMAC_SHA1_AES BIT(1)
#define KEY_INFO_TYPE_AES_128_CMAC 3
#define KEY_INFO_KEY_TYPE BIT(3) /* 1 = Pairwise, 0 = Group key */
/* bit4..5 is used in WPA, but is reserved in IEEE 802.11i/RSN */
#define KEY_INFO_KEY_INDEX_MASK		(BIT(4) | BIT(5))
#define KEY_INFO_KEY_INDEX_SHIFT 4
#define KEY_INFO_INSTALL			BIT(6) /* pairwise */
#define KEY_INFO_TXRX				BIT(6) /* group */
#define KEY_INFO_ACK				BIT(7)
#define KEY_INFO_MIC				BIT(8)
#define KEY_INFO_SECURE				BIT(9)
#define KEY_INFO_ERROR				BIT(10)
#define KEY_INFO_REQUEST			BIT(11)
#define KEY_INFO_ENCR_KEY_DATA		BIT(12) /* IEEE 802.11i/RSN only */
#define KEY_INFO_SMK_MESSAGE		BIT(13)

void ap_proc_eapol(struct wm_vif *bss, struct sta_context *sta, struct eapol_hdr *eapol, unsigned int data_len);

#endif /* _EAPOL_H_ */ 
