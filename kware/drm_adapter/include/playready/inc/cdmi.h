/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#ifndef __CDMI_H__
#define __CDMI_H__

// For the support of portable data types such as uint8_t.
#include <stdint.h>

namespace PRCDMi
{

// EME error code to which CDMi errors are mapped. Please
// refer to the EME spec for details of the errors
// https://dvcs.w3.org/hg/html-media/raw-file/tip/encrypted-media/encrypted-media.html
#define MEDIA_KEYERR_UNKNOWN        1
#define MEDIA_KEYERR_CLIENT         2
#define MEDIA_KEYERR_SERVICE        3
#define MEDIA_KEYERR_OUTPUT         4
#define MEDIA_KEYERR_HARDWARECHANGE 5
#define MEDIA_KEYERR_DOMAIN         6

// The status code returned by CDMi APIs.
typedef int32_t CDMi_RESULT;

#define CDMi_SUCCESS            ((CDMi_RESULT)0)
#define CDMi_S_FALSE            ((CDMi_RESULT)1)
#define CDMi_E_OUT_OF_MEMORY    ((CDMi_RESULT)0x80000002)
#define CDMi_E_FAIL             ((CDMi_RESULT)0x80004005)
#define CDMi_E_INVALID_ARG      ((CDMi_RESULT)0x80070057)

#define CDMi_E_SERVER_INTERNAL_ERROR    ((CDMi_RESULT)0x8004C600)
#define CDMi_E_SERVER_INVALID_MESSAGE   ((CDMi_RESULT)0x8004C601)
#define CDMi_E_SERVER_SERVICE_SPECIFIC  ((CDMi_RESULT)0x8004C604)

// More CDMi status codes can be defined. In general
// CDMi status codes should use the same PK error codes.

#define CDMi_FAILED(Status)     ((CDMi_RESULT)(Status)<0)
#define CDMi_SUCCEEDED(Status)  ((CDMi_RESULT)(Status) >= 0)

#define ChkCDMi(expr) DRM_DO {                              \
            cr = (expr);                                    \
            if( CDMi_FAILED( cr ) )                         \
            {                                               \
                goto ErrorExit;                             \
            }                                               \
        } DRM_WHILE_FALSE


// IMediaKeySessionCallback defines the callback interface to receive
// events originated from MediaKeySession.
class IMediaKeySessionCallback
{
public:
    virtual ~IMediaKeySessionCallback(void) {}

    // Event fired when a key message is successfully created.
    virtual void OnKeyMessage(
        __in_bcount(f_cbKeyMessage) const uint8_t *f_pbKeyMessage,
        __in uint32_t f_cbKeyMessage,
        __in_z_opt char *f_pszUrl) = 0;

    // Event fired when MediaKeySession has found a usable key.
    virtual void OnKeyReady(void) = 0;

    // Event fired when MediaKeySession encounters an error.
    virtual void OnKeyError(
        __in int16_t f_nError,
        __in CDMi_RESULT f_crSysError) = 0;
};

// IMediaKeySession defines the MediaKeySession interface.
class IMediaKeySession
{
public:
    virtual ~IMediaKeySession(void) {}

    // Kicks off the process of acquiring a key. A MediaKeySession callback is supplied
    // to receive notifications during the process.
    virtual void Run(
        __in const IMediaKeySessionCallback *f_piMediaKeySessionCallback) = 0;

    // Process a key message response.
    virtual void Update(
        __in_bcount(f_cbKeyMessageResponse) const uint8_t *f_pbKeyMessageResponse,
        __in uint32_t f_cbKeyMessageResponse) = 0;

    // Explicitly release all resources associated with the MediaKeySession.
    virtual void Close(void) = 0;

    // Return the session ID of the MediaKeySession. The returned pointer
    // is valid as long as the associated MediaKeySession still exists.
    virtual const char *GetSessionId(void) const = 0;

    // Return the key system of the MediaKeySession.
    virtual const char *GetKeySystem(void) const = 0;

    virtual void setKIDS(
        char *f_kid_0,
        char *f_kid_1) = 0;

    virtual int SetDrmDecryptIndex(
        int index) = 0;
};

// IMediaKeys defines the MediaKeys interface.
class IMediaKeys
{
public:
    virtual ~IMediaKeys(void) {}

