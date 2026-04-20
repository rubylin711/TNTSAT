/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file ap_eap.c
*   \brief  
*   \author Montage
*/
#ifdef CONFIG_LYNX_OS_UCOS
/* ucos */
#include <mt_type.h>
#include <os/mtos_mem.h>
#include <os/mtos_misc.h>
#include <os/mtos_task.h>
#include <os/mtos_printk.h>
#include <os/mtos_sem.h>
#include <porting.h>
#include <util/list.h>
#include <util/lib_bitops.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "init.h"
#include "core.h"
#include <wlan_def.h>
#include <wm_vif.h>
#include <wm_sta.h>
#include <wm_msg.h>
#include <wm_wpa.h>
#include <wm_mlme.h>
#include <wpa.h>
#include <eapol.h>
#include <eap.h>
#include <wlan_ie.h>
#include <sta_wpa.h>
#include "lynx_debug.h"

#ifdef CONFIG_WPS
#include <wps.h>	//fro method register
#endif

#define EAP_DEBUG printd
#define DEBUG_TRAP(c, s) \
	if (c) {		\
		panic(s);	\
	}

#define EAP_REXMIT
#define EAP_REXMIT_MAX 3


#ifdef CONFIG_WPS
static void ewm_msg_send(struct eap_ctx *eap, struct eap_packet *eap_pkt, u32 pkt_len);

	/* EAP proc functions */
static u8 eap_nextId(int id)
{
	if (id < 0) {
		/* RFC 3748 Ch 4.1: recommended to initialize Identifier with a
		 * random number */
		id = rand() & 0xff;
		//if (id != sm->lastId)
		return id;
	}
	return(id + 1) & 0xff;
}

#define eap_initId()	eap_nextId(-1)

static void eap_free_frame(struct eap_ctx *eap)
{
	if (eap && eap->send_frame){
		lynx_dbg(LYNX_DBG_WM, "eap_free_frame: [%08x]\n", eap->send_frame);
		free(eap->send_frame);
		eap->send_frame = eap->req_eapol = NULL;
		eap->send_frame_size = 0;
	}
}

static struct eap_packet *eap_alloc_frame(struct eap_ctx *eap, u32 eap_size)
{
	u32 alloc_size = eap_size + sizeof(struct eapol_hdr);
	
#if 1
	if (eap->send_frame) {
		lynx_dbg(LYNX_DBG_WM, "EAP req_pkt already exist\n");
		return NULL;
	}
#endif

	if(eap->sta_ctx->vif->role == WIF_AP_ROLE)
	{
		eap->send_frame = (void *)ap_alloc_data_frame(eap->sta_ctx->vif, ETHERTYPE_EAPOL, 
										eap->sta_ctx->addr, alloc_size, 
										(eap->sta_ctx->flags & WLAN_STA_QOS)?1:0, 
										(u8 **)&eap->req_eapol);
	}
	else	// if(eap->sta_ctx->vif->role == WIF_STA_ROLE)
	{
		eap->send_frame = (void *)sta_alloc_data_frame(eap->sta_ctx->vif->bssid, ETHERTYPE_EAPOL, 
										eap->sta_ctx->addr, alloc_size, 
										(eap->sta_ctx->flags & WLAN_STA_QOS)?1:0, 
										(u8 **)&eap->req_eapol);
	}

	if (eap->send_frame == NULL) {
		return NULL;
	}

	/* eapol basic attributes */
	eap->req_eapol->ver = EAPOL_VERSION;
	eap->req_eapol->type = EAP_PACKET;
	eap->req_eapol->len = eap_size;	/* handled in send pkt */

	return (struct eap_packet *)eap->req_eapol->data;
}

