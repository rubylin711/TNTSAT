/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mt_mpi_mem.h"

char buf[1024];

int main(int argc, char **argv)
{
	int fd = -1;

	printf("pid: %d\n", getpid());

	memset(buf, 0, sizeof(buf));
	printf("buf: %p\n", (void *)buf);

	fd = open("/media/sda1/watchpoint-test", O_RDWR, 0666);
	printf("fd = %d\n", fd);
	if (fd == -1) {
		return -1;
	}

	if (atoi(argv[1])) {
		printf("walk\n");
		mt_walk_watch(buf, 1, 0);
	}

	printf("enter to access\n");
	getchar();

	read(fd, buf, 10);
	close(fd);

	printf("access done\n");
	getchar();

	return 0;
}
