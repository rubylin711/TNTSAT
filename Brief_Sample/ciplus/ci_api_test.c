

/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
//#if 1//def ENABLE_CIPLUS

//#ifndef HAVE_CISTACK
#include "mt_unf_common.h"
#include "mt_unf_ecs.h"
#include "mt_audio_codec.h"
#include "mt_unf_video.h"
#include "mt_unf_demux.h"
#include "mt_unf_avplay.h"
#include "mt_type.h"
#include "ci_api_test.h"
#include "usbcam_route_ts.h"

/*--------------------------------------------------------------------------*/
/* Symbol Definitions                                                       */
/*--------------------------------------------------------------------------*/
#define OS_PRINTF printf
#define CIPLUS_DEBUG_ENABLE 1

#if CIPLUS_DEBUG_ENABLE
#define CIPLUS_MODULE_DUBUG(format,...)   OS_PRINTF("\033[1;32m [%s] line %d, \033[0m \n"format, __FUNCTION__, __LINE__, ##__VA_ARGS__)                              \

#else
#define CIPLUS_MODULE_DUBUG(format,...)    do{}while(0)
#endif

ci_policy_t  g_ci_policy = EXT_EMPTY_STUBS;
static BOOL is_ciplus_act = FALSE;

#define MAX_TABLE_NUM (3)
#define SECTION_LENGTH (4 * (KBYTES))
#define MAX_CI_MODULE_NUM 10

static U8BIT module_total_num = 0;
static U8BIT module_count = 0;
static U32BIT moduleTale[32] = {0};
static char *p_pmt_buffer = NULL;
static U8BIT *p_pmt_backup = NULL;
static BOOL is_pmt_backup = FALSE;
static BOOL is_ciplus_inited = FALSE;
static BOOL is_ciplus_play = FALSE;
static BOOL is_ci_standby = FALSE;

static ci_callback_t g_ci_cb = NULL;
static ci_list_t *g_p_ci_list = NULL;

static U32BIT ai_module;
static U32BIT ui_module;
static U32BIT ca_module;
static U8BIT g_ui_type;
static U32BIT g_eq_response_len;
static   u8 *p_ci_pool = NULL;


void TEST_Setup_CiPolicy(ci_policy_t *ci_policy)
{
   g_ci_policy = *ci_policy;
}


U8BIT STB_CIGetCSUV(void)
{
    CIPLUS_MODULE_DUBUG();
    U8BIT result = 0;
    if (g_ci_policy.STB_CIGetCSUV) {
        result = g_ci_policy.STB_CIGetCSUV();
    } else {
        CIPLUS_MODULE_DUBUG();
    }
    CIPLUS_MODULE_DUBUG();
    return result;
}

void STB_CINotifyPowerDownOk(U32BIT module)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyPowerDownOk) {
        g_ci_policy.STB_CINotifyPowerDownOk(module);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CINotifyHDSRequest(U32BIT module, BOOLEAN display_diagnostic_screen)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyHDSRequest) {
        g_ci_policy.STB_CINotifyHDSRequest(module, display_diagnostic_screen);
        return;
    }
    CIPLUS_MODULE_DUBUG();

}

void STB_CINotifyAppInfo(U32BIT module, U8BIT app_type, U16BIT app_manf, U16BIT manf_code, U8BIT *menu_string)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyAppInfo) {
        g_ci_policy.STB_CINotifyAppInfo(module, app_type, app_manf, manf_code, menu_string);
        return;
    }
    CIPLUS_MODULE_DUBUG();

}

void STB_CINotifyCaSystems(U32BIT module, U16BIT *ca_ids, U8BIT num_ca_ids)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyCaSystems) {
        g_ci_policy.STB_CINotifyCaSystems(module, ca_ids, num_ca_ids);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CINotifyPmtReply(U32BIT module, U16BIT program_number, U8BIT version_number, U8BIT current_next_indicator, E_STB_CI_DESC ca_enable)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyPmtReply) {
        g_ci_policy.STB_CINotifyPmtReply(module, program_number, version_number, current_next_indicator, ca_enable);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CIGetDateTime(U16BIT *mjd, U8BIT *hour, U8BIT *minute, U8BIT *second, S16BIT *offset)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CIGetDateTime) {
        g_ci_policy.STB_CIGetDateTime(mjd, hour, minute, second, offset);
        return;
    }

    *mjd = 54896;  /* 6.3.2009 */
    *hour = 13;
    *minute = 40;
    *second = 32;
    *offset = 0;
    CIPLUS_MODULE_DUBUG();
}

void STB_CINotifyAuthRequest(U32BIT module, U16BIT auth_protocol_id,
                             U8BIT *auth_req, U32BIT auth_req_len)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyAuthRequest) {
        g_ci_policy.STB_CINotifyAuthRequest(module, auth_protocol_id,
                                            auth_req, auth_req_len);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CINotifyScreenEvent(U32BIT module, U8BIT event)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyScreenEvent) {
        g_ci_policy.STB_CINotifyScreenEvent(module, event);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CIGetHostCountryCode(U32BIT module, U8BIT *code)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CIGetHostCountryCode) {
        g_ci_policy.STB_CIGetHostCountryCode(module, code);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CIGetHostLanguageCode(U32BIT module, U8BIT *code)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CIGetHostLanguageCode) {
        g_ci_policy.STB_CIGetHostLanguageCode(module, code);
        return;
    }
    CIPLUS_MODULE_DUBUG();

}