static void eap_send_frame(struct eap_ctx *eap)
{
	if (eap->send_frame && eap->send_frame_size) 
	{
		lynx_dbg(LYNX_DBG_WM, "eap_send_frame: EAP ID %d flags %04x [%08x]\n", 
				  eap->curId, eap->flags, eap->send_frame);

		ap_send_frame(eap->sta_ctx->vif, eap->send_frame, eap->send_frame_size, 
					(eap->flags & EAP_FLAG_SECU ? WBUF_TX_SPEC_RATE : (WBUF_TX_SPEC_RATE|WBUF_TX_NO_SECU)), eap->sta_ctx); //no secrity TX
		eap->flags &= ~EAP_FLAG_RECV_LOCK;

		if (eap->flags & EAP_FLAG_DONE){
			lynx_dbg(LYNX_DBG_WM, "eap_send_frame:DONE free [%08x]\n", eap->send_frame);
			eap_free_frame(eap);
		}
		else {
#ifdef EAP_REXMIT
			lynx_dbg(LYNX_DBG_WM, "eap_send_frame: rexmit timer trigger!!!!!!!!!!!!!\n");
			wm_add_timer((u32) eap->sta_ctx, EAPOL_REXMIT_TIMEOUT, (1*WLAN_TIME_UNIT)); //1 secs
#endif
		}
	}
}

static int eap_send_success_fail(struct eap_ctx *eap, u8 success)
{
	struct eap_packet *eap_pkt;

	eap_pkt = (struct eap_packet *)ap_eap_alloc(eap, 4, NULL, NULL);
	if (eap_pkt == NULL) 
		return -1;

	eap_pkt->hdr.code = success ? EAP_CODE_SUCCESS : EAP_CODE_FAILURE;
	eap_pkt->hdr.identifier = eap->curId;
    eap_pkt->hdr.len = 4;

	ewm_msg_send(eap, eap_pkt, eap_pkt->hdr.len);
	free(eap_pkt);

	return 0;
}

static int eap_send_id_request(struct eap_ctx *eap)
{
	struct eap_packet *eap_pkt;

	eap_pkt = (struct eap_packet *)ap_eap_alloc(eap, 100, NULL, NULL);
	if (eap_pkt == NULL) 
		return -1;

	lynx_dbg(LYNX_DBG_WM, "EAP: send ID Req\n");

	eap_pkt->hdr.code = EAP_CODE_REQUEST;
	eap_pkt->hdr.identifier = eap->curId = eap_nextId(eap->curId);
	eap_pkt->hdr.len = EAP_HEADER_SIZE;
	eap_pkt->hdr.type = EAP_TYPE_IDENTITY;

	ewm_msg_send(eap, eap_pkt, eap_pkt->hdr.len);
	free(eap_pkt);

	return 0;
}

static inline u32 calculate_curReqPkt_size(struct eap_ctx *eap)
{
	struct eap_packet *eap_pkt = (struct eap_packet *)eap->req_eapol->data;
	u32 head, tail;

	head = (u32)eap->send_frame;
	tail = (u32)((u32)eap_pkt + (u32)eap_pkt->hdr.len);

	return (tail - head);
}

