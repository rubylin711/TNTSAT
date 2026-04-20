///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IAVManager.h"
#include "ITunerSession.h"
#include "AutoLock.h"
#include <map>

// ===============================================================================================================
// ITunerSession Factory
// ===============================================================================================================

class CTunerSessionFactory : public ITunerSessionFactory
{
public:
    //Constructor
    CTunerSessionFactory(IAVManager* avManager) : mAVManager(avManager) {}
    //Destructor
    virtual ~CTunerSessionFactory() {}

    //Creates a tuner session or returns already cached one given a pipe id
    __override ITunerSession*       CreateTunerSession(ITunerSessionCallback* callback, const char* id, uint32 featuresEnabled, uint32& handle);
    //Destroys a tuner session from cache
    __override void                 DestroyTunerSession(ITunerSession* tunerSession);
    //Destroys a tuner session from cache given its handle
    __override void                 DestroyTunerSession(uint32 handle);
    //Returna a ITunerSession instance given its handle
    __override ITunerSession*       GetTunerSession(uint32 handle);
    //Returns a tuner session isntance given its string pipe id
    __override ITunerSession*       GetTunerSession(const std::wstring& pipeId);

private:
    //Factory of factories
    IAVManager*                     mAVManager;
    //Managing cached ITunerSession
    Lockable                        mTunerSessionMapLock;
    std::map<uint32,ITunerSession*> mTunerSessionMap;
};

// ===============================================================================================================
// ===============================================================================================================