    // Check whether the MediaKey supports a specific mime type (optional)
    // and a key system.
    virtual bool IsTypeSupported(
        __in_z_opt const char *f_pszMimeType,
        __in_z const char *f_pszKeySystem) const = 0;

    // Create a MediaKeySession using the supplied init data and CDM data.
    // If the returned media key session interface is not needed then it must
    // be released via the call of IMediaKeys::DestroyMediaKeySession.
    virtual CDMi_RESULT CreateMediaKeySession(
        __in_z_opt                      const char               *f_pszMimeType,
        __in_bcount_opt( f_cbInitData ) const uint8_t            *f_pbInitData,
        __in                                  uint32_t            f_cbInitData,
        __in_bcount_opt( f_cbCDMData )  const uint8_t            *f_pbCDMData,
        __in                                  uint32_t            f_cbCDMData,
        __deref_out_opt                       IMediaKeySession  **f_ppiMediaKeySession ) = 0;

    // Destroy a MediaKeySession instance.
    virtual CDMi_RESULT DestroyMediaKeySession(
        __in IMediaKeySession *f_piMediaKeySession) = 0;
};

// Global factory method that creates a MediaKeys instance.
// If the returned media keys interface is not needed then it must be released
// via the call of DestroyMediaKeys.
CDMi_RESULT CreateMediaKeys(
    __deref_out IMediaKeys **f_ppiMediaKeys);

// Global method that destroys a MediaKeys instance.
CDMi_RESULT DestroyMediaKeys(
    __in IMediaKeys *f_piMediaKeys);

// IMediaEngineSession represents a secure channel between the media engine and the CDM.
// In production it must have protection of the content between the decryption of
// content and decoding/output. The protection can be implemented by some form of
// sample protection or it can come for free from the architecture of the system
// (say, a closed system). or through using HWDRM and passing handles around which
// refer to protected memory usable for graphics system only.
class IMediaEngineSession
{
public:
    virtual ~IMediaEngineSession(void) {}

    // Decrypt a content buffer:
    // The output can be a handle in some protected memory space usable by graphics
    // system, pointer to decrypted content or sample protected (encrypted) content;
    // in case of decrypted content it is assumed that the memory space being
    // operated in is secure from external activities and thus conforms
    // to the requirement of content being protected. Clear content has to be released
    // through ReleaseClearContent API. It is up to the implementer of the CDM to make
    // sure it satisfies all compliance and robustness rules.
    virtual CDMi_RESULT Decrypt(
        __in                                                  uint32_t  f_cdwSubSampleMapping,
        __in_ecount_opt(f_cdwSubSampleMapping)          const uint32_t *f_pdwSubSampleMapping,
        __in                                                  uint32_t  f_cbIV,
        __in_bcount(f_cbIV)                             const uint8_t  *f_pbIV,
        __in                                                  uint32_t  f_cbData,
        __in_bcount(f_cbData)                           const uint8_t  *f_pbData,
        __out                                                 uint32_t *f_pcbOpaqueClearContent,
        __deref_out_bcount_opt( *f_pcbOpaqueClearContent )    uint8_t **f_ppbOpaqueClearContent,
        __in   uint32_t f_cEncryptedRegionSkip,
        __in_ecount_opt(f_cEncryptedRegionSkip)               const uint32_t  *f_pEncryptedRegionSkip) = 0;

    virtual CDMi_RESULT ReleaseClearContent(
        __in                                            const uint32_t  f_cbClearContentOpaque,
        __inout_bcount(f_cbClearContentOpaque)                uint8_t  *f_pbClearContentOpaque ) = 0;
};

// Global factory method that creates a MediaEngineSession instance.
// If the returned media engine session interface is not needed then it must be released
// via the call of DestroyMediaEngineSession.
// if more information is necessary for instantiation of the class,
// the implementation of this method can be augmented.
CDMi_RESULT CreateMediaEngineSession(
    __in            IMediaKeySession     *f_piMediaKeySession,
    __deref_out_opt IMediaEngineSession **f_ppiMediaEngineSession);

// Global method that destroys a MediaEngineSession instance.
CDMi_RESULT DestroyMediaEngineSession(
    __in IMediaEngineSession *f_piMediaEngineSession);

} // namespace PRCDMi

#endif  // __CDMI_H__
