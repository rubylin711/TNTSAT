/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2019, Montage Technology Co., Ltd.
 *
 * File Name      : ipc_secure_plugin.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/09/04
 * Description    : Secure Plugin for IPC.
 * History        :
 * 1.Date         : 2019/09/04
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <linux/kernel.h>   /* printk() */
#include <asm/cacheflush.h>

#include "mt_type.h"
#include "mt_cache.h"
#include "ipc.h"
#include "mt_drv_log.h"

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "concerto_regs_av.h"

#include "mt_drv_ampshm.h"

#define IPC_PRINTF 						printk
//#define IPC_PRINTF 					MT_INFO_LOG

#define MAX_CHANNEL_NUMS 				32

/* Secure: only channel 16-31 */
#define REE2AV_CHANNEL_FROM 16
#define DEFAULT_SECURE_IPC_AV2REE_CH	REE2AV_CHANNEL_FROM

#define REG32_WRITE(addr,val) 			HAL_PUT_U32((volatile u32 *)addr, (u32)val)

extern u32 g_ap_mb_msg_addr;

enum
{
	IPC_SECURE_MSG_SHM_ID = 0x20190904
};

//---------------------------------------------------------------------------//

struct avshm_head_st
{
	mt_u32 id;
	mt_u32 size;
} __attribute__((aligned(CACHE_LINE_SIZE)));

static void *AVSHM_ALLOC(mt_u32 id, mt_u32 size)
{
	void *ptr;
	char name[32];
	int fd;
	size_t length;
	struct avshm_head_st *pHead;

	snprintf(name, MAX_SHM_NAME_LEN, "0x%x", id);

	fd = mt_ampshm_open(name, 0, 0);

	if (fd < 0)
		return NULL;

	length = size + sizeof(struct avshm_head_st);

	if (mt_ampshm_ftruncate(fd, (off_t)length) != 0)
		return NULL;

	ptr = mt_ampshm_mmap(NULL, length, 0, MT_MAP_SECURE, fd, 0);

	if (ptr != NULL)
	{
		pHead = (struct avshm_head_st *)(ptr);
		pHead->id = id;
		pHead->size = size;

		return (void*)((ulong)ptr + sizeof(struct avshm_head_st));
	}
	else
	{
		return NULL;
	}
}

static void AVSHM_FREE(void *ptr)
{
	char name[32];
	size_t length;
	struct avshm_head_st *pHead;

	if (ptr != NULL)
	{
		pHead = (struct avshm_head_st *)((ulong)ptr - sizeof(struct avshm_head_st));

		snprintf(name, MAX_SHM_NAME_LEN, "0x%x", pHead->id);
		length = pHead->size + sizeof(struct avshm_head_st);

		mt_ampshm_munmap((void*)pHead, length);

		mt_ampshm_unlink(name);
	}
}

/**
 * @brief allocate IPC message buffer from AVCPU Secure Share Memory
 */
void *ipc_secure_malloc(unsigned int size)
{
	void *ptr;

	ptr = AVSHM_ALLOC((mt_u32)IPC_SECURE_MSG_SHM_ID, (mt_u32)size);
	if (ptr != NULL)
	{
		IPC_PRINTF("%s: allocate ipc secure msg shm(%p, %u) success.\n",__FUNCTION__,ptr,size);
	}
	else
	{
		IPC_PRINTF("%s: allocate ipc secure msg shm failed!\n",__FUNCTION__);
	}

	return ptr;
}

void ipc_secure_free(void *ptr)
{
	IPC_PRINTF("%s: ptr %p\n",__FUNCTION__,ptr);
	AVSHM_FREE(ptr);
}

/**
 * @brief get AVCPU to REE IPC INT State
 *
 * @return AVCPU to REE IPC INT State
 */
mt_u32 ipc_secure_get_int_state(void)
{
	//FIXME: use share memory to emulate MCPU_MB_INT_STATE register?
	return (0x01 << DEFAULT_SECURE_IPC_AV2REE_CH);
}

/**
 * @brief get AVCPU's IPC message address from the global IPC message buffer
 *
 * @param[in] ch IPC Message Channel
 *
 * @return physical IPC message address
 */
mt_u32 ipc_secure_get_av_msg(int ch)
{
/* mips physical address */
#define mips_phys_addr(a)	((a)&0x1fffffff)

	mt_u32 msg_vir_addr;
	mt_u32 msg_phy_addr;

	if (ch < 0 || ch >= MAX_CHANNEL_NUMS)
	{
		IPC_PRINTF("%s: invalid ch(%d)!\n",__FUNCTION__,ch);
		return 0;
	}

	/*
	 * g_ap_mb_msg_addr:
	 *    -------------------------------------------------------
	 *    | APCPU 32 IPC Channel MSG | AVCPU 32 IPC Channel MSG |
	 *    -------------------------------------------------------
	 */
	msg_vir_addr = g_ap_mb_msg_addr + (MAX_CHANNEL_NUMS + ch) * sizeof(mb_msg_t);

#ifdef CONFIG_ARM
	msg_phy_addr = (mt_u32)__pa(msg_vir_addr);
#else
	msg_phy_addr = mips_phys_addr(msg_vir_addr);
#endif

	/* Secure: REE can NOT write MCPU_MB_INFO_REG! */
	//apply to MCPU_MB_INFO_REG
//	REG32_WRITE(MCPU_MB_INFO_REG(ch), msg_phy_addr);

	IPC_PRINTF("%s: ch(%d), msg phy addr 0x%x\n",__FUNCTION__,ch,msg_phy_addr);

	return msg_phy_addr;
}

