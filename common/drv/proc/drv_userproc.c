/******************************************************************************


******************************************************************************/

/******************************* Include Files *******************************/

/* Sys headers */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/mman.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/list.h>
#include <linux/rbtree.h>
#include <linux/version.h>
#include <asm/atomic.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
#else
#include "../../fs/proc/internal.h"
#endif
/* Unf headers */
#include "mt_module.h"
#include "mt_debug.h"
#include "mt_common.h"
#include "mt_osal.h"
/* Drv headers */
#include "drv_userproc_ioctl.h"
#include "mt_drv_userproc.h"
#include "mt_drv_proc.h"
#include "mt_kernel_adapt.h"
#include "mt_drv_mem.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#if !(0 == MT_PROC_SUPPORT)


/***************************** Macro Definition ******************************/

#define UPROC_K_LOCK(sema) do {down(&sema);}while(0)
#define UPROC_K_UNLOCK(sema) do {up(&sema);} while (0)

/*************************** Structure Definition ****************************/

typedef struct
{
    mt_u32                  entry_name_hash;
    struct rb_node       node;
    struct proc_dir_entry *parent;
    mt_drv_u_proc_entry_t stInfo;
    mt_char                entry_name[MAX_PROC_NAME_LEN+1];
}mt_priv_proc_entry_t;

typedef struct
{
    mt_u32                  dir_name_hash;
    struct rb_node	      node;
    struct rb_root        entry_root;
    struct file *           pstFile;
    struct proc_dir_entry *entry;
    struct proc_dir_entry *parent;
    mt_char                dir_name[MAX_PROC_NAME_LEN+12];   /* '_' 1 and pid 10 */
}mt_proc_dir_t;

typedef struct
{
    struct semaphore stSem; /* Semaphore */
    struct rb_root        root;
    wait_queue_head_t wq_for_read;
    wait_queue_head_t wq_for_write;
    int busy;
    mt_drv_u_proc_cmd_t current_cmd;
    atomic_t atmOpenCnt;
}mt_priv_proc_param_t;

/***************************** Global Definition *****************************/

extern struct proc_dir_entry *gp_mcomm_proc;
extern struct proc_dir_entry *gp_msp_proc;

/***************************** Static Definition *****************************/

static mt_priv_proc_param_t g_stUProcParam =
{
    .root           = RB_ROOT,
    .atmOpenCnt = ATOMIC_INIT(0),
};

static mt_proc_dir_t *g_pst_comm_dirent = MT_NULL, *g_pst_msp_dirent = MT_NULL;

static mt_void mt_proc_drv_rm_dir_by_force(mt_proc_dir_t *pstDir);
static mt_void mt_proc_drv_rm_entry(mt_proc_dir_t *pstDir, mt_priv_proc_entry_t * pstEntry);

/*********************************** Code ************************************/

static mt_proc_dir_t * mt_rb_tree_find_dirent(mt_proc_dir_t * pszParent, const mt_char * pszName)
{
    mt_u32 hash = full_name_hash(NULL, pszName, strlen(pszName))  & 0x7fffffffU;
    struct rb_node *node = g_stUProcParam.root.rb_node;

    while(node)
    {
        mt_s32 result;
        mt_proc_dir_t* this = rb_entry(node, mt_proc_dir_t, node);

        if (hash != this->dir_name_hash)
        {
            result = hash - this->dir_name_hash;
        }
        else
        {
            result = mt_osal_strncmp(pszName, this->dir_name, sizeof(this->dir_name));
        }

        if( result < 0)
        {
             node = node->rb_left;
        }
        else if(result > 0)
        {
            node = node->rb_right;
        }
        else
        {
            return this;
        }
    }

    return MT_NULL;
}

static mt_s32 mt_rb_tree_insert_dirent(mt_proc_dir_t * pszParent, mt_proc_dir_t*pszDirent)
{
    struct rb_root *root = &g_stUProcParam.root;
    struct rb_node **new = &(root->rb_node), *parent = NULL;

    while(*new)
    {
        mt_s32 result;
        mt_proc_dir_t *this = rb_entry(*new, mt_proc_dir_t, node);
        parent = *new;

        if (pszDirent->dir_name_hash != this->dir_name_hash)
        {
            result = pszDirent->dir_name_hash - this->dir_name_hash;
        }
        else
        {
            result =  mt_osal_strncmp(pszDirent->dir_name, this->dir_name, sizeof(pszDirent->dir_name));
        }

        if (result < 0)
        {
            new = &((*new)->rb_left);
        }
        else if (result > 0)
        {
            new = &((*new)->rb_right);
        }
        else
        {
            PROC_ERR("dirent(%s) has existed.", pszDirent->dir_name);
            return MT_FAILURE;
        }
    }

    rb_link_node(&pszDirent->node, parent, new);
    rb_insert_color(&pszDirent->node, root);

    return MT_SUCCESS;
}

