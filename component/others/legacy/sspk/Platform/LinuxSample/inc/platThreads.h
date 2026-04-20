///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __platThreads_h__
#define __platThreads_h__

#define PK_THREAD_TIMEOUT 5000

#define pkEXECUTIVE_THREAD_PRIORITY_TIME_CRITICAL   247
#define pkEXECUTIVE_THREAD_PRIORITY_HIGHEST         248
#define pkEXECUTIVE_THREAD_PRIORITY_HIGH            249
#define pkEXECUTIVE_THREAD_PRIORITY_NORMAL          251
#define pkEXECUTIVE_THREAD_PRIORITY_LOW             253
#define pkEXECUTIVE_THREAD_PRIORITY_LOWEST          254

/* thread entry point looks like this */
typedef uint32_t pkAPI (*THREAD_ENTRY)(void *pParam);


#endif /*__platThreads_h__*/
