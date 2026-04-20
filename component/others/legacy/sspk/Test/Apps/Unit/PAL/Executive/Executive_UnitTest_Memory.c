///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

//
// Executive_UnitTest_Memory.c: Defines the entry point for the sample Media Center Extender Platform.
// Unit tests for the Executive PAL Memory implementation.
//
/* Test the following APIs:

void *  pkAPI Executive_Alloc(
            uint32_t cb,
            bool_t fZeroInit);

void pkAPI Executive_Free(
            void *pv);

void *  pkAPI Executive_ReAlloc(
            void *pv,
            uint32_t cb,
            bool_t fZeroInit);
*/
//=============================================================================
// I N C L U D E     F I L E S
//=============================================================================
#include "Executive_UnitTest.h"
// TODO: harrypy #include <pkDebug.h>
pkRESULT pkAPI Debug_GetCpuUsage(uint32_t *pCpuUsage)
{
    *pCpuUsage = 0;
    return pkS_OK;
}

pkRESULT pkAPI Debug_GetMemoryUsed(uint32_t *pMemoryUsage)
{
    *pMemoryUsage = 0;
    return pkS_OK;
}

pkRESULT pkAPI Debug_GetMemoryAvailable(uint32_t *pMemoryAvailable)
{
    *pMemoryAvailable = 0;
    return pkS_OK;
}


#define VERIFY_ONLY     TRUE
#define FILL_AND_VERIFY FALSE

uint32_t g_FillPattern1 = 0xDEADBEEF;
uint32_t g_FillPattern2 = 0xABCDEF01;

//=============================================================================
// L O C A L    F U N C T I O N    P R O T O T Y P E S
//=============================================================================
bool_t FillandVerifyMemory(void* pBuffer, uint32_t size, bool_t verifyOnly, uint32_t* fillPattern);
bool_t VerifyZeroFill(void* pBuffer, uint32_t size);
pkRESULT AllocAndVerifyMemory(uint32_t size, bool_t fZeroInit);
pkRESULT NullReAllocAndVerifyMemory(uint32_t size, bool_t fZeroInit);
pkRESULT ReAllocAndVerifyMemory(uint32_t original_size, uint32_t new_size, bool_t fOriginalZeroInit, bool_t fNewZeroInit);

//=============================================================================
// P U B L I C    F U N C T I O N    D E F I N I T I O N S
//=============================================================================

pkRESULT ExecTest_MemoryBasicTest(void)
{
    pkRESULT retval, hr;
    uint32_t bytesToAllocate, newBytesToAllocate, casesPassed, casesFailed;
    bool_t fOriginalZeroInit, fNewZeroInit;

    casesPassed = casesFailed = 0;
    retval = pkE_FAIL;


    //Case Number 2
    //Basic Test
    //Functional Group: Memory
    //Test Title: Allocate a block of memory, use debug APIs to verify allocation amount, fill memory
    bytesToAllocate = 2048;
    fOriginalZeroInit = FALSE;
    hr = AllocAndVerifyMemory( bytesToAllocate, fOriginalZeroInit );
    if (pkSUCCEEDED(hr))
    {
        casesPassed++;
    }
    else
    {
        TF_Printf("Test case 2 failed with error %x\n", hr );
        casesFailed++;
    }

    //Case Number 3
    //Basic Test
    //Functional Group: Memory
    //Test Title: ReAlloc a variable to a larger size, use debug APIs to verify delta, fill new size
    bytesToAllocate = 2048;
    newBytesToAllocate = 3000;
    fOriginalZeroInit = FALSE;
    fNewZeroInit = FALSE;

    hr = ReAllocAndVerifyMemory( bytesToAllocate, newBytesToAllocate, fOriginalZeroInit, fNewZeroInit);

    if ( pkSUCCEEDED(hr))
    {
        casesPassed++;
    }
    else
    {
        TF_Printf("Test case 3 failed with error %x\n, hr");
        casesFailed++;
    }

    //Case Number 6
    //Basic Test
    //Functional Group: Memory
    //Test Title: ReAlloc a variable to a smaller size, use debug APIs to verify delta, fill new size
    bytesToAllocate = 2048;
    newBytesToAllocate = 3000;
    fOriginalZeroInit = FALSE;
    fNewZeroInit = FALSE;

    hr = ReAllocAndVerifyMemory( bytesToAllocate, newBytesToAllocate, fOriginalZeroInit, fNewZeroInit);
    if ( pkSUCCEEDED(hr))
    {
        casesPassed++;
    }
    else
    {
        TF_Printf("Test Case 6 failed with error %x\n", hr);
        casesFailed++;
    }

    TF_Printf("*************************************\n");
    TF_Printf("Executive Unit Test: Basic Memory Test Results\n");
    TF_Printf("Tests Passed: %d\n",casesPassed);
    TF_Printf("Tests Failed: %d\n",casesFailed);
    TF_Printf("*************************************\n");

    if ( 0 == casesFailed )
    {
        retval = pkS_OK;
    }
    else
    {
        retval = pkE_FAIL;
    }

    return retval;

}

