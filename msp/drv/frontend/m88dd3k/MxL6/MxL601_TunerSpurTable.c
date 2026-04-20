/********************************************************************************************/
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2016 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/
/*******************************************************************************
 *
 * FILE NAME          : MxL601_TunerSpurTable.cpp
 * 
 * AUTHOR             : Dong Liu                 
 *
 * DATE CREATED       : 11/16/2011
 *
 * DESCRIPTION        : This file contains spur shift table that used in channel 
 *                      tune procedure 
 *                             
 *******************************************************************************
 *                Copyright (c) 2011, MaxLinear, Inc.
 ******************************************************************************/

#include "MxL601_TunerSpurTable.h"
#include "MxL601_TunerCfg.h"

CHAN_DEPENDENT_SPUR_REGISTER_T MxL601_SPUR_REGISTER_dd3k = 
{ 4, {DFE_RFLUT_SWP1_REG, DFE_RFLUT_DIV_MOD_REG, DFE_REFLUT_BYP_REG, DFE_REFSX_INT_MOD_REG} };

// Lookup Table of frequencies in NTSC mode for 16MHz Xtal
// on which optimized channel tunning settungs will be performed
// Programming guide document table 24
CHAN_DEPENDENT_FREQ_TABLE_T MxL601_NTSC_FREQ_LUT_XTAL_16MHZ_LIF_dd3k[] = 
{ 
  { 1, 0x1C, 0x17, 0x00, 0xD8},
  {111000000, 0x1C, 0x17, 0x20, 0xD7},
  {177000000, 0x19, 0x39, 0x20, 0xD6},
  {219000000, 0x1C, 0x17, 0x20, 0xD7},
  {285000000, 0x1F, 0x3F, 0x20, 0xD7},
  {363000000, 0x1B, 0x3B, 0x20, 0xD8},
  {651000000, 0x1C, 0x17, 0x20, 0xD4},
  {663000000, 0x1C, 0x17, 0x20, 0xD7},
  {813000000, 0x1C, 0x17, 0x20, 0xD5},
  {867000000, 0x1C, 0x17, 0x20, 0xD7},
  {0, 0, 0, 0, 0},
};

// Lookup Table of frequencies in NTSC mode For 24MHz Xtal 
// on which optimized channel tunning settungs wll be performed
// Programming guide document table 25
CHAN_DEPENDENT_FREQ_TABLE_T MxL601_NTSC_FREQ_LUT_XTAL_24MHZ_dd3k[] = 
{ 
  { 1, 0x1D, 0x17, 0x00, 0x90},
  {123000000, 0x19, 0x39, 0x20, 0x8D},
  {177000000, 0x1F, 0x3F, 0x20, 0x90},
  {231000000, 0x1D, 0x3D, 0x20, 0x90},
  {255000000, 0x19, 0x39, 0x20, 0x90},
  {699000000, 0x1A, 0x3A, 0x20, 0x90},
  {705000000, 0x19, 0x39, 0x20, 0x8F},
  {0, 0, 0, 0, 0},
};

// Lookup Table of frequencies in NTSC mode for HRC based frequencies 
// on which optimized channel tunning settungs wll be performed
// Programming guide document table 26
CHAN_DEPENDENT_FREQ_TABLE_T MxL601_NTSC_FREQ_LUT_HRC_16MHZ_dd3k[] = 
{ 
  { 1, 0x1D, 0x17, 0x00, 0xD8},
  {55750000, 0x19, 0x39, 0x20, 0xD8},
  {283765625, 0x1E, 0x3E, 0x20, 0xD8},
  {649781250, 0x1A, 0x3A, 0x20, 0xD8},
  {883794100, 0x1C, 0x17, 0x20, 0xD7},
  {0, 0, 0, 0, 0},
};

