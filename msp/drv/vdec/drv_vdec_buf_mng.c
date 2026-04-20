/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
  File Name     : drv_vdec_buf_mng.c
  Version       : Initial Draft
  Author        : Montage MA-SW
  Created       : 2016/01/06
  Description   : Definitions of buffer manager.
  History       :
  1.Date        : 2016/01/06
    Author      :
    Modification: Created file

 *******************************************************************************/

/******************************* Include Files *******************************/

/* Sys headers */
#include <linux/list.h>
#include <linux/sizes.h>	/*SZ_1K*/

/* Common headers */
#include "mt_kernel_adapt.h"
#include "mt_common.h"
#include "mt_module.h"
#include "mt_module_debug.h"
#include "mt_drv_mmz.h"
#include "mt_drv_mem.h"
#include "vfmw.h"
#include "drv_vdec_ext.h"
#include "mt_drv_sys.h"
/* Local headers */
#include "drv_vdec_buf_mng.h"
#include "vconfig.h"
#include "drv_vdec_debug.h"
#include "mt_module_debug.h"

#undef LOG_TAG
#define LOG_TAG				"VDEC_BUFMNG"
#include "Log.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

extern MT_BOOL bSaveoneyuv;
extern mt_u32  u32SaveCnt;

/****************************** Macro Definition *****************************/

#define BUFMNG_INVALID_64BITS_PTS   ((mt_u64)(-1))
#define BUFMNG_INVALID_PTS          ((mt_u32)(-1))

#define MT_VMALLOC_BUFMNG(size)     MT_VMALLOC(MT_ID_VDEC, size)
#define MT_VFREE_BUFMNG(addr)       MT_VFREE(MT_ID_VDEC, addr)
#define MT_KMALLOC_ATOMIC_BUFMNG(size)     MT_KMALLOC(MT_ID_VDEC, size, GFP_ATOMIC)
#define MT_KFREE_BUFMNG(addr)       MT_KFREE(MT_ID_VDEC, addr)

#define MT_FATAL_BUFMNG(fmt...)     MT_FATAL_PRINT(MT_ID_VDEC, fmt)
#define MT_ERR_BUFMNG(fmt...)       MT_ERR_PRINT(MT_ID_VDEC, fmt)
#define MT_WARN_BUFMNG(fmt...)      MT_WARN_PRINT(MT_ID_VDEC, fmt)
#define MT_INFO_BUFMNG(fmt...)      MT_INFO_PRINT(MT_ID_VDEC, fmt)

/* dump stat interval */
#define DEBUG_DUMP_INTERVAL			200

#define BUFMNG_LOCK(sema) \
    do \
    { \
        while (down_killable(&sema)) \
        { \
            MT_ERR_BUFMNG("Lock fail"); \
        } \
    } while (0)
#define BUFMNG_UNLOCK(sema) do {up(&sema);} while (0)

#define BUFMNG_SPIN_LOCK(spinlock) \
    do \
    { \
        spin_lock_irqsave(&spinlock, ulFlags);\
    } while (0)
#define BUFMNG_SPIN_UNLOCK(spinlock) \
    do \
    { \
        spin_unlock_irqrestore(&spinlock, ulFlags);\
    } while (0)

#define IS_HEAD_OF_LIST(pBlock, pInst) (&pBlock->stBlockNode == pInst->stBlockHead.next)
#define IS_READING(pBlock) (BUFMNG_BLOCK_READING == pBlock->enstatus)
#define IS_WRITING(pBlock) (BUFMNG_BLOCK_WRITING == pBlock->enstatus)
#define IS_FREE(pBlock) (BUFMNG_BLOCK_FREE == pBlock->enstatus)

#define IS_FULL(pInst, size) (pInst->u32Free < size)

#define BUFMNG_FIND_INST(hInst, pInst) \
    { \
        struct list_head* pos; \
        struct list_head* n; \
        BUFMNG_INST_S* pstTmp; \
        pInst = MT_NULL; \
        list_for_each_safe(pos, n, &s_stBMParam.stInstHead) \
        { \
            pstTmp = list_entry(pos, BUFMNG_INST_S, stInstNode); \
            if (hInst == pstTmp->hBuf) \
            { \
                pInst = pstTmp; \
                break; \
            } \
        } \
    }

#define BUFMNG_FIND_BLOCK_BY_ADDR(addr, pInst, pBlock) \
    { \
        struct list_head* pos; \
        struct list_head* n; \
        BUFMNG_BLOCK_S* pstTmp; \
        list_for_each_safe(pos, n, &pInst->stBlockHead) \
        { \
            pstTmp = list_entry(pos, BUFMNG_BLOCK_S, stBlockNode); \
            if (addr == pstTmp->u32Addr) \
            { \
                pBlock = pstTmp; \
                break; \
            } \
        } \
    }

#define BUFMNG_FREE_BLOCK_LIST(pInst) \
    { \
        struct list_head* pos; \
        struct list_head* n; \
        BUFMNG_BLOCK_S* pstBlock; \
        list_for_each_safe(pos, n, &pInst->stBlockHead) \
        { \
            pstBlock = list_entry(pos, BUFMNG_BLOCK_S, stBlockNode); \
            list_del(pos); \
            MT_KFREE_BUFMNG(pstBlock); \
        } \
    }

#define BUFMNG_FIND_TAIL_BLOCK(pInst, pBlock) \
    { \
        pBlock = list_entry(pInst->stBlockHead.prev, BUFMNG_BLOCK_S, stBlockNode); \
    }

#define BUFMNG_FIND_HEAD_BLOCK(pInst, pBlock) \
    { \
        pBlock = list_entry(pInst->stBlockHead.next, BUFMNG_BLOCK_S, stBlockNode); \
    }

#define BUFMNG_FIND_NEXT_BLOCK(pBlock, pNextBlock) \
    { \
        pNextBlock = list_entry(pBlock->stBlockNode.next, BUFMNG_BLOCK_S, stBlockNode); \
    }

#define BUFMNG_FIND_FIRST_FREE_BLOCK(pInst, pBlock) \
    { \
        BUFMNG_BLOCK_S* pstTmp = MT_NULL; \
        struct list_head* pstList = pInst->stBlockHead.next; \
        while (pstList != &(pInst->stBlockHead)) \
        { \
            pstTmp = list_entry(pstList, BUFMNG_BLOCK_S, stBlockNode); \
            if (IS_FREE(pstTmp)) \
            { \
                pBlock = pstTmp; \
                break; \
            } \
            pstList = pstList->next; \
        } \
    }

/************************ Static Structure Definition ************************/

typedef enum tagBUFMNG_BLOCK_STATUS_E
{
    BUFMNG_BLOCK_FREE = 0,
    BUFMNG_BLOCK_READING,
    BUFMNG_BLOCK_WRITING,
    BUFMNG_BLOCK_BUTT
} BUFMNG_BLOCK_STATUS_E;

/* Describe a buffer block */
typedef struct tagBUFMNG_BLOCK_S
{
    phys_addr_t u32Addr;                 /* Only save one address in block, use physical address */
    mt_u32 u32Size;                 /* Size of block */
#if (BUFMNG_64BITS_PTS_SUPPORT == 1)
    mt_u64 u64Pts;                  /* PTS of the data filled in a buffer.*/
#else
    mt_u32 u32Pts;                  /* PTS of the data filled in a buffer.*/
#endif
#if (BUFMNG_INDEX_SUPPORT == 1)
    mt_u32 u32Index;                /* Index */
#endif
#if (BUFMNG_MARKER_SUPPORT == 1)
    mt_u32 u32Marker;               /* bit0: 0:End of Frame/1:Half frame
                                     * bit1: 0:Normal/1:End of stream
                                     * bit2: 0:Continuous stream/1:Discontinuous
                                     */
#endif
    BUFMNG_BLOCK_STATUS_E enstatus; /* Status of block */
    struct list_head stBlockNode;   /* Block list node */
} BUFMNG_BLOCK_S;

/* Describe a buffer instance */
typedef struct tagBUFMNG_INST_S
{
    mt_handle   hBuf;               /* Handle of this buffer instance */
    phys_addr_t      u32PhyAddr;         /* Start physical address of the buffer instance. */
    mt_u8*      pu8UsrVirAddr;      /* Start user virtual address of the buffer instance. */
    mt_u8*      pu8KnlVirAddr;      /* Start kerenl virtual address of the buffer instance. */
    ulong      u32Size;            /* Size of the buffer instance */
    mt_u32      u32Used;            /* Used size */
    mt_u32      u32Free;            /* Free size */
    mt_u32      u32Freeze;          /* Freeze size */
    mt_u32      u32DataNum;         /* For stream mode, it is undecoded packet number.
                                       For frame mode, it is undecoded frame number, support BUFMNG_NOT_END_FRAME_BIT.*/
    mt_u32      u32GetTry;          /* GetWriteBuf try times */
    mt_u32      u32GetOK;           /* GetWriteBuf ok times */
    mt_u32      u32PutTry;          /* PutWriteBuf try times */
    mt_u32      u32PutOK;           /* PutWriteBuf ok times */
    mt_u32      u32RecvTry;         /* GetReadBuf try times */
    mt_u32      u32RecvOK;          /* GetReadBuf ok times */
    mt_u32      u32RlsTry;          /* PutReadBuf try times */
    mt_u32      u32RlsOK;           /* PutReadBuf ok times */
    phys_addr_t      u32DescPhyAddr;
    mt_u8*      pu8KnlVirDescAddr;
    mt_u32      u32KnlVirDescBufSize;
#if (BUFMNG_INDEX_SUPPORT == 1)
    mt_u32      u32Index;           /* Index */
#endif
    BUFMNG_ALLOC_TYPE_E enAllocType;/* MMZ alloc type */
    MT_BOOL bMMZMap;                /* Need unmap when destroy */
    spinlock_t stSpinLock;          /* Spin lock */
    struct list_head stBlockHead;   /* Buffer manager block list head */
    struct list_head stInstNode;    /* Instance node */
	//Debug
    mt_u32      u32KnlWriterOff;	/* ES Buffer write pointer */
    mt_u32      u32GetFail;         /* GetWriteBuf fail times */
    mt_u32      u32Offset;			/* ES Data total offset */
} BUFMNG_INST_S;

/* Global parameter of this module */
typedef struct tagBUFMNG_GLOBAL_S
{
    mt_u16 u16InstNum;              /* Instance count */
    mt_u16 u16InstHandle;           /* Allocate handle according to this number */
    struct semaphore stSem;         /* Semaphore */
    struct list_head stInstHead;    /* Instance list head */
} BUFMNG_GLOBAL_S;

/***************************** Global Definition *****************************/

struct file *VdecSaveRawFile = MT_NULL;
mt_s32 VdecRawChanNum = -1;
struct semaphore stRawSem;

struct file *VdecSaveYuvFile = MT_NULL;
mt_s32 VdecYuvChanNum = -1;
mt_u8 *U_Array = NULL;
mt_u8 *V_Array = NULL;
mt_u8 *YUV_Array = NULL;
struct semaphore stYuvSem;

/***************************** Static Definition *****************************/

static BUFMNG_GLOBAL_S s_stBMParam =
{
    .u16InstNum = 0,
    .u16InstHandle = 0,
    .stInstHead = LIST_HEAD_INIT(s_stBMParam.stInstHead)
};

extern mt_s32 VDEC_FindChanIDByESBufferHandle(mt_handle hSteeamBufferHandle, mt_s32 *pChanID);

void vdec_descriptor_init(mt_u32 ch,
							phys_addr_t u32PhyAddr, unsigned char *pu8KnlVirAddr,
							mt_u32 u32Size,
							phys_addr_t u32DescPhyAddr, unsigned char *pu8KnlVirDescAddr,
							mt_u32 u32KnlVirDescBufSize);
mt_u32 vdec_descriptor_get_write_ptr(mt_u32 ch);
void vdec_descriptor_set_write_ptr(mt_u32 ch, mt_u32 offset);
mt_u32 vdec_descriptor_get_read_ptr(mt_u32 ch);

static mt_s32 _BUFMNG_PutDescriptor(BUFMNG_INST_S* pstInst, BUFMNG_BUF_S *pstBuf,
									mt_u32 offset_begin, mt_u32 offset_end);
SINT32 KERN_VDEC_Control(SINT32 ChanID, VDEC_CID_E eCmdID, VOID *pArgs);

/*********************************** Code ************************************/

mt_s32 BUFMNG_Init(mt_void)
{
	ENTER_FUNCTION;

    /* Init global parameter */
    s_stBMParam.u16InstNum = 0;
    s_stBMParam.u16InstHandle = 0;

    /* Init global mutex */
    MT_INIT_MUTEX(&s_stBMParam.stSem);

    INIT_LIST_HEAD(&s_stBMParam.stInstHead);

	LEAVE_FUNCTION;
    return MT_SUCCESS;
}

mt_s32 BUFMNG_DeInit(mt_void)
{
    struct list_head* pos;
    struct list_head* n;
    struct list_head* head;
    BUFMNG_INST_S* pstInst;

	ENTER_FUNCTION;

    BUFMNG_LOCK(s_stBMParam.stSem);
    head = &s_stBMParam.stInstHead;
    if (!list_empty(head))
    {
        list_for_each_safe(pos, n, head)
        {
            pstInst = list_entry(pos, BUFMNG_INST_S, stInstNode);
            BUFMNG_UNLOCK(s_stBMParam.stSem);
            BUFMNG_Destroy(pstInst->hBuf);
            BUFMNG_LOCK(s_stBMParam.stSem);
        }
    }

    s_stBMParam.u16InstNum = 0;
    s_stBMParam.u16InstHandle = 0;
    BUFMNG_UNLOCK(s_stBMParam.stSem);

	//BUFMNG_CloseFile(-1, 2);

	LEAVE_FUNCTION;
    return MT_SUCCESS;
}