static void mt_rb_tree_erase_dirent(mt_proc_dir_t * pszParent, mt_proc_dir_t*pszDirent)
{
    struct rb_root *root = &g_stUProcParam.root;

    rb_erase(&(pszDirent->node), root);
}

static mt_priv_proc_entry_t * mt_rb_tree_find_entry(mt_proc_dir_t * pstDir, const mt_char * pszName)
{
    mt_u32 hash = full_name_hash(NULL, pszName, strlen(pszName))  & 0x7fffffffU;
    struct rb_node *node = pstDir->entry_root.rb_node;

    while(node)
    {
        mt_s32 result;
        mt_priv_proc_entry_t* this = rb_entry(node, mt_priv_proc_entry_t, node);

        if (hash != this->entry_name_hash)
        {
            result = hash - this->entry_name_hash;
        }
        else
        {
            result = mt_osal_strncmp(pszName, this->entry_name, sizeof(this->entry_name));
        }

        if( result < 0)
        {
             node = node->rb_left;
        }
        else if(result > 0)
        {
            node = node->rb_right;
        }
        else
        {
            return this;
        }
    }

    return MT_NULL;
}

static mt_s32 mt_rb_tree_insert_entry(mt_proc_dir_t * pstDir, mt_priv_proc_entry_t*pszEntry)
{
    struct rb_root *root = &(pstDir->entry_root);
    struct rb_node **new = &(root->rb_node), *parent = NULL;

    while(*new)
    {
        mt_s32 result;
        mt_priv_proc_entry_t *this = rb_entry(*new, mt_priv_proc_entry_t, node);
        parent = *new;

        if (pszEntry->entry_name_hash != this->entry_name_hash)
        {
            result = pszEntry->entry_name_hash - this->entry_name_hash;
        }
        else
        {
            result =  mt_osal_strncmp(pszEntry->entry_name, this->entry_name, sizeof(pszEntry->entry_name));
        }

        if (result < 0)
        {
            new = &((*new)->rb_left);
        }
        else if (result > 0)
        {
            new = &((*new)->rb_right);
        }
        else
        {
            PROC_ERR("entry(%s) has existed.", pszEntry->entry_name);
            return MT_FAILURE;
        }
    }

    rb_link_node(&pszEntry->node, parent, new);
    rb_insert_color(&pszEntry->node, root);

    return MT_SUCCESS;
}

static void mt_rb_tree_erase_entry(mt_proc_dir_t * pstDir, mt_priv_proc_entry_t*pszEntry)
{
    struct rb_root *root = &(pstDir->entry_root);

    rb_erase(&(pszEntry->node), root);
}

static mt_priv_proc_entry_t * mt_rb_tree_find_proc_entry(struct proc_dir_entry* pstPde)
{
    mt_proc_dir_t * pstDir;
    mt_priv_proc_entry_t *pstEntry;

    if ( !pstPde || !strlen(pstPde->parent->name) || !strlen(pstPde->name) )
    {
        PROC_ERR("invalid proc entry.");
        goto out;
    }

    pstDir = mt_rb_tree_find_dirent(MT_NULL, pstPde->parent->name);
    if (!pstDir)
    {
        PROC_ERR("Can't find dirent:%p\n", pstPde->parent->name);
        goto out;
    }

    pstEntry = mt_rb_tree_find_entry(pstDir, pstPde->name);
    if(!pstEntry)
    {
        PROC_ERR("Can't find entry:%p\n", pstPde->name);
        goto out;
    }

    return pstEntry;

out:
    return MT_NULL;
}

static int mt_usrmodeproc_open(struct inode *inode, struct file *file)
{
    PROC_INFO("Open User Mode Proc:%s,%d\n", current->comm, current->pid);

    if (atomic_inc_return(&g_stUProcParam.atmOpenCnt) == 1)
    {
        memset(&g_stUProcParam.current_cmd, 0, sizeof(mt_drv_u_proc_cmd_t));
        init_waitqueue_head(&g_stUProcParam.wq_for_read);
        init_waitqueue_head(&g_stUProcParam.wq_for_write);
        g_stUProcParam.busy = 0;
        MT_INIT_MUTEX(&g_stUProcParam.stSem);
    }

    file->private_data = &g_stUProcParam;
    return 0;
}