// Lookup Table of frequencies in NTSC mode for HRC based frequencies 
// on which optimized channel tunning settungs wll be performed
// Programming guide document table 27
CHAN_DEPENDENT_FREQ_TABLE_T MxL601_NTSC_FREQ_LUT_HRC_24MHZ_dd3k[] = 
{ 
  { 1, 0x1D, 0x17, 0x00, 0x90},
  {271765625, 0x1E, 0x3E, 0x20, 0x8F},
  {283765625, 0x1E, 0x3E, 0x20, 0x90},
  {0, 0, 0, 0, 0},
};

// Programming guide document table 28
CHAN_DEPENDENT_FREQ_TABLE_T MxL601_NTSC_FREQ_LUT_IRC_16MHZ_dd3k[] = 
{ 
  { 1, 0x1C, 0x17, 0x00, 0xD8},
  {111012500, 0x1C, 0x17, 0x20, 0xD7},
  {177012500, 0x1F, 0x3F, 0x20, 0xD8},
  {219012500, 0x1C, 0x17, 0x20, 0xD7},
  {285012500, 0x1F, 0x3F, 0x20, 0xD8},
  {363012500, 0x1B, 0x3B, 0x20, 0xD8},
  {663012500, 0x1C, 0x17, 0x20, 0xD7},
  {759012500, 0x1C, 0x17, 0x20, 0xD6},
  {813012500, 0x1C, 0x17, 0x20, 0xD5},
  {867012500, 0x1C, 0x17, 0x20, 0xD7},
  {0, 0, 0, 0, 0},
};

// Programming guide document table 29
CHAN_DEPENDENT_FREQ_TABLE_T MxL601_NTSC_FREQ_LUT_IRC_24MHZ_dd3k[] = 
{ 
  { 1, 0x1D, 0x17, 0x00, 0x90},
  {123015625, 0x19, 0x39, 0x20, 0x8D},
  {231015625, 0x15, 0x35, 0x20, 0x8E},
  {0, 0, 0, 0, 0},
};

// Programming guide document table 31
// Lookup Table of frequencies in PAL - BG 7 MHz for 16MHz Xtal 
// on which optimized channel tunning settungs wll be performed
CHAN_DEPENDENT_FREQ_TABLE_T MxL601_PAL_BG_7MHZ_LUT_XTAL_16MHZ_LIF_dd3k[] = 
{ 
  { 1, 0x1D, 0x17, 0x00, 0xD8},
  {57500000, 0x1C, 0x17, 0x20, 0xD7},
  {219500000, 0x1C, 0x17, 0x20, 0xD7},
  {0, 0, 0, 0, 0},
};

// Programming guide document table 32
// Lookup Table of frequencies in PAL - BG 7 MHz for 24MHz Xtal 
// on which optimized channel tunning settungs wll be performed
CHAN_DEPENDENT_FREQ_TABLE_T MxL601_PAL_BG_7MHZ_LUT_XTAL_24MHZ_dd3k[] = 
{ 
  { 1, 0x1D, 0x17, 0x00, 0x90},
  {128500000, 0x1F, 0x3F, 0x20, 0x91},
  {149500000, 0x1B, 0x3B, 0x20, 0x8F},
  {0, 0, 0, 0, 0},
};

// Programming guide document table 33
// Lookup Table of frequencies in PAL - BG 8 MHz for 16MHz Xtal 
// on which optimized channel tunning settungs wll be performed
CHAN_DEPENDENT_FREQ_TABLE_T MxL601_PAL_BG_8MHZ_LUT_XTAL_16MHZ_LIF_dd3k[] = 
{ 
  { 1, 0x1D, 0x17, 0x00, 0xD8},
  {378000000, 0x1A, 0x3A, 0x20, 0xD5},
  {418000000, 0x1E, 0x3E, 0x20, 0xD4},
  {594000000, 0x19, 0x39, 0x20, 0xDA},
  {786000000, 0x1C, 0x17, 0x20, 0xD5},
  {834000000, 0x1A, 0x3A, 0x20, 0xD5},
  {0, 0, 0, 0, 0},
};

