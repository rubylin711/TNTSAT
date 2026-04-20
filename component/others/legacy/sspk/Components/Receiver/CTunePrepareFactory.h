///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IAVManager.h"
#include "ITunePrepare.h"
#include "AutoLock.h"
#include <map>

// ===============================================================================================================
// ===============================================================================================================

class CTuneRequest;

typedef std::map<uint32, ITunePrepare*> TunePrepareMap;
typedef TunePrepareMap::iterator TunePrepareMapItr;

class CTunePrepareFactory : public ITunePrepareFactory
{
public:
    CTunePrepareFactory(IAVManager* avManager) : _avManager(avManager), _tunePrepareCounter(0) {}
    virtual ~CTunePrepareFactory();

    __override uint32           CreateTunePrepareObject(uint32 pipeIdN, const char* tunerurl);
    __override uint32           CreateTunePrepareObject(uint32 pipeIdN, CTuneRequest& tuneRequest);
    __override ITunePrepare*    GetTunePrepareInterface(uint32 token, bool addRef = true);
    __override void             ReleaseTunePrepare(uint32 token);

private:
    //Factory of factories
    IAVManager*     _avManager;

    //Currently active tune prepare objects
    TunePrepareMap  _tunePrepareMap;
    uint32          _tunePrepareCounter;
    Lockable        _tunePrepareLock;
};

// ===============================================================================================================
// ===============================================================================================================
