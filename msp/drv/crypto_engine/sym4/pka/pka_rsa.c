/***************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/***************************************************************************/
#include "mt_type.h"
#include "pka_rsa.h"
#include "pka_regs.h"
#include "pka_mod.h"

mt_s32 pka_rsa_operation(mt_u32 key_size, const mt_u8 *mod, const mt_u8 *exp,
			 mt_u8 *dst, const mt_u8 *src)
{
	return pka_mod_operation(PKA_MOD_CMD_MOD_POW, key_size, mod, src, exp,
				 dst);
}