U8BIT STB_CIRequestStart(U32BIT module, U8BIT *app_domain, U8BIT domain_length, U8BIT *initial_object, U8BIT object_length)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CIRequestStart) {
        return g_ci_policy.STB_CIRequestStart(module, app_domain, domain_length, initial_object, object_length);
    }
    CIPLUS_MODULE_DUBUG();
    return STB_CI_START_WRONG_API;
}

void STB_CIFileAcknowledge(U32BIT module, BOOLEAN file_ok, U8BIT *data, U32BIT len)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CIFileAcknowledge) {
        g_ci_policy.STB_CIFileAcknowledge(module, file_ok, data, len);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CINotifyAppAbortRequest(U32BIT module, U8BIT *code, U32BIT length)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyAppAbortRequest) {
        g_ci_policy.STB_CINotifyAppAbortRequest(module, code, length);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CINotifyModuleInsert(U8BIT slot_id)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyModuleInsert) {
        g_ci_policy.STB_CINotifyModuleInsert(slot_id);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CINotifyModuleReady(U8BIT slot_id, BOOLEAN ci_plus, U32BIT mask)
{
    CIPLUS_MODULE_DUBUG();

    if (g_ci_policy.STB_CINotifyModuleReady) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyModuleReady(slot_id, ci_plus, mask);
        return;
    }
}

void STB_CINotifyModuleRemove(U8BIT slot_id)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyModuleRemove) {
        g_ci_policy.STB_CINotifyModuleRemove(slot_id);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CINotifyHostControlSession(U32BIT module)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyHostControlSession) {
        g_ci_policy.STB_CINotifyHostControlSession(module);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CINotifyHostControlSessionClosed(U32BIT module)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyHostControlSessionClosed) {
        g_ci_policy.STB_CINotifyHostControlSessionClosed(module);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CITune(U32BIT module, U16BIT nid, U16BIT onid, U16BIT tsid, U16BIT sid)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CITune) {
        g_ci_policy.STB_CITune(module, nid, onid, tsid, sid);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CITuneBroadcastRequest(U32BIT module, U16BIT service_id, U16BIT desc_loop_len, U8BIT *desc_loop, U8BIT *pmt,
                                E_STB_CI_TUNE_QUIETLY_FLAG tune_quietly, E_STB_CI_KEEP_APP_RUNNING_FLAG keep_app_running)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CITuneBroadcastRequest) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CITuneBroadcastRequest(module,
                                               service_id,
                                               desc_loop_len,
                                               desc_loop, pmt,
                                               tune_quietly,
                                               keep_app_running);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CITuneLCNRequest(U32BIT module, U16BIT lcn, E_STB_CI_TUNE_QUIETLY_FLAG tune_quietly,
                          E_STB_CI_KEEP_APP_RUNNING_FLAG keep_app_running)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CITuneLCNRequest) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CITuneLCNRequest(module,
                                         lcn,
                                         tune_quietly,
                                         keep_app_running);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CITuneTripletRequest(U32BIT module, U16BIT original_network_id, U16BIT transport_stream_id, U16BIT service_id,
                              U8BIT delivery_system_descriptor_tag, U8BIT descriptor_tag_extension, E_STB_CI_TUNE_QUIETLY_FLAG tune_quietly, E_STB_CI_KEEP_APP_RUNNING_FLAG keep_app_running)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CITuneTripletRequest) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CITuneTripletRequest(  module,
                                               original_network_id,
                                               transport_stream_id,
                                               service_id,
                                               delivery_system_descriptor_tag,
                                               descriptor_tag_extension,
                                               tune_quietly,
                                               keep_app_running);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CIReplace(U32BIT module, U8BIT ref, U16BIT replaced_pid, U16BIT replacement_pid)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CIReplace) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CIReplace(module, ref, replaced_pid, replacement_pid);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CIClearReplace(U32BIT module, U8BIT ref)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CIClearReplace) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CIClearReplace(module, ref);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CIAskReleaseReply(U32BIT module, U8BIT release_reply)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CIAskReleaseReply) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CIAskReleaseReply(module, release_reply);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CITunerStatusRequest(U32BIT module)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CITunerStatusRequest) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CITunerStatusRequest(module);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CITuneIPRequest(U32BIT module, E_STB_CI_TUNE_QUIETLY_FLAG tune_quietly, E_STB_CI_KEEP_APP_RUNNING_FLAG keep_app_running,
                         U16BIT service_location_length, U8BIT *service_location_data)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CITuneIPRequest) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CITuneIPRequest(module,
                                        tune_quietly,
                                        keep_app_running,
                                        service_location_length,
                                        service_location_data);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CINotifyProgress(U8BIT slot_id, E_STB_CI_CC_STATE state, U16BIT cicam_brand_id)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyProgress) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyProgress(slot_id, state, cicam_brand_id);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CINotifyBrandCertValidation(S_STB_CI_VALIDATION *cert_info)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyBrandCertValidation) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyBrandCertValidation(cert_info);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CINotifyDeviceCertValidation(S_STB_CI_VALIDATION *cert_info)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyDeviceCertValidation) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyDeviceCertValidation(cert_info);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CIGetHostKey(E_STB_CI_KEY_TYPE type, U8BIT **key, U16BIT *length)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CIGetHostKey) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CIGetHostKey(type, key, length);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CINotifyCCKey(U8BIT slot_id, U8BIT cipher, U8BIT key_register, U8BIT *key, U8BIT *civ)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyCCKey) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyCCKey(slot_id, cipher, key_register, key, civ);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CINotifyURI(U8BIT slot_id, U16BIT program_number, S_STB_CI_URI *uri)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyURI) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyURI(slot_id, program_number, uri);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

