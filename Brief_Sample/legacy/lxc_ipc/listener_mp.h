/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __LISTENER_MP_H__
#define __LISTENER_MP_H__

enum LISTENER_MP_FUN {
	LISTENER_MP_FUN_F,
	LISTENER_MP_FUN_EXIT,
};

struct listener_mp_fun_f_param {
	char f[0];
};

struct listener_mp_fun_f_result {
	int r;
	char content[0];
};

#endif /* __LISTENER_MP_H__ */

