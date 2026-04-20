/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __ADP_BT_MEM_H__
#define __ADP_BT_MEM_H__

/* add include here */
#include "mt_type.h"
#include <malloc.h>
#include "malloc.h"
#ifdef __cplusplus
extern "C" {
#endif

/***************************** Macro Definition ******************************/
#define MTGO_Malloc(size)       malloc(size)
#define MTGO_Free(pVirm)       free(pVirm) //(pVirm = pVirm)

#if 0  //sym6 smp test --- temply disabled by dean
#define MTGO_MMZ_Malloc(size, ppPhyAddr)   MTGO_Malloc2(size, ppPhyAddr)
#define MTGO_MMZ_Free(pVirm)    MTGO_Free2(pVirm)
#else
#define MTGO_MMZ_Malloc(size, pPhyAddr)   MTGO_Malloc_New(size, pPhyAddr)
#define MTGO_MMZ_Free(pVirm)    MTGO_Free_New(pVirm)
#endif

/*************************** Structure Definition ****************************/


typedef struct _BT_FREE_S
{
	mt_u32 u32Size;
    struct _BT_FREE_S *pNext;
}BT_FREE_S;

/********************** Global Variable declaration **************************/


/******************************* API declaration *****************************/

mt_s32 MTGO_InitMemory(mt_void);

mt_void* MTGO_Malloc2(mt_u32 u32Size, phys_addr_t *ppPhyAddr);

mt_void MTGO_Free2(mt_void* pAddr);

mt_void* MTGO_Malloc_New(mt_u32 u32Size, phys_addr_t *pPhyAddr);

mt_void MTGO_Free_New(mt_void* pAddr);

mt_void MTGO_DeInitMemory(mt_void);

BT_FREE_S* MTGO_GetFreeList(mt_void);
#ifdef __cplusplus
}
#endif
#endif /* __ADP_BT_MEM_H__ */