BOOLEAN STB_CIReadSecureNVM(U8BIT *dest_addr, U32BIT bytes)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CIReadSecureNVM) {
        CIPLUS_MODULE_DUBUG();
        return g_ci_policy.STB_CIReadSecureNVM(dest_addr, bytes);
    }
    CIPLUS_MODULE_DUBUG();
    return FALSE;
}

BOOLEAN STB_CIWriteSecureNVM(U8BIT *src_addr, U32BIT bytes)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CIWriteSecureNVM) {
        CIPLUS_MODULE_DUBUG();
        return g_ci_policy.STB_CIWriteSecureNVM(src_addr, bytes);
    }
    CIPLUS_MODULE_DUBUG();
    return FALSE;
}

U8BIT STB_CINotifyFirmwareUpgrade(U8BIT slot_id, U8BIT type, U16BIT download_time)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyFirmwareUpgrade) {
        CIPLUS_MODULE_DUBUG();
        return g_ci_policy.STB_CINotifyFirmwareUpgrade(slot_id, type, download_time);
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyFirmwareUpgrade\n");
    return 0x00;
}

void STB_CINotifyFirmwareUpgradeProgress(U8BIT slot_id, U8BIT status)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyFirmwareUpgradeProgress) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyFirmwareUpgradeProgress(slot_id, status);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyFirmwareUpgradeProgress\n");
}

void STB_CINotifyFirmwareUpgradeFailure(U8BIT slot_id)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyFirmwareUpgradeFailure) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyFirmwareUpgradeFailure(slot_id);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyFirmwareUpgradeFailure\n");
}

void STB_CINotifyFirmwareUpgradeComplete(U8BIT slot_id)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyFirmwareUpgradeComplete) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyFirmwareUpgradeComplete(slot_id);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyFirmwareUpgradeComplete\n");
}

U8BIT STB_CINotifySRM(U8BIT slot_id, E_STB_SRM_DATA_TYPE srm_type, U8BIT *data, U16BIT len)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifySRM) {
        CIPLUS_MODULE_DUBUG();
        return g_ci_policy.STB_CINotifySRM(slot_id, srm_type, data, len);
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifySRM\n");
    return 0x00;
}

void STB_CINotifyPinCaps(U8BIT slot_id, E_STB_CI_PIN_CAPS capability, S_STB_CI_DATE *pin_date,
                         S_STB_CI_TIME *pin_time, U8BIT rating)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyPinCaps) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyPinCaps(slot_id, capability, pin_date, pin_time, rating);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyPinCaps\n");
}

void STB_CINotifyPinReply(U8BIT slot_id, E_STB_CI_PIN_STATUS status)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyPinReply) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyPinReply(slot_id, status);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyPinReply\n");
}

void STB_CINotifyPinEvent(U8BIT slot_id, U16BIT program_number, E_STB_CI_PIN_STATUS status, U8BIT rating,
                          S_STB_CI_DATE *event_date, S_STB_CI_TIME *event_time, U8BIT private_data[15])
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyPinEvent) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyPinEvent(slot_id, program_number, status, rating,
                                         event_date, event_time, private_data);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyPinEvent\n");
}

void STB_CINotifyRecordStartStatus(U8BIT slot_id, U8BIT status)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyRecordStartStatus) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyRecordStartStatus(slot_id, status);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyRecordStartStatus\n");
}

void STB_CINotifyRecordingLicense(U8BIT slot_id, U16BIT program_number, U8BIT license_status, S_STB_CI_URI *uri,
                                  U8BIT *license, U16BIT license_len)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyRecordingLicense) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyRecordingLicense(slot_id, program_number,
                   license_status, uri, license,
                   license_len);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyRecordingLicense\n");
}

void STB_CINotifyLicenseStatus(U8BIT slot_id, U8BIT license_status, U8BIT play_count)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyLicenseStatus) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyLicenseStatus(slot_id, license_status, play_count);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyLicenseStatus\n");
}

void STB_CINotifyModeChangeStatus(U8BIT slot_id, U8BIT status)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyModeChangeStatus) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyModeChangeStatus(slot_id, status);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyModeChangeStatus\n");
}

void STB_CINotifyRecordStopStatus(U8BIT slot_id, U8BIT status)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyRecordStopStatus) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyRecordStopStatus(slot_id, status);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyRecordStopStatus\n");
}

