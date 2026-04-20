/*****************************************************************************/
/*                Copyright 2009 - 2014, Huawei Tech. Co., Ltd.              */
/*                           ALL RIGHTS RESERVED                             */
/*                                                                           */
/* FileName: wmalloc.c                                                       */
/* Version: 01a How to realize Memory management                                              */
/*                                                                           */
/* History:                                                                  */
/* 1. 01a,2009-2-25, w54130 Create this file.                                 */
/*****************************************************************************/
#include "tde_define.h"
#include "wmalloc.h"
#define SIZE_256BYTE_ALIGN

typedef struct _MemoryBlock
{
    mt_u32          nSize;
    mt_u16          nFree;
    mt_u16          nFirst;
    mt_u16          nUnitSize;
#if MT_TDE_MEMCOUNT_SUPPORT    
    mt_u16          nMaxUsed;           /* Max used unit number             */
    mt_u16          nMaxNum;            /* Max unit number                  */
#endif    
    mt_u8*          pStartAddr;
    struct _MemoryBlock*    pNext;
}MemoryBlock;

typedef enum 
{
    UNIT_SIZE_CMD = 0,
    UNIT_SIZE_JOB,  
    UNIT_SIZE_NODE,
    UNIT_SIZE_FILTER,
    UNIT_SIZE_BUTT
}UNIT_SIZE_E;

STATIC MemoryBlock g_struMemBlock[UNIT_SIZE_BUTT]; 
#ifndef TDE_BOOT
STATIC spinlock_t s_MemLock;
#endif
#ifndef SIZE_256BYTE_ALIGN
#define CMD_SIZE 64
#define JOB_SIZE 96
#define NODE_SIZE 208
#define FILTER_SIZE 960
#else
#define CMD_SIZE 64
#define JOB_SIZE 96
#define NODE_SIZE 1536
#define FILTER_SIZE 2048
#endif
#define PRINTMEMINFO() do\
{\
    TDE_TRACE(TDE_KERN_DEBUG, "-----------------------------------------------------\n");\
    TDE_TRACE(TDE_KERN_DEBUG, "\tMemBlock Info\ttotal\tfree\n");\
    TDE_TRACE(TDE_KERN_DEBUG, "\t%d\t%d\t%d\n",CMD_SIZE, MT_TDE_CMD_NUM, g_struMemBlock[UNIT_SIZE_CMD].nFree);\
    TDE_TRACE(TDE_KERN_DEBUG, "\t%d\t%d\t%d\n",JOB_SIZE, MT_TDE_JOB_NUM, g_struMemBlock[UNIT_SIZE_JOB].nFree);\
    TDE_TRACE(TDE_KERN_DEBUG, "\t%d\t%d\t%d\n",NODE_SIZE, MT_TDE_NODE_NUM, g_struMemBlock[UNIT_SIZE_NODE].nFree);\
    TDE_TRACE(TDE_KERN_DEBUG, "\t%d\t%d\t%d\n",FILTER_SIZE, MT_TDE_FILTER_NUM, g_struMemBlock[UNIT_SIZE_FILTER].nFree);\
    TDE_TRACE(TDE_KERN_DEBUG, "------------------------------------------------------\n");\
}while(0)


mt_s32 MemoryBlockInit(UNIT_SIZE_E eUnitSize, mt_u32 nUnitNum, mt_u8 *pAddr) 
{
    mt_u16 i;
    mt_u8 *pData = pAddr;
    if((eUnitSize >= UNIT_SIZE_BUTT) || (NULL == pAddr ) || (0 == nUnitNum))
    {
        return MT_FAILURE;
    }
    if(UNIT_SIZE_CMD== eUnitSize)
    {
        g_struMemBlock[eUnitSize].nUnitSize = CMD_SIZE;
    }
    else if(UNIT_SIZE_JOB== eUnitSize)
    {
        g_struMemBlock[eUnitSize].nUnitSize = JOB_SIZE;
    }
    else if(UNIT_SIZE_NODE== eUnitSize)
    {
        g_struMemBlock[eUnitSize].nUnitSize = NODE_SIZE;
    }
    else if(UNIT_SIZE_FILTER== eUnitSize)
    {
        g_struMemBlock[eUnitSize].nUnitSize = FILTER_SIZE;
    }
    for(i = 1; i < nUnitNum; i++)
    {
        /* Don't flag for last unit,for last unit is ready for assigned, which is say no next unit can be assigned */
        *(mt_u16 *)pData = i;   
        
        pData += g_struMemBlock[eUnitSize].nUnitSize;

    }

    g_struMemBlock[eUnitSize].nFirst = 0;
    g_struMemBlock[eUnitSize].nFree = nUnitNum;
    g_struMemBlock[eUnitSize].nSize = nUnitNum * g_struMemBlock[eUnitSize].nUnitSize;
    g_struMemBlock[eUnitSize].pNext = NULL;
    g_struMemBlock[eUnitSize].pStartAddr = pAddr;

#if MT_TDE_MEMCOUNT_SUPPORT    
    g_struMemBlock[eUnitSize].nMaxNum = nUnitNum;
    g_struMemBlock[eUnitSize].nMaxUsed= 0;
#endif

    return MT_SUCCESS;
}