pkRESULT ExecTest_MemoryExtendedTest(void)
{
    pkRESULT retval=pkE_FAIL, hr;
    uint32_t bytesToAllocate, newBytesToAllocate, casesPassed= 0, casesFailed = 0, i;
    bool_t fOriginalZeroInit = FALSE, fNewZeroInit = FALSE, fLoopFailed = FALSE;
    uint32_t bytesToAllocateArray[10] = { 1, 2, 3, 4, 37, 513, 511, 1000, 2047, 2049};

    //Case Number 50
    //Extended Test
    //Functional Group: Memory
    //Test Title: Allocate 10 different blocks of memory of interesting sizes (powers of two, off by one powers of two, etc) and fill them with different data.  Verify all blocks after filling all blocks to be sure that there were no overlaps.
    fOriginalZeroInit = FALSE;
    fLoopFailed = FALSE;
    for ( i = 0; i < 10; i++)
    {
        hr = AllocAndVerifyMemory( bytesToAllocateArray[i], fOriginalZeroInit );
        if (pkFAILED(hr))
        {
            TF_Printf("Test case 50 failed testing an allocation of size %d with error %x\n", bytesToAllocateArray[i], hr );
            fLoopFailed = TRUE;
            break;
        }
    }

    if ( TRUE == fLoopFailed)
    {
        casesFailed++;
    }

    else
    {
        casesPassed++;
    }


    //Case Number 51
    //Extended Test
    //Functional Group: Memory
    //Test Title: Alloc 10 different blocks of memory of interesting sizes (powers of two, off by one powers of two, etc) with fZeroInit set to True.  Verify results.
    fOriginalZeroInit = TRUE;
    fLoopFailed = FALSE;
    for ( i = 0; i < 10; i++)
    {
        hr = AllocAndVerifyMemory( bytesToAllocateArray[i], fOriginalZeroInit );
        if (pkFAILED(hr))
        {
            TF_Printf("Test case 51 failed testing an allocation of size %d with error %x\n", bytesToAllocateArray[i], hr );
            fLoopFailed = TRUE;
            break;
        }
    }

    if ( TRUE == fLoopFailed)
    {
        casesFailed++;
    }

    else
    {
        casesPassed++;
    }


    //Case Number 52
    //Extended Test
    //Functional Group: Memory
    //Test Title: ReAlloc using NULL for the *pv, use debug APIs to verify full size allocated, fill memory and verify
    bytesToAllocate = 63;
    fOriginalZeroInit = FALSE;
    hr = NullReAllocAndVerifyMemory(bytesToAllocate, fOriginalZeroInit);
    if ( pkSUCCEEDED(hr))
    {
        casesPassed++;
    }
    else
    {
        TF_Printf("Test case 52 failed with error %x\n", hr);
        casesFailed++;
    }

    //Case Number 53
    //Extended Test
    //Functional Group: Memory
    //Test Title: Alloc with a cb of 0
    bytesToAllocate = 0;
    fOriginalZeroInit = FALSE;
    hr = AllocAndVerifyMemory(bytesToAllocate, fOriginalZeroInit);
    if ( pkSUCCEEDED(hr))
    {
        casesPassed++;
    }
    else
    {
        TF_Printf("Test case 53 failed with error %x\n", hr);
        casesFailed++;
    }

    //Case Number 54
    //Extended Test
    //Functional Group: Memory
    //Test Title: ReAlloc with null *pv and a cb of zero
    bytesToAllocate = 0;
    fOriginalZeroInit = FALSE;
    hr = NullReAllocAndVerifyMemory(bytesToAllocate, fOriginalZeroInit);
    if ( pkSUCCEEDED(hr))
    {
        casesPassed++;
    }
    else
    {
        TF_Printf("Test case 54 failed with error %x\n", hr);
        casesFailed++;
    }

    //Case Number 55
    //Extended Test
    //Functional Group: Memory
    //Test Title: ReAlloc a variable with >0 size with a cb of zero
    bytesToAllocate = 25;
    newBytesToAllocate = 0;
    fOriginalZeroInit = fNewZeroInit = FALSE;
    hr = ReAllocAndVerifyMemory(bytesToAllocate, newBytesToAllocate, fOriginalZeroInit, fNewZeroInit);

    if ( pkSUCCEEDED(hr))
    {
        casesPassed++;
    }
    else
    {
        TF_Printf("Test case 55 failed with error %x\n", hr);
        casesFailed++;
    }

    //Case Number 56
    //Extended Test
    //Functional Group: Memory
    //Test Title: ReAlloc a block of memory that was Alloced with a cb of 0 with a cb of 0
    bytesToAllocate = 0;
    newBytesToAllocate = 0;
    fOriginalZeroInit = fNewZeroInit = FALSE;
    hr = ReAllocAndVerifyMemory(bytesToAllocate, newBytesToAllocate, fOriginalZeroInit, fNewZeroInit);

    if ( pkSUCCEEDED(hr))
    {
        casesPassed++;
    }
    else
    {
        TF_Printf("Test case 56 failed with error %x\n", hr);
        casesFailed++;
    }

    //Case Number 57
    //Extended Test
    //Functional Group: Memory
    //Test Title: ReAlloc a block of memory that was Alloced with a cb of 0 with a cb > 0
    bytesToAllocate = 0;
    newBytesToAllocate = 25;
    fOriginalZeroInit = fNewZeroInit = FALSE;
    hr = ReAllocAndVerifyMemory(bytesToAllocate, newBytesToAllocate, fOriginalZeroInit, fNewZeroInit);

    if ( pkSUCCEEDED(hr))
    {
        casesPassed++;
    }
    else
    {
        TF_Printf("Test case 57 failed with error %x\n", hr);
        casesFailed++;
    }

    TF_Printf("*************************************\n");
    TF_Printf("Executive Unit Test: Extended Memory Test Results\n");
    TF_Printf("Tests Passed: %d\n",casesPassed);
    TF_Printf("Tests Failed: %d\n",casesFailed);
    TF_Printf("*************************************\n");

    if ( 0 == casesFailed )
    {
        retval = pkS_OK;
    }
    else
    {
        retval = pkE_FAIL;
    }

    return retval;
}


