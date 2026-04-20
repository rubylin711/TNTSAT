/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#ifndef __CDMI_IMP_H__
#define __CDMI_IMP_H__

#include "cdmi.h"

// Note: Remove the following block
// if building from Visual Studio is
// unsupported.
#ifdef VISUAL_STUDIO_BUILD
// DRM_BUILD_PROFILE_OEM
#define DRM_BUILD_PROFILE   10
#include <drmbuild_oem.h>
#endif

#include <drmtypes.h>
#include <drmmanager.h>

namespace PRCDMi
{

// Defines the state of a MediaKeySession.
enum KeyState
{
    // Has been initialized.
    KEY_INIT = 0,
    // Has a key message pending to be processed.
    KEY_PENDING = 1,
    // Has a usable key.
    KEY_READY = 2,
    // Has an error.
    KEY_ERROR = 3,
    // Has been closed.
    KEY_CLOSED = 4
};

// Class that implements the IMediaKeySession interface.
class CMediaKeySession : public IMediaKeySession
{
public:
    // Constructor.
    CMediaKeySession(void);

    // Destructor.
    virtual ~CMediaKeySession(void);

    // Trigger the license acquisition workflow.
    virtual void Run(
        __in const IMediaKeySessionCallback *f_piMediaKeySessionCallback ) ;

    // Process a license acquisition response.
    virtual void Update(
        __in_bcount(f_cbKeyMessageResponse) const uint8_t *f_pbKeyMessageResponse,
        __in uint32_t f_cbKeyMessageResponse ) ;

    // Shutdown the media key session.
    virtual void Close( void ) ;

    // Return a 16-bytes ID of the current media key session.
    virtual const char *GetSessionId( void ) const ;

    // Return the key system string.
    virtual const char *GetKeySystem( void ) const ;

    // Initialize the current media key session instance.
    CDMi_RESULT Init(
        __in_bcount_opt(f_cbInitData) const uint8_t *f_pbInitData,
        __in uint32_t f_cbInitData,
        __in_bcount_opt(f_cbCDMData) const uint8_t *f_pbCDMData,
        __in uint32_t f_cbCDMData);

    // Decrypt is not an IMediaKeySession interface method therefore it can only be
    // accessed from code that has internal knowledge of CMediaKeySession.
    // Decrypted content can end up in the opaque pointer or in-place decrypted
    // (It depends on how the underlying PK is compiled).
    CDMi_RESULT Decrypt(
        __in                                                  uint32_t  f_cdwSubSampleMapping,
        __in_ecount_opt(f_cdwSubSampleMapping)          const uint32_t *f_pdwSubSampleMapping,
        __in                                                  uint32_t  f_cbIV,
        __in_bcount(f_cbIV)                             const uint8_t  *f_pbIV,
        __in                                                  uint32_t  f_cbData,
        __in_bcount(f_cbData)                           const uint8_t  *f_pbData,
        __out                                                 uint32_t *f_pcbOpaqueClearContent,
        __deref_out_bcount_opt( *f_pcbOpaqueClearContent )    uint8_t **f_ppbOpaqueClearContent,
        __in   uint32_t f_cEncryptedRegionSkip,
        __in_ecount_opt(f_cEncryptedRegionSkip)               const uint32_t  *f_pEncryptedRegionSkip );

    // Release Clear content is no an IMediaKeySessio interface method.
    CDMi_RESULT ReleaseClearContent(
        __in                                            const uint32_t  f_cbClearContentOpaque,
        __inout_bcount(f_cbClearContentOpaque)                uint8_t  *f_pbClearContentOpaque );

    virtual void setKIDS(
        char *f_kid_0,
        char *f_kid_1);

    virtual int SetDrmDecryptIndex(
        int index);

private:
    // Retrieve PRO from init data.
    static CDMi_RESULT _GetPROFromInitData(
        __in_bcount(f_cbInitData) const DRM_BYTE *f_pbInitData,
        __in DRM_DWORD f_cbInitData,
        __out DRM_DWORD *f_pibPRO,
        __out DRM_DWORD *f_pcbPRO);

    // Parse init data to retrieve PRO from it.
    CDMi_RESULT _ParseInitData(
        __in_bcount(f_cbInitData) const uint8_t *f_pbInitData,
        __in uint32_t f_cbInitData);

    // Parse CDM data to retrieve key ID and custom data (if exists).
    CDMi_RESULT _ParseCDMData(
        __in_bcount(f_cbCDMData) const uint8_t *f_pbCDMData,
        __in uint32_t f_cbCDMData);

    // Build a key message using the supplied license challenge.
    static CDMi_RESULT _BuildKeyMessage(
        __in_bcount(f_cbChallenge) const DRM_BYTE *f_pbChallenge,
        __in DRM_DWORD f_cbChallenge,
        __deref_out_bcount_opt(*f_pcbKeyMessage) DRM_BYTE **f_ppbKeyMessage,
        __out DRM_DWORD *f_pcbKeyMessage);

