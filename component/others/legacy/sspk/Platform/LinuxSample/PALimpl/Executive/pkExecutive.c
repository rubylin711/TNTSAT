///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  pkExecutive.c
//
//  Implementation of the methods and data types for the set of functions that
//  make up a portable Executive.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

//=============================================================================
// I N C L U D E     F I L E S
//=============================================================================
#include <pkPAL.h>
#include <pkExecutive.h>

#include <platPrivate.h>
#include <palPrint.h>

#include <stdarg.h> // valist
#include <memory.h>
#include <malloc.h>

#include <pthread.h> //Threading
#include <signal.h>  //suspend/resume
#include <errno.h>   //error return values
#include <sched.h>   //priority set/get
#include <time.h>    //clock_gettime
#include <stdlib.h>  //srand; getenv
#include <wchar.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h> // setpriority
#include <sys/syscall.h> // syscall numbers
#include <unistd.h> // syscall

#include <palExecutive.h>
#ifdef MCX_BUILDOPTION_MEMORYTRACKING
#include <palMemTrack.h>
#endif

//=============================================================================
// M A C R O    D E F I N I T I O N S
//=============================================================================

//If the following environment variable is set and the file specified can be opened for write,
//    then Executive_DebugPrintf is output to the file IN PLACE OF stdout
#define DEBUG_PRINTF_LOGFILE_NAME_ENV_VARIABLE "MSPK_DEBUGLOGFILENAME"

//These need to be fine tuned
#define MAX_PLATFORM_PAL_MUTEXES        50
#define MAX_PLATFORM_PAL_SEMAPHORES     50
#define MAX_PLATFORM_PAL_EVENTS         50
#define MAX_SYNC_OBJECT_NAME_LEN        32

#define ELEMENT_OFFSET(object, member) ((uint32_t)(&(((object *)0)->member)))

#define THREAD_ID_INITIALIZER           0xFFFFFFFF

#define THREAD_MAX_START_TIME 1000 /* -ms */

#define c_RealtimeSchedulingPolicy SCHED_RR
#define c_NormalSchedulingPolicy SCHED_OTHER

#ifdef MCX_BUILDOPTION_MEMORYTRACKING
static void MCX_DEBUG_Check_All_Threads(void);
#endif

//=============================================================================
// D A T A     S T R U C T U R E    D E F I N I T I O N S
//=============================================================================
typedef enum
{
    pal_mutex_type,
    pal_semaphore_type,
    pal_event_type,
    pal_lock_type
}pal_sync_object_type;

typedef struct
{
    THREAD_ENTRY Function;
    void *       pParams;
}wrapper_func_params;

struct pal_thread_context_t
{
    //Mutex that is created/opened at time of thread creation, released at thread exit.
    pthread_mutex_t     PalThreadMutex;
    pthread_cond_t      Condition;
    pthread_t           ThreadId;
    pid_t               ProcessId;
    bool_t              bThreadExited;
    wrapper_func_params ThreadParams;
    pkRESULT            iLastSocketError;
    int                 iPalPriority;
    struct pal_thread_context_t *pNext;
#ifdef MCX_BUILDOPTION_MEMORYTRACKING
    const char *        szFile;
    const char *        szFunc;
    int                 line;
#endif
};
typedef struct pal_thread_context_t pal_thread_context;

static pkRESULT   s_MainThread_iLastSocketError = pkS_OK;

//General Note: The name fields can be shortened to the least number of significant places needed
typedef struct
{
    pthread_mutex_t      Handle;            // Handle to this object.
    pthread_cond_t       Condition;
    char                 Name[MAX_SYNC_OBJECT_NAME_LEN];
    union
    {
        int32_t          Count;
        bool_t           bSignaled;         // Has the event been signaled?
    };
    int32_t              MaxCount;
    bool_t               bIsManualReset;    //Is it a manual reset event?
    pal_sync_object_type Type;
    int32_t              ref_count;
    pthread_t            ThreadId;          // ID of the thread currently owns the object.
    pthread_mutex_t      PalSyncMutex;      // Handle to mutex used for thread synchronization.
}pal_sync_object_context;

//
// Memory allocation structure
typedef struct __tagMemoryAllocation
{
    uint32_t                        uiSize;
    uint8_t                         buffer[];
} MemoryAllocation;




//=============================================================================
// M O D U L E    S T A T I C    V A R I A B L E S
//=============================================================================

// We need to track the thread handles so that Executive_GetCurrentThread can work.
static pthread_mutex_t s_PalThreadContextMutex;
static pal_thread_context *s_pPalThreadContextListHead = NULL;

// Debug printing/logging
static pthread_mutex_t s_PalDebugPrintfMutex;
static FILE* s_fdDebugPrintf = NULL;
static double s_secsPrevPrintTime = 0;

// Initialization re-entry/exit protection
static bool_t s_bExecutiveInitialized = FALSE;

// Global Mutex used for Executive_Interlockedxxx functions
#if !defined (__i386__) // interlock mutex is not used in i386 case
static pthread_mutex_t s_InterlockedMutex;
#endif


#ifdef MALLOC_RECORD
static pthread_mutex_t s_PalMemMutex;           // for counting memory allocated
static unsigned long pal_mem_alloced = 0;
#endif

#ifdef MCX_BUILDOPTION_MEMORYTRACKING
    // for future use...
static MemoryAllocation * s_pFirstMemoryAllocation = NULL;
#endif //MCX_BUILDOPTION_MEMORYTRACKING

// Enable/disable debug outputs.
static bool_t s_bDebugOutputEnabled = TRUE;



//=============================================================================
// Private functions
//=============================================================================

static void s_AddPalThreadContext(pal_thread_context *pContext);
static void s_RemovePalThreadContext(pal_thread_context *pContext);
static pal_thread_context* s_GetPalThreadContextByID(pthread_t ThreadID);

static void s_ThreadExitCleanup(void *pvContext);
static void* s_ThreadExecWrapper(void* pvContext);


static double s_SecondsSinceStart ( )
{
    static bool_t isStarted = FALSE;
    static double usStartTime = 0.0;

    struct timeval tv;
    gettimeofday(&tv, NULL);
    double usNowTime = (double)((uint32_t)tv.tv_sec) + (double)tv.tv_usec / 1000000;

    if (!isStarted)
    {
        isStarted = TRUE;
        usStartTime = usNowTime;
    }

    return (usNowTime - usStartTime);
}

//=============================================================================
// Public function implementations
//=============================================================================
void pkAPI Executive_DebugBreak( void )
{
    return;
}

//-----------------------------------------------------------------------------
//  Threads
//-----------------------------------------------------------------------------

#ifdef MCX_BUILDOPTION_MEMORYTRACKING
pkRESULT pkAPI Executive_CreateThread_Tracked
(
    THREAD_ENTRY entry,
    void *pParam,
    uint32_t stackSize,
    pkHANDLE *pHandle,
    const char *szFile,
    const char *szFunc,
    int line
)
#else
pkRESULT pkAPI Executive_CreateThread
(
    THREAD_ENTRY entry,
    void *pParam,
    uint32_t stackSize,
    pkHANDLE *pHandle
)
#endif
{
    pkRESULT result = pkE_UNEXPECTED;
    int retval = 0;
    pal_thread_context *pThread = NULL;
    pthread_attr_t pthreadAttr;
    struct sched_param sParam;

    if (NULL == pHandle)
    {
        PALPRINTMSG(PALPRINT_ERROR,("Executive_CreateThread: NULL pHandle \n"));
        result = pkE_POINTER;
        goto exit;
    }
    *pHandle = NULL; // in case of error exit

    pThread = (pal_thread_context *) Executive_Alloc(sizeof(pal_thread_context), TRUE);

    if (NULL == pThread)
    {
        PALPRINTMSG(PALPRINT_ERROR,("Executive_CreateThread: Cannot allocate handle memory\n"));
        result = pkE_OUTOFMEMORY;
        goto bailContext;
    }

#ifdef MCX_BUILDOPTION_MEMORYTRACKING
    pThread->szFile     = szFile;
    pThread->szFunc     = szFunc;
    pThread->line       = line;
#endif

    if (0 != pthread_attr_init(&pthreadAttr))
    {
        PALPRINTMSG(PALPRINT_ERROR,("Executive_CreateThread: Unable to initialize pthread attr\n"));
        result = pkE_UNEXPECTED;
        goto bailAttr;
    }

    // set the new stack size if given
    // otherwise take the default that is set by pthread_attr_init
    if (0 != stackSize)
    {
        PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("Executive_CreateThread: Call pthread_attr_setstacksize (%u)\n", stackSize));
        if (0 != pthread_attr_setstacksize(&pthreadAttr, stackSize))
        {
            PALPRINTMSG(PALPRINT_ERROR,("Executive_CreateThread: failed to set stack size\n"));
            result = pkE_INVALIDARG;
            goto bailCond;
        }
    }

    // FYI: If the thread is created detached, then use of the ID of the newly created thread by
    //      pthread_detach or pthread_join is an error.
    if (0 != pthread_attr_setdetachstate(&pthreadAttr, PTHREAD_CREATE_DETACHED))
    {
        result = pkE_UNEXPECTED;
        goto bailCond;
    }

    //use a wrapper function so that we can support waiting for the thread exit, setting priority, etc.
    pThread->ThreadParams.Function = entry;
    pThread->ThreadParams.pParams = pParam;

    // set the initial thread socket error value
    pThread->iLastSocketError = pkS_OK;

    //get our mutex and exit condition initialized.
    if (0 != pthread_cond_init(&(pThread->Condition), NULL))
    {
        result = pkE_UNEXPECTED;
        goto bailCond;
    }

    if (0 != pthread_mutex_init(&(pThread->PalThreadMutex), NULL))
    {
        result = pkE_UNEXPECTED;
        goto bailMutex;
    }

    PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("Executive_CreateThread: calling pthread_create\n"));

    pthread_mutex_lock(&(pThread->PalThreadMutex));

    pThread->bThreadExited = FALSE;
    retval = pthread_create
                (
                    &(pThread->ThreadId),
                    &pthreadAttr,
                    s_ThreadExecWrapper,
                    pThread
                );
    PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("Executive_CreateThread: pthread_create result: %d\n", retval));

    switch(retval)
    {
        case 0: //success
        {
            s_AddPalThreadContext(pThread);
            *pHandle = (pkHANDLE)pThread;
            result = pkS_OK;
            break;
        }
        case EAGAIN:
        {
            result = pkE_OUTOFMEMORY;
            break;
        }
        case EINVAL:
        {
            result = pkE_INVALIDARG;
            break;
        }
        case EPERM:
        {
            result = pkE_INVALID_REQUEST;
            break;
        }
        default:
            result = pkE_UNEXPECTED;
    }

    pthread_mutex_unlock(&(pThread->PalThreadMutex));

    if(pkSUCCEEDED(result))
    {
        pthread_attr_destroy(&pthreadAttr);
        goto exit;
    }

    pthread_mutex_destroy(&(pThread->PalThreadMutex));