// Programming guide document table 34
// Lookup Table of frequencies in PAL - BG 8 MHz for 24MHz Xtal 
// on which optimized channel tunning settungs wll be performed
CHAN_DEPENDENT_FREQ_TABLE_T MxL601_PAL_BG_8MHZ_LUT_XTAL_24MHZ_dd3k[] = 
{ 
  { 1, 0x1D, 0x17, 0x00, 0x90},
  {314000000, 0x1F, 0x3F, 0x20, 0x93},
  {378000000, 0x1E, 0x3E, 0x20, 0x91},
  {386000000, 0x18, 0x38, 0x20, 0x8C},
  {418000000, 0x1B, 0x3B, 0x20, 0x91},
  {450000000, 0x1C, 0x3C, 0x20, 0x8E},
  {602000000, 0x19, 0x39, 0x20, 0x8F},
  {0, 0, 0, 0, 0},
};


// Programming guide document table 35
// Lookup Table of frequencies in PAL - D mode for 16MHz Xtal 
// on which optimized channel tunning settungs wll be performed
CHAN_DEPENDENT_FREQ_TABLE_T MxL601_PAL_D_LUT_XTAL_16MHZ_LIF_dd3k[] = 
{ 
  { 1, 0x1D, 0x17, 0x00, 0xD8},
  {227000000, 0x1A, 0x3A, 0x20, 0xDA},
  {243000000, 0x1E, 0x3E, 0x20, 0xDA},
  {898000000, 0x1D, 0x3D, 0x20, 0xD5},
  {0, 0, 0, 0, 0},
};

// Programming guide document table 36
// Lookup Table of frequencies in PAL - D mode for 24MHz Xtal 
// on which optimized channel tunning settungs wll be performed
CHAN_DEPENDENT_FREQ_TABLE_T MxL601_PAL_D_LUT_XTAL_24MHZ_dd3k[] = 
{ 
  { 1, 0x1D, 0x17, 0x00, 0x90},
  {147000000, 0x1A, 0x3A, 0x20, 0x8E},
  {259000000, 0x1B, 0x3B, 0x20, 0x8D},
  {275000000, 0x1A, 0x3A, 0x20, 0x8E},
  {602000000, 0x1C, 0x3C, 0x20, 0x8E},
  {834000000, 0x1B, 0x3B, 0x20, 0x8E},
  {898000000, 0x1D, 0x3D, 0x20, 0x8E},
  {0, 0, 0, 0, 0},
};

// Programming guide document table 37
/* This table frequency point and corresponding data need update ! */
// Lookup Table of frequencies in PAL - I mode for 16MHz Xtal 
// on which optimized channel tunning settungs wll be performed
CHAN_DEPENDENT_FREQ_TABLE_T MxL601_PAL_I_LUT_XTAL_16MHZ_LIF_dd3k[] = 
{ 
  { 1, 0x1D, 0x17, 0x00, 0xD8},
  {80500000, 0x1A, 0x3A, 0x20, 0xE3},
  {186000000, 0x19, 0x39, 0x20, 0xD9},
  {418000000, 0x1E, 0x3E, 0x20, 0xD5},
  {594000000, 0x19, 0x39, 0x20, 0xDA},
  {898000000, 0x1D, 0x3D, 0x20, 0xD5},
  {0, 0, 0, 0, 0},
};

// Programming guide document table 38
/* This table frequency point and corresponding data need update ! */
// Lookup Table of frequencies in PAL - I mode for 24MHz Xtal 
// on which optimized channel tunning settungs wll be performed
CHAN_DEPENDENT_FREQ_TABLE_T MxL601_PAL_I_LUT_XTAL_24MHZ_dd3k[] = 
{ 
  { 1, 0x1D, 0x17, 0x00, 0x90},
  {152500000, 0x19, 0x39, 0x20, 0x90},
  {418000000, 0x1B, 0x3B, 0x20, 0x91},
  {458000000, 0x1A, 0x3A, 0x20, 0x90},
  {898000000, 0x1D, 0x3D, 0x20, 0x8E},
  {0, 0, 0, 0, 0},
};

