/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "mtgo_common.h"
#include "mtgo_memory.h"
#include "mt_common.h"
#include "string.h"
#include "mpi_mmz.h"


#ifdef __cplusplus
extern "C" {
#endif

#define DEC_BUF_LEN            0x1000000

//#define MMZ_ZONE_MTGO   MMZ_ZONE_AV
#define MMZ_ZONE_MTGO   NULL


#ifndef MTGO_CODE_CUT
typedef struct _BT_USED_S
{
    mt_u32 u32Size;
    mt_u8  pData[1];
}BT_USED_S;

#if 0
static mt_u32  s_bt_BufferSize = 0;
static mt_void *s_bt_BufferHead = NULL;
static mt_u32  s_bt_bInitalize = 0;
static mt_u32  s_bt_UsedSize = 0;
static mt_u32  s_bt_BufferHeadPhyAddr = 0;
#endif

#define MEM_ALIGN_SIZE 16
#endif

static BT_FREE_S *s_bt_pFreeBlock = NULL;
static mt_s32 MTGO_InitMemoryBlock(mt_void)
{
//#ifndef MTGO_CODE_CUT
#if 0
    s_bt_pFreeBlock = (BT_FREE_S*)s_bt_BufferHead;
    if(NULL == s_bt_pFreeBlock)
    {
        return MT_FAILURE;
    }
    s_bt_pFreeBlock->u32Size = s_bt_BufferSize;
    s_bt_pFreeBlock->pNext = NULL;
    s_bt_bInitalize = 1;
#endif    
    return MT_SUCCESS;
}

mt_s32 MTGO_InitMemory(mt_void)
{
   mt_s32 ret = MT_SUCCESS;
   
//#ifndef MTGO_CODE_CUT
#if 0
    
    mt_mmz_buf_s  psMBuf;   /// TODO:: 
    mt_char BufName[16] = "osd";
    mt_u32  phyaddr = 0;
    
    psMBuf.bufsize = DEC_BUF_LEN;
    //psMBuf.bufsize = 0x1 * 1024 * 1024; // 1M
    strncpy(psMBuf.bufname, BufName, sizeof(mt_char)*16);
    ret = mt_mpi_mmz_malloc(&psMBuf);
    if(ret != MT_SUCCESS)
    {
        MT_ASSERT(0);
        MTGO_ERROR(ret);
        return MT_FAILURE;
    }        

    if(s_bt_bInitalize) 
    {
        return MT_FAILURE;
    }

    if((0 == psMBuf.user_viraddr) || (DEC_BUF_LEN < sizeof(BT_FREE_S)))
    {
        MT_ASSERT(0);
        return MT_FAILURE;
    }
    
    s_bt_BufferHead = (mt_void*)psMBuf.user_viraddr;
    s_bt_BufferSize = DEC_BUF_LEN;
    s_bt_BufferHeadPhyAddr = phyaddr = psMBuf.phyaddr;
    
//ERROR_1:
    //mt_mem_free(&psMBuf);
    MT_PRINT("%s: %d  buf[%x] size sdadsf[%x]\n", __FUNCTION__, __LINE__, s_bt_BufferHead, s_bt_BufferSize, phyaddr);

#endif   

    ret = MTGO_InitMemoryBlock();

    MT_PRINT("MTGO_InitMemory end [%x]\n", ret);

    return ret;

}

mt_void* MTGO_Malloc2(mt_u32 u32Size, phys_addr_t *pPhyAddr)
{
//#ifndef MTGO_CODE_CUT
#if 0
    BT_FREE_S *pCur = NULL,*pPre = NULL;
    mt_u32 umask = MEM_ALIGN_SIZE -1;
    BT_USED_S *pUsed = NULL;
    
    //u32Size = ((u32Size + umask) & ( ~umask )) + sizeof(mt_u32);
    u32Size = ((u32Size + umask) & ( ~umask )) + MEM_ALIGN_SIZE;  //for memory align
    if(s_bt_bInitalize)
    {
    
        if(NULL == s_bt_pFreeBlock)
        {
            return NULL;
        }
        //not enough memory
        if((s_bt_BufferSize - s_bt_UsedSize) < u32Size)
        {
            return NULL;
        }
            
        pCur = s_bt_pFreeBlock;
        while(NULL != pCur)
        {
            if(pCur->u32Size >= u32Size)
            {
                break;
            }
            pPre = pCur;
            pCur = pCur->pNext;
        }
        //couldn't find a block that is larger than the acquiredsize 
        if(NULL == pCur)
        {
            return NULL;
        }
        //find a block 
        if(pCur->u32Size == u32Size)
        {
            // the first block is ok
            if(NULL == pPre)
            {
                s_bt_pFreeBlock = s_bt_pFreeBlock->pNext;
            }
            else
            {
                pPre->pNext = pCur->pNext;
            }
        }
        else
        {
            pCur->u32Size -= u32Size;
            pCur = (BT_FREE_S*)((mt_u8*)pCur + pCur->u32Size);
        }
        //pCur is the block that will be use
        pUsed = (BT_USED_S*)(mt_void*)pCur;
        pUsed->u32Size = u32Size;
        s_bt_UsedSize += u32Size;
          {
              mt_u32  addr = 0;
              addr = pUsed->pData +(MEM_ALIGN_SIZE -4);
              printf("MTGO_Malloc2: [%x]\n", addr, s_bt_BufferHeadPhyAddr);
          }
        return pUsed->pData +(MEM_ALIGN_SIZE -4);
    }
	return NULL;
#else
    mt_s32 ret;
    mt_mmz_buf_s  psMBuf;
    memset(&psMBuf, 0x00, sizeof(mt_mmz_buf_s));
    strncpy(psMBuf.bufname, "mtgo",(MAX_BUFFER_NAME_SIZE-1));	
    psMBuf.bufsize = u32Size;
    ret = mt_mmz_malloc(&psMBuf);
    if(ret != MT_SUCCESS)
    {
        MTGO_ERROR(ret);
        return NULL;
    }  

//    printf("MTGO_Malloc2: [%x][%x][%x] [%d]\n", (mt_u32)psMBuf.user_viraddr, (mt_u32)psMBuf.kernel_viraddr, psMBuf.phyaddr, psMBuf.bufsize);
      
    //MT_ASSERT(0);
    //return (mt_void*)psMBuf.phyaddr;
    *pPhyAddr = psMBuf.phyaddr;
    return (mt_void*)psMBuf.user_viraddr;
#endif
    return NULL;
}

mt_void MTGO_Free2(mt_void* pAddr)
{
//#ifndef MTGO_CODE_CUT
#if 0
    BT_FREE_S *pFree = NULL,*pPre = NULL,*pCur = NULL;
    BT_USED_S *pUsed = NULL;
    if(s_bt_bInitalize)
    {
        if(NULL == pAddr)
        {
            return ;
        }
        pUsed = (BT_USED_S*)((mt_u32)pAddr - MEM_ALIGN_SIZE);
        pFree = (BT_FREE_S*)(mt_void*)pUsed;
        if(((mt_u8*)pFree < (mt_u8*)s_bt_BufferHead)
            || ((mt_u8*)pFree >= (mt_u8*)s_bt_BufferHead + s_bt_BufferSize))
        {
            return ;
        }
        if(NULL == s_bt_pFreeBlock)
        {
            s_bt_pFreeBlock = pFree;
            s_bt_pFreeBlock->pNext = NULL;
            s_bt_UsedSize -= pFree->u32Size;
            return ;
        }
        pCur = s_bt_pFreeBlock;
        while(NULL !=pCur && pCur < pFree)
        {
            pPre = pCur;
            pCur = pCur->pNext;
        }
        //in front of the list
        if(NULL == pPre)
        {
            pFree->pNext = s_bt_pFreeBlock;
            if((mt_u8*)pFree + pFree->u32Size == (mt_u8*)s_bt_pFreeBlock)
            {
                pFree->pNext = s_bt_pFreeBlock->pNext;
                pFree->u32Size += s_bt_pFreeBlock->u32Size;
            }
            s_bt_pFreeBlock = pFree;
            s_bt_UsedSize -= pFree->u32Size;
            return ;
        }
        //behide the list
        if(NULL == pCur)
        {
            pPre->pNext = pFree;
            pFree->pNext = NULL;
            if((mt_u8*)pPre + pPre->u32Size == (mt_u8*)pFree)
            {
                pPre->pNext = NULL;
                pPre->u32Size += pFree->u32Size;
            }
            s_bt_UsedSize -= pFree->u32Size;
            return ;
        }
        // pPre ,pCur couldn't be NULL 
        pPre->pNext = pFree;
        pFree->pNext = pCur;
        s_bt_UsedSize -= pFree->u32Size;
        //merge with the pre node
        if((mt_u8*)pPre + pPre->u32Size == (mt_u8*)pFree)
        {
            pPre->pNext = pCur;
            pPre->u32Size += pFree->u32Size;
            pFree = pPre;
        }
        //merge with the post node
        if((mt_u8*)pFree + pFree->u32Size == (mt_u8*)pCur)
        {
            pFree->pNext = pCur->pNext;
            pFree->u32Size += pCur->u32Size;
        }
    }
#else

        mt_mmz_buf_s  psMBuf;
        phys_addr_t  phyaddr = 0;
        ulong size = 0;
        memset(&psMBuf, 0x00, sizeof(mt_mmz_buf_s));
        
        mt_mpi_mmz_getphyaddr(pAddr, &phyaddr, &size);

        psMBuf.user_viraddr = pAddr;
        psMBuf.phyaddr = phyaddr;
        mt_mpi_mmz_free(&psMBuf);
//        printf("MTGO_Free2: [%x][%x] [%d]\n", pAddr, phyaddr, size );

#endif    
}

mt_void* MTGO_Malloc_New(mt_u32 u32Size, phys_addr_t *pPhyAddr)
{
    mt_mmz_buf_s  psMBuf;
    memset(&psMBuf, 0x00, sizeof(mt_mmz_buf_s));
    strncpy(psMBuf.bufname, "mtgo",(MAX_BUFFER_NAME_SIZE-1));	
    psMBuf.bufsize = u32Size;

    psMBuf.phyaddr = (phys_addr_t)mt_mmz_new(psMBuf.bufsize, 64, MMZ_ZONE_MTGO, psMBuf.bufname);
    if(psMBuf.phyaddr == 0)
    {
        MTGO_ERROR(0);
        return NULL;
    } 
    *pPhyAddr = psMBuf.phyaddr;
    psMBuf.user_viraddr = (void*)mt_mmz_map(psMBuf.phyaddr, 0);

    return (mt_void*)psMBuf.user_viraddr;
    
}

mt_void MTGO_Free_New(mt_void* pAddr)
{

        mt_mmz_buf_s  psMBuf;
        phys_addr_t  phyaddr = 0;
        ulong size = 0;
        memset(&psMBuf, 0x00, sizeof(mt_mmz_buf_s));

        mt_mpi_mmz_getphyaddr(pAddr, &phyaddr, &size);
        
        psMBuf.user_viraddr = pAddr;
        psMBuf.phyaddr = phyaddr;
        mt_mpi_mmz_free(&psMBuf);
//        printf("MTGO_Free2: [%x][%x] [%d]\n", pAddr, phyaddr, size );
   
}

mt_void MTGO_DeInitMemory(mt_void)
{
// #ifndef MTGO_CODE_CUT
#if 0
    if(s_bt_bInitalize)
    {
       {
            mt_mmz_buf_s  psMBuf;  
            psMBuf.user_viraddr = s_bt_BufferHead;
            psMBuf.bufsize = s_bt_BufferSize;
            mt_mpi_mmz_free(&psMBuf);
         }
    
        s_bt_BufferHead = NULL;
        s_bt_pFreeBlock = NULL;
        s_bt_bInitalize = 0;
        s_bt_BufferSize = 0;
        s_bt_UsedSize =0;

         
    }
#endif    
    return;
}

BT_FREE_S* MTGO_GetFreeList(mt_void)
{
    return s_bt_pFreeBlock;
}

#ifdef __cplusplus
}
#endif

