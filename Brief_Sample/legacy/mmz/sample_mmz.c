/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include "mt_common.h"
#include "mt_mpi_mem.h"

int main(int argc, char **argv)
{
	mt_mmz_buf_s mt_mmz_buf1;
	mt_mmz_buf_s mt_mmz_buf2;
	phys_addr_t PhysAddr1;
	phys_addr_t PhysAddr2;
	mt_void *p1;
	mt_void *p2;
	char new1[] = "test_new_mmz1";
	char new2[] = "test_new_mmz2";
	char yes[16];
	phys_addr_t phys_start;
	ulong size;
	pid_t pid;
	int fd;

	printf("begin\n");
	getchar();

	mt_mmz_buf1.bufsize = 100;
	strcpy(mt_mmz_buf1.bufname, "test_alloc_mmz1");
	mt_mmz_buf2.bufsize = 1000;
	strcpy(mt_mmz_buf2.bufname, "test_alloc_mmz2");

	printf("test 1 mt_mmz_malloc\n");
	mt_mmz_malloc(&mt_mmz_buf1);
	printf("mt_mmz_malloc 1: phy = 0x%llx, virt = 0x%lx, size = %ld, name = %s\n", mt_mmz_buf1.phyaddr, (unsigned long)mt_mmz_buf1.user_viraddr, mt_mmz_buf1.bufsize, mt_mmz_buf1.bufname);
	printf("test 1 mt_mmz_malloc access gap ? yes : no\n");
	scanf("%s", yes);
	getchar();
	if (!strcmp(yes, "yes")) {
		printf("cat /proc/${PID}/maps and /proc/media-mem to see virtual address range\n");
		printf("[0x%lx] = 0x%x, [0x%lx] = 0x%x\n", (unsigned long)&(((unsigned char *)(mt_mmz_buf1.user_viraddr))[0]), ((unsigned char *)mt_mmz_buf1.user_viraddr)[0], (unsigned long)&(((unsigned char *)(mt_mmz_buf1.user_viraddr))[4095]), ((unsigned char *)mt_mmz_buf1.user_viraddr)[4095]);
		sleep(1);
		printf("[0x%lx] = 0x%x\n", (unsigned long)&(((unsigned char *)(mt_mmz_buf1.user_viraddr))[4096]), ((unsigned char *)mt_mmz_buf1.user_viraddr)[4096]);
		sleep(1);
		printf("[0x%lx] = 0x%x\n", (unsigned long)&(((unsigned char *)(mt_mmz_buf1.user_viraddr))[-1]), ((unsigned char *)mt_mmz_buf1.user_viraddr)[-1]);
	}

	printf("test get phyaddr\n");
	getchar();

	phys_start = 0;
	mt_mmz_get_phyaddr(mt_mmz_buf1.user_viraddr + 0x10, &phys_start, &size);
	printf("mt_mmz_get_phyaddr: phys_start = %llx, size = %ld\n", (u64)phys_start, size);
	phys_start = 0;
	mt_mem_get_phyaddr(mt_mmz_buf1.user_viraddr + 0x100, &phys_start);
	printf("mt_mem_get_phyaddr: phys_start = %llx\n", (u64)phys_start);

	printf("test fork\n");
	getchar();

	pid = fork();
	if (pid == 0) {
		int ret;
		printf("child process sleep 5s, then execl\n");
		sleep(5);
		ret = execl("/bin/ls", "ls", "-a", NULL);
		printf("ret = %d, errno = %d\n", ret, errno);
		exit(0);
	} else if (pid == -1) {
		printf("fork failed\n");
	}

	printf("test dio write file\n");
	getchar();
	fd = open("/media/sda1/dio_write_test",
			O_CREAT | O_RDWR | O_DIRECT | O_SYNC,
			S_IRWXU | S_IRWXG | S_IRWXO);
	if (fd == -1) {
		printf("open file failed\n");
	} else {
		ssize_t ret;
		memset(mt_mmz_buf1.user_viraddr, 0xff, 4096);	//mmz keep align with page
		ret = write(fd, mt_mmz_buf1.user_viraddr, 4096);
		close(fd);
		printf("ret = %d\n", (int)ret);
	}

	printf("test 1 mt_mmz_free\n");
	getchar();

	printf("free before child process done, free will fail\n");
	printf("free after child process done, free will success\n");
	mt_mmz_free(&mt_mmz_buf1);
	waitpid(pid, NULL, 0);

	printf("test 2 mt_mmz_malloc\n");
	getchar();
	mt_mmz_malloc(&mt_mmz_buf1);
	mt_mmz_malloc(&mt_mmz_buf2);
	printf("mt_mmz_malloc 1: phy = 0x%llx, virt = 0x%lx, size = %ld, name = %s\n", mt_mmz_buf1.phyaddr, (unsigned long)mt_mmz_buf1.user_viraddr, mt_mmz_buf1.bufsize, mt_mmz_buf1.bufname);
	printf("mt_mmz_malloc 2: phy = 0x%llx, virt = 0x%lx, size = %ld, name = %s\n", mt_mmz_buf2.phyaddr, (unsigned long)mt_mmz_buf2.user_viraddr, mt_mmz_buf2.bufsize, mt_mmz_buf2.bufname);
	printf("test 2 mt_mmz_free\n");
	getchar();
	mt_mmz_free(&mt_mmz_buf1);
	mt_mmz_free(&mt_mmz_buf2);

	printf("test 1 mt_mmz_new\n");
	getchar();
	PhysAddr1 = mt_mmz_new(100, 0, NULL, new1);
	printf("test 1 mt_mmz_map\n");
	getchar();
	p1 = mt_mmz_map(PhysAddr1, 1);
	printf("mt_mmz_map 1 : phy = 0x%llx, virt = 0x%lx\n", PhysAddr1, (unsigned long)p1);

	printf("test read and write mmz\n");
	getchar();
	printf("old *(ulong *)(0x%ld) = %ld\n", (ulong)(p1), *(ulong *)(p1));
	printf("modify to 1234\n");
	*(ulong *)(p1) = 1234;
	printf("new *(ulong *)(0x%ld) = %ld\n", (ulong)(p1), *(ulong *)(p1));
	if (*(ulong *)(p1) != 1234) {
		printf("test read write failed\n");
		return -1;
	}

	printf("test flush and invalid cache\n");
	getchar();
	mt_mmz_flush(p1, 0, 0);
	*(ulong *)(p1) = 5678;
	mt_mmz_invalidate(p1, 0, 0);

	printf("test 1 mt_mmz_unmap\n");
	getchar();
	mt_mmz_unmap(p1);
	printf("test 1 mt_mmz_delete\n");
	getchar();
	mt_mmz_delete(PhysAddr1);

	printf("test 2 mt_mmz_new\n");
	getchar();
	PhysAddr1 = mt_mmz_new(100, 0, NULL, new1);
	PhysAddr2 = mt_mmz_new(100, 0, NULL, new2);
	printf("test 2 mt_mmz_map\n");
	getchar();
	p1 = mt_mmz_map(PhysAddr1, 1);
	p2 = mt_mmz_map(PhysAddr2, 0);
	printf("mt_mmz_map 1 : phy = 0x%llx, virt = 0x%lx\n", PhysAddr1, (unsigned long)p1);
	printf("mt_mmz_map 2 : phy = 0x%llx, virt = 0x%lx\n", PhysAddr2, (unsigned long)p2);
	printf("test 2 mt_mmz_unmap\n");
	getchar();
	mt_mmz_unmap(p1);
	mt_mmz_unmap(p2);
	printf("test 2 mt_mmz_delete\n");
	getchar();
	mt_mmz_delete(PhysAddr1);
	mt_mmz_delete(PhysAddr2);

	printf("test 1 mt_mmz_new with 2 mt_mmz_map\n");
	getchar();
	PhysAddr1 = (mt_u32)mt_mmz_new(100, 0, NULL, new1);
	printf("test 1 mt_mmz_map\n");
	getchar();
	p1 = mt_mmz_map(PhysAddr1, 1);
	printf("mt_mmz_map 1 : phy = 0x%llx, virt = 0x%lx\n", PhysAddr1, (unsigned long)p1);
	printf("test 1 mt_mmz_map again\n");
	getchar();
	p2 = mt_mmz_map(PhysAddr1, 1);
	//p2 = mt_mmz_map(PhysAddr1, 0);
	printf("mt_mmz_map 2 : phy = 0x%llx, virt = 0x%lx\n", PhysAddr1, (unsigned long)p2);
	printf("test 1 mt_mmz_unmap\n");
	getchar();
	mt_mmz_unmap(p1);
	printf("test 1 mt_mmz_unmap again\n");
	getchar();
	mt_mmz_unmap(p1);
	printf("test 1 mt_mmz_delete\n");
	getchar();
	mt_mmz_delete(PhysAddr1);

	printf("test 1 mt_mmz_new with 2 mt_mmap\n");
	getchar();
	PhysAddr1 = (mt_u32)mt_mmz_new(100, 0, NULL, new1);
	printf("test 1 mt_mmap\n");
	getchar();
	p1 = mt_mmap(PhysAddr1, 100);
	printf("mt_mmap 1 : phy = 0x%llx, virt = 0x%lx\n", PhysAddr1, (unsigned long)p1);
	printf("test 1 mt_mmap_cache again\n");
	getchar();
	p2 = mt_mmap_cache(PhysAddr1, 100);
	printf("mt_mmap_cache 2 : phy = 0x%llx, virt = 0x%lx\n", PhysAddr1, (unsigned long)p2);
	printf("test 1 mt_munmap\n");
	getchar();
	mt_munmap(p1);
	printf("test 1 mt_munmap again\n");
	getchar();
	mt_munmap(p2);
	printf("test 1 mt_mmz_delete\n");
	getchar();
	mt_mmz_delete(PhysAddr1);


	printf("end\n");
	getchar();

	printf("get mmz start and size\n");
	mt_mmz_get_start_size("ddr", &phys_start, &size);
	printf("%s: start = %llx, size = %lx\n", "ddr", phys_start, size);
	mt_mmz_get_start_size("av", &phys_start, &size);
	printf("%s: start = %llx, size = %lx\n", "av", phys_start, size);
	mt_mmz_get_start_size("audio", &phys_start, &size);
	printf("%s: start = %llx, size = %lx\n", "audio", phys_start, size);
	mt_mmz_get_start_size("pcm", &phys_start, &size);
	printf("%s: start = %llx, size = %lx\n", "pcm", phys_start, size);

	return 0;
}