// Programming guide document table 39
/* This table frequency point and corresponding data need update ! */
// Lookup Table of frequencies in SECAM_L mode for 16MHz Xtal 
// on which optimized channel tunning settungs wll be performed
CHAN_DEPENDENT_FREQ_TABLE_T MxL601_SECAM_L_LUT_XTAL_16MHZ_dd3k[] = 
{ 
  { 1, 0x1D, 0x17, 0x00, 0xD8},
  {242750000, 0x1E, 0x3E, 0x20, 0xD9},
  {346750000, 0x1B, 0x3B, 0x20, 0xD9},
  {0, 0, 0, 0, 0},
};

// Programming guide document table 40
/* This table frequency point and corresponding data need update ! */
// Lookup Table of frequencies in SECAM_L mode for 24MHz Xtal 
// on which optimized channel tunning settungs wll be performed
CHAN_DEPENDENT_FREQ_TABLE_T MxL601_SECAM_L_LUT_XTAL_24MHZ_dd3k[] = 
{ 
  { 1, 0x1D, 0x17, 0x00, 0x90},
  {226750000, 0x19, 0x39, 0x20, 0x8F},
  {346750000, 0x1D, 0x3D, 0x20, 0x8F},
  {0, 0, 0, 0, 0},
};

// Programming guide document table 50
// Lookup Table of frequencies in PAL - D mode for 16MHz Xtal & HIF
// on which optimized channel tunning settungs wll be performed
CHAN_DEPENDENT_FREQ_TABLE_T MxL601_PAL_D_LUT_XTAL_16MHZ_HIF_dd3k[] = 
{ 
  { 1, 0x1D, 0x17, 0x00, 0xD8}, 
  {171000000, 0x1D, 0x17, 0x20, 0xD9},
  {179000000, 0x1D, 0x17, 0x20, 0xD3},
  {187000000, 0x1D, 0x17, 0x20, 0xDD},
  {227000000, 0x1D, 0x17, 0x20, 0xDB},
  {315000000, 0x1D, 0x17, 0x20, 0xD7},
  {339000000, 0x1D, 0x17, 0x20, 0xD5},
  {363000000, 0x1D, 0x17, 0x20, 0xDB},
  {387000000, 0x1D, 0x17, 0x20, 0xD6},
  {443000000, 0x1D, 0x17, 0x20, 0xDA},
  {522000000, 0x1D, 0x17, 0x20, 0xD7},
  {666000000, 0x1D, 0x17, 0x20, 0xD7},
  {738000000, 0x1D, 0x17, 0x20, 0xD7},
  {834000000, 0x1D, 0x17, 0x20, 0xDA},
  {0, 0, 0, 0, 0},
};

// Programming guide document table 51
// Lookup Table of frequencies in PAL - I mode for 16MHz XTAL & HIF
// on which optimized channel tunning settungs wll be performed
CHAN_DEPENDENT_FREQ_TABLE_T MxL601_PAL_I_LUT_XTAL_16MHZ_HIF_dd3k[] = 
{ 
  { 1, 0x1D, 0x17, 0x00, 0xD8}, 
  {48500000, 0x1D, 0x17, 0x20, 0xD7},
  {55000000, 0x1D, 0x17, 0x20, 0xC9},
  {64500000, 0x1D, 0x17, 0x20, 0xDE},
  {96500000, 0x1D, 0x17, 0x20, 0xD1},
  {120500000, 0x1D, 0x17, 0x20, 0xE0},
  {144500000, 0x1D, 0x17, 0x20, 0xDA},
  {178000000, 0x1D, 0x17, 0x20, 0xD6},
  {186000000, 0x1D, 0x17, 0x20, 0xDC},
  {226000000, 0x1D, 0x17, 0x20, 0xDB},
  {234000000, 0x1D, 0x17, 0x20, 0xD7},
  {306000000, 0x1D, 0x17, 0x20, 0xD7},
  {394000000, 0x1D, 0x17, 0x20, 0xD9},
  {410000000, 0x1D, 0x17, 0x20, 0xD7},
  {450000000, 0x1D, 0x17, 0x20, 0xD6},
  {522000000, 0x1D, 0x17, 0x20, 0xD7},
  {666000000, 0x1D, 0x17, 0x20, 0xD7},
  {738000000, 0x1D, 0x17, 0x20, 0xD7},
  {874000000, 0x1D, 0x17, 0x20, 0xD7},
  {882000000, 0x1D, 0x17, 0x20, 0xD7},
  {0, 0, 0, 0, 0},
};

