/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <execinfo.h>

#define BACKTRACE_SIZ 100

extern void foo(void);
extern void b(void);
extern void a(void);

void foo(void)
{
	void *array[BACKTRACE_SIZ];
	int size, i;
	char **strings;

	size = backtrace(array, BACKTRACE_SIZ);
	strings = backtrace_symbols(array, size);

	printf("size = %ld\n", (long)size);
	for (i = 0; i < size; ++i) {
		printf("%p : %s\n", array[i], strings[i]);
	}

	printf("---------------------------------------------------------\n");
	free(strings);
}

int main(int argc, char **argv)
{
	a();
	return 0;
}

