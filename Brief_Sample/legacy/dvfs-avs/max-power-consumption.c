/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <sys/mman.h>
#include "mt_common.h"

extern void test_start(void);

int main(int argc, char **argv)
{
	void *p;
	int i;

#if defined(CONFIG_AARCH64)
	p = mmap((void *)(0xaaaaa000UL), PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	printf("mmap virt addr: 0x%lx\n", (ulong)p);

	p = mmap((void *)(0x55555000UL), PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	printf("mmap virt addr: 0x%lx\n", (ulong)p);

	i = 0x1080000;		/* approximate linpack elapse */
#elif defined(CONFIG_ARM)
	p = mmap((void *)(0x001ff000UL), PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	printf("mmap virt addr: 0x%lx\n", (ulong)p);

	p = mmap((void *)(0x00120000UL), PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	printf("mmap virt addr: 0x%lx\n", (ulong)p);

	p = mmap((void *)(0x00100000UL), PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	printf("mmap virt addr: 0x%lx\n", (ulong)p);

	p = mmap((void *)(0x002ff000UL), PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	printf("mmap virt addr: 0x%lx\n", (ulong)p);

	p = mmap((void *)(0x00040000UL), PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	printf("mmap virt addr: 0x%lx\n", (ulong)p);

	p = mmap((void *)(0x00020000UL), PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	printf("mmap virt addr: 0x%lx\n", (ulong)p);

	p = mmap((void *)(0x000ff000UL), PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	printf("mmap virt addr: 0x%lx\n", (ulong)p);

	i = 0x1080000;		/* approximate linpack elapse */
#endif

	while (i--) {
		test_start();
	}

	return 0;
}
