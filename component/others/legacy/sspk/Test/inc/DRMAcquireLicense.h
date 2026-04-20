///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "pkPAL.h"

#include <string>

class IXDrm;

namespace SSPK
{
    
class CLicenseAcquirer
{
public:
    CLicenseAcquirer();

    virtual ~CLicenseAcquirer();

    HRESULT Initialize();

public: // properties

    std::string ChallengeCustomData;
    
    std::string LicenseServerUriOverride;

public: // methods

    //
    // Acquire a DRM license synchronously.
    //
    // Arguments:
    // [pszLicenseServerUri] URL of DRM license server.
    // [pszBase64KeyId]      Base64 encoded license key identifier GUID.
    // [ppszResponseData]    Pointer to a pointer to receive any custom response data.
    //
    // Notes:
    // If ppszResponseData is NULL, no custom response data is fetched.
    //    
    HRESULT AcquireLicense( const char* pszLicenseServerUri, const char* pszBase64KeyId, std::string* pResponseCustomData = NULL );

    //
    // Acquire a DRM license synchronously using provided header
    //
    // Arguments:
    // [cbChallengeHdr]      Size of challenge header data
    // [pbChallengeHdr]      Pointer to challenge header data
    // [ppszResponseData]    Pointer to a pointer to receive any custom response data.
    //
    // Notes:
    // If set, LicenseServerUriOverride property overrides any specified in the provided header data.
    // If ppszResponseData is NULL, no custom response data is fetched.
    //    
    HRESULT AcquireLicense( size_t cbChallengeHdr, const uint8_t* pbChallengeHdr, std::string* pResponseCustomData = NULL);

private:
    IXDrm* _poXDrm;
};

}
