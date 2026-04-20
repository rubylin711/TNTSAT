#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include "mt_type.h"
#include "mt_debug.h"
#include "mt_module.h"
#include "mt_debug.h"
#include "mt_osal.h"
#include "drv_mmgr.h"
#include "drv_module_ioctl.h"
#include "mpi_mmgr.h"
#include "mt_drv_proc.h"
#include "mt_drv_dev.h"
#include "mt_drv_module.h"
#include "mt_drv_mem.h"
#include "drv_mem.h"

#define COLOR_START      "\33[32m"
#define COLOR_START_HEAD "\33[35m" /* pink text color*/

#define COLOR_START_RED "\33[31m"  /* red text color */
#define COLOR_END       "\33[0m"

#define KERNEL_MODE "kernel"

#define ABS_SIZE(s) ( (s) > 0 ? (s) : (-1)*(s) )

MT_DECLARE_MUTEX(g_KModuleMemMutex);

#define kmodule_mem_lock(RET)                                   \
do{                                                             \
    mt_s32 s32LockRet = down_interruptible(&g_KModuleMemMutex); \
    if ( s32LockRet != 0 )                                      \
    {                                                           \
        return RET;                                             \
    }                                                           \
}while(0)

#define kmodule_mem_unlock() up(&g_KModuleMemMutex)


typedef struct tagMemoryInfo
{
    mt_u32   u32PID;
    mt_u32   u32ModuleID; /* this may be same with another item */

    // the size in kernel
    mt_u32   u32KKSize;
    mt_u32   u32KVSize;

    // the size in user mode;
    mt_u32   u32UsrSize;
    mt_u32   u32MMZSize;

    // the max size used by this module.
    mt_u32   u32MaxSize;
}kmodule_mem_info_s;

typedef struct tagKModuleMemory
{
    kmodule_mem_info_s stMemInfo;
    mt_u32             u32ItemCnt;

    struct tagKModuleMemory* pNext;
}kmodule_mem_item_s;

typedef struct tagKModuleMemoryPOOL
{
    mt_u32 u32Idle;

    kmodule_mem_item_s stModuleMemItem;
}kmodule_mem_pool_s;

static kmodule_mem_pool_s* g_KModuleMemBaseAddr = NULL;
static mt_u32       g_u32ModuleMemItemCount = 0;

static kmodule_mem_item_s*  g_pstMemItemHeader = NULL;
static mt_u32  g_ModuleMemModInit = 0;



mt_void kmodule_mem_pool_init(mt_u32 u32Count)
{
    if (NULL == g_KModuleMemBaseAddr)
    {
        g_KModuleMemBaseAddr = (kmodule_mem_pool_s*)kmalloc(u32Count * sizeof(kmodule_mem_pool_s), GFP_KERNEL);

        if (NULL == g_KModuleMemBaseAddr)
        {
            MT_ERR_MEM("<%s> malloc %d size failure!\n", KERNEL_MODE, u32Count * sizeof(kmodule_mem_pool_s));

            return;
        }

        memset(g_KModuleMemBaseAddr, 0, u32Count * sizeof(kmodule_mem_pool_s));

        g_u32ModuleMemItemCount = u32Count;
    }

    if (NULL == g_pstMemItemHeader)
    {
        g_pstMemItemHeader = (kmodule_mem_item_s*)kmalloc(sizeof(kmodule_mem_item_s), GFP_KERNEL);

        if (NULL != g_pstMemItemHeader)
        {
            memset(g_pstMemItemHeader, 0, sizeof(kmodule_mem_item_s));
            g_pstMemItemHeader->pNext = NULL;
        }
    }
}

mt_void kmodule_mem_pool_deinit(mt_void)
{
    if (NULL != g_KModuleMemBaseAddr)
    {
        kfree(g_KModuleMemBaseAddr);

        g_u32ModuleMemItemCount = 0;
    }

    if (NULL != g_pstMemItemHeader)
    {
        kfree(g_pstMemItemHeader);
    }
}

static mt_void* kmodule_mem_pool_malloc(mt_u32 u32Size)
{
    mt_u32 u32Index = 0;

    for (u32Index=0; u32Index<g_u32ModuleMemItemCount; u32Index++)
    {
        if (g_KModuleMemBaseAddr[u32Index].u32Idle == 0)
        {
            g_KModuleMemBaseAddr[u32Index].u32Idle = 1;
            break;
        }
    }

    if (u32Index != g_u32ModuleMemItemCount)
    {
        return &g_KModuleMemBaseAddr[u32Index].stModuleMemItem;
    }

    return NULL;
}