bailMutex:

    pthread_cond_destroy(&(pThread->Condition));
bailCond:

    pthread_attr_destroy(&pthreadAttr);
bailAttr:

    Executive_Free(pThread);
bailContext:

exit:
    return result;
}

#ifdef MCX_BUILDOPTION_MEMORYTRACKING
pkRESULT pkAPI Executive_CreateThreadEx_Tracked
(
    THREAD_ENTRY entry,
    void *pParam,
    uint32_t stackSize,
    pkHANDLE *pHandle,
    THREAD_CATEGORY threadCategory,
    const char *szFile,
    const char *szFunc,
    int line
)
#else
pkRESULT pkAPI Executive_CreateThreadEx
(
    THREAD_ENTRY entry,
    void *pParam,
    uint32_t stackSize,
    pkHANDLE *pHandle,
    THREAD_CATEGORY threadCategory
)
#endif
{
    pkRESULT result;

#ifdef MCX_BUILDOPTION_MEMORYTRACKING
    result = Executive_CreateThread_Tracked(entry, pParam, stackSize, pHandle, szFile, szFunc, line);
#else
    result = Executive_CreateThread(entry, pParam, stackSize, pHandle);
#endif

    if (pkSUCCEEDED(result))
    {
        switch(threadCategory)
        {
            case THREAD_CATEGORY_AVCLIENT:
            case THREAD_CATEGORY_RTSP:
                Executive_SetThreadPriority(*pHandle, pkEXECUTIVE_THREAD_PRIORITY_HIGH);
                break;
        }
    }

    return result;
}


pkRESULT pkAPI Executive_SetThreadPriority( pkHANDLE hHandle, int priority )
{
    pkRESULT result = pkE_FAIL;
    int retval = 0;
    pal_thread_context * pThread;

    if (NULL == hHandle)
    {
        result = pkE_INVALIDARG;
        goto exit;
    }

    pThread = (pal_thread_context *)hHandle;

    // catch priority not being changed
    if (pThread->iPalPriority == priority)
    {
        result = pkS_OK;
        goto exit;
    }

    // Set priority. Time critical uses realtime policy. All others use non-realtime policy.

    if (pkEXECUTIVE_THREAD_PRIORITY_TIME_CRITICAL == priority)
    {
        struct sched_param sParam;

        sParam.sched_priority = // 1/4 of the way between min and max:
            (sched_get_priority_max(c_RealtimeSchedulingPolicy) - sched_get_priority_min(c_RealtimeSchedulingPolicy)) / 4
            + sched_get_priority_min(c_RealtimeSchedulingPolicy);

        retval = pthread_setschedparam(pThread->ThreadId, c_RealtimeSchedulingPolicy, &sParam);
    }
    else // not time critical
    {
        int schedPriority;
        int maxStartChecks = THREAD_MAX_START_TIME / 10; // 10 ms per check
        pid_t processIdToSet = pThread->ProcessId;

        // wait until thread proc has had a chance to start and set its process Id
        while (0 == processIdToSet && maxStartChecks-- > 0)
        {
            Executive_Sleep(10);
            processIdToSet = pThread->ProcessId;
        }

        if (0 == processIdToSet)
        {
            PALPRINTMSG(PALPRINT_ERROR,("%s error: PID not set yet!\n", __FUNCTION__));
            result = pkE_UNEXPECTED;
            goto exit;
        }

        switch (priority)
        {
            // see man page for "setpriority" for documentation on schedPriority numbers used below
            case pkEXECUTIVE_THREAD_PRIORITY_HIGHEST:
                schedPriority = -10;
                break;
            case pkEXECUTIVE_THREAD_PRIORITY_HIGH:
                schedPriority = -5;
                break;
            case pkEXECUTIVE_THREAD_PRIORITY_NORMAL:
                schedPriority = 0;
                break;
            case pkEXECUTIVE_THREAD_PRIORITY_LOW:
                schedPriority = 5;
                break;
            case pkEXECUTIVE_THREAD_PRIORITY_LOWEST:
                schedPriority = 10;
                break;
            default:
                result = pkE_INVALIDARG;
                goto exit;
        }

        if (pkEXECUTIVE_THREAD_PRIORITY_TIME_CRITICAL == pThread->iPalPriority)
        {
            // when switching out of time critical, return to "other" policy and static priority zero
            struct sched_param sParam;
            sParam.sched_priority = 0;
            retval = sched_setscheduler(pThread->ThreadId, c_NormalSchedulingPolicy, &sParam);
            if (0 != retval)
            {
                goto bail;
            }
        }

        retval = setpriority(PRIO_PROCESS, processIdToSet, schedPriority);
    }

bail:
    switch(retval)
    {
        case 0: // success
        {
            pThread->iPalPriority = priority;
            result = pkS_OK;
            break;
        }
        case EINVAL:
        case ENOTSUP:
        case EPERM:
        case ESRCH:
        {
            result = pkE_INVALIDARG;
            break;
        }
        default:
        {
            result = pkE_UNEXPECTED;
            break;
        }
    }

    if (0 != retval)
    {
        PALPRINTMSG(PALPRINT_ERROR,("%s failed (%d) to set priority %d for PID: %u, TID: %u\n",
            __FUNCTION__, retval, priority, pThread->ProcessId, pThread->ThreadId));

        result = pkS_OK; // TODO: find a way to set priority without admin rights
    }
    else
    {
        PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("%s set priority %d for PID: %u, TID: %u\n",
            __FUNCTION__, priority, pThread->ProcessId, pThread->ThreadId));
    }

exit:
    return result;
}


pkRESULT pkAPI Executive_GetThreadPriority( pkHANDLE hHandle, int *pPriority )
{
    pkRESULT result;
    pal_thread_context * pThread = (pal_thread_context *)hHandle;;

    if(NULL == pThread)
    {
        result = pkE_INVALIDARG;
        goto exit;
    }

    if(NULL == pPriority)
    {
        result = pkE_POINTER;
        goto exit;
    }

    *pPriority = pThread->iPalPriority;
    result = pkS_OK;

exit:
    return result;
}

pkRESULT pkAPI Executive_CloseThread( pkHANDLE hHandle )
{
    pkRESULT result;
    pal_thread_context * pThread = (pal_thread_context *)hHandle;

    if (NULL == pThread)
    {
        result = pkE_INVALIDARG;
    }
    else
    {
        PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("Executive_CloseThread called for thread id: %u\n", pThread->ThreadId));
        if(pThread->bThreadExited)
        {
            s_RemovePalThreadContext(pThread);
            result = pkS_OK;
        }
        else // don't remove if the thread is still running
        {
            PALPRINTMSG(PALPRINT_ERROR,("Executive_CloseThread called for thread id %u that is still running.\n", pThread->ThreadId));
            result = pkE_UNEXPECTED;
        }
    }

    return result;
}


pkRESULT pkAPI Executive_TerminateThread( pkHANDLE hHandle, int32_t exitCode )
{
    pkRESULT result = pkE_FAIL;
    pal_thread_context * pThread=(pal_thread_context *)hHandle;
    int ret;

    if(pThread)
    {
        pthread_mutex_lock(&(pThread->PalThreadMutex));
        if(pThread->bThreadExited == FALSE)
        {
            ret = pthread_cancel(pThread->ThreadId);
            switch(ret)
            {
                case 0:
                    result = pkS_OK;
                    pthread_cond_wait(&(pThread->Condition), &(pThread->PalThreadMutex));
                    break;
                case ESRCH:
                    result = pkE_INVALIDARG;
                    break;
                default:
                    result = pkE_UNEXPECTED;
                    break;
            }
        }
        else
        {
            result = pkS_OK;
        }
        pthread_mutex_unlock(&(pThread->PalThreadMutex));
    }
    else
    {
        return pkE_INVALIDARG;
    }
    return result;
}