void STB_CINotifyPlaybackLicense(U8BIT slot_id, U16BIT program_number, U8BIT license_status, S_STB_CI_URI *uri,
                                 U8BIT *license, U16BIT license_len)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyPlaybackLicense) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyPlaybackLicense(slot_id, program_number, license_status, uri, license,
                                                license_len);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyPlaybackLicense\n");
}

void STB_CINotifyOperatorStatus(U32BIT module, S_STB_CI_OPERATOR_STATUS *status)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyOperatorStatus) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyOperatorStatus(module, status);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyOperatorStatus\n");
}

void STB_CINotifyOperatorNit(U32BIT module, U16BIT nit_loop_length, U8BIT *nit_sections)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyOperatorNit) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyOperatorNit(module, nit_loop_length,
                                            nit_sections);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyOperatorNit\n");
}

void STB_CINotifyOperatorOSDTReply(U32BIT module, U32BIT osdt_length, U8BIT *osdt)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyOperatorOSDTReply) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyOperatorOSDTReply(module, osdt_length, osdt);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("HandleOperatorOSDTReply\n");
}

void STB_CINotifyOperatorInfo(U32BIT module, U8BIT info_version, S_STB_CI_OPERATOR_INFO *info)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyOperatorInfo) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyOperatorInfo(module, info_version, info);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyOperatorInfo\n");
}

void STB_CINotifyOperatorSearchStatus(U32BIT module, S_STB_CI_OPERATOR_STATUS *status)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyOperatorSearchStatus) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyOperatorSearchStatus(module, status);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyOperatorSearchStatus\n");
}

BOOLEAN STB_CIOpenMulticastConnection(U32BIT module, S_STB_CI_MULTICAST_DESCRIPTOR *descriptor, U8BIT timeout)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CIOpenMulticastConnection) {
        CIPLUS_MODULE_DUBUG();
        return g_ci_policy.STB_CIOpenMulticastConnection(module, descriptor, timeout);
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CIOpenMulticastConnection() returned FALSE\n");
    return FALSE;
}

void STB_CINotifyOperatorTune(U32BIT module, U16BIT desc_loop_len, U8BIT *desc_loop)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyOperatorTune) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyOperatorTune(module, desc_loop_len, desc_loop);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyOperatorTune\n");
}

U16BIT STB_CIGetMaxIPConnections(void)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CIGetMaxIPConnections) {
        CIPLUS_MODULE_DUBUG();
        return g_ci_policy.STB_CIGetMaxIPConnections();
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CIGetMaxIPConnections\n");
    return 10;
}

void STB_CIOpenIPConnection(U32BIT module, S_STB_CI_IP_ADDR *addr, U16BIT port, E_STB_CI_IP_PROTOCOL protocol, U8BIT timeout)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CIOpenIPConnection) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CIOpenIPConnection(module, addr, port, protocol, timeout);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CIOpenIPConnection\n");
}

void STB_CISendIPData(U32BIT module, U8BIT *buffer, U16BIT len)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CISendIPData) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CISendIPData(module, buffer, len);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CISendIPData\n");
}

void STB_CIReleaseIPData(U32BIT module, U8BIT *buffer)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CIReleaseIPData) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CIReleaseIPData(module, buffer);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CIReleaseIPData\n");
}

void STB_CICloseIPConnection(U32BIT module)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CICloseIPConnection) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CICloseIPConnection(module);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CICloseIPConnection\n");
}

void STB_CIGetCommsIPConfig(S_STB_CI_COMMS_IP_CONFIG *comms_ip_config)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CIGetCommsIPConfig) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CIGetCommsIPConfig(comms_ip_config);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CIGetCommsIPConfig\n");
}

BOOLEAN STB_CIGetCommsIPConfigDNSServersAtIndex(U8BIT index, COMMS_IP_CONFIG_IP_ARRAY_T *DNS_server_to_populate)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CIGetCommsIPConfig) {
        CIPLUS_MODULE_DUBUG();
        return g_ci_policy.STB_CIGetCommsIPConfigDNSServersAtIndex(index, DNS_server_to_populate);
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CIGetCommsIPConfigDNSServersAtIndex\n");
    return FALSE;
}

void STB_CINotifySasSession(U32BIT module)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifySasSession) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifySasSession(module);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifySasSession\n");
}

void STB_CINotifySasSessionClosed(U32BIT module)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifySasSessionClosed) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifySasSessionClosed(module);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifySasSessionClosed\n");
}

void STB_CIConfirmSasConnection(U32BIT module, U8BIT *app_id, U8BIT status)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CIConfirmSasConnection) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CIConfirmSasConnection(module, app_id, status);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CIConfirmSasConnection\n");
}

void STB_CINotifyOipfReplyMsg(U32BIT module, U16BIT ca_system_id, U32BIT transaction_id, U8BIT status, U8BIT *ca_info, U16BIT ca_info_len)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyOipfReplyMsg) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyOipfReplyMsg(module, ca_system_id, transaction_id,
                                             status, ca_info, ca_info_len);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyOipfReplyMsg\n");
}
#ifdef CI_SAS_RES

