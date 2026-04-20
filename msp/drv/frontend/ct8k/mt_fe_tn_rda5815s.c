/********************************************************************************************/
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2018 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/

/*
Filename:			RDA5815S_3.6.4a_dvbs.c
Description:		RDA5815S - Digital Satellite Tuner IC driver.

==  Unpdate History  ==
version 1.0		The primary version,  created by Hongxin Wang
						2011-2-22
version 2.0		For B version chip ,  created by Hongxin Wang
						2011-5-27
version 2.1		Add register 0x1a for improving the sensitivity of 1550MHz .
						2011- -
version 2.2		Modify register 0x4a and 0x4b in initial both from 0xbb to 0x88 for improving pll stability .
						2011-06-08
version 2.3		Add register 0x3c in initial for enhance the max level .
						2011-08-11
version 3.1		Modify value of register 0x06 from 0x88 to 0xa8 for enlarge the vco level ;
						Modify value of register 0x4a from 0x88 to 0x48 for decrease presc_1 ;
						Modify value of register 0x4b from 0x88 to 0x58 for decrease presc_3 ;
						Modify value of register 0x15 from 0xaa to 0xae for enlarge buf_ibit_1 ;
						Modify value of register 0x72 from 0x60 to 0x20 and modify value of register 0x73 from 0x76 to 0x72  for change the  pll_sel_vco_freq_th from 1606MHz to 1552MHz ;
						All the change is for pll stability and is necessary.
						2011-10-28
version 3.2		Modify value of register 0x57 from 0x74 to 0x64 for reducing ctsdm_gain2 from 10 to 00 ;
						Modify value of register 0x65 from 0x80 to 0x00 for reducing DC offset ;
						Add register 0x41 in initial  for setting dc_cal_clk to 11 ;
						Both above change is use to enhance the stability of DC .
						2011-12-27
version 3.3		Modify value of register 0x8e from 0x9a to 0xe0 ;
						Modify value of register 0x8f from  0x37 to 0x95 ;
						Modify value of register 0xb0 from 0x37 to 0x95 ;
						Modify value of register 0xb1 from 0x37 to 0x95 ;
						Modify value of register 0xb2 from 0x37 to 0x95 ;
						Both above change is use to enlarge the gain 12dB ;
						Modify the max bandwidth from 45MHz to 40MHz ;
						Add register 0x2b in the rda5815Set  for  improving the pll stability .
						2012-1-12
version 3.4		Modify value of register 0x38 from 0x90 to 0x93 for improving the high temperature performance .
						2012-4-17
version 3.5		Modify value of register 0x4a from 0x48 to 0x68 for enhance pll stability ;
						Modify value of register 0x4b from 0x58 to 0x78 for enhance pll stability .
						2012-7-18
version 3.6		Modify value of register 0x06 from 0xA8 to 0xF8 for enhance pll stability .
						2012-8-10
version 3.6.1		Optimize bandwidth calculation code.
						Add configuration for Xtal = 4MHz, 13.5MHz, 24MHz and 30MHz.
						2013-8-22, by Yilei LIU
version 3.6.2		Optimize bandwidth calculation code.
						2013-11-20, by Yilei LIU
version 3.6.3		Correct a bandwith unit fault. (KSps to MSps)
version3.6.4a     Optimize stages for 250~950MHz.Tempoary version, realsed for test
*/

#if 0
#include "mt_type.h"
#include "mtos_event.h"
#include "mtos_sem.h"
#include "mtos_printk.h"

#include "drv_dev.h"
#include "../../drvbase/drv_dev_priv.h"
#include "../../drvbase/drv_svc.h"
#include "../inc/drv/bus/i2c.h"

#include "nim.h"
#endif

#include "mt_fe_def.h"

#include "mt_fe_i2c_ct8k.h"


#if MT_FE_DMD_DVBS_S2_SUPPORT

//U8	RDA5815S_Address = 0;


