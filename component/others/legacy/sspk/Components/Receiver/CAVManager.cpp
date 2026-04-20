///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IAVManager.h"

#include "CReferenceClock.h"
#include "CDiagsManager.h"
#include "CTimesliceManager.h"
#include "BufferPoolFactory.h"
#include "CTunePrepareFactory.h"
#include "CSocketFactory.h"
#include "IDrmManager.h"
#include "CHalDecoderFactory.h"
#include "CDecoderFactory.h"
#include "CRendererFactory.h"
#include "ISpliceEngine.h"
#include "CTunerSessionFactory.h"

#include "CAVEngineConfiguration.h"
#include "CReceiverConfiguration.h"
#include "CClockConfiguration.h"
#include "CDecoderConfiguration.h"
#include "CTrickConfiguration.h"
#include "CSocketMbrConfiguration.h"
#include "CMbrConfiguration.h"
#include "CDiagsManagerConfiguration.h"
#include "IDiagsProvider.h"
#include "ITimeslice.h"
#include "CManifestUpdateManager.h"

#include "AutoLock.h"
#include "Trace.h"
#include <string>
#include <vector>
using namespace std;

//#define FACTORY_SPEW
#if defined(FACTORY_SPEW)
#define FACTORY_MSG(x) TRACE(x)
#else
#define FACTORY_MSG(x)
#endif

// ===============================================================================================================
// Global configuration parameters controlling AV Engine behavior
// ===============================================================================================================

class CAVManager : public IAVManager
{
private:
    friend class IAVManager;

private:
    //Constructor
    CAVManager()
    {
        //Reference clock
        mReferenceClock = new CReferenceClock(); // Use own sample
        CHECK_ALLOC(mReferenceClock);

        //Diagnostics Manager instance
        mDiagnosticsManager = new CDiagsManager();
        CHECK_ALLOC(mDiagnosticsManager);

        //Buffer Pool Factory instance
        mBufferPoolFactory = new BufferPoolFactory();
        CHECK_ALLOC(mBufferPoolFactory);

        //Timeslice Manager instance
        mTimesliceManager = new CTimesliceManager(this);
        CHECK_ALLOC(mTimesliceManager);

        //DRM Manager instance
        mDrmManager = IDrmManager::Create(this);
        CHECK_ALLOC(mDrmManager);

        //Tune Prepare Factory instance
        mTunePrepareFactory = new CTunePrepareFactory(this);
        CHECK_ALLOC(mTunePrepareFactory);

        //Manifest Update Manager instance
        mManifestUpdateManager = new CManifestUpdateManager();
        CHECK_ALLOC(mManifestUpdateManager);

        //Socket Factory instance
        mSocketFactory = new CSocketFactory(this);
        CHECK_ALLOC(mSocketFactory);

        //HAL Decoder Factory instance
        mHalDecoderFactory = new CHalDecoderFactory(this);
        CHECK_ALLOC(mHalDecoderFactory);

        //Decoder Factory instance
        mDecoderFactory = new CDecoderFactory(this); // Use own sample
        CHECK_ALLOC(mDecoderFactory);

        //Renderer Factory instance
        mRendererFactory = new CRendererFactory(this); // Use own sample
        CHECK_ALLOC(mRendererFactory);

        //Splice Engine Factory
        mSpliceEngineFactory = NULL; // Don't need this factory if eFeature_Splicing is not enabled
        //CHECK_ALLOC(mSpliceEngineFactory);

        //Tuner Session Factory instance
        mTunerSessionFactory = new CTunerSessionFactory(this);
        CHECK_ALLOC(mTunerSessionFactory);
    }

    //Destructor
    virtual ~CAVManager()
    {
        //Tuner Session Factory instance
        delete mTunerSessionFactory;

        //Splice Engine Factory instance
        delete mSpliceEngineFactory;

        //Renderer Factory instance
        delete mRendererFactory;

        //Decoder Factory instance
        delete mDecoderFactory;

        //HAL Decoder Factory instance
        delete mHalDecoderFactory;

        //Socket Factory instance
        delete mSocketFactory;

        //Manifest Update Manager instance
        delete mManifestUpdateManager;

        //Tune Prepare Factory instance
        delete mTunePrepareFactory;
                
        //Cleanup IDrmManager
        IDrmManager::Dispose(mDrmManager);
        
        //Timeslice Manager instance
        delete mTimesliceManager;

        //Buffer Pool Factory instance
        delete mBufferPoolFactory;

        //Diagnostics Manager instance
        delete mDiagnosticsManager;

        //Reference Clock
        delete mReferenceClock;
    }

    //Reference Clock
    __override IReferenceClock* GetReferenceClock(void)
    {
        return mReferenceClock;
    }

    //Diagnostics Manager instance
    __override IDiagsManager* GetDiagsManager(void)
    {
        return mDiagnosticsManager;
    }

    //Network Profile Manager

