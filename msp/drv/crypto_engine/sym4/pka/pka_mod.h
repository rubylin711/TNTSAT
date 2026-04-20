/***************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/***************************************************************************/
#ifndef _PKA_MOD_H_
#define _PKA_MOD_H_

enum PKA_MOD_CMD {
	PKA_MOD_CMD_MOD_POW = 0,
	PKA_MOD_CMD_MOD_MUL,
	PKA_MOD_CMD_MOD_ADD,
	PKA_MOD_CMD_MOD_SUB,
	PKA_MOD_CMD_MOD_INV,
	PKA_MOD_CMD_MOD
};

mt_s32 pka_mod_operation(mt_u32 cmd, mt_u32 size, const mt_u8 *mod,
			 const mt_u8 *op1, const mt_u8 *op2, mt_u8 *out);

mt_s32 pka_mod_is_even(void);
void pka_clock_gating_enable(void);
void pka_clock_gating_disable(void);

#endif /* end of include guard: _PKA_MOD_H_ */
