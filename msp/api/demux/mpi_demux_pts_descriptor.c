/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2019, Montage Technology Co., Ltd.
 *
 * File Name      : mpi_demux_pts_descriptor.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/8/30
 * Description    : PTS Descriptor Parser
 * History        :
 * 1.Date         : 2019/8/30
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mt_type.h"
#include "mt_module_debug.h"
#include "mt_mpi_demux.h"

#define DEBUG_PTS	0

//#define PRINTF		printf
#define PRINTF(...)	do{}while(0)

#undef MALLOC
#define MALLOC		malloc

#undef FREE
#define FREE		free


/* parsed pts descriptor */
struct pts_descritpro_st
{
	MT_PTS64 origPTS;	/* 33bit PTS */
	MT_PTS64 u64PTS;	/* 64bit PTS in unit of microsecond */

	mt_u32 u32EsPhyAddr;
};

static const size_t demux_pts_descriptor_size = sizeof(MT_MPI_DMX_DESCRIPTOR_DATA_S);


//---------------------------------------------------------------------------//

static inline void dump_descriptor(struct pts_descritpro_st *pDesc)
{
	if (DEBUG_PTS)
	{
		PRINTF("Dump PTS Descriptor::\n");
		PRINTF("\tOrig 33bit PTS: 0x%llx, %llu\n",pDesc->origPTS,pDesc->origPTS);
		PRINTF("\t         PTS64: 0x%llx, %llu\n",pDesc->u64PTS,pDesc->u64PTS);
		PRINTF("\t   ES Phy ADDR: 0x%08x\n",pDesc->u32EsPhyAddr);
	}
}

/**
 * get const descriptor size
 */
size_t mpi_demux_desc_get_size(void)
{
	return demux_pts_descriptor_size;
}

/**
 * Only support size*count = Descriptor size.
 */
size_t mpi_demux_desc_read(void *buffer, size_t size, size_t count, mt_handle dmx_ch)
{
	ulong read, write;
	ulong descBufAddr;

	if (buffer == NULL)
	{
		MT_ERR_DEMUX("%s: buffer is null!\n",__FUNCTION__);
		return 0;
	}

	if (size != demux_pts_descriptor_size || count != 1)
	{
		MT_ERR_DEMUX("%s: invalid size(%u), count(%u)!\n",__FUNCTION__,size,count);
		return 0;
	}

	read = MT_MPI_DMX_GetCurrentDescBufferReadPoint(dmx_ch);
	write = MT_MPI_DMX_GetCurrentDescBufferWritePoint(dmx_ch);
	MT_INFO_DEMUX("%s: Ch %x, RD %x, WR %x\n",__FUNCTION__,dmx_ch,read,write);

	if (read != write)
	{
		MT_MPI_DMX_GetDescBuffAddr(dmx_ch, NULL, NULL, &descBufAddr, NULL);

		memcpy(buffer, (void*)(descBufAddr + read), size * count);

		if (DEBUG_PTS)
		{
			PRINTF("%s: Ch %x, RD %x, WR %x\n",__FUNCTION__,dmx_ch,read,write);
		}
		return (size * count);
	}
	else
	{
		MT_WARN_DEMUX("%s: desc buffer empty! Ch %x, RD %x, WR %x\n",__FUNCTION__,
			dmx_ch, read, write);
		return 0;
	}
}

/**
 * Only support seek from SEEK_CUR, and offset shall be an integral multiple
 * of Descriptor size.
 */
