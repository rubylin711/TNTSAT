///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

//
// Executive_UnitTest_Interlocked.c: Defines the entry point for the sample Media Center
// Extender Platform. Unit tests for the Executive PAL Interlocked implementation.
//
/* Test the following APIs:
//-----------------------------------------------------------------------------
// InterLocked Operations
//-----------------------------------------------------------------------------
int32_t pkAPI Executive_InterlockedIncrement(
            int32_t volatile* lpAddend);

int32_t pkAPI Executive_InterlockedDecrement(
            int32_t volatile* lpAddend);

int32_t pkAPI Executive_InterlockedExchange(
            int32_t volatile* Target,
            int32_t Value);

int32_t pkAPI Executive_InterlockedExchangeAdd(
            int32_t volatile* Addend,
            int32_t Value);

int32_t pkAPI Executive_InterlockedCompareExchange(
            int32_t volatile* Destination,
            int32_t Exchange,
            int32_t Comperand);

void *  pkAPI Executive_InterlockedExchangePointer(
            void * volatile* pTarget,
            void *pValue);

void *  pkAPI Executive_InterlockedCompareExchangePointer(
            void * volatile* Destination,
            void * Exchange,
            void * Comperand);
*/

//=============================================================================
// I N C L U D E     F I L E S
//=============================================================================
#include "Executive_UnitTest.h"


//=============================================================================
// C O N S T A N T     D E F I N I T I O N S
//=============================================================================
volatile int32_t g_testVariable1=14;
volatile int32_t g_testVariable2=15;
void * volatile * g_testPointer1=NULL;
void * volatile * g_testPointer2=NULL;

//=============================================================================
// L O C A L     F U N C T I O N     P R O T O T Y P E S
//=============================================================================

pkRESULT ExecTest_InterlockedIncrement(int32_t volatile* variable, int32_t originalValue, int32_t iterations);
pkRESULT ExecTest_InterlockedDecrement(int32_t volatile* variable, int32_t originalValue, int32_t iterations);
pkRESULT ExecTest_InterlockedExchange( int32_t volatile* variable, int32_t originalValue, int32_t newValue);
pkRESULT ExecTest_InterlockedExchangeAdd( int32_t volatile* variable, int32_t originalValue, int32_t changeAmount);
pkRESULT ExecTest_InterlockedCompareExchange(int32_t volatile* variable, int32_t originalValue, int32_t newValue, int32_t comparand);
pkRESULT ExecTest_InterlockedExchangePointer( void* volatile* pTarget, void* pOriginalPointer, void *pNewPointer );
pkRESULT ExecTest_InterlockedCompareExchangePointer(void * volatile * pTarget, void* pOriginalPointer, void *pNewPointer, void* pComparand);