static int mt_usrmodeproc_close(struct inode *inode, struct file *file)
{
    struct rb_node *node;

    if (atomic_dec_return(&g_stUProcParam.atmOpenCnt) >= 0)
    {
        UPROC_K_LOCK(g_stUProcParam.stSem);

scratch_dirent:
        for(node = rb_first(&(g_stUProcParam.root));node; node = rb_next(node))
        {
            mt_proc_dir_t * dirent = rb_entry(node, mt_proc_dir_t, node);

            if (file == dirent->pstFile)
            {
                mt_proc_drv_rm_dir_by_force(dirent);
                goto scratch_dirent;
            }
            else if (MT_NULL == dirent->pstFile) /* for files /proc/msp/xxx or /proc/mcomm/xxx */
            {
                struct rb_node * entry_node;

scratch_entry:
                for(entry_node = rb_first(&(dirent->entry_root));entry_node;entry_node = rb_next(entry_node))
                {
                    mt_priv_proc_entry_t * entry = rb_entry(entry_node, mt_priv_proc_entry_t, node);
                    if (file == entry->stInfo.pFile)
                    {
                        mt_proc_drv_rm_entry(dirent, entry);
                        goto scratch_entry;
                    }
                }
            }
        }

        UPROC_K_UNLOCK(g_stUProcParam.stSem);
    }

    PROC_INFO("Close User Mode Proc\n" );
    return 0;
}

static int mt_proc_seq_show(struct seq_file *m, void *unused)
{

    struct proc_dir_entry* pstPde = (struct proc_dir_entry *)(m->private);
    mt_priv_proc_param_t *proc = ((struct proc_dir_entry *)(m->private))->data;
    mt_priv_proc_entry_t *pstEntry;
    mt_s32 ret;
    DEFINE_WAIT(wait);

    UPROC_K_LOCK(g_stUProcParam.stSem);

    pstEntry = mt_rb_tree_find_proc_entry(pstPde);
    if(!pstEntry)
    {
        PROC_ERR("Can't find entry:%p\n", pstPde->name);
        ret = -1;
        goto out;
    }
	else if (!pstEntry->stInfo.pfnShowFunc)
	{
		PROC_ERR("Entry don't support read.\n");
		ret = -1;
		goto out;
	}

    proc->current_cmd.pEntry = &(pstEntry->stInfo);
    mt_osal_strncpy(proc->current_cmd.aszCmd, MT_UPROC_READ_CMD, sizeof(proc->current_cmd.aszCmd) - 1);

    UPROC_K_UNLOCK(g_stUProcParam.stSem);

    /* Wait write data over */
    prepare_to_wait(&proc->wq_for_read, &wait, TASK_INTERRUPTIBLE);
    schedule();
    finish_wait(&proc->wq_for_read, &wait);

    /* Find it again, pstEntry may be removed when wait event */
    UPROC_K_LOCK(g_stUProcParam.stSem);

    pstEntry = mt_rb_tree_find_proc_entry(pstPde);
    if(!pstEntry)
    {
        PROC_ERR("Can't find entry:%p\n", pstPde->name);
        ret = -1;
        goto out;
    }
    else if (MT_NULL != pstEntry->stInfo.pfnShowFunc)
    {
        PROC_INFO("User Mode Proc Show entry=0x%p, proc=0x%p\n", pstPde, proc);
        PROC_PRINT(m, "%s", (mt_char*)pstEntry->stInfo.stBuf.startVirAddr);
    }

    ret = 0;

 out:
    UPROC_K_UNLOCK(g_stUProcParam.stSem);

    return ret;

}

static int mt_proc_seq_open(struct inode *inode, struct file *file)
{
    mt_priv_proc_param_t *proc = PDE(inode)->data;
    int res;

    PROC_INFO("mt_proc_seq_open 0x%p,%d\n", proc, proc->busy );

    if (proc->busy)
        return -EAGAIN;

    proc->busy = 1;

    res = single_open(file, mt_proc_seq_show, PDE(inode));

    if( res )
        proc->busy = 0;

    return res;
}

static int mt_proc_seq_release(struct inode *inode, struct file *file)
{
    mt_priv_proc_param_t *proc = PDE(inode)->data;

    PROC_INFO("mt_proc_seq_release %d\n", proc->busy );

    proc->busy = 0;
    return single_release(inode, file);
}

static mt_s32 mt_strip_string(mt_char *string, mt_u32 size)
{
    mt_char * p = string;
    mt_u32      index = 0;

    if (!string ||  0 ==  size)
        return MT_FAILURE;

    /* strip '\n' as string end character */
    for (; index < size; index++)
    {
        if ( '\n' == *(p + index) )
        {
            *(p + index) = '\0';
        }
    }

    if (strlen(string))
        return MT_SUCCESS;
    else
        return MT_FAILURE;
}

