#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include "mt_type.h"
#include "mt_debug.h"
#include "mt_kernel_adapt.h"
#include "mt_module.h"
#include "mt_osal.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "drv_module_ioctl.h"
#include "drv_mutils.h"
#include "mt_drv_module.h"
#include "drv_mmgr.h"

#ifdef CMN_MMGR_SUPPORT
#include "drv_mem.h"
#endif

static mt_u32           g_ModuleModInit = 0;
static module_s*        g_pstKModuleHeader = NULL;

#define MODULE_PROC_SUPPORT

#define COLOR_START      "\33[32m"
#define COLOR_START_HEAD "\33[35m" /* pink text color*/

#define COLOR_START_RED "\33[31m"  /* red text color */
#define COLOR_END       "\33[0m"

#define KERNEL_MODE   "kernel"

#define MIN_LEN(x, y) ( (x) > (y) ? (y) : (x) )

#define KMODULE_BASE_ID (MT_DEBUG_ID_BUTT)

#define MIN_USER_MODULE_NUMBER 256

mt_handle              g_hKModuleMgr = 0;

static MT_DECLARE_MUTEX(g_KModuleMgrMutex);

#define kmodule_mgr_lock(RET)                                   \
do {                                                             \
    mt_s32 s32LockRet = down_interruptible(&g_KModuleMgrMutex); \
    if ( s32LockRet != 0 )                                      \
    {                                                           \
        return RET;                                             \
    }                                                           \
} while(0)

#define kmodule_mgr_unlock() up(&g_KModuleMgrMutex)

static DEFINE_SPINLOCK(g_KModuleMgr_spinlock);
static mt_s32 modulemgr_link_delnode(module_s* pNodeHeader, module_s* pstNode, MT_BOOL bForce);
static mt_s32 module_drv_register(mt_u32 u32ModuleID,
                        const mt_u8* pu8ModuleName, mt_void* pFunc, struct file * file);

extern mt_s32 log_add_module(mt_pchar szProcName, mt_mod_id_e u32It);
extern mt_s32 log_remove_module(mt_pchar szProcName, mt_mod_id_e u32ItemID);

#if defined(CONFIG_TEE)
extern void msp_drv_modules_user_init(void);
#endif

/*
MT_U32 ModuleMgr_RegisterID(const MT_U8* pu8Name)
{
    KMD5_CTX md5c = {{0}};
    MT_U32 u32Result = 0;
    MT_U8 u8Md5Result[16] = {0};
    MT_U8 u8BufResult[16]={'\0'};
    MT_U8 u8Tmp[3]={'\0'};
    MT_U8 i = 0;

    MT_INFO_MEM("to md5 name is %s\n", pu8Name);

    KMD5Init( &md5c );
    KMD5Update( &md5c, (unsigned char*)pu8Name, strlen(pu8Name) );
    KMD5Final( u8Md5Result, &md5c );

    //Just only make the first four bytes available.
    for( i=0; i<4; i++ )
    {
        sprintf(u8Tmp,"%02X", u8Md5Result[i] );
        strcat(u8BufResult,u8Tmp);
    }

    MT_INFO_MEM("u8BufResult is %s\n", u8BufResult);

    u32Result = (MT_U32)simple_strtoll(u8BufResult, NULL, 16) + KMODULE_BASE_ID;

    u32Result &= 0x7FFFFFFF;

    MT_INFO_MEM("u32Result 2 is 0x%08x\n", u32Result);

    return u32Result;
}
//*/

static mt_s32 modulemgr_findnode_byname(module_s* pNodeHeader, module_s** pstNode, const mt_u8* pu8ModuleName)
{
    module_s* pItrNode = NULL;
    mt_s32 s32CmpResult = 0;

    pItrNode = pNodeHeader;

    if (NULL == pu8ModuleName)
    {
        MT_ERR_MODULE("param invalid!\n");

        return MT_FAILURE;
    }

    while (NULL != pItrNode)
    {

        s32CmpResult = mt_osal_strncmp(pItrNode->stModuleInfo.u8ModuleName,
                            pu8ModuleName, sizeof(pItrNode->stModuleInfo.u8ModuleName));
        if ( s32CmpResult == 0)
        {
            //find out
            *pstNode = pItrNode;

            break;
        }

        pItrNode = pItrNode->pNextModule;
    }

    if (NULL != pItrNode)
    {
        MT_INFO_MODULE("found out the module name:%s node\n", pu8ModuleName);

        return MT_SUCCESS;
    }

    MT_INFO_MODULE("not found out the module name:%s node\n", pu8ModuleName);

    return MT_FAILURE;
}

static mt_s32 modulemgr_findnode_byid(module_s* pNodeHeader, module_s** pstNode, mt_u32 u32ModuleID)
{
    module_s* pItrNode = NULL;

    pItrNode = pNodeHeader;

    while (NULL != pItrNode)
    {
        if ( u32ModuleID == pItrNode->stModuleInfo.u32ModuleID)
        {
            //find out
            *pstNode = pItrNode;

            break;
        }

        pItrNode = pItrNode->pNextModule;
    }

    if (NULL != pItrNode)
    {
        MT_INFO_MODULE("found out the module id:0x%08x node\n", u32ModuleID);

        return MT_SUCCESS;
    }

    MT_INFO_MODULE("not found out the module id:0x%08x node\n", u32ModuleID);

    return MT_FAILURE;

}

