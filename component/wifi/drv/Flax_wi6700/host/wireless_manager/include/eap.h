/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file eap.h
*   \brief  
*   \author Montage
*/

#ifndef __EAP_H__
#define __EAP_H__

#define WLAN_EID_VENDOR_SPECIFIC 221

enum eap_code { 
	EAP_CODE_REQUEST = 1, 
	EAP_CODE_RESPONSE = 2, 
	EAP_CODE_SUCCESS = 3,
	EAP_CODE_FAILURE = 4 
};

/*
 * EAP Method Types as allocated by IANA:
 * http://www.iana.org/assignments/eap-numbers
 */
enum eap_type {
	EAP_TYPE_NONE = 0,
	EAP_TYPE_IDENTITY = 1 /* RFC 3748 */,
	EAP_TYPE_NOTIFICATION = 2 /* RFC 3748 */,
	EAP_TYPE_NAK = 3 /* Response only, RFC 3748 */,
	EAP_TYPE_MD5 = 4, /* RFC 3748 */
	EAP_TYPE_OTP = 5 /* RFC 3748 */,
	EAP_TYPE_GTC = 6, /* RFC 3748 */
	EAP_TYPE_TLS = 13 /* RFC 2716 */,
	EAP_TYPE_LEAP = 17 /* Cisco proprietary */,
	EAP_TYPE_SIM = 18 /* RFC 4186 */,
	EAP_TYPE_TTLS = 21 /* RFC 5281 */,
	EAP_TYPE_AKA = 23 /* RFC 4187 */,
	EAP_TYPE_PEAP = 25 /* draft-josefsson-pppext-eap-tls-eap-06.txt */,
	EAP_TYPE_MSCHAPV2 = 26 /* draft-kamath-pppext-eap-mschapv2-00.txt */,
	EAP_TYPE_TLV = 33 /* draft-josefsson-pppext-eap-tls-eap-07.txt */,
	EAP_TYPE_TNC = 38 /* TNC IF-T v1.0-r3; note: tentative assignment;
			   * type 38 has previously been allocated for
			   * EAP-HTTP Digest, (funk.com) */,
	EAP_TYPE_FAST = 43 /* RFC 4851 */,
	EAP_TYPE_PAX = 46 /* RFC 4746 */,
	EAP_TYPE_PSK = 47 /* RFC 4764 */,
	EAP_TYPE_SAKE = 48 /* RFC 4763 */,
	EAP_TYPE_IKEV2 = 49 /* RFC 5106 */,
	EAP_TYPE_AKA_PRIME = 50 /* draft-arkko-eap-aka-kdf-10.txt */,
	EAP_TYPE_GPSK = 51 /* RFC 5433 */,
	EAP_TYPE_EXPANDED = 254 /* RFC 3748 */
};

enum eap_vendor_id {
	EAP_VENDOR_IETF = 0,
	EAP_VENDOR_MICROSOFT = 0x000137 /* Microsoft */,
	EAP_VENDOR_WFA = 0x00372A /* Wi-Fi Alliance */
};

/* EAP method related */
enum eap_method_state {
   	EAP_METHOD_NONE, 						/* initial stat */
	EAP_METHOD_SELECTED, 					/* method picked-up*/
   	EAP_METHOD_CONT, 						/* method working period */
   	EAP_METHOD_SUCCESS, EAP_METHOD_FAIL		/* method final state */
};

enum eap_flags {
	EAP_FLAG_CLEAR = 0,
	EAP_FLAG_DONE = 0x1,
	EAP_FLAG_RECV_LOCK = 0x2,
	EAP_FLAG_SECU = 0x4
};

struct eap_method {
	enum eap_vendor_id vendor;
	enum eap_type type;
	void *priv;

	void *(*init)(void *cfg);	
	void (*deinit)(void *priv);
	char (*pkt_proc)(void *priv, void *msg, u32 mlen);
};

struct eap_ctx {
	enum {
		EAP_MODE_NONE = 0,
		EAP_MODE_METHOD,
		EAP_MODE_PASS_THROUGH
	} mode;

	struct sta_context *sta_ctx;			/* back point */

	u8 curId;

	u8 *identity;
	u32 identity_len;

	enum eap_method_state method_state;
	struct eap_method method;

	struct eapol_hdr *req_eapol; 		/* send: points to eapol in send_frame */
	void *send_frame;					/* send: whole frame */
	u32 send_frame_size;

	u8 flags;							/* done, recv_lock, security */
	u8 rexmit_cnt;

	/* specific method only data */
	u8 server;							/* server or peer */
};
/* EAP management structure */

/* EAP method related */

/* EAP packet structure */
struct eap_hdr {
	u8 code;
	u8 identifier;
	u16 len;
	u8 type;
} __attribute__ ((packed));

struct eap_packet {
	struct eap_hdr hdr;
	u8 data[0];
};

#define EAP_HEADER_SIZE		sizeof(struct eap_hdr)
/* EAP packet structure */

/* EAP management structure */
struct wps_registrar;

/* Public Interfaces */
u8 *ap_eap_alloc(struct eap_ctx *eap, u32 alloc_size, 
				  u8 **payload, u32 *payload_size);

void ap_eap_method_send(struct eap_ctx *eap, u8 *pkt, u32 pkt_len);
void ap_eap_aaa_send(struct eap_ctx *eap, u8 *pkt, u32 pkt_len);
void ap_eapol_restart(struct eap_ctx *eap);
void ap_eapol_rexmit(struct eap_ctx *eap);

struct eap_ctx *eap_init(struct sta_context *sta);
void eap_reinit(struct eap_ctx *eap);
void eap_deinit(struct eap_ctx *eap);

#endif /* __EAP_H__ */