//=============================================================================
// P U B L I C    F U N C T I O N    D E F I N I T I O N S
//=============================================================================
pkRESULT ExecTest_InterlockedBasicTest(void)
{
    pkRESULT retval=pkE_FAIL, hr;
    void *pOriginalPointer=NULL, *pNewPointer=NULL, *pOtherPointer=NULL, *pComparand=NULL;
    int32_t originalValue, newValue, iterations, changeAmount, comparand;
    int32_t volatile *variable=NULL;
    void * volatile * pTarget=NULL;

    uint32_t casesPassed = 0, casesFailed = 0;

    variable = &g_testVariable1;
    pTarget = g_testPointer1;

    //Case Number 20
    //Basic Test
    //Functional Group: Interlocked
    //Test Title: Use InterlockedExchange to change the value of a variable
    if ( TRUE == g_bIsAborted)
    {
        goto exit;
    }
    originalValue = -2046;
    g_testVariable1 = -2046;
    newValue = 3;
    hr = ExecTest_InterlockedExchange( variable, originalValue, newValue);
    if ( pkSUCCEEDED(hr))
    {
        casesPassed++;
    }
    else
    {
        TF_Printf("Executive Unit Test: Test case 20 failed with error %x\n", hr);
        casesFailed++;
    }

    //Case Number 21
    //Basic Test
    //Functional Group: Interlocked
    //Test Title: Use InterlockedCompareExchange with a Comparand that matches the dest value to exchange
    if ( TRUE == g_bIsAborted)
    {
        goto exit;
    }
    originalValue = comparand = g_testVariable1 = 8;
    newValue = 12000;
    hr = ExecTest_InterlockedCompareExchange( variable, originalValue, newValue, comparand);
    if ( pkSUCCEEDED(hr))
    {
        casesPassed++;
    }
    else
    {
        TF_Printf("Executive Unit Test: Test case 21 failed with error %x\n", hr);
        casesFailed++;
    }


    //Case Number 22
    //Basic Test
    //Functional Group: Interlocked
    //Test Title: Use InterlockedCompareExchange with a Comparand that doesn't match the target value and verify no exchange
    if ( TRUE == g_bIsAborted)
    {
        goto exit;
    }
    originalValue = g_testVariable1 = -3001;
    newValue = comparand = 4;
    hr = ExecTest_InterlockedCompareExchange( variable, originalValue, newValue, comparand);
    if ( pkSUCCEEDED(hr))
    {
        casesPassed++;
    }
    else
    {
        TF_Printf("Executive Unit Test: Test case 22 failed with error %x\n", hr);
        casesFailed++;
    }

    //Case Number 23
    //Basic Test
    //Functional Group: Interlocked
    //Test Title: Use InterlockedExchangeAdd to add a positive number to a variable
    if ( TRUE == g_bIsAborted)
    {
        goto exit;
    }
    originalValue = g_testVariable1 = 43;
    changeAmount = 76;
    hr= ExecTest_InterlockedExchangeAdd( variable, originalValue, changeAmount);
    if ( pkSUCCEEDED(hr))
    {
        casesPassed++;
    }
    else
    {
        TF_Printf("Executive Unit Test: Test case 23 failed with error %x\n", hr);
        casesFailed++;
    }

    //Case Number 24
    //Basic Test
    //Functional Group: Interlocked
    //Test Title: Use InterlockedExchangeAdd to add a negative number to a variable
    if ( TRUE == g_bIsAborted)
    {
        goto exit;
    }
    originalValue = g_testVariable1 = 43;
    changeAmount = -76;
    hr= ExecTest_InterlockedExchangeAdd( variable, originalValue, changeAmount);
    if ( pkSUCCEEDED(hr))
    {
        casesPassed++;
    }
    else
    {
        TF_Printf("Executive Unit Test: Test case 24 failed with error %x\n", hr);
        casesFailed++;
    }

    //Case Number 25
    //Basic Test
    //Functional Group: Interlocked
    //Test Title: Use InterlockedIncrement to increment a variable
    if ( TRUE == g_bIsAborted)
    {
        goto exit;
    }
    originalValue = g_testVariable1 = -15;
    iterations = 1;
    hr= ExecTest_InterlockedIncrement(variable, originalValue, iterations);
    if ( pkSUCCEEDED(hr))
    {
        casesPassed++;
    }
    else
    {
        TF_Printf("Executive Unit Test: Test case 25 failed with error %x\n", hr);
        casesFailed++;
    }

    //Case Number 26
    //Basic Test
    //Functional Group: Interlocked
    //Test Title: Use InterlockedDecrement to decrement a variable
    if ( TRUE == g_bIsAborted)
    {
        goto exit;
    }
    originalValue = g_testVariable1 = -15;
    iterations = 1;
    hr= ExecTest_InterlockedDecrement(variable, originalValue, iterations);
    if ( pkSUCCEEDED(hr))
    {
        casesPassed++;
    }
    else
    {
        TF_Printf("Executive Unit Test: Test case 26 failed with error %x\n", hr);
        casesFailed++;
    }

    //Case Number 27
    //Basic Test
    //Functional Group: Interlocked
    //Test Title: Use InterlockedExchangePointer to set a pointer value
    if ( TRUE == g_bIsAborted)
    {
        goto exit;
    }
    pOriginalPointer = (void*) &g_testVariable2;
    pNewPointer = (void*) &g_testVariable1;
    g_testVariable1 = -1572;
    g_testVariable2 = 312;
    pTarget = &pOriginalPointer;

    hr = ExecTest_InterlockedExchangePointer( pTarget, pOriginalPointer, pNewPointer);
    if ( pkSUCCEEDED(hr))
    {
        if (  g_testVariable1 == **((int32_t**)(pTarget)) )
        {
            casesPassed++;
        }
        else
        {
            TF_Printf("Executive Unit Test: Test case 27 failed due to Exchange not working. Expected value of %d, actual value was %d\n", g_testVariable1,*((int32_t*)(pTarget)) );
            casesFailed++;
        }
    }
    else
    {
        TF_Printf("Executive Unit Test: Test case 27 failed with error %x\n", hr);
        casesFailed++;
    }

    //Case Number 28
    //Basic Test
    //Functional Group: Interlocked
    //Test Title: Use InterlockedCompareExchangePointer with a Comparand that matches the dest pointer to exchange
    if ( TRUE == g_bIsAborted)
    {
        goto exit;
    }
    pOriginalPointer = pComparand = (void*) &g_testVariable2;
    pNewPointer = (void*) &g_testVariable1;
    g_testVariable1 = 5176;
    g_testVariable2 = -4000;
    pTarget = &pOriginalPointer;

    hr = ExecTest_InterlockedCompareExchangePointer( pTarget, pOriginalPointer, pNewPointer, pComparand);
    if ( pkSUCCEEDED(hr))
    {
        if (  g_testVariable1 == **((int32_t**)(pTarget)) )
        {
            casesPassed++;
        }
        else
        {
            TF_Printf("Executive Unit Test: Test case 28 failed due to Exchange not working. Expected value of %d, actual value was %d\n", g_testVariable1, *((int32_t*)(pTarget)) );
            casesFailed++;
        }
    }
    else
    {
        TF_Printf("Executive Unit Test: Test case 28 failed with error %x\n", hr);
        casesFailed++;
    }



    //Case Number 5
    //Basic Test
    //Functional Group: Interlocked
    //Test Title: Use InterlockedCompareExchangePointer with a Comparand that doesn't match the dest pointer to exchange
    if ( TRUE == g_bIsAborted)
    {
        goto exit;
    }
    pOriginalPointer = (void*) &g_testVariable2;
    pNewPointer = pComparand = (void*) &g_testVariable1;
    g_testVariable1 = 15672;
    g_testVariable2 = -1;
    pTarget = &pOriginalPointer;

    hr = ExecTest_InterlockedCompareExchangePointer( pTarget, pOriginalPointer, pNewPointer, pComparand);
    if ( pkSUCCEEDED(hr))
    {
        if (  g_testVariable2 == **((int32_t**)(pTarget)) )
        {
            casesPassed++;
        }
        else
        {
            TF_Printf("Executive Unit Test: Test case 5 failed due to Exchange not working. Expected value of %d, actual value was %d\n", g_testVariable2, *((int32_t*)(pTarget)) );
            casesFailed++;
        }
    }
    else
    {
        TF_Printf("Executive Unit Test: Test case 5 failed with error %x\n", hr);
        casesFailed++;
    }


exit:
    TF_Printf("*************************************\n");
    TF_Printf("Executive Unit Test: Basic Interlocked Test Results\n");
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

    if ( TRUE == g_bIsAborted)
    {
        retval = pkE_ABORT;
    }
    return retval;
}

