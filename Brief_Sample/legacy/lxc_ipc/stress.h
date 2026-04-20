/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __STRESS_H__
#define __STRESS_H__

#define STRESS_NAME "stress"

enum STRESS_FUN {
	STRESS_FUN_A,
	STRESS_FUN_B,
};

struct stress_fun_a_param {
	int a;
};

struct stress_fun_a_result {
	int r;
};

struct stress_fun_b_param {
	int p;
};

struct stress_detail {
	int x;
	int y;
	int z;
};

struct stress_fun_b_result {
	int r;
	struct stress_detail d;
};

#endif /* __STRESS_H__ */

