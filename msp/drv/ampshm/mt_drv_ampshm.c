/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : mt_drv_ampshm.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2017/12/02
 * Description    : MT share memory functions for asymmetrical(AMP) AV/AP CPUs.
 * History        :
 * 1.Date         : 2017/12/02
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#endif

#include "mt_type.h"
#include "mt_type.h"

#ifdef __KERNEL__
#include "sys_define.h"
#include "ipc.h"
#include "ipc_common.h"

#include "mt_drv_dev.h"
#include "mt_drv_struct.h"
#include "mt_kernel_adapt.h"
#include "mt_drv_module.h"
#include "mt_module_debug.h"
#include "mt_osal.h"
#include "mt_drv_mmz.h"
#endif

#include "rpc.h"
#include "rpc_id.h"
#include "mt_drv_ampshm.h"

#ifdef __KERNEL__
#include "drv_ampshm_ioctl.h"
#include "mt_drv_log.h"
#include "mt_debug.h"
#endif

//#define DRV_TEST_AMPSHM
#ifdef DRV_TEST_AMPSHM
#include "drv_test_ampshm.c"
#endif

#ifdef __KERNEL__
#define SHM_DEBUG(fmt...)				MT_INFO_PRINT(MT_ID_AMPSHM, fmt)
#define SHM_ERROR(fmt...)				MT_ERR_PRINT(MT_ID_AMPSHM, fmt)
#else
#include "Log.h"
#undef LOG_TAG
#define LOG_TAG							"AMPSHM"
#define SHM_DEBUG						MLOGD
#define SHM_ERROR						MLOGE
#endif

#define MT_AMPSHM_NAME					"MT_AMPSHM"
#define MT_AMPSHM_MMZ_NAME				"AMPSHM"
#define MT_AMPSHM_MMZ_ALIGN_SIZE		64
#define MAX_AMPSHM_RECORD_COUNT			MAX_SHM_BUFF_COUNT

/* AMP SHM record struct definition */
struct mt_ampshm_record_t
{
/* same as mmz_buffer_s
typedef struct mtmmz_buffer_s
{
    mt_u32 u32StartVirAddr;
    mt_u32 u32StartPhyAddr;
    mt_u32 u32Size;
}mmz_buffer_s;
*/

	ulong vir_addr;			/* kernel virtual address */
	phys_addr_t phy_addr;
	unsigned int size;

	char name[MAX_SHM_NAME_LEN+1];

	unsigned int flags;

	unsigned int reserved[2];		//aligned 32 byte
};

/* AMP SHM management struct definition */
struct mt_ampshm_manager_t
{
	unsigned int count;				/* record count */
	unsigned int bitmap;			/* record bitmap */
	unsigned int reserved[6];		//aligned 32 byte

	struct mt_ampshm_record_t records[0];
};

/* global AMP SHM controller */
struct mt_ampshm_controller_t
{
#ifdef __KERNEL__
	mmz_buffer_s mmz_buf;

	struct mt_ampshm_manager_t *manager;
#endif
};

static struct mt_ampshm_controller_t g_ampshm_ctrl;
static volatile struct mt_ampshm_manager_t *gp_ampshm_manager = NULL;

//----------------------------------------------------------------------------//

#ifdef __KERNEL__
extern mt_s32 avse_shm_init(void);
extern void avse_shm_deinit(void);
extern void *avse_shm_allocate(mt_u32 id, mt_u32 size);
extern void avse_shm_free(void *ptr);
#endif

#ifdef __KERNEL__
/* remote call AV CPU's SHM module set parameter api */
static inline int _remote_set_parameter(int para_index, unsigned int value)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
	ipc_msg_t msg;

	/* symphony 4: avcpu has only audio module, no rpc */
	msg.msg_id = (u32)AMPSHM_MAGIC_IPC_MSG_ID;
	msg.param1 = (u32)para_index;
	msg.param2 = (u32)value;
	msg.time_out_ms = (u32)3000;
	return ap_send_to_av((u32)AP_SYS_DEV_AUD, &msg, (u8)0);

