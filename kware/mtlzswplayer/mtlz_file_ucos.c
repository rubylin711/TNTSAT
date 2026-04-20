/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2019, Montage Technology Co., Ltd.
 *
 * File Name      : mtlz_file_ucos.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/03/27
 * Description    : File porting APIs of uCOS.
 * History        :
 * 1.Date         : 2019/03/27
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <stdio.h>
#include <string.h>

#include "mt_type.h"
#include "sys_define.h"
#include "mtos_printk.h"
#include "drv_dev.h"
#include "ufs.h"

#define PRINTF					mtos_printk

//FILE *fopen(const char *path, const char *mode);
//int fclose(FILE *fp);
//int feof(FILE *stream);
//size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

struct mtlz_file_st
{
	ufs_file_t stream;

	unsigned long long size;
	unsigned long long offset;
};

static struct mtlz_file_st mtlz_files[1];

FILE *mtlz_fopen(const char *path, const char *mode)
{
	int ret;
    //FIXME: stack overflow!
    u16 w_path[256];
    u32 len = 256;
    //FIXME: multi files?
	struct mtlz_file_st *file = &mtlz_files[0];
	ufs_file_t *fp = &file->stream;
	op_mode_t op_mode = UFS_READ;

	if (path == NULL || mode == NULL)
	{
		PRINTF("[ERROR]%s: invalid parameter!\n",__FUNCTION__);
		return NULL;
	}

    ret = ufs_utf8_to_utf16((u8*)path, w_path, &len);
    if (ret < 0)
	{
		PRINTF("[ERROR]%s: convert (%s) to unicode failed!\n",__FUNCTION__,path);
		return NULL;
	}

	memset(fp, 0, sizeof(ufs_file_t));
	if (mode[0] == 'w' || mode[0] == 'a')
	{
		op_mode |= UFS_WRITE;
		op_mode |= UFS_CREATE_NEW;
	}
	ret = ufs_open(fp, w_path, op_mode);
	if (ret)
	{
		PRINTF("[ERROR]%s: ufs_open(%s) failed!\n",__FUNCTION__,path);
		return NULL;
	}

	ufs_lseek(fp, 0, UFS_SEEK_END);
	ufs_tell(fp, &file->size);
	ufs_lseek(fp, 0, UFS_SEEK_HEAD);
	file->offset = 0;

	PRINTF("%s: ufs_open(%s) success, file size=%llu.\n",__FUNCTION__,path,file->size);
	return (FILE*)file;
}

int mtlz_fclose(FILE *fp)
{
	int ret;
	ufs_file_t *stream;

	if (fp == NULL)
	{
		PRINTF("[ERROR]%s: invalid parameter!\n",__FUNCTION__);
		return -1;
	}

	stream = &((struct mtlz_file_st*)fp)->stream;
	ret = ufs_close(stream);
	if (ret)
	{
		PRINTF("[ERROR]%s: ufs_close failed!\n",__FUNCTION__);
		return -1;
	}

	return ret;
}

int mtlz_feof(FILE *stream)
{
	struct mtlz_file_st *file;

	if (stream == NULL)
	{
		PRINTF("[ERROR]%s: invalid parameter!\n",__FUNCTION__);
		return -1;
	}

	file = (struct mtlz_file_st*)stream;

	return (file->offset >= file->size)?1:0;
}

size_t mtlz_fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	int ret;
	size_t read_bytes;
	struct mtlz_file_st *file;
	ufs_file_t *fp;

	if (stream == NULL || ptr == NULL || size == 0 || nmemb == 0)
	{
		PRINTF("[ERROR]%s: invalid parameter!\n",__FUNCTION__);
		return 0;
	}

	file = (struct mtlz_file_st*)stream;
	fp = &file->stream;
    ret = ufs_read(fp, ptr, size*nmemb, (u32*)&read_bytes);

	if (ret)
	{
		PRINTF("[ERROR]%s: ufs_read failed!\n",__FUNCTION__);
		return 0;
	}

	file->offset += read_bytes;

	return read_bytes;
}

//if use aliasing fopen, then should NOT link libc!
#if 0
FILE *fopen(const char *path, const char *mode) __attribute__((alias("mtlz_fopen")));
int fclose(FILE *fp) __attribute__((alias("mtlz_fclose")));
int feof(FILE *stream) __attribute__((alias("mtlz_feof")));
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) __attribute__((alias("mtlz_fread")));
#endif

