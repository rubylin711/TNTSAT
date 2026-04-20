///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include <pkPAL.h>
#include <pkTestFramework.h>
#include <pkExecutive.h>

#include <platPrivate.h>

//=============================================================================
int TF_Main(int argc, char *argv[])
{
    pkRESULT hr;
    pkRESULT hrTestRun = pkE_FAIL;
    int halError;

    TF_Config config = { sizeof(TF_Config) };

    hr = Test_GetConfig(argc, argv, &config);
    if (pkS_FALSE == hr)
    {
        goto exit;
    }
    else if (pkFAILED(hr))
    {
        TF_Printf("%s() ERROR %s: 0x%x\n", __FUNCTION__, "Test_GetConfig", hr);
        goto exit;
    }

#ifdef MSPK_TF_PAL_NEEDED_EXECUTIVE
    // Initialize_Executive_PAL
    hr = Executive_Startup();
    if(pkFAILED(hr))
    {
        TF_Printf("%s() ERROR %s: 0x%x\n", __FUNCTION__, "Executive_Startup", hr);
        goto executive_exit;
    }
#endif //MSPK_TF_PAL_NEEDED_EXECUTIVE

    // Initialize Decoder_HAL
    //Renderer_SetCmdLineArgs_priv(argc, argv);

    halError = IPTV_HAL_Decoder_HALInit_priv();
    if (0 != halError)
    {
        hr = pkS_FALSE;
        TF_Printf("%s() ERROR %s: 0x%x\n", __FUNCTION__, "IPTV_HAL_Decoder_HALInit_priv", halError);
        goto decoderhal_exit;
    }

    // Run the test ///////////////////////////////
    hrTestRun = Test_Run();

    if (pkFAILED(hrTestRun))
    {
        TF_Printf("%s() ERROR %s: 0x%x\n", __FUNCTION__, "Test_Run", hr);
    }

    // Exit Decoder_HAL
    halError = IPTV_HAL_Decoder_HALExit_priv();
    if (0 != halError)
    {
        hrTestRun = pkE_FAIL;
        TF_Printf("%s() ERROR %s: 0x%x\n", __FUNCTION__, "IPTV_HAL_Decoder_HALExit", halError);
    }

    //Renderer_ClearCmdLineArgs_priv();

decoderhal_exit:

#ifdef MSPK_TF_PAL_NEEDED_EXECUTIVE
    // Terminate_Executive_PAL
    hr = Executive_Shutdown();
    if(pkFAILED(hr))
    {
        TF_Printf("%s() ERROR %s: 0x%x\n", __FUNCTION__, "Executive_Shutdown", hr);
        hrTestRun = pkE_FAIL;
    }
executive_exit:
#endif //MSPK_TF_PAL_NEEDED_EXECUTIVE

exit:
    return pkSUCCEEDED(hrTestRun) ? 0 : 1;
}


