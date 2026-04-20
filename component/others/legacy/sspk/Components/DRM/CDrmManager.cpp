///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AutoLock.h"
#include "CDrmManager.h"
#include "CPassthruDecrypter.h"
#include "CPlayReadyDecrypter.h"
#include "CPlayReadyLicense.h"
#include "Utils.h"

#include <map>
#include <algorithm>

using namespace std;

//#define DRMMANAGER_SPEW
#ifdef DRMMANAGER_SPEW
#define DRMMANAGER_TRACE(x) TRACE(x)
#else
#define DRMMANAGER_TRACE(x)
#endif

#define MAX_NUMBER_OF_LICENSES_DEFERRED     100

CDrmManager::~CDrmManager()
{
    DrmLicenseMap::iterator it;

    for (it = _licenseMap.begin(); it != _licenseMap.end(); ++it)
    {
        ASSERT( it->second.refCount == 0 );
        ASSERT( it->second.pLicense != NULL );

        it->second.pLicense->Dispose();
    }

    _licenseMap.clear();
    _recycleQueue.clear();
}

IDrmLicense* CDrmManager::FindLicenseByHandle( _In_ uint32 hLicense )
{
    if (hLicense != DRM_INVALID_LICENSE)
    {
        AutoLock lock(&_lock);

        DrmLicenseMap::iterator it = _licenseMap.find(hLicense);

        if (it != _licenseMap.end())
        {
            if( it->second.refCount == 0 )
            {
                // The license is 'inactive' but still in cache.
                // Take the entry with the license out of the recycle queue
                // to reactivate it.

                RecycleQueue::iterator itRecycle = std::find( _recycleQueue.begin(), _recycleQueue.end(), it );
                ASSERT( itRecycle != _recycleQueue.end() );

                _recycleQueue.erase( itRecycle );
            }

            ++it->second.refCount;

            return( it->second.pLicense );
        }
    }

    return NULL;
}

IDrmLicense* CDrmManager::FindLicenseByKey(
    _In_ size_t cbKeyId,
    _In_count_(cbKey) const byte* pbKeyId )
{
    AutoLock lock(&_lock);

    DrmLicenseMap::iterator it;

    for (it = _licenseMap.begin(); it != _licenseMap.end(); ++it)
    {
        if( it->second.pLicense->GetEncryptionType() == DrmEncryptionType_PlayReady )
        {
            uint32 nLen = 0;

            byte *pbKeyIdCurr = it->second.pLicense->GetDefaultKeyID(&nLen);

            if (pbKeyIdCurr != NULL
                && nLen == cbKeyId
                && memcmp(pbKeyId, pbKeyIdCurr, nLen) == 0)
            {
                if( it->second.refCount == 0 )
                {
                    // The license is 'inactive' but still in cache.
                    // Take the entry with the license out of the recycle queue
                    // to reactivate it.

                    RecycleQueue::iterator itRecycle = std::find( _recycleQueue.begin(), _recycleQueue.end(), it );
                    ASSERT( itRecycle != _recycleQueue.end() );

                    _recycleQueue.erase( itRecycle );
                }

                ++it->second.refCount;

                return( it->second.pLicense );
            }
        }
    }

    return( NULL );
}

void CDrmManager::CacheLicense( _In_ IDrmCacheableLicense* license )
{
    AutoLock lock(&_lock);

    uint32 handle = license->GetHandle();

    DrmLicenseMap::iterator it = _licenseMap.find( handle );

    if( it == _licenseMap.end() )
    {
        // Create a new entry for the handle since
        // none exists yet.
        LicenseEntry emptyEntry;
        emptyEntry.refCount = 0;
        emptyEntry.pLicense = NULL;

        it = _licenseMap.insert( std::pair<uint32,LicenseEntry>( handle, emptyEntry ) ).first;
    }

    if( it->second.pLicense != NULL )
    {
        // There's already a license in the entry

        // Assert the license left in the entry is:
        // - Stale. There can't be multiple active licenses sharing
        //   the same handle.
        // - Not the same as the one being registered.
        //   The manager must cache a license only once.
        ASSERT( it->second.refCount == 0 );
        ASSERT( it->second.pLicense != license );

        // Take the entry out of the recycle queue
        // It must be there because its ref count must be zero
        RecycleQueue::iterator itRecycle = std::find( _recycleQueue.begin(), _recycleQueue.end(), it );
        ASSERT( itRecycle != _recycleQueue.end() );

        _recycleQueue.erase( itRecycle );

        // Dipose the stale license so we can reuse
        // the entry for the new one.
        it->second.pLicense->Dispose();
    }

    // Set the entry with the license
    it->second.refCount = 1;
    it->second.pLicense = license;
}