static mt_u32 modulemgr_alloc_moduleid(mt_void)
{
    mt_u32 u32ModuleId;
    module_s* pstFindMoudle;

    for (u32ModuleId=MIN_USER_MODULE_NUMBER; u32ModuleId<MIN_USER_MODULE_NUMBER+MT_MAX_USER_MODULE_NUMBER; u32ModuleId++)
    {
        if (MT_SUCCESS != modulemgr_findnode_byid(g_pstKModuleHeader, &pstFindMoudle, u32ModuleId))
        {
            return u32ModuleId;
        }
    }

    return MT_INVALID_MODULE_ID;
}

static mt_s32 modulemgr_link_init(mt_u32 u32Count)
{
    kmem_utils_s stModulePool = {0};

    if (NULL != g_pstKModuleHeader)
    {
        MT_INFO_MODULE("Init has been called.\n");

        return MT_SUCCESS;
    }

    g_pstKModuleHeader = (module_s*)kmalloc(sizeof(module_s), GFP_KERNEL);
    if (NULL == g_pstKModuleHeader)
    {
        MT_ERR_MODULE("kmalloc size %d failure\n", sizeof(module_s));
        return MT_FAILURE;
    }

    memset(g_pstKModuleHeader, 0, sizeof(module_s));

    stModulePool.enType = KMEM_POOL_TYPE_MODULE;

    g_hKModuleMgr = kmem_utils_init(u32Count, stModulePool);

    return MT_SUCCESS;
}

static mt_s32 modulemgr_link_deInit(mt_void)
{
    modulemgr_link_delnode(g_pstKModuleHeader, NULL, MT_TRUE);

    kmem_utils_deinit(g_hKModuleMgr);

    if (NULL != g_pstKModuleHeader)
    {
        kfree(g_pstKModuleHeader);
    }

    return MT_SUCCESS;
}

static module_s* modulemgr_link_hasnode(module_s* pNodeHeader, module_s* pstNode)
{
    module_s* pItrNode = pNodeHeader;

    if (pNodeHeader == NULL)
    {
        return NULL;
    }

    pItrNode = pNodeHeader->pNextModule;

    while (NULL != pItrNode)
    {
        if (pItrNode->stModuleInfo.u32ModuleID == pstNode->stModuleInfo.u32ModuleID)
        {
            break;
        }

        pItrNode = pItrNode->pNextModule;
    }

    if (pItrNode != NULL)
    {
        return pItrNode;
    }

    return NULL;
}

// Make one node and add it to the link tail
static mt_s32 modulemgr_link_addnode(module_s* pNodeHeader, module_s* pstNode)
{
    module_s* pItrNode = NULL;
    module_s* pstAddNode = NULL;
    module_pool_s* pstModuleBase = NULL;

    if (NULL == pNodeHeader)
    {
        MT_ERR_MODULE("add node failure, node header is NULL\n");

        return MT_FAILURE;
    }

    pItrNode = modulemgr_link_hasnode(pNodeHeader, pstNode);
    if (pItrNode)
    {
        if (!pItrNode->stModuleInfo.pFnCallback)
        {
            pItrNode->stModuleInfo.pFnCallback = pstNode->stModuleInfo.pFnCallback;
        }
        //has added it, so return success, directly
        MT_INFO_MODULE("has node %s in link\n" , pstNode->stModuleInfo.u8ModuleName);
        return MT_SUCCESS;
    }

    pstModuleBase = (module_pool_s*)kmem_utils_malloc(g_hKModuleMgr);
    if ( NULL == pstModuleBase || 1 != pstModuleBase->u32Idle )
    {

        MT_ERR_MODULE("add node failure, malloc node failure.\n");
        return MT_FAILURE;
    }
    pstAddNode    =  &pstModuleBase->stItem;

    memset(pstAddNode, 0, sizeof(module_s));

    MT_INFO_MODULE("add module name %s at address %p\n", pstNode->stModuleInfo.u8ModuleName, pstAddNode);

    memcpy(pstAddNode->stModuleInfo.u8ModuleName, pstNode->stModuleInfo.u8ModuleName, sizeof(pstAddNode->stModuleInfo.u8ModuleName));
    pstAddNode->stModuleInfo.u32ModuleID = pstNode->stModuleInfo.u32ModuleID;
    pstAddNode->stModuleInfo.pFnCallback = pstNode->stModuleInfo.pFnCallback;
    pstAddNode->stModuleInfo.s32RegCount = 1;
    pstAddNode->stModuleInfo.file = pstNode->stModuleInfo.file;
    pstAddNode->pNextModule = NULL;

    pItrNode = pNodeHeader;
    pNodeHeader->u32ItemCnt++;

    while ( NULL != pItrNode->pNextModule )
    {
        pItrNode = pItrNode->pNextModule;
    }

    pItrNode->pNextModule = pstAddNode;

    return MT_SUCCESS;
}

