/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

extern void a(void);
extern void b(void);
extern void c(void);
extern void d(void);

char *ap;
char *bp;
char *cp;
char *dp;
size_t size = 0x80000;

void a(void)
{
	ap = malloc(size);
	b();
}

void b(void)
{
	bp = malloc(size);
	c();
}

void c(void)
{
	cp = malloc(size);
	d();
}

void d(void)
{
	dp = malloc(size);
}

int main(int argc, char **argv)
{
	printf("test malloc client\n");
	a();
	printf("test free client\n");
	free(ap);
	free(bp);
	free(cp);
	free(dp);
	printf("free done client\n");
	printf("test malloc again client\n");
	c();
	printf("malloc again done client\n");

	while (1) {
		sleep(100);
	}

	return 0;
}