static void ewm_msg_send(struct eap_ctx *eap, struct eap_packet *eap_pkt, u32 pkt_len)
{
	u8 *eap_head;
	
	if (eap_pkt->hdr.code == EAP_CODE_SUCCESS || 
		eap_pkt->hdr.code == EAP_CODE_FAILURE) {
		eap->flags |= EAP_FLAG_DONE;
	}

	if((eap_head = (u8 *) eap_alloc_frame(eap, pkt_len)) == NULL) {
		lynx_dbg(LYNX_DBG_WM, "%s: eap_alloc_frame is failed!!\n", __func__);
		return;
	}

	memcpy(eap_head, (void *)eap_pkt, pkt_len);
	eap->send_frame_size = calculate_curReqPkt_size(eap);
	eap_send_frame(eap);

#if 0
	if (eap_pkt->hdr.code == EAP_CODE_FAILURE) {
		wm_add_timer(eap->sta_ctx, STA_IDLE_TIMEOUT, 10*WLAN_TIME_UNIT);
	}
#endif
}
#endif	// CONFIG_WPS
#ifdef CONFIG_WPS
static int eap_recv_id_response(struct eap_ctx *eap, struct eap_packet *pkt, u32 pkt_len)
{
	/* some validations */
	if (pkt->hdr.code != EAP_CODE_RESPONSE)
		return -1;

	lynx_dbg(LYNX_DBG_WM, "EAP: recv ID Rsp\n");

	eap->identity_len = pkt_len - EAP_HEADER_SIZE;
	if ((eap->identity = malloc(eap->identity_len)) == NULL)
		return -1;
	memcpy(eap->identity, pkt->data, eap->identity_len);

	if (wps_identity_check(eap->identity, eap->identity_len, &eap->server)) 
	{
   		struct wps_config cfg;

		eap->method_state = EAP_METHOD_SELECTED;
		eap->mode = EAP_MODE_METHOD;

		cfg.registrar = eap->server;
		cfg.cb_ctx = eap;
		cfg.wps = eap->sta_ctx->vif->wps;

		if ((eap->method.priv = eap->method.init(&cfg))) {
			eap->flags &= ~EAP_FLAG_SECU;
			eap->method.pkt_proc(eap->method.priv, NULL, 0);
			return 0;
		}
	}
	
	return -1;
}
#endif

#ifdef CONFIG_WPS
static int eap_recv_id_request(struct eap_ctx *eap, struct eap_packet *pkt, u32 pkt_len)
{
	struct eap_packet *eap_pkt;

	/* some validations */
	if((pkt->hdr.code != EAP_CODE_REQUEST) || (pkt->hdr.type != EAP_TYPE_IDENTITY))
		return -1;

	lynx_dbg(LYNX_DBG_WM, "EAP: recv ID Req\n");

	eap->identity_len = EAP_WSC_ID_ENROLLEE_LEN;
	if ((eap->identity = realloc(eap->identity, eap->identity_len)) == NULL)
		return -1;

	memcpy(eap->identity, EAP_WSC_ID_ENROLLEE, eap->identity_len);

	eap_pkt = (struct eap_packet *)ap_eap_alloc(eap, 100, NULL, NULL);
	if (eap_pkt == NULL) 
		return -1;

	lynx_dbg(LYNX_DBG_WM, "EAP: send ID Resp\n");

	eap_pkt->hdr.code = EAP_CODE_RESPONSE;
	eap_pkt->hdr.identifier = eap->curId;
	eap_pkt->hdr.len = EAP_HEADER_SIZE + eap->identity_len;
	eap_pkt->hdr.type = EAP_TYPE_IDENTITY;
	memcpy(eap_pkt->data, eap->identity, eap->identity_len);

	ewm_msg_send(eap, eap_pkt, eap_pkt->hdr.len);
	free(eap_pkt);

	struct wps_config cfg;

	eap->method_state = EAP_METHOD_SELECTED;
	eap->mode = EAP_MODE_METHOD;

	cfg.registrar = 0;
	cfg.cb_ctx = eap;
	cfg.wps = eap->sta_ctx->vif->wps;
	if ((eap->method.priv = eap->method.init(&cfg))) 
	{
		eap->flags &= ~EAP_FLAG_SECU;
		return 0;
	}

	return 0;
}
#endif	// CONFIG_WPS

#ifdef CONFIG_WPS
static void eapol_start_proc(struct eap_ctx *eap, struct eapol_hdr *eapol)
{
	eap_send_id_request(eap);
}