static S32 _mt_fe_tn_get_reg_rda5815s(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 *reg_data)
{
	return _mt_fe_tn_get_reg_ss2_ct8k(handle, reg_addr, reg_data);
}

static S32 _mt_fe_tn_set_reg_rda5815s(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 reg_data)
{
	return _mt_fe_tn_set_reg_ss2_ct8k(handle, reg_addr, reg_data);
}




#define Xtal_27M
//define Xtal_4M
//define Xtal_13p5M
//define Xtal_24M
//define Xtal_30M

// i2c addr = 0x18
// for rda5818s config:
//	drv_cfg.nim_cfg.dev_cfg[0].tn_cfg.i2c_cfg.i2c_addr = 0x18;
//  drv_cfg.nim_cfg.dev_cfg[0].pin_cfg.lnb_vol_pin = 6;
//  drv_cfg.nim_cfg.dev_cfg[0].pin_cfg.lnb_enable_pin = 7;


// [0] select 250-950; [1] select 950-2150
static unsigned long freq_sel = 1;

#if 0
static MT_FE_RET RDA5815_tn_get_offset(MT_FE_CT8K_Device_Handle handle, S32 *p_offset)
{
	*p_offset = 0;

	return 0;
}
#endif

MT_FE_RET mt_fe_tn_init_RDA5815S_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	S32 ret = 0;

	//RDA5815S_Address = handle->m_device_ss2.tuner_cfg.tuner_dev_addr;

	_mt_delay_ct8k(1);		//Wait 1ms.
	// Chip register soft reset
	ret = _mt_fe_tn_set_reg_rda5815s(handle, 0x04,0x04);
	if(ret != 0)
	{
		return -1;
	}

	_mt_fe_tn_set_reg_rda5815s(handle, 0x04,0x05);

	// Initial configuration start

	//pll setting
	_mt_fe_tn_set_reg_rda5815s(handle, 0x1a,0x13);		//add by rda 2011.5.28
	_mt_fe_tn_set_reg_rda5815s(handle, 0x41,0x53);		//add by rda 2011.12.27
	_mt_fe_tn_set_reg_rda5815s(handle, 0x38,0x93);		//modify by rda 2012.04.17
	_mt_fe_tn_set_reg_rda5815s(handle, 0x39,0x15);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x3A,0x00);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x3B,0x00);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x3C,0x0c);		//add by rda 2011.8.9
	_mt_fe_tn_set_reg_rda5815s(handle, 0x0c,0xE2);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x2e,0x6F);

#ifdef Xtal_27M
	_mt_fe_tn_set_reg_rda5815s(handle, 0x72,0x07);			//1552~1553
	_mt_fe_tn_set_reg_rda5815s(handle, 0x73,0x20);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x74,0x72);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x75,0x06);			//1363~1364, 1862~1863
	_mt_fe_tn_set_reg_rda5815s(handle, 0x76,0x40);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x77,0x89);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x79,0x04);			//1075
	_mt_fe_tn_set_reg_rda5815s(handle, 0x7A,0xFA);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x7B,0x12);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x7C,0xF7);
#endif
#ifdef Xtal_4M
	_mt_fe_tn_set_reg_rda5815s(handle, 0x72,0x30);			//v3.6.1, 1551~1552
	_mt_fe_tn_set_reg_rda5815s(handle, 0x73,0x73);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x74,0x07);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x75,0x2A);		//v3.6.1, 1363~1364, 1863~1864
	_mt_fe_tn_set_reg_rda5815s(handle, 0x76,0x93);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x77,0xA3);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x79,0x21);			//v3.6.1, 1075
	_mt_fe_tn_set_reg_rda5815s(handle, 0x7A,0x98);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x7B,0x00);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x7C,0x00);