    //Timeslice Manager instance
    __override ITimesliceManager* GetTimesliceManager(void)
    {
        return mTimesliceManager;
    }

    //Buffer Pool Factory instance
    __override IBufferPoolFactory* GetBufferPoolFactory(void)
    {
        return mBufferPoolFactory;
    }

    //Tune Prepare Factory instance
    __override ITunePrepareFactory* GetTunePrepareFactory(void)
    {
        return mTunePrepareFactory;
    }

    //Socket Factory instance
    __override ISocketFactory* GetSocketFactory(void)
    {
        return mSocketFactory;
    }

    //DRM Manager instance
    __override IDrmManager* GetDrmManager(void)
    {
        return mDrmManager;
    }

    //HAL Decoder Factory instance
    __override IHalDecoderFactory* GetHalDecoderFactory(void)
    {
        return mHalDecoderFactory;
    }

    //Decoder Factory instance
    __override IDecoderFactory* GetDecoderFactory(void)
    {
        return mDecoderFactory;
    }

    //Renderer Factory instance
    __override IRendererFactory* GetRendererFactory(void)
    {
        return mRendererFactory;
    }

    //Splice Engine Factory instance
    __override ISpliceEngineFactory* GetSpliceEngineFactory(void)
    {
        return mSpliceEngineFactory;
    }

    //Tuner Session Factory instance
    __override ITunerSessionFactory* GetTunerSessionFactory(void)
    {
        return mTunerSessionFactory;
    }

    //Manifest Update Manager instance
    __override IManifestUpdateManager* GetManifestUpdateManager(void)
    {
        return mManifestUpdateManager;
    }

private:
    //Reference Clock
    IReferenceClock*        mReferenceClock;

    //Diagnostics Manager instance
    IDiagsManager*          mDiagnosticsManager;

    //Timeslice Manager instance
    ITimesliceManager*      mTimesliceManager;

    //Buffer Pool Factory instance
    IBufferPoolFactory*     mBufferPoolFactory;

    //Tune Prepare Factory instance
    ITunePrepareFactory*    mTunePrepareFactory;

    //Socket Factory instance
    ISocketFactory*         mSocketFactory;

    //DRM Manager instance
    IDrmManager*            mDrmManager;

    //HAL Decoder Factory instance
    IHalDecoderFactory*     mHalDecoderFactory;

    //Decoder Factory instance
    IDecoderFactory*        mDecoderFactory;

    //Renderer Factory instance
    IRendererFactory*       mRendererFactory;

    //Splice Engine Factory instance
    ISpliceEngineFactory*   mSpliceEngineFactory;

    //Tuner Session Factory instance
    ITunerSessionFactory*   mTunerSessionFactory;

    //Manifest Update Manager instance
    IManifestUpdateManager* mManifestUpdateManager;
};

// ===============================================================================================================
// ===============================================================================================================

//Global factory of factories instance
static IAVManager* gAVEngineFactory = NULL;
//Lock to protect global factory of factories instance
static Lockable gAVEngineFactoryLock;

//Create a global factory of factories instance
IAVManager* IAVManager::Create(void)
{
    AutoLock lock(&gAVEngineFactoryLock);
    if (gAVEngineFactory == NULL)
    {
        gAVEngineFactory = new CAVManager();
        CHECK_ALLOC(gAVEngineFactory);
    }
    return gAVEngineFactory;
}

//Destroy the global factory of factories instance
IAVManager* IAVManager::Destroy(void)
{
    AutoLock lock(&gAVEngineFactoryLock);
    if (gAVEngineFactory)
    {
        delete gAVEngineFactory;
        gAVEngineFactory = NULL;
    }
    return NULL;
}

//Returns the current global instance of the factory
IAVManager* IAVManager::Instance(void)
{
    AutoLock lock(&gAVEngineFactoryLock);
    ASSERT(gAVEngineFactory);
    return gAVEngineFactory;
}

// ===============================================================================================================
// Setting global configurations
// ===============================================================================================================

bool IAVManager::SetGlobalConfiguration(const std::string& command, const std::vector<std::string>& args)
{
    ITimesliceManager* timesliceManager = IAVManager::Instance()->GetTimesliceManager();
    if (timesliceManager && timesliceManager->Command(command, args))
        return true;

    if (gAVEngineConfiguration.Command(command, args))
        return true;

    if (gReceiverConfiguration.Command(command, args))
        return true;

    if (gClockConfiguration.Command(command, args))
        return true;

    if (gDecoderConfiguration.Command(command, args))
        return true;

    if (gTrickConfiguration.Command(command, args))
        return true;

    if (gSocketMbrConfiguration.Command(command, args))
        return true;

    if (gMbrConfiguration.Command(command, args))
        return true;

    if (gDiagsManagerConfiguration.Command(command, args))
        return true;

    return false;
}

// ===============================================================================================================
// ===============================================================================================================