//=============================================================================
// L O C A L    F U N C T I O N    D E F I N I T I O N S
//=============================================================================
bool_t VerifyZeroFill(void* pBuffer, uint32_t size)
{
    bool_t retval = TRUE;
    BYTE *memoryToVerify=NULL;
    uint32_t i;

    memoryToVerify = (BYTE*) pBuffer;

    for (i = 0; i < size; i++)
    {
        if ( 0 !=  memoryToVerify[i] )
        {
            retval = FALSE;
            TF_Printf("Executive Unit Test: Error Verifying array!  Expected 0 at byte %d, got %d instead!\n", i, memoryToVerify[i]);
            break;
        }
     }

    return retval;
}

bool_t FillandVerifyMemory(void* pBuffer, uint32_t size, bool_t verifyOnly, uint32_t *fillPattern)
{
    bool_t retval = TRUE;
    BYTE *fillData = NULL, *memoryToFill = NULL;
    uint32_t i;

    memoryToFill = (BYTE*) pBuffer;
    fillData = (BYTE*) fillPattern;

    for (i = 0; i < size; i++)
    {
        memoryToFill[i] = fillData[i%4];
    }

    for (i = 0; i < size; i++)
    {
        if ( fillData[i%4] != memoryToFill[i] )
        {
            TF_Printf("Executive Unit Test: Error verifying byte %d. Actual value: %d, Expected Value: %d!\n", i, memoryToFill[i], fillData[i%4]);
            retval = FALSE;
            break;
        }
    }

    return retval;
 }