static ssize_t mt_proc_seq_write (struct file *file, const char __user *buf, size_t size, loff_t *pos)
{
    struct proc_dir_entry* pstPde = PDE(file->f_path.dentry->d_inode);
    mt_priv_proc_param_t *proc = pstPde->data;
    mt_priv_proc_entry_t *pstEntry ;
    mt_s32 ret;

    DEFINE_WAIT(wait);

    UPROC_K_LOCK(g_stUProcParam.stSem);

    pstEntry = mt_rb_tree_find_proc_entry(pstPde);
    if(!pstEntry ||  size > sizeof(proc->current_cmd.aszCmd))
    {
        PROC_ERR("Can't find entry:%p\n", pstPde->name);
        ret = -1;
        goto out;
    }
    else if (MT_NULL == pstEntry->stInfo.pfnCmdFunc)
    {
        PROC_ERR("Entry don't support write.\n");
        ret = -1;
        goto out;
    }

    memset(proc->current_cmd.aszCmd, 0, sizeof(proc->current_cmd.aszCmd));
    if (copy_from_user(proc->current_cmd.aszCmd, buf, size))
    {
        PROC_ERR("get cmd failed.");
        ret = -EIO;
        goto out;
    }
    proc->current_cmd.aszCmd[size > 1 ? size - 1 : 0] = '\0';

    if (MT_FAILURE == mt_strip_string(proc->current_cmd.aszCmd, size))
    {
        PROC_WARN("echo string invalid.");
        UPROC_K_UNLOCK(g_stUProcParam.stSem);
        ret =  -EINVAL;
        goto out;
    }

    proc->current_cmd.pEntry = &(pstEntry->stInfo);

    UPROC_K_UNLOCK(g_stUProcParam.stSem);

    /* Wait write data over */
    prepare_to_wait(&proc->wq_for_write, &wait, TASK_INTERRUPTIBLE);
    schedule();
    finish_wait(&proc->wq_for_write, &wait);

    UPROC_K_LOCK(g_stUProcParam.stSem);

    /* if buffer not empty , try echo to current terminal */
    pstEntry = mt_rb_tree_find_proc_entry(pstPde);
    if ( MT_NULL != pstEntry && pstEntry->stInfo.pfnCmdFunc )
    {
        PROC_INFO( "mt_proc_seq_write: proc=%p, entry=%p %d bytes\n", proc, pstPde, size);
        if (strlen( (mt_char*)pstEntry->stInfo.stBuf.startVirAddr))
        {
            mt_drv_proc_echohelp((mt_char*)pstEntry->stInfo.stBuf.startVirAddr);
        }
    }

    UPROC_K_UNLOCK(g_stUProcParam.stSem);

    return size;

out:
    UPROC_K_UNLOCK(g_stUProcParam.stSem);

    return ret;
}


mt_proc_dir_t * mt_proc_drv_add_dir(const mt_char* pszName, const mt_char* pszParent, struct file *pstFile)
{
    mt_proc_dir_t * pstDir;

    /* Check parameter */
    if ((MT_NULL == pszName) || (strlen(pszName) == 0) || (strlen(pszName) > MAX_PROC_NAME_LEN))
    {
        PROC_ERR("Invalid name\n");
        return MT_NULL;
    }

    /* Find directory node, if exist, return success directlly */
    pstDir = mt_rb_tree_find_dirent(MT_NULL, pszName);
    if (MT_NULL != pstDir)
    {
        PROC_INFO("Dir %s exist\n", pszName);
        return pstDir;
    }

    /* Alloc directory resource */
    pstDir = MT_KMALLOC(MT_ID_PROC, sizeof(mt_proc_dir_t), GFP_KERNEL);
    if (MT_NULL == pstDir)
    {
        PROC_ERR("kmalloc fail\n");
        return MT_NULL;
    }

    /* Init directory parameter */
    mt_osal_snprintf(pstDir->dir_name, sizeof(pstDir->dir_name), "%s", pszName);
    pstDir->dir_name_hash = full_name_hash(NULL, pszName, strlen(pszName)) & 0x7fffffffU;
    pstDir->entry_root = RB_ROOT;
    pstDir->parent = MT_NULL;
    pstDir->pstFile = pstFile;

    /* Make proc directory */
    pstDir->entry = proc_mkdir(pstDir->dir_name, gp_mcomm_proc);
    if (MT_NULL == pstDir->entry)
    {
        PROC_ERR("proc_mkdir fail\n");
        goto out1;
    }

    PROC_INFO("Proc add dir %s, file=%p, entry=0x%p\n", pstDir->dir_name, pstFile, pstDir->entry);

    /* Add directory to rbtree */
    if (MT_SUCCESS != mt_rb_tree_insert_dirent(MT_NULL, pstDir))
    {
        PROC_ERR("Insert new dirent failed.\n");
        goto out2;
    }

    return pstDir;

 out2:
    remove_proc_entry(pstDir->dir_name, gp_mcomm_proc);

 out1:
    MT_KFREE(MT_ID_PROC, pstDir);

    return MT_NULL;
}

mt_s32 mt_proc_drv_rm_dir(mt_proc_dir_t * pstDir)
{
    /* Check parameter */
    if (MT_NULL == pstDir)
    {
        PROC_ERR("Invalid name\n");
        return MT_FAILURE;
    }

    /* If there are entries in this directory, remove fail */
    if (pstDir->entry_root.rb_node)
    {
        PROC_ERR("dir %s non-null\n", pstDir->dir_name);
        return MT_FAILURE;
    }

    /* Remove proc directory */
    remove_proc_entry(pstDir->dir_name, gp_mcomm_proc);

    PROC_INFO("Proc remove dir %s\n", pstDir->dir_name);

    /* Remove directory from rbtree */
    mt_rb_tree_erase_dirent(MT_NULL, pstDir);

    /* Free directory resource */
    MT_KFREE(MT_ID_PROC, pstDir);

    return MT_SUCCESS;
}