u8 eap_packet_proc(struct eap_ctx *eap, struct eap_packet *pkt, u32 pkt_len)
{
	lynx_dbg(LYNX_DBG_WM, "eap_packet_proc: pkt id %d\n", pkt->hdr.identifier);

	/* drop frame if recv_lock =1 or identifier number are not match */
	if ((eap->flags & EAP_FLAG_RECV_LOCK) || pkt->hdr.identifier != eap->curId) {
		lynx_dbg(LYNX_DBG_WM, "eap_packet_proc: dup frame drop......flags %04x pkt id %d\n",
				  eap->flags, pkt->hdr.identifier);
		return -1;
	}
	else{
#ifdef EAP_REXMIT
		wm_del_timer((u32) eap->sta_ctx, EAPOL_REXMIT_TIMEOUT);
		eap->rexmit_cnt = 0;
#endif
		eap->flags |= EAP_FLAG_RECV_LOCK;
		lynx_dbg(LYNX_DBG_WM, "eap_packet_proc: flags %04x rexmit_cnt %d\n", eap->flags, eap->rexmit_cnt);
		eap_free_frame(eap);
	}

	if (eap->mode == EAP_MODE_PASS_THROUGH) 
	{
		lynx_dbg(LYNX_DBG_WM, "Enter EAP_PASS_THROUGH.........\n");
	}
	else if (eap->mode == EAP_MODE_METHOD) 
	{
   		eap->method_state = EAP_METHOD_FAIL;
		if(eap->method.pkt_proc)
		{
			lynx_dbg(LYNX_DBG_WM, "Enter EAP METHOD.........\n");
			eap->method_state = eap->method.pkt_proc(eap->method.priv, pkt, pkt_len);
		}
   
   		if (eap->method_state == EAP_METHOD_FAIL) {
			eap_send_success_fail(eap, 0);
	
			/* deauth the sta after the WPS progress has completed */
			ap_sta_deauth(eap->sta_ctx, WLAN_STATUS_UNSPECIFIED_FAILURE, STA_FREE_MEM);
   		} else if (eap->method_state == EAP_METHOD_SUCCESS) {
			eap_send_success_fail(eap, 1);
   		} else {
   			lynx_dbg(LYNX_DBG_WM, "EAP Method continue....\n");
   		}
	}
#if defined(CONFIG_WPS)
	else if((eap->sta_ctx->vif->role == WIF_STA_ROLE) && 
				(eap->sta_ctx->vif->wps->sta_status == WPS_STA_MESSAGE_EXCHANGE))
	{
		return eap_recv_id_request(eap, pkt, pkt_len);
	}

#endif
	else
		return eap_recv_id_response(eap, pkt, pkt_len);

	return 0;
}

void eap_reinit(struct eap_ctx *eap)
{
	if (eap->identity){ 
		free(eap->identity);
		eap->identity = NULL;
	}
	if (eap->method.priv && eap->method.deinit) {
		eap->method.deinit(eap->method.priv);
		eap->method.priv = NULL;
	}

	eap_free_frame(eap);

	eap->method_state = EAP_METHOD_NONE;
#ifdef CONFIG_WPS
	eap->method.vendor = EAP_VENDOR_WFA;
   	eap->method.type =  (enum eap_type) EAP_VENDOR_TYPE_WSC;
   	eap->method.init = (void *(*)(void *))wps_init;
   	eap->method.deinit = (void (*)(void *))wps_deinit;
   	eap->method.pkt_proc= (char (*)(void *, void*, u32))wps_eap_proc;
#endif

	eap->flags = EAP_FLAG_CLEAR | (eap->flags & EAP_FLAG_SECU);
	eap->curId = eap_initId();
	eap->mode = EAP_MODE_NONE;
}

void eap_deinit(struct eap_ctx *eap)
{
	lynx_dbg(LYNX_DBG_WM, "EAP: deinit\n");
	
	eap_reinit(eap);
	free(eap);
}

struct eap_ctx *eap_init(struct sta_context *sta) 
{
	struct eap_ctx *eap;

	eap = malloc(sizeof(*eap));
	if (eap == NULL)
		return NULL;
	memset(eap, 0, sizeof(*eap));

	eap_reinit(eap);

	eap->sta_ctx = sta;

	return eap;
}
/* EAP proc functions */