mt_s32 BUFMNG_Create_forUsrData(mt_handle *phBuf, BUFMNG_INST_CONFIG_S* pstConfig)
{
    mt_s32 s32Ret;
    BUFMNG_INST_S* pstInst;
    mmz_buffer_s stMMZAllocBuf;
    mmz_buffer_s stMMZBuf;
	int i;

    if ((MT_NULL == phBuf) || (MT_NULL == pstConfig))
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    if ((BUFMNG_ALLOC_BUTT <= pstConfig->enAllocType) || (0 == pstConfig->u32Size))
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    if ((BUFMNG_ALLOC_OUTER == pstConfig->enAllocType) && (0 == pstConfig->u32PhyAddr))
    {
        return MT_ERR_BM_INVALID_PARA;
    }

	//BUG: VES buffer instances count overflow!
	if (s_stBMParam.u16InstNum >= CFG_VDEC_VES_INS_COUNT)
	{
		MLOGE("%s: VES Instance count(%u) full!\n",__FUNCTION__,s_stBMParam.u16InstNum);
		WARN(1, "BUFMNG_Create: VES Instance count full!\n");
        return MT_ERR_BM_NO_MEMORY;
	}

    /* If BUFMNG_ALLOC_INNER, alloc MMZ here */
    if (BUFMNG_ALLOC_INNER == pstConfig->enAllocType)
    {
        {
            s32Ret = mt_drv_mmz_alloc_and_map(pstConfig->aszName, MMZ_OTHERS, pstConfig->u32Size, 0, &stMMZAllocBuf);
            if (MT_SUCCESS != s32Ret)
            {
                MT_FATAL_BUFMNG("Alloc MMZ fail:0x%x.\n", s32Ret);
                return MT_ERR_BM_NO_MEMORY;
            }
			else
			{ 
				memset((void*)stMMZAllocBuf.startVirAddr, 0, stMMZAllocBuf.size);
			}

            pstConfig->u32PhyAddr = stMMZAllocBuf.startPhyAddr;
            pstConfig->pu8KnlVirAddr = (mt_u8*)stMMZAllocBuf.startVirAddr;
        }
    }

    /* Allocate an instance */
//    pstInst = MT_VMALLOC_BUFMNG(sizeof(BUFMNG_INST_S));
    pstInst = MT_KMALLOC_ATOMIC_BUFMNG(sizeof(BUFMNG_INST_S));

    if (MT_NULL == pstInst)
    {
        MT_FATAL_BUFMNG("No memory.\n");
        if (BUFMNG_ALLOC_INNER == pstConfig->enAllocType)
        {
            mt_drv_mmz_unmap_and_release(&stMMZAllocBuf);
        }
        return MT_ERR_BM_NO_MEMORY;
    }

    BUFMNG_LOCK(s_stBMParam.stSem);

    /* Allocate handle */
	//BUG: VES buffer instance handle(hBuf) overflow!
	//NOTE: VES buffer MUST destroy sequentially!!!
	//       Create: 1 2 3 4 5 ...
	//      Destroy:  1 2 3 4 5 ...
    //*phBuf = s_stBMParam.u16InstHandle;
    //*phBuf = s_stBMParam.u16InstHandle % CFG_VDEC_VES_INS_COUNT;

	//Find a free u16InstHandle, then VES buffer NO NEED destroy sequentially!!!
	for (i=0; i<2*CFG_VDEC_VES_INS_COUNT; i++)	/* 2*CFG_VDEC_VES_INS_COUNT: avoid u16InstHandle flow! */
	{
		mt_handle hBufTmp;
		BUFMNG_INST_S* pstInstTmp = MT_NULL;

		hBufTmp = s_stBMParam.u16InstHandle % CFG_VDEC_VES_INS_COUNT;
		BUFMNG_FIND_INST(hBufTmp, pstInstTmp);

		//Found a free u16InstHandle
		if (pstInstTmp == MT_NULL)
			break;

		//Continue next one
		s_stBMParam.u16InstHandle ++;

	}
	if (i >= 2*CFG_VDEC_VES_INS_COUNT)
	{
		//shall not happen!
        MT_FATAL_BUFMNG("No Inst Handle.\n");
		BUG();
	}

    *phBuf = s_stBMParam.u16InstHandle % CFG_VDEC_VES_INS_COUNT;

    /* Init instance parameter */
    pstInst->hBuf = *phBuf;
    pstInst->u32PhyAddr = pstConfig->u32PhyAddr;
    pstInst->pu8UsrVirAddr = pstConfig->pu8UsrVirAddr;
    if (MT_NULL == pstConfig->pu8KnlVirAddr)
    {
        /* Map to kernel virtual address */
        stMMZBuf.size = pstConfig->u32Size;
        stMMZBuf.startPhyAddr = pstConfig->u32PhyAddr;
        if (MT_SUCCESS != mt_drv_mmz_map(&stMMZBuf))
        {
            BUFMNG_UNLOCK(s_stBMParam.stSem);
            MT_ERR_BUFMNG("mt_drv_mmz_map fail!\n");
            if (BUFMNG_ALLOC_INNER == pstConfig->enAllocType)
            {
                {
                    mt_drv_mmz_unmap_and_release(&stMMZAllocBuf);
                }
            }
            MT_KFREE_BUFMNG(pstInst);
            return MT_FAILURE;
        }

        pstInst->pu8KnlVirAddr = pstConfig->pu8KnlVirAddr = (mt_u8*)stMMZBuf.startVirAddr;
        MT_INFO_BUFMNG("stMMZBuf.u32StartVirAddr:%p\n", stMMZBuf.startVirAddr);
        pstInst->bMMZMap = MT_TRUE;
    }
    else
    {
        pstInst->pu8KnlVirAddr = pstConfig->pu8KnlVirAddr;
        pstInst->bMMZMap = MT_FALSE;
    }

    pstInst->u32Size = pstConfig->u32Size;
    pstInst->enAllocType = pstConfig->enAllocType;
#if (BUFMNG_INDEX_SUPPORT == 1)
    pstInst->u32Index = 0;
#endif
    //MT_INFO_BUFMNG("[BUFMNG_Create] PHY:%p, U-VIR:%p, K-VIR:%p, SIZE:%d\n",
    //           (mt_void*)pstInst->u32PhyAddr, pstInst->pu8UsrVirAddr, pstInst->pu8KnlVirAddr, pstInst->u32Size);

    //MLOGI("%s: PHY:%p, U-VIR:%p, K-VIR:%p, SIZE:%d\n",__FUNCTION__,
    //           (mt_void*)pstInst->u32PhyAddr, pstInst->pu8UsrVirAddr, pstInst->pu8KnlVirAddr, pstInst->u32Size);
 //   MLOGI("%s: PHY:%p, K-VIR:%p, SIZE:%d\n",__FUNCTION__,
 //              (mt_void*)pstInst->u32PhyAddr, pstInst->pu8KnlVirAddr, pstInst->u32Size);

    spin_lock_init(&pstInst->stSpinLock);

    /* Init block list parameter */
    pstInst->u32Free   = pstInst->u32Size;
    pstInst->u32Freeze = 0;
    pstInst->u32Used    = 0;
    pstInst->u32DataNum = 0;
    pstInst->u32GetTry  = 0;
    pstInst->u32GetOK   = 0;
    pstInst->u32PutTry  = 0;
    pstInst->u32PutOK   = 0;
    pstInst->u32RecvTry = 0;
    pstInst->u32RecvOK  = 0;
    pstInst->u32RlsTry  = 0;
    pstInst->u32RlsOK   = 0;
    INIT_LIST_HEAD(&pstInst->stBlockHead);

    /* Add this instance to instance list */
    list_add_tail(&pstInst->stInstNode, &s_stBMParam.stInstHead);
    s_stBMParam.u16InstHandle++;
    s_stBMParam.u16InstNum++;
    BUFMNG_UNLOCK(s_stBMParam.stSem);
    return MT_SUCCESS;
}

mt_s32 BUFMNG_Create(mt_handle hVdec, mt_handle *phBuf, BUFMNG_INST_CONFIG_S* pstConfig, mt_u32 pip_en)
{
    mt_s32 s32Ret;
    BUFMNG_INST_S* pstInst;
    mmz_buffer_s stMMZAllocBuf;
    mmz_buffer_s stMMZBuf;
	int i;

	//unused
    //mt_size_t ulFlags;

	ENTER_FUNCTION;

    if ((MT_NULL == phBuf) || (MT_NULL == pstConfig))
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    if ((BUFMNG_ALLOC_BUTT <= pstConfig->enAllocType) || (0 == pstConfig->u32Size))
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    if ((BUFMNG_ALLOC_OUTER == pstConfig->enAllocType) && (0 == pstConfig->u32PhyAddr))
    {
        return MT_ERR_BM_INVALID_PARA;
    }

	//BUG: VES buffer instances count overflow!
	if (s_stBMParam.u16InstNum >= CFG_VDEC_VES_INS_COUNT)
	{
		MLOGE("%s: VES Instance count(%u) full!\n",__FUNCTION__,s_stBMParam.u16InstNum);
		WARN(1, "BUFMNG_Create: VES Instance count full!\n");
        return MT_ERR_BM_NO_MEMORY;
	}
	
    if (BUFMNG_ALLOC_INNER == pstConfig->enAllocType)
    {
    
#ifdef CONFIG_MT_VDEC_PIP_SUPPORT
		if(pip_en == 0)
		{
			s32Ret = mt_drv_mmz_alloc_and_map(pstConfig->aszName, MMZ_ZONE_AV, pstConfig->u32Size, 0, &stMMZAllocBuf);
		}
		else
		{
			s32Ret = mt_drv_mmz_alloc_and_map(pstConfig->aszName, MMZ_ZONE_PIP, pstConfig->u32Size, 0, &stMMZAllocBuf);
		}
#else
		s32Ret = mt_drv_mmz_alloc_and_map(pstConfig->aszName, MMZ_ZONE_AV, pstConfig->u32Size, 0, &stMMZAllocBuf);
#endif

		if (MT_SUCCESS != s32Ret)
	    {
	        MT_FATAL_BUFMNG("Alloc MMZ fail:0x%x.\n", s32Ret);
	        return MT_ERR_BM_NO_MEMORY;
	    }
		else
		{ 
			memset(stMMZAllocBuf.startVirAddr, 0, stMMZAllocBuf.size);
		}

	    pstConfig->u32PhyAddr = stMMZAllocBuf.startPhyAddr;
	    pstConfig->pu8KnlVirAddr = (mt_u8*)stMMZAllocBuf.startVirAddr;
	     //     MT_INFO_BUFMNG("%s: PhyAddr=%x, KnlVirAddr=%p, Size=%x \n", __FUNCTION__,pstConfig->u32PhyAddr, pstConfig->pu8KnlVirAddr, pstConfig->u32Size);
	    //    MLOGI("%s: ES Buff PhyAddr=%x, KnlVirAddr=%p, Size=%x \n", __FUNCTION__,pstConfig->u32PhyAddr, pstConfig->pu8KnlVirAddr, pstConfig->u32Size);
    
    }

    /* Allocate an instance */
//    pstInst = MT_VMALLOC_BUFMNG(sizeof(BUFMNG_INST_S));
    pstInst = MT_KMALLOC_ATOMIC_BUFMNG(sizeof(BUFMNG_INST_S));

    if (MT_NULL == pstInst)
    {
        MT_FATAL_BUFMNG("No memory.\n");
        if (BUFMNG_ALLOC_INNER == pstConfig->enAllocType)
        {
            mt_drv_mmz_unmap_and_release(&stMMZAllocBuf);
        }
        return MT_ERR_BM_NO_MEMORY;
    }

    BUFMNG_LOCK(s_stBMParam.stSem);

    /* Allocate handle */
	//BUG: VES buffer instance handle(hBuf) overflow!
	//NOTE: VES buffer MUST destroy sequentially!!!
	//       Create: 1 2 3 4 5 ...
	//      Destroy:  1 2 3 4 5 ...
    //*phBuf = s_stBMParam.u16InstHandle;
    //*phBuf = s_stBMParam.u16InstHandle % CFG_VDEC_VES_INS_COUNT;

	//Find a free u16InstHandle, then VES buffer NO NEED destroy sequentially!!!
	for (i=0; i<2*CFG_VDEC_VES_INS_COUNT; i++)	/* 2*CFG_VDEC_VES_INS_COUNT: avoid u16InstHandle flow! */
	{
		mt_handle hBufTmp;
		BUFMNG_INST_S* pstInstTmp = MT_NULL;

		hBufTmp = s_stBMParam.u16InstHandle % CFG_VDEC_VES_INS_COUNT;
		BUFMNG_FIND_INST(hBufTmp, pstInstTmp);

		//Found a free u16InstHandle
		if (pstInstTmp == MT_NULL)
			break;

		//Continue next one
		s_stBMParam.u16InstHandle ++;

	}
	if (i >= 2*CFG_VDEC_VES_INS_COUNT)
	{
		//shall not happen!
        MT_FATAL_BUFMNG("No Inst Handle.\n");
		BUG();
	}

    *phBuf = s_stBMParam.u16InstHandle % CFG_VDEC_VES_INS_COUNT;

    /* Init instance parameter */
    pstInst->hBuf = *phBuf;
    MT_INFO_BUFMNG("%s: hBuf=%lx\n", __FUNCTION__,pstInst->hBuf);
    MLOGD("%s: hBuf=%lx\n", __FUNCTION__,pstInst->hBuf);
    pstInst->u32PhyAddr = pstConfig->u32PhyAddr;
    pstInst->pu8UsrVirAddr = pstConfig->pu8UsrVirAddr;

	pstInst->u32DescPhyAddr = 0;
	pstInst->pu8KnlVirDescAddr = NULL;
	pstInst->u32KnlVirDescBufSize = 0;

    if (MT_NULL == pstConfig->pu8KnlVirAddr)
    {
        /* Map to kernel virtual address */
        stMMZBuf.size = pstConfig->u32Size;
        stMMZBuf.startPhyAddr = pstConfig->u32PhyAddr;
        if (MT_SUCCESS != mt_drv_mmz_map(&stMMZBuf))
        {
            BUFMNG_UNLOCK(s_stBMParam.stSem);
            MT_ERR_BUFMNG("mt_drv_mmz_map fail!\n");
            if (BUFMNG_ALLOC_INNER == pstConfig->enAllocType)
            {
                {
                    mt_drv_mmz_unmap_and_release(&stMMZAllocBuf);
                }
            }
            MT_KFREE_BUFMNG(pstInst);
            return MT_FAILURE;
        }

        pstInst->pu8KnlVirAddr = pstConfig->pu8KnlVirAddr = (mt_u8*)stMMZBuf.startVirAddr;
        MT_INFO_BUFMNG("stMMZBuf.u32StartVirAddr:%p\n", stMMZBuf.startVirAddr);
        pstInst->bMMZMap = MT_TRUE;
    }
    else
    {
        pstInst->pu8KnlVirAddr = pstConfig->pu8KnlVirAddr;
        pstInst->bMMZMap = MT_FALSE;
    }

    pstInst->u32Size = pstConfig->u32Size;
    pstInst->enAllocType = pstConfig->enAllocType;
#if (BUFMNG_INDEX_SUPPORT == 1)
    pstInst->u32Index = 0;
#endif
    //MT_INFO_BUFMNG("[BUFMNG_Create] PHY:%p, U-VIR:%p, K-VIR:%p, SIZE:%d\n",
    //           (mt_void*)pstInst->u32PhyAddr, pstInst->pu8UsrVirAddr, pstInst->pu8KnlVirAddr, pstInst->u32Size);

    spin_lock_init(&pstInst->stSpinLock);

    /* Init block list parameter */
    pstInst->u32Free   = pstInst->u32Size;
    pstInst->u32Freeze = 0;
    pstInst->u32Used    = 0;
    pstInst->u32DataNum = 0;
    pstInst->u32GetTry  = 0;
    pstInst->u32GetOK   = 0;
    pstInst->u32PutTry  = 0;
    pstInst->u32PutOK   = 0;
    pstInst->u32RecvTry = 0;
    pstInst->u32RecvOK  = 0;
    pstInst->u32RlsTry  = 0;
    pstInst->u32RlsOK   = 0;
    pstInst->u32KnlWriterOff = 0;
    pstInst->u32GetFail = 0;
    pstInst->u32Offset = 0;
    INIT_LIST_HEAD(&pstInst->stBlockHead);

    /* Add this instance to instance list */
    list_add_tail(&pstInst->stInstNode, &s_stBMParam.stInstHead);
    s_stBMParam.u16InstHandle++;
    s_stBMParam.u16InstNum++;
    BUFMNG_UNLOCK(s_stBMParam.stSem);
	LEAVE_FUNCTION;
    return MT_SUCCESS;
}