void STB_CINotifyOipfParentalControlInfo(U32BIT module, U16BIT ca_system_id, S_STB_CI_OIPF_PARENTAL_CONTROL_INFO *info)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyOipfParentalControlInfo) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyOipfParentalControlInfo(module, ca_system_id, info);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyOipfParentalControlInfo\n");
}
#endif

void STB_CINotifyOipfRightsInfo(U32BIT module, U16BIT ca_system_id, U8BIT access_status, U8BIT *rights_issuer_url, U16BIT url_len)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyOipfRightsInfo) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyOipfRightsInfo(module, ca_system_id, access_status, rights_issuer_url, url_len);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyOipfRightsInfo\n");
}

void STB_CINotifyOipfSystemInfo(U32BIT module, U16BIT ca_system_id, U8BIT *ca_info, U16BIT ca_info_len)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyOipfSystemInfo) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyOipfSystemInfo(module, ca_system_id, ca_info, ca_info_len);
        return;
    }
    CIPLUS_MODULE_DUBUG();
    STB_CIDebugPrintf("STB_CINotifyOipfSystemInfo\n");
}

void STB_CINotifyOipfCanPlayStatus(U32BIT module, U16BIT ca_system_id, BOOLEAN can_play_status)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyOipfCanPlayStatus) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyOipfCanPlayStatus(module, ca_system_id, can_play_status);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}

void STB_CINotifyOipfCanRecordStatus(U32BIT module, U16BIT ca_system_id, BOOLEAN can_record_status)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ci_policy.STB_CINotifyOipfCanRecordStatus) {
        CIPLUS_MODULE_DUBUG();
        g_ci_policy.STB_CINotifyOipfCanRecordStatus(module, ca_system_id, can_record_status);
        return;
    }
    CIPLUS_MODULE_DUBUG();
}


void stb_ci_enter_menu(void)
{
    OS_PRINTF("cgf debug ci api[%s].%d:ai_module:0x%x\n", __FUNCTION__, __LINE__, ai_module);

    if (is_ciplus_act) {
        STB_EnterCIMenu(ai_module);
    }
    OS_PRINTF("\033[1;32m [%s] [%d] \033[0m\n", __FUNCTION__, __LINE__);

}

void stb_ci_close_screen(void)
{
    OS_PRINTF("cgf debug ci api[%s].%d:ui_module:%d\n", __FUNCTION__, __LINE__, ui_module);

    if (is_ciplus_act) {
        STB_CloseCIScreen(ui_module);
    } else if (g_ci_cb != NULL) { //after CI plugin out, enter CI menu, but can't exit
        g_ci_cb(CI_MENU_CLOSE, 0, 0);
    }

}

void stb_ci_response_menu(u8 menu_index)
{
    OS_PRINTF("cgf debug ci api[%s].%d:ui_module:%d;menu_index:%d\n", __FUNCTION__, __LINE__, ui_module, menu_index);

    if (is_ciplus_act) {
        STB_SetCIMenuScreenResponse(ui_module, (U8BIT)menu_index);
    } else if (g_ci_cb != NULL) {
        g_ci_cb(CI_MENU_CLOSE, 0, 0);
    }

}

void stb_ci_reset_ca_module(void)
{
    memset(moduleTale, 0, sizeof(moduleTale));
    module_count = 0;
    is_pmt_backup = FALSE;
}

static void stb_ci_notify_app_info(U32BIT module, U8BIT app_type, U16BIT app_manf, U16BIT manf_code, U8BIT *menu_string)
{
    U8BIT slot_id;

    ai_module = module;

    CIPLUS_MODULE_DUBUG();
    OS_PRINTF("Application information:\n");
    OS_PRINTF("  application_type         = 0x%02x\n", app_type);
    OS_PRINTF("  application_manufacturer = 0x%04x\n", app_manf);
    OS_PRINTF("  manufacturer_code        = 0x%04x\n", manf_code);
    OS_PRINTF("  menu_string              = %s\n", menu_string);
    OS_PRINTF("  ai_module=0x%x module  = 0x%x\n", ai_module, module);

    slot_id = STB_GetCIApplicationSlotId(module);
    OS_PRINTF("  slot_id                  = %u\n", slot_id);


}