pkRESULT AllocAndVerifyMemory(uint32_t size, bool_t fZeroInit)
{
    uint32_t memoryBefore, memoryAfter;
    pkRESULT retval = pkE_FAIL, hr;
    void *pBuffer1 = NULL;

    hr = Debug_GetMemoryUsed( &memoryBefore );

    if ( pkFAILED(hr) )
    {
        if ( pkE_NOTIMPL == hr)
        {
            TF_Printf("Executive Unit Test: WARNING - Debug_GetMemoryUsed not implemented, some verification could not be completed.\n\n");
        }
        else
        {
            TF_Printf("Executive Unit Test: Verifying memory allocation amount failed with error: %x\n", hr);
            goto cleanup;
        }
    }

    pBuffer1 = Executive_Alloc( size, fZeroInit);
    if ( ( NULL == pBuffer1 ) && ( size > 0 ))
    {
        TF_Printf("Executive Unit Test: Unable to allocate a chunk of memory in the size: %d \n", size);
        retval = pkE_FAIL;
        goto exit;
    }

    else if ( 0 == size )
    {
        if ( NULL == pBuffer1)
        {
            TF_Printf("Executive Unit Test: Memory allocation for a block of size 0 failed.\n");
            retval = pkE_FAIL;
            goto exit;
        }
        else
        {
            retval = pkS_OK;
            goto cleanup; //The Alloc succeeded, but trying to fill or verify a block of size 0 is pointless
        }
    }

    //Verify zero fill, if applicable
    if ( TRUE == fZeroInit)
    {
        if ( FALSE == VerifyZeroFill(pBuffer1, size) )
        {
            TF_Printf("Executive Unit Test: TEST FAILED - allocated memory not zero filled.\n");
            retval = pkE_FAIL;
            goto cleanup;
        }
    }

    //Test case failure for the next step will be detected in the form of a segmentation fault
    if ( FALSE == FillandVerifyMemory(pBuffer1, size, FILL_AND_VERIFY, &g_FillPattern1) )
    {
        TF_Printf("Executive Unit Test: TEST FAILED - Unable to fill and verify allocated memory.\n");
        retval = pkE_FAIL;
        goto cleanup;
    }

    retval = pkS_OK;

cleanup:
    if ( NULL != pBuffer1 )
    {
        Executive_Free( pBuffer1 );
        pBuffer1 = NULL;
    }

    //Make sure there were no leaks during testing
    hr = Debug_GetMemoryUsed( &memoryAfter );
    if ( pkFAILED(hr) )
    {
        if ( pkE_NOTIMPL != hr)
        {
            TF_Printf("Executive Unit Test: Verifying memory allocation amount failed with error: %x\n", hr);
            retval = pkE_UNEXPECTED;
        }
    }
    else
    {
        if ( memoryAfter != memoryBefore )
        {
            TF_Printf("Executive Unit Test: TEST FAILED - Expected no change from original allocated memory after Free, actual change was %d\n", (memoryAfter - memoryBefore));
            retval = pkE_UNEXPECTED;
        }
    }

exit:
    return retval;

}


