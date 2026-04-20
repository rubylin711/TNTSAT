///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  pkMem.cpp
//
//  Implementation of the methods for tracking C++ resource allocations.
//
//  This module contains replacements for malloc/free and new/delete with
//  versions that track the allocations and where they came from (source file
//  and line number).
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

#include <palPrint.h>

#include <stdarg.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>

#include <palMemTrack.h> // TODO: harrypy is include order important here???

//=============================================================================
// The code in this module is only relevant if the MCX_BUILDOPTION_MEMORYTRACKING
// macro is defined
//=============================================================================
#ifdef MCX_BUILDOPTION_MEMORYTRACKING

//=============================================================================
// This module makes use of C++ specific compiler extensions, but since it's
// in the PAL, it gets compiled both as C and as C++. In order to avoid a
// build break in the C pass, we #ifdef out all of the modules code for a
// C compile
//=============================================================================
#ifdef __cplusplus


//=============================================================================
// CriticalSection
//
// This class is being implemented outside of the Exectutive PAL because we
// don't want to consume tracked resources in the code that is actually doing
// the resource tracking
//=============================================================================
class CriticalSection
{
public:
    bool Initialize(void);
    void Terminate(void);
    void Lock(void);
    void Unlock(void);

private:
    pthread_mutex_t     m_mutex;
};


//=============================================================================
// CriticalSection::Initialize
//
// Initialize the internal data structures for a critical section.
//=============================================================================
bool CriticalSection::Initialize(void)
{
    int icode;
    pthread_mutexattr_t attributes;

    // clear the mutex structure
    memset(&this->m_mutex, 0, sizeof(this->m_mutex));

    // set an attribute so that the lock is recursive. this will allow it
    // to keep a refcount and allow the thread that owns the lock to lock
    // it multiple times
    icode=pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE_NP);
    if (0 != icode)
    {
        // there was an error setting up a mutex attribute structure
        return false;
    }

    // initialize the mutex
    icode=pthread_mutex_init(&this->m_mutex, &attributes);
    if (0 != icode)
    {
        // there was an error initializing the mutex
        pthread_mutexattr_destroy(&attributes);
        return false;
    }

    // the mutex attribute structure is no longer needed
    icode=pthread_mutexattr_destroy(&attributes);
    if (0 != icode)
    {
        // there was an error destroying the mutex attribute structure
        pthread_mutex_destroy(&this->m_mutex);
        return false;
    }

    return true;
}

//=============================================================================
// CriticalSection::Terminate
//
// Clean up the internal data structures for a critical section.
//=============================================================================
void CriticalSection::Terminate(void)
{
    pthread_mutex_destroy(&this->m_mutex);
}

//=============================================================================
// CriticalSection::Lock
//
// Lock the critical section. If another thread tries to lock it while we
// have it locked, it will wait until we unlock it. If we try to lock it again,
// we will succeed, but we will increment a refcount and will need to unlock
// it the same number of times before another thread can lock it.
//=============================================================================
void CriticalSection::Lock(void)
{
    pthread_mutex_lock(&this->m_mutex);
}

//=============================================================================
// CriticalSection::Unlock
//
// Unlock the critical section.
//=============================================================================
void CriticalSection::Unlock(void)
{
    pthread_mutex_unlock(&this->m_mutex);
}


//==================================================================================
// undefine the macros for memory allocations so that we can use the real versions
// of these in this module
//==================================================================================
#ifdef new
#undef new
#endif

#ifdef delete
#undef delete
#endif

#ifdef malloc
#undef malloc
#endif

#ifdef free
#undef free
#endif

//==================================================================================
// Linked list keeping track of all allocations
//==================================================================================
struct MCX_DEBUG_MemoryAllocation
{
    void *                              address;        // the address as the app sees it (not necessarily
                                                        // the whole allocation
    uint32_t                            size;           // the size as seen by the application
    bool                                bnew;           // was it created with a "new"
    const char *                        file;
    const char *                        func;
    int                                 line;
    struct MCX_DEBUG_MemoryAllocation * next;
};

