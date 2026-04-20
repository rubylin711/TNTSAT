/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <sys/sdt.h>
#include "mt_ftrace.h"

extern void bar(int m, int n);

void bar(int m, int n)
{
	DTRACE_PROBE2(MT_USDT_NAMESPACE, test_lib, m, n);
}