#endif
#ifdef Xtal_13p5M
	_mt_fe_tn_set_reg_rda5815s(handle, 0x72,0x0E);			//v3.6.1, 1552~1553
	_mt_fe_tn_set_reg_rda5815s(handle, 0x73,0x50);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x74,0xE5);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x75,0x0C);		//v3.6.1, 1363~1364, 1862~1863
	_mt_fe_tn_set_reg_rda5815s(handle, 0x76,0x91);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x77,0x13);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x79,0x09);			//v3.6.1, 1075
	_mt_fe_tn_set_reg_rda5815s(handle, 0x7A,0xF4);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x7B,0x25);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x7C,0xED);
#endif
#ifdef Xtal_24M
	_mt_fe_tn_set_reg_rda5815s(handle, 0x72,0x08);			//v3.6.1, 1547~1548
	_mt_fe_tn_set_reg_rda5815s(handle, 0x73,0x00);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x74,0x80);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x75,0x07);			//v3.6.1, 1367~1368, 1859~1860
	_mt_fe_tn_set_reg_rda5815s(handle, 0x76,0x10);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x77,0x9A);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x79,0x05);			//v3.6.1, 1075
	_mt_fe_tn_set_reg_rda5815s(handle, 0x7A,0x99);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x7B,0x55);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x7C,0x55);
#endif
#ifdef Xtal_30M
	_mt_fe_tn_set_reg_rda5815s(handle, 0x72,0x06);			//v3.6.1, 1552~1553
	_mt_fe_tn_set_reg_rda5815s(handle, 0x73,0x60);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x74,0x66);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x75,0x05);		//v3.6.1, 1363~1364, 1862~1863
	_mt_fe_tn_set_reg_rda5815s(handle, 0x76,0xA0);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x77,0x7B);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x79,0x04);			//v3.6.1, 1075
	_mt_fe_tn_set_reg_rda5815s(handle, 0x7A,0x7A);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x7B,0xAA);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x7C,0xAA);