    // Map PlayReady specific CDMi error to one of the EME errors.
    static int16_t _MapCDMiError(
        __in CDMi_RESULT f_crError);

    static DRM_RESULT DRM_CALL _PolicyCallback(
        __in     const DRM_VOID *f_pvOutputLevelsData,
        __in     DRM_POLICY_CALLBACK_TYPE f_dwCallbackType,
        __in_opt const DRM_KID *f_pKID,
        __in_opt const DRM_LID *f_pLID,
        __in     const DRM_VOID *f_pv );

    DRM_APP_CONTEXT *m_poAppContext;
    DRM_DECRYPT_CONTEXT m_oDecryptContext0;
    DRM_DECRYPT_CONTEXT m_oDecryptContext1;

    DRM_BYTE *m_pbOpaqueBuffer;
    DRM_DWORD m_cbOpaqueBuffer;

    DRM_BYTE *m_pbRevocationBuffer;

    DRM_BYTE *m_pbPRO;
    DRM_DWORD m_cbPRO;

    DRM_ID m_oKeyId;
    DRM_BOOL m_fKeyIdSet;

    char *m_kid_0;
    char *m_kid_1;
    int  m_oDecryptContext_index;

    DRM_CHAR *m_pchCustomData;
    DRM_DWORD m_cchCustomData;

    IMediaKeySessionCallback *m_piCallback;

    KeyState m_eKeyState;

    DRM_CHAR m_rgchSessionID[CCH_BASE64_EQUIV(sizeof(DRM_ID)) + 1];

    DRM_BOOL m_fCommit;
};

// Class that implements the IMediaKeys interface.
class CMediaKeys : public IMediaKeys
{
public:
    // Constructor.
    CMediaKeys(void);

    // Destructor.
    virtual ~CMediaKeys(void);

    // Return whether a key system and mime type (optional) is supported.
    virtual bool IsTypeSupported(
        __in_z_opt const char *f_pwszMimeType,
        __in_z const char *f_pwszKeySystem ) const ;

    // Factory method that creates a media key session using the supplied
    // init data and CDM data (both are optional).
    virtual CDMi_RESULT CreateMediaKeySession(
        __in_z_opt                      const char               *f_pszMimeType,
        __in_bcount_opt( f_cbInitData ) const uint8_t            *f_pbInitData,
        __in                                  uint32_t            f_cbInitData,
        __in_bcount_opt( f_cbCDMData )  const uint8_t            *f_pbCDMData,
        __in                                  uint32_t            f_cbCDMData,
        __deref_out_opt                       IMediaKeySession  **f_ppiMediaKeySession ) ;

    // Close a media key session interface and frees
    // all resources associated with it.
    virtual CDMi_RESULT DestroyMediaKeySession(
        __in IMediaKeySession *f_piMediaKeySession ) ;
};

// Class that implements the IMediaEngineSession interface.
class CMediaEngineSession : public IMediaEngineSession
{
public:
    // Constructor.
    CMediaEngineSession(void);

    // Destructor.
    virtual ~CMediaEngineSession(void);

    // Decrypt a block of data using supplied IV and the optional
    // subsample mapping data. The decrypted data is immediately
    // re-encrypted using the cached session key before being returned
    // to the caller.
    virtual CDMi_RESULT Decrypt(
        __in                                                      uint32_t  f_cdwSubSampleMapping,
        __in_ecount_opt(f_cdwSubSampleMapping)              const uint32_t *f_pdwSubSampleMapping,
        __in                                                      uint32_t  f_cbIV,
        __in_bcount(f_cbIV)                                 const uint8_t  *f_pbIV,
        __in                                                      uint32_t  f_cbData,
        __in_bcount(f_cbData)                               const uint8_t  *f_pbData,
        __out                                                     uint32_t *f_pcbOpaqueClearContentOpaque,
        __deref_out_bcount_opt( *f_pcbOpaqueClearContentOpaque )  uint8_t **f_ppbOpaqueClearContentOpaque ,
        __in   uint32_t f_cEncryptedRegionSkip,
        __in_ecount_opt(f_cEncryptedRegionSkip)               const uint32_t  *f_pEncryptedRegionSkip);

    virtual CDMi_RESULT ReleaseClearContent(
        __in                                            const uint32_t  f_cbClearContentOpaque,
        __inout_bcount(f_cbClearContentOpaque)                uint8_t  *f_pbClearContentOpaque ) ;

    // Initialize the current media engine session instance.
    CDMi_RESULT Init(
        __in CMediaKeySession *f_poMediaKeySession);

private:
    CMediaKeySession *m_poMediaKeySession;
};

} // namespace PRCDMi

#endif  // __CDMI_IMP_H__
