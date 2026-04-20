/***************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/***************************************************************************/
#ifndef _PKA_ECC_H_
#define _PKA_ECC_H_

mt_s32 pka_ecc_point_add(mt_u32 size, const mt_u8 *p, const mt_u8 *a,
			 const mt_u8 *x1, const mt_u8 *y1, const mt_u8 *x2,
			 const mt_u8 *y2, mt_u8 *r_x, mt_u8 *r_y);

mt_s32 pka_ecc_point_mul(mt_u32 size, const mt_u8 *p, const mt_u8 *a,
			 const mt_u8 *x, const mt_u8 *y, const mt_u8 *k,
			 mt_u8 *r_x, mt_u8 *r_y);

#endif /* end of include guard: _PKA_ECC_H_ */