mt_s32 BUFMNG_SetUserAddr(mt_handle hBuf, ulong u32Addr)
{
    BUFMNG_INST_S* pstInst = MT_NULL;
    //mt_size_t ulFlags;
    /* Find instance by handle */
    BUFMNG_FIND_INST(hBuf, pstInst);
    if (MT_NULL == pstInst)
    {
        return MT_ERR_BM_INVALID_PARA;
    }

	MLOGD("%s: hBuf 0x%lx, u32Addr 0x%lx\n",__FUNCTION__,hBuf,u32Addr);
    pstInst->pu8UsrVirAddr = (mt_u8*)u32Addr;
    return MT_SUCCESS;
}

mt_s32 BUFMNG_Get(mt_handle hBuf, BUFMNG_INST_CONFIG_S* pstConfig)
{
    BUFMNG_INST_S* pstInst = MT_NULL;
    BUFMNG_FIND_INST(hBuf, pstInst);

    if (MT_NULL == pstInst)
    {
        return MT_ERR_BM_INVALID_PARA;
    }
    pstConfig->u32PhyAddr = pstInst->u32PhyAddr;
    pstConfig->u32Size = pstInst->u32Size;
    pstConfig->u32DescPhyAddr = pstInst->u32DescPhyAddr;
    pstConfig->pu8KnlVirDescAddr = pstInst->pu8KnlVirDescAddr;
    pstConfig->u32KnlVirDescBufSize = pstInst->u32KnlVirDescBufSize;

	return MT_SUCCESS;
}

mt_s32 BUFMNG_Destroy_forUsrData(mt_handle hBuf)
{
    BUFMNG_INST_S* pstInst = MT_NULL;
    mmz_buffer_s stMMZBuf;
    mt_size_t ulFlags;

//	MLOGD("%s: hBuf=%u\n",__FUNCTION__,hBuf);

    /* Find instance by handle */
    BUFMNG_FIND_INST(hBuf, pstInst);
    if (MT_NULL == pstInst)
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    BUFMNG_SPIN_LOCK(pstInst->stSpinLock);

    /* If has blocks, free them */
    if (!list_empty(&pstInst->stBlockHead))
    {
        BUFMNG_FREE_BLOCK_LIST(pstInst);
    }

#if (BUFMNG_DEBUG == 1)
    MT_ERR_BUFMNG("Before destroy, used %d, free %d, freeze %d\n",
              pstInst->u32Used, pstInst->u32Free, pstInst->u32Freeze);
#endif
    BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);

    /* If need, Unmap */
    if (pstInst->bMMZMap)
    {
		// TODO: REMOVE BUFMNG_LOCK
        BUFMNG_SPIN_LOCK(pstInst->stSpinLock);

        stMMZBuf.startPhyAddr = pstInst->u32PhyAddr;
        stMMZBuf.startVirAddr = (void *)pstInst->pu8KnlVirAddr;
        stMMZBuf.size = pstInst->u32Size;
	    BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
        mt_drv_mmz_unmap(&stMMZBuf);
    }

    /* If need, free */
    if (BUFMNG_ALLOC_INNER == pstInst->enAllocType)
    {
        {
            BUFMNG_SPIN_LOCK(pstInst->stSpinLock);
            stMMZBuf.startPhyAddr = pstInst->u32PhyAddr;
            stMMZBuf.startVirAddr = (void *)pstInst->pu8KnlVirAddr;
            stMMZBuf.size = pstInst->u32Size;
	          BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
//			MLOGI("%s: free UsrData MMZ Buff(%x %x %u)\n",__FUNCTION__,
//				stMMZBuf.startPhyAddr,stMMZBuf.u32StartVirAddr,stMMZBuf.u32Size);
            mt_drv_mmz_unmap_and_release(&stMMZBuf);
        }
    }

    BUFMNG_LOCK(s_stBMParam.stSem);

    /* Remove instance from list */
    list_del(&pstInst->stInstNode);

    if (s_stBMParam.u16InstNum > 0)
    {
        s_stBMParam.u16InstNum--;
    }

    BUFMNG_UNLOCK(s_stBMParam.stSem);

    /* Free resource */
    MT_KFREE_BUFMNG(pstInst);

    return MT_SUCCESS;
}

mt_s32 BUFMNG_Destroy(mt_handle hBuf)
{
    BUFMNG_INST_S* pstInst = MT_NULL;
    mmz_buffer_s stMMZBuf;
    mt_size_t ulFlags;

	ENTER_FUNCTION;

    /* Find instance by handle */
    BUFMNG_FIND_INST(hBuf, pstInst);
    if (MT_NULL == pstInst)
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    BUFMNG_SPIN_LOCK(pstInst->stSpinLock);

    /* If has blocks, free them */
    if (!list_empty(&pstInst->stBlockHead))
    {
        BUFMNG_FREE_BLOCK_LIST(pstInst);
    }

#if (BUFMNG_DEBUG == 1)
    MT_ERR_BUFMNG("Before destroy, used %d, free %d, freeze %d\n",
              pstInst->u32Used, pstInst->u32Free, pstInst->u32Freeze);
#endif
    BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);

    /* If need, Unmap */
    if (pstInst->bMMZMap)
    {
		// TODO: REMOVE BUFMNG_LOCK
        BUFMNG_SPIN_LOCK(pstInst->stSpinLock);

        stMMZBuf.startPhyAddr = pstInst->u32PhyAddr;
        stMMZBuf.startVirAddr = (void *)pstInst->pu8KnlVirAddr;
        stMMZBuf.size = pstInst->u32Size;
	    BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
        mt_drv_mmz_unmap(&stMMZBuf);
    }

    /* If need, free */
    if (BUFMNG_ALLOC_INNER == pstInst->enAllocType)
    {
        {
            BUFMNG_SPIN_LOCK(pstInst->stSpinLock);
            stMMZBuf.startPhyAddr = pstInst->u32PhyAddr;
            stMMZBuf.startVirAddr = (void *)pstInst->pu8KnlVirAddr;
            stMMZBuf.size = pstInst->u32Size;
	          BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
//			MLOGI("%s: free ES MMZ Buff(%x %x %u)\n",__FUNCTION__,
//				stMMZBuf.startPhyAddr,stMMZBuf.startVirAddr,stMMZBuf.size);
            mt_drv_mmz_unmap_and_release(&stMMZBuf);

            BUFMNG_SPIN_LOCK(pstInst->stSpinLock);
            stMMZBuf.startPhyAddr = pstInst->u32DescPhyAddr;
            stMMZBuf.startVirAddr = (void *)pstInst->pu8KnlVirDescAddr;
            stMMZBuf.size = pstInst->u32KnlVirDescBufSize;
	        BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);

			if (stMMZBuf.startPhyAddr != 0)
			{
//				MLOGI("%s: free ES Descriptor MMZ Buff(%llx %lx %lu)\n",__FUNCTION__,
//					stMMZBuf.startPhyAddr,stMMZBuf.startVirAddr,stMMZBuf.size);
	            mt_drv_mmz_unmap_and_release(&stMMZBuf);
           	}
        }
    }

    BUFMNG_LOCK(s_stBMParam.stSem);

    /* Remove instance from list */
    list_del(&pstInst->stInstNode);

    if (s_stBMParam.u16InstNum > 0)
    {
        s_stBMParam.u16InstNum--;
    }

    BUFMNG_UNLOCK(s_stBMParam.stSem);

    /* Free resource */
    MT_KFREE_BUFMNG(pstInst);

	LEAVE_FUNCTION;
    return MT_SUCCESS;
}