// Delete the node from link
static mt_s32 modulemgr_link_delnode(module_s* pNodeHeader, module_s* pstNode, MT_BOOL bForce)
{
    module_s* pItrNode = NULL;
    module_s* pDelNode = NULL;

    if (NULL == pNodeHeader || pNodeHeader->pNextModule == NULL)
    {
        MT_ERR_MODULE("delete node failure, node header is NULL\n");

        return MT_FAILURE;
    }

    pItrNode = pNodeHeader;

    //if pstNode is NULL, delete all the node, exclusive the header node;
    if (NULL == pstNode)
    {
        pItrNode = pItrNode->pNextModule;

        while ( NULL != pItrNode )
        {
            pDelNode = pItrNode;
            pItrNode = pItrNode->pNextModule;

            MT_INFO_MODULE("delete module node:%s\n", pDelNode->stModuleInfo.u8ModuleName);

            kmem_utils_free(g_hKModuleMgr, container_of(pDelNode, module_pool_s, stItem));
        }

        pNodeHeader->pNextModule = NULL;

        return MT_SUCCESS;
    }

    while (NULL != pItrNode && NULL != pItrNode->pNextModule)
    {
        if (pItrNode->pNextModule->stModuleInfo.u32ModuleID == pstNode->stModuleInfo.u32ModuleID)
        {
            break;
        }
        pItrNode = pItrNode->pNextModule;
    }

    if (NULL != pItrNode && NULL != pItrNode->pNextModule)
    {
        /* If user mode registered more than 1 time, only sub count before count = 1 */
        if ((!bForce) && (pItrNode->pNextModule->stModuleInfo.s32RegCount > 1))
        {
            pItrNode->pNextModule->stModuleInfo.s32RegCount--;
            return MT_SUCCESS;
        }
        //found out the next node to delete.
        pDelNode = pItrNode->pNextModule;
        pItrNode->pNextModule = pItrNode->pNextModule->pNextModule;

        MT_INFO_MODULE("delete module node:%s\n", pDelNode->stModuleInfo.u8ModuleName);

        kmem_utils_free(g_hKModuleMgr, container_of(pDelNode, module_pool_s, stItem));

        pNodeHeader->u32ItemCnt--;

        return MT_SUCCESS;
    }

    return MT_FAILURE;
}

static mt_s32 cmpi_module_ioctl(struct inode *inode, struct file *file,mt_u32 cmd, mt_void *arg)
{
    mt_s32 s32Ret = MT_FAILURE;

    switch(cmd)
    {
        case CMD_ADD_MODULE_INFO:
        {
            module_info_s *pModule = (module_info_s*)arg;

            if (MT_INVALID_MODULE_ID != pModule->u32ModuleID)
            {
                s32Ret = module_drv_register(pModule->u32ModuleID, pModule->u8ModuleName, pModule->pFnCallback, file);
            }

            MT_INFO_MODULE("add module:%s, id %#x!\n", pModule->u8ModuleName, pModule->u32ModuleID);
        }
        break;
        case CMD_GET_MODULE_INFO:
        {
            module_info_s *pModule = (module_info_s*)arg;
            module_s      *pNode = NULL;

            if (NULL != pModule)
            {
                if (pModule->u32ModuleID != 0)
                {
                    s32Ret = modulemgr_findnode_byid(g_pstKModuleHeader, &pNode, pModule->u32ModuleID);
                }
                else
                {
                    s32Ret = modulemgr_findnode_byname(g_pstKModuleHeader, &pNode, pModule->u8ModuleName);
                }

                if (MT_SUCCESS == s32Ret)
                {
                    mt_u32 u32MinLen = 0;

                    pModule->u32ModuleID = pNode->stModuleInfo.u32ModuleID;
                    pModule->pFnCallback = pNode->stModuleInfo.pFnCallback;

                    u32MinLen = MIN_LEN( sizeof(pModule->u8ModuleName)-1, strlen(pNode->stModuleInfo.u8ModuleName));
                    memcpy(pModule->u8ModuleName, pNode->stModuleInfo.u8ModuleName, u32MinLen);

                    MT_INFO_MODULE("get module:%s, id %#x!\n", pModule->u8ModuleName, pModule->u32ModuleID);
                }
            }
        }
        break;
        case CMD_DEL_MODULE_INFO:
        {
            module_info_s *pModule = (module_info_s*)arg;

            s32Ret = mt_drv_module_unregister(pModule->u32ModuleID);
            MT_INFO_MODULE("del module:%s, id %#x!\n", pModule->u8ModuleName, pModule->u32ModuleID);
        }
        break;
        case CMD_ALLOC_MODULE_ID:
        {
            module_alloc_s *pstModule = (module_alloc_s*)arg;

            pstModule->s32Status = -1;
            s32Ret = mt_drv_module_allocid(pstModule->u8ModuleName, &(pstModule->u32ModuleID), &(pstModule->s32Status));
        }
        break;
        default:
            MT_ERR_MODULE("================cmd:%#x\n", cmd);
            s32Ret = MT_SUCCESS;
        break;
    }

    UNUSED(file);
    UNUSED(inode);

    return s32Ret;
}