pkRESULT pkAPI Executive_GetCurrentThread( pkHANDLE *phHandle )
{
    pkRESULT result = pkE_FAIL;
    pal_thread_context * context = NULL;

    if(NULL == phHandle)
    {
        result = pkE_POINTER;
        goto exit;
    }

    context = s_GetPalThreadContextByID(pthread_self());
    if(NULL != context)
    {
        result = pkS_OK;
    }
    *phHandle = (pkHANDLE *)context; // sets NULL if not found

exit:
    return result;
}


pkRESULT pkAPI Executive_WaitForThread( pkHANDLE hHandle, uint32_t timeOutMs )
{
    pkRESULT result = pkE_FAIL;
    int retval = 0;
    pal_thread_context * pThread;
    struct timeval CurrentTime;
    struct timespec ExitTime;

    if(NULL != hHandle)
    {
        pThread = (pal_thread_context *)hHandle;
    }
    else
    {
        result = pkE_POINTER;
        goto exit;
    }

    PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("Executive_WaitForThread called with handle: %x  timeout %x \n", hHandle, timeOutMs));

    if (EXEC_WAIT_INFINITE == timeOutMs) // Wait forever.
    {
        pthread_mutex_lock(&(pThread->PalThreadMutex));
        if (FALSE == pThread->bThreadExited)
        {
            // Only wait if the thread is still running.
            pthread_cond_wait(&(pThread->Condition), &(pThread->PalThreadMutex));
        }
        pthread_mutex_unlock(&(pThread->PalThreadMutex));
        result = pkS_OK;
        goto exit;
    }
    else // Wait with timeout.
    {
        PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("Executive_WaitForThread mutex locking start \n"));
        pthread_mutex_lock(&(pThread->PalThreadMutex));
        PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("Executive_WaitForThread mutex locked \n"));
        // Get current time
        gettimeofday(&CurrentTime,NULL);
        // Calculate seconds from MS and any over run of nano seconds
        if(0 != timeOutMs)
        {
            uint32_t seconds = timeOutMs / 1000;
            uint32_t milliseconds = timeOutMs % 1000;
            ExitTime.tv_sec = CurrentTime.tv_sec + seconds + ((CurrentTime.tv_usec + (milliseconds * 1000)) / 1000000);
            ExitTime.tv_nsec = ((CurrentTime.tv_usec + (milliseconds * 1000)) % 1000000) * 1000;
        }
        else
        {
            // Note: You can't call pthread_cond_timedwait with a 0 delta from the value
            // you received from clock_gettime. Add 100 nsecs to the target expiration time
            // to make it happy.
            ExitTime.tv_sec = CurrentTime.tv_sec + (((CurrentTime.tv_usec * 1000) + 100) / 1000000000);
            ExitTime.tv_nsec = (((CurrentTime.tv_usec * 1000) + 100) % 1000000000);
        }

        if (FALSE == pThread->bThreadExited)
        {
            // Wait for that long if the thread is still running.
            PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("Executive_WaitForThread calling pthread_cond_timedwait... \n"));
            retval = pthread_cond_timedwait(&(pThread->Condition),&(pThread->PalThreadMutex), &ExitTime);
        }
        else
        {
            // If the thread has already exited, return success status and quit.
            retval = 0;
        }
        pthread_mutex_unlock(&(pThread->PalThreadMutex));
        PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("Executive_WaitForThread mutex unlocked \n"));
    }

    switch(retval)
    {
        case 0: //success
        {
            result = pkS_OK;
            break;
        }
        case ETIMEDOUT: //not signaled yet
        {
            result = pkS_FALSE;
            break;
        }
        default:
        {
            result = pkE_UNEXPECTED; //unknown error
            break;
        }
    }
exit:
    return result;
}


pkRESULT pkAPI Executive_GetCurrentThreadId( uint32_t *pThreadId )
{
    pkRESULT result = pkE_POINTER;

    if(pThreadId)
    {
       *pThreadId = (uint32_t) pthread_self();
       result = pkS_OK;
    }
    return result;
}


pkRESULT pkAPI Executive_GetThreadId( pkHANDLE hHandle, uint32_t *pThreadId )
{
    pkRESULT result;
    pal_thread_context * pThread = (pal_thread_context *)hHandle;;

    if(NULL == pThread)
    {
        result = pkE_INVALIDARG;
        goto exit;
    }

    if(NULL == pThreadId)
    {
        result = pkE_POINTER;
        goto exit;
    }

    *pThreadId = pThread->ThreadId;
    result = pkS_OK;

exit:
    return result;
}

//-----------------------------------------------------------------------------
//  Locks
//-----------------------------------------------------------------------------
#ifdef MCX_BUILDOPTION_MEMORYTRACKING
pkRESULT pkAPI Executive_CreateLock_Tracked( pkHANDLE *pHandle, const char *szFile, const char *szFunc, int line)
#else
pkRESULT pkAPI Executive_CreateLock( pkHANDLE *pHandle )
#endif
{
    pkRESULT result = pkE_POINTER;
    pal_sync_object_context * plock = NULL;
    int retval = 0;

    // Sanity check.
    if (NULL == pHandle)
    {
        PALPRINTMSG(PALPRINT_ERROR,("Executive_CreateLock: NULL pHandle \n"));
        result = pkE_POINTER;
        goto exit;
    }

#ifdef MCX_BUILDOPTION_MEMORYTRACKING
    plock = (pal_sync_object_context*)malloc_Tracked(sizeof(pal_sync_object_context), szFile, szFunc, line);
#else
    plock = (pal_sync_object_context*)malloc(sizeof(pal_sync_object_context));
#endif

    if(NULL != plock)
    {
        PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE, ("++Thread %x %s %s %x \n", pthread_self(), __FILE__, __FUNCTION__, plock));
        retval = pthread_mutex_init(&(plock->PalSyncMutex), NULL);
        if(!retval)
        {
            retval = pthread_mutex_init(&(plock->Handle), NULL);
            if(!retval)
            {
                plock->Type = pal_lock_type;
                plock->ref_count = 0;
                plock->ThreadId = THREAD_ID_INITIALIZER; // No thread owns this lock yet.
                *pHandle = (pkHANDLE)plock;
                result = pkS_OK;
            }
            else
            {
                result = pkE_UNEXPECTED;
                pthread_mutex_destroy(&(plock->PalSyncMutex));
                free(plock);
                goto exit;
            }
        }
        else
        {
            result = pkE_UNEXPECTED;
            free(plock);
            goto exit;
        }
    }

exit:
    return result;
}

pkRESULT pkAPI Executive_TryEnterLock( pkHANDLE hHandle )
{
    pkRESULT result = pkE_POINTER;
    int retval;
    pal_sync_object_context * plock = (pal_sync_object_context *)hHandle;

    // Sanity checks.
    if (NULL == plock)
    {
        result = pkE_POINTER;
        goto exit;
    }
    else if (plock->Type != pal_lock_type)
    {
        result = pkE_INVALIDARG;
        goto exit;
    }
    else if (Executive_IsLockedByCurrentThread(hHandle))
    {
        // This lock's already acquired by this thread, just return right away.
        plock->ref_count++;
        result = pkS_OK;
        goto exit;
    }
    else
    {
        // Now try to acquire the lock.
        PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("++Thread %x  %s  %x \n", pthread_self(), __FUNCTION__, hHandle));
        retval = pthread_mutex_trylock(&(plock->Handle)); // Try to acquire the lock.
        switch(retval)
        {
            case 0:
                result = pkS_OK;    // Acquired the lock.
                pthread_mutex_lock(&(plock->PalSyncMutex));
                plock->ref_count = 1;
                plock->ThreadId = pthread_self();
                pthread_mutex_unlock(&(plock->PalSyncMutex));
                break;
            case EBUSY:
                result = pkS_FALSE; // Already acquired by another thread.
                break;
            default:
                result = pkE_FAIL;  // Something went wrong?
                break;
        }

        PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("--Thread %x  %s  %x \n", pthread_self(), __FUNCTION__, hHandle));
    }

exit:
    return result;
}

pkRESULT pkAPI Executive_EnterLock( pkHANDLE hHandle )
{
    pkRESULT result = pkE_POINTER;
    int retval;
    pal_sync_object_context * plock = (pal_sync_object_context *)hHandle;

    // Sanity checks.
    if (NULL == plock)
    {
        result = pkE_POINTER;
        goto exit;
    }
    else if (plock->Type != pal_lock_type)
    {
        result = pkE_INVALIDARG;
        goto exit;
    }
    else if (Executive_IsLockedByCurrentThread(hHandle))
    {
        // This lock's already acquired by this thread, just return right away.
        plock->ref_count++;
        result = pkS_OK;
        goto exit;
    }
    else
    {
        // Now try to acquire the lock.
        retval = pthread_mutex_trylock(&(plock->Handle));
        PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("++Thread %x  Enter  %x \n",pthread_self(), hHandle));

        switch(retval)
        {
            case 0:
                result = pkS_OK;            // Acquire lock successfully.
                pthread_mutex_lock(&(plock->PalSyncMutex));
                plock->ref_count = 1;
                plock->ThreadId = pthread_self();
                pthread_mutex_unlock(&(plock->PalSyncMutex));
                break;

            case EINVAL:
                result = pkE_INVALIDARG;    // Not a valid mutex (permissions, not initialized, etc.)
                break;

            case EDEADLK:   // This case is not currently supported for pthread_mutex_trylock() in RedHat Linux. Leave it here just in case.
                result = pkE_INVALID_REQUEST; // Would result in a deadlock
                break;

            case EBUSY:                     // Already locked by another thread, wait for it.
            default:                        // All others, just wait on the lock.
                retval = pthread_mutex_lock(&(plock->Handle));
                switch(retval)
                {
                    case 0:
                        result = pkS_OK;
                        pthread_mutex_lock(&(plock->PalSyncMutex));
                        plock->ref_count = 1;
                        plock->ThreadId = pthread_self();
                        pthread_mutex_unlock(&(plock->PalSyncMutex));
                        break;
                    default:
                        result = pkE_FAIL;
                        break;
                }
                break;
        }
    }

