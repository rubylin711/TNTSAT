/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HDMI20_TESTCASE_H__
#define __HDMI20_TESTCASE_H__

#include "hdmi20.h"
/*=========================================================================
| TYPEDEFS
 ========================================================================*/
typedef struct {
	mt_u8 type;
	mt_u8 version;
	mt_u8 length;
	mt_u8 pb_byte[28];
} info_struct_t;

#define HDMI_CORE_RESET_BIT      (1 << 8)
#define HDMI_AHB_RESET_BIT       (1 << 7)

/***** local functions *******************************************************/
void hdmi_block_reset_cmd(void);
mt_u32 do_case_hdcp_load_key(void);
mt_u32 do_case_reg_rw(void);

/***** end of file ***********************************************************/

#endif /* __HDMI20_TESTCASE_H__ */
