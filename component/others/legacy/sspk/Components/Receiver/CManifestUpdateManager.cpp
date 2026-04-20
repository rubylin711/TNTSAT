///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CManifestUpdateManager.h"
#include "CSocketMbr.h"

CManifestUpdateManager::CManifestUpdateManager()
    : m_pWorker( NULL )
{
}

CManifestUpdateManager::~CManifestUpdateManager()
{
}

// IManifestUpdate implementation

pkRESULT CManifestUpdateManager::SelectStream( _In_ IManifestStream* pStream )
{
    return( ( NULL != m_pWorker ) ? m_pWorker->SelectStream( pStream ) : pkE_NOT_READY );
}

pkRESULT CManifestUpdateManager::DeselectStream( _In_ IManifestStream* pStream )
{
    return( ( NULL != m_pWorker ) ? m_pWorker->DeselectStream( pStream ) : pkE_NOT_READY );
}