exit:
    return result;
}

pkRESULT pkAPI Executive_ExitLock( pkHANDLE hHandle )
{
    pkRESULT result = pkE_FAIL;
    int retval;
    pal_sync_object_context * plock = (pal_sync_object_context *)hHandle;

    // Sanity checks.
    if (NULL == plock)
    {
        result = pkE_POINTER;
        goto exit;
    }
    else if (plock->Type != pal_lock_type)
    {
        result = pkE_INVALIDARG;
        goto exit;
    }
    else if (!Executive_IsLockedByCurrentThread(hHandle))
    {
        // Error: the current thread doesn't own this lock!
        result = pkE_FAIL;
        goto exit;
    }
    else
    {
        pthread_mutex_lock(&(plock->PalSyncMutex));
        if(plock->ref_count == 1)
        {
            retval = pthread_mutex_unlock(&(plock->Handle));
        }
        else
        {
            retval = 0;
        }

        switch(retval)
        {
            case 0:
                result = pkS_OK;            // Exit lock.
                if (plock->ref_count > 0)
                {
                    plock->ref_count--;
                }

                if (plock->ref_count == 0)
                {
                    plock->ThreadId = THREAD_ID_INITIALIZER; // This thread no longer owns this lock.
                }
                PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("++Thread %x  Exit Lock %x\n", pthread_self(), hHandle));
                break;
            case EINVAL:
                result = pkE_INVALIDARG;    // Not a valid mutex (permissions, not initialized, etc.)
                break;
            case EPERM:                     // This thread doesn't own the mutex!
                result = pkE_INVALID_REQUEST;
                break;
            default:                        // All others just return pkE_FAIL.
                result = pkE_FAIL;
                break;
        }

        pthread_mutex_unlock(&(plock->PalSyncMutex));
    }

exit:
    return result;
}

pkRESULT pkAPI Executive_DeleteLock( pkHANDLE hHandle )
{
    pkRESULT result = pkE_POINTER;
    int retval;
    pal_sync_object_context * plock = (pal_sync_object_context *)hHandle;

    // Sanity checks.
    if (NULL == plock)
    {
        result = pkE_POINTER;
        goto exit;
    }
    else if (plock->Type != pal_lock_type)
    {
        result = pkE_INVALIDARG;
        goto exit;
    }
    else
    {
        PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE, ("++Thread %x %s %s %x \n", pthread_self(), __FILE__, __FUNCTION__, plock));
        pthread_mutex_lock(&(plock->PalSyncMutex));
        if (plock->ref_count == 0)
        {
            retval = pthread_mutex_destroy(&(plock->Handle));
        }
        else
        {
            retval = EBUSY;
            PALPRINTMSG(PALPRINT_EXECUTIVE, ("%s %s called for lock that is still in use: %x\n", __FILE__, __FUNCTION__, plock));
        }
        pthread_mutex_unlock(&(plock->PalSyncMutex));

        switch(retval)
        {
            case 0:
                result = pkS_OK;    // Lock destroyed successfully.
                pthread_mutex_lock(&(plock->PalSyncMutex));
                plock->ThreadId = THREAD_ID_INITIALIZER; // This thread no longer owns this lock.
                pthread_mutex_unlock(&(plock->PalSyncMutex));
                pthread_mutex_destroy(&(plock->PalSyncMutex));
                free(hHandle);
                break;

            case EBUSY:             // This or another thread is waiting on this lock.
                result = pkE_INVALID_REQUEST;
                break;

            case EINVAL:            // hHandle is not a mutex.
                result = pkE_INVALIDARG;
                break;

            default:                // All others, just return pkE_FAIL.
                result = pkE_FAIL;
                break;
        }
    }

exit:
    return result;
}

bool_t pkAPI Executive_IsLockedByCurrentThread( pkHANDLE hHandle )
{
    bool_t result = TRUE;
    int retval = pkE_POINTER;
    pal_sync_object_context * plock = (pal_sync_object_context *)hHandle;

    if(NULL != plock && plock->Type == pal_lock_type)
    {
        PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("++Thread %x %s %s %x \n", pthread_self(), __FILE__, __FUNCTION__, plock));
        retval = pthread_mutex_trylock(&(plock->Handle)); // Try to acquire the lock.
        switch(retval)
        {
            case 0:
                result = FALSE; // Lock was free.
                PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("++++ Lock is free!\n"));
                pthread_mutex_unlock(&(plock->Handle));
                break;
            case EBUSY:         // Lock's already acquired by this or another thread.
                pthread_mutex_lock(&(plock->PalSyncMutex));
                if (plock->ThreadId == pthread_self())
                {
                    PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("++++ Locked by this thread!\n"));
                    result = TRUE; // Locked by this thread.
                }
                else
                {
                    PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("++++ Locked by another thread!\n"));
                    result = FALSE; // Locked by another thread.
                }
                pthread_mutex_unlock(&(plock->PalSyncMutex));
                break;
            default:
                break;          // TRUE as guard against deadlock.
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
//  Semaphores
//-----------------------------------------------------------------------------

// Using the pthread condition APIs, not the posix Semaphore APIs to have access
// to timed wait and ability to signal all threads waiting on a semaphore.
#ifdef MCX_BUILDOPTION_MEMORYTRACKING
pkRESULT pkAPI Executive_CreateSemaphore_Tracked
(
    const char *pszIgnored,
    int32_t initCount,
    int32_t maxCount,
    pkHANDLE *pHandle,
    const char *szFile,
    const char *szFunc,
    int line
)
#else
pkRESULT pkAPI Executive_CreateSemaphore
(
    const char *pszIgnored,
    int32_t initCount,
    int32_t maxCount,
    pkHANDLE *pHandle
)
#endif
{
    pkRESULT result = pkE_FAIL;
    pal_sync_object_context * psem = NULL;
    int retval = 0;

    pkASSERT(NULL==pszIgnored);

    // Sanity check.
    if (NULL == pHandle)
    {
        PALPRINTMSG(PALPRINT_ERROR,("Executive_CreateSemaphore: NULL pHandle \n"));
        result = pkE_POINTER;
        goto exit;
    }

#ifdef MCX_BUILDOPTION_MEMORYTRACKING
    psem = (pal_sync_object_context*)malloc_Tracked(sizeof(pal_sync_object_context), szFile, szFunc, line);
#else
    psem = (pal_sync_object_context*)malloc(sizeof(pal_sync_object_context));
#endif

    if(NULL != psem)
    {
        memset(psem, 0, sizeof(pal_sync_object_context));

        retval = pthread_mutex_init(&(psem->Handle), NULL);
        if(!retval)
        {
            retval = pthread_cond_init(&(psem->Condition),NULL);
            if(!retval)
            {
                psem->Count = initCount;
                psem->MaxCount = maxCount;
                psem->Type = pal_semaphore_type;
                *pHandle = (pkHANDLE)psem;
                result = pkS_OK;
            }
            else
            {
                free(psem);
            }
        }
        else
        {
            free(psem);
        }
    }

exit:
    return result;
}

