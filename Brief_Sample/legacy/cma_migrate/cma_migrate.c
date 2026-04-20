/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <mt_common.h>
#include <sys/mman.h>

#define MALLOC_SIZE (20*1024*1024)
#define MMZ_SIZE (10*1024*1024)

static void *start_routine(void *p);

void *start_routine(void *p)
{
	int i;
	while (1) {
		for (i = 0; i < (PAGE_SIZE / 4); i++) {
			*(int *)((int *)p + i) = i;
		}
	}

	return NULL;
}

int main(int argc, char **argv)
{
	void *p;
	int ret;
	mt_mmz_buf_s mmz_buf;
	char c;
	int i = MALLOC_SIZE;
	pthread_t thread;

	p = malloc(MALLOC_SIZE);
	for (i = 0; i < (MALLOC_SIZE / 4); i++) {
		*(int *)((int *)p + i) = i;
	}

	pthread_create(&thread, NULL, start_routine, p);

#if 1
	mlock(p, MALLOC_SIZE);
#endif

	printf("stage1\n");
	scanf("%c", &c);

	mmz_buf.bufsize = MMZ_SIZE;
	strcpy(mmz_buf.bufname, "xxoo");
	ret = mt_mmz_malloc(&mmz_buf);
	printf("mt_mmz_malloc: %d\n", ret);

	printf("stage2\n");
	scanf("%c", &c);

	return 0;
}