static long module_drv_ioctl(struct file *file,
                            mt_u32 cmd,
                            unsigned long arg)
{
    int ret;
    ret=mt_drv_usercopy(file->f_path.dentry->d_inode, file, cmd, arg, cmpi_module_ioctl);

    return ret;
}

static mt_s32 module_drv_release(struct inode * inode, struct file * file)
{
    module_s* pstLastNode = g_pstKModuleHeader;
    module_s* pstNode = pstLastNode->pNextModule;

    /* UnRegister moudules registered by this file */
    while (MT_NULL != pstNode)
    {
        if ((ulong)file == pstNode->stModuleInfo.file)
        {
            MT_INFO_MODULE("Remove module %s\n", pstNode->stModuleInfo.u8ModuleName);
            log_remove_module(pstNode->stModuleInfo.u8ModuleName, pstNode->stModuleInfo.u32ModuleID);
            modulemgr_link_delnode(pstLastNode, pstNode, MT_TRUE);
            /* pstNode had been freed */
            pstNode = pstLastNode->pNextModule;
        }
        else
        {
            pstLastNode = pstNode;
            pstNode = pstNode->pNextModule;
        }
    }

    return 0;
}

static mt_s32 module_drv_open(struct inode * inode, struct file * file)
{
#if defined(CONFIG_TEE) && defined(CONFIG_MT_MSP)
	msp_drv_modules_user_init();
#endif

    return 0;
}

static struct file_operations drv_module_fops=
{
    .owner          =THIS_MODULE,
    .open           =module_drv_open,
    .unlocked_ioctl =module_drv_ioctl,
    .release        =module_drv_release,
};

#ifdef MODULE_PROC_SUPPORT
mt_s32 module_proc_write( struct file * file,  const char __user * buf,
                     size_t count, loff_t *ppos)
{
    return 0;
}

#define SPLIT_LINE "--------------------------------------------------------\n"

extern mt_s32 log_getlevel(mt_u32 enModId, mt_u8* u8Buf, mt_u32 u32Length);

#ifdef CMN_MMGR_SUPPORT
mt_u32 kmodule_memmgr_get_usedsize(mt_u32 u32ModuleID);
#endif

mt_s32 module_proc_read(struct seq_file *s, mt_void *pArg)
{
    mt_u32 u32Count = 0;
    module_s* pItr = NULL;
    mt_u8  u8LevelName[8] = {0};
    mt_u8  u8LogLevel = 0;

#ifdef CMN_MMGR_SUPPORT
    mt_u32 u32TotalSize = 0;
    mt_u32 u32Size = 0;
#endif

    if (0 == g_ModuleModInit)
    {
        PROC_PRINT(s,"    Module module not init\n");
        return 0;
    }

    kmodule_mgr_lock(MT_FAILURE);

    PROC_PRINT(s, COLOR_START_HEAD);
    PROC_PRINT(s, SPLIT_LINE);
    PROC_PRINT(s, "|  Module Name  |     ID      | Log Level | Heap Memory |\n");
    PROC_PRINT(s, SPLIT_LINE);
    PROC_PRINT(s, COLOR_END);

    if (NULL != g_pstKModuleHeader)
    {
        pItr = g_pstKModuleHeader->pNextModule;
        while (NULL != pItr)
        {
            u8LogLevel = log_getlevel(pItr->stModuleInfo.u32ModuleID, u8LevelName, sizeof(u8LevelName));

            if (u32Count%2 == 0)
            {
                PROC_PRINT(s, COLOR_START_HEAD"|"COLOR_END);
                PROC_PRINT(s, " %-16.16s 0x%08x   %d: %-s ", pItr->stModuleInfo.u8ModuleName,  pItr->stModuleInfo.u32ModuleID, \
                                                             u8LogLevel, u8LevelName);
#ifdef CMN_MMGR_SUPPORT
                u32Size = kmodule_memmgr_get_usedsize(pItr->stModuleInfo.u32ModuleID);
                u32TotalSize += u32Size;
                PROC_PRINT(s, "     %-10d", u32Size);
#else
                PROC_PRINT(s, "     %-10s", " ");
#endif
                PROC_PRINT(s, COLOR_START_HEAD"|\n"COLOR_END);

                u32Count = 1;
            }
            else
            {
                PROC_PRINT(s, COLOR_START_HEAD"|"COLOR_END COLOR_START_RED);
                PROC_PRINT(s, " %-16.16s 0x%08x   %d: %-s ", pItr->stModuleInfo.u8ModuleName,  pItr->stModuleInfo.u32ModuleID, \
                                                             u8LogLevel, u8LevelName);
#ifdef CMN_MMGR_SUPPORT
                u32Size = kmodule_memmgr_get_usedsize(pItr->stModuleInfo.u32ModuleID);
                u32TotalSize += u32Size;
                PROC_PRINT(s, "     %-10d", u32Size);
#else
                PROC_PRINT(s, "     %-10s", " ");
#endif
                PROC_PRINT(s, COLOR_END COLOR_START_HEAD"|"COLOR_END"\n");

                u32Count = 0;
            }

            pItr = pItr->pNextModule;
        }
    }

    PROC_PRINT(s, COLOR_START_HEAD);
    PROC_PRINT(s, SPLIT_LINE);

#ifdef CMN_MMGR_SUPPORT
    PROC_PRINT(s, "| %-43s %-10d|\n", "Total", u32TotalSize);
    PROC_PRINT(s, SPLIT_LINE);
#endif

    PROC_PRINT(s, COLOR_END);

    kmodule_mgr_unlock();

    return 0;
}
#endif

