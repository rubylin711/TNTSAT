/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HDVENC_TESTCASE_H__
#define __HDVENC_TESTCASE_H__

/*=========================================================================
| TYPEDEFS
 ========================================================================*/
#define HD_RESET_REG_CNT (2)

/***** local functions *******************************************************/
mt_u32 do_case_hd_reset(void);
mt_u32 do_case_hd_dac_sin(void);
mt_u32 do_case_share_irq(mt_u32 irq);

/***** end of file ***********************************************************/

#endif /* __HDVENC_TESTCASE_H__ */
