///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

// Constants
const int VectorSize = 2048;
const int HttpRequestLength = 1024;
const int TestPurposeStringLength = 512;
const int TestCriteriaStringLength = 512;

// position of individual components in the vector
const int VectorNumberPosition = 0;
const int TestTypePosition = 3;
const int ResponseLengthPosition = 6;

const int TestVectorNumberLength = 2;
const int TestTypeBufferLength = 2;
const int ResponseLength = 10;

// Data Structures
struct tTestVector
{
    char HttpRequest[HttpRequestLength];
    char TestPurpose[TestPurposeStringLength];
    char TestCriteria[TestCriteriaStringLength];
};

struct tTestCriteria
{
    int VectorNumber;
    // TestType  00 Invalid 01 Text-Xml 02 Video-MP4 03 Audio-MP4
    int TestType;
    // bits 6-10
    int Length;
    // File Size    ABCDEFGHIJ => ABCDEFGHIJ bytes
};


void ParseVector(uint8_t * Vector, tTestVector * testVector, tTestCriteria * testCriteria);

pkRESULT ParseScript(pkHANDLE hLog, char * pScriptBuf, uint8_t * TestVector);

bool LaunchStreamer(pkHANDLE hLog, uint8_t * TestVector);


