/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/types.h>
#include <mt_mach/symphony_regs.h>
#include "mt_mach/symphony_io.h"
#include "hw_opc.h"

static void opc_irq_delay(int cnt)
{
	while(cnt--);
}

void hw_opc_clear_interrupt(opc_handle_t *handle)
{
	ulong opc_base     = handle->base;
	ulong opc_clr_lo   = OPC_REE_IRQ_CLR_REG(opc_base);
	ulong opc_clr_hi   = OPC_REE_IRQ_CLR_REG(opc_base) + 4;

	HAL_PUT_U32((volatile mt_u32 *)opc_clr_lo, DISP_OPC_MASK_LO_VALUE);
	HAL_PUT_U32((volatile mt_u32 *)opc_clr_hi, DISP_OPC_MASK_HI_VALUE);
	opc_irq_delay(20);
	HAL_PUT_U32((volatile mt_u32 *)opc_clr_lo, 0);
	HAL_PUT_U32((volatile mt_u32 *)opc_clr_hi, 0);
}


void hw_opc_onoff_int_mask(opc_handle_t *handle, uint32_t onoff)
{
	ulong opc_base      = handle->base;
	ulong opc_mask_lo   = OPC_REE_IRQ_MASK_REG(opc_base);
	ulong opc_mask_hi   = OPC_REE_IRQ_MASK_REG(opc_base) + 4;


	if (onoff){
		HAL_PUT_U32((volatile mt_u32 *)opc_mask_lo, DISP_OPC_MASK_LO_VALUE);
		HAL_PUT_U32((volatile mt_u32 *)opc_mask_hi, DISP_OPC_MASK_HI_VALUE);
	}else{
		HAL_PUT_U32((volatile mt_u32 *)opc_mask_lo, 0);
		HAL_PUT_U32((volatile mt_u32 *)opc_mask_hi, 0);
	}
}

uint64_t hw_opc_get_status(opc_handle_t *handle)
{
	ulong opc_base      = handle->base;
	ulong opc_st_lo     = OPC_VIOLATION_STATUS_REG(opc_base);
	ulong opc_st_hi     = OPC_VIOLATION_STATUS_REG(opc_base) + 4;

	opc_st_lo           = HAL_GET_U32((volatile mt_u32 *)opc_st_lo);
	opc_st_hi           = HAL_GET_U32((volatile mt_u32 *)opc_st_hi);

	return (uint64_t)(((uint64_t)opc_st_hi << 32) | opc_st_lo);
}