#define DEBUG_MEMORY_MAX_ALLOCATIONS                    4000

static struct MCX_DEBUG_MemoryAllocation allocations[DEBUG_MEMORY_MAX_ALLOCATIONS];
static struct MCX_DEBUG_MemoryAllocation *allocations_used=NULL;
static struct MCX_DEBUG_MemoryAllocation *allocations_free=NULL;
static CriticalSection s_AllocationListLock;

//==================================================================================
// Additional memory allocation statistics
//==================================================================================
static uint32_t MCX_DEBUG_MemoryAllocationsTotal = 0;
static uint32_t MCX_DEBUG_MemoryAllocationsMax = 0;
static uint32_t MCX_DEBUG_MemoryAllocationsCurrent = 0;
static uint32_t MCX_DEBUG_MemoryUsedMax = 0;
static uint32_t MCX_DEBUG_MemoryUsedCurrent = 0;
static uint32_t MCX_DEBUG_MemoryUsedPlusFragmentationMax = 0;

//==================================================================================
// Internal functions for tracking memory allocation
//==================================================================================
static void * AddNewAllocation(const char* szFile, const char* szFunc, int line, void *pMem, uint32_t cb, bool bnew);
static void * MarkAllocationFreed(void *pMem, const char* szFile=NULL, const char* szFunc=NULL, int line=0);
static void PrintInfoAboutAllocation(struct MCX_DEBUG_MemoryAllocation *pThis, const char *szMessage);

//==================================================================================
// Replacement functions for the standard memory allocation functions (malloc/free
// anb new/delete)
//==================================================================================
extern "C" void * malloc_Tracked(size_t size, const char *szFile, const char *szFunc, int line);
extern "C" void free_Tracked(void *p, const char *szFile, const char *szFunc, int line);

extern void * operator new(size_t size, const char *file, const char *func, int line);
extern void * operator new[](size_t size, const char *file, const char *func, int line);
extern void operator delete(void *p);
extern void operator delete[](void *p);

//==================================================================================
// The "new" new
//
// This function takes the filename, function name, and line number of the new,
// which was called by #define'ing new differently, and stores the information
// about the allocation in a linked list of tracked allocations.
//==================================================================================
void * operator new(size_t size, const char *szFile, const char *szFunc, int line)
{
    return AddNewAllocation(szFile, szFunc, line, malloc(size), size, true);
}

//==================================================================================
// The "new" new[]
//
// This function takes the filename, function name, and line number of the new,
// which was called by #define'ing new differently, and stores the information
// about the allocation in a linked list of tracked allocations.
//==================================================================================
void * operator new[](size_t size, const char *szFile, const char *szFunc, int line)
{
    return AddNewAllocation(szFile, szFunc, line, malloc(size), size, true);
}

//==================================================================================
// The "new" delete
//
// This function looks through the list of tracked allocations and tries to find
// the one we're trying to delete. If found, it removes it from the list, and frees
// the memory associated with it.
//==================================================================================
void operator delete(void *p)
{
    free(MarkAllocationFreed(p));
}

//==================================================================================
// The "new" delete[]
//
// This function looks through the list of tracked allocations and tries to find
// the one we're trying to delete. If found, it removes it from the list, and frees
// the memory associated with it.
//==================================================================================
void operator delete[](void *p)
{
    free(MarkAllocationFreed(p));
}


//==================================================================================
// The "new" malloc
//
// This function takes the filename, function name, and line number of the malloc,
// which was called by #define'ing malloc differently, and stores the information
// about the allocation in a linked list of tracked allocations.
//==================================================================================
void * malloc_Tracked(size_t size, const char *szFile, const char *szFunc, int line)
{
    return AddNewAllocation(szFile, szFunc, line, malloc(size), size, false);
}

//==================================================================================
// The "new" free
//
// This function looks through the list of tracked allocations and tries to find
// the one we're trying to free. If found, it removes it from the list, and frees
// the memory associated with it.
//==================================================================================
void free_Tracked(void *p, const char *szFile, const char *szFunc, int line)
{
    free(MarkAllocationFreed(p, szFile, szFunc, line));
}



