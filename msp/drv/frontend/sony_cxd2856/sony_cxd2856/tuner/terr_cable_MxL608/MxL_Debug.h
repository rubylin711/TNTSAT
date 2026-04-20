/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage-LZ Group                                                          */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2021 Montage-LZ Group Limited. All Rights Reserved.         */
/*****************************************************************************/

/*******************************************************************************
 *
 * FILE NAME          : MxL_Debug.h
 * 
 * AUTHOR             : Brenndon Lee
 *
 * DATE CREATED       : 10/25/2010
 *
 * DESCRIPTION        : Debug header files 
 *
 *******************************************************************************
 *                Copyright (c) 2010, MaxLinear, Inc.
 ******************************************************************************/

#ifndef __MXL_DEBUG_H__
#define __MXL_DEBUG_H__

/******************************************************************************
    Include Header Files
    (No absolute paths - paths handled by make file)
******************************************************************************/

//#include <stdio.h>
#include <linux/printk.h>
#include "MaxLinearDataTypes.h"

/******************************************************************************
    Macros
******************************************************************************/

//#define MxL_DLL_DEBUG0 printk
#define MxL_DLL_DEBUG0(fmt, args...)   do{ } while (0) 

#endif /* __MXL_DEBUG_H__*/
