///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

//
// Executive_UnitTest.h :  Defines the entry point for the sample Media Center Extender Platform
//                         Unit tests for the Executive PAL implementation.
//
#include <pkPAL.h>
#include <pkExecutive.h>
#include <pkTestFramework.h>
#include <TLCommon.h>

#ifndef __EXECUTIVE_UNITTESTS_H_
#define __EXECUTIVE_UNITTESTS_H_

#define GOTO_ERR { s_nLine=__LINE__;goto exit; }
#define ACCEPTABLE_SKEW_MS 10

#ifdef __cplusplus
extern "C" {
#endif

extern bool_t g_bIsAborted;
extern pkRESULT ExecTest_EventBasicTest(void);
extern pkRESULT ExecTest_EventExtendedTest(void);
extern pkRESULT ExecTest_InterlockedBasicTest(void);
extern pkRESULT ExecTest_InterlockedExtendedTest(void);
extern pkRESULT ExecTest_LockBasicTest(void);
extern pkRESULT ExecTest_LockExtendedTest(void);
extern pkRESULT ExecTest_MemLeakTest(void);
extern pkRESULT ExecTest_MemoryBasicTest(void);
extern pkRESULT ExecTest_MemoryExtendedTest(void);
extern pkRESULT ExecTest_MiscBasicTest(void);
extern pkRESULT ExecTest_MiscExtendedTest(void);
extern pkRESULT ExecTest_SemaphoreBasicTest(void);
extern pkRESULT ExecTest_SemaphoreExtendedTest(void);
extern pkRESULT ExecTest_ThreadBasicTest(void);
extern pkRESULT ExecTest_ThreadExtendedTest(void);

#ifdef __cplusplus
}
#endif

#endif // __EXECUTIVE_UNITTESTS_H_