pkRESULT pkAPI Executive_WaitForSemaphore( pkHANDLE hHandle, uint32_t timeOutMs )
{
    pkRESULT result = pkE_POINTER;
    pal_sync_object_context *psem = (pal_sync_object_context *)hHandle;
    int retval = 0;
    struct timespec target;

    // Sanity checks.
    if (psem == NULL)
    {
        result = pkE_POINTER;
        goto exit;
    }
    else if (psem->Type != pal_semaphore_type)
    {
        result = pkE_INVALIDARG;
        goto exit;
    }
    else
    {
        // If a semaphore is available, acquire it and return immediately.
        retval = pthread_mutex_lock(&(psem->Handle));
        if(retval)
        {
            result = pkE_FAIL;
            goto exit;
        }

        if (psem->Count > 0)
        {
            psem->Count--;
            retval = pthread_mutex_unlock(&(psem->Handle));
            result = pkS_OK;
            goto exit;
        }

        // Wait forever if specified.
        if(EXEC_WAIT_INFINITE == timeOutMs)
        {
            retval = pthread_cond_wait(&(psem->Condition), &(psem->Handle));
        }
        // Otherwise, calculate the expiration time and only wait until then.
        else
        {
            // Get the current time.
            clock_gettime(CLOCK_REALTIME, &target);

            if (0 != timeOutMs)
            {
                uint32_t seconds = timeOutMs / 1000;
                uint32_t milliseconds = timeOutMs % 1000;
                target.tv_sec += (seconds + ((target.tv_nsec + (milliseconds * 1000000)) / 1000000000));
                target.tv_nsec = ((target.tv_nsec + (milliseconds * 1000000)) % 1000000000);
            }
            else
            {
                // Note: You can't call pthread_cond_timedwait with a 0 delta from the value
                // you received from clock_gettime. Add 100 nsecs to the target expiration time
                // to make it happy.
                target.tv_sec += ((target.tv_nsec + 100) / 1000000000);
                target.tv_nsec = ((target.tv_nsec + 100) % 1000000000);
            }

            retval = pthread_cond_timedwait(&(psem->Condition), &(psem->Handle), &target);
        }

        switch(retval)
        {
            case 0:
                result = pkS_OK;
                break;
            case ETIMEDOUT:
                result = pkE_TIMEOUT;
                pthread_mutex_unlock(&(psem->Handle));
                goto exit;
            default:
                result = pkS_FALSE;
                pthread_mutex_unlock(&(psem->Handle));
                goto exit;
        }

        // In the off-chance that there's still no available semaphore, wait some more.
        if (psem->Count <= 0)
        {
            // However, if the original request is NO WAIT, then just quit immediately.
            if (timeOutMs == 0)
            {
                result = pkE_TIMEOUT;
                pthread_mutex_unlock(&(psem->Handle));
                goto exit;
            }
            // Or if the original request was WAIT FOREVER, we'll wait for 1 more
            // second, then quit, no matter what. Otherwise, we'll wait up to the
            // original timeout and then quit.
            else if (timeOutMs == EXEC_WAIT_INFINITE)
            {
                clock_gettime(CLOCK_REALTIME, &target);
                target.tv_sec += 1;
            }

            retval = pthread_cond_timedwait(&(psem->Condition), &(psem->Handle), &target);
            switch(retval)
            {
                case 0:
                    psem->Count--;
                    result = pkS_OK;
                    break;
                case ETIMEDOUT:
                    result = pkE_TIMEOUT;
                    break;
                default:
                    result = pkS_FALSE;
                    break;
            }
        }
        else
        {
            psem->Count--;
            result = pkS_OK;
        }

        pthread_mutex_unlock(&(psem->Handle));
    }

exit:
    return result;
}

pkRESULT pkAPI Executive_ReleaseSemaphore( pkHANDLE hHandle, int32_t *pPrevCount )
{
    pkRESULT result = pkE_POINTER;
    pal_sync_object_context * psem = (pal_sync_object_context *) hHandle;
    int retval = 0;

    // Sanity checks.
    if (NULL == psem)
    {
        result = pkE_POINTER;
        goto exit;
    }
    else if (psem->Type != pal_semaphore_type)
    {
        result = pkE_INVALIDARG;
        goto exit;
    }
    else
    {
        retval = pthread_mutex_lock(&(psem->Handle));
        if (!retval)
        {
            // To avoid previous count has reached the Max count limitation.
            if ( psem->Count >=  psem->MaxCount )
            {
                // If increasing 1 would cause the semaphore's count to exceed the maximum count that was specified
                // when the semaphore was created, the count will not be changed and the function
                // returns pkE_FAIL directly without further action.

                result = pkE_FAIL;
                pthread_mutex_unlock(&(psem->Handle));

                goto exit;
            }

            if (NULL != pPrevCount)
            {
                *pPrevCount = psem->Count;
            }
            psem->Count+=1;

            pthread_mutex_unlock(&(psem->Handle));

            retval = pthread_cond_signal(&(psem->Condition));
            if (!retval)
            {
                result = pkS_OK;
            }
            else
            {
                result = pkE_FAIL;
            }
        }
        else
        {
            retval = pkE_FAIL;
        }
    }

exit:
    return result;
}

pkRESULT pkAPI Executive_CloseSemaphore( pkHANDLE hHandle )
{
    pkRESULT result = pkE_POINTER;
    pal_sync_object_context * psem = (pal_sync_object_context *) hHandle;
    int retval = 0;

    // Sanity checks.
    if (NULL == psem)
    {
        result = pkE_POINTER;
        goto exit;
    }
    else if (psem->Type != pal_semaphore_type)
    {
        result = pkE_INVALIDARG;
        goto exit;
    }
    else
    {
        retval = pthread_mutex_destroy(&(psem->Handle));
        if (!retval)
        {
            retval = pthread_cond_destroy(&(psem->Condition));
            if (!retval)
            {
                free(hHandle);
                result = pkS_OK;
            }
            else
            {
                result = pkE_UNEXPECTED;
            }
        }
        else
        {
            result = pkE_UNEXPECTED;
        }
    }

exit:
    return result;
}


//-----------------------------------------------------------------------------
// Events
//
// The state of a manual-reset event object remains signaled until it is set
// explicitly to the nonsignaled state by the Executive_ResetEvent function.
// Any number of waiting threads, or threads that subsequently begin wait
// operations for the specified event object by calling one of the wait functions,
// can be released while the object's state is signaled.
//
// The state of an auto-reset event object remains signaled until a single waiting
// thread is released, at which time the system automatically sets the state to
// nonsignaled. If no threads are waiting, the event object's state remains signaled.
//
//Setting an event that is already set has no effect.
//-----------------------------------------------------------------------------
#ifdef MCX_BUILDOPTION_MEMORYTRACKING
pkRESULT pkAPI Executive_CreateEvent_Tracked
(
    const char *pszIgnored,
    bool_t isManualReset,
    bool_t isSignaled,
    pkHANDLE *pHandle,
    const char *szFile,
    const char *szFunc,
    int line
)
#else
pkRESULT pkAPI Executive_CreateEvent
(
    const char *pszIgnored,
    bool_t isManualReset,
    bool_t isSignaled,
    pkHANDLE *pHandle
)
#endif
{
    pkRESULT result = pkE_FAIL;
    pal_sync_object_context * pEvent = NULL;
    int retval = 0;

    pkASSERT(NULL==pszIgnored);

    // Sanity check.
    if (NULL == pHandle)
    {
        PALPRINTMSG(PALPRINT_ERROR,("Executive_CreateEvent: NULL pHandle \n"));
        result = pkE_POINTER;
        goto exit;
    }

#ifdef MCX_BUILDOPTION_MEMORYTRACKING
    pEvent = (pal_sync_object_context*)malloc_Tracked(sizeof(pal_sync_object_context), szFile, szFunc, line);
#else
    pEvent = (pal_sync_object_context*)malloc(sizeof(pal_sync_object_context));
#endif

    if(NULL != pEvent)
    {
        memset(pEvent, 0, sizeof(pal_sync_object_context));

        pthread_mutex_init(&(pEvent->Handle), NULL);
        pthread_cond_init(&(pEvent->Condition), NULL);

        pthread_mutex_lock(&(pEvent->Handle));
        pEvent->bIsManualReset = isManualReset;
        pEvent->bSignaled = isSignaled;
        pEvent->Type = pal_event_type;

        if(isSignaled)
        {
            pthread_cond_signal(&(pEvent->Condition));
        }

        pthread_mutex_unlock(&(pEvent->Handle));
        result = pkS_OK;
    }

exit:
    if(pkS_OK == result)
    {
        *pHandle = (pkHANDLE *)pEvent;
    }

    return result;
}

pkRESULT pkAPI Executive_SetEvent( pkHANDLE hHandle )
{
    pkRESULT result = pkE_FAIL;
    pal_sync_object_context * pEvent = NULL;
    int retval = 0;

    if(NULL == hHandle)
    {
        result = pkE_POINTER;
        goto exit;
    }
    pEvent = (pal_sync_object_context *)hHandle;

    pthread_mutex_lock(&(pEvent->Handle));
    retval = pthread_cond_signal(&(pEvent->Condition));
    switch(retval)
    {
        case 0:
            pEvent->bSignaled = TRUE;
            result = pkS_OK;
            break;
        case EINVAL:
            result = pkE_INVALIDARG;
            break;
        default:
            PALPRINTMSG(PALPRINT_EXECUTIVE,("Executive_SetEvent - %x returned from pthread_cond_signal \n",retval));
            result = pkE_FAIL;
            break;
    }

    pthread_mutex_unlock(&(pEvent->Handle));

exit:
    return result;
}

pkRESULT pkAPI Executive_ResetEvent( pkHANDLE hHandle )
{
    pkRESULT result = pkE_FAIL;
    pal_sync_object_context * pEvent = NULL;
    int retval = 0;

    if(NULL == hHandle)
    {
        result = pkE_POINTER;
        goto exit;
    }
    pEvent = (pal_sync_object_context *)hHandle;

    retval = pthread_mutex_lock(&(pEvent->Handle));
    switch(retval)
    {
        case 0:
            pEvent->bSignaled = FALSE;
            result = pkS_OK;
            break;
        default:
            PALPRINTMSG(PALPRINT_EXECUTIVE,("Executive_ResetEvent - %x returned from pthread_mutex_lock \n", retval));
            result = pkE_FAIL;
            break;
    }

    pthread_mutex_unlock(&(pEvent->Handle));
exit:
    return result;
}

