/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2019, Montage Technology Co., Ltd.
 *
 * File Name      : mpi_ampshm.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/8/22
 * Description    : AVCPU Asymmetric Share Memory with APCPU. [Draft]
 * History        :
 * 1.Date         : 2019/8/22
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include "mt_type.h"
#include "mt_module_debug.h"
#include "mt_mpi_mem.h"
#include "mt_drv_ampshm.h"
#include "drv_ampshm_ioctl.h"

#define AMPSHM_DEVNAME "/dev/mt_ampshm"
static mt_s32 g_s32AmpshmFd = -1; /*ampshm device not open*/

static pthread_mutex_t g_ampshm_mutex = PTHREAD_MUTEX_INITIALIZER;

#define AMPSHM_LOCK(p_mutex)		pthread_mutex_lock(p_mutex)
#define AMPSHM_UNLOCK(p_mutex)		pthread_mutex_unlock(p_mutex)

/* one AMPSHM one file */
static struct mt_ampshm_file_st g_ampsh_files[MAX_SHM_BUFF_COUNT];

//---------------------------------------------------------------------------//

static int init(void)
{
	int i;

	memset(&g_ampsh_files[0], 0, MAX_SHM_BUFF_COUNT*sizeof(struct mt_ampshm_file_st));

	for (i=0; i<MAX_SHM_BUFF_COUNT; i++)
	{
		g_ampsh_files[i].fd = -1;
	}

	return 0;
}

static void deinit(void)
{
	//do nothing
}

static mt_s32 devicecheckopen(mt_void)
{
    AMPSHM_LOCK(&g_ampshm_mutex);

    if (-1 == g_s32AmpshmFd)
    {
        g_s32AmpshmFd = open(AMPSHM_DEVNAME, O_RDWR);
        if (-1 == g_s32AmpshmFd)
        {
            MT_FATAL_AMPSHM("Open ampshm device failed!\n");
            AMPSHM_UNLOCK(&g_ampshm_mutex);
            return  MT_FAILURE;
        }

        init();
    }

    AMPSHM_UNLOCK(&g_ampshm_mutex);

    return MT_SUCCESS;
}

static mt_s32 devicecheckclose(mt_void)
{
    AMPSHM_LOCK(&g_ampshm_mutex);

    if (g_s32AmpshmFd != -1)
    {
        (mt_void)close(g_s32AmpshmFd);
        g_s32AmpshmFd = -1;

        deinit();
    }

    AMPSHM_UNLOCK(&g_ampshm_mutex);

    return MT_SUCCESS;
}

static struct mt_ampshm_file_st *allocate_file(void)
{
	int i;

	for (i=0; i<MAX_SHM_BUFF_COUNT; i++)
	{
		if (g_ampsh_files[i].fd == -1)
		{
			return &g_ampsh_files[i];
		}
	}

	return NULL;
}

static struct mt_ampshm_file_st *find_by_name(const char *name)
{
	int i;

	for (i=0; i<MAX_SHM_BUFF_COUNT; i++)
	{
		if (g_ampsh_files[i].fd != -1
			&& g_ampsh_files[i].name[0] != 0
			&& strcmp(g_ampsh_files[i].name, name) == 0)
		{
			return &g_ampsh_files[i];
		}
	}

	return NULL;
}

static struct mt_ampshm_file_st *find_by_fd(int fd)
{
	int i;

	for (i=0; i<MAX_SHM_BUFF_COUNT; i++)
	{
		if (g_ampsh_files[i].fd != -1
			&& g_ampsh_files[i].fd == fd)
		{
			return &g_ampsh_files[i];
		}
	}

	return NULL;
}

static struct mt_ampshm_file_st *find_by_usr_vir_addr(mt_u32 usr_vir_addr)
{
	int i;

	for (i=0; i<MAX_SHM_BUFF_COUNT; i++)
	{
		if (g_ampsh_files[i].fd != -1
			&& g_ampsh_files[i].usr_vir_addr == usr_vir_addr)
		{
			return &g_ampsh_files[i];
		}
	}

	return NULL;
}

//---------------------------------------------------------------------------//

int mt_ampshm_init(void)
{
	return devicecheckopen();
}

void mt_ampshm_exit(void)
{
	devicecheckclose();
}