pkRESULT ExecTest_InterlockedExtendedTest(void)
{
    uint32_t casesPassed = 0, casesFailed = 0;
    pkRESULT retval = pkE_FAIL;

    TF_Printf("*************************************\n");
    TF_Printf("Executive Unit Test: Extended Interlocked Test Results\n");
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

    if ( TRUE == g_bIsAborted)
    {
        retval = pkE_ABORT;
    }
    return retval;
}


//=============================================================================
// L O C A L    F U N C T I O N    D E F I N I T I O N S
//=============================================================================
pkRESULT ExecTest_InterlockedIncrement(int32_t volatile* variable, int32_t originalValue, int32_t iterations)
{
    int32_t result, i;

    if (0 >= iterations)
    {
        return pkE_INVALIDARG;
    }

    for (i = 0; i < iterations; i++)
    {
        result = Executive_InterlockedIncrement(variable);
    }

    if ( result != (originalValue + iterations) )
    {
        TF_Printf("Executive Unit Test: TEST FAIL - Expected value %d after %d iterations. Actual value: %d\n", originalValue +iterations, iterations, result);
        return pkE_FAIL;
    }
    else
    {
        return pkS_OK;
    }

}

pkRESULT ExecTest_InterlockedDecrement(int32_t volatile* variable, int32_t originalValue, int32_t iterations)
{
    int32_t result, i;

    if (0 >= iterations)
    {
        return pkE_INVALIDARG;
    }

    for (i = 0; i < iterations; i++)
    {
        result = Executive_InterlockedDecrement(variable);
    }

    if ( result != (originalValue - iterations) )
    {
        TF_Printf("Executive Unit Test: TEST FAIL - Expected value %d after %d iterations. Actual value: %d\n", originalValue +iterations, iterations, result);
        return pkE_FAIL;
    }
    else
    {
        return pkS_OK;
    }

}

