/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	double d1;
	double d2;
	double d3;

	float f1;
	float f2;
	float f3;

	printf("test float\n");

	printf("input d1: \n");
	scanf("%lf", &d1);

	printf("input d2: \n");
	scanf("%lf", &d2);

	d3 = d1 / d2;
	printf("%f / %f = %f\n", d1, d2, d3);

	d3 = d1 * d2;
	printf("%f * %f = %f\n", d1, d2, d3);


	printf("input f1: \n");
	scanf("%f", &f1);

	printf("input f2: \n");
	scanf("%f", &f2);

	f3 = f1 / f2;
	printf("%f / %f = %f\n", f1, f2, f3);

	f3 = f1 * f2;
	printf("%f * %f = %f\n", f1, f2, f3);

	return 0;
}