// Programming guide document table 52
// Lookup Table of frequencies in PAL - B/G 7 MHz BW mode for 16MHz XTAL & HIF
// on which optimized channel tunning settungs wll be performed
CHAN_DEPENDENT_FREQ_TABLE_T MxL601_PAL_BG_7MHZ_LUT_XTAL_16MHZ_HIF_dd3k[] = 
{ 
  { 1, 0x1D, 0x17, 0x00, 0xD8}, 
  {57500000, 0x1D, 0x17, 0x20, 0xD7},
  {191500000, 0x1D, 0x17, 0x20, 0xD5},
  {219500000, 0x1D, 0x17, 0x20, 0xD7},
  {240500000, 0x1D, 0x17, 0x20, 0xDB},
  {282500000, 0x1D, 0x17, 0x20, 0xDC},
  {296500000, 0x1D, 0x17, 0x20, 0xDC},
  {0, 0, 0, 0, 0},
};

// Programming guide document table 53
// Lookup Table of frequencies in PAL - B/G 8 MHz BW mode for 16MHz XTAL & HIF
// on which optimized channel tunning settungs wll be performed
CHAN_DEPENDENT_FREQ_TABLE_T MxL601_PAL_BG_8MHZ_LUT_XTAL_16MHZ_HIF_dd3k[] = 
{ 
  { 1, 0x1D, 0x17, 0x00, 0xD8}, 
  {306000000, 0x1D, 0x17, 0x20, 0xD9},
  {314000000, 0x1D, 0x17, 0x20, 0xD6},
  {450000000, 0x1D, 0x17, 0x20, 0xD7},
  {522000000, 0x1D, 0x17, 0x20, 0xD7},
  {666000000, 0x1D, 0x17, 0x20, 0xD7},
  {730000000, 0x1D, 0x17, 0x20, 0xD3},
  {738000000, 0x1D, 0x17, 0x20, 0xD7},
  {0, 0, 0, 0, 0},
};

// Programming guide document table 54
// Lookup Table of frequencies in NTSC mode for 16MHz XTAL & HIF
// on which optimized channel tunning settungs wll be performed
CHAN_DEPENDENT_FREQ_TABLE_T MxL601_NTSC_FREQ_LUT_XTAL_16MHZ_HIF_dd3k[] = 
{ 
  { 1, 0x1C, 0x17, 0x00, 0xD8}, 
  {99000000, 0x1C, 0x17, 0x20, 0xD7},
  {111000000, 0x1C, 0x17, 0x20, 0xD6},
  {219000000, 0x1C, 0x17, 0x20, 0xD7},
  {261000000, 0x1C, 0x17, 0x20, 0xD9},
  {333000000, 0x1C, 0x17, 0x20, 0xD9},
  {363000000, 0x1C, 0x17, 0x20, 0xDB},
  {387000000, 0x1C, 0x17, 0x20, 0xD9},
  {441000000, 0x1C, 0x17, 0x20, 0xD9},
  {447000000, 0x1C, 0x17, 0x20, 0xD6},
  {495000000, 0x1C, 0x17, 0x20, 0xD7},
  {549000000, 0x1C, 0x17, 0x20, 0xD7},
  {603000000, 0x1C, 0x17, 0x20, 0xD7},
  {663000000, 0x1C, 0x17, 0x20, 0xD6},
  {705000000, 0x1C, 0x17, 0x20, 0xD7},
  {735000000, 0x1C, 0x17, 0x20, 0xD7},
  {867000000, 0x1C, 0x17, 0x20, 0xD7},
  {0, 0, 0, 0, 0},
};