
/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __PLAYREADY_ADAPTER_H__
#define __PLAYREADY_ADAPTER_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "mt_type.h"

typedef int32_t DRMPL_RESULT;
typedef void DRMPL_VOID;

#ifndef __in_z
#define __in_z
#endif

#ifndef __inout
#define __inout
#endif

#ifndef __in_z_opt
#define __in_z_opt
#endif

#define ADAPTER_CDMI_MAX_SUPPORTED_INIT_DATA_TYPES 2
#define ADAPTER_CDMI_MAX_SUPPORTED_SESSION_TYPES 3

#define BASE64_EQUIV(cb)   ((((cb)/3) + (((cb)%3)?1:0)) * 4)



typedef enum
{
    ADAPTER_CDMI_KEY_STATUS_UNDEFINED           = 0,    /*
                                                    ** An invalid argument, e.g. an unknown KID, was passed to DRM_CDMI_GetMediaKeyStatus.
                                                    ** Do not include this KID's information in the MediaKeyStatusMap.
                                                    */
    ADAPTER_CDMI_KEY_STATUS_PENDING             = 1,    /* Maps to "status-pending" */
    ADAPTER_CDMI_KEY_STATUS_USABLE              = 2,    /* Maps to "usable" */
    ADAPTER_CDMI_KEY_STATUS_EXPIRED             = 3,    /* Maps to "expired" */
    ADAPTER_CDMI_KEY_STATUS_RELEASED            = 4,    /* Maps to "released" */
    ADAPTER_CDMI_KEY_STATUS_INTERNAL_ERROR      = 5,    /* Maps to "internal-error" */
} ADAPTER_CDMI_KEY_STATUS;


typedef struct
{
    uint32_t    dwSessionTypesAllowedMask;
    uint32_t    cSupportedInitDataTypes;
    char    *rgszSupportedInitDataTypes[ ADAPTER_CDMI_MAX_SUPPORTED_INIT_DATA_TYPES ];
    uint32_t    cSupportedSessionTypes;
    char    *rgszSupportedSessionTypes[ ADAPTER_CDMI_MAX_SUPPORTED_SESSION_TYPES ];
} ADAPTER_CDMI_MEDIA_KEY_SYSTEM_ACCESS;


typedef struct
{
    uint32_t                        cbServerCertificate;
    uint8_t                        *pbServerCertificate;
} ADAPTER_CDMI_CERT_DATA;


typedef struct
{
    uint32_t           dwSessionTypesAllowedMask;
    ADAPTER_CDMI_CERT_DATA  oCertData;
} ADAPTER_CDMI_MEDIA_KEYS;

/* generic ID type, currently all the same size */
#define ADAPTER_ID_SIZE  16

typedef struct
{
    uint8_t rgb[ ADAPTER_ID_SIZE ];
} ADAPTER_ID;

#define ADAPTER_CDMI_CHARS_IN_B64_DRM_ID (BASE64_EQUIV(sizeof(ADAPTER_ID)))

typedef void( *ADP_DRM_CDMI_OnCloseCallback )(
                                        void    *f_pMediaKeySession,
                                    void                      *f_pvCallbackContext );
typedef void( *ADP_DRM_CDMI_OnKeyStatusChangeCallback )(
                                       void    *f_pMediaKeySession,
                                    void                      *f_pvCallbackContext );
typedef void( *ADP_DRM_CDMI_OnKeyMessageCallback )(
                                        void    *f_pMediaKeySession,
                                    void                      *f_pvCallbackContext,
                                 const char                      *f_pszMediaKeyMessageType,
                                          uint32_t                      f_cbMediaKeyMessage,
          uint8_t                      *f_pbMediaKeyMessage );

/* Note: used as a bitmask. */
typedef enum 
{
    ADAPTER_CDMI_SESSION_TYPE_TEMPORARY                 = 0x1,
    ADAPTER_CDMI_SESSION_TYPE_PERSISTENT_USAGE_RECORD   = 0x2,
    ADAPTER_CDMI_SESSION_TYPE_PERSISTENT_LICENSE        = 0x4,
} ADAPTER_CDMI_SESSION_TYPE;

typedef struct
{
    void                               *pvCallbackContext;
    ADP_DRM_CDMI_OnCloseCallback            pfnOnClose;
    ADP_DRM_CDMI_OnKeyStatusChangeCallback  pfnKeyStatusChange;
    ADP_DRM_CDMI_OnKeyMessageCallback       pfnOnKeyMessage;
} ADAPTER_CDMI_MEDIA_KEY_SESSION_CALLBACKS;

