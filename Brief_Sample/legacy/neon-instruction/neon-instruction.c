/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <arm_neon.h>

extern void neon_test(void);

int main(int argc, char **argv)
{
	neon_test();
	printf("done\n");
	return 0;
}
