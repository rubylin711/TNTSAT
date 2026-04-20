/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __CC_ARIB_H__
#define __CC_ARIB_H__
#include <stdlib.h>
#include <stdio.h>

#define DEBUG_LEVEL   0


/****************************************************************************
 * Local structures
 ****************************************************************************/


/**
 * \file
 * This file defines functions, structures for handling streams of bits in vlc
 */


void ccParsePesData(sCcContext *ccContext, mt_u8 *pes_data, int len);


#endif
