/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2019, Montage Technology Co., Ltd.
 *
 * File Name      : mtlz_file_source.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/03/14
 * Description    : File Source Component for Monage-LZ SW Player.
 * History        :
 * 1.Date         : 2019/03/14
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <stdio.h>

#include "mtlz_types.h"
#include "mtlzplayer_comp.h"

#ifdef __UC_OS__
static ulong file_source_open(const char *path, int oflag)
{
	return (ulong)mtlz_fopen(path, "rb");
}

static int file_source_close(ulong fd)
{
	mtlz_fclose((FILE*)fd);
	return MTLZ_SUCCESS;
}

static int file_source_eos(ulong fd)
{
	return mtlz_feof((FILE*)fd);
}

static ssize_t file_source_read(ulong fd, void *buf, size_t count)
{
	return (ssize_t)mtlz_fread(buf, 1, count, (FILE*)fd);
}
#else
static ulong file_source_open(const char *path, int oflag)
{
	return (ulong)fopen(path, "rb");
}

static int file_source_close(ulong fd)
{
	fclose((FILE*)fd);
	return MTLZ_SUCCESS;
}

static int file_source_eos(ulong fd)
{
	return feof((FILE*)fd);
}

static ssize_t file_source_read(ulong fd, void *buf, size_t count)
{
	return (ssize_t)fread(buf, 1, count, (FILE*)fd);
}
#endif

struct mtlzplayer_comp_source_st file_source_comp =
{
	.open	= file_source_open,
	.close	= file_source_close,
	.eos	= file_source_eos,
	.read	= file_source_read,
};

