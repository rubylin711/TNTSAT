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
size_t size = 0x100000;

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
	printf("parent\n");
	if (fork() == 0) {
		printf("client\n");
		execl("/usr/local/bin/heap-profile-client", "heap-profile-client", NULL);
		return 0;
	}
	printf("test malloc parent\n");
	a();
	printf("test free parent\n");
	free(ap);
	free(bp);
	free(cp);
	free(dp);
	printf("free done parent\n");
	printf("test malloc again parent\n");
	c();
	printf("malloc again done parent\n");

	while (1) {
		sleep(100);
	}

	return 0;
}