static mt_void kmodule_mem_pool_free(mt_void* pAddr)
{
    mt_u32 u32Index = 0;

    for (u32Index=0; u32Index<g_u32ModuleMemItemCount; u32Index++)
    {
        if (&g_KModuleMemBaseAddr[u32Index].stModuleMemItem == pAddr)
        {
            g_KModuleMemBaseAddr[u32Index].u32Idle = 0;

            memset(&g_KModuleMemBaseAddr[u32Index].stModuleMemItem, 0, sizeof(g_KModuleMemBaseAddr[u32Index].stModuleMemItem));
            break;
        }
    }

    return;
}

mt_s32 kmodule_mem_pool_find_node(kmodule_mem_item_s* pNodeHeader, mt_u32 u32ModuleID, kmodule_mem_item_s** pstNode)
{
    kmodule_mem_item_s* pItrNode = NULL;

    if (NULL == pNodeHeader || pstNode == NULL)
    {
        MT_ERR_MEM("<%s>: add node failure, node header is NULL\n", KERNEL_MODE);

        return MT_FAILURE;
    }

    pItrNode = pNodeHeader->pNext;

    while ( NULL != pItrNode )
    {
        if (pItrNode->stMemInfo.u32ModuleID == u32ModuleID)
        {
            break;
        }

        pItrNode = pItrNode->pNext;
    }

    if (pItrNode != NULL)
    {
        *pstNode = pItrNode;

        return MT_SUCCESS;
    }

    return MT_FAILURE;

}

// Make one node and add it to the link tail
mt_s32 kmodule_mem_pool_add_node(kmodule_mem_item_s* pNodeHeader, kmodule_mem_item_s* pstNode)
{
    kmodule_mem_item_s* pItrNode   = NULL;
    kmodule_mem_item_s* pstAddNode = NULL;
    mt_s32      s32Ret     = MT_FAILURE;
    kmodule_mem_item_s* pMemItem   = NULL;

    mt_u32 u32TotalSize = 0;

    if (NULL == pNodeHeader || pstNode == NULL)
    {
        MT_ERR_MEM("<%s>: add node failure, node header is NULL\n", KERNEL_MODE);

        return MT_FAILURE;
    }

    MT_INFO_MEM("<%s>........ pNodeHeader address is %p, and next address is %p\n",KERNEL_MODE, pNodeHeader, pNodeHeader->pNext);

    s32Ret = kmodule_mem_pool_find_node(pNodeHeader, pstNode->stMemInfo.u32ModuleID, &pMemItem);
    if (MT_SUCCESS == s32Ret)
    {
        pMemItem->stMemInfo.u32KKSize += pstNode->stMemInfo.u32KKSize;
        pMemItem->stMemInfo.u32KVSize += pstNode->stMemInfo.u32KVSize;

        pMemItem->stMemInfo.u32UsrSize += pstNode->stMemInfo.u32UsrSize;
        pMemItem->stMemInfo.u32MMZSize += pstNode->stMemInfo.u32MMZSize;

        u32TotalSize = pMemItem->stMemInfo.u32KKSize + pMemItem->stMemInfo.u32KVSize;
        u32TotalSize += pMemItem->stMemInfo.u32UsrSize + pMemItem->stMemInfo.u32MMZSize;

        if ( u32TotalSize > pMemItem->stMemInfo.u32MaxSize)
        {
            pMemItem->stMemInfo.u32MaxSize = u32TotalSize;
        }

        return MT_SUCCESS;
    }

    pstAddNode = (kmodule_mem_item_s*)kmodule_mem_pool_malloc(sizeof(kmodule_mem_item_s));
    if ( NULL == pstAddNode)
    {
        MT_ERR_MEM("<%s>: add node failure, malloc node failure.\n", KERNEL_MODE);
        return MT_FAILURE;
    }

    memset(pstAddNode, 0, sizeof(kmodule_mem_item_s));

    pstAddNode->stMemInfo.u32ModuleID = pstNode->stMemInfo.u32ModuleID;

    pstAddNode->stMemInfo.u32KKSize = pstNode->stMemInfo.u32KKSize;
    pstAddNode->stMemInfo.u32KVSize = pstNode->stMemInfo.u32KVSize;

    pstAddNode->stMemInfo.u32UsrSize = pstNode->stMemInfo.u32UsrSize;
    pstAddNode->stMemInfo.u32MMZSize = pstNode->stMemInfo.u32MMZSize;
    pstAddNode->stMemInfo.u32PID = pstNode->stMemInfo.u32PID;

    pstAddNode->pNext = NULL;

    u32TotalSize = pstAddNode->stMemInfo.u32KKSize + pstAddNode->stMemInfo.u32KVSize;
    u32TotalSize += pstAddNode->stMemInfo.u32UsrSize + pstAddNode->stMemInfo.u32MMZSize;
    if ( u32TotalSize > pstAddNode->stMemInfo.u32MaxSize)
    {
        pstAddNode->stMemInfo.u32MaxSize = u32TotalSize;
    }

    pItrNode = pNodeHeader;

    while ( NULL != pItrNode && NULL != pItrNode->pNext )
    {
        pItrNode = pItrNode->pNext;
    }

    pItrNode->pNext = pstAddNode;
    pNodeHeader->u32ItemCnt++;

    MT_INFO_MEM("<%s>++++++++++ pNodeHeader address is %p, and next address is %p\n", KERNEL_MODE, pNodeHeader, pNodeHeader->pNext);

    return MT_SUCCESS;
}

