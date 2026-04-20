
#include <linux/kernel.h>
#include <linux/module.h>
//#include <mach/hardware.h>

#include "mt_type.h"
#include "mt_reg_common.h"
#if 0
volatile MT_REG_SYSCTRL_S   *g_pstRegSysCtrl    = (MT_REG_SYSCTRL_S*)IO_ADDRESS(MT_SYS_BASE_ADDR);
volatile MT_REG_PERI_S      *g_pstRegPeri       = (MT_REG_PERI_S*)IO_ADDRESS(MT_PERI_BASE_ADDR);
volatile MT_REG_IO_S        *g_pstRegIO         = (MT_REG_IO_S*)IO_ADDRESS(MT_IO_BASE_ADDR);
volatile MT_REG_CRG_S       *g_pstRegCrg        = (MT_REG_CRG_S*)IO_ADDRESS(MT_CRG_BASE_ADDR);

EXPORT_SYMBOL(g_pstRegSysCtrl);
EXPORT_SYMBOL(g_pstRegPeri);
EXPORT_SYMBOL(g_pstRegIO);
EXPORT_SYMBOL(g_pstRegCrg);
#endif