pkRESULT pkAPI Executive_WaitForEvent( pkHANDLE hHandle, uint32_t timeOutMs )
{
    pkRESULT result = pkE_FAIL;
    pal_sync_object_context *pEvent = (pal_sync_object_context *)hHandle;
    int retval = 0;
    struct timespec target;

    // Sanity checks.
    if (pEvent == NULL)
    {
        result = pkE_POINTER;
        goto exit;
    }
    else if (pEvent->Type != pal_event_type)
    {
        result = pkE_INVALIDARG;
        goto exit;
    }
    else
    {
        retval = pthread_mutex_lock(&(pEvent->Handle));
        PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("Executive_WaitForEvent - %x returned from pthread_mutex_lock \n",retval));
        if (retval)
        {
            result = pkE_FAIL;
            goto exit;
        }

        if (pEvent->bSignaled)
        {
            // If this event has been signaled, just return immediately.
            if (!pEvent->bIsManualReset)
            {
                // Only reset this flag if this is an auto-reset event. Otherwise, let the user
                // reset it explicitly by calling Executive_ResetEvent().
                pEvent->bSignaled = FALSE;
            }
            pthread_mutex_unlock(&(pEvent->Handle));
            result = pkS_OK;
            goto exit;
        }

         // Wait forever if specified.
        if (EXEC_WAIT_INFINITE == timeOutMs)
        {
            retval = pthread_cond_wait(&(pEvent->Condition), &(pEvent->Handle));
        }
        // Otherwise, calculate the expiration time and only wait until then.
        else
        {
            // Get the current time.
            clock_gettime(CLOCK_REALTIME, &target);

            if (0 != timeOutMs)
            {
                uint32_t seconds = timeOutMs / 1000;
                uint32_t milliseconds = timeOutMs % 1000;
                target.tv_sec += (seconds + ((target.tv_nsec + (milliseconds * 1000000)) / 1000000000));
                target.tv_nsec = ((target.tv_nsec + (milliseconds * 1000000)) % 1000000000);
            }
            else
            {
                // Note: You can't call pthread_cond_timedwait with a 0 delta from the value
                // you received from clock_gettime. Add 100 nsecs to the target expiration time
                // to make it happy.
                target.tv_sec += ((target.tv_nsec + 100) / 1000000000);
                target.tv_nsec = ((target.tv_nsec + 100) % 1000000000);
            }

            retval = pthread_cond_timedwait(&(pEvent->Condition), &(pEvent->Handle), &target);
        }

        switch(retval)
        {
            case 0:
                if (!pEvent->bIsManualReset)
                {
                    // If this is an auto-reset event, reset the flag since we now consumed it already.
                    // Otherwise, let the user reset it explicitly by calling Executive_ResetEvent().
                    pEvent->bSignaled = FALSE;
                }
                result = pkS_OK;
                break;
            case ETIMEDOUT:
                result = pkS_FALSE;
                break;
            case EINVAL:
                result = pkE_INVALIDARG;
                break;
            default:
                PALPRINTMSG(PALPRINT_EXECUTIVE,("Executive_WaitForEvent - %x returned from pthread_cond_timedwait \n",retval));
                result = pkE_FAIL;
                break;
        }

        // If this is a manual-reset event, make sure the event stays signaled
        // until it is reset.
        if(pEvent->bIsManualReset && pEvent->bSignaled)
        {
            pthread_cond_signal(&(pEvent->Condition));
        }

        pthread_mutex_unlock(&(pEvent->Handle));
    }

exit:
    return result;
}

pkRESULT pkAPI Executive_CloseEvent( pkHANDLE hHandle )
{
    pkRESULT result = pkE_FAIL;
    pal_sync_object_context * pEvent = NULL;
    int retval = 0;

    if(NULL == hHandle)
    {
        result = pkE_POINTER;
        goto exit;
    }
    pEvent = (pal_sync_object_context *)hHandle;

    retval = pthread_mutex_destroy(&(pEvent->Handle));
    if(!retval)
    {
        retval = pthread_cond_destroy(&(pEvent->Condition));
        if(!retval)
        {
            result = pkS_OK;
            free(hHandle);
        }
        else
        {
            result = pkE_FAIL;
        }
    }
    else
    {
        result = pkE_UNEXPECTED;
    }
exit:
    return result;
}

//-----------------------------------------------------------------------------
//  Time related operations
//-----------------------------------------------------------------------------
uint32_t pkAPI Executive_GetTickCount()
{

    struct timeval tvNow;
    gettimeofday(&tvNow, NULL);

    return (1000 * tvNow.tv_sec + tvNow.tv_usec/1000); // Strict int div on tv_usec
}


void pkAPI Executive_Sleep( uint32_t msecs )
{
    if (0 == msecs)
    {
        sched_yield();
    }
    else
    {
        struct timespec sTimeReq = {0};

        // Note that nanosleep can fail, or be awakened before the time interval
        // elapses. I am currently ignoring the failure case.
        sTimeReq.tv_sec = msecs / 1000;
        sTimeReq.tv_nsec = (long int)(msecs % 1000) * 1000000; // sTimeReq.tv_nsec is long int
        nanosleep(&sTimeReq, NULL);
    }
}

#define C_PERF_FREQUENCY 1000000ULL // 1 MHz to match the microsecond resolution in the timeval

uint64_t pkAPI Executive_GetPerformanceCounter(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)((tv.tv_sec * C_PERF_FREQUENCY) + tv.tv_usec);
}

uint64_t pkAPI Executive_GetPerformanceFrequency(void)
{
    return C_PERF_FREQUENCY;
}


//-----------------------------------------------------------------------------
// InterLocked Operations
//-----------------------------------------------------------------------------
int32_t pkAPI Executive_InterlockedIncrement(int32_t volatile* lpAddend)
{
#if defined (__i386__)

    __asm__ __volatile__("lock incl %0"
            : "+m"(*lpAddend)
            : : "memory");
    return *lpAddend;

#else

    int32_t retval = 0;

    pthread_mutex_lock(&s_InterlockedMutex);
    retval = ++(*lpAddend);
    pthread_mutex_unlock(&s_InterlockedMutex);
exit:
    return retval;

#endif
}

int32_t pkAPI Executive_InterlockedDecrement(int32_t volatile* lpAddend)
{
#if defined (__i386__)

    __asm__ __volatile__("lock decl %0"
            : "+m"(*lpAddend)
            : : "memory");
    return *lpAddend;

#else

    int32_t retval = 0;

    pthread_mutex_lock(&s_InterlockedMutex);
    retval = --(*lpAddend);
    pthread_mutex_unlock(&s_InterlockedMutex);
exit:
    return retval;

#endif
}

int32_t pkAPI Executive_InterlockedExchange(int32_t volatile* Target, int32_t Value)
{
#if defined (__i386__)

    __asm__ __volatile__("lock xchgl %0, %1"
            : "=r" (Value)
            : "m" (*Target), "0" (Value)
            : "memory");
    return Value;

#else

    int32_t retval = 0;

    pthread_mutex_lock(&s_InterlockedMutex);
    retval = *Target;
    *Target = Value;
    pthread_mutex_unlock(&s_InterlockedMutex);
exit:
    return retval;

#endif
}

int32_t pkAPI Executive_InterlockedExchangeAdd(int32_t volatile* Addend, int32_t Value)
{
#if defined (__i386__)

    __asm__ __volatile__("lock xaddl %0, %1"
            : "=r" (Value)
            : "m" (*Addend), "0" (Value)
            : "memory");
    return Value;

#else

    int32_t retval = 0;

    pthread_mutex_lock(&s_InterlockedMutex);
    retval = *Addend;
    *Addend += Value;
    pthread_mutex_unlock(&s_InterlockedMutex);
exit:
    return retval;

#endif
}

int32_t pkAPI Executive_InterlockedCompareExchange(int32_t volatile* Destination, int32_t Exchange, int32_t Comperand)
{
#if defined (__i386__)

    __asm__ __volatile__("lock cmpxchgl %1,%2"
            : "=a"(Comperand)
            : "r"(Exchange), "m"(*Destination), "0"(Comperand)
            : "memory");
    return Comperand;

#else

    int32_t retval = 0;

    pthread_mutex_lock(&s_InterlockedMutex);
    retval = *Destination;
    if(Comperand == *Destination)
    {
        *Destination = Exchange;
    }
    pthread_mutex_unlock(&s_InterlockedMutex);
exit:
    return retval;

#endif
}

void *  pkAPI Executive_InterlockedExchangePointer(void * volatile* pTarget, void *pValue)
{
#if defined (__i386__)

    __asm__ __volatile__("lock xchgl %0, %1"
            : "=r" (pValue)
            : "m" (*pTarget), "0" (pValue)
            : "memory");
    return pValue;

#else
    void * retval = 0;

    pthread_mutex_lock(&s_InterlockedMutex);
    retval = *pTarget;
    *pTarget = pValue;
    pthread_mutex_unlock(&s_InterlockedMutex);
exit:
    return retval;

#endif
}

void *  pkAPI Executive_InterlockedCompareExchangePointer(void * volatile* Destination, void * Exchange, void * Comperand)
{
#if defined (__i386__)

    __asm__ __volatile__("lock cmpxchgl %1,%2"
                : "=a"(Comperand)
                : "r"(Exchange), "m"(*Destination), "0"(Comperand)
                : "memory");
    return Comperand;

#else

    void * retval = 0;

    pthread_mutex_lock(&s_InterlockedMutex);
    retval = *Destination;
    if(Comperand == *Destination)
    {
        *Destination = Exchange;
    }
    pthread_mutex_unlock(&s_InterlockedMutex);
exit:
    return retval;

#endif
}