mt_s32 BUFMNG_GetWriteBuffer_forUsrData(mt_handle hBuf, BUFMNG_BUF_S *pstBuf)
{
    BUFMNG_INST_S* pstInst   = MT_NULL;
    BUFMNG_BLOCK_S* pstBlock = MT_NULL;
    BUFMNG_BLOCK_S* pstBlockRead = MT_NULL;
    mt_u32 u32ReadOffset = 0;
    mt_u32 u32WriteOffset = 0;
    mt_u32 u32TailFree = 0;
    mt_u32 u32HeadFree = 0;
    MT_BOOL bAlloc = MT_FALSE;
    mt_size_t ulFlags = 0;

    if (MT_NULL == pstBuf)
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    BUFMNG_FIND_INST(hBuf, pstInst);
    if (MT_NULL == pstInst)
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    BUFMNG_SPIN_LOCK(pstInst->stSpinLock);

    /* Get buffer try times increase */
    pstInst->u32GetTry++;

    /* Buffer full */
    if (IS_FULL(pstInst, pstBuf->u32Size))
    {
        BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
        return MT_ERR_BM_BUFFER_FULL;
    }

    /* If empty, write from start */
    if (list_empty(&pstInst->stBlockHead))
    {
        u32WriteOffset = u32ReadOffset = 0;
    }
    /* Else, find write offset and read offset */
    else
    {
        /* Only support one writer */
        BUFMNG_FIND_TAIL_BLOCK(pstInst, pstBlock);
        if (IS_WRITING(pstBlock))
        {
            pstBuf->u32PhyAddr = pstBlock->u32Addr;
            pstBuf->u32Size = pstBlock->u32Size;
            pstBuf->pu8KnlVirAddr = (MT_NULL==pstInst->pu8KnlVirAddr) ? MT_NULL :
                                    (pstBlock->u32Addr - pstInst->u32PhyAddr )+ pstInst->pu8KnlVirAddr;
            pstBuf->pu8UsrVirAddr = (MT_NULL==pstInst->pu8UsrVirAddr) ? MT_NULL :
                                    (pstBlock->u32Addr - pstInst->u32PhyAddr) + pstInst->pu8UsrVirAddr;
            BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
            MT_ERR_BUFMNG("BUFMNG BUSY\n");
            return MT_ERR_BM_BUSY;
        }

        u32WriteOffset = pstBlock->u32Addr + pstBlock->u32Size - pstInst->u32PhyAddr;
#if (BUFMNG_DEBUG == 1)
        if (u32WriteOffset > pstInst->u32Size)
        {
            MT_FATAL_BUFMNG("!!!!!!!!!!! [ u32WriteOffset ERROR ] !!!!!!!!!!!!\n");
            MT_FATAL_BUFMNG("u32WriteOffset = %d\n", u32WriteOffset);
            MT_FATAL_BUFMNG("pstBlock->u32Addr = %p\n", (mt_void*)pstBlock->u32Addr);
            MT_FATAL_BUFMNG("pstBlock->u32Size = %d\n", pstBlock->u32Size);
            MT_FATAL_BUFMNG("pstInst->u32PhyAddr = %p\n", (mt_void*)pstInst->u32PhyAddr);
        }
#endif

        BUFMNG_FIND_HEAD_BLOCK(pstInst, pstBlockRead);
        u32ReadOffset = pstBlockRead->u32Addr - pstInst->u32PhyAddr;
#if (BUFMNG_DEBUG == 1)
        if (u32ReadOffset > pstInst->u32Size)
        {
            MT_FATAL_BUFMNG("!!!!!!!!!!!! [ u32ReadOffset ERROR ] !!!!!!!!!!!!!!\n");
            MT_FATAL_BUFMNG("u32ReadOffset = %d\n", u32ReadOffset);
            MT_FATAL_BUFMNG("pstBlockRead->u32Addr = %p\n", (mt_void*)pstBlockRead->u32Addr);
            MT_FATAL_BUFMNG("pstBlockRead->u32Size = %d\n", pstBlockRead->u32Size);
            MT_FATAL_BUFMNG("pstInst->u32PhyAddr = %p\n", (mt_void*)pstInst->u32PhyAddr);
        }
#endif

    }

    /* Reverse: write pointer before read pointer, the free area is continuous */
    if (u32WriteOffset <= u32ReadOffset)
    {
        bAlloc = MT_TRUE;

#if (BUFMNG_GET_SMSP == 1)
        /* For return size */
        u32TailFree = pstInst->u32Free;
#endif
    }
    /* Normal: write pointer after read pointer, the free area isn't continuous */
    else
    {
        u32TailFree = pstInst->u32Size - u32WriteOffset;
        u32HeadFree = pstInst->u32Free - u32TailFree;

#if (BUFMNG_DEBUG == 1)
        if ((u32TailFree > pstInst->u32Free) || (u32TailFree > pstInst->u32Size))
        {
            MT_FATAL_BUFMNG("!!!!!!!!!!!! [ u32TailFree ERROR ] !!!!!!!!!!!!!!\n");
            if (pstBlock)
            {
                MT_FATAL_BUFMNG("u32WriteOffset = %d\n", u32WriteOffset);
                MT_FATAL_BUFMNG("pstBlock->u32Addr = %p\n", (mt_void*)pstBlock->u32Addr);
                MT_FATAL_BUFMNG("pstBlock->u32Size = %d\n", pstBlock->u32Size);
            }

            if (pstBlockRead)
            {
                MT_FATAL_BUFMNG("u32ReadOffset = %d\n", u32ReadOffset);
                MT_FATAL_BUFMNG("pstBlockRead->u32Addr = %p\n", (mt_void*)pstBlockRead->u32Addr);
                MT_FATAL_BUFMNG("pstBlockRead->u32Size = %d\n", pstBlockRead->u32Size);
            }

            MT_FATAL_BUFMNG("pstInst->u32PhyAddr = %p\n", (mt_void*)pstInst->u32PhyAddr);
            MT_FATAL_BUFMNG("u32WriteOffset = %d\n", u32WriteOffset);
            MT_FATAL_BUFMNG("u32ReadOffset = %d\n", u32ReadOffset);
            MT_FATAL_BUFMNG("pstInst->u32Size = %d\n", pstInst->u32Size);
            MT_FATAL_BUFMNG("pstInst->u32Free = %d\n", pstInst->u32Free);
            MT_FATAL_BUFMNG("u32TailFree = %d\n", u32TailFree);
            MT_FATAL_BUFMNG("u32HeadFree = %d\n", u32HeadFree);
        }

        if (u32HeadFree > pstInst->u32Size)
        {
            MT_FATAL_BUFMNG("!!!!!!!!!!!! [ u32HeadFree ERROR ] !!!!!!!!!!!!!!\n");
            MT_FATAL_BUFMNG("u32WriteOffset = %d\n", u32WriteOffset);
            MT_FATAL_BUFMNG("u32ReadOffset = %d\n", u32ReadOffset);
            MT_FATAL_BUFMNG("pstInst->u32Size = %d\n", pstInst->u32Size);
            MT_FATAL_BUFMNG("pstInst->u32Free = %d\n", pstInst->u32Free);
            MT_FATAL_BUFMNG("u32TailFree = %d\n", u32TailFree);
            MT_FATAL_BUFMNG("u32HeadFree = %d\n", u32HeadFree);
        }
#endif

        if (u32TailFree >= pstBuf->u32Size)
        {
            bAlloc = MT_TRUE;
        }
        else if (u32HeadFree >= pstBuf->u32Size)
        {
            bAlloc = MT_TRUE;

            /* Alloc from head */
            u32WriteOffset = 0;

            /* Freeze the last area */
            pstInst->u32Freeze = u32TailFree;
            pstInst->u32Free -= u32TailFree;

#if (BUFMNG_GET_SMSP == 1)
            /* For return size */
            u32TailFree = pstInst->u32Free;
#endif
        }
    }

    /* Allocate fail, return */
    if (!bAlloc)
    {
        BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
        return MT_ERR_BM_BUFFER_FULL;
    }

    /* Allocate new block */
    pstBlock = MT_KMALLOC_ATOMIC_BUFMNG(sizeof(BUFMNG_BLOCK_S));
    if (MT_NULL == pstBlock)
    {
        BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
        return MT_ERR_BM_NO_MEMORY;
    }

    /* Init block parameter */
#if (BUFMNG_GET_SMSP == 1)
    pstBlock->u32Size = pstBuf->u32Size = u32TailFree;
#else
    pstBlock->u32Size = pstBuf->u32Size;
#endif
    pstBuf->u32PhyAddr = pstBlock->u32Addr = pstInst->u32PhyAddr + u32WriteOffset;

    MT_INFO_VDEC("UUUUUUUUUUU BUFMNG_GetWriteBuffer pstBuf->u32PhyAddr = %x \n", pstBuf->u32PhyAddr);
    if (0 != pstInst->pu8UsrVirAddr)
    {
        pstBuf->pu8UsrVirAddr = pstInst->pu8UsrVirAddr + u32WriteOffset;
    }
    else
    {
        pstBuf->pu8UsrVirAddr = MT_NULL;
    }

    pstBuf->pu8KnlVirAddr = pstInst->pu8KnlVirAddr + u32WriteOffset;
#if (BUFMNG_64BITS_PTS_SUPPORT == 1)
    pstBuf->u64Pts = pstBlock->u64Pts = BUFMNG_INVALID_64BITS_PTS;
#else
    pstBuf->u64Pts = pstBlock->u32Pts = BUFMNG_INVALID_PTS;
#endif
#if (BUFMNG_INDEX_SUPPORT == 1)
    pstBuf->u32Index = pstBlock->u32Index = pstInst->u32Index++;
#endif
#if (BUFMNG_MARKER_SUPPORT == 1)
    pstBuf->u32Marker = pstBlock->u32Marker = 0;
#else
    pstBuf->u32Marker = 0;
#endif
    pstBlock->enstatus = BUFMNG_BLOCK_WRITING;

    /* Add block to list */
    list_add_tail(&pstBlock->stBlockNode, &pstInst->stBlockHead);

    /* Get buffer OK times increase */
    pstInst->u32GetOK++;

    BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);

    return MT_SUCCESS;
}

mt_s32 BUFMNG_GetWriteBuffer(mt_handle hBuf, BUFMNG_BUF_S *pstBuf)
{
    BUFMNG_INST_S* pstInst   = MT_NULL;
    //unused
    //BUFMNG_BLOCK_S* pstBlockRead = MT_NULL;
    mt_u32 u32ReadOffset;
    mt_u32 u32WriteOffset;
    //unused
    //mt_u32 u32TailFree;
    //mt_u32 u32HeadFree;
    //MT_BOOL bAlloc = MT_FALSE;
    mt_size_t ulFlags;
    VES_BUF_STATE_S vdec_es_buffer_status;
	mt_s32 ChanID;
	mt_s32 Ret;

    if (MT_NULL == pstBuf)
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    BUFMNG_FIND_INST(hBuf, pstInst);
    if (MT_NULL == pstInst)
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    BUFMNG_SPIN_LOCK(pstInst->stSpinLock);

    /* Get buffer try times increase */
    pstInst->u32GetTry++;

    //printk("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! BUFMNG_GetWriteBuffer \n");

	//statement with no effect
    //IS_FULL(pstInst, pstBuf->u32Size);

    u32WriteOffset = u32ReadOffset = 0;

    pstBuf->u32PhyAddr = pstInst->u32PhyAddr + u32WriteOffset;
    //printk("UUUU BUFMNG_GetWriteBuffer pstBuf->u32PhyAddr = %x \n", pstBuf->u32PhyAddr);

	Ret = VDEC_FindChanIDByESBufferHandle(hBuf, &ChanID);
	if (Ret != MT_SUCCESS || ChanID == MT_INVALID_HANDLE)
	{
        BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
		MLOGE("%s: find Chan ID failed!\n",__FUNCTION__);
        return MT_ERR_BM_INVALID_PARA;
	}
    KERN_VDEC_Control(ChanID, VDEC_CID_GET_CHAN_VES_BUF_STATE, &vdec_es_buffer_status);

    //printk("AAAA  u32Size = %x, buf_left = %x \n", pstBuf->u32Size, vdec_es_buffer_status.buf_left);
    /* Buffer full */
    if ((pstBuf->u32Size + 100 * 1024) > vdec_es_buffer_status.buf_left)
    {
        BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
        //printk("BBBB  u32Size = %x, buf_left = %x \n", pstBuf->u32Size, vdec_es_buffer_status.buf_left);
/* dump es buffer stat */
//#ifdef CONFIG_MT_DEBUG_V_BUF_STAT
		if ((pstInst->u32GetFail % DEBUG_DUMP_INTERVAL) == 0)
		{
//			MT_INFO_VDEC("\n");
//			MLOGW("[V ES BUF STAT][%s]: h %u, Addr 0x%x, Size 0x%x, Used 0x%x, LFT 0x%x, RD 0x%x, WR 0x%x; [DESC] RD 0x%x, WR 0x%x - failed!\n\n",__FUNCTION__,
//					pstInst->hBuf,
//					pstInst->u32PhyAddr,
//					vdec_es_buffer_status.buf_size,
//					vdec_es_buffer_status.buf_used,
//					vdec_es_buffer_status.buf_left,
//					vdec_es_buffer_status.read_pointer,
//					vdec_es_buffer_status.write_pointer,
//					vdec_descriptor_get_read_ptr(hBuf),
//					vdec_descriptor_get_write_ptr(hBuf));
		}
//#endif

		pstInst->u32GetFail ++;
        return MT_ERR_BM_BUFFER_FULL;
    }

    //printk("0000 u32ScrapSize = %x \n", pstBuf->u32ScrapSize);

    u32WriteOffset = vdec_es_buffer_status.write_pointer + pstBuf->u32ScrapSize;
    u32ReadOffset = vdec_es_buffer_status.read_pointer;
    pstBuf->u32PhyAddr = pstInst->u32PhyAddr + u32WriteOffset;

    if (0 != pstInst->pu8UsrVirAddr)
    {
        pstBuf->pu8UsrVirAddr = pstInst->pu8UsrVirAddr + u32WriteOffset;
    }
    else
    {
        pstBuf->pu8UsrVirAddr = MT_NULL;
    }

    pstBuf->pu8KnlVirAddr = pstInst->pu8KnlVirAddr + u32WriteOffset;

    if((u32WriteOffset + pstBuf->u32Size) > vdec_es_buffer_status.buf_size)
    {
      pstBuf->u32ESBufLeftSize = vdec_es_buffer_status.buf_size - u32WriteOffset;
      //rewind
      //MLOGI("%s: u32ESBufLeftSize = %x,  u32WriteOffset = %x \n", __FUNCTION__, pstBuf->u32ESBufLeftSize, u32WriteOffset);
    }
    else
    {
      pstBuf->u32ESBufLeftSize = pstBuf->u32Size;
    }

	//Check RD/WR
	if (u32WriteOffset > u32ReadOffset)
	{
		if (u32WriteOffset + pstBuf->u32ESBufLeftSize >= vdec_es_buffer_status.buf_size
			&& u32WriteOffset + pstBuf->u32ESBufLeftSize - vdec_es_buffer_status.buf_size >= u32ReadOffset)
		{
			MLOGE("%s: WR(0x%x) + SZ(0x%x) >= RD(0x%x)\n",__FUNCTION__,
					u32WriteOffset,
					pstBuf->u32ESBufLeftSize,
					u32ReadOffset);
		}
	}
	else if (u32WriteOffset < u32ReadOffset)
	{
		if (u32WriteOffset + pstBuf->u32ESBufLeftSize >= u32ReadOffset)
		{
			MLOGE("%s: WR(0x%x) + SZ(0x%x) >= RD(0x%x)\n",__FUNCTION__,
					u32WriteOffset,
					pstBuf->u32ESBufLeftSize,
					u32ReadOffset);
		}
	}

#if (BUFMNG_64BITS_PTS_SUPPORT == 1)
    pstBuf->u64Pts = BUFMNG_INVALID_64BITS_PTS;
#else
    pstBuf->u64Pts = BUFMNG_INVALID_PTS;
#endif
#if (BUFMNG_INDEX_SUPPORT == 1)
    pstBuf->u32Index = pstInst->u32Index++;
#endif
#if (BUFMNG_MARKER_SUPPORT == 1)
    pstBuf->u32Marker  = 0;
#else
    pstBuf->u32Marker = 0;
#endif

   /* Get buffer OK times increase */
    pstInst->u32GetOK++;

    BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);

    return MT_SUCCESS;
}

