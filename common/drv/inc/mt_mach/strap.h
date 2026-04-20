/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)

#include <asm/mach-symphony/strap.h>

#elif defined(CONFIG_MT_CHIP_SYMPHONY4)

#include <mach/strap.h>

#else

#error "Please config select one correct chipset, or not include mt_mach/strap.h"

#endif