mt_s32 kmodule_mem_pool_add_module(mt_u32 u32ModuleID)
{
    kmodule_mem_item_s stNode = {{0}};

    stNode.stMemInfo.u32ModuleID = u32ModuleID;
    stNode.stMemInfo.u32PID = current->pid;

    return kmodule_mem_pool_add_node(g_pstMemItemHeader, &stNode);
}


// Delete the node from link
mt_s32 kmodule_mem_pool_del_node(kmodule_mem_item_s* pNodeHeader, kmodule_mem_item_s* pstNode)
{
    kmodule_mem_item_s* pItrNode = NULL;
    kmodule_mem_item_s* pDelNode = NULL;

    if (NULL == pNodeHeader || pNodeHeader->pNext == NULL)
    {
        MT_ERR_MEM("<%s>:delete node failure, node header is NULL\n", KERNEL_MODE);

        return MT_FAILURE;
    }

    pItrNode = pNodeHeader;

    //if pstNode is NULL, delete all the node, exclusive the header node;
    if (NULL == pstNode)
    {
        pItrNode = pItrNode->pNext;

        while ( NULL != pItrNode )
        {
            pDelNode = pItrNode;
            pItrNode = pItrNode->pNext;

            MT_INFO_MEM("<%s>: delete module node:%#x\n", KERNEL_MODE, pDelNode->stMemInfo.u32ModuleID);

            kmodule_mem_pool_free(pDelNode);
        }

        pNodeHeader->pNext = NULL;
        pNodeHeader->u32ItemCnt = 0;

        return MT_SUCCESS;
    }

    while ((NULL != pItrNode) && (NULL != pItrNode->pNext))
    {
        if (pItrNode->pNext->stMemInfo.u32ModuleID == pstNode->stMemInfo.u32ModuleID)
        {
            break;
        }
        pItrNode = pItrNode->pNext;
    }

    if (NULL != pItrNode && NULL != pItrNode->pNext)
    {
        //found out the next node to delete.
        pDelNode = pItrNode->pNext;
        pItrNode->pNext = pItrNode->pNext->pNext;

        MT_INFO_MEM("<%s>: delete module node:%#x\n", KERNEL_MODE, pDelNode->stMemInfo.u32ModuleID);

        kmodule_mem_pool_free(pDelNode);

        pNodeHeader->u32ItemCnt--;

        return MT_SUCCESS;
    }

    return MT_FAILURE;
}

mt_s32 kmodule_mem_pool_del_module(mt_u32 u32ModuleID)
{
    kmodule_mem_item_s stNode = {{0}};

    stNode.stMemInfo.u32ModuleID = u32ModuleID;

    return kmodule_mem_pool_del_node(g_pstMemItemHeader, &stNode);
}


