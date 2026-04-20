/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2022, Montage-LZ Technology Co., Ltd.
 *
 * File Name      : pinmux_stub.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2022/07/04
 * Description    : Pinmux stub file.
 * History        :
 * 1.Date         : 2022/07/04
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include "mt_drv_pinctrl.h"
#include "pinctrl.h"

struct pinmux_group g_pin_group[] =
{
};

unsigned int g_pin_group_count = sizeof(g_pin_group) / sizeof(g_pin_group[0]);

