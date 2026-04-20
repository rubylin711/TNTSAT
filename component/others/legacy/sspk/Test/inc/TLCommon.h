///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __TLCOMMON_H_
#define __TLCOMMON_H_

//===========================================================================
// cmdLineArgs.c
//===========================================================================
#ifdef __cplusplus
extern "C" {
#endif

void TLC_SetCmdLineArgs(int argc, char **argv);
void TLC_ClearCmdLineArgs(void);
pkRESULT TLC_GetCmdLineArg(const char *pszSwitch, int iPos, char **ppArg);
pkRESULT TLC_GetCmdLineArgInt(const char *pszSwitch, int iPos, int *piArg);
bool_t TLC_IsHelpRequested(bool_t *pfVerbose);
bool_t TLC_CmdLine_IsCPUUtilizationRequested(void);

pkRESULT TLC_GetCmdLineVideoGraphicsConfig(TF_Config *pConfig);
const char* TLC_GetCmdLineVideoGraphicsConfigUsage(void);

#ifdef __cplusplus
}
#endif

//===========================================================================
// fileUtils.cpp
//===========================================================================

// This header may be included by *.c files, in which case we must exclude C++ declarations
#ifdef __cplusplus

class ITLCFile
{
public:
    virtual ~ITLCFile();
    virtual pkRESULT FOpen(const char *pszFilename, const char *pszMode) = 0;
    virtual pkRESULT FClose(void) = 0;
    virtual pkRESULT GetFileSize(size_t *piSize) = 0;
    virtual pkRESULT FRead(size_t *pcItemsRead, void *pbBuffer, size_t iSize,
        size_t iCount) = 0;
    virtual pkRESULT FSeek(int32_t offset, TF_TestVectors_SeekWhence whence) = 0;
};

typedef enum
{
    RW_READONLY,
    RW_READWRITE,
} TLC_FILE_RWMODE;

typedef struct TLC_FILE_EMBEDDED
{
    char *pszEmbedFilename; // Including the embedded:/// prefix (URISCHEME_EMBEDDEDFILE)
    TLC_FILE_RWMODE rw;
    uint8_t *pbFileData;
    const uint32_t *pcbFileData;
} TLC_FILE_EMBEDDED;

#define URISCHEME_EMBEDDEDFILE  "embedded:///"

pkRESULT TLC_AddEmbeddedFiles(TLC_FILE_EMBEDDED *rgEmbeddedFiles, int cEmbeddedFiles);
pkRESULT TLC_CreateFile(const char *pszFilename, ITLCFile **ppITLCFile);

#endif // __cplusplus

//===========================================================================
// CPUUtilization.c
//===========================================================================
#ifdef __cplusplus
extern "C" {
#endif

typedef void (*PFNCPUUTILNOTIFICATION)(void *pCPUUtilNotificationContext);

pkRESULT TLC_CPUUtilization_Start(uint32_t uiPollingIntervalInMS,
                                  bool_t fPrintCPUUtilization,
                                  PFNCPUUTILNOTIFICATION pfnCPUUtilNotification,
                                  void *pCPUUtilNotificationContext);
pkRESULT TLC_CPUUtilization_Stop(void);
pkRESULT TLC_CPUUtilization_GetPercent(uint32_t *puiCPUUtilization);
const char *TLC_GetCmdLineCPUUtilization(void);

#ifdef __cplusplus
}
#endif

//===========================================================================
// loggingWrapper.c
//===========================================================================
#ifdef __cplusplus
extern "C" {
#endif

pkRESULT TLC_LogUnitTestBegin
    (
        pkHANDLE hLog,
        const char *szUnitTestName,
        const char *szSuitesSelected,
        const char *szTestPassType
    );

pkRESULT TLC_LogUnitTestSummary
    (
        pkHANDLE hLog,
        const char *szUnitTestName,
        const char *szSuiteName,
        const char *szTestPassType,
        uint32_t* passCount,
        uint32_t* failCount,
        uint32_t* skipCount
     );

pkRESULT TLC_LogTestGroupBegin
    (
        pkHANDLE hLog,
        const char *szUnitTestName,
        const char *szFunctionalGroupName
     );

pkRESULT TLC_LogTestGroupEnd
    (
        pkHANDLE hLog,
        const char *szUnitTestName,
        const char *szFunctionalGroupName,
        uint32_t* passCount,
        uint32_t* failCount,
        uint32_t* skipCount
    );

pkRESULT TLC_LogTestCasePass
    (
        pkHANDLE hLog,
        const char *szUnitTestName,
        const char *szTestCaseName,
        uint32_t testCaseNumber
    );

pkRESULT TLC_LogTestCaseFail
    (
        pkHANDLE hLog,
        const char *szUnitTestName,
        const char *szTestCaseName,
        uint32_t testCaseNumber,
        const char *szFailureReason,
        pkRESULT FailureCode
    );

pkRESULT TLC_LogTestCaseSkip
    (
        pkHANDLE hLog,
        const char *szUnitTestName,
        const char *szTestCaseName,
        uint32_t testCaseNumber,
        const char *szSkipReason
    );

pkRESULT TLC_LogWarning
    (
        pkHANDLE hLog,
        const char *szUnitTestName,
        const char *szWarningText
    );

pkRESULT TLC_LogVerbose
    (
        pkHANDLE hLog,
        const char *szUnitTestName,
        const char *szFormatString,
        ...
    );

void TLC_SetVerboseLogging
    (
        bool_t LoggingSetting
    );

void TLC_SetLogToFile
    (
        bool_t UseLogFile
    );

#ifdef __cplusplus
}
#endif

#endif // __TLCOMMON_H_