// Make one node and add it to the link tail
mt_s32 kmodule_mem_pool_update_node(kmodule_mem_item_s* pNodeHeader, kmodule_mem_item_s* pstNode, MT_BOOL bAddMemory)
{
    mt_s32      s32Ret     = MT_FAILURE;
    kmodule_mem_item_s* pMemItem   = NULL;

    mt_u32 u32TotalSize = 0;

    if (NULL == pNodeHeader || pstNode == NULL)
    {
        return MT_FAILURE;
    }

    s32Ret = kmodule_mem_pool_find_node(pNodeHeader, pstNode->stMemInfo.u32ModuleID, &pMemItem);
    if (MT_SUCCESS == s32Ret)
    {
        if (bAddMemory == MT_TRUE)
        {
            MT_INFO_MEM("<%s>++++ ++++, update module[0x%08x] memory info:add %d to old:%d\n", KERNEL_MODE,pstNode->stMemInfo.u32ModuleID, \
                pstNode->stMemInfo.u32UsrSize, pMemItem->stMemInfo.u32UsrSize);

            pMemItem->stMemInfo.u32KKSize += pstNode->stMemInfo.u32KKSize;
            pMemItem->stMemInfo.u32KVSize += pstNode->stMemInfo.u32KVSize;

            pMemItem->stMemInfo.u32UsrSize += pstNode->stMemInfo.u32UsrSize;
            pMemItem->stMemInfo.u32MMZSize += pstNode->stMemInfo.u32MMZSize;

            u32TotalSize = pMemItem->stMemInfo.u32KKSize + pMemItem->stMemInfo.u32KVSize;
            u32TotalSize += pMemItem->stMemInfo.u32UsrSize + pMemItem->stMemInfo.u32MMZSize;
        }
        else
        {
            MT_INFO_MEM("<%s>---- ----, update module[0x%08x] memory info:del:%d from old:%d\n", KERNEL_MODE,pstNode->stMemInfo.u32ModuleID, \
                pstNode->stMemInfo.u32MMZSize, pMemItem->stMemInfo.u32MMZSize);

            u32TotalSize = pMemItem->stMemInfo.u32KKSize + pMemItem->stMemInfo.u32KVSize;
            u32TotalSize += pMemItem->stMemInfo.u32UsrSize + pMemItem->stMemInfo.u32MMZSize;

            pMemItem->stMemInfo.u32KKSize -= pstNode->stMemInfo.u32KKSize;
            pMemItem->stMemInfo.u32KVSize -= pstNode->stMemInfo.u32KVSize;
            pMemItem->stMemInfo.u32UsrSize -= pstNode->stMemInfo.u32UsrSize;
            pMemItem->stMemInfo.u32MMZSize -= pstNode->stMemInfo.u32MMZSize;

            u32TotalSize = pMemItem->stMemInfo.u32KKSize + pMemItem->stMemInfo.u32KVSize;
            u32TotalSize += pMemItem->stMemInfo.u32UsrSize + pMemItem->stMemInfo.u32MMZSize;
        }

        MT_INFO_MEM("<%s>, total size:%d and max size:%d\n", KERNEL_MODE,u32TotalSize, pMemItem->stMemInfo.u32MaxSize);

        if ( u32TotalSize > pMemItem->stMemInfo.u32MaxSize)
        {
            pMemItem->stMemInfo.u32MaxSize = u32TotalSize;
        }

        return MT_SUCCESS;
    }

    MT_ERR_MEM("<%s> update module memory failure, id = 0x%x!!!\n",KERNEL_MODE, pstNode->stMemInfo.u32ModuleID);

    return MT_FAILURE;
}

static mt_s32 mem_kstat_callback(mt_u32 u32ModuleID, KMEM_TYPE_E enType, mt_s32 s32Size)
{
    mt_s32 s32Ret = MT_FAILURE;
    kmodule_mem_item_s stNode = {{0}};

    stNode.stMemInfo.u32ModuleID = u32ModuleID;

    switch(enType)
    {
        case KMEM_TYPE_MMZ:
        {
            stNode.stMemInfo.u32MMZSize = ABS_SIZE(s32Size);
        }
        break;

        case KMEM_TYPE_VMEM:
        {
            stNode.stMemInfo.u32KVSize = ABS_SIZE(s32Size);
        }
        break;

        case KMEM_TYPE_KMEM:
        {
            stNode.stMemInfo.u32KKSize = ABS_SIZE(s32Size);
        }
        break;

        default:
        break;
    }

    kmodule_mem_lock(MT_FAILURE);

    if (s32Size >= 0)
    {
        s32Ret = kmodule_mem_pool_update_node(g_pstMemItemHeader, &stNode, MT_TRUE);
    }
    else
    {
        s32Ret = kmodule_mem_pool_update_node(g_pstMemItemHeader, &stNode, MT_FALSE);
    }

    kmodule_mem_unlock();

    return s32Ret;
}