mt_s32 BUFMNG_PutWriteBuffer_forUsrData(mt_handle hBuf, BUFMNG_BUF_S *pstBuf)
{
    phys_addr_t u32PhyAddr;
    BUFMNG_INST_S* pstInst   = MT_NULL;
    BUFMNG_BLOCK_S* pstBlock = MT_NULL;
    mt_size_t ulFlags;

    if (MT_NULL == pstBuf)
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    BUFMNG_FIND_INST(hBuf, pstInst);
    if (MT_NULL == pstInst)
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    BUFMNG_SPIN_LOCK(pstInst->stSpinLock);

    /* Put buffer try times increase */
    pstInst->u32PutTry++;

    /* Find the tail block */
    BUFMNG_FIND_TAIL_BLOCK(pstInst, pstBlock);

    if (pstBuf->u32Size > pstBlock->u32Size)
    {
        BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
        MT_ERR_BUFMNG("Put size err:%d>%d\n", pstBuf->u32Size, pstBlock->u32Size);
        return MT_ERR_BM_WRITE_FREE_ERR;
    }

    if (0 != pstBuf->pu8UsrVirAddr)
    {
        u32PhyAddr = (pstBuf->pu8UsrVirAddr - pstInst->pu8UsrVirAddr) + pstInst->u32PhyAddr;
		pstBuf->pu8KnlVirAddr = pstBuf->pu8UsrVirAddr - pstInst->pu8UsrVirAddr + pstInst->pu8KnlVirAddr;
    }
    else
    {
        u32PhyAddr = (pstBuf->pu8KnlVirAddr - pstInst->pu8KnlVirAddr) + pstInst->u32PhyAddr;
		pstBuf->pu8UsrVirAddr = (pstBuf->pu8KnlVirAddr - pstInst->pu8KnlVirAddr) + pstInst->pu8UsrVirAddr;
    }

    /* The block must be WRITING status and its address must be right */
    if (IS_WRITING(pstBlock) && (u32PhyAddr == pstBlock->u32Addr))
    {
        /* If size=0, drop this block */
        if (0 == pstBuf->u32Size)
        {
            /* Delete block from list */
            list_del(&pstBlock->stBlockNode);

            /* Free block resource */
            MT_KFREE_BUFMNG(pstBlock);

            pstInst->u32PutOK++;
            BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
            return MT_SUCCESS;
        }

#if (BUFMNG_MARKER_SUPPORT == 1)
        pstBlock->u32Marker = pstBuf->u32Marker;
#endif
#if (BUFMNG_64BITS_PTS_SUPPORT == 1)
        pstBlock->u64Pts = pstBuf->u64Pts;
#else
        pstBlock->u32Pts = (mt_u32)(pstBuf->u64Pts);
#endif
        pstBlock->u32Size  = pstBuf->u32Size;
        pstBlock->enstatus = BUFMNG_BLOCK_FREE;
    }
    else
    {
        BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
        MT_ERR_BUFMNG("Put fail.\n");
        return MT_ERR_BM_WRITE_FREE_ERR;
    }

    /* Manage instance */
    pstInst->u32Used += pstBuf->u32Size;
    pstInst->u32Free -= pstBuf->u32Size;

#if (BUFMNG_MARKER_SUPPORT == 1)
    /* Add up frame number */
    if (0 == (pstBlock->u32Marker & BUFMNG_NOT_END_FRAME_BIT))
    {
        pstInst->u32DataNum++;
    }
#endif

#if (BUFMNG_DEBUG == 1)
    if (pstInst->u32Size != pstInst->u32Used + pstInst->u32Free + pstInst->u32Freeze)
    {
        MT_FATAL_BUFMNG("!!!!!!!!!!!!!!!!!UNmatch when write!!!!!!!!!!!!!!!!!!\n");
        MT_FATAL_BUFMNG("pstInst->u32Size = %d\n", pstInst->u32Size);
        MT_FATAL_BUFMNG("pstInst->u32Used = %d\n", pstInst->u32Used);
        MT_FATAL_BUFMNG("pstInst->u32Free = %d\n", pstInst->u32Free);
        MT_FATAL_BUFMNG("pstInst->u32Freeze = %d\n", pstInst->u32Freeze);
    }
#endif

    /* Put buffer OK times increase */
    pstInst->u32PutOK++;

    BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
    return MT_SUCCESS;
}
//#define CONFIG_MT_DEBUG_V_BUF_STAT
mt_s32 BUFMNG_PutWriteBuffer(mt_handle hBuf, BUFMNG_BUF_S *pstBuf)
{
	//unused
    //mt_u32 u32PhyAddr;
    BUFMNG_INST_S* pstInst   = MT_NULL;
    //unused
    //BUFMNG_BLOCK_S* pstBlock = MT_NULL;
    mt_size_t ulFlags;
    VES_BUF_STATE_S vdec_es_buffer_status;
    mt_u32 u32WriteOffset, u32ReadOffset;
    mt_u32 u32ESBegin, u32ESEnd;
	mt_s32 ChanID;
	mt_s32 Ret;

    if (MT_NULL == pstBuf)
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    if (pstBuf->u32Size <= 0)
    {
    	MLOGE("%s: Size(%u) invalid!\n",__FUNCTION__,pstBuf->u32Size);
        return MT_ERR_BM_INVALID_PARA;
    }

    BUFMNG_FIND_INST(hBuf, pstInst);
    if (MT_NULL == pstInst)
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    BUFMNG_SPIN_LOCK(pstInst->stSpinLock);

    /* Put buffer try times increase */
    pstInst->u32PutTry++;

    /* Find the tail block */
    //BUFMNG_FIND_TAIL_BLOCK(pstInst, pstBlock);
	Ret = VDEC_FindChanIDByESBufferHandle(hBuf, &ChanID);
	if (Ret != MT_SUCCESS || ChanID == MT_INVALID_HANDLE)
	{
        BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
		MLOGE("%s: find Chan ID failed!\n",__FUNCTION__);
        return MT_ERR_BM_INVALID_PARA;
	}
    KERN_VDEC_Control(ChanID, VDEC_CID_GET_CHAN_VES_BUF_STATE, &vdec_es_buffer_status);

	//Debug
	//Note: 在上层应用
	//      BUFMNG_GetWriteBuffer -> ... -> BUFMNG_PutWriteBuffer
	//      的过程中,FW可能由于有错误数据,而做了一次Reset,重置了Write指针.
	//      从而可能会出现数据拷贝的地址,和最后更新的Write指针不一致的情况.
	if (pstInst->u32KnlWriterOff != vdec_es_buffer_status.write_pointer)
	{
		MLOGE("%s: WR, pre 0x%x, cur 0x%x\n",__FUNCTION__,
			pstInst->u32KnlWriterOff,
			vdec_es_buffer_status.write_pointer);
#ifdef DEBUG
		WARN_ON(1);
#endif
	}

    if (pstBuf->u32Size > vdec_es_buffer_status.buf_left)
    {
        BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
        MLOGE("%s: Put size err:%d>%d\n", __FUNCTION__, pstBuf->u32Size, vdec_es_buffer_status.buf_left);
        return MT_ERR_BM_WRITE_FREE_ERR;
    }

	//Check RD/WR
    u32WriteOffset = vdec_es_buffer_status.write_pointer + pstBuf->u32ScrapSize;
	u32ReadOffset = vdec_es_buffer_status.read_pointer;
    pstBuf->pu8KnlVirAddr = pstInst->pu8KnlVirAddr + u32WriteOffset;
	if (u32WriteOffset > u32ReadOffset)
	{
		if (u32WriteOffset + pstBuf->u32Size >= vdec_es_buffer_status.buf_size
			&& u32WriteOffset + pstBuf->u32Size - vdec_es_buffer_status.buf_size >= u32ReadOffset)
		{
			MLOGE("%s: WR(0x%x) + SZ(0x%x) >= RD(0x%x)\n",__FUNCTION__,
					u32WriteOffset,
					pstBuf->u32Size,
					u32ReadOffset);
		}
	}
	else if (u32WriteOffset < u32ReadOffset)
	{
		if (u32WriteOffset + pstBuf->u32Size >= u32ReadOffset)
		{
			MLOGE("%s: WR(0x%x) + SZ(0x%x) >= RD(0x%x)\n",__FUNCTION__,
					u32WriteOffset,
					pstBuf->u32Size,
					u32ReadOffset);
		}
	}

    if((vdec_es_buffer_status.write_pointer + pstBuf->u32Size) >= vdec_es_buffer_status.buf_size)
    {
      u32WriteOffset = (vdec_es_buffer_status.write_pointer + pstBuf->u32Size) - vdec_es_buffer_status.buf_size;
    }
    else
    {
      u32WriteOffset = vdec_es_buffer_status.write_pointer + pstBuf->u32Size;
    }
    
    u32WriteOffset += pstBuf->u32ScrapSize;
    u32ESBegin = vdec_es_buffer_status.write_pointer+pstBuf->u32ScrapSize;
    u32ESEnd   = u32WriteOffset-1;
    pstBuf->u32ScrapSize = u32WriteOffset;
    u32WriteOffset = (u32WriteOffset >> 3) << 3;
    pstBuf->u32ScrapSize -= u32WriteOffset;
    //FIXME: buf_size must aligned 1M/2M/4M/8M/...?
    //vdec_es_buffer_status.write_pointer = u32WriteOffset & (vdec_es_buffer_status.buf_size - 1);
    BUG_ON(vdec_es_buffer_status.buf_size == 0);
    vdec_es_buffer_status.write_pointer = u32WriteOffset % vdec_es_buffer_status.buf_size;
    KERN_VDEC_Control(ChanID, VDEC_CID_SET_CHAN_VES_BUF_STATE, &vdec_es_buffer_status);
	_BUFMNG_PutDescriptor(pstInst, pstBuf, u32ESBegin, u32ESEnd);

	//Debug
	pstInst->u32KnlWriterOff = vdec_es_buffer_status.write_pointer;

    //printk("1111 u32ScrapSize = %x, u32WriteOffset = %x \n", pstBuf->u32ScrapSize, u32WriteOffset);

#if (BUFMNG_MARKER_SUPPORT == 1)
    /* Add up frame number */
    //if (0 == (pstBlock->u32Marker & BUFMNG_NOT_END_FRAME_BIT))
    //{
   //     pstInst->u32DataNum++;
   // }
#endif

    /* Put buffer OK times increase */
    pstInst->u32PutOK++;

#ifdef CONFIG_MT_DUMP_V_ES_BUF_AT_PUT
	vdec_dump_es_write(pstBuf->pu8KnlVirAddr, pstBuf->u32Size);

	//Format: Frame No. | Offset | Length | Flags(Type,PTS Valid,EOS...) | PTS
	VDEBUG_LOG("[Frame Index] %7d, OFS %10d, SZ %8d, Flg(PTS %d, EOS %d), PTS %llu\n",
			(pstInst->u32PutOK-1),
			pstInst->u32Offset,
			pstBuf->u32Size,
			pstBuf->u32PtsValide,
			pstBuf->u32EosFlag,
			pstBuf->u64Pts);

	pstInst->u32Offset += pstBuf->u32Size;
#endif

//Video ES Bit-Error-Ratio Disturb
#ifdef CONFIG_MT_V_BER_TEST
	vdec_ber_disturb(pstBuf->pu8KnlVirAddr, pstBuf->u32Size);
#endif

    BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);

/* dump es buffer stat */
#ifdef CONFIG_MT_DEBUG_V_BUF_STAT
	VDEBUG_LOG("[V ES BUF STAT][%s]: DSize 0x%x, h %lx, Addr 0x%lx, Size 0x%x, Usded 0x%x, RD 0x%x, WR 0x%x\n\n",__FUNCTION__,
				pstBuf->u32Size,
				pstInst->hBuf,
				(ulong)pstInst->u32PhyAddr,
				vdec_es_buffer_status.buf_size,
				vdec_es_buffer_status.buf_used,
				vdec_es_buffer_status.read_pointer,
				vdec_es_buffer_status.write_pointer);
#endif

    return MT_SUCCESS;
}


mt_s32 BUFMNG_AcqReadBuffer(mt_handle hBuf, BUFMNG_BUF_S *pstBuf)
{
    BUFMNG_INST_S* pstInst   = MT_NULL;
    BUFMNG_BLOCK_S* pstBlock = MT_NULL;
    mt_size_t ulFlags;

    if (MT_NULL == pstBuf)
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    pstBuf->u32Size = 0;

    BUFMNG_FIND_INST(hBuf, pstInst);
    if (MT_NULL == pstInst)
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    BUFMNG_SPIN_LOCK(pstInst->stSpinLock);

    /* Request buffer try times increase */
    pstInst->u32RecvTry++;

    /* Buffer empty */
    if ((0 == pstInst->u32Used) || (list_empty(&pstInst->stBlockHead)))
    {
        BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
        return MT_ERR_BM_BUFFER_EMPTY;
    }

#if (BUFMNG_MULTI_READ_SUPPORT == 0)
    /* Find head block */
    BUFMNG_FIND_HEAD_BLOCK(pstInst, pstBlock);

    /* The block must be FREE status */
    if (!IS_FREE(pstBlock))
    {
        BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
        return MT_ERR_BM_BUFFER_EMPTY;
    }

#else
    /* Find first free block */
    BUFMNG_FIND_FIRST_FREE_BLOCK(pstInst, pstBlock);
    if (MT_NULL == pstBlock)
    {
        BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
        return MT_ERR_BM_BUFFER_EMPTY;
    }
#endif


    /* Read from the block */
    pstBuf->u32PhyAddr = pstBlock->u32Addr;
    pstBuf->pu8KnlVirAddr = pstBlock->u32Addr - pstInst->u32PhyAddr + pstInst->pu8KnlVirAddr;
    if (MT_NULL != pstInst->pu8UsrVirAddr)
    {
        pstBuf->pu8UsrVirAddr = pstBlock->u32Addr - pstInst->u32PhyAddr + pstInst->pu8UsrVirAddr;
    }
    else
    {
        pstBuf->pu8UsrVirAddr = MT_NULL;
    }
#if (BUFMNG_64BITS_PTS_SUPPORT == 1)
    pstBuf->u64Pts = pstBlock->u64Pts;
#else
    pstBuf->u64Pts = pstBlock->u32Pts;
#endif
    pstBuf->u32Size = pstBlock->u32Size;
#if (BUFMNG_INDEX_SUPPORT == 1)
    pstBuf->u32Index = pstBlock->u32Index;
#endif
#if (BUFMNG_MARKER_SUPPORT == 1)
    pstBuf->u32Marker = pstBlock->u32Marker;
#else
    pstBuf->u32Marker = 0;
#endif

    /* Change status */
    pstBlock->enstatus = BUFMNG_BLOCK_READING;

    /* Request buffer OK times increase */
    pstInst->u32RecvOK++;

    BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
    return MT_SUCCESS;
}