static void stb_ci_notify_screen_event(U32BIT module, U8BIT event)
{
    U8BIT type;
    U8BIT *text;
    U8BIT num_title = 0;
    int i = 0;


    OS_PRINTF("STB_CINotifyScreenEvent(0x%x, %d)\n",
              module, event);

    if (event == STB_CI_SCREEN_EVENT_REQUEST) {
        type = STB_GetCIScreenType(module);
        ui_module = module;
        g_ui_type = type;
        if (type == STB_CI_MENU_LIST_SCREEN) {
            OS_PRINTF("type = STB_CI_MENU_LIST_SCREEN\n");

            text = STB_GetCIMenuScreenTitle(module);
            if (text) {
                OS_PRINTF("\"%s\"\n", (char *)text);
            }

            text = STB_GetCIMenuScreenSubtitle(module);
            if (text) {
                OS_PRINTF("\"%s\"\n", (char *)text);
                memcpy((char *)(g_p_ci_list->sub_title), text, strlen((char *)text) + 1);
            }
            num_title = STB_GetCIMenuScreenNumItems(module);

            for (i = 0; i < num_title; i++) {
                text = STB_GetCIMenuScreenItemText(module, (U8BIT)i);
                if (text) {
                    OS_PRINTF(" [%d] \"%s\"\n", i + 1, (char *)text);
                    memcpy((char *)(g_p_ci_list->p_items[i].title), text, strlen(text) + 1);
                }
            }
            g_p_ci_list->item_count = num_title;

            text = STB_GetCIMenuScreenBottomText(module);
            if (text) {
                OS_PRINTF("\"%s\"\n", (char *)text);
            }
            if (g_ci_cb != NULL && (memcmp(g_p_ci_list->sub_title, "CA Message", 10) != 0)) {
                g_ci_cb(CI_MENU_SHOW, (unsigned long)g_p_ci_list, 0);
            }
        } else if (type == STB_CI_ENQUIRY_SCREEN) {
            OS_PRINTF("type = STB_CI_ENQUIRY_SCREEN\n");
            text = STB_GetCIEnquiryScreenText(module);
            OS_PRINTF("text = \"%s\"\n", (char *)text);

            OS_PRINTF("res hide = %s\n", STB_GetCIEnqScreenHideResponse(module) ? "TRUE" : "FALSE");

            g_eq_response_len = STB_GetCIEnqScreenResponseLength(module);
            OS_PRINTF("res len = %d\n", g_eq_response_len);

        } else if (type == STB_NO_CI_SCREEN) {
            OS_PRINTF("type = STB_NO_CI_SCREEN\n");
        }

    } else if (event == STB_CI_SCREEN_EVENT_CLOSE) {
        OS_PRINTF("event = STB_CI_SCREEN_EVENT_CLOSE\n");
        g_ui_type = 0;
        if (g_ci_cb != NULL) {
            g_ci_cb(CI_MENU_CLOSE, 0, 0);
        }
    } else {
        OS_PRINTF("Unexpected event\n");
    }

}


/*!**************************************************************************
 * @brief    Handle CA systems notification
 * @param    module - module ID
 * @param    ca_ids - array of CA IDs
 * @param    num_ca_ids - number of IDs in the array
 ****************************************************************************/
static void stb_ci_notify_ca_systems(U32BIT module, U16BIT *ca_ids, U8BIT num_ca_ids)
{
    CIPLUS_MODULE_DUBUG();

    U8BIT i = 0;
    ca_module = module;

    OS_PRINTF("STB_CINotifyCaSystems find ca_module:0x%x\n", module);
    for (i = 0; i < num_ca_ids; i++) {
        OS_PRINTF("ids[%d]:0x%04x\n", i, ca_ids[i]);
    }
    if (module != 0) {
        if (module_count < MAX_CI_MODULE_NUM) {
            moduleTale[module_count] = module;
            module_count ++;
        }
    }

    if ((is_pmt_backup == TRUE) && (p_pmt_backup != NULL)) {
        if (module != 0) {
            OS_PRINTF("cgf debug[%s].%d:update ca_module:0x%x pmt\n", __FUNCTION__, __LINE__, module);
            STB_CIUpdatePmt(module, (U8BIT *)p_pmt_backup, TRUE, STB_CI_PMT_CMD_OK_DESCRAMBLE);
        }
    } else if (g_ci_cb != NULL) {
        g_ci_cb(CI_CA_MODULE_UP, module, 0);
    }
    usbcam_route_ts_set_usbcam_state(TRUE);
}

static void _send_enquiry_response(int ok, char *res)
{
    CIPLUS_MODULE_DUBUG();
    if (g_ui_type != STB_CI_ENQUIRY_SCREEN) {
        return;
    }

    OS_PRINTF("ok=%d, res=%s\n", ok, res);

    if (ok == FALSE) {
        STB_SetCIEnquiryScreenResponse(ui_module, FALSE, NULL);
        return;
    }

    U32BIT res_len = strlen(res);

    if (res_len != g_eq_response_len) {
        OS_PRINTF("res len %d != %d\n", res_len, g_eq_response_len);
        return;
    }

    char *p = mtos_malloc(res_len + 1);
    if (!p) {
        return;
    }

    strncpy(p, res, res_len);
    p[res_len] = 0;

    STB_SetCIEnquiryScreenResponse(ui_module, TRUE, (U8BIT *)p);

}


void stb_ci_notify_pmt_reply(U32BIT module, U16BIT program_number, U8BIT version_number, U8BIT current_next_indicator,
                             E_STB_CI_DESC ca_enable)
{

    OS_PRINTF("cgf debug[%s].%d:module[%d].pg_num:%d;version:0x%x;ca_enable:%d\n",
                __FUNCTION__, __LINE__, module, program_number, version_number, ca_enable);
}