/* Entrance function */
void ap_eapol_restart(struct eap_ctx *eap)
{
	eap_reinit(eap);
	eapol_start_proc(eap, NULL);
}

void ap_eapol_rexmit(struct eap_ctx *eap)
{
#ifdef EAP_REXMIT
	lynx_dbg(LYNX_DBG_WM, "ap_eapol_rexmit: cnt %d [%08x]\n", eap->rexmit_cnt, eap->send_frame);

	if((eap->flags & EAP_FLAG_RECV_LOCK) == 0)
	{
		if (eap->rexmit_cnt < EAP_REXMIT_MAX)
		{
			lynx_dbg(LYNX_DBG_WM, "ap_eapol_rexmit: do rexmit [%08x]!!\n", eap->send_frame);
			eap->rexmit_cnt++;
			eap_send_frame(eap);
		}
		else
		{
			wm_del_timer( (u32) eap->sta_ctx, EAPOL_REXMIT_TIMEOUT);
			lynx_dbg(LYNX_DBG_WM, "ap_eapol_rexmit: rexmit too many times => EAP FAIL\n");
			eap_free_frame(eap);
			eap_send_success_fail(eap, 0);
		}
	}
	else
		wm_del_timer((u32) eap->sta_ctx, EAPOL_REXMIT_TIMEOUT);
#endif
}

u8 *ap_eap_alloc(struct eap_ctx *eap, u32 alloc_size, u8 **payload, u32 *payload_size)
{
	void *p;
	struct eap_packet *eap_pkt;

	p = malloc(alloc_size);
	if (p == NULL) 
		return NULL;
	memset(p, 0, alloc_size);

	eap_pkt = (struct eap_packet *)p;
	eap_pkt->hdr.len = EAP_HEADER_SIZE;

	if (payload && payload_size) {
		*payload = eap_pkt->data;
		*payload_size = alloc_size - EAP_HEADER_SIZE;
	}

	return (u8 *)eap_pkt;
}

void ap_eap_method_send(struct eap_ctx *eap, u8 *pkt, u32 pkt_len)
{
	struct eap_packet *eap_pkt = (struct eap_packet *)pkt;

#if defined(CONFIG_WPS)
	if(eap->sta_ctx->vif->wps->is_enrollee)
	{
		eap_pkt->hdr.code = EAP_CODE_RESPONSE;
   		eap_pkt->hdr.identifier = eap->curId;
	}
	else
#endif // CONFIG_WPS
	{
		eap_pkt->hdr.code = EAP_CODE_REQUEST;
   		eap_pkt->hdr.identifier = eap->curId = eap_nextId(eap->curId);
	}	
   	eap_pkt->hdr.len = pkt_len;

#ifdef CONFIG_WPS
   	if (eap->mode == EAP_MODE_METHOD && 
		(eap->method.vendor == EAP_VENDOR_WFA && eap->method.type == EAP_VENDOR_TYPE_WSC)) {
   		eap_pkt->hdr.type = EAP_TYPE_EXPANDED;
   	}
   	else
#endif
	{
   		lynx_dbg(LYNX_DBG_WM, "ap_eap_method_send: method is not support\n");
		return;
	}

	ewm_msg_send(eap, eap_pkt, eap_pkt->hdr.len);
}

void ap_eap_aaa_send(struct eap_ctx *eap, u8 *pkt, u32 pkt_len)
{
	struct eap_packet *eap_pkt = (struct eap_packet *)pkt;

	lynx_dbg(LYNX_DBG_WM, "ap_eap_aaa_send: \n");
	/* dont touch INPUT eap packet */
    eap->curId = eap_pkt->hdr.identifier;
	ewm_msg_send(eap, eap_pkt, pkt_len);
}
#endif // CONFIG_WPS