int mt_ampshm_open(const char *name, int oflag, mode_t mode)
{
	mt_s32 ret;
	struct mt_ampshm_file_st *fp;

	if (devicecheckopen() != MT_SUCCESS)
	{
		return (-1);
	}

	if (name == NULL)
	{
		MT_ERR_AMPSHM("invalid parameter!\n");
		return (-1);
	}

    AMPSHM_LOCK(&g_ampshm_mutex);

	if ((fp=find_by_name(name)) != NULL)
	{
		AMPSHM_UNLOCK(&g_ampshm_mutex);
		return fp->fd;
	}
	else
	{
		fp = allocate_file();

		if (fp != NULL)
		{
			strncpy(fp->name, name, MAX_SHM_NAME_LEN);
			fp->name[MAX_SHM_NAME_LEN] = '\0';
			//strlcpy(fp->name, name, sizeof(fp->name));
			fp->oflag = oflag;
			//FIXME
			fp->mode = (unsigned int)mode;

			ret = ioctl(g_s32AmpshmFd, CMD_AMPSHM_OPEN, fp);
			if (ret == MT_SUCCESS)
			{
				AMPSHM_UNLOCK(&g_ampshm_mutex);
				return fp->fd;
			}
			else
			{
				MT_ERR_AMPSHM("ioctl failed!\n");
			}
		}
		else
		{
			MT_ERR_AMPSHM("allocate_file failed!\n");
		}
	}

    AMPSHM_UNLOCK(&g_ampshm_mutex);
	return (-1);
}

int mt_ampshm_ftruncate(int fd, off_t length)
{
	mt_s32 ret = 0;
	struct mt_ampshm_file_st *fp;

	if (devicecheckopen() != MT_SUCCESS)
	{
		return (-1);
	}

	if (fd < 0 || length <= 0)
	{
		MT_ERR_AMPSHM("invalid parameter!\n");
		return (-1);
	}

    AMPSHM_LOCK(&g_ampshm_mutex);

	if ((fp=find_by_fd(fd)) != NULL)
	{
		//FIXME
		fp->length = (unsigned int)length;

		ret = ioctl(g_s32AmpshmFd, CMD_AMPSHM_FTRUNCATE, fp);

		AMPSHM_UNLOCK(&g_ampshm_mutex);
		return ret;
	}
	else
	{
		MT_ERR_AMPSHM("not find fd(%d)!\n",fd);
		AMPSHM_UNLOCK(&g_ampshm_mutex);
		return (-1);
	}
}

void *mt_ampshm_mmap(void *addr, size_t length, int prot, int flags,
		           int fd, off_t offset)
{
	mt_s32 ret;
	struct mt_ampshm_file_st *fp;

	if (devicecheckopen() != MT_SUCCESS)
	{
		return NULL;
	}

	if (fd < 0 || length == 0)
	{
		MT_ERR_AMPSHM("invalid parameter!\n");
		return NULL;
	}

    AMPSHM_LOCK(&g_ampshm_mutex);

	if ((fp=find_by_fd(fd)) != NULL)
	{
		if (length != (size_t)fp->length)
		{
			MT_ERR_AMPSHM("length(%u vs. %u) not match!\n",length,fp->length);
			AMPSHM_UNLOCK(&g_ampshm_mutex);
			return NULL;
		}

		/* already mapped */
		if (fp->usr_vir_addr != 0)
		{
			AMPSHM_UNLOCK(&g_ampshm_mutex);
			return (void*)fp->usr_vir_addr;
		}

		fp->prot = prot;
		fp->flags = flags;

		ret = ioctl(g_s32AmpshmFd, CMD_AMPSHM_MMAP, fp);
		if (ret == MT_SUCCESS)
		{
			if (fp->phy_addr != 0)
			{
				fp->usr_vir_addr = (ulong)mt_mmap(fp->phy_addr, length);
				AMPSHM_UNLOCK(&g_ampshm_mutex);
				return (void*)fp->usr_vir_addr;
			}
			else
			{
				MT_ERR_AMPSHM("physical address is NULL!\n");
			}
		}
		else
		{
			MT_ERR_AMPSHM("ioctl failed!\n");
		}
	}
	else
	{
		MT_ERR_AMPSHM("not find fd(%d)!\n",fd);
	}

	AMPSHM_UNLOCK(&g_ampshm_mutex);
	return NULL;
}

int mt_ampshm_munmap(void *addr, size_t length)
{
	mt_s32 ret = 0;
	struct mt_ampshm_file_st *fp;

	if (devicecheckopen() != MT_SUCCESS)
	{
		return (-1);
	}

	if (addr == NULL || length == 0)
	{
		MT_ERR_AMPSHM("invalid parameter!\n");
		return (-1);
	}

    AMPSHM_LOCK(&g_ampshm_mutex);

	if ((fp=find_by_usr_vir_addr((ulong)addr)) != NULL)
	{
		ret = mt_munmap((mt_void*)fp->usr_vir_addr);

		fp->usr_vir_addr = 0;

		ret |= ioctl(g_s32AmpshmFd, CMD_AMPSHM_MUNMAP, fp);

		AMPSHM_UNLOCK(&g_ampshm_mutex);
		return ret;
	}
	else
	{
		MT_ERR_AMPSHM("not find addr(%p)!\n",addr);
	}

	AMPSHM_UNLOCK(&g_ampshm_mutex);
	return (-1);
}

