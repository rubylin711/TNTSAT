///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// I N C L U D E     F I L E S
//=============================================================================
#include "Executive_UnitTest.h"

//=============================================================================
// M A C R O    D E F I N I T I O N S
//=============================================================================
//
#define EXECUTIVE_TEST_THREADS      0x01
#define EXECUTIVE_TEST_SEMAPHORES   0x02
#define EXECUTIVE_TEST_EVENTS       0x04
#define EXECUTIVE_TEST_LOCKS        0x08
#define EXECUTIVE_TEST_MEMLEAK      0x10
#define EXECUTIVE_TEST_INTERLOCKED  0x20
#define EXECUTIVE_TEST_MISC         0x40
#define EXECUTIVE_TEST_MEMORY       0x80
#define EXECUTIVE_TEST_ALLGROUPS    0xFF

#define EXECUTIVE_TEST_BASIC        0x01
#define EXECUTIVE_TEST_EXTENDED        0x02
#define EXECUTIVE_TEST_ALLTYPES        0xFF
//=============================================================================
// G L O B A L    V A R I A B L E    D E F I N I T I O N S
//=============================================================================
bool_t g_bIsAborted;

//=============================================================================
// D A T A   S T R U C T U R E    D E F I N I T I O N S
//=============================================================================


//=============================================================================
// L O C A L    F U N C T I O N    D E C L A R A T I O N S
//=============================================================================
static void PrintUsage(void);

//=============================================================================
// P U B L I C    F U N C T I O N    D E F I N I T I O N S
//=============================================================================
pkRESULT Test_GetConfig(int argc, char *argv[], TF_Config *pConfig)
{
    pkRESULT retval = pkS_OK;
    bool_t fVerbose;

    TLC_SetCmdLineArgs(argc, argv);
    if (TLC_IsHelpRequested(&fVerbose))
    {
        PrintUsage();
        retval = pkS_FALSE;
        goto exit;
    }

    // Set the configuration.
    if (pConfig->size < sizeof(TF_Config))
    {
        retval = pkE_INVALIDARG;
        goto exit;
    }

exit:
    return retval;
}

