/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "sanitize.h"

int n;
volatile int m;
int A;



void *tsan0_thread(void *param)
{
	while (1) {
		m = 0;
	}
}

int main(int argc, char **argv)
{
	pthread_t thread;
	pthread_t tsan0, tsan1;
	int xx;
	unsigned char oo[] = {240, 160, 1, 255};
	unsigned char *tmp = oo;

	printf("00 11 22 33\n");

	xx = ((unsigned int)tmp[0] << 24);
	xx = ((unsigned int)tmp[1] << 24);
	xx = ((unsigned int)tmp[2] << 24);
	xx = ((unsigned int)tmp[3] << 24);
	xx = ((int)tmp[0] << 24);
	xx = ((int)tmp[1] << 24);
	xx = ((int)tmp[2] << 24);
	xx = ((int)tmp[3] << 24);
	xx = (tmp[0] << 24);
	xx = (tmp[1] << 24);
	xx = (tmp[2] << 24);
	xx = (tmp[3] << 24);

	pthread_create(&thread, NULL, sanitize, malloc(16*1024));
	pthread_create(&tsan0, NULL, tsan0_thread, NULL);
	pthread_create(&tsan1, NULL, tsan1_thread, NULL);
	pthread_join(thread, NULL);
	exit(0);
	return 0;
}

