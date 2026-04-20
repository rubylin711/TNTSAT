///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

// ===============================================================================================================
// The structure used for passing any manifest changes such as stream changes
// to other A/V components
// ===============================================================================================================

// ===============================================================================================================

class IManifestUpdate;
class IManifestStream;

class IManifestUpdateManager
{
public:

    virtual pkRESULT SelectStream( _In_ IManifestStream* pStream ) = 0;
    virtual pkRESULT DeselectStream( _In_ IManifestStream* pStream ) = 0;
    virtual void SetUpdateWorker( _In_ IManifestUpdate* pWorker ) = 0;
};
