///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h" // includes pkPAL.h
#include "MP4Info.h"
#include "StreamerHelper.h"
#include "StreamerTest.h"

#include <pkTestFramework.h>

void ParseVector(uint8_t * Vector, tTestVector * testVector, tTestCriteria * testCriteria)
{

    char TestCriteria[TestCriteriaStringLength];

    // HTTP Request
    for (int i = 0; i < HttpRequestLength; i++)
    {
        testVector->HttpRequest[i] = *Vector++;
    }

    // Test Purpose
    for (int i = HttpRequestLength; i < HttpRequestLength+TestPurposeStringLength; i++)
    {
        testVector->TestPurpose[i-HttpRequestLength] = *Vector++;
    }

    // Test Criteria
    for (int i = HttpRequestLength+TestPurposeStringLength; i < VectorSize; i++)
    {
        testVector->TestCriteria[i-HttpRequestLength-TestPurposeStringLength] = *Vector;
        TestCriteria[i-HttpRequestLength-TestPurposeStringLength] = *Vector++;
    }

    char TestVectorNumberBuffer[TestVectorNumberLength];
    char TestTypeBuffer[TestTypeBufferLength];
    char TestCriteriaBuffer[ResponseLength];

    TestVectorNumberBuffer[0] = TestCriteria[0];
    TestVectorNumberBuffer[1] = TestCriteria[1];

    testCriteria->VectorNumber = atoi(TestVectorNumberBuffer);

    TestTypeBuffer[0] = TestCriteria[TestTypePosition];
    TestTypeBuffer[1] = TestCriteria[TestTypePosition+1];

    testCriteria->TestType = atoi(TestTypeBuffer);

    TestCriteriaBuffer[0] = TestCriteria[ResponseLengthPosition];
    TestCriteriaBuffer[1] = TestCriteria[ResponseLengthPosition+1];
    TestCriteriaBuffer[2] = TestCriteria[ResponseLengthPosition+2];
    TestCriteriaBuffer[3] = TestCriteria[ResponseLengthPosition+3];
    TestCriteriaBuffer[4] = TestCriteria[ResponseLengthPosition+4];
    TestCriteriaBuffer[5] = TestCriteria[ResponseLengthPosition+5];
    TestCriteriaBuffer[6] = TestCriteria[ResponseLengthPosition+6];
    TestCriteriaBuffer[7] = TestCriteria[ResponseLengthPosition+7];
    TestCriteriaBuffer[8] = TestCriteria[ResponseLengthPosition+8];
    TestCriteriaBuffer[9] = TestCriteria[ResponseLengthPosition+9];

    testCriteria->Length = atoi(TestCriteriaBuffer);

}

pkRESULT ParseScript(pkHANDLE hLog, char * pScriptBuf, uint8_t * TestVector)
{
    pkRESULT result = pkS_OK;

    if (strcmp(pScriptBuf, "streamerunit"))
    {
        if( !LaunchStreamer(hLog, TestVector) )
        {
            result = pkE_FAIL;
        }
    }

    return result;
}

bool LaunchStreamer(pkHANDLE hLog, uint8_t * VectorData)
{
    bool result = false;
    pkRESULT pkresult;
    tTestVector CurrentTestVector;
    tTestCriteria CurrentTestCriteria;

    ParseVector(VectorData, &CurrentTestVector, &CurrentTestCriteria);

    // Log the Test
    pkresult = TF_Logging_Printf(hLog, "***************** UNIT TEST STARTED ********* \n");
    pkresult = TF_Printf("***************** UNIT TEST STARTED ********* \n");
    pkresult = TF_Logging_Printf(hLog, "PURPOSE: %s \n", CurrentTestVector.TestPurpose);
    pkresult = TF_Printf("PURPOSE: %s \n", CurrentTestVector.TestPurpose);

    StreamerTest CurrentStreamerTest = StreamerTest(hLog);
    result = CurrentStreamerTest.Init(CurrentTestVector.HttpRequest);

    result = CurrentStreamerTest.VerifyStreaming(CurrentTestVector.HttpRequest);

    if (result == true)
    {
        result = CurrentStreamerTest.DecidePassOrFail(CurrentTestCriteria.Length, CurrentTestCriteria.TestType);

        pkresult = TF_Logging_Printf(hLog, "Test Passed \n");
    }
    else
    {
        if ((result == false) && (CurrentTestCriteria.TestType == 0))
        {
            pkresult = TF_Logging_Printf(hLog, "Test Passed \n");
        }
        else
        {
            pkresult = TF_Logging_Printf(hLog, "Test Failed \n");
        }
    }

    // Parse the Test Criteria
    pkresult = TF_Logging_Printf(hLog, "***************** UNIT TEST ENDED ********* \n");

    return result;
}

