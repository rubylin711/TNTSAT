///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include "IDrmManager.h"
#include <map>
#include <list>
#include "IDrmLicense.h"


class IAVManager;

class CDrmManager : public IDrmManager
{
public:

    CDrmManager( _In_ IAVManager* avManager)
        : _avManager(avManager)
    {
    }

private:

    virtual ~CDrmManager();

    struct LicenseEntry
    {
        uint32 refCount;
        IDrmCacheableLicense* pLicense;
    };

    typedef std::map<uint32, LicenseEntry> DrmLicenseMap;
    typedef std::list<DrmLicenseMap::iterator> RecycleQueue;

    //
    // IDrmManager implementation
    //

    __override bool IsDecrypterOnDemand(DrmDecryptionMode mode, size_t cbHdr, const byte* pbHdr);

    __override IDrmDecrypter* GetDecrypter(DrmDecryptionMode mode);
    __override IDrmDecrypter* GetDecrypter(DrmDecryptionMode mode, size_t cbHdr, const byte* pbHdr, size_t cbKeyId, const byte* pbKeyId, IDrmCallbackSink* pCallback);
    __override IDrmDecrypter* GetDecrypter(DrmDecryptionMode mode, GUID& guid);

    // Fetch new DrmContext using handle from pre-existing object (in case license is shared)
    // Each DrmContext will track sampleID / offset independently
    __override IDrmDecrypter* GetDecrypter(DrmDecryptionMode mode, uint32 hLicense);
    __override void           ReleaseDecrypter( _In_ IDrmDecrypter* pContext);
    
    __override pkRESULT OverrideCustomData( const char * pResponseCustomData );
    __override pkRESULT OverrideLicenseServerUri( const char * pszLicenseServerUri);
    __override pkRESULT OverrideCertPath( const char * pszCertPath);
private:

    IDrmLicense*    FindLicenseByHandle( _In_ uint32 hLicense );
    IDrmLicense*    FindLicenseByKey( _In_ size_t cbKeyId, _In_count_(cbKey) const byte* pbKeyId );
    void            CacheLicense( _In_ IDrmCacheableLicense* license );
    void            ReleaseLicense( _In_ uint32 hLicense );

    Lockable                _lock;
    DrmLicenseMap           _licenseMap;
    RecycleQueue            _recycleQueue;
    IAVManager*             _avManager;
    uint32                  _nextLicenseHandle;

    std::string ChallengeCustomData;
    std::string LicenseServerUriOverride;
    std::string CertPathOverride;
};
