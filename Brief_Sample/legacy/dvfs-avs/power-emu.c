/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	volatile double d0;
	volatile double d1;
	volatile double d2;
	volatile double d3;
	volatile double d4;
	volatile double d5;
	volatile double d6;
	volatile double d7;
	volatile double d8;
	volatile double d9;
	volatile double d10;
	volatile double d11;
	volatile double d12;
	volatile double d13;
	volatile double d14;
	volatile double d15;

	volatile double d16;
	volatile double d17;
	volatile double d18;
	volatile double d19;
	volatile double d20;
	volatile double d21;
	volatile double d22;
	volatile double d23;
	volatile double d24;
	volatile double d25;
	volatile double d26;
	volatile double d27;
	volatile double d28;
	volatile double d29;
	volatile double d30;
	volatile double d31;

	int i;

	i = atoi(argv[1]);

	while (i-- > 0) {
		d0 = 0.1 * 0.2 * (i + 1);
		d1 = 0.3 * 0.4 * (i + 1);
		d2 = 0.5 * 0.6 * (i + 1);
		d3 = 0.7 * 0.8 * (i + 1);
		d4 = 0.9 * 1.1 * (i + 1);
		d5 = 1.2 * 1.3 * (i + 1);
		d6 = 1.4 * 1.5 * (i + 1);
		d7 = 1.6 * 1.7 * (i + 1);
		d8 = d0 / d1;
		d9 = d2 / d3;
		d10 = d4 / d5;
		d11 = d6 / d7;
		d12 = d8 / d9;
		d13 = d10 / d11;
		d14 = d12 / d13;
		d15 = d14 * d14;

		d16 = 0.1 / 0.2 * (i + 1);
		d17 = 0.3 / 0.4 * (i + 1);
		d18 = 0.5 / 0.6 * (i + 1);
		d19 = 0.7 / 0.8 * (i + 1);
		d20 = 0.9 / 1.1 * (i + 1);
		d21 = 1.2 / 1.3 * (i + 1);
		d22 = 1.4 / 1.5 * (i + 1);
		d23 = 1.6 / 1.7 * (i + 1);
		d24 = d16 * d17;
		d25 = d18 * d19;
		d26 = d20 * d21;
		d27 = d22 * d23;
		d28 = d24 * d25;
		d29 = d26 * d27;
		d30 = d28 * d29;
		d31 = d30 * d30;
	}

	printf("%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
		d0, d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15);
	printf("%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
		d16, d17, d18, d19, d20, d21, d22, d23, d24, d25, d26, d27, d28, d29, d30, d31);

	return 0;

}