typedef enum
{
    ADAPTER_PLAY_OPL_CALLBACK                       = 0x1,  /* DRM_PLAY_OPL_EX2                                 */
    ADAPTER_RESERVED_CALLBACK2                          = 0x2,  /* Never called, enum value maintained for compat   */
    ADAPTER_RESERVED_CALLBACK3                          = 0x3,  /* Never called, enum value maintained for compat   */
    ADAPTER_EXTENDED_RESTRICTION_CONDITION_CALLBACK = 0x4,  /* DRM_EXTENDED_RESTRICTION_CALLBACK_STRUCT         */
    ADAPTER_EXTENDED_RESTRICTION_ACTION_CALLBACK    = 0x5,  /* DRM_EXTENDED_RESTRICTION_CALLBACK_STRUCT         */
    ADAPTER_EXTENDED_RESTRICTION_QUERY_CALLBACK     = 0x6,  /* DRM_EXTENDED_RESTRICTION_CALLBACK_STRUCT         */
    ADAPTER_SECURE_STATE_TOKEN_RESOLVE_CALLBACK     = 0x7,  /* DRM_SECURE_STATE_TOKEN_RESOLVE_DATA              */
    ADAPTER_RESTRICTED_SOURCEID_CALLBACK            = 0x8,  /* DRM_RESTRICTED_SOURCEID_CALLBACK_STRUCT          */
    ADAPTER_OEM_KEY_INFO_CALLBACK                   = 0x9,  /* DRM_OEM_KEY_INFO_CALLBACK_STRUCT                 */
} ADAPTER_POLICY_CALLBACK_TYPE;

typedef DRMPL_RESULT (* ADAPTERPFNPOLICYCALLBACK)(
    const void                 *f_pvCallbackData,
    ADAPTER_POLICY_CALLBACK_TYPE  f_dwCallbackType,
    const ADAPTER_ID                  *f_pKID,            /* KID that is being enumerated, i.e. the KID of the leaf-most license in a chain.  Will be NULL for callbacks not dealing with a license. */
    const ADAPTER_ID                  *f_pLID,            /* LID of the actual license being called upon, i.e. may be leaf or root license in a chain.  Will be NULL for callbacks not dealing with a license. */
    const void                 *f_pv );            /* Void pointer to opaque data passed in alongside the DRMPFNPOLICYCALLBACK parameter which is then passed to the callback, e.g. in Drm_Reader_Bind */


DRMPL_RESULT ADAPTER_CDMI_CreateMediaKeySystemAccess(
     const char                         *f_pszKeySystem,
     uint32_t                              *f_pcInitDataTypes,
     char                               **f_rgpszInitDataTypes,
     const char                         *f_pszDistinctiveIdentifierRequested,
     char                               **f_ppszDistinctiveIdentifierUsed,
     const char                         *f_pszPersistedStateRequested,
     char                               **f_ppszPersistedStateUsed,
     uint32_t                              f_cSessionTypes,
     const char                         **f_rgpszSessionTypes,
     ADAPTER_CDMI_MEDIA_KEY_SYSTEM_ACCESS       *f_pMediaKeySystemAccess );

DRMPL_VOID ADAPTER_CDMI_DestroyMediaKeySystemAccess(
     ADAPTER_CDMI_MEDIA_KEY_SYSTEM_ACCESS       *f_pMediaKeySystemAccess );


const char * ADAPTER_CDMI_GetKeySystem(const ADAPTER_CDMI_MEDIA_KEY_SYSTEM_ACCESS       *f_pMediaKeySystemAccess );

DRMPL_RESULT ADAPTER_CDMI_GetConfiguration(
    ADAPTER_CDMI_MEDIA_KEY_SYSTEM_ACCESS       *f_pMediaKeySystemAccess,
    uint32_t                              *f_pcInitDataTypes,
    char                             ***f_prgpszInitDataTypes,
    char                              **f_ppszDistinctiveIdentifier,
    char                              **f_ppszPersistedState,
    uint32_t                              *f_pcSessionTypes,
    char                             ***f_prgpszSessionTypes );

DRMPL_RESULT ADAPTER_CDMI_CreateMediaKeys(
    const ADAPTER_CDMI_MEDIA_KEY_SYSTEM_ACCESS       *f_pMediaKeySystemAccess,
    ADAPTER_CDMI_MEDIA_KEYS                    *f_pMediaKeys );