mt_s32 BUFMNG_RlsReadBuffer(mt_handle hBuf, BUFMNG_BUF_S *pstBuf)
{
    phys_addr_t u32PhyAddr;
    BUFMNG_INST_S* pstInst   = MT_NULL;
    BUFMNG_BLOCK_S* pstBlock = MT_NULL;
    mt_size_t ulFlags;

    if (MT_NULL == pstBuf)
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    BUFMNG_FIND_INST(hBuf, pstInst);
    if (MT_NULL == pstInst)
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    BUFMNG_SPIN_LOCK(pstInst->stSpinLock);

    /* Release buffer try times increase */
    pstInst->u32RlsTry++;

    /* Find block by address */
    if (MT_NULL != pstBuf->pu8UsrVirAddr)
    {
        u32PhyAddr = pstBuf->pu8UsrVirAddr - pstInst->pu8UsrVirAddr + pstInst->u32PhyAddr;
    }
    else
    {
        u32PhyAddr = pstBuf->pu8KnlVirAddr - pstInst->pu8KnlVirAddr + pstInst->u32PhyAddr;
    }

    BUFMNG_FIND_BLOCK_BY_ADDR(u32PhyAddr, pstInst, pstBlock);
    if (MT_NULL == pstBlock)
    {
        BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
        return MT_ERR_BM_READ_FREE_ERR;
    }

    /* Unfreeze */
    if ((0 != pstInst->u32Freeze) &&
        (pstBlock->u32Addr+pstBlock->u32Size == pstInst->u32PhyAddr+pstInst->u32Size-pstInst->u32Freeze))
    {
        pstInst->u32Free  += pstInst->u32Freeze;
        pstInst->u32Freeze = 0;
    }

#if (BUFMNG_MULTI_READ_SUPPORT == 0)
    /* Only support free orderly */
    if (IS_HEAD_OF_LIST(pstBlock, pstInst) && IS_READING(pstBlock))
    {
        /* Change parameter of instance */
        pstInst->u32Used -= pstBlock->u32Size;
        pstInst->u32Free += pstBlock->u32Size;

#if (BUFMNG_MARKER_SUPPORT == 1)
        /* Sub frame number */
        if (0 == (pstBlock->u32Marker & BUFMNG_NOT_END_FRAME_BIT))
        {
            if (pstInst->u32DataNum > 0)
            {
                pstInst->u32DataNum--;
            }
        }
#endif

        /* Delete block from list */
        list_del(&pstBlock->stBlockNode);

        /* Free block resource */
        MT_KFREE_BUFMNG(pstBlock);
    }
    else
    {
        BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
        return MT_ERR_BM_READ_FREE_ERR;
    }
#else
    /* Support free out-of-order */
    if (IS_READING(pstBlock))
    {
        /* Change parameter of instance */
        pstInst->u32Used -= pstBlock->u32Size;
        pstInst->u32Free += pstBlock->u32Size;

#if (BUFMNG_MARKER_SUPPORT == 1)
        /* Sub frame number */
        if (0 == (pstBlock->u32Marker & BUFMNG_NOT_END_FRAME_BIT))
        {
            if (pstInst->u32DataNum > 0)
            {
                pstInst->u32DataNum--;
            }
        }
#endif

        /* Delete block from list */
        list_del(&pstBlock->stBlockNode);

        /* Free block resource */
        MT_KFREE_BUFMNG(pstBlock);
    }
    else
    {
        BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
        return MT_ERR_BM_READ_FREE_ERR;
    }
#endif

#if (BUFMNG_DEBUG == 1)
    if (pstInst->u32Size != pstInst->u32Used + pstInst->u32Free + pstInst->u32Freeze)
    {
        MT_FATAL_BUFMNG("!!!!!!!!!!!!!!!!UNmatch when read!!!!!!!!!!!!!!!!!!!!\n");
        MT_FATAL_BUFMNG("pstInst->u32Size = %d\n", pstInst->u32Size);
        MT_FATAL_BUFMNG("pstInst->u32Used = %d\n", pstInst->u32Used);
        MT_FATAL_BUFMNG("pstInst->u32Free = %d\n", pstInst->u32Free);
        MT_FATAL_BUFMNG("pstInst->u32Freeze = %d\n", pstInst->u32Freeze);
    }
#endif

    /* Release buffer OK times increase */
    pstInst->u32RlsOK++;
    BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
    return MT_SUCCESS;
}

mt_s32 BUFMNG_Reset(mt_handle hBuf)
{
    BUFMNG_INST_S* pstInst = MT_NULL;
    mt_size_t ulFlags;
	mt_u32 u32KnlDescReadOff =  0;
	mt_handle pBuf =  MT_NULL;
	
    BUFMNG_FIND_INST(hBuf, pstInst);
	
    if (MT_NULL == pstInst)
    {
        return MT_ERR_BM_INVALID_PARA;
    }
	
	pBuf = pstInst->hBuf; 	  
	u32KnlDescReadOff = vdec_descriptor_get_read_ptr(pBuf);
	vdec_descriptor_set_write_ptr(pBuf, u32KnlDescReadOff);

    BUFMNG_SPIN_LOCK(pstInst->stSpinLock);

    /* Free all block */
    BUFMNG_FREE_BLOCK_LIST(pstInst);

    /* Set used/free/freeze size */
    pstInst->u32Used    = 0;
    pstInst->u32Free    = pstInst->u32Size;
    pstInst->u32Freeze  = 0;
    pstInst->u32DataNum = 0;
    pstInst->u32GetTry  = 0;
    pstInst->u32GetOK   = 0;
    pstInst->u32PutTry  = 0;
    pstInst->u32PutOK   = 0;
    pstInst->u32RecvTry = 0;
    pstInst->u32RecvOK  = 0;
    pstInst->u32RlsTry  = 0;
    pstInst->u32RlsOK   = 0;
    //FIXME:
    //Flush: no need reset u32KnlWriterOff
    //Resolution Change/Stop/Start/Reset: need reset u32KnlWriterOff???
    //pstInst->u32KnlWriterOff = 0;
    pstInst->u32GetFail = 0;

    BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
    return MT_SUCCESS;
}

mt_s32 BUFMNG_GetStatus(mt_handle hBuf, BUFMNG_STATUS_S* pstStatus)
{
    BUFMNG_INST_S* pstInst = MT_NULL;
    mt_size_t ulFlags;
#if 0
    if (MT_NULL == pstStatus)
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    BUFMNG_FIND_INST(hBuf, pstInst);
    if (MT_NULL == pstInst)
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    BUFMNG_SPIN_LOCK(pstInst->stSpinLock);
    pstStatus->u32Used    = pstInst->u32Used + pstInst->u32Freeze;
    pstStatus->u32Free    = pstInst->u32Free;
    pstStatus->u32DataNum = pstInst->u32DataNum;
    pstStatus->u32GetTry  = pstInst->u32GetTry;
    pstStatus->u32GetOK   = pstInst->u32GetOK;
    pstStatus->u32PutTry  = pstInst->u32PutTry;
    pstStatus->u32PutOK   = pstInst->u32PutOK;
    pstStatus->u32RecvTry = pstInst->u32RecvTry;
    pstStatus->u32RecvOK  = pstInst->u32RecvOK;
    pstStatus->u32RlsTry  = pstInst->u32RlsTry;
    pstStatus->u32RlsOK   = pstInst->u32RlsOK;
    BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
#else
    VES_BUF_STATE_S vdec_es_buffer_status;

	mt_s32 ChanID;
	mt_s32 Ret;

    if (MT_NULL == pstStatus)
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    BUFMNG_FIND_INST(hBuf, pstInst);
    if (MT_NULL == pstInst)
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    BUFMNG_SPIN_LOCK(pstInst->stSpinLock);

	Ret = VDEC_FindChanIDByESBufferHandle(hBuf, &ChanID);
	if (Ret != MT_SUCCESS || ChanID == MT_INVALID_HANDLE)
	{
        BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
		MLOGE("%s: find Chan ID failed!\n",__FUNCTION__);
        return MT_ERR_BM_INVALID_PARA;
	}

    KERN_VDEC_Control(ChanID, VDEC_CID_GET_CHAN_VES_BUF_STATE, &vdec_es_buffer_status);
    pstStatus->u32Used    = (mt_u32)vdec_es_buffer_status.buf_used;
    pstStatus->u32Free    = (mt_u32)vdec_es_buffer_status.buf_left;
    pstStatus->u32RdPtr   = (mt_u32)vdec_es_buffer_status.read_pointer;
    pstStatus->u32WrPtr   = (mt_u32)vdec_es_buffer_status.write_pointer;
    BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
#endif

    return MT_SUCCESS;
}

#if (BUFMNG_DEBUG == 1)
mt_s32 BUFMNG_Debug(mt_handle hBuf)
{
    BUFMNG_INST_S* pstInst   = MT_NULL;
    BUFMNG_BLOCK_S* pstBlock = MT_NULL;
    struct list_head* pos;
    struct list_head* n;
    mt_s32 i = 0;
    mt_size_t ulFlags;

    BUFMNG_FIND_INST(hBuf, pstInst);
    if (MT_NULL == pstInst)
    {
        return MT_ERR_BM_INVALID_PARA;
    }

    BUFMNG_SPIN_LOCK(pstInst->stSpinLock);

    MT_INFO_BUFMNG("----------------------------------------\n");
    MT_INFO_BUFMNG("SIZE  :%d\n", pstInst->u32Size);
    MT_INFO_BUFMNG("P-ADDR:%d\n", pstInst->u32PhyAddr);
    MT_INFO_BUFMNG("V-ADDR:%p\n", pstInst->pu8VirAddr);
    MT_INFO_BUFMNG("USED  :%d\n", pstInst->u32Used);
    MT_INFO_BUFMNG("FREE  :%d\n", pstInst->u32Free);
    MT_INFO_BUFMNG("FREEZE:%d\n", pstInst->u32Freeze);
    MT_INFO_BUFMNG("FRAME :%d\n", pstInst->u32DataNum);

    if (list_empty(&pstInst->stBlockHead))
    {
        MT_INFO_BUFMNG("\tNONE NODE\n");
    }
    else
    {
        list_for_each_safe(pos, n, &pstInst->stBlockHead)
        {
            pstBlock = list_entry(pos, BUFMNG_BLOCK_S, stBlockNode);
            MT_INFO_BUFMNG("\tNODE %d: V-ADDR=%p SIZE=%d STATUS=%d\n", i++, pstBlock->pu8VirAddr, pstBlock->u32Size,
                       pstBlock->enstatus);
        }
    }

    MT_INFO_BUFMNG("----------------------------------------\n");

    BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);

    return MT_SUCCESS;
}

#endif


mt_void BUFMNG_SaveInit(mt_void)
{
    MT_INIT_MUTEX(&stRawSem);
    MT_INIT_MUTEX(&stYuvSem);

	return;
}


MT_BOOL BUFMNG_CheckFile(mt_s32 Handle, mt_s8 Flag)
{
    mt_s32 ret = MT_FALSE;
    struct semaphore *p_stSem;

    switch(Flag)
    {
        case 0:
            p_stSem = &stRawSem;
        break;

        case 1:
            p_stSem = &stYuvSem;
        break;

        default:
            MT_ERR_BUFMNG("%s unkown flag(%d).\n", __func__, Flag);
        return -1;
    }


    BUFMNG_LOCK(*p_stSem);

    switch(Flag)
    {
        case 0:
            if (MT_NULL != VdecSaveRawFile && Handle == VdecRawChanNum)
            {
                ret = MT_TRUE;
            }
        break;

        case 1:
            if (MT_NULL != VdecSaveYuvFile && Handle == VdecYuvChanNum)
            {
                ret = MT_TRUE;
            }
        break;
    }

    BUFMNG_UNLOCK(*p_stSem);


    return ret;

}


mt_s32 BUFMNG_OpenFile(mt_s32 Handle, mt_s8 *FilePath, mt_s8 Flag)
{
    mt_s32 ret = MT_FAILURE;
    struct semaphore *p_stSem;

    switch(Flag)
    {
        case 0:
            p_stSem = &stRawSem;
        break;

        case 1:
            p_stSem = &stYuvSem;
        break;

        default:
            MT_ERR_BUFMNG("%s unkown flag(%d).\n", __func__, Flag);
        return -1;
    }


    BUFMNG_LOCK(*p_stSem);

    switch(Flag)
    {
        case 0:
            VdecSaveRawFile = filp_open(FilePath, O_RDWR|O_CREAT|O_TRUNC, 0);
            if (IS_ERR(VdecSaveRawFile))
            {
                VdecSaveRawFile = MT_NULL;
            }
            else
            {
                VdecRawChanNum = Handle;
                ret = MT_SUCCESS;
            }
            break;

        case 1:
            VdecSaveYuvFile = filp_open(FilePath, O_RDWR|O_CREAT|O_TRUNC, 0);
            if (IS_ERR(VdecSaveYuvFile))
            {
                VdecSaveYuvFile = MT_NULL;
            }
            else
            {
                VdecYuvChanNum = Handle;
                ret = MT_SUCCESS;
            }
            break;
    }

    BUFMNG_UNLOCK(*p_stSem);

    if (MT_SUCCESS == ret)
    {
        MT_ERR_BUFMNG("Open file %s of inst%d success.\n",FilePath, Handle);
    }
    else
    {
        MT_ERR_BUFMNG("Open file %s of inst%d failed.\n",FilePath, Handle);
    }
    return ret;

}


