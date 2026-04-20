/***************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/***************************************************************************/
#ifndef _PKA_RSA_H_
#define _PKA_RSA_H_

enum PKA_RSA_KEY_SIZE {
	PKA_RSA_KEY_1024 = 1024 >> 3,
	PKA_RSA_KEY_2048 = 2048 >> 3,
};

mt_s32 pka_rsa_operation(mt_u32 key_size, const mt_u8 *mod, const mt_u8 *exp,
			 mt_u8 *dst, const mt_u8 *src);

#endif /* end of include guard: _PKA_RSA_H_ */
