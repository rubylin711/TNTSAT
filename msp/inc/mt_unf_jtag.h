/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_UNF_JTAG_H__
#define __MT_UNF_JTAG_H__
#include "mt_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set jtag password to unlock jtag.
 *
 * @param p_password: pointer of password buffer.
 * @param len: byte length of password. 
 *
 * @return: MT_SUCCESS for success or MT_FAILURE.
 *
 * NOTE: For jtag-protected mode only!
 */
mt_s32 MT_UNF_JTAG_set_password(mt_u8 *p_password, mt_u32 len);

/**
 * @brief Set jtag master password to unlock jtag.
 *
 * @param p_password: pointer of password buffer.
 * @param len: byte length of password.
 *
 * @return: MT_SUCCESS for success or MT_FAILURE.
 *
 * NOTE: For jtag-protected mode only!
 */
mt_s32 MT_UNF_JTAG_set_master_password(mt_u8 *p_password, mt_u32 len);

/**
 * @brief Toggle jtag mode before connecting
 *
 * NOTE: If MT_UNF_JTAG_set_password is called, then no need to call this!
 */
void MT_UNF_JTAG_toggle(void);

#ifdef __cplusplus
}
#endif

#endif //__MT_UNF_JTAG_H__
