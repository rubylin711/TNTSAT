///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

//===========================================================================
// Includes
//===========================================================================
#include <pkPAL.h>
#include <pkExecutive.h>
#include <pkTestFramework.h>
#include <TLCommon.h>

#include <errTrace.h>


//===========================================================================
// Data Types
//===========================================================================


//===========================================================================
// Globals
//===========================================================================
static bool_t g_fExitIdlerThread = FALSE;
static pkHANDLE g_hIdlerThread = NULL;
static uint32_t g_uiUpdateIntervalInMS = 0;
static PFNCPUUTILNOTIFICATION g_pfnCPUUtilNotification = NULL;
static void *g_pCPUUtilNotificationContext = NULL;
static uint32_t g_uiCpuUtilizationPercent = 0;
static bool_t g_fPrintCPUUtilization = FALSE;

//===========================================================================
// Private Functions
//===========================================================================

static uint32_t pkAPI CPUUtilizationThread(void* param)
{
    uint64_t timeLast;
    float maxIdleLoad = 0.0001F;
    uint64_t idleCount = 0;
    uint64_t uiUpdateInterval;

    uiUpdateInterval = (uint64_t)(g_uiUpdateIntervalInMS *
        Executive_GetPerformanceFrequency() / 1000.0F + 0.5F);
    if (uiUpdateInterval <= 0)
        uiUpdateInterval = 1; // Give us a chance at least for 1 or more iterations

    timeLast = Executive_GetPerformanceCounter();
    while (!g_fExitIdlerThread)
    {
        uint64_t timeNow;
        timeNow = Executive_GetPerformanceCounter();

        // Wait until update interval to compute CPU utilization
        if (timeNow - timeLast >= uiUpdateInterval)
        {
            float idleLoad;

            idleLoad = (float)(idleCount) / (timeNow - timeLast);
            if ( idleLoad > maxIdleLoad )
                maxIdleLoad = idleLoad;

            g_uiCpuUtilizationPercent = (uint32_t)(((maxIdleLoad - idleLoad) * 100)/maxIdleLoad);
            if (NULL != g_pfnCPUUtilNotification)
            {
                g_pfnCPUUtilNotification(g_pCPUUtilNotificationContext);
            }


            if (g_fPrintCPUUtilization)
            {
                TF_Printf("Proc%%: %u, %f, %llu, %f\n", g_uiCpuUtilizationPercent, idleLoad, idleCount, maxIdleLoad);
            }

            idleCount = 0;

            // Note the processing time to report the load is excluded from the timespan
            timeLast = Executive_GetPerformanceCounter();
        }
        idleCount++;
        Executive_Sleep(0); // We are relying on this to be equivalent to sched_yield()
    }

    return 0;
}


//===========================================================================
// Public Functions
//===========================================================================

