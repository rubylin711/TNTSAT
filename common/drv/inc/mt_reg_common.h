
#ifndef __MT_REG_COMMON_H__
#define __MT_REG_COMMON_H__

//#include "mt_reg_sys.h"
#include "./mt_symphony/mt_reg_crg.h"

#define MT_SYS_BASE_ADDR        0xF8000000
#define MT_PERI_BASE_ADDR       0xF8A20000
#define MT_IO_BASE_ADDR         0xF8A21000
#define MT_CRG_BASE_ADDR        0xF8A22000

typedef S_CRG_REGS_TYPE         MT_REG_CRG_S;
extern volatile MT_REG_CRG_S    *g_pstRegCrg;

#if 0
typedef S_SYSCTRL_REGS_TYPE     MT_REG_SYSCTRL_S;
typedef S_PERICTRL_REGS_TYPE    MT_REG_PERI_S;
typedef S_IO_REGS_TYPE          MT_REG_IO_S;


extern volatile MT_REG_SYSCTRL_S    *g_pstRegSysCtrl;
extern volatile MT_REG_IO_S         *g_pstRegIO;
extern volatile MT_REG_PERI_S       *g_pstRegPeri;
#endif
#endif  // __MT_REG_COMMON_H__

