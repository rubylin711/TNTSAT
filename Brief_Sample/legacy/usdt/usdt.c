/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <sys/sdt.h>
#include <stdlib.h>
#include <stdio.h>
#include "mt_ftrace.h"

extern void bar(int m, int n);

static void foo(int x, int y)
{
	DTRACE_PROBE2(MT_USDT_NAMESPACE, MT_USDT_MARK2, x, y);
}

int main(int argc, char **argv)
{
	int a = atoi(argv[1]);
	int b = atoi(argv[2]);
	int c = 5;

	DTRACE_PROBE3(MT_USDT_NAMESPACE, montage_1, 1, 2, 3);
	DTRACE_PROBE(MT_USDT_NAMESPACE, MT_USDT_MARK0);
	DTRACE_PROBE1(MT_USDT_NAMESPACE, MT_USDT_MARK1, a);
	printf("param: %d %d\n", a, b);
	foo(a, b);
	bar(b, a);
	DTRACE_PROBE3(MT_USDT_NAMESPACE, montage_2, 10, 100, 1000);
	DTRACE_PROBE1(MT_USDT_NAMESPACE, montage_3, c);
	return 0;
}
