/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HW_KT_IF_H__
#define __HW_KT_IF_H__

#include "kt_result.h"

#ifdef CONFIG_MT_KEYTABLE_SYM6
#include "sym6/hw_kt_if.h"
#elif defined CONFIG_MT_KEYTABLE_SYM4
#include "sym4/hw_kt_if.h"
#else
#include "sym2/hw_kt_if.h"
#endif

#endif	/*__HW_KT_IF_H__*/

