/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HDMI20_TESTCASE_H__
#define __HDMI20_TESTCASE_H__

//#include "hdmi20.h"

#define SSHH_PRINTF printf
/*=========================================================================
| TYPEDEFS
 ========================================================================*/
typedef struct {
	mt_u8 type;
	mt_u8 version;
	mt_u8 length;
	mt_u8 pb_byte[28];
} info_struct_t;

#if 0
typedef struct {
	mt_u8 scanInfo;
	mt_u8 barInfo;
	mt_u8 activeFormatInfo;
	mt_u8 clrSpc;
	mt_u8 activeAR;
	mt_u8 pictureAR;
	mt_u8 colorimetry;
	mt_u8 scalingInfo;
	mt_u8 rgbQR;
	mt_u8 extColorimetry;
	mt_u8 itContent;
	mt_u8 vic;
	mt_u8 repetition;
	mt_u8 itContentType;
	mt_u8 yccQR;
} MtAviInfo_t;
#endif

typedef struct TVSYSSWITCH_S {
	SiiInst_t        instTx;
	mt_u32 tvsys;
	mt_u32 first_set;
	mt_u32 cs;
	#if (!__HDMI_UBOOT__)
	pthread_t task_id;
	#endif
	mt_u32 mode_3d;
	mt_u32 mode_3d_ext;
	mt_u32 bdepth;
	mt_u32 colorimetry;
} TVSYSSWITCH_T;

#define HDMI_CORE_RESET_BIT      (1 << 8)
#define HDMI_AHB_RESET_BIT       (1 << 7)
#define REG_SYS_BLOCK_RESET      (0xbf50a60c)

/***** local functions *******************************************************/
void test_set_tvsys_cmd(TVSYSSWITCH_T *ptvsyscmd);
mt_void set_ae_test_en(mt_void);
mt_void set_tvsys_cmd(TVSYSSWITCH_T tvsys);
mt_void hdmi_save_avinfoframe(SiiInst_t inst);
mt_u32 hdmi_get_test_tvsys(mt_void);
mt_void test_set_ar_cmd(SiiInst_t        instTx);

/***** end of file ***********************************************************/

#endif /* __HDMI20_TESTCASE_H__ */