mt_s32 mt_drv_mmngr_init(mt_u32 u32ModuleCount, mt_u32 u32ModuleMemCount)
{
    modulemgr_link_init(u32ModuleCount);

    return MT_SUCCESS;
}

#if 0
static mt_device_s    g_stModuleDev = {{0}};

mt_s32 mmngr_drv_modinit(mt_u32 u32ModuleCount, mt_u32 u32ModuleMemCount)
{
    mt_s32 s32Ret;
    mt_proc_entry_t *item ;

    if (g_ModuleModInit)
    {
        return MT_SUCCESS;
    }

#ifdef CMN_MMGR_SUPPORT
    kmodule_memmgr_init(u32ModuleCount, u32ModuleMemCount);
#endif

    s32Ret = mt_drv_module_register(MT_ID_SYS,    "MT_SYS",      MT_NULL);
    s32Ret |= mt_drv_module_register(MT_ID_MODULE, "MT_MODULE",   MT_NULL);
    s32Ret |= mt_drv_module_register(MT_ID_LOG,    "MT_LOG",      MT_NULL);
    s32Ret |= mt_drv_module_register(MT_ID_PROC,   "MT_PROC",     MT_NULL);
    s32Ret |= mt_drv_module_register(MT_ID_STAT,   "MT_STAT",     MT_NULL);
    s32Ret |= mt_drv_module_register(MT_ID_MEM,    "MT_MEM",      MT_NULL);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_MODULE("Reg basic module err:%#x.\n", s32Ret);
    }

    mt_osal_snprintf(g_stModuleDev.devfs_name, sizeof(g_stModuleDev.devfs_name), "%s", UMAP_DEVNAME_MODULE);
    g_stModuleDev.fops = &drv_module_fops;
    g_stModuleDev.minor = UMAP_MIN_MINOR_MODULE;
    g_stModuleDev.owner  = THIS_MODULE;
    g_stModuleDev.drvops = NULL;

    if(mt_drv_dev_register(&g_stModuleDev) < 0)
    {
        MT_ERR_MODULE("Unable to register dbg dev\n");
        return MT_FAILURE;
    }
    // 1
    item = mt_drv_proc_add_module(MT_MOD_MODULE, NULL, NULL);
    if (! item)
    {
        mt_drv_dev_unregister(&g_stModuleDev);
        return MT_FAILURE;
    }

#ifdef MODULE_PROC_SUPPORT
    //proc read and write interface.
    item->read = module_proc_read;
    item->write = module_proc_write;
#endif
    g_ModuleModInit = 1;

    return MT_SUCCESS;
}
#endif

mt_void mt_drv_mmngr_exit(mt_void)
{
    modulemgr_link_deInit();

    return;
}

#if 0
mt_void mmngr_drv_modexit(mt_void)
{
    if(g_ModuleModInit == 0)
    {
        return;
    }

#ifdef CMN_MMGR_SUPPORT
    mt_drv_module_unregister(MT_ID_SYS);
    mt_drv_module_unregister(MT_ID_MEM);
    mt_drv_module_unregister(MT_ID_LOG);
    mt_drv_module_unregister(MT_ID_MODULE);
    mt_drv_module_unregister(MT_ID_PROC);
    mt_drv_module_unregister(MT_ID_STAT);

    kmodule_memmgr_exit();
#endif

    g_ModuleModInit = 0;

#ifdef MODULE_PROC_SUPPORT
    mt_drv_proc_rm_module(MT_MOD_MODULE);
    mt_drv_dev_unregister(&g_stModuleDev);
#endif

    return;
}
#endif



struct mmngr_drv_device {
	struct device *dev;
	void *platdata;
	dev_t devt;
	int id;
};

struct mmngr_drv_driver {
	struct cdev cdev;
	dev_t devt;
	dev_t major;
	dev_t minor;
	u32 minors;
};

static struct mmngr_drv_driver *mmngr_drv_drv;

