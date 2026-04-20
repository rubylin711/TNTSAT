///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#ifndef PLAT_MEMTRACK_H_
#define PLAT_MEMTRACK_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MCX_BUILDOPTION_MEMORYTRACKING

#define malloc(size) malloc_Tracked(size, __FILE__, __FUNCTION__, __LINE__)
#define free(pointer) free_Tracked(pointer, __FILE__, __FUNCTION__, __LINE__)

extern void * malloc_Tracked(size_t size, const char *szFile, const char *szFunc, int line);
extern void free_Tracked(void *p, const char *szFile, const char *szFunc, int line);

extern void InitMemoryAllocations(void);
extern void CheckMemoryAllocations(void);
extern void TermMemoryAllocations(void);

#endif

#ifdef __cplusplus
}
#endif

#endif /* PLAT_MEMTRACK_H_ */
