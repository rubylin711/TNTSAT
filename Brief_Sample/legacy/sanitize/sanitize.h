/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
extern volatile int m;
extern void *sanitize(void *param);
extern void *tsan0_thread(void *param);
extern void *tsan1_thread(void *param);
