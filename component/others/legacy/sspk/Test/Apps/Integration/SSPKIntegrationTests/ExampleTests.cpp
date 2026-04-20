///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <PKTestSuite.h>
#include "STWrapper.h"

#if 0

using namespace SSPK;
using namespace SSPKTest;

////////////////////////////////////////////////////////////////////////////////
PKTEST_GROUP( ExampleTests )
{
    STWrapper* _STObject;
    ExampleTests()
    {

    }

    bool TestSetup()
    {
        _STObject = new STWrapper();
        return true;
    }

    bool TestCleanup()
    {
        _STObject->Close();
        delete _STObject;
        return true;
    }

    ////////////////////////////////////
    PKTEST_METHOD( PauseAndPlay )
    {
        _STObject->OpenVideo("http://iis-wms99/OD/EE4-H264/Content.ism/Manifest",true);
        _STObject->Delay(10000);
        _STObject->Pause();
        _STObject->Delay(5000);
        PKTEST_FUNC_EXIT(_STObject->PlayAndValidate(10000));

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( PlayStateTest )
    {
        _STObject->OpenVideo("http://iis-wms99/OD/EE4-H264/Content.ism/Manifest",false);
        _STObject->Delay(10000);
        PKTEST_FUNC_EXIT(_STObject->PlayAndValidate(10000));

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( PauseStateTest )
    {
        _STObject->OpenVideo("http://iis-wms99/OD/EE4-H264/Content.ism/Manifest",true);
        _STObject->Delay(10000);
        PKTEST_FUNC_EXIT(_STObject->PauseAndValidate(10000));

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( PlayAndStop )
    {
        _STObject->OpenVideo("http://iis-wms99/OD/EE4-H264/Content.ism/Manifest",true);
        _STObject->Delay(10000);
        PKTEST_FUNC_EXIT(_STObject->PauseAndValidate(5000));
        PKTEST_FUNC_EXIT(_STObject->CloseAndValidate(1000));

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( PlayAndPlay )
    {
        _STObject->OpenVideo("http://iis-wms99/OD/EE4-H264/Content.ism/Manifest",true);
        _STObject->Delay(10000);
        PKTEST_FUNC_EXIT(_STObject->PlayAndValidate(5000));
        PKTEST_FUNC_EXIT(_STObject->PlayAndValidate(5000));

    exit:
        return;
    }

};

#endif
