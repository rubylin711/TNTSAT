/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __DRV_REG_PROC_H__
#define __DRV_REG_PROC_H__
//#include "si_lib_obj_api.h"
#include "si_drv_cra_api.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

void HdmiPrivDrvWrReg32(SiiInst_t inst, ulong addr, uint32_t val);
uint32_t HdmiPrivDrvRdReg32(SiiInst_t inst, ulong addr );
void HdmiPrivDrvSetBit32(SiiInst_t inst, ulong addr, uint32_t mask );
void HdmiPrivDrvClrBit32(SiiInst_t inst, ulong addr, uint32_t mask );
void HdmiPrivDrvPutBit32(SiiInst_t inst, ulong addr, uint32_t mask, uint32_t val );
uint8_t             SiiDrvCraRdReg8_4dump(SiiInst_t inst,  SiiDrvCraAddr_t addr );

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __DRV_REG_PROC_H__ */

