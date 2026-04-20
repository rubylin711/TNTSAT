/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "mt_common.h"

int main(int argc, char **argv)
{
	mt_mmz_buf_s stBuf;
	int fd = -1;
	int ret;

	snprintf(stBuf.bufname, sizeof(stBuf.bufname), "test gup");
	stBuf.bufsize = 0x40000;

	mt_sys_init();
	mt_mmz_malloc(&stBuf);
	printf("stBuf.user_viraddr = %p\n", stBuf.user_viraddr);

#if 1
	printf("dio\n");
	fd = open("/media/sda1/gup", O_CREAT | O_DIRECT | O_SYNC | O_RDWR, 0777);
#else
	printf("page cache\n");
	fd = open("/media/sda1/gup", O_CREAT | O_RDWR, 0777);
#endif
	memset(stBuf.user_viraddr, 0xf1, stBuf.bufsize);

	printf("\n press enter to write 1\n");
	getchar();
	lseek(fd, 0, SEEK_SET);
	ret = write(fd, stBuf.user_viraddr + 0, 0x1000);
	printf("write 1 hope %d, autual %d\n", 0x1000, ret);
	if (ret < 0) {
		printf("errno = %d\n", errno);
	}

	printf("\n press enter to write 2\n");
	getchar();
	lseek(fd, 0x1000, SEEK_SET);
	ret = write(fd, stBuf.user_viraddr + 0x1000, 0x2000);
	printf("write 2 hope %d, autual %d\n", 0x2000, ret);
	if (ret < 0) {
		printf("errno = %d\n", errno);
	}

	printf("\n press enter to write 3\n");
	getchar();
	lseek(fd, 0, SEEK_SET);
	ret = write(fd, stBuf.user_viraddr + 0, stBuf.bufsize);
	printf("write 3 hope %lu, autual %d\n", stBuf.bufsize, ret);
	if (ret < 0) {
		printf("errno = %d\n", errno);
	}

	printf("\n press enter to write 4\n");
	getchar();
	lseek(fd, -0x2000, SEEK_END);
	ret = write(fd, stBuf.user_viraddr + stBuf.bufsize - 0x2000, 0x2000);
	printf("write 4 hope %d, autual %d\n", 0x2000, ret);
	if (ret < 0) {
		printf("errno = %d\n", errno);
	}

	printf("\n press enter to write 5\n");
	getchar();
	lseek(fd, -0x1000, SEEK_END);
	ret = write(fd, stBuf.user_viraddr + stBuf.bufsize - 0x1000, 0x2000);
	printf("write 5 hope %d, autual %d\n", 0x2000, ret);
	if (ret < 0) {
		printf("errno = %d\n", errno);
	}

	printf("\n press enter to write 6\n");
	getchar();
	lseek(fd, 0, SEEK_END);
	ret = write(fd, stBuf.user_viraddr + stBuf.bufsize, 0x1000);
	printf("write 6 hope %d, autual %d\n", 0x1000, ret);
	if (ret < 0) {
		printf("errno = %d\n", errno);
	}

	printf("\n press enter to write 7\n");
	getchar();
	lseek(fd, 0, SEEK_END);
	ret = write(fd, stBuf.user_viraddr + stBuf.bufsize, 0x1000);
	printf("write 7 hope %d, autual %d\n", 0x1000, ret);
	if (ret < 0) {
		printf("errno = %d\n", errno);
	}

	printf("\n press enter to write 8\n");
	getchar();
	lseek(fd, 0, SEEK_END);
	ret = write(fd, stBuf.user_viraddr - 0x1000, 0x1000);
	printf("write 8 hope %d, autual %d\n", 0x1000, ret);
	if (ret < 0) {
		printf("errno = %d\n", errno);
	}

	printf("\n press enter to write 9\n");
	getchar();
	lseek(fd, 0, SEEK_END);
	ret = write(fd, stBuf.user_viraddr - 0x1000, 0x2000);
	printf("write 9 hope %d, autual %d\n", 0x2000, ret);
	if (ret < 0) {
		printf("errno = %d\n", errno);
	}

	printf("\n press enter to write 10\n");
	getchar();
	lseek(fd, 0, SEEK_END);
	ret = write(fd, stBuf.user_viraddr - 0x2000, 0x3000);
	printf("write 10 hope %d, autual %d\n", 0x3000, ret);
	if (ret < 0) {
		printf("errno = %d\n", errno);
	}

	sleep(2);
	printf("press enter to quit\n");
	getchar();

	close(fd);
	mt_mmz_free(&stBuf);

	return 0;
}