pkRESULT ReAllocAndVerifyMemory(uint32_t original_size, uint32_t new_size, bool_t fOriginalZeroInit, bool_t fNewZeroInit)
{
    uint32_t memoryBefore, memoryAfter, verifySize;
    pkRESULT retval = pkE_FAIL, hr;
    void *pBuffer1=NULL, *pBuffer2=NULL;

    hr = Debug_GetMemoryUsed( &memoryBefore );
    if ( pkFAILED(hr) )
    {
        if ( pkE_NOTIMPL == hr)
        {
            TF_Printf("Executive Unit Test: WARNING - Debug_GetMemoryUsed not implemented, some verification could not be completed.\n\n");
        }
        else
        {
            TF_Printf("Executive Unit Test: Verifying memory allocation amount failed with error: %x\n", hr);
            goto cleanup;
        }
    }

    pBuffer1 = Executive_Alloc( original_size, fOriginalZeroInit);
    if ( (NULL == pBuffer1) && (original_size > 0) )
    {
        TF_Printf("Executive Unit Test: Unable to allocate a chunk of memory in the size: %d \n", original_size);
        retval = pkE_FAIL;
        goto exit;
    }

    else if ( 0 == original_size )
    {
        if ( NULL == pBuffer1)
        {
            TF_Printf("Executive Unit Test: Memory allocation for a block of size 0 failed.\n");
            retval = pkE_FAIL;
            goto exit;
        }
    }

    //Verify zero fill, if applicable
    if ( TRUE == fOriginalZeroInit)
    {
        if ( FALSE == VerifyZeroFill(pBuffer1, original_size) )
        {
            TF_Printf("Executive Unit Test: TEST FAILED - allocated memory not zero filled.\n");
            retval = pkE_FAIL;
            goto cleanup;
        }
    }

    //Test case failure for the next step will be detected in the form of a segmentation fault
    if ( FALSE == FillandVerifyMemory(pBuffer1, original_size, FILL_AND_VERIFY, &g_FillPattern1) )
    {
        retval = pkE_FAIL;
        TF_Printf("Executive Unit Test: TEST FAILED - Unable to fill and verify allocated memory.\n");
        goto cleanup;
    }

    //Original allocation confirmed, now realloc it to a smaller size
    pBuffer2 = Executive_ReAlloc( pBuffer1, new_size, fNewZeroInit);
    if (  (NULL == pBuffer2) && ( new_size > 0 ) )
    {
        TF_Printf("Executive Unit Test: Unable to allocate a chunk of memory in the size: %d \n", new_size);
        retval = pkE_FAIL;
        goto cleanup;
    }
    else if ( 0 == new_size )
    {
        if ( NULL == pBuffer2)
        {
            TF_Printf("Executive Unit Test: Memory ReAllocation for a block of size 0 failed.\n");
            retval = pkE_FAIL;
            goto cleanup;
        }
        else
        {
            retval = pkS_OK;
            pBuffer1 = NULL; //Avoid a double free on cleanup
            goto cleanup; //The ReAlloc succeeded, but trying to fill or verify a block of size 0 is pointless
        }
    }

    pBuffer1 = NULL; //Avoid a double free on cleanup

    //Verify zero fill, if applicable
    if ( TRUE == fNewZeroInit)
    {
        if ( FALSE == VerifyZeroFill(pBuffer2, new_size) )
        {
            TF_Printf("Executive Unit Test: TEST FAILED - allocated memory not zero filled.\n");
            retval = pkE_FAIL;
            goto cleanup;
        }
    }
    else
    {
        if ( new_size < original_size)
        {
            verifySize = new_size;
        }
        else
        {
            verifySize = original_size;
        }

        //Verify the original memory was preserved to the extent possible
        if ( FALSE == FillandVerifyMemory(pBuffer2, verifySize, VERIFY_ONLY, &g_FillPattern1) )
        {
            TF_Printf("Executive Unit Test: TEST FAILED - original data not preserved.\n");
            retval = pkE_FAIL;
            goto cleanup;
        }
    }

    //Fill and verify the whole contents
    if ( FALSE == FillandVerifyMemory(pBuffer2, new_size, FILL_AND_VERIFY, &g_FillPattern2) )
    {
        TF_Printf("Executive Unit Test: TEST FAILED - Unable to write and verify new size.\n");
        retval = pkE_FAIL;
        goto cleanup;
    }


    retval = pkS_OK;

cleanup:
    if ( NULL != pBuffer1)
    {
        Executive_Free( pBuffer1 );
        pBuffer1 = NULL;
    }

    if ( NULL != pBuffer2)
    {
        Executive_Free( pBuffer2 );
        pBuffer2 = NULL;
    }

    //Make sure there were no leaks during testing
    hr = Debug_GetMemoryUsed( &memoryAfter );
    if ( pkFAILED(hr) )
    {
        if ( pkE_NOTIMPL != hr)
        {
            TF_Printf("Executive Unit Test: Verifying memory allocation amount failed with error: %x\n", hr);
            retval = pkE_UNEXPECTED;
        }
    }
    else
    {
        if ( memoryAfter != memoryBefore )
        {
            TF_Printf("Executive Unit Test: TEST FAILED - Expected no change from original allocated memory after Free, actual change was %d\n", (memoryAfter - memoryBefore));
            retval = pkE_UNEXPECTED;
        }
    }


exit:
    return retval;

}