//-----------------------------------------------------------------------------
//  C Run-Time stuff
//-----------------------------------------------------------------------------
#ifdef MCX_BUILDOPTION_MEMORYTRACKING
void *  pkAPI   Executive_Alloc_Tracked(uint32_t cb,  bool_t fZeroInit, const char *szFile, const char *szFunc, int line)
#else
void *  pkAPI   Executive_Alloc(uint32_t cb,  bool_t fZeroInit)
#endif
{
    MemoryAllocation * pAlloc = NULL;

    PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("Executive_Alloc:  %d bytes\n", cb));
    PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("Size:             %d\n", sizeof(MemoryAllocation)));
    PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("Offset of uiSize: %d\n", ELEMENT_OFFSET(MemoryAllocation, uiSize)));
    PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("Offset of buffer: %d\n", ELEMENT_OFFSET(MemoryAllocation, buffer)));

#ifdef MCX_BUILDOPTION_MEMORYTRACKING
    pAlloc = (MemoryAllocation *)malloc_Tracked(cb + sizeof(MemoryAllocation), szFile, szFunc, line);
#else
    pAlloc = (MemoryAllocation *)malloc(cb + sizeof(MemoryAllocation));
#endif

    if(pAlloc)
    {
#ifdef MALLOC_RECORD
        pthread_mutex_lock(&s_PalMemMutex);
        pal_mem_alloced+=malloc_usable_size(pAlloc);
        pthread_mutex_unlock(&s_PalMemMutex);
#endif
        if(fZeroInit)
        {
            memset(pAlloc->buffer, 0, cb);
        }

        pAlloc->uiSize = cb;
        return &pAlloc->buffer;
    }

    return NULL;

}

#ifdef MCX_BUILDOPTION_MEMORYTRACKING
void *  pkAPI   Executive_ReAlloc_Tracked(void *pv, uint32_t cb, bool_t fZeroInit, const char *szFile, const char *szFunc, int line)
#else
void *  pkAPI   Executive_ReAlloc(void *pv, uint32_t cb, bool_t fZeroInit)
#endif
{
    if (pv == NULL)
    {
        return Executive_Alloc(cb, fZeroInit);
    }

    MemoryAllocation *pAlloc = (MemoryAllocation *)((uint32_t)pv - ELEMENT_OFFSET(MemoryAllocation, buffer));

    if (cb <= pAlloc->uiSize)
    {
        return &pAlloc->buffer;
    }

#ifdef MCX_BUILDOPTION_MEMORYTRACKING
    MemoryAllocation *pNew = (MemoryAllocation *)malloc_Tracked(cb + sizeof(MemoryAllocation), szFile, szFunc, line);
#else
    MemoryAllocation *pNew = (MemoryAllocation *)malloc(cb + sizeof(MemoryAllocation));
#endif

    if (pNew)
    {
        memcpy(pNew->buffer, pAlloc->buffer, pAlloc->uiSize);

        if(fZeroInit)
        {
            memset(pNew->buffer + pAlloc->uiSize, 0, cb-pAlloc->uiSize);
        }
#ifdef MALLOC_RECORD
        pthread_mutex_lock(&s_PalMemMutex);
        pal_mem_alloced += malloc_usable_size(pNew) - malloc_usable_size(pAlloc);
        pthread_mutex_unlock(&s_PalMemMutex);
#endif
        free(pAlloc);

        pNew->uiSize = cb;
        return &pNew->buffer;
    }

    return NULL;
}

void    pkAPI   Executive_Free(void *pv)
{
    if (NULL != pv)
    {
        MemoryAllocation *pAlloc = (MemoryAllocation *)((uint32_t)pv - ELEMENT_OFFSET(MemoryAllocation, buffer));
#ifdef MALLOC_RECORD
        pthread_mutex_lock(&s_PalMemMutex);
        pal_mem_alloced -= malloc_usable_size(pAlloc);
        pthread_mutex_unlock(&s_PalMemMutex);
#endif
        free(pAlloc);
    }
    return;
}

void pkAPI Executive_DebugPrintf( const char *pFmt, ... )
{
    FILE* fdLog = (NULL != s_fdDebugPrintf) ? s_fdDebugPrintf : stdout;

    if (s_bDebugOutputEnabled)
    {
        double secsTimeNow = s_SecondsSinceStart();

        pthread_mutex_lock(&s_PalDebugPrintfMutex);

        fprintf(fdLog, "%.3f [%.6f]: ", secsTimeNow, secsTimeNow - s_secsPrevPrintTime);

        s_secsPrevPrintTime = secsTimeNow;
        {
            va_list ap;

            va_start(ap, pFmt);
            vfprintf(fdLog, pFmt, ap);
            va_end(ap);
        }
        fflush(fdLog); // this is efficient in desktop with disk write buffering

        pthread_mutex_unlock(&s_PalDebugPrintfMutex);
    }
}

void pkAPI Executive_DebugOut( const wchar_t *pFmt)
{
    if (s_bDebugOutputEnabled)
    {
        fprintf(stderr, "%ls", pFmt);
        fflush(stderr);
    }
}

void pkAPI Executive_EnableDebugOutput(bool_t bEnable)
{
    s_bDebugOutputEnabled = bEnable;
}

//-----------------------------------------------------------------------------
//  Multi-byte conversion stuff
//-----------------------------------------------------------------------------

pkRESULT pkAPI Executive_MultiByteToWideChar(
            uint32_t codePage,
            uint32_t flags,
            int8_t *pMultiByteStr,
            int32_t cbMultiByte,
            wchar_t *pWideCharStr,
            int32_t cchWideChar,
            uint32_t *pWritten)
{
    size_t size;

    // Sanity check.
    // Note that we don't need to check the pointer "pWideCharStr" since the system call
    // "mbstowcs" can accept a NULL pointer as a valid case. If the pointer "pWideCharStr"
    // is NULL, "cchWideChar" would be ignored, and the conversion would proceed as normal,
    // except that the converted wide characters are not written out to memory, and that no
    // length limit exists.
    if (pMultiByteStr == NULL)
    {
        return pkE_POINTER;
    }

    size = mbstowcs(pWideCharStr, (char *)pMultiByteStr, cchWideChar);
    if (size != (size_t)(-1))
    {
        if (pWritten)
        {
            *pWritten = (uint32_t)size + 1;
        }
        return pkS_OK;
    }
    else
    {
        return pkE_UNEXPECTED;
    }
}

pkRESULT pkAPI Executive_WideCharToMultiByte(
            uint32_t codePage,
            uint32_t flags,
            const wchar_t *pWideCharStr,
            int32_t cchWideChar,
            int8_t *pMultiByteStr,
            int32_t cbMultiByte,
            int8_t *pDefaultChar,
            bool_t *pUsedDefaultChar,
            uint32_t *pWritten)
{
    size_t size;

    // Sanity checks.
    // Note that we don't need to check the pointer "pMultiByteStr" since the system call
    // "wcstombs" can accept a NULL pointer as a valid case. If the pointer "pMultiByteStr"
    // is NULL, "cbMultiByte" would be ignored, and the conversion would proceed as normal,
    // except that the converted wide characters are not written out to memory, and that no
    // length limit exists.
    if (pWideCharStr == NULL)
    {
        return pkE_POINTER;
    }

    size = wcstombs((char *)pMultiByteStr, pWideCharStr, cbMultiByte);
    if (size != (size_t)(-1))
    {
        if (pWritten)
        {
            *pWritten = (uint32_t)size + 1;
        }
        return pkS_OK;
    }
    else
    {
        return pkE_UNEXPECTED;
    }
}

//=============================================================================
// P R I V A T E    F U N C T I O N    D E F I N I T I O N S
//=============================================================================

pkRESULT Executive_Startup()
{
    pkRESULT retval = pkS_OK;

    pthread_mutex_init(&s_PalDebugPrintfMutex, NULL);

    s_SecondsSinceStart(); // establish start time

    if(!s_bExecutiveInitialized)
    {
#ifdef MCX_BUILDOPTION_MEMORYTRACKING
        // initialize the allocations
        InitMemoryAllocations();
#endif
        // Initialize mutexes.

#if !defined (__i386__) // interlock mutex is not used in i386 case
        pthread_mutex_init(&s_InterlockedMutex, NULL);
#endif

        pthread_mutex_init(&s_PalThreadContextMutex, NULL);

#ifdef MALLOC_RECORD
        pthread_mutex_init(&s_PalMemMutex, NULL);
#endif // MALLOC_RECORD

        // Seed the random number generator.
        uint32_t seed = time(NULL);
        PALPRINTMSG(PALPRINT_EXECUTIVE,("Initialize the random number generator with seed: %d\n", seed));
        srand(seed);

        // default socket error value for main thread
        s_MainThread_iLastSocketError = pkS_OK;

        s_bExecutiveInitialized = TRUE;

        //Set up debug output file if specified in environment variable
        {
            const char* szDbgFileName = getenv(DEBUG_PRINTF_LOGFILE_NAME_ENV_VARIABLE);
            if (NULL == szDbgFileName)
            {
                PALPRINTMSG(PALPRINT_EXECUTIVE,("Debug log file env var %s not set\nDebug output going to stdout\n",
                    DEBUG_PRINTF_LOGFILE_NAME_ENV_VARIABLE));
            }
            else
            {
                s_fdDebugPrintf = fopen(szDbgFileName, "w+");
                if (NULL == s_fdDebugPrintf)
                {
                    PALPRINTMSG(PALPRINT_EXECUTIVE,("Failed to open log file %s\fopen errno: %d\n",
                        szDbgFileName, errno));
                }
                else
                {
                    PALPRINTMSG(PALPRINT_EXECUTIVE,("Debug log file: %s\n", szDbgFileName));
                }
            }
        }
    }
    else
    {
        retval = pkE_FAIL;
        PALPRINTMSG(PALPRINT_EXECUTIVE,("Executive_Startup error: already initialized!\n"));
    }

exit:
    if(!s_bExecutiveInitialized)
    {
        retval = pkE_FAIL;
        PALPRINTMSG(PALPRINT_ERROR,("Executive_Startup failed!\n"));
        assert(0);
    }
    return retval;
}