#else
	//return rpc_call(RPC_AV_FN_ID_AMPSHM_SET_PARAMETER, para_index, value, 0, NULL);
	return rpc_call_to(RPC_AV_FN_ID_AMPSHM_SET_PARAMETER, para_index, value, 0, NULL, RPC_CALL_TIMEOUT_DEFAULT);
#endif
}
#endif

static inline void mt_set_bit(unsigned int *bitmap, int bit)
{
	*bitmap |= (0x01 << bit);
}

static inline void mt_clear_bit(unsigned int *bitmap, int bit)
{
	*bitmap &= ~(0x01 << bit);
}

static unsigned long name_to_id(const char *name)
{
	unsigned long id;

	if(0 != kstrtoul(name, 0, &id))
	{
	    return (unsigned long)-1;
	}

	return id;
}

/*
 * @param[out] fd record index
 */
static volatile struct mt_ampshm_record_t *find_record_by_name(const char *name, int *fd)
{
	unsigned int i;
	unsigned int count = gp_ampshm_manager->count;
	volatile struct mt_ampshm_record_t *records = gp_ampshm_manager->records;

	for (i=0; i<count; i++)
	{
		if (records[i].name[0] != 0
			&& strcmp(name, (char *)records[i].name) == 0)
		{
			*fd = i;
			return &records[i];
		}
	}

	return NULL;
}

ulong mt_ampshm_get_addr_by_name(const char *name)
{    
    volatile struct mt_ampshm_record_t *record = NULL;
    int fd = 0;

    record = find_record_by_name(name, &fd);
    if(record != NULL)
        return record->vir_addr;
    
    return 0;
}
/*
 * @param[out] fd record index
 */
static volatile struct mt_ampshm_record_t *find_record_by_addr(void *addr, int *fd)
{
	unsigned int i;
	unsigned int count = gp_ampshm_manager->count;
	volatile struct mt_ampshm_record_t *records = gp_ampshm_manager->records;

	for (i=0; i<count; i++)
	{
		if ((ulong)addr == records[i].vir_addr)
		{
			*fd = i;
			return &records[i];
		}
	}

	return NULL;
}

/*
 * @param[out] fd record index
 */
static volatile struct mt_ampshm_record_t *allocate_record(int *fd)
{
	unsigned int i;
	unsigned int count = gp_ampshm_manager->count;
	volatile struct mt_ampshm_record_t *records = gp_ampshm_manager->records;

	for (i=0; i<count; i++)
	{
		if (records[i].name[0] == 0
			&& records[i].phy_addr == 0
			&& records[i].size == 0)
		{
			*fd = i;
			return &records[i];
		}
	}

	return NULL;
}

#ifndef __KERNEL__
/* AMP SHM set parameter */
static int mt_ampshm_set_parameter(unsigned int para_index, unsigned int value)
{
	if (para_index == AMPSHM_PARA_INDEX_MANAGER_ADDR)
	{
		//set manager address from AP CPU AMP SHM driver
		gp_ampshm_manager = (struct mt_ampshm_manager_t *)hal_addr_nc(value);
		SHM_DEBUG("shm manager address=%p, count=%u\n",gp_ampshm_manager,gp_ampshm_manager->count);
		return 0;
	}
	else
	{
		SHM_ERROR("invalid arguments(%x, %x)!\n",para_index,value);
		return ERR_PARAM;
	}
}

/* RPC proxy function */
static RPC_RETVAL mt_ampshm_set_parameter_proxy(unsigned int what, unsigned int param1, unsigned int param2, struct rpc_ext_arg_t *ext)
{
	return mt_ampshm_set_parameter(what, param1);
}

/* IPC MSG Visitor */
static RET_CODE mt_ampshm_ipc_visitor(u32 msg_id, u32 param1, u32 param2)
{
	if (msg_id == AMPSHM_MAGIC_IPC_MSG_ID)
	{
		if (param1 == AMPSHM_PARA_INDEX_MANAGER_ADDR)
			mt_ampshm_set_parameter(param1, param2);
	}

	return 0;
}
#endif

//----------------------------------------------------------------------------//

/*
 * @retval fd AMP SHM file descriptor
 *         >=0: valid
 *          <0: error, invalid fd
 */