static int mmngr_drv_probe(struct platform_device *pdev)
{
	mt_proc_entry_t *item = NULL;
	struct mmngr_drv_device *mmngr_drv_dev = NULL;
	int ret = 0;

    if (g_ModuleModInit) {
        return MT_SUCCESS;
    }

	mmngr_drv_dev = kzalloc(sizeof(struct mmngr_drv_device), GFP_KERNEL);
	if (!mmngr_drv_dev) {
		pr_err("Error kzalloc mmngr_drv_dev\n\n");
		ret = -ENOMEM;
		goto fail_kzalloc_mmngr_drv_dev;
	}
	mmngr_drv_dev->id = pdev->id;
	mmngr_drv_dev->devt = mmngr_drv_drv->devt + mmngr_drv_dev->id;
	mmngr_drv_dev->platdata = dev_get_platdata(&pdev->dev);

#ifdef CMN_MMGR_SUPPORT
    kmodule_memmgr_init(
    	((struct moduleCount_moduleMemCount *)(mmngr_drv_dev->platdata))->u32ModuleCount,
		((struct moduleCount_moduleMemCount *)(mmngr_drv_dev->platdata))->u32ModuleMemCount);
#endif

    ret = mt_drv_module_register(MT_ID_SYS,    "MT_SYS",      MT_NULL);
    ret |= mt_drv_module_register(MT_ID_MODULE, "MT_MODULE",   MT_NULL);
    ret |= mt_drv_module_register(MT_ID_LOG,    "MT_LOG",      MT_NULL);
    ret |= mt_drv_module_register(MT_ID_PROC,   "MT_PROC",     MT_NULL);
    ret |= mt_drv_module_register(MT_ID_STAT,   "MT_STAT",     MT_NULL);
    ret |= mt_drv_module_register(MT_ID_MEM,    "MT_MEM",      MT_NULL);
    if (MT_SUCCESS != ret) {
        MT_ERR_MODULE("Reg basic module err:%#x.\n", ret);
    }

#if 0
	mmngr_drv_dev->dev = device_create(mt_class, &pdev->dev, mmngr_drv_dev->devt, NULL, UMAP_DEVNAME_MODULE);
#else
	mmngr_drv_dev->dev = device_create(mt_class, NULL, mmngr_drv_dev->devt, NULL, UMAP_DEVNAME_MODULE);
#endif
	if (IS_ERR(mmngr_drv_dev->dev)) {
		pr_err("Error device_create\n\n");
		ret = PTR_ERR(mmngr_drv_dev->dev);
		goto fail_device_create;
	}

	platform_set_drvdata(pdev, mmngr_drv_dev);
	dev_set_drvdata(mmngr_drv_dev->dev, mmngr_drv_dev);

    item = mt_drv_proc_add_module(MT_MOD_MODULE, NULL, NULL);
    if (!item) {
        goto fail_mt_drv_proc_add_module;;
    }

#ifdef MODULE_PROC_SUPPORT
    //proc read and write interface.
    item->read = module_proc_read;
    item->write = module_proc_write;
#endif
    g_ModuleModInit = 1;

	return 0;

fail_mt_drv_proc_add_module:
	device_destroy(mt_class, mmngr_drv_dev->devt);
fail_device_create:
	kfree(mmngr_drv_dev);
	mmngr_drv_dev = NULL;
fail_kzalloc_mmngr_drv_dev:
	return ret;
}

static int mmngr_drv_remove(struct platform_device *pdev)
{
	struct mmngr_drv_device *mmngr_drv_dev = platform_get_drvdata(pdev);

    if (g_ModuleModInit == 0) {
        return 0;
    }

    mt_drv_module_unregister(MT_ID_SYS);
    mt_drv_module_unregister(MT_ID_MEM);
    mt_drv_module_unregister(MT_ID_LOG);
    mt_drv_module_unregister(MT_ID_MODULE);
    mt_drv_module_unregister(MT_ID_PROC);
    mt_drv_module_unregister(MT_ID_STAT);
#ifdef CMN_MMGR_SUPPORT
	kmodule_memmgr_exit();
#endif

    g_ModuleModInit = 0;

#ifdef MODULE_PROC_SUPPORT
    mt_drv_proc_rm_module(MT_MOD_MODULE);
#endif

	device_destroy(mt_class, mmngr_drv_dev->devt);
	mmngr_drv_dev->dev = NULL;

	platform_set_drvdata(pdev, NULL);

	if (mmngr_drv_dev) {
		kfree(mmngr_drv_dev);
		mmngr_drv_dev = NULL;
	}

	return 0;
}

static int mmngr_drv_suspend(struct platform_device *pdev, pm_message_t stState)
{
	return 0;
}