mt_s32 BUFMNG_CloseFile(mt_s32 Handle, mt_s8 Flag)
{
    mt_s32 ret = MT_FAILURE;
    struct semaphore *p_stSem;

    if (2 == Flag) // Close ALL
    {
        BUFMNG_LOCK(stRawSem);
        if (MT_NULL != VdecSaveRawFile)
        {
            filp_close(VdecSaveRawFile, MT_NULL);
            MT_ERR_BUFMNG("Close raw file of inst%d.\n", VdecRawChanNum);
            VdecSaveRawFile = MT_NULL;
            VdecRawChanNum = -1;
        }
        BUFMNG_UNLOCK(stRawSem);

        BUFMNG_LOCK(stYuvSem);
        if (MT_NULL != VdecSaveYuvFile)
        {
            filp_close(VdecSaveYuvFile, MT_NULL);
            MT_ERR_BUFMNG("Close yuv file of inst%d.\n", VdecYuvChanNum);
            VdecSaveYuvFile = MT_NULL;
            MT_VFREE_BUFMNG(YUV_Array);
            VdecYuvChanNum = -1;
            YUV_Array = U_Array = V_Array = MT_NULL;
        }
        BUFMNG_UNLOCK(stYuvSem);

        return MT_SUCCESS;
    }

    switch(Flag) // Close Raw/Yuv
    {
        case 0:
            p_stSem = &stRawSem;
        break;

        case 1:
            p_stSem = &stYuvSem;
        break;

        default:
        MT_ERR_BUFMNG("%s unkown flag(%d).\n", __func__, Flag);
        return -1;
    }


    BUFMNG_LOCK(*p_stSem);

    switch(Flag)
    {
        case 0:
        if (Handle == VdecRawChanNum)
        {
            filp_close(VdecSaveRawFile, MT_NULL);
            MT_ERR_BUFMNG("Close raw file of inst%d.\n", VdecRawChanNum);
            VdecSaveRawFile = MT_NULL;
            VdecRawChanNum = -1;
            ret = MT_SUCCESS;
        }
        break;

        case 1:
        if (Handle == VdecYuvChanNum)
        {
            filp_close(VdecSaveYuvFile, MT_NULL);
            MT_ERR_BUFMNG("Close yuv file of inst%d.\n", VdecYuvChanNum);
            MT_VFREE_BUFMNG(YUV_Array);
            VdecSaveYuvFile = MT_NULL;
            VdecYuvChanNum = -1;
            YUV_Array = U_Array = V_Array = MT_NULL;
            ret = MT_SUCCESS;
        }
        break;
    }

    BUFMNG_UNLOCK(*p_stSem);

    return ret;

}


mt_s32 BUFMNG_SaveRaw(mt_s32 Handle, mt_s8 *Addr, mt_s32 Length)
{
#if 0
    mt_s32 len = 0;
    mm_segment_t oldfs;

    BUFMNG_LOCK(stRawSem);

    if (MT_NULL == VdecSaveRawFile || Handle != VdecRawChanNum)
    {
        BUFMNG_UNLOCK(stRawSem);
        return -1;
    }

    oldfs = get_fs();
    set_fs(0);
	//Bug: kernel panic! why?
    //len = VdecSaveRawFile->f_op->write(VdecSaveRawFile, Addr, Length, &VdecSaveRawFile->f_pos);
    len = vfs_write(VdecSaveRawFile, Addr, Length, &VdecSaveRawFile->f_pos);
    if(len != Length)
    {
        MT_ERR_BUFMNG("Save Raw Error return :%d,length :%d",len,Length);
        {
            filp_close(VdecSaveRawFile, MT_NULL);
            MT_ERR_BUFMNG("Close RAW file of inst%d.\n", VdecRawChanNum);
            VdecSaveRawFile = MT_NULL;
            VdecRawChanNum = -1;
            BUFMNG_UNLOCK(stRawSem);
            return -1;
        }
    }
    set_fs(oldfs);

    BUFMNG_UNLOCK(stRawSem);
    return len;
#endif
	return 0;

}

extern int tile_convert(void *in_addr, int width, int height, void *out_addr);
extern int tile_convert_U(void *in_addr, int width, int height, void *out_addr);
extern int tile_convert_V(void *in_addr, int width, int height, void *out_addr);

mt_s32 BUFMNG_SaveYuv(mt_s32 Handle, MT_DRV_VIDEO_FRAME_S *pstFrame,MT_UNF_VCODEC_TYPE_E enType)
{
#if 0
    mt_u32 i, j;
    mt_s32 len = 0;
    //mt_s32 chroma_width = 0;
    //mt_s32 chroma_height = 0;
    //mt_s32 chroma_len = 0;
    mt_u8 *dst, *src, *tmp;
    mt_u32 Stride =0;
    MT_BOOL bVcmpFlag = MT_TRUE;
    mt_s8 *Y_Addr = MT_NULL;
    mt_s8 *C_Addr = MT_NULL;
    mm_segment_t oldfs;
    char pgm_head[256];
    if(bSaveoneyuv && u32SaveCnt >0)
    {
        return MT_SUCCESS;
    }
    else if(bSaveoneyuv)
    {
        u32SaveCnt ++;
    }

    BUFMNG_LOCK(stYuvSem);

    if (MT_NULL == VdecSaveYuvFile || Handle != VdecYuvChanNum || MT_NULL == pstFrame)
    {
        BUFMNG_UNLOCK(stYuvSem);
        return -1;
    }

#ifdef CONFIG_MIPS
	//non-cache addr
    Y_Addr = (mt_s8 *)(pstFrame->stBufAddr[0].u32PhyAddr_Y | 0xA0000000);
    C_Addr = (mt_s8 *)(pstFrame->stBufAddr[0].u32PhyAddr_C | 0xA0000000);
#elif defined(CONFIG_ARM)
    Y_Addr = (mt_s8 *)phys_to_virt(pstFrame->stBufAddr[0].u32PhyAddr_Y);
    C_Addr = (mt_s8 *)phys_to_virt(pstFrame->stBufAddr[0].u32PhyAddr_C);
#else
	BUG();
#endif

    MLOGD("%s: addrLuma 0x%x, addrChroma 0x%x\n",__FUNCTION__,
    		pstFrame->slotInfo.filedInfoTop.addrLuma,
    		pstFrame->slotInfo.filedInfoTop.addrChroma);
    MLOGD("%s: Y_Addr %p, C_Addr %p\n",__FUNCTION__,Y_Addr,C_Addr);
    MLOGD("%s: u32Width %u, u32Height %u\n",__FUNCTION__,pstFrame->u32Width,pstFrame->u32Height);
    MLOGD("%s: StrideY %u, StrideC %u\n",__FUNCTION__,
    		pstFrame->stBufAddr[0].u32Stride_Y,
    		pstFrame->stBufAddr[0].u32Stride_C);
    MLOGD("%s: PixFormat %d\n",__FUNCTION__,pstFrame->ePixFormat);

    oldfs = get_fs();
    set_fs(0);

    if (MT_DRV_PIX_FMT_NV21 == pstFrame->ePixFormat)
    {
#if 1
        if (MT_NULL == YUV_Array)
        {
            YUV_Array = MT_VMALLOC_BUFMNG(pstFrame->u32Width * pstFrame->u32Height);
            if (MT_NULL == YUV_Array)
            {
                filp_close(VdecSaveYuvFile, MT_NULL);
                VdecSaveYuvFile = MT_NULL;
                VdecYuvChanNum = -1;
                BUFMNG_UNLOCK(stYuvSem);
                return -1;
            }
        }

		//pgm header
		if(bSaveoneyuv)
		{
		    snprintf(pgm_head, 255, "P5\n%d %d\n%d\n", pstFrame->u32Width,pstFrame->u32Height,255);
		    vfs_write(VdecSaveYuvFile, pgm_head, strlen(pgm_head), &VdecSaveYuvFile->f_pos);
	    }

		//Y
		tile_convert((void*)Y_Addr, pstFrame->u32Width, pstFrame->u32Height, (void*)YUV_Array);

		len = vfs_write(VdecSaveYuvFile, YUV_Array, pstFrame->u32Width * pstFrame->u32Height, &VdecSaveYuvFile->f_pos);
		MLOGD("%s: vfs_write %d\n",__FUNCTION__,len);
		if (len != (mt_s32)(pstFrame->u32Width * pstFrame->u32Height))
		{
			MT_ERR_BUFMNG("%s write y failed.\n", __func__);
		}

		//U
		tile_convert_U((void*)C_Addr, pstFrame->u32Width/2, pstFrame->u32Height/2, (void*)YUV_Array);

		len = vfs_write(VdecSaveYuvFile, YUV_Array, pstFrame->u32Width * pstFrame->u32Height / 4, &VdecSaveYuvFile->f_pos);
		MLOGD("%s: vfs_write %d\n",__FUNCTION__,len);
		if (len != (mt_s32)(pstFrame->u32Width * pstFrame->u32Height / 4))
		{
			MT_ERR_BUFMNG("%s write u failed.\n", __func__);
		}

		//V
		tile_convert_V((void*)C_Addr, pstFrame->u32Width/2, pstFrame->u32Height/2, (void*)YUV_Array);

		len = vfs_write(VdecSaveYuvFile, YUV_Array, pstFrame->u32Width * pstFrame->u32Height / 4, &VdecSaveYuvFile->f_pos);
		MLOGD("%s: vfs_write %d\n",__FUNCTION__,len);
		if (len != (mt_s32)(pstFrame->u32Width * pstFrame->u32Height / 4))
		{
			MT_ERR_BUFMNG("%s write v failed.\n", __func__);
		}

#else
        if (MT_NULL == YUV_Array)
        {
            YUV_Array = MT_VMALLOC_BUFMNG(pstFrame->u32Width * pstFrame->u32Height / 2);
            if (MT_NULL == YUV_Array)
            {
                filp_close(VdecSaveYuvFile, MT_NULL);
                VdecSaveYuvFile = MT_NULL;
                VdecYuvChanNum = -1;
                BUFMNG_UNLOCK(stYuvSem);
                return -1;
            }
            U_Array = YUV_Array;
            V_Array = U_Array + pstFrame->u32Width * pstFrame->u32Height / 4;
        }

        Stride = pstFrame->stBufAddr[0].u32Stride_Y;

    	/* Y */
        for (i=0; i<pstFrame->u32Height; i++)
        {
            len = VdecSaveYuvFile->f_op->write(VdecSaveYuvFile, Y_Addr, pstFrame->u32Width, &VdecSaveYuvFile->f_pos);
            if (len < pstFrame->u32Width)
            {
                MT_ERR_BUFMNG("%s write y failed.\n", __func__);
            }
            Y_Addr += pstFrame->stBufAddr[0].u32Stride_Y;
        }

        /* UV */
        chroma_width = pstFrame->u32Width/2;
        chroma_height = pstFrame->u32Height/2;

        for (i=0; i<chroma_height; i++)
        {
            for (j=0; j<chroma_width; j++)
            {
                V_Array[i*chroma_width+j] = C_Addr[2*j];
                U_Array[i*chroma_width+j] = C_Addr[2*j+1];
            }
            C_Addr += pstFrame->stBufAddr[0].u32Stride_C;
        }

        chroma_len = chroma_width*chroma_height;
        len = VdecSaveYuvFile->f_op->write(VdecSaveYuvFile, U_Array, chroma_len, &VdecSaveYuvFile->f_pos);
        if(len != chroma_len)
        {
            MT_ERR_BUFMNG("%s write u failed.\n", __func__);
        }

        len = VdecSaveYuvFile->f_op->write(VdecSaveYuvFile, V_Array, chroma_len, &VdecSaveYuvFile->f_pos);
        if(len != chroma_len)
        {
            MT_ERR_BUFMNG("%s write v failed.\n", __func__);
        }
#endif
        set_fs(oldfs);
    }
    else	/* not support */
    {
        MT_DRV_VDEC_GetVcmpFlag(&bVcmpFlag);
        if(!bVcmpFlag)
        {
            if(MT_NULL == YUV_Array)
            {
                YUV_Array = MT_VMALLOC_BUFMNG(pstFrame->u32Width * pstFrame->u32Height * 3/2);
                if (MT_NULL == YUV_Array)
                {
                    filp_close(VdecSaveYuvFile, MT_NULL);
                    VdecSaveYuvFile = MT_NULL;
                    VdecYuvChanNum = -1;
                    BUFMNG_UNLOCK(stYuvSem);
                    return -1;
                }
                U_Array = YUV_Array + pstFrame->u32Width * pstFrame->u32Height;
                V_Array = U_Array + pstFrame->u32Width * pstFrame->u32Height/4;
            }

            Stride = pstFrame->stBufAddr[0].u32Stride_Y * 16;

            /*Y*/
            for(i=0;i<pstFrame->u32Height;i++)
            {
                for(j=0;j<pstFrame->u32Width;j+=256)
                {
                    dst  = (unsigned char*)(YUV_Array+ pstFrame->u32Width*i + j);
                    src =  Y_Addr + Stride*(i/16)+(i%16)*256 + (j/256)*256*16;
                    memcpy(dst,src,256);
                }
            }
            len = VdecSaveYuvFile->f_op->write(VdecSaveYuvFile, YUV_Array, pstFrame->u32Width*pstFrame->u32Height, &VdecSaveYuvFile->f_pos);
            if(len != pstFrame->u32Width*pstFrame->u32Height)
            {
                MT_ERR_BUFMNG("%s write Y failed.len : %d,W*H :%d\n", __func__,len,pstFrame->u32Width*pstFrame->u32Height);
                {
                    filp_close(VdecSaveYuvFile, MT_NULL);
                    MT_ERR_BUFMNG("Close yuv file of inst%d.\n", VdecYuvChanNum);
                    VdecSaveYuvFile = MT_NULL;
                    MT_VFREE_BUFMNG(YUV_Array);
                    VdecYuvChanNum = -1;
                    YUV_Array = U_Array = V_Array = MT_NULL;
                    BUFMNG_UNLOCK(stYuvSem);
                     return -1;
                }
            }

            /*UV*/
            for(i=0;i<pstFrame->u32Height/2;i++)
            {
                for(j=0;j<pstFrame->u32Width;j+=256)
                {
                   dst  = (unsigned char*)(YUV_Array + pstFrame->u32Width*i + j);
                   src =  (unsigned char*)C_Addr + (Stride/2)*(i/8)+(i%8)*256 + (j/256)*256*8;
                    memcpy(dst,src,256);
                }
            }
            tmp = YUV_Array;
            {
                for (i=0;i<pstFrame->u32Height/2;i++)
                {
                    for (j=0;j<pstFrame->u32Width/2;j++)
                    {
                        V_Array[i*pstFrame->u32Width/2+j] = tmp[2*j];
                        U_Array[i*pstFrame->u32Width/2+j] = tmp[2*j+1];
                    }
                    tmp+= pstFrame->u32Width;
                }
            }
            len = VdecSaveYuvFile->f_op->write(VdecSaveYuvFile, U_Array, pstFrame->u32Width*pstFrame->u32Height/4, &VdecSaveYuvFile->f_pos);
            if(len != pstFrame->u32Width*pstFrame->u32Height/4)
            {
                MT_ERR_BUFMNG("%s write U failed.len : %d,W*H :%d\n", __func__,len,pstFrame->u32Width*pstFrame->u32Height/4);
                {
                    filp_close(VdecSaveYuvFile, MT_NULL);
                    MT_ERR_BUFMNG("Close yuv file of inst%d.\n", VdecYuvChanNum);
                    VdecSaveYuvFile = MT_NULL;
                    MT_VFREE_BUFMNG(YUV_Array);
                    VdecYuvChanNum = -1;
                    YUV_Array = U_Array = V_Array = MT_NULL;
                    BUFMNG_UNLOCK(stYuvSem);
                    return -1;
                }
            }

            len = VdecSaveYuvFile->f_op->write(VdecSaveYuvFile, V_Array, pstFrame->u32Width*pstFrame->u32Height/4, &VdecSaveYuvFile->f_pos);
            if(len != pstFrame->u32Width*pstFrame->u32Height/4)
            {
                MT_ERR_BUFMNG("%s write V failed.len : %d,W*H :%d\n", __func__,len,pstFrame->u32Width*pstFrame->u32Height/4);
                {
                    filp_close(VdecSaveYuvFile, MT_NULL);
                    MT_ERR_BUFMNG("Close yuv file of inst%d.\n", VdecYuvChanNum);
                    VdecSaveYuvFile = MT_NULL;
                    MT_VFREE_BUFMNG(YUV_Array);
                    VdecYuvChanNum = -1;
                    YUV_Array = U_Array = V_Array = MT_NULL;
                    BUFMNG_UNLOCK(stYuvSem);
                    return -1;
                }
            }

            MT_ERR_BUFMNG("Saving YUV(%dx%d)...\n", pstFrame->u32Width, pstFrame->u32Height);
        }
        else
        {
            //save 1D compressed data
            //TO DO
        }
        set_fs(oldfs);

    }

    BUFMNG_UNLOCK(stYuvSem);

    if(bSaveoneyuv)
    {
        if(BUFMNG_CloseFile(Handle, 1) != MT_SUCCESS)
        {
             MT_FATAL_VDEC("FATAL: failed close file for vdec%2d yuv save!\n", Handle);
        }
    }
    return len;
#endif
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

/** Append SW PTS Descriptor buffer to this ES buffer instance */
mt_s32 BUFMNG_AppendDescriptor(mt_handle hBuf, mt_char *name, ulong size)
{
    mt_s32 s32Ret;
    BUFMNG_INST_S* pstInst = MT_NULL;
    mmz_buffer_s stMMZBuf;
    unsigned long ulFlags;

    BUFMNG_FIND_INST(hBuf, pstInst);
    if (MT_NULL == pstInst)
    {
        return MT_ERR_BM_INVALID_PARA;
    }

	s32Ret = mt_drv_mmz_alloc_and_map(name, MMZ_OTHERS, size, 0, &stMMZBuf);
	if (MT_SUCCESS != s32Ret)
	{
		MT_FATAL_BUFMNG("Alloc MMZ fail:0x%x.\n", s32Ret);
		return MT_ERR_BM_NO_MEMORY;
	}
	else
	{
		memset((void*)stMMZBuf.startVirAddr, 0, stMMZBuf.size);
	}

    BUFMNG_SPIN_LOCK(pstInst->stSpinLock);

	pstInst->u32DescPhyAddr = stMMZBuf.startPhyAddr;
	pstInst->pu8KnlVirDescAddr = (mt_u8*)stMMZBuf.startVirAddr;
	pstInst->u32KnlVirDescBufSize = stMMZBuf.size;

	MLOGI("%s: DES Buff PhyAddr=%lx, KnlVirAddr=%lx, Size=%x \n", __FUNCTION__,
		(ulong)pstInst->u32DescPhyAddr, (ulong)pstInst->pu8KnlVirDescAddr, pstInst->u32KnlVirDescBufSize);

    vdec_descriptor_init(hBuf,
    					pstInst->u32PhyAddr, pstInst->pu8KnlVirAddr, pstInst->u32Size,
    					pstInst->u32DescPhyAddr, pstInst->pu8KnlVirDescAddr,
    					pstInst->u32KnlVirDescBufSize);

    BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);

	return MT_SUCCESS;
}

