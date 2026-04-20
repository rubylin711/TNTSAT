///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IXDrm.h" // for XDRM_VERSION_CODE values

//String constants passed into IDrmDecrypter::GetProperty()
#define DRM_PROPERTY_LICENSETIMES "DRM_LICENSETIMES"    // returns DrmLicenseTiming array
#define DRM_PROPERTY_RESOURCEID   "DRM_RESOURCEID"      // returns GUID

//Drm define constants
#define DRM_INVALID_LICENSE       0

enum DrmLicenseTimes
{
    DrmLicenseTimes_Start = 0,          // [0] = Start of license acquisition
    DrmLicenseTimes_GenerateChallenge,  // [1] = Start of GenerateChallenge()
    DrmLicenseTimes_GetLicense,         // [2] = Start of SendHttp(GETLICENSE)
    DrmLicenseTimes_ProcessResponse,    // [3] = Start of ProcessResponse()
    DrmLicenseTimes_End,                // [4] = End of license acquisition
    DrmLicenseTimes_Count
};

typedef uint32_t DrmLicenseTiming[DrmLicenseTimes_Count]; // 1ms ticks

enum DrmDecryptionMode
{
    DrmDecryptionMode_Unknown,
    DrmDecryptionMode_IPTV,                 // Mediaroom encryption w/no header
    DrmDecryptionMode_SSProtectionHeader,   // Smooth Streaming content protection
    DrmDecryptionMode_SSManifest,           // Smooth Streaming Manifest protection
};

enum DrmEncryptionType
{
    DrmEncryptionType_Unknown   = XDRM_VERSIONCODE_UNKNOWN,
    DrmEncryptionType_PlayReady = XDRM_VERSIONCODE_PLAYREADY,
    DrmEncryptionType_top
};
