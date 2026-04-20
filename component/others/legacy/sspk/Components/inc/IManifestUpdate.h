///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

// ===============================================================================================================
// The structure used for any manifest changes such as stream changes
// ===============================================================================================================

#include "IManifestStream.h"

// ===============================================================================================================

class IManifestUpdate
{
public:
    /// <summary>
    /// Notification for stream selection
    /// </summary>
    /// <param name="pStream">The stream is selected in the manifest</param>
    virtual pkRESULT SelectStream( _In_ IManifestStream* pStream ) = 0;

    /// <summary>
    /// Notification for stream deselection
    /// </summary>
    /// <param name="pStream">The stream is deselected in the manifest</param>
    virtual pkRESULT DeselectStream( _In_ IManifestStream* pStream ) = 0;
};
