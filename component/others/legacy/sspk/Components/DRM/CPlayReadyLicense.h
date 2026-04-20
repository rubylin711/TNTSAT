///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include "IDrmLicense.h"
#include "IXDrm.h"



/// <summary>
/// Implements IDrmCacheableLicense for PlayReady
/// </summary>

class CPlayReadyLicense
    : public IDrmCacheableLicense
{
public:

    CPlayReadyLicense();
    CPlayReadyLicense(std::string &CustomData, std::string &LicenseServerUri);
    CPlayReadyLicense(std::string &CustomData, std::string &LicenseServerUri, std::string &CertPath);

    virtual ~CPlayReadyLicense();

    bool Init(
            _In_ size_t cbHdr,
            _In_count_(cbHdr) const byte* pbHdr,
            _In_ size_t cbKeyId,
            _In_count_(cbKeyId) const byte* pbKeyId,
            _In_opt_ IDrmCallbackSink* pCallback = NULL,
            _In_ bool fInitOnly = false,
            _Out_opt_ bool *pfIsDecrypterOnDemand = NULL );

    void SetHandle( _In_ uint32 handle ) { _handle = handle; }

    //
    // IDrmLicense
    //

    __override void GetLicenseTimes(
                    _Out_ DrmLicenseTiming* pTimesOut );

    __override byte* GetDefaultKeyID(
                    _Out_ uint32 *pLen )
    {
        CHECK_ALLOC(pLen);
        *pLen = _defaultKeyID.Size();
        return( _defaultKeyID.Data() );
    }

    __override IXDrm* GetIXDrm(void)
    {
        return _pIXDrm;
    }

    //
    // IDrmLicenseDispose
    //

    __override void Dispose();

    //
    // Implementation
    //

    void OPLCallback( _In_ XDRM_OPL_DATA* pOPLData );

private:
    void              Cleanup(void);
    pkRESULT          AcquireLicense(void *pvDecryptContext);
    bool              IsDecrypterOnDemand();

    //
    // Attributes
    //

    IXDrm*            _pIXDrm;
    IDrmCallbackSink* _pCallbackSink;
    DrmLicenseTiming  _drmTimes;

    GUID              _guidResID;
    CByteBuffer       _defaultKeyID;
    
    std::string ChallengeCustomData;
    std::string LicenseServerUriOverride;
    std::string CertPathOverride;
};