int mt_ampshm_open(const char *name, int oflag, mode_t mode)
{
	int fd = -1;
	volatile struct mt_ampshm_record_t *record;

	if (gp_ampshm_manager == NULL)
	{
		SHM_ERROR("mt_ampshm_open: module not initialized!\n");
		return -1;
	}

	if (name == NULL)
	{
		SHM_ERROR("mt_ampshm_open: invalid argument!\n");
		return -1;
	}

	record = find_record_by_name(name, &fd);
	if (record == NULL)
	{
#ifdef __KERNEL__
		record = allocate_record(&fd);
		if (record == NULL)
		{
			SHM_ERROR("mt_ampshm_open: allocate shm(%s) failed!\n",name);
			return -1;
		}

		memset((struct mt_ampshm_record_t *)record, 0, sizeof(struct mt_ampshm_record_t));
		strlcpy((char *)record->name, name, sizeof(record->name));
#else
		SHM_ERROR("mt_ampshm_open: not found shm(%s)!\n",name);
		return -1;
#endif
	}
#ifdef __KERNEL__
	mt_set_bit((unsigned int *)&gp_ampshm_manager->bitmap, fd);
#endif

	SHM_DEBUG("%s, return fd=%d\n",name,fd);
	return fd;
}

int mt_ampshm_ftruncate(int fd, off_t length)
{
#ifdef __KERNEL__
	volatile struct mt_ampshm_record_t *record;

	if (gp_ampshm_manager == NULL)
	{
		SHM_ERROR("mt_ampshm_open: module not initialized!\n");
		return -1;
	}

	if (fd < 0 || fd >= gp_ampshm_manager->count || length <= 0)
	{
		SHM_ERROR("mt_ampshm_ftruncate: invalid argument!\n");
		return -1;
	}

	record = &gp_ampshm_manager->records[fd];
	record->size = length;

	SHM_DEBUG("fd=%d, length=%lu\n",fd,length);
#endif
	return 0;
}

void *mt_ampshm_mmap(void *addr, size_t length, int prot, int flags,
		           int fd, off_t offset)
{
#ifdef __KERNEL__
    mt_s32 ret;
	mmz_buffer_s mmz_buf;
#endif
	volatile struct mt_ampshm_record_t *record;

	if (gp_ampshm_manager == NULL)
	{
		SHM_ERROR("mt_ampshm_mmap: module not initialized!\n");
		return NULL;
	}

	if (fd < 0 || fd >= gp_ampshm_manager->count || length == 0)
	{
		SHM_ERROR("mt_ampshm_mmap: invalid argument!\n");
		return NULL;
	}

	record = &gp_ampshm_manager->records[fd];

#ifdef __KERNEL__
	/* already mapped */
	if (record->vir_addr != 0)
	{
		return (void*)record->vir_addr;
	}

	/* already unmapped, not support re-map again */
	if (record->phy_addr != 0)
	{
		SHM_ERROR("phy_addr(0x%x) !=0, not support re-map again!\n",record->phy_addr);
		return NULL;
	}

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
	/* secure */
	if ((flags & MT_MAP_SECURE) == MT_MAP_SECURE)
	{
		record->vir_addr = (ulong)avse_shm_allocate(name_to_id((const char *)record->name), record->size);
		record->phy_addr = (phys_addr_t)virt_to_phys((void*)record->vir_addr);

		record->flags = flags;
		return (void*)record->vir_addr;
	}
	else
#endif
	{
		ret = mt_drv_mmz_alloc((const char *)record->name, NULL, record->size, MT_AMPSHM_MMZ_ALIGN_SIZE,
								&mmz_buf);
		if (MT_SUCCESS != ret)
		{
			SHM_ERROR("mt_drv_mmz_alloc failed\n");
			return NULL;
		}

		ret = mt_drv_mmz_map(&mmz_buf);
		if (MT_SUCCESS != ret)
		{
			SHM_ERROR("mt_drv_mmz_map failed\n");
			mt_drv_mmz_release(&mmz_buf);
			return NULL;
		}

		record->phy_addr = mmz_buf.startPhyAddr;
		record->vir_addr = (ulong)mmz_buf.startVirAddr;

		SHM_DEBUG("fd=%d, mapped addr=%x\n",fd,record->vir_addr);
		return (void*)record->vir_addr;
	}
#else
	SHM_DEBUG("fd=%d, mapped addr=%x\n",fd,hal_addr_nc(record->phy_addr));
	return (void*)hal_addr_nc(record->phy_addr);
#endif
}

