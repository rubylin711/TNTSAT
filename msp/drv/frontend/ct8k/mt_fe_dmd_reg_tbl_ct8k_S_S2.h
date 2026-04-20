/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/***************************************************************************/
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                    */
/* Montage Technology (Shanghai) Co., Ltd                                  */
/* Copyright (C) 2019                                                      */
/* Montage Technology Group Limited and/or its affiliated companies        */
/* All Rights Reserved                                                     */
/***************************************************************************/
/*
* File:				mt_fe_dmd_ct8k_reg_tbl_S_S2.h
*
* Current version:	0.00.09
*
* Description:		register table of Symphony2
*
* Log:	Description			Version		Date			Author
*		---------------------------------------------------------------------
*		Create				0.00.01		2018.06.26		YZ.Huang
*		Modify				0.00.01		2018.06.26		YZ.Huang
*		Modify				0.00.02		2018.10.17		YZ.Huang
*		Modify				0.00.04		2018.12.28		YZ.Huang
*		Modify				0.00.05		2019.04.24		Daniel.Hao
*		Modify				0.00.09		2019.09.06		Daniel.Hao
****************************************************************************/

#ifndef _CT8K_SS2_REG_TBL_DEF
#define _CT8K_SS2_REG_TBL_DEF


const U8 reg_tbl_def_ct8k_ss2[][2] =
{
	{0x04, 0x10},

	{0x8a, 0x01},	//modify by Henry 20171211
	{0x16, 0xa7},

	{0x30, 0x08},
	{0x32, 0x32},
	{0x33, 0x35},
	{0x35, 0xff},
	{0x4a, 0x80},
	{0x4d, 0x93},
	{0xae, 0x09},
	{0x22, 0x01},
	{0x23, 0x00},
	{0x24, 0x00},
	{0x27, 0x07},
	{0x9c, 0x31},
	{0x9d, 0xc1},
	{0xcb, 0xf4},
	{0xca, 0x00},
	{0x7f, 0x04},
	{0x78, 0x0c},
	{0x85, 0x08},
	//for s2 mode ts out en
	{0x08, 0x47},
	{0xf0, 0x03},

	{0xfa, 0x01},
	{0xf2, 0x00},
	{0xfa, 0x00},
	{0xe6, 0x00},
	{0xe7, 0xf3},

	//for s mode vtb code rate all en
	{0x08, 0x43},
	{0xe0, 0xf8},
	{0x00, 0x00},
	{0xbd, 0x83},
	{0xbe, 0xa1}
};


/*register setting for blind scan*/
const U8 reg_tbl_def_blindscan_ct8k_ss2[][2] =
{
	{0x04, 0x10},

	{0x8a, 0x01},	//modify by Henry 20171211
	{0x16, 0xa7},

	{0x30, 0x08},
	{0x32, 0x32},
	{0x33, 0x35},
	{0x35, 0xff},
	{0x4a, 0x80},
	{0x4d, 0x93},
	{0x63, 0x60},
	{0x64, 0x30},
	{0x65, 0x40},
	{0x68, 0x26},
	{0x69, 0x4c},
	{0xae, 0x09},
	{0x22, 0x01},
	{0x23, 0x00},
	{0x24, 0x00},
	{0x27, 0x07},
	{0x9c, 0x31},
	{0x9d, 0xc1},
	{0xc3, 0x10},
	{0xc4, 0x08},
	{0xc5, 0xf0},
	{0xc6, 0x40},
	{0xcb, 0xf4},
	{0xca, 0x00},
	{0x85, 0x08},
	//for s2 mode ts out en
	{0x08, 0x47},
	{0xf0, 0x03},

	{0xfa, 0x01},
	{0xf2, 0x00},
	{0xfa, 0x00},
	{0xe6, 0x00},
	{0xe7, 0x03},	// 0xf3 @ 190906

	//for s mode vtb code rate all en
	{0x08, 0x43},
	{0xe0, 0xf8},
	{0x00, 0x00},
	{0xbd, 0x83},
	{0xbe, 0xa1}
};


#endif
