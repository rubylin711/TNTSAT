///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>

// ===============================================================================================================
// External classes accessed
// ===============================================================================================================

class IReferenceClock;
class IDiagsManager;
class ITimesliceManager;
class IBufferPoolFactory;
class ITunePrepareFactory;
class ISocketFactory;
class IDrmManager;
class IHalDecoderFactory;
class IDecoderFactory;
class IRendererFactory;
class ISpliceEngineFactory;
class ITunerSessionFactory;
class IManifestUpdateManager;

// ===============================================================================================================
// Global factory of factories
// ===============================================================================================================

class IAVManager
{
public:
    //Create a global factory of factories instance
    static IAVManager*              Create(void);

    //Destroy the global factory of factories instance
    static IAVManager*              Destroy(void);

    //Returns the global factory of factories instance
    static IAVManager*              Instance(void);

public:

    //Setting global configurations
    static bool                     SetGlobalConfiguration(const std::string& command, const std::vector<std::string>& args);

public:
    //Destructor
    virtual ~IAVManager() {}

    //Reference Clock
    virtual IReferenceClock*        GetReferenceClock(void) = 0;

    //Diagnostics Manager instance
    virtual IDiagsManager*          GetDiagsManager(void) = 0;

    //Timeslice Manager instance
    virtual ITimesliceManager*      GetTimesliceManager(void) = 0;

    //Buffer Pool Factory instance
    virtual IBufferPoolFactory*     GetBufferPoolFactory(void) = 0;

    //Tune Prepare Factory instance
    virtual ITunePrepareFactory*    GetTunePrepareFactory(void) = 0;

    //Socket Factory instance
    virtual ISocketFactory*         GetSocketFactory(void) = 0;

    //DRM Manager instance
    virtual IDrmManager*            GetDrmManager(void) = 0;

    //HAL Decoder Factory instance
    virtual IHalDecoderFactory*     GetHalDecoderFactory(void) = 0;

    //Decoder Factory instance
    virtual IDecoderFactory*        GetDecoderFactory(void) = 0;

    //Renderer Factory instance
    virtual IRendererFactory*       GetRendererFactory(void) = 0;

    //Splice Engine Factory instance
    virtual ISpliceEngineFactory*   GetSpliceEngineFactory(void) = 0;

    //Tuner Session Factory instance
    virtual ITunerSessionFactory*   GetTunerSessionFactory(void) = 0;

    //Manifest Update Manager instance
    virtual IManifestUpdateManager* GetManifestUpdateManager(void) = 0;
};

// ===============================================================================================================
// ===============================================================================================================