int mt_ampshm_munmap(void *addr, size_t length)
{
#ifdef __KERNEL__
	int fd;
	volatile struct mt_ampshm_record_t *record;

	if (gp_ampshm_manager == NULL)
	{
		SHM_ERROR("mt_ampshm_munmap: module not initialized!\n");
		return -1;
	}

	if (addr == NULL)
	{
		SHM_ERROR("mt_ampshm_munmap: invalid argument!\n");
		return -1;
	}

	record = find_record_by_addr(addr, &fd);
	if (record == NULL)
	{
		SHM_ERROR("shm(addr:%p) not found!\n",addr);
		return -1;
	}

	/* already unmapped */
	if (record->vir_addr == 0)
	{
		return 0;
	}

	/* secure */
	if ((record->flags & MT_MAP_SECURE) == MT_MAP_SECURE)
	{
		//do nothing
	}
	else
	{
		mt_drv_mmz_unmap((mmz_buffer_s*)record);

		record->vir_addr = 0;
	}

	SHM_DEBUG("addr=%p, length=%u, fd=%d\n",addr,length,fd);
#endif
	return 0;
}

int mt_ampshm_unlink(const char *name)
{
#ifdef __KERNEL__
	int fd;
	volatile struct mt_ampshm_record_t *record;

	if (gp_ampshm_manager == NULL)
	{
		SHM_ERROR("mt_ampshm_unlink: module not initialized!\n");
		return -1;
	}

	if (name == NULL)
	{
		SHM_ERROR("mt_ampshm_unlink: invalid argument!\n");
		return -1;
	}

	record = find_record_by_name(name, &fd);
	if (record == NULL)
	{
		SHM_ERROR("shm(%s) not found!\n",name);
		return -1;
	}

	/* already unlinked */
	if (record->phy_addr == 0)
	{
		return 0;
	}

	/* secure */
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
	if ((record->flags & MT_MAP_SECURE) == MT_MAP_SECURE)
	{
		avse_shm_free((void*)record->vir_addr);
	}
	else
#endif
	{
		mt_drv_mmz_release((mmz_buffer_s*)record);
	}
	memset((struct mt_ampshm_record_t *)record, 0, sizeof(struct mt_ampshm_record_t));
	mt_clear_bit((unsigned int *)&gp_ampshm_manager->bitmap, fd);

	SHM_DEBUG("%s, fd=%d\n",name,fd);
#endif
	return 0;
}