pkRESULT Test_Run(void)
{
    pkRESULT retval = pkE_FAIL, hr;
    BYTE functionalGroups = 0, testTypes = 0;
    bool_t bTestFailed = FALSE;

    // Check command line arguments.
    if (TLC_GetCmdLineArgInt("-h", 0, NULL) == pkS_OK)
    {
        // Display the usage menu.
        PrintUsage();
        goto exit;
        }

    //Parse through functional group and test case type specifications

    if (TLC_GetCmdLineArg("-thread", 0, NULL) == pkS_OK)
    {
        functionalGroups |= EXECUTIVE_TEST_THREADS;
    }
    if (TLC_GetCmdLineArg("-semaphore", 0, NULL) == pkS_OK)
    {
        functionalGroups |= EXECUTIVE_TEST_SEMAPHORES;
    }
    if (TLC_GetCmdLineArg("-event", 0, NULL) == pkS_OK)
    {
        functionalGroups |= EXECUTIVE_TEST_EVENTS;
    }
    if (TLC_GetCmdLineArg("-lock", 0, NULL) == pkS_OK)
    {
        functionalGroups |= EXECUTIVE_TEST_LOCKS;
    }
    if (TLC_GetCmdLineArg("-leak", 0, NULL) == pkS_OK)
    {
        functionalGroups |= EXECUTIVE_TEST_MEMLEAK;
    }
    if (TLC_GetCmdLineArg("-interlocked", 0, NULL) == pkS_OK)
    {
        functionalGroups |= EXECUTIVE_TEST_INTERLOCKED;
    }
    if (TLC_GetCmdLineArg("-memory", 0, NULL) == pkS_OK)
    {
        functionalGroups |= EXECUTIVE_TEST_MEMORY;
    }
    if (TLC_GetCmdLineArg("-misc", 0, NULL) == pkS_OK)
    {
        functionalGroups |= EXECUTIVE_TEST_MISC;
    }
    if (TLC_GetCmdLineArg("-allgroups", 0, NULL) == pkS_OK)
    {
        functionalGroups |= EXECUTIVE_TEST_ALLGROUPS;
    }

    if (TLC_GetCmdLineArg("-basic", 0, NULL) == pkS_OK)
    {
        testTypes |= EXECUTIVE_TEST_BASIC;
    }

    if (TLC_GetCmdLineArg("-extended", 0, NULL) == pkS_OK)
    {
        testTypes |= EXECUTIVE_TEST_EXTENDED;
    }

    if (TLC_GetCmdLineArg("-alltypes", 0, NULL) == pkS_OK)
    {
        testTypes |= EXECUTIVE_TEST_ALLTYPES;
    }

    //Check to see if a default needs to be set
    if ( 0 == testTypes )
    {
        testTypes |= EXECUTIVE_TEST_BASIC;
    }

    if ( 0 == functionalGroups)
    {
        functionalGroups |= EXECUTIVE_TEST_ALLGROUPS;
    }


    // Now perform the requested unit tests.
    if ( ( FALSE == g_bIsAborted ) && ( functionalGroups & EXECUTIVE_TEST_MISC ))
    {
        if ( (FALSE == g_bIsAborted) && ( testTypes & EXECUTIVE_TEST_BASIC ))
        {
            TF_Printf("Executive Unit Test: Running Basic Miscellaneous Test.\n");
            hr = ExecTest_MiscBasicTest();
            if ( pkFAILED(hr) )
            {
                bTestFailed = TRUE;
                TF_Printf("Executive Unit Test: Basic Miscellaneous Test failed with error %x\n", hr);
            }
        }

        if ( ( FALSE == g_bIsAborted ) && ( testTypes & EXECUTIVE_TEST_EXTENDED ))
        {
            TF_Printf("Executive Unit Test: Running Extended Miscellaneous Test.\n");
            hr = ExecTest_MiscExtendedTest();
            if ( pkFAILED(hr) )
            {
                bTestFailed = TRUE;
                TF_Printf("Executive Unit Test: Extended Miscellaneous Test failed with error %x\n", hr);
            }
        }
    }


    if ( (FALSE == g_bIsAborted) && ( functionalGroups & EXECUTIVE_TEST_INTERLOCKED ))
    {
        if ( ( FALSE == g_bIsAborted ) && ( testTypes & EXECUTIVE_TEST_BASIC ))
        {
            TF_Printf("Executive Unit Test: Running Basic Interlocked Test.\n");
            hr = ExecTest_InterlockedBasicTest();
            if ( pkFAILED(hr) )
            {
                bTestFailed = TRUE;
                TF_Printf("Executive Unit Test: Basic Interlocked Test failed with error %x\n", hr);
            }
        }

        if ( ( FALSE == g_bIsAborted ) && ( testTypes & EXECUTIVE_TEST_EXTENDED ))
        {
            TF_Printf("Executive Unit Test: Running Extended Interlocked Test.\n");
            hr = ExecTest_InterlockedExtendedTest();
            if ( pkFAILED(hr) )
            {
                bTestFailed = TRUE;
                TF_Printf("Executive Unit Test: Extended Interlocked Test failed with error %x\n", hr);
            }
        }
    }


    if ( (FALSE == g_bIsAborted) && ( functionalGroups & EXECUTIVE_TEST_MEMORY ))
    {
        if ( ( FALSE == g_bIsAborted ) && ( testTypes & EXECUTIVE_TEST_BASIC ))
        {
            TF_Printf("Executive Unit Test: Running Basic Memory Test.\n");
            hr = ExecTest_MemoryBasicTest();
            if ( pkFAILED(hr) )
            {
                bTestFailed = TRUE;
                TF_Printf("Executive Unit Test: Basic Memory Test failed with error %x\n", hr);
            }
        }

        if ( ( FALSE == g_bIsAborted ) && ( testTypes & EXECUTIVE_TEST_EXTENDED ))
        {
            TF_Printf("Executive Unit Test: Running Extended Memory Test.\n");
            hr = ExecTest_MemoryExtendedTest();
            if ( pkFAILED(hr) )
            {
                bTestFailed = TRUE;
                TF_Printf("Executive Unit Test: Extended Memory Test failed with error %x\n", hr);
            }
        }
    }


    if ( (FALSE == g_bIsAborted) && ( functionalGroups & EXECUTIVE_TEST_MEMLEAK ))
    {
        if ( ( FALSE == g_bIsAborted ) && ( testTypes & EXECUTIVE_TEST_BASIC ))
        {
            TF_Printf("Executive Unit Test: Running Memory Leak Test.\n");
            hr = ExecTest_MemLeakTest();
            if ( pkFAILED(hr) )
            {
                bTestFailed = TRUE;
                TF_Printf("Executive Unit Test: Memory Leak Test failed with error %x\n", hr);
            }
        }

        if ( ( FALSE == g_bIsAborted ) && ( testTypes & EXECUTIVE_TEST_EXTENDED ))
        {
            // there is no extended memleak test
        }
    }


    if ( (FALSE == g_bIsAborted) && ( functionalGroups & EXECUTIVE_TEST_SEMAPHORES ))
    {
        if ( ( FALSE == g_bIsAborted ) && ( testTypes & EXECUTIVE_TEST_BASIC ))
        {
            TF_Printf("Executive Unit Test: Running Basic Semaphore Test.\n");
            hr = ExecTest_SemaphoreBasicTest();
            if ( pkFAILED(hr) )
            {
                bTestFailed = TRUE;
                TF_Printf("Executive Unit Test: Basic Semaphore Test failed with error %x\n", hr);
            }
        }

        if ( ( FALSE == g_bIsAborted ) && ( testTypes & EXECUTIVE_TEST_EXTENDED ))
        {
            TF_Printf("Executive Unit Test: Running Extended Semaphore Test.\n");
            hr = ExecTest_SemaphoreExtendedTest();
            if ( pkFAILED(hr) )
            {
                bTestFailed = TRUE;
                TF_Printf("Executive Unit Test: Extended Semaphore Test failed with error %x\n", hr);
            }
        }
    }


    if ( (FALSE == g_bIsAborted) && ( functionalGroups & EXECUTIVE_TEST_LOCKS ))
    {
        if ( ( FALSE == g_bIsAborted ) && ( testTypes & EXECUTIVE_TEST_BASIC ))
        {
            TF_Printf("Executive Unit Test: Running Basic Lock Test.\n");
            hr = ExecTest_LockBasicTest();
            if ( pkFAILED(hr) )
            {
                bTestFailed = TRUE;
                TF_Printf("Executive Unit Test: Basic Lock Test failed with error %x\n", hr);
            }
        }

        if ( ( FALSE == g_bIsAborted ) && ( testTypes & EXECUTIVE_TEST_EXTENDED ))
        {
            TF_Printf("Executive Unit Test: Running Extended Lock Test.\n");
            hr = ExecTest_LockExtendedTest();
            if ( pkFAILED(hr) )
            {
                bTestFailed = TRUE;
                TF_Printf("Executive Unit Test: Extended Lock Test failed with error %x\n", hr);
            }
        }
    }


    if ( (FALSE == g_bIsAborted) && ( functionalGroups & EXECUTIVE_TEST_EVENTS ))
    {
        if ( ( FALSE == g_bIsAborted ) && ( testTypes & EXECUTIVE_TEST_BASIC ))
        {
            TF_Printf("Executive Unit Test: Running Basic Event Test.\n");
            hr = ExecTest_EventBasicTest();
            if ( pkFAILED(hr) )
            {
                bTestFailed = TRUE;
                TF_Printf("Executive Unit Test: Basic Event Test failed with error %x\n", hr);
            }
        }

        if ( ( FALSE == g_bIsAborted ) && ( testTypes & EXECUTIVE_TEST_EXTENDED ))
        {
            TF_Printf("Executive Unit Test: Running Extended Event Test.\n");
            hr = ExecTest_EventExtendedTest();
            if ( pkFAILED(hr) )
            {
                bTestFailed = TRUE;
                TF_Printf("Executive Unit Test: Extended Event Test failed with error %x\n", hr);
            }
        }
    }


    if ( (FALSE == g_bIsAborted) && ( functionalGroups & EXECUTIVE_TEST_THREADS ))
    {
        if ( ( FALSE == g_bIsAborted ) && ( testTypes & EXECUTIVE_TEST_BASIC ))
        {
            TF_Printf("Executive Unit Test: Running Basic Thread Test.\n");
            hr = ExecTest_ThreadBasicTest();
            if ( pkFAILED(hr) )
            {
                bTestFailed = TRUE;
                TF_Printf("Executive Unit Test: Basic Thread Test failed with error %x\n", hr);
            }
        }

        if ( ( FALSE == g_bIsAborted ) && ( testTypes & EXECUTIVE_TEST_EXTENDED ))
        {
            TF_Printf("Executive Unit Test: Running Extended Thread Test.\n");
            hr = ExecTest_ThreadExtendedTest();
            if ( pkFAILED(hr) )
            {
                bTestFailed = TRUE;
                TF_Printf("Executive Unit Test: Extended Thread Test failed with error %x\n", hr);
            }
        }
    }



exit:
    TF_Printf("\n*************************************\n");
    TF_Printf("Overall Executive Unit Test Result:\n");

    if ( TRUE == g_bIsAborted)
    {
        retval = pkE_ABORT;
        TF_Printf("Executive Unit Test Aborted.\n");
    }

    else
    {
        if ( FALSE == bTestFailed)
        {
            TF_Printf("Executive Unit Test: Unit Test Passed!\n");
            retval = pkS_OK;
        }
        else
        {
            TF_Printf("Executive Unit Test: Unit Test Failed!\n");
            retval = pkE_FAIL;
        }
    }
    TF_Printf("*************************************\n");
    return retval;
}

