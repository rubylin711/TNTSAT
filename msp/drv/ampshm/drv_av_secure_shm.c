/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2019, Montage Technology Co., Ltd.
 *
 * File Name      : drv_av_secure_shm.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/8/20
 * Description    : AVCPU Secure Share Memory with APCPU. [Draft]
 * History        :
 * 1.Date         : 2019/8/20
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/

/*
 * 高安仅支持配置一块可控流量的共享内存,
 * 所以就预先分配一块大的AVCPU共享内存,ADEC/IPC再从中各自分配小块内存.
 */

#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include "mt_type.h"
#include "mt_cache.h"
#include "mt_drv_mmz.h"

//#define PRINTF
#define PRINTF						printk

#define MAX_AV_SE_SHM_COUNT			32
#define MAX_AV_SE_SHM_BUFFER_SIZE	(128 * 1024)

#ifndef ALIGN
#define ALIGN(val,align) 			(((val) + ((align) - 1)) & ~((align) - 1))
#endif

struct shm_item_st
{
	mt_u32 id;
	mt_u32 offset;
	mt_u32 size;
};

struct av_secure_shm_manager_st
{
	mmz_buffer_s avse_shm_mmzbuf;

	int count;
	struct shm_item_st items[0];
};

static mmz_buffer_s manager_mmzbuf;
static struct av_secure_shm_manager_st *g_avse_shm_manager = NULL;

mt_s32 avse_shm_init(void);
void avse_shm_deinit(void);
void *avse_shm_allocate(mt_u32 id, mt_u32 size);
void avse_shm_free(void *ptr);

//---------------------------------------------------------------------------//

static mt_s32 init_manager(void)
{
	mt_s32 ret;
	mmz_buffer_s *pMmzBuf;
	mt_u32 size;

	/* manager */
	size = sizeof(struct av_secure_shm_manager_st)
				+ MAX_AV_SE_SHM_COUNT * sizeof(struct shm_item_st);

	pMmzBuf = &manager_mmzbuf;

	ret = mt_drv_mmz_alloc_and_map("AV_SE_SHM_M", MT_NULL, size, CACHE_LINE_SIZE, pMmzBuf);
	if (ret == MT_SUCCESS)
	{
		PRINTF("%s: av secure shm manager - addr (0x%llx, 0x%px), size %u\n",__FUNCTION__,
				pMmzBuf->startPhyAddr,pMmzBuf->startVirAddr,size);

		g_avse_shm_manager = (struct av_secure_shm_manager_st*)pMmzBuf->startVirAddr;

		memset(g_avse_shm_manager, 0, size);
		g_avse_shm_manager->count = MAX_AV_SE_SHM_COUNT;

		/* AVCPU Secure Share Memory Buffer */
		pMmzBuf = &g_avse_shm_manager->avse_shm_mmzbuf;

		/* FIXME: how AVCPU<->APCPU share memory allocate from? */
		ret = mt_drv_mmz_alloc_and_map("AV_SE_SHM", MT_NULL, MAX_AV_SE_SHM_BUFFER_SIZE, CACHE_LINE_SIZE, pMmzBuf);
		if (ret == MT_SUCCESS)
		{
			PRINTF("%s: av secure shm - addr (0x%llx, 0x%px), size %u\n",__FUNCTION__,
					pMmzBuf->startPhyAddr,pMmzBuf->startVirAddr,MAX_AV_SE_SHM_BUFFER_SIZE);

			//TODO:
			//config avse_shm_mmzbuf to be AV Secure Share Memory.
			PRINTF("%s: TODO: config avse_shm_mmzbuf to be AV Secure Share Memory.\n",__FUNCTION__);
			return MT_SUCCESS;
		}
		else
		{
			mt_drv_mmz_unmap_and_release(&manager_mmzbuf);
			g_avse_shm_manager = NULL;
			PRINTF("[ERROR]%s: allocate av secure shm failed!\n",__FUNCTION__);
			return MT_FAILURE;
		}
	}
	else
	{
		PRINTF("[ERROR]%s: allocate av secure shm manager failed!\n",__FUNCTION__);
		return MT_FAILURE;
	}
}