int mt_ampshm_unlink(const char *name)
{
	mt_s32 ret = 0;
	struct mt_ampshm_file_st *fp;

	if (devicecheckopen() != MT_SUCCESS)
	{
		return (-1);
	}

	if (name == NULL)
	{
		MT_ERR_AMPSHM("invalid parameter!\n");
		return (-1);
	}

    AMPSHM_LOCK(&g_ampshm_mutex);

	if ((fp=find_by_name(name)) != NULL)
	{
		ret = ioctl(g_s32AmpshmFd, CMD_AMPSHM_UNLINK, fp);

		AMPSHM_UNLOCK(&g_ampshm_mutex);
		return ret;
	}
	else
	{
		MT_ERR_AMPSHM("not find (%s)!\n",name);
	}

	AMPSHM_UNLOCK(&g_ampshm_mutex);
	return (-1);
}

//---------------------------------------------------------------------------//

#define TEST_ASSERT(expr)	do {				\
								if (!(expr)) {	\
									printf("[ASSERT]Test Failed @%s - %d!\n",__FUNCTION__,__LINE__);	\
								}				\
							}while(0)

/* TEST CASE*/
int test_ampshm_case01_nsecure(void);
int test_ampshm_case02_secure(void);

static void testcase(int flags)
{
	int i;
	int count = MAX_SHM_BUFF_COUNT + 1;
	char name[32];
	unsigned int id = 0x20190823;
	int ret;

	int fd[MAX_SHM_BUFF_COUNT + 1];
	ulong vir_addr[MAX_SHM_BUFF_COUNT + 1];

	int oflag = 0;
	mode_t mode = 0;
	off_t length = 0x2000;
	int prot = 0;

	for (i=0; i<count; i++)
	{
		printf("TEST[1]-%d\n",i);

		//1
		snprintf(name, 32, "0x%x", id+(unsigned int)i);
		fd[i] = mt_ampshm_open(name, oflag, mode);
		if (i < MAX_SHM_BUFF_COUNT)
		{
			TEST_ASSERT(fd[i] >= 0);
		}
		else
		{
			TEST_ASSERT(fd[i] < 0);
		}
		printf("open %s, return %d\n",name,fd[i]);

		//2
		ret = mt_ampshm_ftruncate(fd[i], length);
		if (i < MAX_SHM_BUFF_COUNT)
		{
			TEST_ASSERT(ret == 0);
		}
		else
		{
			TEST_ASSERT(ret != 0);
		}
		printf("ftruncate %lu, return %d\n",length,ret);

		//3
		vir_addr[i] = (ulong)mt_ampshm_mmap(NULL, (size_t)length, prot, flags, fd[i], 0);
		if (i < MAX_SHM_BUFF_COUNT)
		{
			TEST_ASSERT(vir_addr[i] != 0);
		}
		else
		{
			TEST_ASSERT(vir_addr[i] == 0);
		}
		printf("mmap %d, return 0x%lx\n",fd[i],vir_addr[i]);
	}

	for (i=0; i<count; i++)
	{
		printf("TEST[2]-%d\n",i);
		//4
		ret = mt_ampshm_munmap((void*)vir_addr[i], (size_t)length);
		if (i < MAX_SHM_BUFF_COUNT)
		{
			TEST_ASSERT(ret == 0);
		}
		else
		{
			TEST_ASSERT(ret != 0);
		}
		printf("unmap 0x%lx, return %d\n",vir_addr[i],ret);

		//5
		snprintf(name, 32, "0x%x", id+(unsigned int)i);
		ret = mt_ampshm_unlink(name);
		if (i < MAX_SHM_BUFF_COUNT)
		{
			TEST_ASSERT(ret == 0);
		}
		else
		{
			TEST_ASSERT(ret != 0);
		}
		printf("unlink %s, return %d\n",name,ret);
	}
}

int test_ampshm_case01_nsecure(void)
{
	printf("[AMPSHM] TEST CASE-1: none secure share memory\n");

	testcase(0);

	return 0;
}

int test_ampshm_case02_secure(void)
{
	printf("[AMPSHM] TEST CASE-2: secure share memory\n");

	testcase(MT_MAP_SECURE);

	return 0;
}