pkRESULT Test_Abandon(void)
{
    TF_Printf("Executive Unit Test: Abandoning Executive Unit Test . . .\n");
    g_bIsAborted  = TRUE;

    return pkS_OK;
}

//=============================================================================
// L O C A L    F U N C T I O N    D E F I N I T I O N S
//=============================================================================
static void PrintUsage(void)
{
    const char *pszProgName = "Executive";

    TF_Printf(
        "=========================================================\n"
        "%s Usage:\n%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", pszProgName,
        "   -h : Help (display this menu)\n",
        "    \nTEST CASE TYPES:\n",
        "   -------------------------------\n",
        "   Pick one or more test case type\n\n",
        "   -basic [default]: Run Basic tests for selected Functional Groups\n",
        "   -extended       : Run Extended tests for selected Functional groups\n",
        "   -alltypes       : Run both Basic and Extended tests for selected Functional Groups\n",
        "   \nTEST CASE GROUPS:\n",
        "   -------------------------------\n",
        "   Pick one more more test case group\n\n",
        "   -thread              : Test Thread Functional Group\n",
        "   -semaphore           : Test Semaphore Functional Group\n",
        "   -event               : Test Event Functional Group\n",
        "   -lock                : Test Lock Functional Group\n",
        "   -leak                : Test Memory Leak Detection\n",
        "   -memory              : Test Memory Functional Group\n",
        "   -interlocked         : Test Interlocked Functional Group\n",
        "   -misc                : Test Miscellaneous Functional Group\n",
        "   -allgroups [default] : Test all Functional Groups\n\n"
        );
}