mt_s32 mt_proc_drv_rm_dir_by_name(const mt_char* pszName)
{
    mt_char aszDir[MAX_PROC_NAME_LEN+12];
    mt_proc_dir_t * pstDir = MT_NULL;

    /* Check parameter */
    if ((MT_NULL == pszName) || (strlen(pszName) == 0) ||  (strlen(pszName) > MAX_PROC_NAME_LEN))
    {
        PROC_ERR("Invalid name\n");
        return MT_FAILURE;
    }

    /* Make directory name */
    mt_osal_snprintf(aszDir, sizeof(aszDir), "%s", pszName);

    /* Find directory node */
    pstDir = mt_rb_tree_find_dirent(MT_NULL, aszDir);
    if (MT_NULL == pstDir )
    {
        PROC_ERR("Find dir %s fail\n", aszDir);
        return MT_FAILURE;
    }

    return mt_proc_drv_rm_dir(pstDir);
}

mt_proc_dir_t * mt_proc_drv_add_private_dir(const mt_char* pszName, struct proc_dir_entry *pstEntry)
{
    mt_proc_dir_t * pstDir;

    /* Check parameter */
    if ((MT_NULL == pszName) || (strlen(pszName) > MAX_PROC_NAME_LEN) || (MT_NULL == pstEntry))
    {
        goto out;
    }

    /* Alloc directory resource */
    pstDir = MT_KMALLOC(MT_ID_PROC, sizeof(mt_proc_dir_t), GFP_KERNEL);
    if (MT_NULL == pstDir)
    {
        PROC_ERR("kmalloc fail\n");
        goto out;
    }

    /* Init other parameter */
    mt_osal_strncpy(pstDir->dir_name, pszName, sizeof(pstDir->dir_name)-1);

    pstDir->dir_name_hash = full_name_hash(NULL, pszName, strlen(pszName)) & 0x7fffffffU;
    pstDir->entry_root = RB_ROOT;
    pstDir->entry = pstEntry;
    pstDir->parent = MT_NULL;
    pstDir->pstFile = MT_NULL;

    /* Add directory to rbtree */
    if (MT_SUCCESS != mt_rb_tree_insert_dirent(MT_NULL, pstDir))
    {
        PROC_ERR("Insert new dirent failed.\n");
        goto out1;
    }

    return pstDir;

out1:

    MT_KFREE(MT_ID_PROC, pstDir);

out:

    return MT_NULL;
}

mt_s32 mt_proc_drv_rm_private_dir(const mt_char* pszName)
{
    mt_proc_dir_t * pstDir = MT_NULL;

    /* Check parameter */
    if (MT_NULL == pszName)
    {
        return MT_FAILURE;
    }

    /* Find directory node */
    pstDir = mt_rb_tree_find_dirent(MT_NULL, pszName);
    if (MT_NULL == pstDir)
    {
        PROC_ERR("Find dir %s fail\n", pszName);
        return MT_FAILURE;
    }

    /* Remove directory from rbtree */
    mt_rb_tree_erase_dirent(MT_NULL, pstDir);

    /* Free directory resource */
    MT_KFREE(MT_ID_PROC, pstDir);

    return MT_SUCCESS;
}