static int mmngr_drv_resume(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver mmngr_drv_platform_driver = {
	.probe = mmngr_drv_probe,
	.remove = mmngr_drv_remove,
	.suspend = mmngr_drv_suspend,
	.resume = mmngr_drv_resume,
	.driver = {
		.name = UMAP_DEVNAME_MODULE,
		.owner = THIS_MODULE,
	}
};

static void mmngr_drv_release_device(struct device *pdev) {  }

static struct moduleCount_moduleMemCount moduleCount_moduleMemCount0;
static struct platform_device mmngr_drv_platform_device = {
	.name = UMAP_DEVNAME_MODULE,
	.id = 0,
	.dev= {
		.platform_data = NULL,
		.release = mmngr_drv_release_device,
	},
};

int __init mmngr_drv_modinit(mt_u32 u32ModuleCount, mt_u32 u32ModuleMemCount)
{
	int ret;

	mmngr_drv_drv = kzalloc(sizeof(struct mmngr_drv_driver), GFP_KERNEL);
	if (!mmngr_drv_drv) {
		pr_err("Error kzalloc mmngr_drv_drv\n\n");
		ret = -ENOMEM;
		goto fail_kzalloc_mmngr_drv_drv;
	}

	mmngr_drv_drv->major = MT_DEVICE_MAJOR;
	mmngr_drv_drv->minor = UMAP_MIN_MINOR_MODULE;
	mmngr_drv_drv->minors = UMAP_DEV_NUM_MODULE;
	mmngr_drv_drv->devt = MKDEV(mmngr_drv_drv->major, mmngr_drv_drv->minor);
	cdev_init(&mmngr_drv_drv->cdev, &drv_module_fops);
	mmngr_drv_drv->cdev.owner = THIS_MODULE;

	ret = cdev_add(&mmngr_drv_drv->cdev, mmngr_drv_drv->devt, mmngr_drv_drv->minors);
	if (ret) {
		pr_err("Error cdev_add\n\n");
		ret = -EINVAL;
		goto fail_cdev_add;
	}

	ret = platform_driver_register(&mmngr_drv_platform_driver);
	if (ret) {
		pr_err("Error platform_driver_register\n\n");
		goto fail_platform_driver_register;
	}

	mmngr_drv_platform_device.dev.platform_data = &moduleCount_moduleMemCount0;
	moduleCount_moduleMemCount0.u32ModuleCount = u32ModuleCount;
	moduleCount_moduleMemCount0.u32ModuleMemCount = u32ModuleMemCount;
	ret = platform_device_register(&mmngr_drv_platform_device);
	if (ret) {
		pr_err("Error platform_device_register\n\n");
		goto fail_platform_device_register;
	}

	return 0;

fail_platform_device_register:
	platform_driver_unregister(&mmngr_drv_platform_driver);
fail_platform_driver_register:
	cdev_del(&mmngr_drv_drv->cdev);
fail_cdev_add:
	kfree(mmngr_drv_drv);
	mmngr_drv_drv = NULL;
fail_kzalloc_mmngr_drv_drv:
	return ret;
}

void mmngr_drv_modexit(void)
{
	platform_driver_unregister(&mmngr_drv_platform_driver);
	platform_device_unregister(&mmngr_drv_platform_device);
	kfree(mmngr_drv_drv);
	mmngr_drv_drv = NULL;
}

#ifdef CMN_MMGR_SUPPORT
mt_u8* mt_drv_module_getname_byid(mt_u32 u32ModuleID)
{
    module_s* pItr = NULL;
    mt_u32    u32Count = 0;
    mt_u32    u32Total = 0;

    kmodule_mgr_lock(NULL);

    if (NULL == g_pstKModuleHeader)
    {
        kmodule_mgr_unlock();

        return NULL;
    }

    pItr = g_pstKModuleHeader->pNextModule;
    u32Total = g_pstKModuleHeader->u32ItemCnt;

    while (NULL != pItr)
    {
        u32Count++;
        if (pItr->stModuleInfo.u32ModuleID == u32ModuleID)
        {
            break;
        }

        if (u32Count > u32Total)
        {
            pItr = NULL;
            break;
        }

        pItr = pItr->pNextModule;
    }

    if (NULL != pItr)
    {
        kmodule_mgr_unlock();

        return pItr->stModuleInfo.u8ModuleName;
    }

    kmodule_mgr_unlock();

    return NULL;
}

mt_u32 mt_drv_module_getid_byname(mt_u8* pu8Name)
{
    module_s* pItr = NULL;

    if(pu8Name == NULL)
    {
        return 0;
    }

    kmodule_mgr_lock(0);

    if (NULL == g_pstKModuleHeader)
    {
        kmodule_mgr_unlock();

        return 0;
    }

    pItr = g_pstKModuleHeader->pNextModule;

    while (NULL != pItr)
    {
        if (memcmp(pItr->stModuleInfo.u8ModuleName, pu8Name, strlen(pu8Name)) == 0)
        {
            break;
        }

        pItr = pItr->pNextModule;
    }

    if (NULL != pItr)
    {
        kmodule_mgr_unlock();

        return pItr->stModuleInfo.u32ModuleID;
    }

    kmodule_mgr_unlock();

    return 0;
}
#endif

static mt_s32 module_drv_register(mt_u32 u32ModuleID,
                        const mt_u8* pu8ModuleName, mt_void* pFunc, struct file * file)
{
    module_s stModule = {{0}};
    mt_u32   u32MinLen = 0;
    mt_s32   s32Ret = MT_FAILURE;

    MT_INFO_MODULE("Register %d %s %p\n", u32ModuleID, pu8ModuleName, file);

    if ( NULL == pu8ModuleName )
    {
        return MT_FAILURE;
    }

    //request ID by name
    //u32ModuleID = ModuleMgr_RegisterID(pu8ModuleName);

    //build module node;
    stModule.pNextModule = NULL;
    stModule.stModuleInfo.u32ModuleID = u32ModuleID;
    u32MinLen = MIN_LEN( sizeof(stModule.stModuleInfo.u8ModuleName)-1, strlen(pu8ModuleName));
    memcpy(stModule.stModuleInfo.u8ModuleName, pu8ModuleName, u32MinLen);

    stModule.stModuleInfo.pFnCallback = pFunc;
    stModule.stModuleInfo.file = (ulong)file;

    // Add a node to into the link.
    kmodule_mgr_lock(MT_FAILURE);

    s32Ret = modulemgr_link_addnode(g_pstKModuleHeader, &stModule);
    kmodule_mgr_unlock();

    if (MT_SUCCESS == s32Ret)
    {
        //KModule_Log_Pool_AddNode(u32ModuleID);

#ifdef CMN_MMGR_SUPPORT
        kmodule_mem_pool_add_module(u32ModuleID);
#endif
        log_add_module((mt_pchar)pu8ModuleName, (mt_mod_id_e)u32ModuleID);
    }

    MT_INFO_MODULE("add module:%s, id %#x!\n", stModule.stModuleInfo.u8ModuleName, stModule.stModuleInfo.u32ModuleID);

    return s32Ret;
}

mt_s32 mt_drv_module_register(mt_u32 u32ModuleID, const mt_u8* pu8ModuleName, mt_void* pFunc)
{
    return module_drv_register(u32ModuleID, pu8ModuleName, pFunc, MT_NULL);
}

mt_s32 mt_drv_module_allocid(mt_u8* pu8ModuleName, mt_u32 *pu32ModuleID, mt_s32 *ps32Status)
{
    module_s *pstFindModude = MT_NULL;
    mt_s32   s32Ret = MT_FAILURE;
    mt_char  aszModuleName[MAX_MODULE_NAME+12];

    if ( NULL == pu8ModuleName )
    {
        return MT_FAILURE;
    }

    mt_osal_snprintf(aszModuleName, sizeof(aszModuleName), "%s_%d", pu8ModuleName, current->pid);

    kmodule_mgr_lock(MT_FAILURE);

    /* Find module */
    s32Ret = modulemgr_findnode_byname(g_pstKModuleHeader, &pstFindModude, aszModuleName);
    if ((MT_SUCCESS == s32Ret) && (MT_NULL != pstFindModude))
    {
        pstFindModude->stModuleInfo.s32RegCount++;
        kmodule_mgr_unlock();
        MT_INFO_MODULE("Alloc again, reg count=%d\n", pstFindModude->stModuleInfo.s32RegCount);
        mt_osal_strncpy(pu8ModuleName, aszModuleName, MAX_MODULE_NAME+12-1);
        *pu32ModuleID = pstFindModude->stModuleInfo.u32ModuleID;
        *ps32Status = 1;
        return MT_SUCCESS;
    }

    /* Alloc module ID */
    *pu32ModuleID = modulemgr_alloc_moduleid();

    kmodule_mgr_unlock();

    if (MT_INVALID_MODULE_ID == *pu32ModuleID)
    {
        return MT_FAILURE;
    }

    *ps32Status = 0;
    mt_osal_strncpy(pu8ModuleName, aszModuleName, MAX_MODULE_NAME+12-1);

    MT_INFO_MODULE("AllocId %s, %d\n", pu8ModuleName, *pu32ModuleID);
    return MT_SUCCESS;
}

mt_s32 mt_drv_module_unregister(mt_u32 u32ModuleID)
{
    mt_s32 s32Ret = MT_FAILURE;
    module_s stNode;

    kmodule_mgr_lock(MT_FAILURE);

    stNode.stModuleInfo.u32ModuleID = u32ModuleID;
    log_remove_module(stNode.stModuleInfo.u8ModuleName, u32ModuleID);

    s32Ret = modulemgr_link_delnode(g_pstKModuleHeader, &stNode, MT_FALSE);
    if (MT_FAILURE == s32Ret)
    {
        kmodule_mgr_unlock();

        return MT_FAILURE;
    }

   // KModule_Log_Pool_DelNode(u32ModuleID);

#ifdef CMN_MMGR_SUPPORT
    kmodule_mem_pool_del_module(u32ModuleID);
#endif

    kmodule_mgr_unlock();

    return MT_SUCCESS;
}



mt_s32 mt_drv_module_getfunction(mt_u32 u32ModuleID, mt_void** ppFunc)
{
	unsigned long flags;
    mt_s32 s32Ret = MT_FAILURE;
    module_s* pMoudle = NULL;

    if (ppFunc == NULL)
    {
        MT_ERR_MODULE("param invalid!\n");

        return MT_FAILURE;
    }

    spin_lock_irqsave(&g_KModuleMgr_spinlock, flags);

    s32Ret = modulemgr_findnode_byid(g_pstKModuleHeader, &pMoudle, u32ModuleID);

    spin_unlock_irqrestore(&g_KModuleMgr_spinlock, flags);

    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_MODULE("Not found module ID: 0x%08x\n", u32ModuleID);

        return MT_FAILURE;
    }

    *ppFunc = pMoudle->stModuleInfo.pFnCallback;

    return MT_SUCCESS;
}



EXPORT_SYMBOL(mt_drv_mmngr_init);
EXPORT_SYMBOL(mt_drv_mmngr_exit);

EXPORT_SYMBOL(mt_drv_module_register);
EXPORT_SYMBOL(mt_drv_module_unregister);


#ifdef CMN_MMGR_SUPPORT
EXPORT_SYMBOL(mt_drv_module_getname_byid);
EXPORT_SYMBOL(mt_drv_module_getid_byname);
#endif

EXPORT_SYMBOL(mt_drv_module_getfunction);