#endif

	_mt_fe_tn_set_reg_rda5815s(handle, 0x5b,0x20);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x2f,0x57);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x0d,0x70);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x16,0x03);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x18,0x4B);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x30,0xFF);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x5c,0xFF);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x6c,0xFF);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x6e,0xFF);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x65,0x00);		//modify by rda 2011.12.27
	_mt_fe_tn_set_reg_rda5815s(handle, 0x70,0x3F);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x71,0x3F);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x53,0xA8);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x46,0x21);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x47,0x84);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x48,0x10);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x49,0x08);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x60,0x80);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x61,0x80);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x6A,0x08);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x6B,0x63);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x69,0xF8);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x57,0x64);		//modify by rda 2011.12.27
	_mt_fe_tn_set_reg_rda5815s(handle, 0x05,0x88);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x06,0xF8);		//modify by rda 2012.8.10
	_mt_fe_tn_set_reg_rda5815s(handle, 0x15,0xAE);		//modify by rda 2011.10.28
	_mt_fe_tn_set_reg_rda5815s(handle, 0x4a,0x68);		//modify by rda 2012.7.18
	_mt_fe_tn_set_reg_rda5815s(handle, 0x4b,0x78);		//modify by rda 2012.7.18

	//agc setting
	_mt_fe_tn_set_reg_rda5815s(handle, 0x4f,0x40);
	_mt_fe_tn_set_reg_rda5815s(handle, 0x5b,0x20);


	// for blocker
	if(freq_sel == 0)//(250<=freq<=950)                //250MHz<=frequency<=950MHz select if
	{
		_mt_fe_tn_set_reg_rda5815s(handle, 0x16,0x02);//stage setting
		_mt_fe_tn_set_reg_rda5815s(handle, 0x18,0x12);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x30,0x22);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x5c,0x32);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x6c,0x22);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x6e,0xB4);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x1b,0x94);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x1d,0xE4);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x1f,0x79);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x21,0x79);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x23,0xEB);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x25,0xFB);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x27,0xFA);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x29,0xFF);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xb3,0xFF);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xb5,0xFF);

		_mt_fe_tn_set_reg_rda5815s(handle, 0x17,0xFC);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x19,0xFC);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x31,0xFC);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x5d,0xFC);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x6d,0xFE);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x6f,0xFC);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x1c,0x3F);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x1e,0x7E);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x20,0x9E);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x22,0xBF);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x24,0xBE);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x26,0xBE);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x28,0xCF);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x2a,0xDE);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xb4,0x0F);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xb6,0x0F);

		_mt_fe_tn_set_reg_rda5815s(handle, 0xb7,0x20);	//start
		_mt_fe_tn_set_reg_rda5815s(handle, 0xb9,0x46);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xbb,0x3E);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xbd,0x3E);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xbf,0x4E);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xc1,0x46);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xc3,0x4E);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xc5,0x4E);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xa3,0x50);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xa5,0x46);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xa7,0x40);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xa9,0x46);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xab,0x57);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xad,0x4E);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xaf,0x46);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xb1,0x85);			//modify by rda 2015.4.10

		_mt_fe_tn_set_reg_rda5815s(handle, 0xb8,0xB7);		//end
		_mt_fe_tn_set_reg_rda5815s(handle, 0xba,0x7A);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xbc,0x7A);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xbe,0x66);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xc0,0x7A);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xc2,0x75);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xc4,0x75);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xc6,0x87);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xa4,0x80);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xa6,0x75);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xa8,0x75);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xaa,0x63);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xac,0x87);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xae,0x75);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xb0,0x7A);		//modify by rda 2015.4.10
		_mt_fe_tn_set_reg_rda5815s(handle, 0xb2,0xA3);		//modify by rda 2015.4.10

		_mt_fe_tn_set_reg_rda5815s(handle, 0x81,0x6C);		//rise
		_mt_fe_tn_set_reg_rda5815s(handle, 0x82,0xB9);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x83,0xA7);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x84,0xD5);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x85,0xD0);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x86,0xBE);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x87,0xD8);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x88,0xD8);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x89,0xB5);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x8a,0xD5);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x8b,0xB5);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x8c,0xDD);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x8d,0xC8);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x8e,0xBE);		//modify by rda 2015.4.10
		_mt_fe_tn_set_reg_rda5815s(handle, 0x8f,0xA7);		//modify by rda 2015.4.10

		_mt_fe_tn_set_reg_rda5815s(handle, 0x90,0x00);		//fall
		_mt_fe_tn_set_reg_rda5815s(handle, 0x91,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x92,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x93,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x94,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x95,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x96,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x97,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x98,0x10);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x99,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x9a,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x9b,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x9c,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x9d,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x9e,0x2C);
	}
	else
	{
		_mt_fe_tn_set_reg_rda5815s(handle, 0x16,0x10);//950MHz<=frequency<=2150MHz select else
		_mt_fe_tn_set_reg_rda5815s(handle, 0x18,0x20);//stage setting
		_mt_fe_tn_set_reg_rda5815s(handle, 0x30,0x30);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x5c,0x30);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x6c,0x30);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x6e,0x70);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x1b,0xB2);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x1d,0xB2);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x1f,0xB2);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x21,0xB2);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x23,0xB6);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x25,0xB6);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x27,0xBA);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x29,0xBF);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xb3,0xFF);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xb5,0xFF);

		_mt_fe_tn_set_reg_rda5815s(handle, 0x17,0xF0);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x19,0xF0);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x31,0xF0);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x5d,0xF1);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x6d,0xF2);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x6f,0xF2);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x1c,0x31);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x1e,0x72);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x20,0x96);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x22,0xBA);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x24,0xBA);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x26,0xBE);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x28,0xCE);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x2a,0xDE);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xb4,0x0F);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xb6,0x0F);

		_mt_fe_tn_set_reg_rda5815s(handle, 0xb7,0x10);	//start
		_mt_fe_tn_set_reg_rda5815s(handle, 0xb9,0x10);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xbb,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xbd,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xbf,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xc1,0x10);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xc3,0x10);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xc5,0x10);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xa3,0x19);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xa5,0x2E);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xa7,0x37);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xa9,0x47);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xab,0x47);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xad,0x3F);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xaf,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xb1,0x95);		//modify by rda 2012.1.12

		_mt_fe_tn_set_reg_rda5815s(handle, 0xb8,0x47);		//end
		_mt_fe_tn_set_reg_rda5815s(handle, 0xba,0x3F);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xbc,0x37);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xbe,0x3F);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xc0,0x3F);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xc2,0x3F);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xc4,0x3F);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xc6,0x3F);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xa4,0x47);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xa6,0x57);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xa8,0x5F);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xaa,0x70);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xac,0x70);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xae,0x68);
		_mt_fe_tn_set_reg_rda5815s(handle, 0xb0,0x95);		//modify by rda 2012.1.12
		_mt_fe_tn_set_reg_rda5815s(handle, 0xb2,0x95);		//modify by rda 2012.1.12

		_mt_fe_tn_set_reg_rda5815s(handle, 0x81,0x77);		//rise
		_mt_fe_tn_set_reg_rda5815s(handle, 0x82,0x68);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x83,0x70);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x84,0x68);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x85,0x68);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x86,0x68);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x87,0x70);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x88,0x47);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x89,0x68);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x8a,0x8E);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x8b,0x8E);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x8c,0x8E);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x8d,0x9C);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x8e,0xe0);		//modify by rda 2012.1.12
		_mt_fe_tn_set_reg_rda5815s(handle, 0x8f,0x95);		//modify by rda 2012.1.12

		_mt_fe_tn_set_reg_rda5815s(handle, 0x90,0x00);		//fall
		_mt_fe_tn_set_reg_rda5815s(handle, 0x91,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x92,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x93,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x94,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x95,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x96,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x97,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x98,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x99,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x9a,0x10);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x9b,0x24);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x9c,0x10);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x9d,0x00);
		_mt_fe_tn_set_reg_rda5815s(handle, 0x9e,0x00);
	}

	_mt_sleep_ct8k(10);		//Wait 10ms;

	// Initial configuration end

	return 0;
}