static struct proc_ops s_proc_seq_fops;

 mt_priv_proc_entry_t* mt_proc_drv_add_entry(const mt_drv_u_proc_entry_t* pstParam, int bUsrMode)
{
    mt_s32 ret;
    mt_priv_proc_entry_t * pstEntry = MT_NULL;
    mt_proc_dir_t * pstDir = MT_NULL;
    mt_char aszDir[MAX_PROC_NAME_LEN+12];
    mt_char aszMMZName[32];
    mt_u32 u32EntryLen;

    /* Check parameter */
    if (MT_NULL == pstParam)
    {
        return MT_NULL;
    }

    u32EntryLen = strlen(pstParam->aszName);
    if ((0 == u32EntryLen) || (u32EntryLen > MAX_PROC_NAME_LEN))
    {
        PROC_ERR("Invalid name\n");
        return MT_NULL;
    }

    /* Make parent directory name */
    if (0 == strlen(pstParam->aszParent))
    {
        strlcpy(aszDir, "mcomm", sizeof(aszDir) - 1);
    }
    else
    {
        mt_osal_snprintf(aszDir, sizeof(aszDir), "%s", pstParam->aszParent);
    }

    /* Find directory node, if don't exist, return fail */
    pstDir = mt_rb_tree_find_dirent(MT_NULL, aszDir);
    if (MT_NULL == pstDir)
    {
        PROC_ERR("Dir %s don't exist\n", pstParam->aszParent);
        goto out;
    }

    /* Find entry in the directory, if exist, return success directlly */
    pstEntry = mt_rb_tree_find_entry(pstDir, pstParam->aszName);
    if (MT_NULL != pstEntry)
    {
        PROC_INFO("Entry %s exist\n", pstParam->aszName);
        goto out;
    }

    /* Alloc entry resource */
    pstEntry = MT_KMALLOC(MT_ID_PROC, sizeof(mt_priv_proc_entry_t), GFP_KERNEL);
    if (MT_NULL == pstEntry)
    {
        PROC_ERR("kmalloc fail\n");
        goto out;
    }
    memset(pstEntry, 0, sizeof(mt_priv_proc_entry_t));

    /* Create proc entry */
    pstEntry->stInfo.pEntry = proc_create(pstParam->aszName, 0, pstDir->entry, &s_proc_seq_fops);
    if (MT_NULL == pstEntry->stInfo.pEntry)
    {
        PROC_FATAL("create_proc_entry fail\n");
        goto out1;
    }

    PROC_INFO("Proc add entry %s, file=%p, entry=0x%p\n", pstParam->aszName, pstParam->pFile, pstEntry->stInfo.pEntry);

    /* Init other parameter */
    mt_osal_strncpy(pstEntry->entry_name, pstParam->aszName, sizeof(pstEntry->entry_name)-1);
    pstEntry->entry_name_hash = full_name_hash(NULL, pstParam->aszName, strlen(pstParam->aszName)) & 0x7fffffffU;
    pstEntry->parent = pstDir->entry;
    pstEntry->stInfo.pFile = pstParam->pFile;
    pstEntry->stInfo.pfnShowFunc = pstParam->pfnShowFunc;
    pstEntry->stInfo.pfnCmdFunc = pstParam->pfnCmdFunc;
    pstEntry->stInfo.pPrivData   = pstParam->pPrivData;

    /* pfnShowFunc need mmz */
    if ( bUsrMode )
    {
        /* Alloc MMZ */
        mt_osal_snprintf(aszMMZName, sizeof(aszMMZName), "CMN_Uproc_%s", pstEntry->entry_name);
        ret = mt_drv_mmz_alloc_and_map(aszMMZName, NULL, MT_PROC_BUFFER_SIZE, 0, &(pstEntry->stInfo.stBuf));
        if (MT_SUCCESS != ret)
        {
            PROC_FATAL("Alloc fail:%#x\n", ret);
            goto out2;
        }
        memset((mt_void*)pstEntry->stInfo.stBuf.startVirAddr, 0, pstEntry->stInfo.stBuf.size);
    }
    else
    {
        pstEntry->stInfo.stBuf.size = 0;
        pstEntry->stInfo.stBuf.startPhyAddr = 0;
        pstEntry->stInfo.stBuf.startVirAddr = 0;
    }

    /* Add entry to rbtree */
    if (MT_SUCCESS != mt_rb_tree_insert_entry(pstDir, pstEntry))
    {
        PROC_ERR("Insert new file entry failed.\n");
        goto out2;
    }

    return pstEntry;

out2:
    remove_proc_entry(pstEntry->entry_name, pstEntry->parent);
out1:
    MT_KFREE(MT_ID_PROC, pstEntry);
out:

    return MT_NULL;
}

mt_void mt_proc_drv_rm_entry(mt_proc_dir_t *pstDir, mt_priv_proc_entry_t * pstEntry)
{
    /* Check parameter */
    if (MT_NULL == pstEntry || MT_NULL == pstDir)
    {
        return;
    }

    /* Remove proc entry */
    remove_proc_entry(pstEntry->entry_name, pstEntry->parent);

    PROC_INFO("Proc remove entry %s\n", pstEntry->entry_name);

    /* Free MMZ */
    if (0 != pstEntry->stInfo.stBuf.size)
    {
        mt_drv_mmz_unmap_and_release(&(pstEntry->stInfo.stBuf));
    }

    /* Remove entry from rbtree */
    mt_rb_tree_erase_entry(pstDir, pstEntry);

    /* If current command belongs to this entry, clear it. */
    if (g_stUProcParam.current_cmd.pEntry == (mt_void*)&(pstEntry->stInfo))
    {
        g_stUProcParam.current_cmd.pEntry = MT_NULL;
        memset(&g_stUProcParam.current_cmd, 0, sizeof(g_stUProcParam.current_cmd));
    }

    /* Free resource */
    MT_KFREE(MT_ID_PROC, pstEntry);

    return;
}