static mt_s32 cmpi_mem_ioctl(struct inode *inode, struct file *file,mt_u32 cmd, mt_void *arg)
{
    mt_s32 s32Ret = MT_SUCCESS;

    kmodule_mem_lock(MT_FAILURE);

    switch(cmd)
    {
        case CMD_ADD_MODULE_INFO:
        {
            module_info_s* pModule = (module_info_s*)arg;
            kmodule_mem_item_s stModuleMem = {{0}};

            // MT_INFO_MEM("<%s:%s>++++Add module info and module id=0x%08x name = %s\n", KERNEL_MODE, __func__, pModule->u32ModuleID, pModule->u8ModuleName);

            stModuleMem.stMemInfo.u32ModuleID = pModule->u32ModuleID;
            stModuleMem.stMemInfo.u32PID = current->pid;
            MT_INFO_MEM("stModuleMem.stMemInfo.u32PID = 0x%08x.\n", stModuleMem.stMemInfo.u32PID);

            s32Ret = kmodule_mem_pool_add_node(g_pstMemItemHeader, &stModuleMem);
        }
        break;
        case CMD_DEL_MODULE_INFO:
        {
            module_info_s* pModule = (module_info_s*)arg;
            kmodule_mem_item_s stModuleMem = {{0}};

            stModuleMem.stMemInfo.u32ModuleID = pModule->u32ModuleID;
            stModuleMem.stMemInfo.u32PID = current->pid;

            MT_INFO_MEM("stModuleMem.stMemInfo.u32PID = 0x%08x.\n", stModuleMem.stMemInfo.u32PID);

            // MT_INFO_MEM("<%s:%s>----Del module info and module id=0x%08x name = %s \n",KERNEL_MODE, __func__, pModule->u32ModuleID, pModule->u8ModuleName);

            s32Ret = kmodule_mem_pool_del_node(g_pstMemItemHeader, &stModuleMem);
        }
        break;
        case CMD_MEM_ADD_INFO:
        {
            module_mem_info_s* pMemInfo = (module_mem_info_s*)arg;
            kmodule_mem_item_s stModuleMem = {{0}};


            MT_INFO_MEM("<%s>Add module memory with id=0x%08x mmz size:%u, user size:%u\n", KERNEL_MODE, pMemInfo->u32ModuleID, pMemInfo->u32SizeMMZ, \
                    pMemInfo->u32SizeUsrMem);

            stModuleMem.stMemInfo.u32ModuleID = pMemInfo->u32ModuleID;
            stModuleMem.stMemInfo.u32MMZSize  = pMemInfo->u32SizeMMZ;
            stModuleMem.stMemInfo.u32UsrSize  = pMemInfo->u32SizeUsrMem;

            //stModuleMem.stMemInfo.u32MaxSize  = pMemInfo->u32MaxMemSize;

            s32Ret = kmodule_mem_pool_update_node(g_pstMemItemHeader, &stModuleMem, MT_TRUE);
        }
        break;
        case CMD_MEM_DEL_INFO:
        {
            module_mem_info_s* pMemInfo = (module_mem_info_s*)arg;
            kmodule_mem_item_s stModuleMem = {{0}};

            MT_INFO_MEM("<%s>del module memory info ... with id=0x%08x\n", KERNEL_MODE, pMemInfo->u32ModuleID);
            MT_INFO_MEM("<%s>del user size is %d, and mmz size is %d\n", KERNEL_MODE, pMemInfo->u32SizeUsrMem, \
                    pMemInfo->u32SizeMMZ);

            stModuleMem.stMemInfo.u32ModuleID = pMemInfo->u32ModuleID;
            stModuleMem.stMemInfo.u32MMZSize  = pMemInfo->u32SizeMMZ;
            stModuleMem.stMemInfo.u32UsrSize  = pMemInfo->u32SizeUsrMem;

            //stModuleMem.stMemInfo.u32MaxSize  = pMemInfo->u32MaxMemSize;

            s32Ret = kmodule_mem_pool_update_node(g_pstMemItemHeader, &stModuleMem, MT_FALSE);
        }
        break;
        default:
            kmodule_mem_unlock();
            MT_ERR_MEM("<%s>================Unknown cmd:%#x\n", KERNEL_MODE, cmd);
        return MT_FAILURE;
    }

    UNUSED(file);
    UNUSED(inode);

    kmodule_mem_unlock();

    return s32Ret;
}

static long mem_drv_ioctl(struct file *file,
                            mt_u32 cmd,
                            unsigned long arg)
{
    int ret;
    ret=MT_DRV_UserCopy(file->f_path.dentry->d_inode, file, cmd, arg, cmpi_mem_ioctl);

    return (long)ret;
}

static mt_s32 mem_drv_release(struct inode * inode, struct file * file)
{
    return 0;
}

static mt_s32 mem_drv_open(struct inode * inode, struct file * file)
{
    return 0;
}