/*************************************************************************/
//	Function to Set the RDA5815
//	fPLL:		Frequency			unit: MHz from 250 to 2300
//	fSym:	SymbolRate			unit: KSps from 1000 to 45000
/************************************************************************/

MT_FE_RET mt_fe_tn_set_freq_RDA5815S_ct8k(MT_FE_CT8K_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz)//unsigned long fPLL, unsigned long fSym )
{
	unsigned char	buffer;
	unsigned long	temp_value = 0;
	unsigned char	Filter_bw_control_bit;
	unsigned long fPLL = freq_KHz/1000;

	_mt_fe_tn_set_reg_rda5815s(handle, 0x04,0xc1);		//add by rda 2011.8.9,RXON = 0 , change normal working state to idle state
	_mt_fe_tn_set_reg_rda5815s(handle, 0x2b,0x95);		//clk_interface_27m=0  add by rda 2012.1.12
	//set frequency start
#ifdef Xtal_27M
	temp_value = (unsigned long)fPLL* 77672;		//((2^21) / RDA5815_XTALFREQ);
#endif
#ifdef Xtal_4M
	temp_value = (unsigned long)fPLL* 524288;		//((2^21) / RDA5815_XTALFREQ);
#endif
#ifdef Xtal_13p5M
	temp_value = (unsigned long)fPLL* 155345;		//((2^21) / RDA5815_XTALFREQ);
#endif
#ifdef Xtal_24M
	temp_value = (unsigned long)fPLL* 87381;		//((2^21) / RDA5815_XTALFREQ);
#endif
#ifdef Xtal_30M
	temp_value = (unsigned long)fPLL* 69905;		//((2^21) / RDA5815_XTALFREQ);
#endif
	buffer = ((unsigned char)((temp_value>>24)&0xff));
	_mt_fe_tn_set_reg_rda5815s(handle, 0x07,buffer);
	buffer = ((unsigned char)((temp_value>>16)&0xff));
	_mt_fe_tn_set_reg_rda5815s(handle, 0x08,buffer);
	buffer = ((unsigned char)((temp_value>>8)&0xff));
	_mt_fe_tn_set_reg_rda5815s(handle, 0x09,buffer);
	buffer = ((unsigned char)( temp_value&0xff));
	_mt_fe_tn_set_reg_rda5815s(handle, 0x0a,buffer);
	//set frequency end
	// set Filter bandwidth start
	Filter_bw_control_bit = (unsigned char)((sym_rate_KSs*135/200+4000)/1000);

	if(Filter_bw_control_bit<4)
		Filter_bw_control_bit= 4;		// MSps
	else if(Filter_bw_control_bit>40)
		Filter_bw_control_bit = 40;		// MSps

	Filter_bw_control_bit&=0x3f;
	Filter_bw_control_bit |= 0x40;
	_mt_fe_tn_set_reg_rda5815s(handle, 0x0b,Filter_bw_control_bit);
	// set Filter bandwidth end
	_mt_fe_tn_set_reg_rda5815s(handle, 0x04,0xc3);		//add by rda 2011.8.9,RXON = 0 ,rxon=1,normal working
	_mt_fe_tn_set_reg_rda5815s(handle, 0x2b,0x97);		//clk_interface_27m=1  add by rda 2012.1.12


	_mt_delay_ct8k(5);		//Wait 5ms;

	return 0;
}