pkRESULT Executive_Shutdown()
{
    pkRESULT retval = pkS_OK;

    if(s_bExecutiveInitialized)
    {
#if !defined (__i386__) // interlock mutex is not used in i386 case
        pthread_mutex_destroy(&s_InterlockedMutex);
#endif
        pthread_mutex_destroy(&s_PalThreadContextMutex);

#ifdef MALLOC_RECORD
        pthread_mutex_destroy(&s_PalMemMutex);
#endif // MALLOC_RECORD

        s_bExecutiveInitialized = FALSE;
    }
    else
    {
        retval = pkE_FAIL;
        PALPRINTMSG(PALPRINT_ERROR,("Executive_Shutdown error: not initialized!\n"));
    }

#ifdef MCX_BUILDOPTION_MEMORYTRACKING
    // make sure all threads are gone!!
    MCX_DEBUG_Check_All_Threads();

    // check memory allocations
    CheckMemoryAllocations();

    // cleanup
    TermMemoryAllocations();
#endif

    // Finally shut down the debug output
    if (NULL != s_fdDebugPrintf)
    {
        fclose(s_fdDebugPrintf);
        s_fdDebugPrintf = NULL;
    }

    pthread_mutex_destroy(&s_PalDebugPrintfMutex);

    return retval;
}

//=============================================================================
// L O C A L    F U N C T I O N    D E F I N I T I O N S
//=============================================================================

// Add the given thread context to the linked list.
static void s_AddPalThreadContext(pal_thread_context *pCtxToAdd)
{
    pthread_mutex_lock(&s_PalThreadContextMutex);
    // Sanity check.
    if (!pCtxToAdd)
    {
        PALPRINTMSG(PALPRINT_ERROR,("s_AddPalThreadContext: NULL pointer.\n"));
    }
    else
    {
        // Add this context to the head of the list.
        pCtxToAdd->pNext = s_pPalThreadContextListHead;
        s_pPalThreadContextListHead = pCtxToAdd;
        PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("%s: thread id %u added.\n", __FUNCTION__, pCtxToAdd->ThreadId));
    }
    pthread_mutex_unlock(&s_PalThreadContextMutex);
    return;
}

// Look in the linked list for a particular thread context by its ID.
static pal_thread_context *s_GetPalThreadContextByID(pthread_t ThreadID)
{
    pthread_mutex_lock(&s_PalThreadContextMutex);
    pal_thread_context *ctx = s_pPalThreadContextListHead;

    // Traverse the list from the head and look for the matched ID.
    while (ctx != NULL)
    {
        // Found the thread context!
        if(ctx->ThreadId == ThreadID)
        {
            break;
        }

        ctx = ctx->pNext;
    }

    pthread_mutex_unlock(&s_PalThreadContextMutex);
    return(ctx);
}

// Remove the given thread context from the linked list.
static void s_RemovePalThreadContext(pal_thread_context *pCtxToRemove)
{
    if (NULL == pCtxToRemove)
    {
        PALPRINTMSG(PALPRINT_ERROR,("s_RemovePalThreadContext: NULL pointer.\n"));
        return;
    }

    pthread_mutex_lock(&s_PalThreadContextMutex);

    // Extract the context from the list if it is there
    {
        pal_thread_context *pCtxPrev = NULL;
        pal_thread_context *pCtxNext = s_pPalThreadContextListHead;

        while (pCtxNext != NULL)
        {
            // Remove this context from the list.
            if(pCtxNext == pCtxToRemove)
            {
                // Is this the head of the list?
                if (pCtxNext == s_pPalThreadContextListHead)
                {
                    s_pPalThreadContextListHead = s_pPalThreadContextListHead->pNext;
                }
                else // extract from list
                {
                    pCtxPrev->pNext = pCtxNext->pNext;
                }

                break;
            }

            pCtxPrev = pCtxNext;
            pCtxNext = pCtxNext->pNext;
        }

        if (NULL == pCtxNext)
        {
            PALPRINTMSG(PALPRINT_WARNING,("s_RemovePalThreadContext: thread with id %u not found on list.\n",
                pCtxToRemove->ThreadId));
        }
    }

    PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("%s: thread id %u removed.\n", __FUNCTION__, pCtxToRemove->ThreadId));

    // destroy pthread objects
    pthread_mutex_destroy(&(pCtxToRemove->PalThreadMutex));
    pthread_cond_destroy(&(pCtxToRemove->Condition));
    //free up the context object
    Executive_Free(pCtxToRemove);

    pthread_mutex_unlock(&s_PalThreadContextMutex);
}

static void s_ThreadExitCleanup(void *pvContext)
{
    pal_thread_context *pThreadContext  = (pal_thread_context*)pvContext;
    if(pThreadContext)
    {
        //Exited thread..signal it.
        pthread_mutex_lock(&(pThreadContext->PalThreadMutex));
        pthread_cond_broadcast(&(pThreadContext->Condition));
        pThreadContext->bThreadExited = TRUE;
        pthread_mutex_unlock(&(pThreadContext->PalThreadMutex));
    }
}

//This functions will return the memory allocated now.
//if memory malloc/free recorded
//  return what we recorded.
//else
//  return memory "allocated" in current process.
//endif
unsigned long GetMemoryAllocated()
{
#ifdef MALLOC_RECORD
    return pal_mem_alloced;
#else
    struct mallinfo mi;
    mi = mallinfo();
    return (unsigned long)(mi.uordblks+mi.hblkhd);
#endif
}


// This will wrap the pthread, and allow us to have an
// eventing object that can be signalled when the thread
// exits to support Executive_WaitForThread(). This wrapper
// function will also provide a place to store the threadID,
// which is needed for various thread handling functions.

void* s_ThreadExecWrapper(void* pvContext)
{
    int oldtype;
    pal_thread_context *pThreadContext = (pal_thread_context *)pvContext;

    if(NULL == pThreadContext)
    {
        PALPRINTMSG(PALPRINT_ERROR,("%s provided null context and simply exiting\n", __FUNCTION__));
    }
    else
    {
        pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &oldtype);
        pthread_cleanup_push(s_ThreadExitCleanup, pThreadContext);

        pthread_mutex_lock(&(pThreadContext->PalThreadMutex));

        if (pThreadContext->ThreadId != pthread_self())
        {
            PALPRINTMSG(PALPRINT_WARNING,("%s context ThreadId %u != self (%u)\n", __FUNCTION__,
                pThreadContext->ThreadId, pthread_self()));

            pThreadContext->ThreadId = pthread_self();
        }

        pThreadContext->ProcessId = (pid_t)syscall(__NR_gettid); // returns PID of thread process

        pthread_mutex_unlock(&(pThreadContext->PalThreadMutex));

        PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("Launching wrapped thread with ThreadId: %u\n",
            pThreadContext->ThreadId));

        (pThreadContext->ThreadParams.Function)(pThreadContext->ThreadParams.pParams);

        PALPRINTMSG(PALPRINT_EXECUTIVE_VERBOSE,("Returning from wrapped thread with ThreadId: %u\n",
            pThreadContext->ThreadId));

        pthread_cleanup_pop(1);
        pthread_setcanceltype(oldtype, NULL);
    }

    pthread_exit(NULL);
    return NULL;
}


//=============================================================================
// SetThreadLastSocketError_priv
//
// This finds the pal_thread_context structure for the current thread, and
// sets the iLastSocketError member of that structure.
//=============================================================================
void SetThreadLastSocketError_priv(pkRESULT lastError)
{
    pal_thread_context * pThreadContext = NULL;

    // don't make this function call until AFTER we've used errno
    pThreadContext = s_GetPalThreadContextByID(pthread_self());

    if (NULL != pThreadContext)
    {
        pThreadContext->iLastSocketError = lastError;
    }
    else
    {
        s_MainThread_iLastSocketError = lastError;
    }
}

//=============================================================================
// GetThreadLastSocketError_priv
//
// This finds the pal_thread_context structure for the current thread, and
// returns the iLastSocketError member of that structure.
//=============================================================================
pkRESULT GetThreadLastSocketError_priv(void)
{
    pal_thread_context * pThreadContext = s_GetPalThreadContextByID(pthread_self());

    if (NULL != pThreadContext)
    {
        return pThreadContext->iLastSocketError;
    }
    else
    {
        return s_MainThread_iLastSocketError;
    }
}



//=============================================================================
//=============================================================================
#ifdef MCX_BUILDOPTION_MEMORYTRACKING
void MCX_DEBUG_Check_All_Threads(void)
{
    pal_thread_context *pThread = s_pPalThreadContextListHead;
    while (pThread)
    {
        Executive_DebugPrintf("################################################\n");
        Executive_DebugPrintf("##\n");
        Executive_DebugPrintf("## Thread leak found!\n");
        Executive_DebugPrintf("##   function: %s\n", pThread->szFunc);
        Executive_DebugPrintf("##   file:     %s\n", pThread->szFile);
        Executive_DebugPrintf("##   line:     %d\n", pThread->line);
        Executive_DebugPrintf("##\n");
        Executive_DebugPrintf("################################################\n");

        pThread=pThread->pNext;
    }
}
#endif