static struct file_operations drv_mem_fops=
{
    .owner          = THIS_MODULE,
    .open           = mem_drv_open,
    .unlocked_ioctl = mem_drv_ioctl,
    .release        = mem_drv_release,
};

mt_s32 mem_proc_write( struct file * file,  const char __user * buf,
                     size_t count, loff_t *ppos)
{
    return 0;
}

mt_s32 mem_proc_read(struct seq_file *s, mt_void *pArg)
{
    mt_u32 u32Count = 0;
    mt_u32 u32Total = 0;
    mt_u8* pModuleName = NULL;
    kmodule_mem_item_s* pItr = NULL;
    mt_u32 u32TotalSize = 0;
    mt_u32 u32SizeK = 0;

    if (0 == g_ModuleMemModInit)
    {
        PROC_PRINT(s,"    Mem module not init\n");
        return 0;
    }


    kmodule_mem_lock(MT_FAILURE);

    PROC_PRINT(s, COLOR_START_HEAD);
    PROC_PRINT(s, "----------------------------------------------------------------------------------------------\n");
    PROC_PRINT(s, "|Module Name |    ID    |   MMZ   |   USR_MEM   |   KERNEL_MEM   |   MAX_MEM   |   CUR_MEM   |\n");
    PROC_PRINT(s, "----------------------------------------------------------------------------------------------\n");
    PROC_PRINT(s, COLOR_END);

    if (NULL != g_pstMemItemHeader )
    {
        pItr = g_pstMemItemHeader->pNext;
        u32Total = g_pstMemItemHeader->u32ItemCnt;

        while (NULL != pItr)
        {
            pModuleName = mt_drv_module_getname_byid(pItr->stMemInfo.u32ModuleID);

            if (NULL == pModuleName)
            {
                pModuleName = "UnknownModule";
            }

            u32SizeK = pItr->stMemInfo.u32KKSize + pItr->stMemInfo.u32KVSize;

            u32TotalSize = pItr->stMemInfo.u32MMZSize + pItr->stMemInfo.u32UsrSize;
            u32TotalSize += u32SizeK;

            if (u32Count%2 == 0)
            {
                PROC_PRINT(s, COLOR_START_HEAD"|"COLOR_END);
                PROC_PRINT(s, " %-8.8s    0x%-8x   %-8u      %-8u      %-8u      %-8u      %-8u  ", pModuleName, \
                     pItr->stMemInfo.u32ModuleID, pItr->stMemInfo.u32MMZSize, pItr->stMemInfo.u32UsrSize, \
                     u32SizeK, pItr->stMemInfo.u32MaxSize, u32TotalSize);
                PROC_PRINT(s, COLOR_START_HEAD"|\n"COLOR_END);

                u32Count = 1;
            }
            else
            {
                PROC_PRINT(s, COLOR_START_HEAD"|"COLOR_END COLOR_START_RED);
                PROC_PRINT(s, " %-8.8s    0x%-8x   %-8u      %-8u      %-8u      %-8u      %-8u  ", pModuleName, \
                     pItr->stMemInfo.u32ModuleID, pItr->stMemInfo.u32MMZSize, pItr->stMemInfo.u32UsrSize, \
                     u32SizeK, pItr->stMemInfo.u32MaxSize, u32TotalSize);
                PROC_PRINT(s, COLOR_END COLOR_START_HEAD"|"COLOR_END"\n");

               u32Count = 0;
            }

            pItr = pItr->pNext;
        }
    }
    PROC_PRINT(s, COLOR_START_HEAD);
    PROC_PRINT(s, "----------------------------------------------------------------------------------------------\n");
    PROC_PRINT(s, COLOR_END);

    kmodule_mem_unlock();

    return 0;
}

mt_u32 kmodule_memmgr_get_usedsize(mt_u32 u32ModuleID)
{
    kmodule_mem_item_s* pMemItem   = NULL;

    mt_u32 u32TotalSize = 0;

    kmodule_mem_lock(0);

    kmodule_mem_pool_find_node(g_pstMemItemHeader, u32ModuleID, &pMemItem);

    kmodule_mem_unlock();

    if ( NULL != pMemItem)
    {
        u32TotalSize = pMemItem->stMemInfo.u32KKSize + pMemItem->stMemInfo.u32KVSize;
        u32TotalSize += pMemItem->stMemInfo.u32MMZSize + pMemItem->stMemInfo.u32UsrSize;
    }

    return u32TotalSize;
}

#if 0
static mt_device_s    g_stModuleMemDev = {{0}};

