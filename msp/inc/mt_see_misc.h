/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_SEE_MISC_H__
#define __MT_SEE_MISC_H__

#include "../../drv/ipcs/ipcs_symphony.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/**
 * @brief 
 */
void see_mcpu_mailbox_init(void);

/**
 * @brief:Take lock for crypto engine 
 *
 * @return 
 */
int see_mb_take_cryptolock(void);

/**
 * @brief:Release lock for crypto engine
 *
 * @return 
 */
int see_mb_give_cryptolock(void);


/** @} */  /** <!-- ==== API declaration end ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_SEE_MISC_H__ */