DRMPL_VOID ADAPTER_CDMI_DestroyMediaKeys(ADAPTER_CDMI_MEDIA_KEYS                    *f_pMediaKeys );


DRMPL_RESULT ADAPTER_CDMI_SetServerCertificate(
    ADAPTER_CDMI_MEDIA_KEYS                    *f_pMediaKeys,
    uint32_t                               f_cbServerCertificate,
    const uint8_t                               *f_pbServerCertificate );

DRMPL_RESULT ADAPTER_CDMI_CreateMediaKeySession(
    const ADAPTER_CDMI_MEDIA_KEYS                    *f_pMediaKeys,
    const char                               *f_pszSessionType,
    DRMPL_VOID                               *f_pOEMContext,
    char                               *f_pszDeviceStoreName,
    void             **f_pMediaKeySession );

void ADAPTER_CDMI_DestroyMediaKeySession(void    *f_pMediaKeySession );

char * ADAPTER_CDMI_GetSessionId(const void             *f_pMediaKeySession );


void ADAPTER_CDMI_SetSessionCallbacks(
    void             *f_pMediaKeySession,
    void                               *f_pvCallbackContext,
    ADP_DRM_CDMI_OnCloseCallback            f_pfnOnClose,
    ADP_DRM_CDMI_OnKeyStatusChangeCallback  f_pfnOnKeyStatusChange,
    ADP_DRM_CDMI_OnKeyMessageCallback       f_pfnOnKeyMessage );

DRMPL_RESULT  ADAPTER_CDMI_GetExpiration(
    void             *f_pMediaKeySession,
    uint64_t        *f_pui64Expiration );

DRMPL_RESULT ADAPTER_CDMI_GetMediaKeyCount(
    const void           *f_pMediaKeySession,
    void                              *f_pcMediaKeys );

DRMPL_RESULT ADAPTER_CDMI_GetMediaKeyStatus(
    void             *f_pMediaKeySession,
    uint32_t                              *f_piMediaKey,
    ADAPTER_ID                                 *f_pKeyId,
    MT_BOOL                               *f_pfHasKey,
    ADAPTER_CDMI_KEY_STATUS                    *f_peKeyStatus );


DRMPL_RESULT ADAPTER_CDMI_GenerateRequest(
    void             *f_pMediaKeySession,
    const char                               *f_pszInitDataType,
    uint32_t                               f_cbInitData,
    const uint8_t                               *f_pbInitData );

DRMPL_RESULT ADAPTER_CDMI_Load(
    void             *f_pMediaKeySession,
    const char                               *f_pszSessionId,
    ADAPTERPFNPOLICYCALLBACK                    f_pfnPolicyCallback,
    const void                               *f_pvPolicyCallbackContext );

DRMPL_RESULT ADAPTER_CDMI_Update(
    void             *f_pMediaKeySession,
    ADAPTERPFNPOLICYCALLBACK                    f_pfnPolicyCallback,
    const void                               *f_pvPolicyCallbackContext,
    uint32_t                               f_cbResponse,
    const uint8_t                               *f_pbResponse );

DRMPL_RESULT ADAPTER_CDMI_Close(
    void             *f_pMediaKeySession );

DRMPL_RESULT ADAPTER_CDMI_Remove(
    void             *f_pMediaKeySession );

DRMPL_RESULT ADAPTER_CDMI_DecryptOpaque(
    void             *f_pMediaKeySession,
    ADAPTER_ID                                 *f_pKeyId,
    uint32_t                               f_cEncryptedRegionMappings,
    const uint32_t                              *f_pdwEncryptedRegionMappings,
    uint64_t                              f_ui64InitializationVector,
    uint32_t                               f_cbEncryptedContent,
    const uint8_t                               *f_pbEncryptedContent,
    uint32_t                              *f_pcbOpaqueClearContent,
    uint8_t                              **f_ppbOpaqueClearContent );

void ADAPTER_CDMI_FreeOpaqueDecryptedContent(
    void             *f_pMediaKeySession,
    const ADAPTER_ID                                 *f_pKeyId,
    uint32_t                               f_cbOpaqueClearContent,
    uint8_t                               *f_pbOpaqueClearContent );

void*  ADAPTER_CDMI_GetSessionAppContext(
    void             *f_pMediaKeySession );

#endif