mt_s32 kmodule_memmgr_init(mt_u32 u32ModuleCount, mt_u32 u32ModuleMemCount)
{
    DRV_PROC_ITEM_S *item ;

    kmodule_mem_init(u32ModuleMemCount, mem_kstat_callback);

    kmodule_mem_pool_init(u32ModuleCount);
    // 0
    MT_OSAL_Snprintf(g_stModuleMemDev.devfs_name, sizeof(g_stModuleMemDev.devfs_name), "%s", UMAP_DEVNAME_MEM2);
    g_stModuleMemDev.fops = &drv_mem_fops;
    g_stModuleMemDev.minor = UMAP_MIN_MINOR_MEM2;
    g_stModuleMemDev.owner  = THIS_MODULE;
    g_stModuleMemDev.drvops = NULL;

    if(MT_DRV_DEV_Register(&g_stModuleMemDev) < 0)
    {
        MT_ERR_MEM("<%s>Unable to register dbg dev\n", KERNEL_MODE);
        return -1;
    }
    // 1
    item = mt_drv_proc_add_module(MT_MOD_MEM2, NULL, NULL);
    if (! item)
    {
        MT_DRV_DEV_UnRegister(&g_stModuleMemDev);
        return -1;
    }
    //proc read and write interface.
    item->read = mem_proc_read;
    item->write = mem_proc_write;
    g_ModuleMemModInit = 1;

    return 0;
}

mt_void kmodule_memmgr_exit(mt_void)
{
    kmodule_mem_deinit();
    kmodule_mem_pool_deinit();
    g_ModuleMemModInit = 0;
    mt_drv_proc_rm_module(MT_MOD_MEM2);
    MT_DRV_DEV_UnRegister(&g_stModuleMemDev);
    return;
}
#endif




struct kmodule_memmgr_device {
	struct device *dev;
	void *platdata;
	dev_t devt;
	int id;
};

struct kmodule_memmgr_driver {
	struct cdev cdev;
	dev_t devt;
	dev_t major;
	dev_t minor;
	u32 minors;
};

static struct kmodule_memmgr_driver *kmodule_memmgr_drv;

static int kmodule_memmgr_probe(struct platform_device *pdev)
{
	mt_proc_entry_t *item = NULL;
	struct kmodule_memmgr_device *kmodule_memmgr_dev = NULL;
	int ret = 0;

	kmodule_memmgr_dev = kzalloc(sizeof(struct kmodule_memmgr_device), GFP_KERNEL);
	if (!kmodule_memmgr_dev) {
		pr_err("Error kzalloc kmodule_memmgr_dev\n\n");
		ret = -ENOMEM;
		goto fail_kzalloc_kmodule_memmgr_dev;
	}
	kmodule_memmgr_dev->id = pdev->id;
	kmodule_memmgr_dev->devt = kmodule_memmgr_drv->devt + kmodule_memmgr_dev->id;
	kmodule_memmgr_dev->platdata = dev_get_platdata(&pdev->dev);

#if 0
	kmodule_memmgr_dev->dev = device_create(mt_class, &pdev->dev, kmodule_memmgr_dev->devt, NULL, UMAP_DEVNAME_MEM2);
#else
	kmodule_memmgr_dev->dev = device_create(mt_class, NULL, kmodule_memmgr_dev->devt, NULL, UMAP_DEVNAME_MEM2);
#endif
	if (IS_ERR(kmodule_memmgr_dev->dev)) {
		pr_err("Error device_create\n\n");
		ret = PTR_ERR(kmodule_memmgr_dev->dev);
		goto fail_device_create;
	}

	platform_set_drvdata(pdev, kmodule_memmgr_dev);
	dev_set_drvdata(kmodule_memmgr_dev->dev, kmodule_memmgr_dev);

    kmodule_mem_init(((struct moduleCount_moduleMemCount *)(kmodule_memmgr_dev->platdata))->u32ModuleMemCount, mem_kstat_callback);

    kmodule_mem_pool_init(((struct moduleCount_moduleMemCount *)(kmodule_memmgr_dev->platdata))->u32ModuleCount);

    item = mt_drv_proc_add_module(MT_MOD_MEM2, NULL, NULL);
    if (!item)
    {
		goto fail_mt_drv_proc_add_module;
    }

    //proc read and write interface.
    item->read = mem_proc_read;
    item->write = mem_proc_write;
    g_ModuleMemModInit = 1;

	return 0;

fail_mt_drv_proc_add_module:
	device_destroy(mt_class, kmodule_memmgr_dev->devt);
fail_device_create:
	kfree(kmodule_memmgr_dev);
	kmodule_memmgr_dev = NULL;
fail_kzalloc_kmodule_memmgr_dev:
	return ret;
}

