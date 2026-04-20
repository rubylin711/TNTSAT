/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2024 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include "mt_type.h"

#define U64 unsigned long long
#define S64 long long
typedef unsigned long long mt_u64;
typedef long long mt_s64;
typedef unsigned long long u64;
typedef long long s64;

#if 1
typedef U64 phys_addr_t;
typedef mt_u64 phys_addr_t;
typedef u64 phys_addr_t;
#else
typedef S64 phys_addr_t;
typedef mt_s64 phys_addr_t;
typedef s64 phys_addr_t;
#endif

int main(int argc, char **argv)
{
	phys_addr_t a = atoll(argv[1]);
	return (int)a;
}