#ifdef CONFIG_WPA	
void ap_proc_eapol(struct wm_vif *bss, struct sta_context *sta, struct eapol_hdr *eapol, unsigned int data_len)
{
	lynx_dbg(LYNX_DBG_WM, ">>>RX eapol type=%d\n", eapol->type);

	switch (eapol->type) 
	{
	case EAPOL_KEY:
		if(!sta->wpa_ctx)
			break;
		ap_proc_eapol_key(bss, sta, eapol, data_len);
		break;
#ifdef CONFIG_WPS
	case EAPOL_START:
	case EAP_PACKET:
		/*	WPS SPEC 7.2 p51: 
			An AP must ignore EAPOL-start frames received from clients that associated 
			to AP with an RSN or SSN IE indicating a WPA2-PSK/WPA-PSK authentication method in 
			the association request 
		*/
		if(sta->wpa_ctx && (key_mgt_type(sta->wpa_ctx->key_mgt) == AUTH_CAP_KEY_MGT_ALL_PSK))
			break;

		if(!(sta->flags & (WLAN_STA_WPS|WLAN_STA_MAYBE_WPS)) && 
			(key_mgt_type(high_prioity_bit(AUTH_CAP_TO_MGT_TYPE(bss->auth_capability), 16)) !=
			 AUTH_CAP_KEY_MGT_ALL_1X))
			break;

		if(sta->eap_ctx == NULL)
		{
			sta->eap_ctx = eap_init(sta);
    		if(sta->eap_ctx == NULL) 
			{
    			/* send fail */
    			return;
    		}
			lynx_dbg(LYNX_DBG_WM, "EAP:sta [%08x] eap [%08x] create!!!\n", sta, sta->eap_ctx);
		}
	
		if(eapol->type == EAPOL_START) 
			ap_eapol_restart(sta->eap_ctx);
		else
			eap_packet_proc(sta->eap_ctx, (struct eap_packet *)&eapol->data, eapol->len);
		
		break;
	case EAPOL_LOGOFF:
		break;
#endif
	default:
		lynx_dbg(LYNX_DBG_WM, "received unknown eapol type=%d\n", eapol->type);
		break; 
	}
}

void sta_receive_eapol(struct sta_context *sta, struct eapol_hdr *eapol, u32 data_len)
{
	if (eapol->type == EAPOL_KEY) 
	{
		lynx_dbg(LYNX_DBG_WM, ">>>STA RX EAPOL_KEY\n");
		wpa_sta_proc_eapol_key(sta, eapol, data_len);
	}
#ifdef CONFIG_WPS
	else if(eapol->type == EAP_PACKET)
	{
		if(sta->eap_ctx == NULL)
		{
			lynx_dbg(LYNX_DBG_WM, "%s(): eap_ctx is empty\n", __FUNCTION__);
			return;
		}

		sta->eap_ctx->curId = ((struct eap_packet *)&eapol->data)->hdr.identifier;
		eap_packet_proc(sta->eap_ctx, (struct eap_packet *)&eapol->data, eapol->len);
	}
#endif // CONFIG_WPS
	else
		lynx_dbg(LYNX_DBG_WM, "sta_proc_eapol: wrong eapol type %d\n", eapol->type);
}
#endif	// CONFIG_WPA	

#ifdef CONFIG_WPS
int wps_send_eapol_start(struct wm_bss *bss)
{
	struct wlan_hdr *hdr;
	struct eapol_hdr *ehdr;
	struct sta_context *sta = bss->sta_list;

	if((bss == NULL) || (sta == NULL))
		return -1;

	hdr = (struct wlan_hdr *)sta_alloc_data_frame(bss->bssid, ETHERTYPE_EAPOL, sta->addr, 
											sizeof(struct eapol_hdr), 0, (u8 **) (int)&ehdr);
	if(hdr == 0)
		return -1;

	ehdr->ver = EAPOL_VERSION;
	ehdr->type = EAPOL_START;
	ehdr->len = 0;

	ap_send_frame(bss, (u8 *)hdr, (u8 *)ehdr->data - (u8 *)hdr, 0, sta);

	return 0;
}
#endif
/* Entrance function */
