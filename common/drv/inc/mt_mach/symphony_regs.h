/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)

#include <asm/mach-symphony/symphony_reg_base_addr.h>
#include <asm/mach-symphony/symphony_regs.h>

#elif defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)

#include <mach/symphony_reg_base_addr.h>
#include <mach/symphony_regs.h>

#else

#error "Please config select one correct chipset"

#endif

