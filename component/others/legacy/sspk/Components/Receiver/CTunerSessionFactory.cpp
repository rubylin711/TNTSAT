///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CTunerSessionFactory.h"
#include "CReceiver.h"
#include "CPipeId.h"
#include "Trace.h"
#include <map>
#include <string>
using namespace std;

//#define FACTORY_SPEW
#if defined(FACTORY_SPEW)
#define FACTORY_MSG(x) TRACE(x)
#else
#define FACTORY_MSG(x)
#endif

// ===============================================================================================================
// ITunerSession Factory
// ===============================================================================================================

//Creates a tuner session given a pipe id
ITunerSession* CTunerSessionFactory::CreateTunerSession(ITunerSessionCallback* callback, const char* id, uint32 featuresEnabled, uint32& handle)
{
    ITunerSession* tunerSession = NULL;
    handle = 0;
    if (id != NULL)
    {
        //Get our tuner session pipe id
        string pipeId(id);

        //Create a new tuner session
        tunerSession = new CReceiver(mAVManager, callback, pipeId, featuresEnabled);
        CHECK_ALLOC(tunerSession);

        //Add the new tuner session to the cache
        {
            AutoLock lock(&mTunerSessionMapLock);
            handle = tunerSession->GetPipeIdN();
            mTunerSessionMap[handle] = tunerSession;
        }
    }

    return tunerSession;
}

//Destroys a tuner session from cache
void CTunerSessionFactory::DestroyTunerSession(ITunerSession* tunerSession)
{
    AutoLock lock(&mTunerSessionMapLock);
    if (tunerSession != NULL)
    {
        //Delete the tuner session from the active list
        uint32 pipeIdN = tunerSession->GetPipeIdN();
        if (mTunerSessionMap[pipeIdN] == tunerSession)
        {
            mTunerSessionMap.erase(pipeIdN);
        }
        //Let go of the tuner session
        delete tunerSession;
    }
}

//Destroys a tuner session from cache given its handle
void CTunerSessionFactory::DestroyTunerSession(uint32 handle)
{
    AutoLock lock(&mTunerSessionMapLock);
    ITunerSession* tunerSession = GetTunerSession(handle);
    if (tunerSession != NULL)
    {
        //Delete the tuner session from the active list
        if (mTunerSessionMap[handle] == tunerSession)
        {
            mTunerSessionMap.erase(handle);
        }
        //Let go of the tuner session
        delete tunerSession;
    }
}

//Returns a tuner session instance given its handle
ITunerSession* CTunerSessionFactory::GetTunerSession(uint32 handle)
{
    AutoLock lock(&mTunerSessionMapLock);
    return mTunerSessionMap[handle];
}

//Returns a tuner session isntance given it's pipeid
ITunerSession* CTunerSessionFactory::GetTunerSession(const std::wstring& pipeId)
{
    AutoLock lock(&mTunerSessionMapLock);
    uint32 pipeIdN = PipeId_XMLStringToU32(pipeId);
    return mTunerSessionMap[pipeIdN];
}

// ===============================================================================================================
// ===============================================================================================================