void CDrmManager::ReleaseLicense( _In_ uint32 hLicense )
{
    AutoLock lock(&_lock);

    DrmLicenseMap::iterator it = _licenseMap.find(hLicense);

    if( it != _licenseMap.end() )
    {
        // ReleaseLicense can only be called
        // if the license is referenced.
        ASSERT( it->second.refCount > 0 );

        --it->second.refCount;

        if( it->second.refCount == 0 )
        {
            // Set up the entry for recycle
            _recycleQueue.push_back( it );
        }
    }

    // Collect any garbage that overflows
    // the target capacity of the recycle queue

    while( _recycleQueue.size() > MAX_NUMBER_OF_LICENSES_DEFERRED )
    {
        DrmLicenseMap::iterator it = _recycleQueue.front();

        _recycleQueue.pop_front();

        // Entries up for recycling should be inactive.
        ASSERT( it->second.refCount == 0 );

        it->second.pLicense->Dispose();

        _licenseMap.erase( it );
    }
}

// Check whether the decrypter is to be created on demand (when key rotation is used).
bool CDrmManager::IsDecrypterOnDemand(DrmDecryptionMode mode, size_t cbHdr, const byte* pbHdr)
{
    CPlayReadyLicense* licensePR = NULL;
    bool result = false;

    switch (mode)
    {
        case DrmDecryptionMode_SSProtectionHeader:

            licensePR = new CPlayReadyLicense(ChallengeCustomData, LicenseServerUriOverride, CertPathOverride);
            ASSERT(licensePR);

            if (!licensePR->Init(cbHdr, pbHdr, 0, NULL, NULL, true, &result))
            {
                // Failed to get valid license from header
                TRACE_ERROR(("CDrmManager::IsDecrypterOnDemand() -- CPlayReadyLicense::Init() failed. cbHdr:%d, pbHdr:0x%x", cbHdr, pbHdr));
            }
            break;

        default:
            TRACE_ERROR(("CDrmManager::IsDecrypterOnDemand() -- Invalid mode:%d", mode));
            break;
    }

    if( licensePR != NULL )
    {
        licensePR->Dispose();
    }

    return result;
}

IDrmDecrypter* CDrmManager::GetDecrypter(DrmDecryptionMode mode)
{
    return GetDecrypter(mode, 0, NULL, 0, NULL, NULL);
}