mt_void *mallocUnit(UNIT_SIZE_E eUnitSize)
{
#ifndef TDE_BOOT
    mt_size_t lockflags;
#endif
    MemoryBlock *pBlock=NULL;
    mt_u8* pFree = NULL;
    
    TDE_LOCK(&s_MemLock,lockflags);
    pBlock = &g_struMemBlock[eUnitSize];
   
    TDE_TRACE(TDE_KERN_DEBUG, "eUnitSize %d, free units:%d, first free unit:%d...\n", pBlock->nUnitSize, pBlock->nFree, pBlock->nFirst);
    if(!pBlock->nFree)
    {
        TDE_UNLOCK(&s_MemLock,lockflags);
        return NULL;
    }

    pFree = pBlock->pStartAddr + pBlock->nFirst * pBlock->nUnitSize;
    pBlock->nFirst = *(mt_u16 *)pFree;
    pBlock->nFree--;
#if MT_TDE_MEMCOUNT_SUPPORT    
    if((g_struMemBlock[eUnitSize].nMaxNum - pBlock->nFree) > g_struMemBlock[eUnitSize].nMaxUsed)
    {
        g_struMemBlock[eUnitSize].nMaxUsed = g_struMemBlock[eUnitSize].nMaxNum - pBlock->nFree;
    }
#endif
    TDE_UNLOCK(&s_MemLock,lockflags);
    memset(pFree, 0, pBlock->nUnitSize);
    return pFree;
}


mt_void *wmalloc(mt_size_t size)
{
    UNIT_SIZE_E i;
    mt_void *pMalloc;
    if((size > FILTER_SIZE) || (0 == size))
    {
        return NULL;
    }

    if(size <= CMD_SIZE)
    {
        for(i = UNIT_SIZE_CMD; i < UNIT_SIZE_BUTT; i++)
        {
            pMalloc = mallocUnit(i);
            if(NULL != pMalloc)
            {
                return pMalloc;
            }
        }
        return NULL;
    }
    else if (size <= JOB_SIZE)
    {
        for(i = UNIT_SIZE_JOB; i < UNIT_SIZE_BUTT; i++)
        {
            pMalloc = mallocUnit(i);
            if(NULL != pMalloc)
            {
                return pMalloc;
            }
        }
        return NULL;
    }
    else if(size <= NODE_SIZE)
    {
        for(i = UNIT_SIZE_NODE; i < UNIT_SIZE_BUTT; i++)
        {
            pMalloc = mallocUnit(i);
            if(NULL != pMalloc)
            {
                return pMalloc;
            }
        }
        return NULL;
    }
    else
    {
        return mallocUnit(UNIT_SIZE_FILTER);
    }
    
}

mt_s32 freeUnit(UNIT_SIZE_E eUnitSize, mt_void *ptr)
{
#ifndef TDE_BOOT
    mt_size_t lockflags;
#endif
    MemoryBlock *pBlock = NULL;
    TDE_LOCK(&s_MemLock,lockflags);
    pBlock = &g_struMemBlock[eUnitSize];

    if(((ulong)ptr < (ulong)pBlock->pStartAddr) 
        || ((ulong)ptr >= ((ulong)pBlock->pStartAddr + pBlock->nSize)))
    {
        TDE_UNLOCK(&s_MemLock,lockflags);
        return MT_FAILURE;
    }

    pBlock->nFree++;
    *(mt_u16*)ptr = pBlock->nFirst; /* point to next unit can be assigned */
    pBlock->nFirst = ((ulong)ptr - (ulong)pBlock->pStartAddr)/pBlock->nUnitSize;
    TDE_TRACE(TDE_KERN_DEBUG, "eUnitSize:%d,first free unit:%d, free units:%d\n", pBlock->nUnitSize, pBlock->nFirst, pBlock->nFree);
    TDE_UNLOCK(&s_MemLock,lockflags);
    return MT_SUCCESS;
}

mt_s32 wfree(mt_void *ptr)
{
    UNIT_SIZE_E i;
    for(i = UNIT_SIZE_CMD; i < UNIT_SIZE_BUTT; i++)
    {
       if(MT_SUCCESS == freeUnit(i, ptr)) 
       {
            return MT_SUCCESS;
       }
    }
    TDE_TRACE(TDE_KERN_INFO, "Free mem failed!vir:%p, phy:%x\n", ptr, wgetphy(ptr));
    return MT_FAILURE;
}

