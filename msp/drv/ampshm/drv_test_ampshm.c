/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : drv_test_ampshm.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2017/12/09
 * Description    : MT share memory test function.
 * History        :
 * 1.Date         : 2017/12/09
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <linux/delay.h>

static void drv_test_ampshm(void)
{
	int fd1, fd2;
	int ret;
	void *addr1;
	void *addr2;
	unsigned int *int_ptr;
	unsigned int counter = 1;

	MT_INFO_LOG("\ndrv_test_ampshm start\n");

	//create fd1
	fd1 = mt_ampshm_open("ampshm1", 0, 0);
	if (fd1 < 0)
	{
		MT_ERR_LOG("mt_ampshm_open ampshm1 FAIL!\n");
		return;
	}

	ret = mt_ampshm_ftruncate(fd1, 4096);
	if (ret != 0)
	{
		MT_ERR_LOG("mt_ampshm_ftruncate fd1 FAIL!\n");
		return;
	}

	addr1 = mt_ampshm_mmap(NULL, 4096, 0, 0, fd1, 0);
	if (addr1 == NULL)
	{
		MT_ERR_LOG("mt_ampshm_mmap fd1 FAIL!\n");
		return;
	}

	//fd2
	fd2 = mt_ampshm_open("ampshm2", 0, 0);
	if (fd2 < 0)
	{
		MT_ERR_LOG("mt_ampshm_open ampshm2 FAIL!\n");
		return;
	}

	ret = mt_ampshm_ftruncate(fd2, 4096);
	if (ret != 0)
	{
		MT_ERR_LOG("mt_ampshm_ftruncate fd2 FAIL!\n");
		return;
	}

	addr2 = mt_ampshm_mmap(NULL, 4096, 0, 0, fd2, 0);
	if (addr1 == NULL)
	{
		MT_ERR_LOG("mt_ampshm_mmap fd2 FAIL!\n");
		return;
	}

	//read/write test
	while (1)
	{
		//write fd1
		MT_INFO_LOG("write %p, value %x\n",addr1,counter);
		int_ptr = (unsigned int *)addr1;
		*int_ptr = counter;

		//read fd2
		int_ptr = (unsigned int *)addr2;
		while (*int_ptr != counter)
			msleep(10);

		MT_ERR_LOG("read %p, value %x\n",addr2,counter); 
		counter ++;
	}

	//unlink fd1
	ret = mt_ampshm_munmap(addr1, 4096);
	if (ret != 0)
	{
		MT_ERR_LOG("mt_ampshm_munmap fd1 FAIL!\n");
		return;
	}

	ret = mt_ampshm_unlink("ampshm1");
	if (ret != 0)
	{
		MT_ERR_LOG("mt_ampshm_unlink ampshm1 FAIL!\n");
		return;
	}

	//unlink fd2
	ret = mt_ampshm_munmap(addr2, 4096);
	if (ret != 0)
	{
		MT_ERR_LOG("mt_ampshm_munmap fd2 FAIL!\n");
		return;
	}

	ret = mt_ampshm_unlink("ampshm2");
	if (ret != 0)
	{
		MT_ERR_LOG("mt_ampshm_unlink ampshm2 FAIL!\n");
		return;
	}

	MT_INFO_LOG("\ndrv_test_ampshm end\n");
}