//
// Function: TLC_CPUUtilization_Start
//
// Purpose: Launches and calibrates a low priority thread to measure CPU
//   utilization. Should be called before any other threads are launched,
//   to allow proper calibration at 0%.
//
// Arguments:
//   uiUpdateIntervalInMS [in] - How often CPU utilization should be
//     computed, in milliseconds. If the requested interval cannot
//     be honored due to insufficient precision in QueryPerformanceCounter,
//     this function returns pkS_FALSE.
//   fPrintCPUUtilization [in] - TRUE if you want the CPU idler thread to
//     print CPU usage statistics to console, otherwise FALSE.
//   pfnCPUUtilNotification [in, optional] - pointer to a notification
//     callback which will be called when a new CPU utilization computation
//     is available. Set to NULL if not needed. Typical action of the callback
//     will be to call TLC_CPUUtilization_GetPercent.
//   pCPUUtilNotificationContext [in, optional] - pointer to context
//     which will be passed to pfnCPUUtilNotification.
//
// Returns:
//   pkRESULT indicating success or failure.
//     pkS_OK means total success.
//     pkS_FALSE means that the requested update interval could not be
//       honored. CPU utilization will will be computed, but at a longer
//       interval than requested.
//
pkRESULT TLC_CPUUtilization_Start(uint32_t uiUpdateIntervalInMS,
                                  bool_t fPrintCPUUtilization,
                                  PFNCPUUTILNOTIFICATION pfnCPUUtilNotification,
                                  void *pCPUUtilNotificationContext)
{
    pkRESULT pkRes = pkS_OK;
    bool_t fInsufficientResolution = FALSE;

    if (FALSE == TLC_CmdLine_IsCPUUtilizationRequested())
        goto exit;

    if (0 == uiUpdateIntervalInMS)
    {
        pkRes = TraceResult(pkE_INVALIDARG);
        goto exit;
    }

    if (g_fExitIdlerThread || NULL != g_hIdlerThread)
    {
        // Previous idler thread instance was not properly shut down
        pkRes = TraceResult(pkE_UNEXPECTED);
        goto exit;
    }

    if (Executive_GetPerformanceFrequency() < 1000.0F / uiUpdateIntervalInMS)
    {
        fInsufficientResolution = TRUE;
    }

    g_uiUpdateIntervalInMS = uiUpdateIntervalInMS;
    g_fPrintCPUUtilization = fPrintCPUUtilization;
    g_pfnCPUUtilNotification = pfnCPUUtilNotification;
    g_pCPUUtilNotificationContext = pCPUUtilNotificationContext;

    TRACEPK_EXIT(pkRes, Executive_CreateThread(CPUUtilizationThread, NULL, 0,
        &g_hIdlerThread));
    TRACEPK_EXIT(pkRes, Executive_SetThreadPriority(g_hIdlerThread,
        pkEXECUTIVE_THREAD_PRIORITY_LOWEST));
    Executive_Sleep(uiUpdateIntervalInMS + uiUpdateIntervalInMS/2); // Allow idler thread to measure and calibrate 0% CPU

exit:
    if (pkS_OK == pkRes && fInsufficientResolution)
        pkRes = pkS_FALSE;

    return pkRes;
}


//
// Function: TLC_CPUUtilization_Stop
//
// Purpose: Stops the thread previously launched to measure CPU utilization.
//
// Arguments: None.
//
// Returns:
//   pkRESULT indicating success or failure.
//
pkRESULT TLC_CPUUtilization_Stop(void)
{
    pkRESULT pkRes = pkS_OK;

    if (FALSE == TLC_CmdLine_IsCPUUtilizationRequested())
        goto exit;

    if (NULL == g_hIdlerThread)
        goto exit;

    g_fExitIdlerThread = TRUE;
    TRACEPK_EXIT(pkRes, Executive_WaitForThread(g_hIdlerThread, EXEC_WAIT_INFINITE));
    TRACEPK_EXIT(pkRes, Executive_CloseThread(g_hIdlerThread));

    g_hIdlerThread = NULL;
    g_fExitIdlerThread = FALSE;
    g_uiUpdateIntervalInMS = 0;
    g_pfnCPUUtilNotification = NULL;
    g_pCPUUtilNotificationContext = NULL;

exit:
    return pkRes;
}



//
// Function: TLC_CPUUtilization_GetPercent
//
// Purpose: Returns percent CPU utilization.
//
// Arguments:
//   puiCPUUtilization [out] - returns percent CPU utilization.
//
// Returns:
//   pkRESULT indicating success or failure. If user did not request CPU
//   utilization, this function will return failure.
//
pkRESULT TLC_CPUUtilization_GetPercent(uint32_t *puiCPUUtilization)
{
    pkRESULT pkRes = pkS_OK;

    *puiCPUUtilization = 0;
    if (NULL == g_hIdlerThread)
    {
        pkRes = pkE_FAIL; // Do not trace this one, might happen often by design
        goto exit;
    }

    *puiCPUUtilization = g_uiCpuUtilizationPercent;

exit:
    return pkRes;
}



const char *TLC_GetCmdLineCPUUtilization(void)
{
    return "  -proc       Requests that CPU utilization be measured.\n";
}