pkRESULT ExecTest_InterlockedExchange( int32_t volatile* variable, int32_t originalValue, int32_t newValue)
{
    int32_t result;

    result = Executive_InterlockedExchange(variable, newValue);

    if ( result != originalValue )
    {
        TF_Printf("Executive Unit Test: TEST FAIL - Executive_InterlockedExchange returned an unexpected value.  Expected: %d, Actual: %d\n", originalValue, result);
        return pkE_FAIL;
    }

    if ( *variable != newValue)
    {
        TF_Printf("Executive Unit Test: TEST FAIL - Executive_InterlockedExchange did not properly exchange.  Result was %d, expected %d\n", *variable, newValue);
        return pkE_FAIL;
    }

    return pkS_OK;
}

pkRESULT ExecTest_InterlockedExchangeAdd( int32_t volatile* variable, int32_t originalValue, int32_t changeAmount)
{
    int32_t result;

    result = Executive_InterlockedExchangeAdd(variable, changeAmount);

    if ( result != originalValue )
    {
        TF_Printf("Executive Unit Test: TEST FAIL - Executive_InterlockedExchangeAdd returned an unexpected value.  Expected: %d, Actual: %d\n", originalValue, result);
        return pkE_FAIL;
    }

    if ( *variable != originalValue + changeAmount)
    {
        TF_Printf("Executive Unit Test: TEST FAIL - Executive_InterlockedExchangeAdd did not properly add.  Result was %d, expected %d\n", *variable, originalValue + changeAmount);
        return pkE_FAIL;
    }

    return pkS_OK;
}

pkRESULT ExecTest_InterlockedCompareExchange(int32_t volatile* variable, int32_t originalValue, int32_t newValue, int32_t comparand)
{
    int32_t result, expected;

    result = Executive_InterlockedCompareExchange(variable, newValue, comparand);

    if (comparand == originalValue)
    {
        expected = newValue;
    }
    else
    {
        expected = originalValue;
    }

   if ( result != originalValue )
    {
        TF_Printf("Executive Unit Test: TEST FAIL - Executive_InterlockedCompareExchange returned an unexpected value. Expected: %d, Actual: %d\n", originalValue, result);
        return pkE_FAIL;
    }

    if ( *variable != expected )
    {
        TF_Printf("Executive Unit Test: TEST FAIL - Executive_InterlockedCompareExchange did not properly exchange.  Result was %d, expected %d\n", *variable, expected);
        return pkE_FAIL;
    }

    return pkS_OK;

}

pkRESULT ExecTest_InterlockedExchangePointer( void* volatile* pTarget, void* pOriginalPointer, void *pNewPointer)
{
    void *pResult = NULL;

    pResult = Executive_InterlockedExchangePointer(pTarget, pNewPointer);

    if ( pResult != pOriginalPointer )
    {
        TF_Printf("Executive Unit Test: TEST FAIL - Executive_InterlockedExchangePointer did not return the original pointer. Expected address: %d, Actual address %d\n", pOriginalPointer, pResult);
        return pkE_FAIL;
    }

    if ( *pTarget != pNewPointer)
    {
        TF_Printf("Executive Unit Test: TEST FAIL - Executive_InterlockedExchangePointer did not properly exchange.\n");
        return pkE_FAIL;
    }

    return pkS_OK;
}

pkRESULT ExecTest_InterlockedCompareExchangePointer(void* volatile* pTarget, void* pOriginalPointer, void *pNewPointer, void* pComparand)
{
    void *pResult = NULL, *pExpected = NULL;

    pResult = Executive_InterlockedCompareExchangePointer(pTarget, pNewPointer, pComparand);

    if ( pOriginalPointer == pComparand )
    {
        pExpected = pNewPointer;
    }
    else
    {
        pExpected = pOriginalPointer;
    }

    if ( pResult != pOriginalPointer )
    {
        TF_Printf("Executive Unit Test: TEST FAIL - Executive_InterlockedCompareExchangePointer did not return the original pointer. Expected address: %d, Actual address %d\n", pOriginalPointer, pResult);
        return pkE_FAIL;
    }

    if ( *pTarget != pExpected)
    {
        TF_Printf("Executive Unit Test: TEST FAIL - Executive_InterlockedCompareExchangePointer did not properly exchange. Expected address: %d, Actual address %d\n", pExpected, *pTarget);
        return pkE_FAIL;
    }

    return pkS_OK;

}

