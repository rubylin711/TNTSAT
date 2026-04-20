/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MSS_PKA_H__
#define __MSS_PKA_H__

mt_s32 mss_pka_test(int argc, char *const argv[]);

int mss_pka_rsa_test(int argc, char *const argv[]);

int mss_pka_sm2_test(int argc, char *const argv[]);

int mss_pka_ecdh_test(int argc, char *const argv[]);

int mss_pka_ecdsa_test(int argc, char *const argv[]);

#endif