mt_s32 BUFMNG_GetDescWriteBuffer(mt_handle hBuf, ulong *pu32Addr)
{
	BUFMNG_INST_S* pstInst = MT_NULL;
    unsigned long ulFlags;
    mt_u32 u32KnlDescWriterOff;
    mt_u32 u32KnlDescReadOff;

	BUFMNG_FIND_INST(hBuf, pstInst);
	if (MT_NULL == pstInst)
	{
		return MT_ERR_BM_INVALID_PARA;
	}

	BUFMNG_SPIN_LOCK(pstInst->stSpinLock);

	u32KnlDescWriterOff = vdec_descriptor_get_write_ptr(hBuf);
	u32KnlDescReadOff = vdec_descriptor_get_read_ptr(hBuf);

	/* Desc Buffer full */
	if (pstInst->u32KnlVirDescBufSize != 0
		&& ((u32KnlDescWriterOff + 32) % pstInst->u32KnlVirDescBufSize)
			== u32KnlDescReadOff)
	{
		BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);
		MLOGW("%s: SW Descriptor buffer full, WR ptr(0x%x) RD ptr(0x%x)!\n", __FUNCTION__,
				u32KnlDescWriterOff, u32KnlDescReadOff);
		pstInst->u32GetFail ++;
		return MT_ERR_BM_BUFFER_FULL;
	}

	if (pu32Addr != NULL)
	{
		*pu32Addr = (ulong)pstInst->pu8KnlVirDescAddr + u32KnlDescWriterOff;
	}

	BUFMNG_SPIN_UNLOCK(pstInst->stSpinLock);

    return MT_SUCCESS;
}

static mt_s32 _BUFMNG_PutDescriptor(BUFMNG_INST_S* pstInst, BUFMNG_BUF_S *pstBuf,
									mt_u32 offset_begin, mt_u32 offset_end)
{
    mt_u32 u32DescInfo[8];

    mt_u8* pu8KnlVirDescAddr;
    mt_u32 u32KnlVirDescBufSize;
    mt_u32 u32KnlDescWriterOff;
    mt_u32 u32KnlDescReadOff;
    mt_u32 u32PTS31Bit;
    mt_u32 u32PTS1Bit;
    mt_u32 u32PtsValide;

    mt_u32 u32PTSHi;
    mt_u32 u32PTSLow;

    mt_handle hBuf = pstInst->hBuf;

	if (((1 == pstBuf->u32FrameFinsh)&&(1 == pstBuf->u32PreFrameFinsh))
		|| ((0 == pstBuf->u32FrameFinsh)&&(1 == pstBuf->u32PreFrameFinsh)))
	{
		u32PTSHi = (mt_u32)((pstBuf->u64Pts>>32) & 0x00000000ffffffff);
		u32PTSLow = (mt_u32)(pstBuf->u64Pts & 0x00000000ffffffff);

		pu8KnlVirDescAddr = pstInst->pu8KnlVirDescAddr;
		u32KnlVirDescBufSize = pstInst->u32KnlVirDescBufSize;
		//SW ES Descriptor 以 ves_buffer_inst 为准
		//它处(VFMW) will Reset Descriptor
		//u32KnlDescWriterOff = ves_buffer_inst[hBuf].u32KnlDescWriterOff;
		u32KnlDescWriterOff = vdec_descriptor_get_write_ptr(hBuf);

		pu8KnlVirDescAddr += u32KnlDescWriterOff;

		//u32PTS30Bit = (mt_u32)pstBuf->u64Pts;
		u32PTS31Bit = u32PTSLow;
		u32PTS1Bit = u32PTS31Bit & 0x80000000;
		u32PTS1Bit >>= 6;	//bit 25 = pts low 32bits' 31bit
		u32PTS31Bit = u32PTS31Bit & 0x7fffffff;

		u32PtsValide = pstBuf->u32PtsValide;

		//printk("u32PTS30Bit = %x, u64Pts = %llx, u32PTSHi = %x, u32PTSLow = %x \n", u32PTS31Bit, pstBuf->u64Pts, u32PTSHi, u32PTSLow);

		//if (u32PtsValide)
		{
		  //bit25: pts low 32 bits: bit 31
		  u32DescInfo[0] = u32PTS1Bit | 1 ;
		  //u32DescInfo[1] = pstInst->u32PhyAddr + vdec_es_buffer_status.write_pointer + u32WriteOffset + pstBuf->u32ScrapSize;
		  u32DescInfo[1] = pstInst->u32PhyAddr + offset_end;
		  u32DescInfo[2] = 0;
		  //bit31: valid flag
		  //bit 0-30: pts low 31bits
		  u32DescInfo[3] = u32PTS31Bit | (u32PtsValide << 31);

		  u32DescInfo[4] = 0;
		  u32DescInfo[5] = pstInst->u32PhyAddr + offset_begin;
		  u32DescInfo[6] = u32PTSHi;
		  u32DescInfo[7] = pstBuf->u32EosFlag;

		  //printk("mem_addr = %x, u32Size = %x \n", u32DescInfo[5], pstBuf->u32Size);

		  memcpy(pu8KnlVirDescAddr , u32DescInfo, 32);

		  //TODO
		  //flush the cache?

		  u32KnlDescWriterOff += 32;

		  //Bug: u32KnlVirDescBufSize < (1024*1024)!
		  //if(pstInst->u32KnlDescWriterOff >= (1024*1024))
		  if (u32KnlDescWriterOff >= u32KnlVirDescBufSize)
		  {
			u32KnlDescWriterOff = 0;
		  }

		  //Bug: 007 WR pointer might over write RD pointer if DESC_BUFF_SIZE is not enough
		  u32KnlDescReadOff = vdec_descriptor_get_read_ptr(hBuf);
		  //if (u32KnlDescWriterOff == ves_buffer_inst[hBuf].u32KnDescReadOff)
		  if (u32KnlDescWriterOff == u32KnlDescReadOff)
		  {
			MLOGE("%s: Error, SW Descriptor WR ptr(0x%x) over-write RD ptr(0x%x)!\n", __FUNCTION__,
					u32KnlDescWriterOff, u32KnlDescReadOff);
		  }

		  //ves_buffer_inst[hBuf].u32KnlDescWriterOff = u32KnlDescWriterOff;
		  vdec_descriptor_set_write_ptr(hBuf, u32KnlDescWriterOff);

	/* dump es buffer stat */
#ifdef CONFIG_MT_DEBUG_V_BUF_STAT
			VDEBUG_LOG("[V DESC BUF STAT][%s]: h %u, Addr 0x%x, Size 0x%x, RD 0x%x, WR 0x%x, PTS (%u, %llu), EOS %u\n\n",__FUNCTION__,
						pstInst->hBuf,
						pstInst->u32DescPhyAddr,
						pstInst->u32KnlVirDescBufSize,
						u32KnlDescReadOff,
						u32KnlDescWriterOff,
						pstBuf->u32PtsValide,
						pstBuf->u64Pts,
						pstBuf->u32EosFlag);
#endif
		}
	}

	return MT_SUCCESS;
}

u32 vdec_get_ves_rd(int ChanID)
{
	VES_BUF_STATE_S vdec_es_buffer_status;
	KERN_VDEC_Control(ChanID, VDEC_CID_GET_CHAN_VES_BUF_STATE, &vdec_es_buffer_status);
	return vdec_es_buffer_status.read_pointer;
}

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */
