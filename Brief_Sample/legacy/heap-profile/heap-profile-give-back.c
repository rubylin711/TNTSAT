/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
//#include <gperftools/malloc_extension_c.h>

extern void a(void);
extern void b(void);
extern void c(void);
extern void d(void);

char *ap;
char *bp;
char *cp;
char *dp;
size_t size = 0xa00000;

void a(void)
{
	ap = malloc(size);
	memset(ap, 'a', size);
	printf("malloc ap done\n");
	getchar();
	b();
}

void b(void)
{
	bp = malloc(size);
	memset(bp, 'b', size);
	printf("malloc bp done\n");
	getchar();
	c();
}

void c(void)
{
	cp = malloc(size);
	memset(cp, 'c', size);
	printf("malloc cp done\n");
	getchar();
	d();
}

void d(void)
{
	dp = malloc(size);
	memset(dp, 'd', size);
	printf("malloc dp done\n");
	getchar();
}

int main(int argc, char **argv)
{
	void *p;

	printf("test malloc\n");
	a();

	printf("test free\n");
	free(ap);
	printf("free ap done\n");
	getchar();
	free(bp);
	printf("free bp done\n");
	getchar();
	free(cp);
	printf("free cp done\n");
	getchar();
	free(dp);
	printf("free dp done\n");
	getchar();
	printf("free all done\n");

	printf("dlopen\n");
	p = dlopen("/usr/local/lib/libmt_common.so", RTLD_NOW | RTLD_LOCAL);
	printf("p = 0x%lx\n", (unsigned long)p);

	printf("test malloc again\n");
	c();
	printf("malloc again done\n");

	printf("test free again\n");
	free(cp);
	printf("free again cp done\n");
	getchar();
	free(dp);
	printf("free again dp done\n");
	printf("free again all done\n");

	//MallocExtension_ReleaseFreeMemory();

	while (1) {
		sleep(100);
	}

	return 0;
}
