///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

//
// Executive_UnitTest_Memleak.c: Defines the entry point for the sample Media Center Extender Platform.
// Unit tests for the Executive PAL Memory leak detection.
//

//=============================================================================
// I N C L U D E     F I L E S
//=============================================================================
#include "Executive_UnitTest.h"


//=============================================================================
// P U B L I C    F U N C T I O N    D E F I N I T I O N S
//=============================================================================

pkRESULT ExecTest_MemLeakTest(void)
{
#ifdef MCX_BUILDOPTION_MEMORYTRACKING
    // create a memory leak on purpose to show what the memory leak detection
    // code does
    do
    {
        char *pBuffer1 = new char[1024];
        char *pBuffer2 = (char *)Executive_Alloc(1024, FALSE);
    } while (false);
#endif

    return pkS_OK;
}