//==================================================================================
// AddNewAllocation
//
// This function takes a newly allocated memory buffer and tracks information
// about it in a linked list of tracked allocations.
//==================================================================================
static void * AddNewAllocation(const char* szFile, const char* szFunc, int line, void *pMem, uint32_t cb, bool bnew)
{
    s_AllocationListLock.Lock();

    // grab the first allocation from the free list
    struct MCX_DEBUG_MemoryAllocation *pThis = allocations_free;

    // did we get one?
    if (pThis)
    {
        // remove this allocation from the free list, and add it to the used list
        allocations_free = pThis->next;
        pThis->next = allocations_used;
        allocations_used = pThis;

        // save off the file and function names
        pThis->file = szFile;
        pThis->func = szFunc;

        // save off other info
        pThis->line = line;
        pThis->address = pMem;
        pThis->size = cb;
        pThis->bnew = bnew;
    }
    else
    {
        // we never found an allocation to use!!
        Executive_DebugPrintf("MemoryTracking: Trying to allocate memory, but no empty allocation object found! %s:%d\n", szFile, line);
    }
    

    // some memory tracking stuff
    MCX_DEBUG_MemoryAllocationsTotal++;
    MCX_DEBUG_MemoryAllocationsCurrent++;

    if (MCX_DEBUG_MemoryAllocationsCurrent > MCX_DEBUG_MemoryAllocationsMax)
        MCX_DEBUG_MemoryAllocationsMax = MCX_DEBUG_MemoryAllocationsCurrent;

    MCX_DEBUG_MemoryUsedCurrent += cb;
    if (MCX_DEBUG_MemoryUsedCurrent > MCX_DEBUG_MemoryUsedMax)
        MCX_DEBUG_MemoryUsedMax = MCX_DEBUG_MemoryUsedCurrent;

    // now, calculate the upper and lower bounds of all active allocations, and see
    // if the MemoryPlusFragmentation has changed.
    do
    {
        uint32_t memoryBot = 0xFFFFFFFF;
        uint32_t memoryTop = 0;
        uint32_t memoryTotal = 0;

        pThis = allocations_used;
        while (pThis)
        {
            uint32_t thisBot = (uint32_t)pThis->address;
            uint32_t thisTop = (uint32_t)pThis->address + pThis->size;

            if (memoryBot > thisBot)
                memoryBot = thisBot;

            if (memoryTop < thisTop)
                memoryTop = thisTop;

            pThis = pThis->next;
        }

        memoryTotal = memoryTop - memoryBot;

        if (MCX_DEBUG_MemoryUsedPlusFragmentationMax < memoryTotal)
            MCX_DEBUG_MemoryUsedPlusFragmentationMax = memoryTotal;

    } while (FALSE);

    s_AllocationListLock.Unlock();
#if PALPRINT_EXECUTIVE_VERBOSE
    Executive_DebugPrintf("MemoryTracking: Allocated memory %s:%d 0x%08X\n", szFile, line, pMem);
#endif 
    return pMem;
}

//==================================================================================
// MarkAllocationFreed
//
// When an app tries to free memory, we call this function to look for it in the
// list of tracked allocations. If found, we delete the structure with the tracking
// info (and add it to an "empty" list for future re-use).
//==================================================================================
static void * MarkAllocationFreed(void *pMem, const char* szFile, const char* szFunc, int line)
{
    s_AllocationListLock.Lock();

    // see if it's on the list
    struct MCX_DEBUG_MemoryAllocation *pThis=allocations_used;

    while (pThis)
    {
        if (pThis->address == pMem)
            break;

        pThis = pThis->next;
    }

    // did we find it?
    if (pThis)
    {
        // tracking stuff...
        MCX_DEBUG_MemoryAllocationsCurrent--;
        MCX_DEBUG_MemoryUsedCurrent -= pThis->size;
    }
    else
    {
        // we never found the allocation!
        Executive_DebugPrintf("MemoryTracking: Memory freed from unknown allocation! 0x%08X\n", pMem);
    }

    // now, move it to the free list
    if (pThis)
    {
        struct MCX_DEBUG_MemoryAllocation **ppThis = &allocations_used;

        while (*ppThis != pThis)
        {
            ppThis = &((*ppThis)->next);
        }

        if (*ppThis == pThis)
        {
            *ppThis = pThis->next;
            pThis->next = allocations_free;
            allocations_free = pThis;
        }
    }

    s_AllocationListLock.Unlock();

#if PALPRINT_EXECUTIVE_VERBOSE

    if ( NULL == szFile )
    {
        Executive_DebugPrintf("MemoryTracking: Freed memory 0x%08X\n", pMem);
    }
    else
    {
        Executive_DebugPrintf("MemoryTracking: Freed memory %s:%d 0x%08X\n", szFile, line,pMem);
    }
#endif // PALPRINT_EXECUTIVE_VERBOSE

    return pMem;
}

