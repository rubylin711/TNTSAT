/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdint.h>
#include <pthread.h>
#include "sys_define.h"
#include "mtos_misc.h"
#include "mtos_printk.h"
#include "mtos_mem.h"
#include "mtos_task.h"
#include "cistack/stbcios.h"
#ifndef KEEP_V1_MMI_API
#include "cistack/ci_appmmi.h"
#endif
#include "cistack/techtype.h"
#include "cistack/stbci.h"
#include "cistack/stbhwnvm.h"
#include "cistack/cip_debug.h"

#ifndef CIPLUS_API_H_
#define CIPLUS_API_H_

#ifdef __cplusplus
extern "C" {
#endif


#define CI_TITLE_LEN 64
#define MAX_CI_ITEM_COUNT 16

typedef enum {
    CI_MENU_SHOW = 0,
    CI_MENU_CLOSE,
    CI_DATA_UNRECOGNIZE,
    CI_CARD_INSERT,
    CI_CARD_READY,
    CI_CARD_REMOVE,
    CI_CA_MODULE_UP,
    CI_CARD_UNKOW,

} ci_cb_e;

typedef int ( *ci_callback_t)(ci_cb_e cb_e, unsigned long para1, u32 para2);

typedef struct {
    u8 title[CI_TITLE_LEN];

} ci_item_t;

typedef struct {
    u8 item_count;
    u8 sub_title[CI_TITLE_LEN];
    ci_item_t p_items[0];
} ci_list_t;

#define EXT_EMPTY_STUBS                                           \
{                                                                 \
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,    \
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,    \
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,    \
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,    \
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,    \
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,    \
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,    \
  NULL                                                           \
}

typedef struct {
    U8BIT ( *STB_CIGetCSUV)();
    void ( *STB_CINotifyPowerDownOk)(U32BIT module);
    void ( *STB_CINotifyHDSRequest)(U32BIT module, BOOLEAN display_diagnostic_screen);
    void ( *STB_CINotifyAppInfo)(U32BIT module, U8BIT app_type, U16BIT app_manf, U16BIT manf_code, U8BIT *menu_string);
    void ( *STB_CINotifyCaSystems)(U32BIT module, U16BIT *ca_ids, U8BIT num_ca_ids);
    void ( *STB_CINotifyPmtReply)(U32BIT module, U16BIT program_number, U8BIT version_number, U8BIT current_next_indicator, E_STB_CI_DESC ca_enable);
    void ( *STB_CIGetDateTime)(U16BIT *mjd, U8BIT *hour, U8BIT *minute, U8BIT *second, S16BIT *offset);
    void ( *STB_CINotifyAuthRequest)(U32BIT module, U16BIT auth_protocol_id, U8BIT *auth_req, U32BIT auth_req_len);
    void ( *STB_CINotifyScreenEvent)(U32BIT module, U8BIT event);
    void ( *STB_CIGetHostCountryCode)(U32BIT module, U8BIT *code);
    void ( *STB_CIGetHostLanguageCode)(U32BIT module, U8BIT *code);
    U8BIT ( *STB_CIRequestStart)(U32BIT module, U8BIT *app_domain, U8BIT domain_length, U8BIT *initial_object, U8BIT object_length);
    void ( *STB_CIFileAcknowledge)(U32BIT module, BOOLEAN file_ok, U8BIT *data, U32BIT len);
    void ( *STB_CINotifyAppAbortRequest)(U32BIT module, U8BIT *code, U32BIT length);
    void ( *STB_CINotifyModuleInsert)(U8BIT slot_id);
    void ( *STB_CINotifyModuleReady)(U8BIT slot_id, BOOLEAN ci_plus, U32BIT mask);
    void ( *STB_CINotifyModuleRemove)(U8BIT slot_id);
    void ( *STB_CINotifyHostControlSession)(U32BIT module);
    void ( *STB_CINotifyHostControlSessionClosed)(U32BIT module);
    void ( *STB_CITune)(U32BIT module, U16BIT nid, U16BIT onid, U16BIT tsid, U16BIT sid);
    void ( *STB_CITuneBroadcastRequest)(U32BIT module, U16BIT service_id, U16BIT desc_loop_len, U8BIT *desc_loop, U8BIT *pmt,
                                        E_STB_CI_TUNE_QUIETLY_FLAG tune_quietly, E_STB_CI_KEEP_APP_RUNNING_FLAG keep_app_running);

    void ( *STB_CIReplace)(U32BIT module, U8BIT ref, U16BIT replaced_pid, U16BIT replacement_pid);
    void ( *STB_CIClearReplace)(U32BIT module, U8BIT ref);
    void ( *STB_CIAskReleaseReply)(U32BIT module, U8BIT release_reply);
    void ( *STB_CITuneLCNRequest)(U32BIT module, U16BIT lcn, E_STB_CI_TUNE_QUIETLY_FLAG tune_quietly, E_STB_CI_KEEP_APP_RUNNING_FLAG keep_app_running);
    void ( *STB_CITuneTripletRequest)(U32BIT module, U16BIT original_network_id, U16BIT transport_stream_id, U16BIT service_id, U8BIT delivery_system_descriptor_tag,
                                      U8BIT descriptor_tag_extension, E_STB_CI_TUNE_QUIETLY_FLAG tune_quietly, E_STB_CI_KEEP_APP_RUNNING_FLAG keep_app_running);

    void ( *STB_CITunerStatusRequest)(U32BIT module);
    void ( *STB_CITuneIPRequest)(U32BIT module, E_STB_CI_TUNE_QUIETLY_FLAG tune_quietly, E_STB_CI_KEEP_APP_RUNNING_FLAG keep_app_running,
                                 U16BIT service_location_length, U8BIT *service_location_data);

    void ( *STB_CIGetHostKey)(E_STB_CI_KEY_TYPE type, U8BIT **key, U16BIT *length);
    void ( *STB_CINotifyProgress)(U8BIT slot_id, E_STB_CI_CC_STATE state, U16BIT cicam_brand_id);
    void ( *STB_CINotifyBrandCertValidation)(S_STB_CI_VALIDATION *cert_info);
    void ( *STB_CINotifyDeviceCertValidation)(S_STB_CI_VALIDATION *cert_info);
    void ( *STB_CINotifyCCKey)(U8BIT slot_id, U8BIT cipher, U8BIT key_register, U8BIT *key, U8BIT *civ);
    void ( *STB_CINotifyURI)(U8BIT slot_id, U16BIT program_number, S_STB_CI_URI *uri);
    U8BIT ( *STB_CINotifySRM)(U8BIT slot_id, E_STB_SRM_DATA_TYPE srm_type, U8BIT *data, U16BIT len);
    void ( *STB_CINotifyPinCaps)(U8BIT slot_id, E_STB_CI_PIN_CAPS capability, S_STB_CI_DATE *pin_date, S_STB_CI_TIME *pin_time, U8BIT rating);
    void ( *STB_CINotifyPinReply)(U8BIT slot_id, E_STB_CI_PIN_STATUS status);
    void ( *STB_CINotifyPinEvent)(U8BIT slot_id, U16BIT program_number, E_STB_CI_PIN_STATUS status, U8BIT rating, S_STB_CI_DATE *event_date,
                                  S_STB_CI_TIME *event_time, U8BIT private_data[15]);

    void ( *STB_CINotifyRecordStartStatus)(U8BIT slot_id, U8BIT status);
    void ( *STB_CINotifyRecordingLicense)(U8BIT slot_id, U16BIT program_number, U8BIT license_status, S_STB_CI_URI *uri, U8BIT *license, U16BIT license_len);
    void ( *STB_CINotifyLicenseStatus)(U8BIT slot_id, U8BIT license_status, U8BIT play_count);
    void ( *STB_CINotifyModeChangeStatus)(U8BIT slot_id, U8BIT status);
    void ( *STB_CINotifyRecordStopStatus)(U8BIT slot_id, U8BIT status);
    void ( *STB_CINotifyPlaybackLicense)(U8BIT slot_id, U16BIT program_number, U8BIT license_status, S_STB_CI_URI *uri, U8BIT *license, U16BIT license_len);

    void ( *STB_CINotifyOperatorStatus)(U32BIT module, S_STB_CI_OPERATOR_STATUS *status);
    void ( *STB_CINotifyOperatorNit)(U32BIT module, U16BIT nit_loop_length, U8BIT *nit_sections);
    void ( *STB_CINotifyOperatorInfo)(U32BIT module, U8BIT info_version, S_STB_CI_OPERATOR_INFO *info);
    void ( *STB_CINotifyOperatorSearchStatus)(U32BIT module, S_STB_CI_OPERATOR_STATUS *status);
    void ( *STB_CINotifyOperatorTune)(U32BIT module, U16BIT desc_loop_len, U8BIT *desc_loop);
    void ( *STB_CINotifyOperatorOSDTReply)(U32BIT module, U16BIT osdt_length, U8BIT *osdt);

    BOOLEAN ( *STB_CIReadSecureNVM)(U8BIT *dest_addr, U32BIT bytes);
    BOOLEAN ( *STB_CIWriteSecureNVM)(U8BIT *src_addr, U32BIT bytes);

    U8BIT ( *STB_CINotifyFirmwareUpgrade)(U8BIT slot_id, U8BIT type, U16BIT download_time);
    void ( *STB_CINotifyFirmwareUpgradeProgress)(U8BIT slot_id, U8BIT status);
    void ( *STB_CINotifyFirmwareUpgradeFailure)(U8BIT slot_id);
    void ( *STB_CINotifyFirmwareUpgradeComplete)(U8BIT slot_id);

    U16BIT ( *STB_CIGetMaxIPConnections)(void);
    void ( *STB_CIOpenIPConnection)(U32BIT module, S_STB_CI_IP_ADDR *addr, U16BIT port, E_STB_CI_IP_PROTOCOL protocol, U8BIT timeout);
    BOOLEAN ( *STB_CIOpenMulticastConnection)(U32BIT module, S_STB_CI_MULTICAST_DESCRIPTOR *descriptor, U8BIT timeout);
    void ( *STB_CISendIPData)(U32BIT module, U8BIT *buffer, U16BIT len);
    BOOLEAN ( *STB_CIHandleIPData)(U32BIT module, U8BIT *buffer, U32BIT len);
    void ( *STB_CIReleaseIPData)(U32BIT module, U8BIT *buffer);
    BOOLEAN ( *STB_CINotifyIPStatus)(U32BIT module, E_STB_CI_IP_STATUS status);
    void ( *STB_CICloseIPConnection)(U32BIT module);
    void ( *STB_CIGetCommsIPConfig)(S_STB_CI_COMMS_IP_CONFIG *comms_ip_config);
    BOOLEAN ( *STB_CIGetCommsIPConfigDNSServersAtIndex)(U8BIT index, COMMS_IP_CONFIG_IP_ARRAY_T *DNS_server_to_populate);

    void ( *STB_CINotifySasSession)(U32BIT module);
    void ( *STB_CINotifySasSessionClosed)(U32BIT module);
    void ( *STB_CIConfirmSasConnection)(U32BIT module, U8BIT *app_id, U8BIT status);
    void ( *STB_CINotifyOipfReplyMsg)(U32BIT module, U16BIT ca_system_id, U32BIT transaction_id, U8BIT status, U8BIT *ca_info, U16BIT ca_info_len);
#ifdef CI_SAS_RES
    void ( *STB_CINotifyOipfParentalControlInfo)(U32BIT module, U16BIT ca_system_id, S_STB_CI_OIPF_PARENTAL_CONTROL_INFO *info);
#endif
    void ( *STB_CINotifyOipfRightsInfo)(U32BIT module, U16BIT ca_system_id, U8BIT access_status, U8BIT *rights_issuer_url, U16BIT url_len);
    void ( *STB_CINotifyOipfSystemInfo)(U32BIT module, U16BIT ca_system_id, U8BIT *ca_info, U16BIT ca_info_len);
    void ( *STB_CINotifyOipfCanPlayStatus)(U32BIT module, U16BIT ca_system_id, BOOLEAN can_play_status);
    void ( *STB_CINotifyOipfCanRecordStatus)(U32BIT module, U16BIT ca_system_id, BOOLEAN can_record_status);
} ci_policy_t;



void stb_ci_enter_menu();
void stb_ci_close_screen();
void stb_ci_response_menu(u8 menu_index);

void stb_ci_update_pmt(void *pmt_data);

void stb_ci_route_ts(u16 prog_type, u32 pass);

void stb_ci_reset_ca_module(void);

//void ci_api_init(u8 task_prio);
void ci_api_set_callback(ci_callback_t ci_cb);
void ci_api_powner_off(void);
void ci_api_powner_on(void);
void ci_api_debug_info(const char *p_function, int line);
S32 ci_api_get_ciplus_active_status(void);
s32 ci_start_play(void);
s32 ci_stop_play(void);
s32 ci_module_init(void);
u32 ci_module_get_status(void);
s32 ci_module_uninit(void);
void TEST_Setup_CiPolicy(ci_policy_t *ci_policy);

#endif