void stb_ci_notify_module_insert(U8BIT slot_id)
{
    OS_PRINTF("\033[1;31m [%s] [%d] slot_id =%d \033[0m\n", __FUNCTION__, __LINE__, slot_id);
    if (!is_ciplus_inited) {
        return;
    }
    OS_PRINTF("%s slot_id:%d\n", __func__, slot_id);
    if (g_ci_cb != NULL) {
        g_ci_cb(CI_CARD_INSERT, slot_id, 0);
    }
}
void stb_ci_notify_module_remove(U8BIT slot_id)
{

    OS_PRINTF("\033[1;31m [%s] [%d] slot_id =%d  \033[0m\n", __FUNCTION__, __LINE__, slot_id);
    is_ciplus_act = FALSE;

    if (is_ciplus_play)
        is_ciplus_play = FALSE ;

    memset(moduleTale, 0, sizeof(moduleTale));
    module_total_num = 0;
    module_count = 0;


    memset(p_pmt_backup, 0, SECTION_LENGTH);
    is_pmt_backup = FALSE;

    //usbcam_route_ts_uninit();

    if (g_ci_cb != NULL) {
        g_ci_cb(CI_CARD_REMOVE, slot_id, 0);
    }
    // g_slot_id = 0;
}

void stb_ci_notify_module_ready(U8BIT slot_id, BOOLEAN ci_plus, U32BIT mask)
{

    int ret = 0;
    record_info_t record_info = {0};
    OS_PRINTF("\033[1;32m [%s] [%d] slot_id =%d  ci_plus =%d mask =0x%x\033[0m\n",     \
                __FUNCTION__, __LINE__, slot_id, ci_plus, mask);
    CIPLUS_MODULE_DUBUG();
    if (!is_ciplus_inited) {
        return;
    }

    if (is_ciplus_act) {
        return;
    }
    CIPLUS_MODULE_DUBUG();

	/*
    ret = usbcam_route_ts_init();
    if(ret != SUCCESS){
        OS_PRINTF("%s usbcam_route_ts_init failed\n",__func__);
        return;
    }
    */

    
    is_ciplus_act = TRUE;

    module_total_num = STB_GetNumCIModules();
    if (module_total_num > MAX_CI_MODULE_NUM) {
        module_total_num = MAX_CI_MODULE_NUM;
    }
    memset(moduleTale, 0, sizeof(moduleTale));
    module_count = 0;
    is_pmt_backup = FALSE;

    if (g_ci_cb != NULL) {
        g_ci_cb(CI_CARD_READY, slot_id, ci_plus);
    }

    OS_PRINTF("\033[1;32m [%s] [%d]  ret =%ld\033[0m\n", __FUNCTION__, __LINE__, ret);

    return;
}


BOOL ci_api_get_ciplus_active_status(void)
{
    return is_ciplus_act;
}

void stb_ci_update_pmt(void *pmt_data)
{
    int section_length = 0;
    U8BIT i = 0;

    CIPLUS_MODULE_DUBUG();


    if (!is_ciplus_act) {
        return;
    }
    CIPLUS_MODULE_DUBUG();


    if (NULL == pmt_data|| NULL == p_pmt_backup) {
        return;
    }
    section_length = (((char *)pmt_data)[1] << 8 | ((char *)pmt_data)[2]) & 0xfff;
    section_length += 3;

    OS_PRINTF("section_length = %d,SECTION_LENGTH = %d\n",section_length,SECTION_LENGTH);
    if (section_length > SECTION_LENGTH) {
        section_length = SECTION_LENGTH;
        OS_PRINTF("cgf debug[%s].%d warning :pmt Data length is larger than %d bytes !\n",
                  __FUNCTION__, __LINE__, SECTION_LENGTH);
    }

    CIPLUS_MODULE_DUBUG();

    if (memcmp(p_pmt_backup, pmt_data, section_length) == 0) {
        CIPLUS_MODULE_DUBUG("pmt not change !!!");
        return 0;
    }

    if ((p_pmt_backup != NULL) && (pmt_data != NULL)) {
        memcpy(p_pmt_backup, pmt_data, section_length);
        is_pmt_backup = TRUE;
        OS_PRINTF("cgf debug[%s].%d:fond and backup pmt data \n", __FUNCTION__, __LINE__);
    }
    CIPLUS_MODULE_DUBUG();
    if ((is_pmt_backup == TRUE) && (p_pmt_backup != NULL)) {
        for (i = 0; i < module_count; i ++) {
            if (moduleTale[i] != 0) {
                OS_PRINTF("cgf debug[%s].%d:update ca_module:0x%x pmt\n", __FUNCTION__, __LINE__, moduleTale[i]);
                STB_CIUpdatePmt(moduleTale[i], (U8BIT *)p_pmt_backup, TRUE, STB_CI_PMT_CMD_OK_DESCRAMBLE);
            }
        }
    }
}


static void ci_policy_init(ci_policy_t *ci_policy)
{
    CIPLUS_MODULE_DUBUG();
    g_ci_policy = *ci_policy;
}