int _ampshm_init(void)
{
#ifdef __KERNEL__
    mt_s32 ret;
    mmz_buffer_s *pmmz_buf = &g_ampshm_ctrl.mmz_buf;
    mt_u32 size;
    struct mt_ampshm_manager_t *manager;

	if (gp_ampshm_manager == NULL)
	{
		SHM_DEBUG("mt_ampshm init start.\n");
		size = sizeof(struct mt_ampshm_manager_t)
					+ MAX_AMPSHM_RECORD_COUNT*sizeof(struct mt_ampshm_record_t);

		ret = mt_drv_mmz_alloc(MT_AMPSHM_MMZ_NAME, NULL, size, MT_AMPSHM_MMZ_ALIGN_SIZE,
								pmmz_buf);
		if (MT_SUCCESS != ret)
		{
			SHM_ERROR("mt_drv_mmz_alloc failed\n");
			return -1;
		}

		ret = mt_drv_mmz_map(pmmz_buf);
		if (MT_SUCCESS != ret)
		{
			SHM_ERROR("mt_drv_mmz_map failed\n");
			return -1;
		}

		gp_ampshm_manager = (struct mt_ampshm_manager_t*)pmmz_buf->startVirAddr;
		g_ampshm_ctrl.manager = (struct mt_ampshm_manager_t *)gp_ampshm_manager;
		manager = (struct mt_ampshm_manager_t *)gp_ampshm_manager;
		manager->count = MAX_AMPSHM_RECORD_COUNT;
		memset(manager->records, 0, MAX_AMPSHM_RECORD_COUNT*sizeof(struct mt_ampshm_record_t));

		SHM_DEBUG("mt_ampshm init: manager %p, count %d.\n",manager,manager->count);

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
		ret = avse_shm_init();
		BUG_ON(ret != MT_SUCCESS);
#endif

#if 1
		ret = _remote_set_parameter(AMPSHM_PARA_INDEX_MANAGER_ADDR, (unsigned int)pmmz_buf->startPhyAddr);
		if (MT_SUCCESS != ret)
		{
			SHM_ERROR("FATAL Error, _remote_set_parameter failed\n");
			mt_drv_mmz_unmap(pmmz_buf);
			mt_drv_mmz_release(pmmz_buf);
			gp_ampshm_manager = NULL;
			g_ampshm_ctrl.manager = NULL;
			return -1;
		}
#endif
		SHM_DEBUG("mt_ampshm init done.\n");
	}

#ifdef DRV_TEST_AMPSHM
	drv_test_ampshm();
#endif

#else
/* AV CPU */
#ifdef CONFIG_MT_CHIP_SYMPHONY4
	ipc_register_visitor(mt_ampshm_ipc_visitor);
#else
	//register remote call proxy functions
	rpc_register(RPC_AV_FN_ID_AMPSHM_SET_PARAMETER, mt_ampshm_set_parameter_proxy);
#endif
#endif
	return 0;
}

void _ampshm_exit(void)
{
#ifdef __KERNEL__
    mmz_buffer_s *pmmz_buf = &g_ampshm_ctrl.mmz_buf;

	if (gp_ampshm_manager != NULL)
	{
		SHM_DEBUG("mt_ampshm exit\n");
		mt_drv_mmz_unmap(pmmz_buf);
		mt_drv_mmz_release(pmmz_buf);
		gp_ampshm_manager = NULL;
		g_ampshm_ctrl.manager = NULL;

		_remote_set_parameter(AMPSHM_PARA_INDEX_MANAGER_ADDR, 0);

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
		avse_shm_deinit();
#endif
	}
#endif
}

//---------------------------------------------------------------------------//

#ifdef __KERNEL__
static mt_device_s g_AmpshmRegisterData;

MT_DECLARE_MUTEX(g_AmpshmMutex);

mt_s32 AMPSHM_Ioctl(struct inode *inode, struct file *file, unsigned int cmd, mt_void *arg)
{
    mt_s32 Ret = MT_SUCCESS;
    struct mt_ampshm_file_st *fp;

    Ret = down_interruptible(&g_AmpshmMutex);

	fp = (struct mt_ampshm_file_st *)arg;

    switch (cmd)
    {
		case CMD_AMPSHM_OPEN:
			fp->fd = mt_ampshm_open(fp->name, fp->oflag, fp->mode);

			if (fp->fd < 0)
				Ret = MT_FAILURE;

			break;

		case CMD_AMPSHM_FTRUNCATE:
			Ret = mt_ampshm_ftruncate(fp->fd, fp->length);
			break;

		case CMD_AMPSHM_MMAP:
			fp->kn_vir_addr = (ulong)mt_ampshm_mmap(NULL, fp->length, fp->prot, fp->flags, fp->fd, 0);

			if (fp->kn_vir_addr != 0)
				fp->phy_addr = (phys_addr_t)virt_to_phys((void*)fp->kn_vir_addr);
			else
				Ret = MT_FAILURE;

			break;

		case CMD_AMPSHM_MUNMAP:
			Ret = mt_ampshm_munmap((void*)fp->kn_vir_addr, fp->length);
			fp->kn_vir_addr = 0;
			break;

		case CMD_AMPSHM_UNLINK:
			Ret = mt_ampshm_unlink(fp->name);
			fp->fd = -1;
			break;

        default:
			SHM_ERROR("ERR: AMPSHM_Ioctl, unknown CMD = %x!\n", cmd);
            up(&g_AmpshmMutex);
            return -ENOIOCTLCMD;
    }

    up(&g_AmpshmMutex);
    return Ret;
}

