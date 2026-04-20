/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __LISTENER1_H__
#define __LISTENER1_H__

enum LISTENER1_FUN {
	LISTENER1_FUN_A,
	LISTENER1_FUN_B,
	LISTENER1_FUN_EXIT,
};

struct listener1_fun_a_param {
	int a;
};

struct listener1_fun_a_result {
	int r;
};

struct listener1_fun_b_param {
	int p;
};

struct detail {
	int x;
	int y;
	int z;
};

struct listener1_fun_b_result {
	int r;
	struct detail d;
};

#endif /* __LISTENER1_H__ */