mt_s32 mt_proc_drv_rm_entry_by_name(const mt_char* pszName, const mt_char* pszParent)
{
    mt_priv_proc_entry_t * pstEntry = MT_NULL;
    mt_proc_dir_t * pstDir = MT_NULL;
    mt_char aszDir[MAX_PROC_NAME_LEN+12];

    /* Check parameter */
    if ((MT_NULL == pszName) || (strlen(pszName) > MAX_PROC_NAME_LEN))
    {
        PROC_ERR("Invalid name\n");
        return MT_FAILURE;
    }
    if ((MT_NULL == pszParent) || (strlen(pszParent) > MAX_PROC_NAME_LEN))
    {
        PROC_ERR("Invalid parent name\n");
        return MT_FAILURE;
    }

    /* Make parent directory name */
    if (0 == strlen(pszParent))
    {
        strlcpy(aszDir, "mcomm", sizeof(aszDir) - 1);
    }
    else
    {
        mt_osal_snprintf(aszDir, sizeof(aszDir), "%s", pszParent);
    }

    /* Find directory node, if don't exist, return fail */
    pstDir = mt_rb_tree_find_dirent(MT_NULL, aszDir);
    if (MT_NULL == pstDir)
    {
        PROC_ERR("Dir %s don't exist\n", pszParent);
        return MT_FAILURE;
    }

    /* Find entry in the directory, if don't exist, return fail */
    pstEntry = mt_rb_tree_find_entry(pstDir, pszName);
    if (MT_NULL == pstEntry)
    {
        PROC_WARN("Entry %s don't exist\n", pszName);
        return MT_FAILURE;
    }

    /* Remove entry */
    mt_proc_drv_rm_entry(pstDir, pstEntry);

    return MT_SUCCESS;
}

mt_void mt_proc_drv_rm_dir_by_force(mt_proc_dir_t *pstDir)
{
    struct rb_node *node;
    mt_priv_proc_entry_t * this;

    /* Check parameter */
    if (MT_NULL == pstDir)
    {
        return;
    }

    PROC_INFO("Proc remove dir: %s\n", pstDir->dir_name);

    while (pstDir->entry_root.rb_node)
    {
        node = rb_first(&(pstDir->entry_root));
        this = rb_entry(node, mt_priv_proc_entry_t, node);
        mt_proc_drv_rm_entry(pstDir, this);
    }

    mt_proc_drv_rm_dir(pstDir);
}

static struct proc_ops s_proc_seq_fops = {
        .proc_open = mt_proc_seq_open,
        .proc_read = seq_read,
        .proc_write   = mt_proc_seq_write,
        .proc_lseek = seq_lseek,
        .proc_release = mt_proc_seq_release,
};