/************************all before is unrelated with tde, can be transplant to other modules ************************/

/************************ The follow is related with tde encapsulation******************************************/
#define TDE_MIN_BUFFER ((FILTER_SIZE)*2+((CMD_SIZE)+(NODE_SIZE)+(JOB_SIZE))*2)
#define TDE_MAX_BUFFER 1024*1024
STATIC phys_addr_t g_u32MemPoolPhyAddr;
STATIC ulong g_u32MemPoolVrtAddr;
STATIC mt_u32 g_u32TdeBuf;

#define MT_TDE_CMD_NUM  (((g_u32TdeBuf)-(FILTER_SIZE)*2)/((CMD_SIZE)+(NODE_SIZE)+(JOB_SIZE)))
#define MT_TDE_JOB_NUM  MT_TDE_CMD_NUM
#define MT_TDE_NODE_NUM   MT_TDE_CMD_NUM
#define MT_TDE_FILTER_NUM 2
#define TDE_CMD_OFFSET   0
#define TDE_JOB_OFFSET  ((MT_TDE_CMD_NUM) * CMD_SIZE)
#define TDE_NODE_OFFSET  (TDE_JOB_OFFSET + ((MT_TDE_JOB_NUM) * JOB_SIZE))
#define TDE_FILTER_OFFSET  (TDE_NODE_OFFSET + ((MT_TDE_NODE_NUM) * NODE_SIZE))
#define TDE_MEMPOOL_SIZE                ((MT_TDE_CMD_NUM) * CMD_SIZE + (MT_TDE_JOB_NUM) * JOB_SIZE + (MT_TDE_NODE_NUM) * NODE_SIZE +(MT_TDE_FILTER_NUM) * FILTER_SIZE)

mt_s32 wmeminit(void)
{
    if(MT_TDE_BUFFER>TDE_MAX_BUFFER)
    {
        g_u32TdeBuf = TDE_MAX_BUFFER;
    }  
    else if(MT_TDE_BUFFER<TDE_MIN_BUFFER)
    {
        g_u32TdeBuf = TDE_MIN_BUFFER;
    }
    else
    {
        g_u32TdeBuf = MT_TDE_BUFFER;
    }
    g_u32MemPoolPhyAddr = MT_GFX_AllocMem("TDE_MemPool",NULL,TDE_MEMPOOL_SIZE);
    if(0 == g_u32MemPoolPhyAddr)
    {
        TDE_TRACE(TDE_KERN_INFO, "malloc mempool buffer failed!\n");
        return MT_FAILURE;
    }
    #ifdef TDE_CACHE_STRATEGY
    g_u32MemPoolVrtAddr = (ulong )MT_GFX_MapCached(g_u32MemPoolPhyAddr);
    #else
    g_u32MemPoolVrtAddr = (ulong )MT_GFX_Map(g_u32MemPoolPhyAddr);
    #endif
    MemoryBlockInit(UNIT_SIZE_CMD, MT_TDE_CMD_NUM, (mt_void *)(g_u32MemPoolVrtAddr));
    MemoryBlockInit(UNIT_SIZE_JOB, MT_TDE_JOB_NUM, (mt_void *)(g_u32MemPoolVrtAddr + TDE_JOB_OFFSET));
    MemoryBlockInit(UNIT_SIZE_NODE, MT_TDE_NODE_NUM, (mt_void *)(g_u32MemPoolVrtAddr + TDE_NODE_OFFSET));
    MemoryBlockInit(UNIT_SIZE_FILTER, MT_TDE_FILTER_NUM, (mt_void *)(g_u32MemPoolVrtAddr + TDE_FILTER_OFFSET));
    #ifndef TDE_BOOT
    spin_lock_init(&s_MemLock);
    #endif
    PRINTMEMINFO();
    
    return MT_SUCCESS;
}

mt_void wmemterm(void)
{
    PRINTMEMINFO();
    MT_GFX_Unmap ((mt_void *)(g_u32MemPoolVrtAddr));
    MT_GFX_FreeMem( g_u32MemPoolPhyAddr);

    g_u32MemPoolPhyAddr = 0;
    g_u32MemPoolVrtAddr = 0;
}

ulong wgetphy(mt_void *ptr)
{
    ulong u32MemVrt = (ulong)ptr;  

    if((u32MemVrt < g_u32MemPoolVrtAddr)
        || (u32MemVrt >= (g_u32MemPoolVrtAddr + TDE_MEMPOOL_SIZE)))
    {
        return 0;
    }

    return (g_u32MemPoolPhyAddr + (u32MemVrt - g_u32MemPoolVrtAddr));
}