pkRESULT NullReAllocAndVerifyMemory(uint32_t size, bool_t fZeroInit)
{
    uint32_t memoryBefore, memoryAfter;
    pkRESULT retval, hr;
    void *pBuffer1 = NULL;

    retval = pkE_FAIL;

    hr = Debug_GetMemoryUsed( &memoryBefore );

    if ( pkFAILED(hr) )
    {
        if ( pkE_NOTIMPL == hr)
        {
            TF_Printf("Executive Unit Test: WARNING - Debug_GetMemoryUsed not implemented, some verification could not be completed.\n\n");
        }
        else
        {
            TF_Printf("Executive Unit Test: Verifying memory allocation amount failed with error: %x\n", hr);
            goto cleanup;
        }
    }

    pBuffer1 = Executive_ReAlloc( NULL, size, fZeroInit);
    if ( ( NULL == pBuffer1 ) && ( size > 0 ))
    {
        TF_Printf("Executive Unit Test: Unable to allocate a chunk of memory in the size: %d \n", size);
        goto exit;
    }

    else if ( 0 == size )
    {
        if ( NULL == pBuffer1)
        {
            TF_Printf("Executive Unit Test: Memory ReAllocation for a block of size 0 failed.\n");
            retval = pkE_FAIL;
            goto exit;
        }
        else
        {
            retval = pkS_OK;
            goto cleanup; //The ReAlloc succeeded, but trying to fill or verify a block of size 0 is pointless
        }
    }

    //Verify zero fill, if applicable
    if ( TRUE == fZeroInit)
    {
        if ( FALSE == VerifyZeroFill(pBuffer1, size) )
        {
            TF_Printf("Executive Unit Test: TEST FAILED - allocated memory not zero filled.\n");
            retval = pkE_FAIL;
            goto cleanup;
        }
    }

    //Test case failure for the next step will be detected in the form of a segmentation fault
    if ( FALSE == FillandVerifyMemory(pBuffer1, size, FILL_AND_VERIFY, &g_FillPattern1) )
    {
        TF_Printf("Executive Unit Test: TEST FAILED - Unable to fill and verify allocated memory.\n");
        retval = pkE_FAIL;
        goto cleanup;
    }

    retval = pkS_OK;

cleanup:
    if ( NULL != pBuffer1 )
    {
        Executive_Free( pBuffer1 );
        pBuffer1 = NULL;
    }

    //Make sure there were no leaks during testing
    hr = Debug_GetMemoryUsed( &memoryAfter );
    if ( pkFAILED(hr) )
    {
        if ( pkE_NOTIMPL != hr)
        {
            TF_Printf("Executive Unit Test: Verifying memory allocation amount failed with error: %x\n", hr);
            retval = pkE_UNEXPECTED;
        }
    }
    else
    {
        if ( memoryAfter != memoryBefore )
        {
            TF_Printf("Executive Unit Test: TEST FAILED - Expected no change from original allocated memory after Free, actual change was %d\n", (memoryAfter - memoryBefore));
            retval = pkE_UNEXPECTED;
        }
    }

exit:

    return retval;
}
