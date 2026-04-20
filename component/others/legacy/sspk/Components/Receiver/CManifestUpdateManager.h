///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IManifestUpdateManager.h"
#include "IManifestStream.h"

class CSocketMbr;

class CManifestUpdateManager : public IManifestUpdateManager
{
public:

    CManifestUpdateManager();
    ~CManifestUpdateManager();

    // IManifestUpdateManager implementation

    __override pkRESULT SelectStream( _In_ IManifestStream* pStream );
    __override pkRESULT DeselectStream( _In_ IManifestStream* pStream );
    __override void SetUpdateWorker( _In_ IManifestUpdate* pWorker ) { m_pWorker = pWorker; }

private:

    IManifestUpdate* m_pWorker;
};