mt_void * wgetvrt(phys_addr_t phyaddr)
{
    if((phyaddr < g_u32MemPoolPhyAddr)
        || (phyaddr >= (g_u32MemPoolPhyAddr + TDE_MEMPOOL_SIZE)))
    {
        return NULL;
    }

    return (mt_void *)((ulong)(g_u32MemPoolVrtAddr + (phyaddr - g_u32MemPoolPhyAddr)));
}

mt_u32 wgetfreenum(mt_void)
{
    UNIT_SIZE_E eUnitSize = 0;
    mt_u32 u32FreeUnitNum = g_struMemBlock[eUnitSize].nFree;
    
    for(eUnitSize = UNIT_SIZE_CMD; eUnitSize < UNIT_SIZE_FILTER; eUnitSize++)
    {
        u32FreeUnitNum = (u32FreeUnitNum > g_struMemBlock[eUnitSize].nFree)?g_struMemBlock[eUnitSize].nFree:u32FreeUnitNum;
    }

    return u32FreeUnitNum;
}
#ifndef TDE_BOOT
#ifndef CONFIG_TDE_PROC_DISABLE
struct seq_file * wprintinfo(struct seq_file *page)
{
    #if MT_TDE_MEMCOUNT_SUPPORT
    mt_u32 u32MaxUsedCmd    = g_struMemBlock[UNIT_SIZE_CMD].nMaxUsed;
    mt_u32 u32MaxUsedJob   = g_struMemBlock[UNIT_SIZE_JOB].nMaxUsed;
    mt_u32 u32MaxUsedNode  = g_struMemBlock[UNIT_SIZE_NODE].nMaxUsed;
    mt_u32 u32MaxUsedFilter =g_struMemBlock[UNIT_SIZE_FILTER].nMaxUsed;
    #else
    mt_u32 u32FreeCmd    = g_struMemBlock[UNIT_SIZE_CMD].nFree;
    mt_u32 u32FreeJob   = g_struMemBlock[UNIT_SIZE_JOB].nFree;
    mt_u32 u32FreeNode  = g_struMemBlock[UNIT_SIZE_NODE].nFree;
    mt_u32 u32FreeFilter  = g_struMemBlock[UNIT_SIZE_FILTER].nFree;
    #endif

    #ifndef CONFIG_TDE_STR_DISABLE
    PROC_PRINT(page, "--------- Montage TDE Memory Pool Info ---------\n");
    #if MT_TDE_MEMCOUNT_SUPPORT
    PROC_PRINT(page, "     Type         Total       MaxUsed\n");
    PROC_PRINT(page, "[Unit %d ]   %8u  %8u\n",CMD_SIZE,MT_TDE_CMD_NUM, u32MaxUsedCmd);
    PROC_PRINT(page, "[Unit %d ]   %8u  %8u\n",JOB_SIZE,MT_TDE_JOB_NUM, u32MaxUsedJob);
    PROC_PRINT(page, "[Unit %d]   %8u  %8u\n",NODE_SIZE,MT_TDE_NODE_NUM, u32MaxUsedJob);
    PROC_PRINT(page, "[Unit%d]   %8u  %8u\n",FILTER_SIZE,MT_TDE_FILTER_NUM, u32MaxUsedFilter);
    PROC_PRINT(page, "[Total   ]   %8uK %8uK\n", TDE_MEMPOOL_SIZE/1024, (CMD_SIZE* u32MaxUsedCmd + JOB_SIZE* u32MaxUsedJob + NODE_SIZE* u32MaxUsedNode +FILTER_SIZE* u32MaxUsedFilter)/1024);
    #else
    PROC_PRINT(page, "     Type         Total       Used\n");
    PROC_PRINT(page, "[Unit %d ]   %8u  %8u\n",CMD_SIZE, MT_TDE_CMD_NUM, MT_TDE_CMD_NUM - u32FreeCmd);
    PROC_PRINT(page, "[Unit %d ]   %8u  %8u\n", JOB_SIZE,MT_TDE_JOB_NUM, MT_TDE_JOB_NUM - u32FreeJob);
    PROC_PRINT(page, "[Unit %d]   %8u  %8u\n", NODE_SIZE,MT_TDE_NODE_NUM, MT_TDE_NODE_NUM - u32FreeNode);
    PROC_PRINT(page, "[Unit%d]   %8u  %8u\n",FILTER_SIZE, MT_TDE_FILTER_NUM, MT_TDE_FILTER_NUM - u32FreeFilter);
    PROC_PRINT(page, "[Total   ]   %8uK %8uK\n", TDE_MEMPOOL_SIZE/1024, (TDE_MEMPOOL_SIZE - (CMD_SIZE* u32FreeCmd + JOB_SIZE* u32FreeJob+NODE_SIZE* u32FreeNode+ FILTER_SIZE * u32FreeFilter))/1024);
    #endif
    #endif
    return page;
}
#endif
#endif