//==================================================================================
// CheckMemoryAllocations
//
// Called when the app is done with just about everything, this function looks
// for tracked allocations. If any exist, that means they haven't been freed yet,
// and are leaks. For all of these, print information about the source of the
// allocation.
//
// Then, print out some statistics about memory usage overall that we've been
// keeping track of.
//==================================================================================
void CheckMemoryAllocations(void)
{
    // check all allocations to determine which, if any, are still active
    struct MCX_DEBUG_MemoryAllocation * pThis = allocations_used;

    s_AllocationListLock.Lock();
    while (pThis)
    {
        PrintInfoAboutAllocation(pThis, pThis->bnew ? "Memory leak (new) found:" : "Memory leak (malloc) found:");
        pThis = pThis->next;
    }
    s_AllocationListLock.Unlock();

    // now, print some interesting stuff...
    Executive_DebugPrintf("################################################\n");
    Executive_DebugPrintf("##\n");
    Executive_DebugPrintf("## Memory allocation report:\n");
    Executive_DebugPrintf("##   total allocations:                     %d\n", MCX_DEBUG_MemoryAllocationsTotal);
    Executive_DebugPrintf("##   max allocations:                       %d\n", MCX_DEBUG_MemoryAllocationsMax);
    Executive_DebugPrintf("##   max memory used (in allocations):      %d\n", MCX_DEBUG_MemoryUsedMax);
    Executive_DebugPrintf("##   max memory used (incl. fragmentation): %d\n", MCX_DEBUG_MemoryUsedPlusFragmentationMax);
    Executive_DebugPrintf("##\n");
    Executive_DebugPrintf("################################################\n");
}



//==================================================================================
// PrintInfoAboutAllocation
//
// Print out information about a tracked memory allocation that was determined to
// be a leak
//==================================================================================
void PrintInfoAboutAllocation(struct MCX_DEBUG_MemoryAllocation *pThis, const char *szMessage)
{
    Executive_DebugPrintf("%s file: %s:%d function: %s addr: 0x%08X size: %d\n", 
                 szMessage,
                 pThis->file, 
                 pThis->line, 
                 pThis->func, 
                 pThis->address, 
                 pThis->size);

    return;
}

//==================================================================================
// InitMemoryAllocations
//
// Prepare the data strucures used to track memory allocations
//==================================================================================
void InitMemoryAllocations(void)
{
    int i;

    s_AllocationListLock.Initialize();
    s_AllocationListLock.Lock();

    for (i=0 ; i<DEBUG_MEMORY_MAX_ALLOCATIONS-1 ; i++)
    {
        allocations[i].next = & allocations[i+1];
    }
    allocations[DEBUG_MEMORY_MAX_ALLOCATIONS-1].next = NULL;
    allocations_free = &allocations[0];
    allocations_used=NULL;

    s_AllocationListLock.Unlock();
}

//==================================================================================
// TermMemoryAllocations
//
// Clean up the data strucures used to track memory allocations
//==================================================================================
void TermMemoryAllocations(void)
{
    s_AllocationListLock.Terminate();
}


#endif // __cplusplus
#endif // MCX_BUILDOPTION_MEMORYTRACKING
