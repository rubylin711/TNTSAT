///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CTunePrepareFactory.h"
#include "CSocketMbrManifest.h"
#include "CTuneRequest.h"
#include "Trace.h"
#include <map>
using namespace std;

// ===============================================================================================================
// ===============================================================================================================

CTunePrepareFactory::~CTunePrepareFactory()
{
    while (true)
    {
        // Don't allow factory to be deleted until _tunePrepareMap is empty
        uint32 size = 0;
        {
            AutoLock lock(&_tunePrepareLock);
            size = _tunePrepareMap.size();
        }

        if (size == 0)
            break;

        // Sleep 100ms to give socket threads a chance to exit and for ITunePrepare objects to release
        Executive_Sleep(100);
    }
}

uint32 CTunePrepareFactory::CreateTunePrepareObject(uint32 pipeIdN, const char* tunerurl)
{
    CTuneRequest tuneRequest;
    if (!tuneRequest.ParseUrl(tunerurl))
    {
        TRACE_ERROR(("[%08x] Failed to parse %s", pipeIdN, tunerurl));
        return 0;
    }
    return CreateTunePrepareObject(pipeIdN, tuneRequest);
}

uint32 CTunePrepareFactory::CreateTunePrepareObject(uint32 pipeIdN, CTuneRequest& tuneRequest)
{
    ITunePrepare* tunePrepare = NULL;
    uint32 token = 0;

    if (tuneRequest.IsMbr())
    {
        tunePrepare = new CMbrManifest(_avManager, pipeIdN, tuneRequest);
        ASSERT(tunePrepare);
    }

    if (tunePrepare)
    {
        AutoLock lock(&_tunePrepareLock);

        token = ++_tunePrepareCounter;
        _tunePrepareMap[token] = tunePrepare;
        tunePrepare->SetToken(token);
    }
    else
    {
        TRACE_ERROR(("[%08x] Failed to instantiate TunePrepare for %s", pipeIdN, tuneRequest.TunerUrl.c_str()));
    }

    return token;
}

ITunePrepare* CTunePrepareFactory::GetTunePrepareInterface(uint32 token, bool addRef /*= true*/)
{
    AutoLock lock(&_tunePrepareLock);

    TunePrepareMapItr it = _tunePrepareMap.find(token);
    if (it != _tunePrepareMap.end())
    {
        if (addRef)
            it->second->AddRef();
        return it->second;
    }
    else
    {
        TRACE_ERROR(("GetTunePrepareInterface: Failed to find token:%d", token));
        return NULL;
    }
}

void CTunePrepareFactory::ReleaseTunePrepare(uint32 token)
{
    if (token)
    {
        AutoLock lock(&_tunePrepareLock);

        TunePrepareMapItr it = _tunePrepareMap.find(token);
        if (it != _tunePrepareMap.end())
        {
            if (it->second->Release() == 0)
            {
                _tunePrepareMap.erase(token);
            }
        }
        else
        {
            TRACE_ERROR(("ReleaseTunePrepare: Failed to find token:%d", token));
        }
    }
    else
    {
        TRACE_ERROR(("ReleaseTunePrepare: Invalid token:%d", token));
    }
}

// ===============================================================================================================
// ===============================================================================================================