s32 ci_module_init(void)
{
    ci_policy_t policy = {0};
    if (!is_ciplus_inited) {
        p_pmt_buffer = mtos_malloc(MAX_TABLE_NUM *SECTION_LENGTH);
        if (p_pmt_buffer == NULL) {
            OS_PRINTF("cgf debug[%s].%d: ERR:malloc fail! \n", __FUNCTION__, __LINE__);
            return -1;
        }
        memset(p_pmt_buffer, 0, (MAX_TABLE_NUM *SECTION_LENGTH));

        p_pmt_backup = mtos_malloc(SECTION_LENGTH);
        if (p_pmt_backup == NULL) {
            OS_PRINTF("cgf debug[%s].%d: ERR:malloc fail! \n", __FUNCTION__, __LINE__);
            mtos_free(p_pmt_buffer);
            p_pmt_buffer = NULL;
            return -1;
        }
        memset(p_pmt_backup, 0, SECTION_LENGTH);

        //OS_PRINTF("\033[1;31m [%s] [%d] \033[0m\n",__FUNCTION__,__LINE__);
#if 1
        g_p_ci_list = (ci_list_t *)mtos_malloc(sizeof(ci_list_t) + MAX_CI_ITEM_COUNT *sizeof(ci_item_t));
        if (g_p_ci_list == NULL) {
            OS_PRINTF("cgf debug[%s].%d: ERR:malloc fail! \n", __FUNCTION__, __LINE__);
            mtos_free(p_pmt_buffer);
            mtos_free(p_pmt_backup);
            p_pmt_buffer = NULL;
            p_pmt_backup = NULL;
            return -1;
        }
        memset(g_p_ci_list, 0, (sizeof(ci_list_t) + MAX_CI_ITEM_COUNT *sizeof(ci_item_t)));
        //  is_ciplus_inited = TRUE;
        g_ci_cb = NULL;
        //  routed_tunerId = 2;
        memset(moduleTale, 0, sizeof(moduleTale));
        module_total_num = 0;
        module_count = 0;
        is_pmt_backup = FALSE;
#endif

        //  OS_PRINTF("\033[1;31m [%s] [%d] GetMask =0x%x\033[0m\n",__FUNCTION__,__LINE__,CIP_DebugGetMask());
        //CIP_DebugSetMask(0xff);
        memset( &policy, 0, sizeof(policy));
        policy.STB_CINotifyModuleReady = stb_ci_notify_module_ready;
        policy.STB_CINotifyScreenEvent = stb_ci_notify_screen_event;
        policy.STB_CINotifyAppInfo = stb_ci_notify_app_info;
        policy.STB_CINotifyCaSystems = stb_ci_notify_ca_systems;
        policy.STB_CINotifyModuleRemove = stb_ci_notify_module_remove;
        policy.STB_CINotifyModuleInsert = stb_ci_notify_module_insert;
        policy.STB_CINotifyPmtReply = stb_ci_notify_pmt_reply;
        ci_policy_init( &policy);

        /*start ci task run*/
        CI_Initialise(180);
        is_ciplus_inited = TRUE;
    }

    return 0;
}

s32 ci_module_uninit(void)
{
    //CI_UnInitialise();
    mtos_free(g_p_ci_list);
    mtos_free(p_pmt_buffer);
    mtos_free(p_pmt_backup);
    is_ciplus_inited = FALSE;
}

void ci_api_set_callback(ci_callback_t ci_cb)
{
    OS_PRINTF("ci_api_set_callback \n");
    g_ci_cb = ci_cb;
}

void ci_api_powner_off(void)
{
    if (is_ci_standby) {
        return;
    }

    g_ci_cb = NULL;
    is_ci_standby = TRUE;
}

void ci_api_powner_on(void)
{
    if (!is_ci_standby) {
        return;
    }

    is_ci_standby = FALSE;
}

u32 ci_module_get_status(void)
{
    return TRUE;
}

s32 ci_start_play()
{
    s32 ret = 0;
    CIPLUS_MODULE_DUBUG();
    if (is_ciplus_play == TRUE) {
        ret = usbcam_route_ts_stop();
        OS_PRINTF("\033[1;31m [%s] [%d] ret =%d \033[0m\n", __FUNCTION__, __LINE__, ret);
    }

    is_ciplus_play = TRUE ;
    usbcam_route_set_record_info();
    ret = usbcam_route_ts_start();
    if (ret < 0) {
        OS_PRINTF("%s call usbcam_route_ts_start failed\n", __func__);
        return ret;
    }

    return 0;
}

s32 ci_stop_play(void)
{
    if (is_ciplus_play != TRUE) {
        return 0;
    }
    is_ciplus_play = FALSE ;
    s32 ret = usbcam_route_ts_stop();
    OS_PRINTF("\033[1;31m [%s] [%d] ret =%d \033[0m\n", __FUNCTION__, __LINE__, ret);
}

#if 0
s32 ciplus_add_pid(void)
{
    s32 ret = 0;
    ret =  usbcam_route_ts_adddel_pid(0x231, TRUE);
    OS_PRINTF("\033[1;31m [%s] [%d] ret =%ld \033[0m\n", __FUNCTION__, __LINE__, ret);
    return ret;
}

s32 ciplus_del_pid(u16 pid)
{
    s32 ret = 0;
    ret =  usbcam_route_ts_adddel_pid(pid, FALSE);
    OS_PRINTF("\033[1;31m [%s] [%d] ret =%d \033[0m\n", __FUNCTION__, __LINE__, ret);
    return ret;
}
#endif
//#endif
