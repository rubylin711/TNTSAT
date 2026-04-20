/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/****************************************************************************
* MONTAGE PROPRIETARY AND CONFIDENTIAL
* Montage Technology (Shanghai) Inc.
* All Rights Reserved
* --------------------------------------------------------------------------
*
* File:				mt_fe_dmd_rs6060_reg_tbl.h
*
* Current version:	2.02.05
*
* Description:		register table of RS6060
*
* Log:	Description			Version				Date			Author
*		---------------------------------------------------------------------
*		Create				2.00.00				2017.03.12		Allan.Xiu
*		Modify				2.00.08				2017.04.12		Allan.Xiu
*		Modify				2.00.10				2017.04.25		Allan.Xiu
*		Modify				2.00.13				2017.05.09		Allan.Xiu
*		Modify				2.00.14				2017.05.16		Allan.Xiu
*		Modify				2.00.16				2017.05.25		Allan.Xiu
*		Modify				2.01.00				2017.07.05		Allan.Xiu
*		Modify				2.01.04				2017.08.15		Allan.Xiu
*		Modify				2.01.10				2017.11.27		Daniel.Hao
*		Modify				2.01.11				2017.12.11		Daniel.Hao
*		Modify				2.01.17				2018.10.11		Daniel.Hao
*		Modify				2.02.00				2019.01.24		Daniel.Hao
*		Modify				2.02.01				2019.04.11		Daniel.Hao
*		Modify				2.02.05				2019.09.06		Daniel.Hao
****************************************************************************/


const U8 rs6060_reg_tbl_def[][2] =
{
	{0x04, 0x00},

	{0x8a, 0x01},	//modify by Henry 20171211
	{0x16, 0xa7},

	{0x30, 0x88},
	{0x4a, 0x80}, //0x81 for fpga, and 0x80 for asic
	{0x4d, 0x91},
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
	{0xbd, 0x82},//0x83
	{0x80, 0xa8},
	{0x81, 0xea},

	{0xbe, 0xa1}
};


/*register setting for blind scan*/
const U8 rs6060_reg_tbl_bs_def[][2] =
{
	{0x04, 0x00},

	{0x8a, 0x01},	//modify by Henry 20171211
	{0x16, 0xa7},

	{0x30, 0x88},
	{0x4a, 0x80},
	{0x4d, 0x91},
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
	{0xbd, 0x82},//0x83
	{0x80, 0xa8},
	{0x81, 0xea},
	{0xbe, 0xa1}
};