mt_s32 mpi_demux_desc_seek(mt_handle dmx_ch, long offset, int fromwhere)
{
	ulong read, write;
	ulong descBufSize;

	if (fromwhere != SEEK_CUR)
	{
		MT_ERR_DEMUX("%s: invalid fromwhere(%d)!\n",__FUNCTION__,fromwhere);
		return MT_FAILURE;
	}

	if (offset <= 0 || ((size_t)offset % demux_pts_descriptor_size) != 0)
	{
		MT_ERR_DEMUX("%s: invalid offset(%ld)!\n",__FUNCTION__,offset);
		return MT_FAILURE;
	}

	read = MT_MPI_DMX_GetCurrentDescBufferReadPoint(dmx_ch);
	write = MT_MPI_DMX_GetCurrentDescBufferWritePoint(dmx_ch);
	MT_INFO_DEMUX("%s: Ch %x, RD %x, WR %x\n",__FUNCTION__,dmx_ch,read,write);

	if (read != write)
	{
		MT_MPI_DMX_GetDescBuffAddr(dmx_ch, NULL, NULL, NULL,
								&descBufSize);

		read = (read + (mt_u32)offset) % descBufSize;

		MT_MPI_DMX_SetCurrentDescBufferReadPoint(dmx_ch, read);

		return MT_SUCCESS;
	}
	else
	{
		MT_WARN_DEMUX("%s: desc buffer empty! Ch %x, RD %x, WR %x\n",__FUNCTION__,
			dmx_ch, read, write);
		return MT_FAILURE;
	}
}

/**
 * @brief parse PTS Descriptor
 *
 * @return handle of parsed PTS Descriptor
 */
void *mpi_demux_desc_parse(void *buffer, size_t size)
{
	MT_MPI_DMX_DESCRIPTOR_DATA_S *pDesc;
	struct pts_descritpro_st *pOut;

	if (buffer == NULL || size != demux_pts_descriptor_size)
	{
		MT_ERR_DEMUX("%s: invalid buffer(%p), size(%u)!\n",__FUNCTION__,buffer,size);
		return NULL;
	}

	pDesc = (MT_MPI_DMX_DESCRIPTOR_DATA_S*)buffer;
	if (DEBUG_PTS)
	{
		PRINTF("Original Descriptor::\n");
		PRINTF("\tADDR: 0x%08x\n",pDesc->u32Addr);
		PRINTF("\t PTS: 0x%08x\n",pDesc->u32Pts);
		PRINTF("\tINFO: 0x%08x\n",pDesc->u32Info);
		PRINTF("\t DTS: 0x%08x\n",pDesc->u32Dts);
	}

	if ((pDesc->u32Pts & 0x80000000) == 0)
	{
		MT_WARN_DEMUX("%s: invalid PTS(0x%x) flag!\n",__FUNCTION__,pDesc->u32Pts);
		return NULL;
	}

	pOut = (struct pts_descritpro_st *)MALLOC(sizeof(struct pts_descritpro_st));
	if (pOut == NULL)
	{
		MT_ERR_DEMUX("%s: allocate descriptor failed!\n",__FUNCTION__);
		return NULL;
	}

	pOut->origPTS = (pDesc->u32Pts >> 1) & 0x3FFFFFFF;		/* bit[29:0] */
	pOut->origPTS |= ((pDesc->u32Info >> 24) & 0x03) << 30; /* bit[31:30] */
	pOut->origPTS = pOut->origPTS << 1;						/* Convert to 33bit */

	pOut->u64PTS = pOut->origPTS * 1000 / 90;		/* 33bit * 1000 should not overflow */
	pOut->u32EsPhyAddr = pDesc->u32Addr;
	dump_descriptor(pOut);

	return (void*)pOut;
}

void mpi_demux_desc_destroy(void *desc)
{
	if (desc == NULL)
	{
		MT_ERR_DEMUX("%s: desc is null!\n",__FUNCTION__);
		return;
	}

	FREE(desc);
}

/**
 * @brief get original 33bit PTS
 */
mt_s32 mpi_demux_desc_get_orig_pts(void *desc, MT_PTS64 *pts)
{
	struct pts_descritpro_st *pDesc;

	if (desc == NULL || pts == NULL)
	{
		MT_ERR_DEMUX("%s: invalid desc(%p), pts(%p)!\n",__FUNCTION__,desc,pts);
		return MT_FAILURE;
	}

	pDesc = (struct pts_descritpro_st*)desc;

	*pts = pDesc->origPTS;
	return MT_SUCCESS;
}

/**
 * @brief get 64bit PTS in unit of microsecond
 */