static mt_s32 allocate(mt_u32 id, mt_u32 size, ulong *addr)
{
	struct shm_item_st *pItem;
	mt_u32 offset = 0;
	int i;

	if (g_avse_shm_manager != NULL)
	{
		pItem = g_avse_shm_manager->items;

		for (i=0; i<g_avse_shm_manager->count; i++)
		{
			if (pItem[i].id == 0)
			{
				if (offset + size <= g_avse_shm_manager->avse_shm_mmzbuf.size)
				{
					pItem[i].id = id;
					pItem[i].size = size;
					pItem[i].offset = offset;

					*addr = (ulong)g_avse_shm_manager->avse_shm_mmzbuf.startVirAddr + offset;
					PRINTF("%s: allocate id 0x%x -> addr 0x%lx\n",__FUNCTION__,
							id, *addr);
					return MT_SUCCESS;
				}
				else
				{
					PRINTF("[ERROR]%s: allocate(0x%x %u) failed! out of memory!\n",
							__FUNCTION__,id,size);
					return MT_FAILURE;
				}
			}

			offset = pItem[i].offset + pItem[i].size;
			offset = ALIGN(offset, CACHE_LINE_SIZE);
		}
	}
	else
	{
		PRINTF("[ERROR]%s: av secure shm manager not inited!\n",__FUNCTION__);
		return MT_FAILURE;
	}

	PRINTF("[ERROR]%s: allocate(0x%x %u) failed!\n",__FUNCTION__,id,size);
	return MT_FAILURE;
}

static MT_BOOL find(mt_u32 id, mt_u32 size, ulong *addr)
{
	struct shm_item_st *pItem;
	int i;

	if (g_avse_shm_manager != NULL)
	{
		pItem = g_avse_shm_manager->items;

		for (i=0; i<g_avse_shm_manager->count; i++)
		{
			if (pItem[i].id == id)
			{
				if (pItem[i].size != size)
				{
					//FIXME:
					PRINTF("[ERROR]%s: id[0x%x] size(%u vs. %u) not match!\n",__FUNCTION__,
							id, size, pItem[i].size);

					WARN_ON(pItem[i].size != size);
				}

				*addr = (ulong)g_avse_shm_manager->avse_shm_mmzbuf.startVirAddr + pItem[i].offset;
				PRINTF("%s: found id 0x%x -> addr 0x%lx\n",__FUNCTION__,
						id, *addr);
				return MT_TRUE;
			}
		}
	}
	else
	{
		PRINTF("[ERROR]%s: av secure shm manager not inited!\n",__FUNCTION__);
	}

	return MT_FALSE;
}

//---------------------------------------------------------------------------//

mt_s32 avse_shm_init(void)
{
	if (g_avse_shm_manager == NULL)
	{
		init_manager();
	}

	return (g_avse_shm_manager==NULL)?MT_FAILURE:MT_SUCCESS;
}

void avse_shm_deinit(void)
{
	if (g_avse_shm_manager != NULL)
	{
		mt_drv_mmz_unmap_and_release(&g_avse_shm_manager->avse_shm_mmzbuf);

		mt_drv_mmz_unmap_and_release(&manager_mmzbuf);

		g_avse_shm_manager = NULL;
	}
}

void *avse_shm_allocate(mt_u32 id, mt_u32 size)
{
	ulong addr = 0;

	//PRINTF("%s: id 0x%x, size %u\n", __FUNCTION__, id, size);

	if (size == 0)
	{
		PRINTF("[ERROR]%s: invalid parameter(id 0x%x, size %u)!\n",__FUNCTION__,
				id, size);
		return NULL;
	}

	if (g_avse_shm_manager == NULL)
	{
		init_manager();
	}

	if (!find(id, size, &addr))
	{
		allocate(id, size, &addr);
	}

	return (void*)addr;
}

void avse_shm_free(void *ptr)
{
	//do nothing
}