static int kmodule_memmgr_remove(struct platform_device *pdev)
{
	struct kmodule_memmgr_device *kmodule_memmgr_dev = platform_get_drvdata(pdev);

    kmodule_mem_deinit();
    kmodule_mem_pool_deinit();
    g_ModuleMemModInit = 0;
    mt_drv_proc_rm_module(MT_MOD_MEM2);

	device_destroy(mt_class, kmodule_memmgr_dev->devt);
	kmodule_memmgr_dev->dev = NULL;

	platform_set_drvdata(pdev, NULL);

	if (kmodule_memmgr_dev) {
		kfree(kmodule_memmgr_dev);
		kmodule_memmgr_dev = NULL;
	}

	return 0;
}

static int kmodule_memmgr_suspend(struct platform_device *pdev, pm_message_t stState)
{
	return 0;
}

static int kmodule_memmgr_resume(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver kmodule_memmgr_platform_driver = {
	.probe = kmodule_memmgr_probe,
	.remove = kmodule_memmgr_remove,
	.suspend = kmodule_memmgr_suspend,
	.resume = kmodule_memmgr_resume,
	.driver = {
		.name = UMAP_DEVNAME_MEM2,
		.owner = THIS_MODULE,
	}
};

static void kmodule_memmgr_release_device(struct device *pdev) {  }

static struct moduleCount_moduleMemCount moduleCount_moduleMemCount0;
static struct platform_device kmodule_memmgr_platform_device = {
	.name = UMAP_DEVNAME_MEM2,
	.id = 0,
	.dev= {
		.platform_data = NULL,
		.release = kmodule_memmgr_release_device,
	},
};

int __init kmodule_memmgr_init(mt_u32 u32ModuleCount, mt_u32 u32ModuleMemCount)
{
	int ret;

	kmodule_memmgr_drv = kzalloc(sizeof(struct kmodule_memmgr_driver), GFP_KERNEL);
	if (!kmodule_memmgr_drv) {
		pr_err("Error kzalloc kmodule_memmgr_drv\n\n");
		ret = -ENOMEM;
		goto fail_kzalloc_kmodule_memmgr_drv;
	}

	kmodule_memmgr_drv->major = MT_DEVICE_MAJOR;
	kmodule_memmgr_drv->minor = UMAP_MIN_MINOR_MEM2;
	kmodule_memmgr_drv->minors = UMAP_DEV_NUM_MEM2;
	kmodule_memmgr_drv->devt = MKDEV(kmodule_memmgr_drv->major, kmodule_memmgr_drv->minor);
	cdev_init(&kmodule_memmgr_drv->cdev, &drv_mem_fops);
	kmodule_memmgr_drv->cdev.owner = THIS_MODULE;

	ret = cdev_add(&kmodule_memmgr_drv->cdev, kmodule_memmgr_drv->devt, kmodule_memmgr_drv->minors);
	if (ret) {
		pr_err("Error cdev_add\n\n");
		ret = -EINVAL;
		goto fail_cdev_add;
	}

	ret = platform_driver_register(&kmodule_memmgr_platform_driver);
	if (ret) {
		pr_err("Error platform_driver_register\n\n");
		goto fail_platform_driver_register;
	}

	kmodule_memmgr_platform_device.dev.platform_data = &moduleCount_moduleMemCount0;
	moduleCount_moduleMemCount0.u32ModuleCount = u32ModuleCount;
	moduleCount_moduleMemCount0.u32ModuleMemCount = u32ModuleMemCount;
	ret = platform_device_register(&kmodule_memmgr_platform_device);
	if (ret) {
		pr_err("Error platform_device_register\n\n");
		goto fail_platform_device_register;
	}

	return 0;

fail_platform_device_register:
	platform_driver_unregister(&kmodule_memmgr_platform_driver);
fail_platform_driver_register:
	cdev_del(&kmodule_memmgr_drv->cdev);
fail_cdev_add:
	kfree(kmodule_memmgr_drv);
	kmodule_memmgr_drv = NULL;
fail_kzalloc_kmodule_memmgr_drv:
	return ret;
}

void kmodule_memmgr_exit(void)
{
	platform_driver_unregister(&kmodule_memmgr_platform_driver);
	platform_device_unregister(&kmodule_memmgr_platform_device);
	kfree(kmodule_memmgr_drv);
	kmodule_memmgr_drv = NULL;
}