mt_s32 mpi_demux_desc_get_pts64(void *desc, MT_PTS64 *pts)
{
	struct pts_descritpro_st *pDesc;

	if (desc == NULL || pts == NULL)
	{
		MT_ERR_DEMUX("%s: invalid desc(%p), pts(%p)!\n",__FUNCTION__,desc,pts);
		return MT_FAILURE;
	}

	pDesc = (struct pts_descritpro_st*)desc;

	*pts = pDesc->u64PTS;
	return MT_SUCCESS;
}

/**
 * @brief get ES packet start physical address
 */
mt_s32 mpi_demux_desc_get_es_start_addr(void *desc, mt_u32 *addr)
{
	struct pts_descritpro_st *pDesc;

	if (desc == NULL || addr == NULL)
	{
		MT_ERR_DEMUX("%s: invalid desc(%p), addr(%p)!\n",__FUNCTION__,desc,addr);
		return MT_FAILURE;
	}

	pDesc = (struct pts_descritpro_st*)desc;

	*addr = pDesc->u32EsPhyAddr;
	return MT_SUCCESS;
}

//---------------------------------------------------------------------------//

void mpi_demux_desc_sample(mt_handle dmx_ch);

/*
 * get current PTS Descriptor and seek to next one.
 */
void *mpi_demux_desc_next(mt_handle dmx_ch)
{
	size_t size;
	mt_u8 buffer[32];
	void *descriptor;
	//mt_s32 ret;

	size = mpi_demux_desc_get_size();

	if (size > sizeof(buffer))
	{
		MT_ERR_DEMUX("%s: descriptor size(%u) > buffer size(%u)!\n",__FUNCTION__,
			size,sizeof(buffer));
		return NULL;
	}

	do
	{
		if (mpi_demux_desc_read((void*)buffer, size, 1, dmx_ch) != size)
		{
			return NULL;
		}

		mpi_demux_desc_seek(dmx_ch, (long)size, SEEK_CUR);

		descriptor = mpi_demux_desc_parse(buffer, size);
		if (descriptor != NULL)
		{
			return descriptor;
		}

	} while (1);
}

void mpi_demux_desc_sample(mt_handle dmx_ch)
{
	size_t size;
	mt_u8 buffer[32];
	void *descriptor;
	MT_PTS64 origPTS;
	MT_PTS64 u64PTS;
	mt_u32 u32EsAddr;
	mt_s32 ret;

	size = mpi_demux_desc_get_size();

	if (size > sizeof(buffer))
	{
		MT_ERR_DEMUX("%s: descriptor size(%u) > buffer size(%u)!\n",__FUNCTION__,
			size,sizeof(buffer));
		return;
	}

	if (mpi_demux_desc_read((void*)buffer, size, 1, dmx_ch) != size)
	{
		return;
	}

	descriptor = mpi_demux_desc_parse(buffer, size);
	if (descriptor == NULL)
	{
		goto seek_next;
	}

	ret = mpi_demux_desc_get_orig_pts(descriptor, &origPTS);
	if (ret == MT_SUCCESS)
	{
		//PRINTF("%s: orig pts=0x%llx, %llu\n",__FUNCTION__,origPTS,origPTS);
	}
	else
	{
		PRINTF("[ERROR]%s: get orig pts failed!\n",__FUNCTION__);
	}

	ret = mpi_demux_desc_get_pts64(descriptor, &u64PTS);
	if (ret == MT_SUCCESS)
	{
		//PRINTF("%s: pts64=0x%llx, %llu\n",__FUNCTION__,u64PTS,u64PTS);
	}
	else
	{
		PRINTF("[ERROR]%s: get pts64 failed!\n",__FUNCTION__);
	}

	ret = mpi_demux_desc_get_es_start_addr(descriptor, &u32EsAddr);
	if (ret == MT_SUCCESS)
	{
		//PRINTF("%s: es addr=0x%08x\n",__FUNCTION__,u32EsAddr);
	}
	else
	{
		PRINTF("[ERROR]%s: get es start addr failed!\n",__FUNCTION__);
	}

	mpi_demux_desc_destroy(descriptor);
	descriptor = NULL;

seek_next:
	ret = mpi_demux_desc_seek(dmx_ch, (long)size, SEEK_CUR);
	if (ret != MT_SUCCESS)
	{
		PRINTF("[ERROR]%s: desc seek failed!\n",__FUNCTION__);
	}
}

