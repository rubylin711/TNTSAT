/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __LISTENER3_H__
#define __LISTENER3_H__

enum LISTENER3_FUN {
	LISTENER3_FUN_E,
	LISTENER3_FUN_EXIT,
};

struct listener3_fun_e_param {
	char f[0];
};

struct listener3_fun_e_result {
	int r;
	char content[0];
};

#endif /* __LISTENER3_H__ */