long mt_usrmodeproc_ioctl(struct file *file,
				unsigned int cmd, unsigned long arg)
{
    mt_s32 ret = MT_SUCCESS;
    mt_priv_proc_param_t *proc = file->private_data;
    mt_drv_u_proc_entry_t mt_proc_entry;
    mt_char aszName[MAX_PROC_NAME_LEN+1];
    mt_proc_dir_t * pstDir;
    mt_priv_proc_entry_t* pstEntry;
    mt_drv_u_proc_cmdinfo_t *pstCmdInfo;

    switch(cmd)
    {
        case UMPIOC_ADD_ENTRY:
        {
            if (copy_from_user(&mt_proc_entry, (void __user *)arg, sizeof(mt_proc_entry)))
            {
                ret = MT_FAILURE;
                break;
            }
            mt_proc_entry.aszName[sizeof(mt_proc_entry.aszName)-1] = 0;
            mt_proc_entry.aszParent[sizeof(mt_proc_entry.aszParent)-1] = 0;
            mt_proc_entry.pFile = (mt_void*)file;

            UPROC_K_LOCK(g_stUProcParam.stSem);
            pstEntry = mt_proc_drv_add_entry(&mt_proc_entry, MT_TRUE);
            UPROC_K_UNLOCK(g_stUProcParam.stSem);
            if (MT_NULL == pstEntry)
            {
                ret = MT_FAILURE;
                break;
            }

            ((struct proc_dir_entry *)pstEntry->stInfo.pEntry)->data = proc;

            break;
        }

        case UMPIOC_REMOVE_ENTRY:
        {
            if (copy_from_user(&mt_proc_entry, (void __user *)arg, sizeof(mt_proc_entry)))
            {
                ret = MT_FAILURE;
                break;
            }
            mt_proc_entry.aszName[sizeof(mt_proc_entry.aszName)-1] = 0;
            mt_proc_entry.aszParent[sizeof(mt_proc_entry.aszParent)-1] = 0;

            UPROC_K_LOCK(g_stUProcParam.stSem);
            /* Removed by name now, can be removed by mt_proc_entry.pEntry  */
            ret = mt_proc_drv_rm_entry_by_name((mt_char*)mt_proc_entry.aszName, (mt_char*)mt_proc_entry.aszParent);
            UPROC_K_UNLOCK(g_stUProcParam.stSem);
            break;
        }

        case UMPIOC_ADD_DIR:
        {
            mt_char *ptrDirName = aszName;
            if (copy_from_user(ptrDirName, (void __user *)arg, sizeof(mt_proc_dir_name_t)))
            {
                ret = MT_FAILURE;
                break;
            }
            aszName[sizeof(aszName)-1] = 0;

            UPROC_K_LOCK(g_stUProcParam.stSem);
            pstDir = mt_proc_drv_add_dir(aszName, MT_NULL, file);
            if (MT_NULL == pstDir)
            {
                ret = MT_FAILURE;
            }
            UPROC_K_UNLOCK(g_stUProcParam.stSem);
            break;
        }
        case UMPIOC_REMOVE_DIR:
        {
            mt_char *ptrDirName = aszName;

            if (copy_from_user(ptrDirName, (void __user *)arg, sizeof(mt_proc_dir_name_t)))
            {
                ret = MT_FAILURE;
                break;
            }
            aszName[sizeof(aszName)-1] = 0;

            UPROC_K_LOCK(g_stUProcParam.stSem);
            ret = mt_proc_drv_rm_dir_by_name(aszName);
            UPROC_K_UNLOCK(g_stUProcParam.stSem);
            break;
        }
        case UMPIOC_GETCMD:
        {
            pstCmdInfo = (mt_drv_u_proc_cmdinfo_t*)arg;

            UPROC_K_LOCK(g_stUProcParam.stSem);
                /* If there is a command */
            if ((strlen(proc->current_cmd.aszCmd) > 0) &&
                /* and it must belong to a entry */
                (MT_NULL != proc->current_cmd.pEntry) &&
                /* and the entry must belong to this file(this process). */
                ((mt_void*)file == ((mt_drv_u_proc_entry_t*)proc->current_cmd.pEntry)->pFile))
            {
                if (copy_to_user((void __user *)&(pstCmdInfo->stCmd),
                    &(proc->current_cmd), sizeof(mt_drv_u_proc_cmd_t)))
                {
                    UPROC_K_UNLOCK(g_stUProcParam.stSem);
                    return -EFAULT;
                }
                if (copy_to_user((void __user *)&(pstCmdInfo->stEntry),
                    proc->current_cmd.pEntry, sizeof(mt_drv_u_proc_entry_t)))
                {
                    UPROC_K_UNLOCK(g_stUProcParam.stSem);
                    return -EFAULT;
                }

                memset(proc->current_cmd.aszCmd, 0, sizeof(proc->current_cmd.aszCmd));
            }
            UPROC_K_UNLOCK(g_stUProcParam.stSem);
            break;
        }

        case UMPIOC_WAKE_READ_TASK:
            wake_up_interruptible(&(proc->wq_for_read));
            break;

         case UMPIOC_WAKE_WRITE_TASK:
            wake_up_interruptible(&(proc->wq_for_write));
            break;

        default:
            ret = MT_FAILURE;
            break;
    }

    return ret;
}


static struct file_operations mt_usrmodeproc_fops =
{
    .open    = mt_usrmodeproc_open,
    .release   = mt_usrmodeproc_close,
    .unlocked_ioctl = mt_usrmodeproc_ioctl,
};

static struct miscdevice mt_usrmodeproc_dev =
{
    MISC_DYNAMIC_MINOR,
    MT_USERPROC_DEVNAME,
    &mt_usrmodeproc_fops
};

mt_s32 mt_proc_drv_mod_init(mt_void)
{
    mt_s32 ret;

    ret = misc_register(&mt_usrmodeproc_dev);
    if (ret)
    {
        PROC_ERR("%s device register failed\n", MT_USERPROC_DEVNAME);
    }

    g_pst_comm_dirent = mt_proc_drv_add_private_dir("mcomm", gp_mcomm_proc);
    if (!g_pst_comm_dirent)
    {
        PROC_ERR("add 'mcomm' directory failed.\n");
        ret = -1;
        goto out;
    }

    g_pst_msp_dirent = mt_proc_drv_add_private_dir("msp", gp_msp_proc);
    if (!g_pst_msp_dirent)
    {
        PROC_ERR("add 'msp' directory failed.\n");
        ret = -1;
        goto out;
    }

out:
    return ret;
}

mt_void mt_proc_drv_mod_exit(mt_void)
{
    if (g_pst_msp_dirent)
    {
        mt_proc_drv_rm_private_dir(g_pst_msp_dirent->dir_name);
        g_pst_msp_dirent = MT_NULL;
    }

    if (g_pst_comm_dirent)
    {
        mt_proc_drv_rm_private_dir(g_pst_comm_dirent->dir_name);
        g_pst_comm_dirent = MT_NULL;
    }

    misc_deregister(&mt_usrmodeproc_dev);
}

MODULE_DESCRIPTION("mt User Mode Proc Driver");
MODULE_LICENSE("GPL");

#endif

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* End of #ifdef __cplusplus */
