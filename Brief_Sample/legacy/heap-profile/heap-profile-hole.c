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

char *ap[0xa000];
char *bp[0xa000];
char *cp[0xa000];
char *dp[0xa000];
size_t size = 0x100;
int cnt = 0xa000;

void a(void)
{
	int i;
	for (i = 0; i < cnt; i++) {
		ap[i] = malloc(size);
		memset(ap[i], 'a', size);
	}
	printf("malloc ap done\n");
	getchar();
	b();
}

void b(void)
{
	int i;
	for (i = 0; i < cnt; i++) {
		bp[i] = malloc(size);
		memset(bp[i], 'b', size);
	}
	printf("malloc bp done\n");
	getchar();
	c();
}

void c(void)
{
	int i;
	for (i = 0; i < cnt; i++) {
		cp[i] = malloc(size);
		memset(cp[i], 'c', size);
	}
	printf("malloc cp done\n");
	getchar();
	d();
}

void d(void)
{
	int i;
	for (i = 0; i < cnt; i++) {
		dp[i] = malloc(size);
		memset(dp[i], 'd', size);
	}
	printf("malloc dp done\n");
	getchar();
}

int main(int argc, char **argv)
{
	int i;

	printf("test malloc\n");
	a();

	printf("test free\n");
	for (i = 0; i < cnt; i++) {
		free(ap[i]);
	}
	printf("free ap done\n");
	getchar();
	for (i = 0; i < cnt; i++) {
		free(bp[i]);
	}
	printf("free bp done\n");
	getchar();
	for (i = 0; i < cnt; i++) {
		free(cp[i]);
	}
	printf("free cp done\n");
	getchar();
	for (i = 0; i < cnt; i++) {
		free(dp[i]);
	}
	printf("free dp done\n");
	getchar();
	printf("free all done\n");

	printf("test malloc again\n");
	c();
	printf("malloc again done\n");

	printf("test free again\n");
	for (i = 0; i < cnt; i++) {
		free(dp[i]);
	}
	printf("free again dp done\n");
	getchar();
	for (i = 0; i < cnt; i++) {
		free(cp[i]);
	}
	printf("free again cp done\n");
	printf("free again all done\n");

	while (1) {
		sleep(100);
	}

	return 0;
}