mt_s32 AMPSHM_Set_Share_Memory(void)
{
	mt_s32 ret = 0;
	mmz_buffer_s *pmmz_buf = &g_ampshm_ctrl.mmz_buf;
	ret = _remote_set_parameter(AMPSHM_PARA_INDEX_MANAGER_ADDR, (unsigned int)pmmz_buf->startPhyAddr);
	return ret;
}

static mt_s32 AMPSHM_DRV_Open(struct inode *finode, struct file  *ffile)
{
	SHM_DEBUG("AMPSHM Open\n");
    return 0;
}

static mt_s32 AMPSHM_DRV_Close(struct inode *finode, struct file  *ffile)
{
	SHM_DEBUG("AMPSHM Close\n");
    return 0;
}

static long AMPSHM_DRV_Ioctl(struct file *ffile, unsigned int cmd, unsigned long arg)
{
    mt_s32 Ret;

    Ret = mt_drv_usercopy(ffile->f_path.dentry->d_inode, ffile, cmd, arg, AMPSHM_Ioctl);

    return Ret;
}

static mt_s32 AMPSHM_Suspend(basedev_s *pdev, pm_message_t state)
{
	SHM_DEBUG("AMPSHM Suspend\n");
    return 0;
}

static mt_s32 AMPSHM_Resume(basedev_s *pdev)
{
	SHM_DEBUG("AMPSHM Resume\n");
    return 0;
}

static struct file_operations AMPSHM_FOPS =
{
    .owner          = THIS_MODULE,
    .open           = AMPSHM_DRV_Open,
    .unlocked_ioctl = AMPSHM_DRV_Ioctl,
    .release        = AMPSHM_DRV_Close,
};

static baseops_s AMPSHM_DRVOPS = {
    .probe        = NULL,
    .remove       = NULL,
    .shutdown     = NULL,
    .prepare      = NULL,
    .complete     = NULL,
    .suspend      = AMPSHM_Suspend,
    .suspend_late = NULL,
    .resume_early = NULL,
    .resume       = AMPSHM_Resume,
};

#endif

int mt_ampshm_init(void)
{
    mt_s32      Ret;

	_ampshm_init();

#ifdef __KERNEL__

    Ret = mt_drv_module_register(MT_ID_AMPSHM, MT_AMPSHM_NAME, MT_NULL);
    if (MT_SUCCESS != Ret)
    {
        SHM_ERROR("ERR: mt_drv_module_register, Ret = %#x!\n", Ret);
    }

    mt_osal_snprintf(g_AmpshmRegisterData.devfs_name, sizeof(g_AmpshmRegisterData.devfs_name), UMAP_DEVNAME_AMPSHM);
    g_AmpshmRegisterData.fops = &AMPSHM_FOPS;
    g_AmpshmRegisterData.minor = UMAP_MIN_MINOR_AMPSHM;
    g_AmpshmRegisterData.owner = THIS_MODULE;
    g_AmpshmRegisterData.drvops = &AMPSHM_DRVOPS;

	if (mt_drv_dev_register(&g_AmpshmRegisterData) < 0)
	{
		SHM_ERROR("register AMPSHM failed.\n");
		return MT_FAILURE;
	}
#endif

	return MT_SUCCESS;
}

void mt_ampshm_exit(void)
{
#ifdef __KERNEL__

	_ampshm_exit();

    mt_drv_dev_unregister(&g_AmpshmRegisterData);

    mt_drv_module_unregister(MT_ID_AMPSHM);
#endif
}

#ifdef __KERNEL__
EXPORT_SYMBOL(mt_ampshm_init);
EXPORT_SYMBOL(mt_ampshm_exit);
EXPORT_SYMBOL(mt_ampshm_open);
EXPORT_SYMBOL(mt_ampshm_ftruncate);
EXPORT_SYMBOL(mt_ampshm_mmap);
EXPORT_SYMBOL(mt_ampshm_munmap);
EXPORT_SYMBOL(mt_ampshm_unlink);
#endif