MT_FE_RET mt_fe_tn_get_gain_RDA5815S_ct8k(MT_FE_CT8K_Device_Handle handle, U32 *p_gain)
{
	U8 reg_0x3f, reg_0x40;
	U32 tmp;

	_mt_fe_tn_get_reg_rda5815s(handle, 0x3f, &reg_0x3f);
	_mt_fe_tn_get_reg_rda5815s(handle, 0x40, &reg_0x40);

	tmp = reg_0x3f * 32 + reg_0x40;

	*p_gain = tmp;

	return MtFeErr_Ok;
}


#define STRENGTH_RATIO 100

MT_FE_RET mt_fe_tn_get_strength_RDA5815S_ct8k(MT_FE_CT8K_Device_Handle handle, S8 *p_strength)
{
//	U8  ii = 0;
	U32 gain_val, strength;
	MT_FE_LOCK_STATE lock_state;

	mt_fe_tn_get_gain_RDA5815S_ct8k(handle, &gain_val);

	if (gain_val <= 338)		strength = 99;										  //about -30dBm    99%
	else if (gain_val > 2080)	strength = 0;
	else 						strength = 99 - (gain_val - 338) * 10 / 156;


	mt_fe_dmd_get_lock_state_ct8k_ss2(handle, &lock_state);
	if ((lock_state == MtFeLockState_Locked) && (strength < 40))
		strength = 20 + strength / 2;


	strength = strength * STRENGTH_RATIO / 100;


	if (strength >= 100) strength = 99;
	//if (strength < 0) strength = 0;

	*p_strength = strength;

	return MtFeErr_Ok;
}



MT_FE_RET	mt_fe_tn_sleep_RDA5815S_ct8k(MT_FE_CT8K_Device_Handle handle)
{
#if 0
	SleepTuner(handle);
	mt_fe_WaitTime(50);
#endif
	return MtFeErr_Ok;
}

MT_FE_RET	mt_fe_tn_wake_up_RDA5815S_ct8k(MT_FE_CT8K_Device_Handle handle)
{
#if 0
	U8 val;

	ts2022_tn_get_reg(handle, 0x00, &val);
	if((val&0x01) == 0x00)
	{
		ts2022_tn_set_reg(handle, 0x00, 0x01);
		mt_fe_WaitTime(50);
	}

	ts2022_tn_set_reg(handle, 0x00, 0x03);
	mt_fe_WaitTime(50);
#endif
	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_get_tuner_freq_offset_RDA5815S_ct8k(MT_FE_CT8K_Device_Handle handle, S32 *p_offset)
{
	*p_offset = 0;
	return MtFeErr_Ok;
}

#endif