IDrmDecrypter* CDrmManager::GetDecrypter(
    _In_ DrmDecryptionMode mode,
    _In_ size_t cbHdr,
    _In_count_(cbHdr) const byte* pbHdr,
    _In_ size_t cbKeyId,
    _In_count_(cbKeyId) const byte* pbKeyId,
    _In_ IDrmCallbackSink* pCallback )
{
    IDrmDecrypter* pContext = NULL;
    IDrmLicense* license = NULL;

    switch (mode)
    {
    case DrmDecryptionMode_IPTV:
        {
            pContext = new CPassthruDecrypter();
            ASSERT(pContext);
        }
        break;
    case DrmDecryptionMode_SSProtectionHeader:
        {

            // When the passed in key id is not NULL, the content is
            // using key rotation. Share the same license instance for
            // fragments that use the same key id.
            if (pbKeyId != NULL)
            {
                ASSERT(cbKeyId > 0);

                license = FindLicenseByKey( cbKeyId, pbKeyId );
            }

            //Create PlayReady license to parse ProtectionHeader
            if ( license == NULL )
            {
                CPlayReadyLicense* licensePR = new CPlayReadyLicense(ChallengeCustomData, LicenseServerUriOverride, CertPathOverride);
                ASSERT(licensePR);

                if( !licensePR->Init( cbHdr, pbHdr, cbKeyId, pbKeyId, pCallback ) )
                {
                    //Failed to get valid license from header
                    TRACE_ERROR(("CDrmManager::GetDecrypter() -- CPlayReadyLicense::Init() failed. cbHdr:%d, pbHdr:0x%x", cbHdr, pbHdr));

                    licensePR->Dispose();

                    return( NULL );
                }

                // Cache the license

                {
                    AutoLock lock(&_lock);

                    do
                    {
                        ++_nextLicenseHandle;
                    }
                    while( _nextLicenseHandle == DRM_INVALID_LICENSE );

                    licensePR->SetHandle( _nextLicenseHandle );

                    CacheLicense( licensePR );

                    license = licensePR;
                }
            }

            ASSERT( license != NULL );

            if (license->GetEncryptionType() == DrmEncryptionType_PlayReady)
            {
                pContext = NEW_NO_THROW CPlayReadyDecrypter(license);
            }

            ASSERT(pContext);
        }
        break;

    default:
        //Unrecognized decryption mode
        TRACE_ERROR(("CDrmManager::GetDecrypter() -- unrecognized decryption mode:%d", mode));
        ASSERT(false);
        break;
    }

    if( license != NULL )
    {
        DRMMANAGER_TRACE(("CDrmManager::GetDecrypter@%p, handle: %u, license: %p", license->GetHandle(), license ));
    }

    DRMMANAGER_TRACE(("CDrmManager::GetDecrypter(), created DRM context for DRM mode(%d):0x%x", mode, pContext));
    return pContext;
}

IDrmDecrypter* CDrmManager::GetDecrypter(DrmDecryptionMode mode, GUID& guid)
{
    IDrmDecrypter* pContext = NULL;

    if (mode == DrmDecryptionMode_SSManifest)
    {
    }
    else
    {
        //Unrecognized decryption mode
        TRACE_ERROR(("CDrmManager::GetDecrypter() -- unrecognized decryption mode:%d, DrmDecryptionMode_SSManifest expected.", mode));
        ASSERT(false);
    }

    return pContext;
}

IDrmDecrypter* CDrmManager::GetDecrypter(DrmDecryptionMode mode, uint32 hLicense)
{
    IDrmDecrypter* pContext = NULL;

    if (mode == DrmDecryptionMode_SSProtectionHeader)
    {
        //Currently only CPlayReadyLicense objects are stored in the license map
        IDrmLicense* license = FindLicenseByHandle( hLicense );
        if ( license != NULL )
        {
            DrmEncryptionType encryption_type = license->GetEncryptionType();
            switch (encryption_type)
            {
            case DrmEncryptionType_PlayReady:
                pContext = NEW_NO_THROW CPlayReadyDecrypter(license);
                break;
            default:
                //Invalid encryption type
                TRACE_ERROR(("CDrmManager::GetDecrypter() -- unrecognized encryption type:%d.", encryption_type));
                break;
            }
            ASSERT(pContext);
        }
    }

    if( pContext == NULL )
    {
        // License not taken by a context
        ReleaseLicense( hLicense );
    }

    return pContext;
}

void CDrmManager::ReleaseDecrypter( _In_ IDrmDecrypter* pContext)
{
    CHECK_ALLOC(pContext);

    uint32 handle = pContext->GetLicenseHandle();

    pContext->Dispose();

    ReleaseLicense( handle );
}

pkRESULT CDrmManager::OverrideCustomData( const char * pResponseCustomData )
{
    ChallengeCustomData = pResponseCustomData;
}

pkRESULT CDrmManager::OverrideLicenseServerUri( const char * pszLicenseServerUri)
{
    LicenseServerUriOverride = pszLicenseServerUri;
}

pkRESULT CDrmManager:: OverrideCertPath( const char * pszCertPath)
{
    CertPathOverride = pszCertPath;
}
// ===============================================================================================================
//DRM Manager factory
// ===============================================================================================================

IDrmManager* IDrmManager::Create(IAVManager* pAVManager)
{
    return new CDrmManager(pAVManager);
}


void IDrmManager::Dispose(IDrmManager* pDrmManager)
{
    delete pDrmManager;
}


